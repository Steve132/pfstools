/**
 * @brief Algorithm for automatic HDR images capture.
 *
 * 
 * This file is a part of PFS CALIBRATION package.
 * ---------------------------------------------------------------------- 
 * Copyright (C) 2006 Radoslaw Mantiuk
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
 * @author Radoslaw Mantiuk, <radoslaw.mantiuk@gmail.com>
 *
 */

#include <iostream>
#include <algorithm>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pfs.h>
#include <responses.h>

#include <config.h>
#include "mitsunaga99_numerical.h"
#include "mitsunaga99.h"

#define PROG_NAME "HDRCaptureMitsunaga"

extern bool verbose; /* verbose should be declared for each standalone code */

using namespace std;

/** Capture HDR images based on a sequence of LD images of different exposures.
*
*	@param imgsR, imgsG, imgsB Input image data (for R,G and B channels).
*	@param M Number of luma levels (256 for 8-bits image).
*	@param poly_degree Desired degree of inverse response function.
*	@param Xj, Yj, Zj Containers for output data (for R, G and B channels). 
*
*	@return HDR image data in separate Red, Green and Blue channels.
*/
int  HDRCaptureMitsunaga::capture( ExposureList &imgsR,  
				   ExposureList &imgsG,  
				   ExposureList &imgsB,
				   int M, int poly_degree, 
				   pfs::Array2D* Xj, 
				   pfs::Array2D* Yj, 
				   pfs::Array2D* Zj, 
				   unsigned long samples_no,
				   float* Ir, 
				   float* Ig, 
				   float* Ib, 
				   float* weight, 
				   int comp_response ) 
{

  float co      [ CH ][ 20 ];
  float InvResp [ CH ][ M ];
  float w       [ CH ][ M ];
	
	
  if ( !comp_response ) 
    {

      for (int i = 0; i < M; i++) 
	{
	  InvResp [ R ][ i ] = Ir[ i ];
	  InvResp [ G ][ i ] = Ig[ i ];
	  InvResp [ B ][ i ] = Ib[ i ];
	  
	  w[ R ][ i ] = weight[ i ];
	  w[ G ][ i ] = weight[ i ];
	  w[ B ][ i ] = weight[ i ];
	}		
      
		
      VERBOSE_STR << "Computing ... ";

      sort( imgsR.begin(), imgsR.end(), Exposure::msort);	
      applyResponse( Xj, imgsR, InvResp[R], M, w[R]);

      sort( imgsG.begin(), imgsG.end(), Exposure::msort);	
      applyResponse( Yj, imgsG, InvResp[G], M, w[G]);

      sort( imgsB.begin(), imgsB.end(), Exposure::msort);
      applyResponse( Zj, imgsB, InvResp[B], M, w[B]);

      VERBOSE_STR << "finished" <<  endl;
    }
  else
    {
		
      // generate a set of points used to compute inverse exposure
      randomPixels( imgsR, samples_no);			
	
      // Red
      VERBOSE_STR << "Red channel ... ";
	    
      sort( imgsR.begin(), imgsR.end(), Exposure::msort);	
		
      if( getResponseFast( imgsR, poly_degree, co[R], M) > 0 ) 
	{		
	  getInverseResponseArray( poly_degree, co[R], M, InvResp[R]);

	  getDerivativeOfInverseResponseArray(poly_degree, co[R], M, w[R]);

	  applyResponse( Xj, imgsR, InvResp[R], M, w[R]);	
	}	

      VERBOSE_STR << "computed " << co[R][0] << " " << co[R][1] << " " << co[R][2] << " " << co[R][3] <<  endl;	
	

	    
      // Green
      VERBOSE_STR << "Green channel ... ";
	    
      sort( imgsG.begin(), imgsG.end(), Exposure::msort);	
		
      if( getResponseFast( imgsG, poly_degree, co[G], M) > 0 ) 
	{
	  getInverseResponseArray( poly_degree, co[G], M, InvResp[G]);

	  getDerivativeOfInverseResponseArray(poly_degree, co[G], M, w[G]);

	  applyResponse( Yj, imgsG, InvResp[G], M, w[G]);
	}	
		
      VERBOSE_STR << "computed " << co[G][0] << " " << co[G][1] << " " << co[G][2] << " " << co[G][3] <<  endl;	
	

      // Blue
      VERBOSE_STR << "Blue channel ... ";
	
      sort( imgsB.begin(), imgsB.end(), Exposure::msort);	
		
      if( getResponseFast( imgsB, poly_degree, co[B], M) > 0 ) 
	{
	  getInverseResponseArray( poly_degree, co[B], M, InvResp[B]);

	  getDerivativeOfInverseResponseArray(poly_degree, co[B], M, w[B]);

	  applyResponse( Zj, imgsB, InvResp[B], M, w[B]);	
	}
		
      VERBOSE_STR << "computed " << co[B][0] << " " << co[B][1] << " " << co[B][2] << " " << co[B][3] <<  endl;	
	
      // data for external file with the response and weights		
      for (int i = 0; i < M; i++) 
	{
	  if ( InvResp [ R ][ i ] < 0 ) 
	    Ir [ i ] = 0;
	  else 
	    Ir [ i ] = InvResp [ R ][ i ];
		
	  if ( InvResp [ G ][ i ] < 0 )
	    Ig [ i ] =  0;
	  else 
	    Ig [ i ] = InvResp [ G ][ i ];
		
	  if ( InvResp [ B ][ i ] < 0 ) 
	    Ib [ i ] = 0;
	  else 
	    Ib [ i ] = InvResp [ B ][ i ];
			

          // TODO: there should be a separate weight for each colour channel
          // weight value (inverse response divided by derivative of inverse response function)
	  weight [ i ] = (InvResp[R][i]+InvResp[G][i]+InvResp[B][i])/3.f / ( w [ R ][ i ] + w[ G ][ i ] + w[ B ][ i ] ) / 3.f; 
	}
    }

  return 0;
}


