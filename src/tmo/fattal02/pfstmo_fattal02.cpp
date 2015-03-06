/**
 * @file pfstmo_fattal02.cpp
 * @brief Tone map XYZ channels using Fattal02 model
 *
 * Gradient Domain High Dynamic Range Compression
 * R. Fattal, D. Lischinski, and M. Werman
 * In ACM Transactions on Graphics, 2002.
 *
 * 
 * This file is a part of PFSTMO package.
 * ---------------------------------------------------------------------- 
 * Copyright (C) 2003,2004 Grzegorz Krawczyk
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
 * @author Grzegorz Krawczyk, <krawczyk@mpi-sb.mpg.de>
 *
 * $Id: pfstmo_fattal02.cpp,v 1.10 2012/06/22 12:01:32 rafm Exp $
 */

#include <config.h>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>

#include <pfs.h>

#include "tmo_fattal02.h"

using namespace std;

#define PROG_NAME "pfstmo_fattal02"

/// verbose mode
bool verbose = false;

class QuietException 
{
};

void printHelp()
{
  fprintf( stderr, PROG_NAME " (" PACKAGE_STRING ") : \n"
    "\t[--alpha <val>] [--beta <val>] \n"
    "\t[--gamma <val>] \n"
    "\t[--saturation <val>] \n"
    "\t[--noise <val>] \n"
    "\t[--detail-level <val>] \n"
    "\t[--black-point <val>] [--white-point <val>] \n"
    "\t[--multigrid] \n"
    "\t[--verbose] [--help] \n"
    "See man page for more information.\n" );
}

