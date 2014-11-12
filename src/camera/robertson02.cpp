/**
 * @brief Robertson02 algorithm for automatic self-calibration.
 *
 * 
 * This file is a part of PFS CALIBRATION package.
 * ---------------------------------------------------------------------- 
 * Copyright (C) 2004 Grzegorz Krawczyk
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * ---------------------------------------------------------------------- 
 * 
 * @author Grzegorz Krawczyk, <gkrawczyk@users.sourceforge.net>
 *         Ivo Ihrke        , <ihrke@mmci.uni-saarland.de>
 *
 * $Id: robertson02.cpp,v 1.11 2011/02/25 13:45:14 ihrke Exp $
 */


#include <config.h>

#include <iostream>
#include <vector>
#include <cstdlib>

#include <math.h>

#include <responses.h>
#include <robertson02.h>

#define PROG_NAME "robertson02"

// maximum iterations after algorithm accepts local minima
#define MAXIT 100

// maximum accepted error
#define MAX_DELTA 1e-5f

extern bool verbose; /* verbose should be declared for each standalone code */

float normalize_rcurve( float *rcurve, int M );


int robertson02_applyResponse( pfs::Array2D *xj,         
			       const ExposureList &imgs, 
			       const float *rcurve, 
			       const float *weights, 
			       int M )
{

  // number of exposures
  int N = imgs.size();

  // frame size
  int width  = xj -> getCols( );
  int height = xj -> getRows( );

  // number of saturated pixels
  int saturated_pixels = 0;
  
  
  // all pixels
  for( int j = 0; j < width * height; j++ )
    {
      // all exposures for each pixel
      float sum = 0.0f;
      float div = 0.0f;

      for( int i = 0 ; i < N ; i++ )
	{
	  int   m  = (int) (*imgs[ i ].yi)( j );
	  float ti = imgs[ i ].ti;
	   
	  sum += weights [ m ] / ti * rcurve [ m ];
	  div += weights [ m ];
	}

    
      if( div != 0.0f )
	(*xj)( j ) = sum / div;
      else
	(*xj)( j ) = 0.0f;
    
    }

  return saturated_pixels;

}


int robertson02_getResponse( pfs::Array2D       *xj, 
			     const ExposureList &imgs,
			     float              *rcurve, 
			     const float        *weights, 
			     int M )
{
  // number of exposures
  int N = imgs.size();
    
  // frame size
  int width  = imgs[0].yi -> getCols( );
  int height = imgs[0].yi -> getRows( );

  // number of saturated pixels
  int saturated_pixels = 0;

  // indices
  int i, j, m;
  
  float* rcurve_prev = new float[ M ];	// previous response

  if( rcurve_prev == NULL )
    {
      std::cerr << "robertson02: could not allocate memory for camera response" << std::endl;
      exit(1);
    }

  // 0. Initialization

  normalize_rcurve( rcurve, M );

  for( m = 0 ; m < M ; m++ )
    rcurve_prev [ m ] = rcurve [ m ];

  robertson02_applyResponse( xj, imgs, rcurve, weights, M );

  // Optimization process
  bool   converged  = false;
  long  *cardEm     = new long [ M ];
  float *sum        = new float[ M ];

  if( sum    == NULL || 
      cardEm == NULL )
    {
      std::cerr << "robertson02: could not allocate memory for optimization process" << std::endl;
      exit(1);
    }
  
  int   cur_it  = 0;
  float pdelta  = 0.0f;

  while( !converged )
    {
  
      // 1. Minimize with respect to rcurve
      for( m = 0 ; m < M ; m++ )
	{
	  cardEm [ m ] = 0;
	  sum[ m ] = 0.0f;
	}
    
      for( i = 0 ; i < N ; i++ )
	{

	  pfs::Array2D* yi = imgs[i].yi;
	  float         ti = imgs[ i ].ti;

	  for( j = 0 ; j < width * height ; j++ )
	    {
	      m = (int) (*yi)( j );

	      if( m < M && m >= 0 )
		{
		  sum[ m ] += ti * (*xj)( j );
		  cardEm[ m ] ++;
		}
	      else
		std::cerr << "robertson02: m out of range: " << m << std::endl;
	    }
	}


      for( m = 0; m < M ; m++ )
	{
	  if( cardEm[ m ] != 0 )
	    rcurve [ m ] = sum [ m ] / cardEm [ m ];
	  else
	    rcurve [ m ] = 0.0f;
	}

      // 2. Normalize rcurve
      float middle_response = normalize_rcurve( rcurve, M );

      // 3. Apply new response
      saturated_pixels = robertson02_applyResponse( xj, imgs, rcurve, weights, M );

      // 4. Check stopping condition
      float delta = 0.0f;
      int   hits  = 0;

      for( m = 0 ; m < M ; m++ )
	{
	  if( rcurve[ m ] != 0.0f )
	    {
	      float diff = rcurve [ m ] - rcurve_prev [ m ];
	      
	      delta += diff * diff;
	      
	      rcurve_prev [ m ] = rcurve[ m ];
	      
	      hits++;
	    }
	}

      delta /= hits;

      VERBOSE_STR << " #" << cur_it
		  << " delta=" << delta
		  << " (coverage: " << 100*hits/M << "%)\n";
    
      if( delta < MAX_DELTA )
	converged = true;
      else if( isnan(delta) || cur_it > MAXIT )
	{
	  VERBOSE_STR << "algorithm failed to converge, too noisy data in range\n";
	  break;
	}

      pdelta = delta;
      cur_it++;
    }

  if( converged )
    VERBOSE_STR << " #" << cur_it
                << " delta=" << pdelta << " <- converged\n";

  delete[] rcurve_prev;
  delete[] cardEm;
  delete[] sum;
  
  return saturated_pixels;
}