/** Computes camera inverse response function based on algorithm decribed in 
* Tomoo Mitsunaga and Shree K. Nayar in "Radiometric Self Calibration" paper.
*
*	@param imgs - input image data for a separate channel
*	@param poly_degree - desired degree of inverse response function
*	@param coef - array with computed polynomial cooficients
*	@param M - number of luma levels (256 for 8-bits data)
*/
int HDRCaptureMitsunaga::getResponse( const ExposureList &imgs, int poly_degree, float* coef, int M) {

	// number of exposures (input images)
	int Q = imgs.size();	
	if( Q < 2) {
		fprintf( stderr, "WARNING: not enough input images (mitsunaga_getResponse())\n");
		return -1;
	}
	
  	// frame size
	int P = imgs[0].yi->getCols() * imgs[0].yi->getRows();

	float R; // exposure ratio
	int N = poly_degree; // degree of polynomial
	
	float c[N][NR_LINEAR_EQUATIONS_MAX]; 
	for( int e = 0; e < N; e ++)
		for( int i = 0; i < N; i++)
			c[e][i] = 0;
				
	float b[N];
	for( int e = 0; e < N; e ++)		
		b[e] = 0;
				
	float M1, M2;
	float Imax = 1.0;
	
	for( int q = 0; q < (Q-1); q++) { // for all images - 1
	
		R = imgs[q].ti / imgs[q+1].ti;
	
		for( int p = 0; p < P; p++) { // for all pixels
		
			M1 = (*imgs[q].yi)(p); 			
			M2 = (*imgs[q+1].yi)(p); 

			float an = pow( M1, N) - R * pow( M2, N);
		
			for( int e = 0; e < N; e ++) { // for all equations
			
				float aj = pow( M1, e) - R * pow( M2, e);
			
				for( int i = 0; i < N; i++) {
			
					float ai = pow( M1, i) - R * pow( M2, i);
					c[e][i] += (( ai - an) * ( aj - an));
				}
				b[e] += (-an * Imax * ( aj - an));
			}
			
			
		}
	}

	MitsunagaNumerical::linearEquationsSystem( N, c, b);
		
	for( int e = 0; e < N; e ++) {
		if( isnan(b[e])) {
			fprintf(stderr, "ERROR: NaN cooefficient (mitsunaga_getResponse())\n");
			return 0;
		}	
	}	
						
	float cn;
	float sum = 0;
	for( int e = 0; e < N; e ++)
		sum += b[e];
	cn = 1.0 - sum;	
	
	coef[0] = cn;
	
	for(int i = (N-1); i >=0; i--) {
		coef[N-i] = b[i];
	}

	return N;
}