void pfstmo_fattal02( int argc, char* argv[] )
{
  pfs::DOMIO pfsio;

  //--- default tone mapping parameters;
  float opt_alpha = 1.0f;
  float opt_beta = 0.9f;
  float opt_gamma = -1.0f;    // not set (0.8 for fft solver, 1.0 otherwise)
  float opt_saturation=0.8f;
  float opt_noise = -1.0f;    // not set 
  int   opt_detail_level=-1;  // not set (2 for fft solver, 0 otherwise)
  float opt_black_point=0.1f;
  float opt_white_point=0.5f;

  // Use multigrid if FFTW lib not available
#if !defined(HAVE_FFTW3) || !defined(HAVE_OpenMP)
  bool  opt_fftsolver=false;
#else  
  bool  opt_fftsolver=true;
#endif
  
  static struct option cmdLineOptions[] = {
    { "help", no_argument, NULL, 'h' },
    { "verbose", no_argument, NULL, 'v' },
    { "multigrid", no_argument, NULL, 'm' },
    { "alpha", required_argument, NULL, 'a' },
    { "beta", required_argument, NULL, 'b' },
    { "gamma", required_argument, NULL, 'g' },
    { "saturation", required_argument, NULL, 's' },
    { "noise", required_argument, NULL, 'n' },
    { "detail-level", required_argument, NULL, 'd' },
    { "white-point", required_argument, NULL, 'w' },
    { "black-point", required_argument, NULL, 'l' },
    { NULL, 0, NULL, 0 }
  };

  int optionIndex = 0;
  while( 1 ) {
    int c = getopt_long (argc, argv, "hvma:b:g:s:n:d:w:k:", cmdLineOptions, &optionIndex);
    if( c == -1 ) break;
    switch( c ) {
    case 'h':
      printHelp();
      throw QuietException();
    case 'v':
      verbose = true;
      break;
    case 'm':
      opt_fftsolver = false;
      break;
    case 'a':
      opt_alpha = (float)strtod( optarg, NULL );
      if( opt_alpha<=0.0f )
        throw pfs::Exception("alpha value out of range, should be >0");
      break;
    case 'b':
      opt_beta = (float)strtod( optarg, NULL );
      if( opt_beta<=0.0f )
        throw pfs::Exception("beta value out of range, should be >0");
      break;
    case 'g':
      opt_gamma = (float)strtod( optarg, NULL );
      if( opt_gamma<=0.0f || opt_gamma>=10.0 )
        throw pfs::Exception("gamma value out of range, should be >0");
      break;
    case 's':
      opt_saturation = (float)strtod( optarg, NULL );
      if( opt_saturation<=0.0f || opt_saturation>1.0f )
        throw pfs::Exception("saturation value out of range, should be 0..1");
      break;
    case 'n':
      opt_noise = (float)strtod( optarg, NULL );
      if( opt_noise<0.0f )
        throw pfs::Exception("noise level value out of range, should be >=0");
      break;
    case 'd':
      opt_detail_level = (int) strtod( optarg, NULL );
      if( opt_detail_level<0 || opt_detail_level>9 )
        throw pfs::Exception("detail-level value out of range, should be 0..9");
      break;
    case 'w':
      opt_white_point = (float)strtod( optarg, NULL );
      if( opt_white_point<0.0f || opt_white_point>=50.0f )
        throw pfs::Exception("white-point value out of range, should be 0..50");
      break;
    case 'k':
      opt_black_point = (float)strtod( optarg, NULL );
      if( opt_black_point<0.0f || opt_black_point>=50.0f )
        throw pfs::Exception("black-point value out of range, should be 0..50");
      break;
    case '?':
      throw QuietException();
    case ':':
      throw QuietException();
    }
  }

  if( opt_fftsolver ) {
    // Default params for the fftsolver
    if(opt_detail_level==-1)
      opt_detail_level=3;     // 2 for low res, 3 is better for high res
    if(opt_gamma==-1.0f)
      opt_gamma=0.8f;
    if(opt_noise==-1.0f)
      opt_noise=0.002f;
  }

  
  // adjust noise floor if not set by user
  if( opt_noise<0.0f )
    opt_noise = opt_alpha*0.01;
  // set gamma and detail level to default values which produce the
  // same output as older programme versions using the multi-level pde solver
  if(opt_detail_level==-1)
    opt_detail_level=0;
  if(opt_gamma==-1.0f)
    opt_gamma=1.0f;

  VERBOSE_STR << "threshold gradient (alpha): " << opt_alpha << endl;
  VERBOSE_STR << "strengh of modification (beta): " << opt_beta << endl;
  VERBOSE_STR << "gamma: " << opt_gamma << endl;
  VERBOSE_STR << "noise floor: " << opt_noise << endl;  
  VERBOSE_STR << "saturation: " << opt_saturation << endl;
  VERBOSE_STR << "detail level: " << opt_detail_level << endl;
  VERBOSE_STR << "white point: " << opt_white_point << "%" << endl;
  VERBOSE_STR << "black point: " << opt_black_point << "%" << endl;
  VERBOSE_STR << "use fft pde solver: " << opt_fftsolver << endl;

  while( true ) 
  {
    pfs::Frame *frame = pfsio.readFrame( stdin );
    if( frame == NULL ) break; // No more frames

    pfs::Channel *X, *Y, *Z;
    frame->getXYZChannels( X, Y, Z );
    frame->getTags()->setString("LUMINANCE", "RELATIVE");
    //---

    if( Y==NULL || X==NULL || Z==NULL)
      throw pfs::Exception( "Missing X, Y, Z channels in the PFS stream" );
        
    // tone mapping
    int w = Y->getCols();
    int h = Y->getRows();

    pfs::Array2DImpl* L = new pfs::Array2DImpl(w,h);
    tmo_fattal02(w, h, Y->getRawData(), L->getRawData(), opt_alpha, opt_beta,
            opt_gamma, opt_noise, opt_detail_level,
            opt_black_point, opt_white_point, opt_fftsolver);

    // in-place color space transform
    pfs::Array2DImpl *G = new pfs::Array2DImpl( w, h ); // copy for G to preserve Y
    pfs::Array2D *R = X, *B = Z;
    pfs::transformColorSpace( pfs::CS_XYZ, X, Y, Z, pfs::CS_RGB, R, G, B );
    
    // Color correction 
    for( int i=0; i < w*h; i++ )
    {
      const float epsilon = 1e-4f;
      float y = max( (*Y)(i), epsilon );
      float l = max( (*L)(i), epsilon );
      (*R)(i) = powf( max((*R)(i)/y,0.f), opt_saturation ) * l;      
      (*G)(i) = powf( max((*G)(i)/y,0.f), opt_saturation ) * l;
      (*B)(i) = powf( max((*B)(i)/y,0.f), opt_saturation ) * l;
    }
    
    
    pfs::transformColorSpace( pfs::CS_RGB, R, G, B, pfs::CS_XYZ, X, Y, Z );

    delete G;    
    delete L;
    
    //---
    pfsio.writeFrame( frame, stdout );
    pfsio.freeFrame( frame );        
  }
}

int main( int argc, char* argv[] )
{
  try {
    pfstmo_fattal02( argc, argv );
  }
  catch( pfs::Exception ex ) {
    fprintf( stderr, PROG_NAME " error: %s\n", ex.getMessage() );
    return EXIT_FAILURE;
  }        
  catch( QuietException  ex ) {
    return EXIT_FAILURE;
  }        
  return EXIT_SUCCESS;
}