//----------------------------------------------------------
// private part


int comp_floats( const void *a, const void *b )
{
  return ( (*((float*)a))< (*((float*)b)) ) ;
}

float normalize_rcurve( float* rcurve, int M )
{
  int   FILTER_SIZE =  M / 256;
  float mean;
  float rcurve_filt [ M ];
  float to_sort [ 2 * FILTER_SIZE + 1 ];

  for ( int i = 0; i < M; i++ )
    {
      mean += rcurve [ i ];
    }
  mean /= M;

  if( mean != 0.0f )
    for( int m = 0 ; m < M ; m++ )
      {
	rcurve [ m ] /= mean;

	/* filtered curve - initialization */
	rcurve_filt [ m ] = 0.0f;
      }

  /* median filter response curve */
  for ( int m = FILTER_SIZE ; m < M - FILTER_SIZE; m++ )
    {
      for ( int i = -FILTER_SIZE; i <= FILTER_SIZE; i++ )
	to_sort [ i + FILTER_SIZE ] = rcurve[ m + i ];
      
      qsort ( to_sort, 2 * FILTER_SIZE + 1 , sizeof(float), comp_floats );
      
      rcurve_filt [ m ]  = to_sort [ FILTER_SIZE ]; 
      
    }

  /* boundaries */
  for( int m = 0 ; m < FILTER_SIZE ; m++ )
    {
      rcurve_filt [ m ]     = rcurve_filt [ FILTER_SIZE ];
      rcurve_filt [ M - m - 1 ] = rcurve_filt [ M - FILTER_SIZE - 1 ];
    }
    
  /* copy curve */
  for( int m = 0 ; m < M ; m++ )
      {
	rcurve [ m ] = rcurve_filt [ m ];
      }

  return mean;
}



// float normalizeI( float* rcurve, int M )
// {
//   int Mmin, Mmax;
//   // find min max
//   for( Mmin=0 ; Mmin<M && rcurve[Mmin]==0 ; Mmin++ );
//   for( Mmax=M-1 ; Mmax>0 && rcurve[Mmax]==0 ; Mmax-- );
  
//   int Mmid = Mmin+(Mmax-Mmin)/2;
//   float mid = rcurve[Mmid];

// //   std::cerr << "robertson02: middle response, mid=" << mid
// //             << " [" << Mmid << "]"
// //             << " " << Mmin << ".." << Mmax << std::endl;
  
//   if( mid==0.0f )
//   {
//     // find first non-zero middle response
//     while( Mmid<Mmax && rcurve[Mmid]==0.0f )
//       Mmid++;
//     mid = rcurve[Mmid];
//   }

//   if( mid!=0.0f )
//     for( int m=0 ; m<M ; m++ )
//       rcurve[m] /= mid;
//   return mid;
// }