/** Creates radiance map based on a sequence of LDR images and a given inverse response function.
*
*	@param xj - output array with HDR luminance data (for one color channel)
*	@param imgs - input image data for a separate channel
*	@param Iresp - array with data of inverse response function (calculated based on polynomial coefficients)
*	@param M - number of luma levels (256 for 8-bits data)
*	@param w - array with weight function data
*/
void HDRCaptureMitsunaga::applyResponse( pfs::Array2D* xj, ExposureList &imgs, float *rcurve, int M, float *weights) {
  
  	// frame size
	int count  = xj->getCols() * xj->getRows();
	
	int width  = xj->getCols();
	int height = xj->getRows();

	// number of exposures
	int N = imgs.size();

#if 1
        float ex_sum = 0; // to normalize exposure time
        for( int i=0 ; i<N ; i++ ) 
          ex_sum += imgs[i].ti;
          
	// all pixels
	for( int j = 0 ; j < count ; j++ ) {
	
	  float sum = 0;
	  float div = 0;
	  float ex;
	  int m;
	  float ww;

		
	  // all exposures 
	  for( int i=0 ; i<N ; i++ )  {				
												
	    m = lroundf(( (*imgs[i].yi)(j) * (float)M ));

	    if(m >= M)
	      m = M - 1;
		  
	    if( m < 0 ) 
	      {
		fprintf( stderr, "WARNING: wrong pixel value (mitsunaga_applyResponse())\n");
		continue;
	      }			
		  
	    ww   = weights [ m ];

            // bofore the response curve handling was unified:
//	    ww   = rcurve [ m ] / weights [ m ];	// weight value (inverse response divided by derivative of inverse response function)
            
	    ex   = imgs [ i ].ti / ex_sum;		// normalized exposure value
	    sum += ww * rcurve [ m ] / ex;
	    div += ww;

	  }

	  if( div != 0.0f )
	    (*xj)( j ) = sum / div;
	  else
	    (*xj)( j ) = 0.0f;
	
	}
#endif

	/* this is robertson style response curve application - for testing purposes only */
#if 1
  // all pixels
  for( int j = 0; j < width * height; j++ )
    {
      // all exposures for each pixel
      float sum = 0.0f;
      float div = 0.0f;

      for( int i = 0 ; i < N ; i++ )
	{
	  int   m  = (int) ((*imgs[ i ].yi)( j ) * (float)M);
          
          
	  float ti = imgs[ i ].ti;

          float w = weights[m];
          
	  sum += w / ti * rcurve [ m ];
	  div += w;
	}

    
      if( div != 0.0f )
	(*xj)( j ) = sum / div;
      else
	(*xj)( j ) = 0.0f;
    
    }
#endif
	return;
}

/** 
* Computes values of polynomial response function and puts them into Iresp[].
*
*	@param c_cc - polynomial degree
*	@param c - polynomial coefficients
*	@param cc - number of luma levels (256 for 8-bits data)
*	@param Iresp - output array with data of the inverse response function
*/
void HDRCaptureMitsunaga::getInverseResponseArray( int c_cc, float* c, int cc, float* Iresp ) {

	for( int m = 0; m < cc; m++) {
	
//          float v = float(m);
          float v = float(m)/(float)cc;
          float sum = 0;
		
		for( int i = 0; i < (c_cc+1); i++) {
		
			sum += ( pow( v, (c_cc - i)) * c[i]);
		}
		Iresp[m] = sum;
	}

	return;
}

/** 
* Computes the derivative of inverse response function and puts the values into w[].
*
*	@param c_cc - polynomial degree
*	@param c - polynomial coefficients
*	@param cc - number of luma levels (256 for 8-bits data)
*	@param Iresp - output array with derivative of the inverse response function
*/
void HDRCaptureMitsunaga::getDerivativeOfInverseResponseArray( int c_cc, float* c, int cc, float* w ) {

	float der;
	for(int m = 0; m < cc; m++) {
	
		der = 0;

//                float v = float(m);
                float v = float(m)/(float)cc;
                
		for( int i = 0; i < c_cc; i++) {
		
			der += ( pow( (float)v, (c_cc - i - 1)) * c[i] * (float)(c_cc-i));
		}
		w[m] = der;
	}

	return;	
}

/** 
* Generates a set of random pixels to speed-up calculations of inverse response,
*	only these pixels are taken into account.
*
*	@param imgs - input image data for a separate channel
*	@param no - number of points which will be selected
*/
int HDRCaptureMitsunaga::randomPixels( const ExposureList &imgs, unsigned long no) {

	if( imgs.size() < 1 ) {
		VERBOSE_STR << "ERROR: wrong input image list" << endl;  
		return 1;
	}
	// frame size
	unsigned long idx;
	float N = imgs[0].yi->getCols() * imgs[0].yi->getRows();
	 
	for( unsigned long i = 0; i < no; i++) {
	
		idx = (unsigned long)( (float)rand() / (float)RAND_MAX * N);
		pixelList.push_back( idx);
		
	}

	//VERBOSE_STR << "pixelList: " << pixelList.size() << endl;

	return 0;
}


/** 
* Computes camera inverse response function based on algorithm decribed in 
* Tomoo Mitsunaga and Shree K. Nayar in "Radiometric Self Calibration" paper.
* This function uses selected pixels only to compute inverse response.
*
*	@param imgs - list of input images (one channel) and exposure times
*	@param poly_degree - degree of output inverse response polynomial
*	@param coef - computed (output) polynomial coeficients (poly_degree+1 for each channel)
*	@param M - number of color levels (256 for 8-bits channels)
*
*	@return Coefficients of inverse response polynomial stored in coef[poly_degree+1].
*/
int HDRCaptureMitsunaga::getResponseFast( const ExposureList &imgs, int poly_degree, float* coef, int M) {

	// number of exposures (input images)
	int Q = imgs.size();	
	if( Q < 2) {
		fprintf( stderr, "WARNING: not enough input images (mitsunaga_getResponse())\n");
		return -1;
	}
	
  	// frame size
	int P = imgs[0].yi->getCols() * imgs[0].yi->getRows();

	float R; // exposure ratio
	int N = poly_degree; // degree of polynomial
	
	float c[N][NR_LINEAR_EQUATIONS_MAX]; 
	for( int e = 0; e < N; e ++)
		for( int i = 0; i < N; i++)
			c[e][i] = 0;
				
	float b[N];
	for( int e = 0; e < N; e ++)		
		b[e] = 0;
				
	float M1, M2;
	float Imax = 1.0;
	
	list<unsigned long>::iterator i;

  	for( int q = 0; q < (Q-1); q++) { // for all images - 1
	
		R = imgs[q].ti / imgs[q+1].ti;

		 for( i = pixelList.begin(); i  != pixelList.end(); ++i) {
		
			M1 = (*imgs[q].yi)(*i); 			
			M2 = (*imgs[q+1].yi)(*i); 

			float an = pow( M1, N) - R * pow( M2, N);
		
			for( int e = 0; e < N; e ++) { // for all equations
			
				float aj = pow( M1, e) - R * pow( M2, e);
			
				for( int i = 0; i < N; i++) {
			
					float ai = pow( M1, i) - R * pow( M2, i);
					c[e][i] += (( ai - an) * ( aj - an));
				}
				b[e] += (-an * Imax * ( aj - an));
			}
			
			
		}
	}

	MitsunagaNumerical::linearEquationsSystem( N, c, b);
		
	for( int e = 0; e < N; e ++) {
		if( isnan(b[e])) {
			fprintf(stderr, "ERROR: NaN cooefficient (mitsunaga_getResponse())\n");
			return 0;
		}	
	}	
						
	float cn;
	float sum = 0;
	for( int e = 0; e < N; e ++)
		sum += b[e];
	cn = 1.0 - sum;	
	
	coef[0] = cn;
	
	for(int i = (N-1); i >=0; i--) {
		coef[N-i] = b[i];
	}

	return N;
}






