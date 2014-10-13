/**
 * @brief Contrast mapping TMO
 *
 * From:
 * 
 * Rafal Mantiuk, Karol Myszkowski, Hans-Peter Seidel.
 * A Perceptual Framework for Contrast Processing of High Dynamic Range Images
 * In: ACM Transactions on Applied Perception 3 (3), pp. 286-308, 2006
 * http://www.mpi-inf.mpg.de/~mantiuk/contrast_domain/
 *
 * This file is a part of PFSTMO package.
 * ---------------------------------------------------------------------- 
 * Copyright (C) 2007 Grzegorz Krawczyk
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
 * @author Rafal Mantiuk, <mantiuk@gmail.com>
 * Updated 2007/12/17 by Ed Brambley <E.J.Brambley@damtp.cam.ac.uk>
 *
 * $Id: pfstmo_mantiuk06.cpp,v 1.10 2009/09/02 01:11:39 rafm Exp $
 */

#include <config.h>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>
#include <fcntl.h>
#include <iostream>

#include <pfs.h>

#include "contrast_domain.h"


#define PROG_NAME "pfstmo_mantiuk06"

class QuietException 
{
};

using namespace std;

void printHelp()
{
  fprintf( stderr, PROG_NAME " (" PACKAGE_STRING ") : \n"
    "\t[--factor <val>] [--saturation <val>] [--equalize-contrast <val>]\n"
    "\t[--help] [--quiet] [--verbose]\n"
    "See man page for more information.\n" );
}


bool verbose = false;
bool quiet = false;

int progress_report( int progress )
{
  if( !quiet ) {  
    fprintf( stderr, "\rcompleted %d%%", progress );
    if( progress == 100 )
      fprintf( stderr, "\n" );    
  }

  return PFSTMO_CB_CONTINUE;
}



void tmo_mantiuk06(int argc, char * argv[])
{

  //--- default tone mapping parameters;
  float scaleFactor = 0.1f;
  float saturationFactor = 0.8f;
  bool cont_map = false, cont_eq = false, bcg = false;
  int itmax = 200;
  float tol = 1e-3;

  //--- process command line args

  static struct option cmdLineOptions[] = {
    { "help", no_argument, NULL, 'h' },
    { "verbose", no_argument, NULL, 'v' },
//    { "bcg", no_argument, NULL, 'b' },
    { "factor", required_argument, NULL, 'f' },
    { "saturation", required_argument, NULL, 's' },
//    { "itmax", required_argument, NULL, 'm' },
//    { "tol", required_argument, NULL, 't' },
    { "quiet", no_argument, NULL, 'q' },    
    { "equalize-contrast", required_argument, NULL, 'e' },
    { NULL, 0, NULL, 0 }
  };

  int optionIndex = 0;
  while( 1 ) {
    int c = getopt_long (argc, argv, "vhf:s:e:q", cmdLineOptions, &optionIndex);
    if( c == -1 ) break;
    switch( c ) {
    case 'h':
      printHelp();
      throw QuietException();
    case 'q':
      quiet = true;
      break;
    case 'v':
      verbose = true;
      break;
    case 'b':
      bcg = true;
      break;
    case 'e':
      cont_eq = true;
      scaleFactor = 0.0f - (float)strtod( optarg, NULL );
      if( scaleFactor > 0.0f )
        throw pfs::Exception("incorrect contrast scale factor, accepted range is any positive number");
      break;
    case 's':
      saturationFactor = (float)strtod( optarg, NULL );
      if( saturationFactor < 0.0f || saturationFactor > 2.0f )
        throw pfs::Exception("incorrect saturation factor, accepted range is (0..2)");
      break;
    case 'f':
      cont_map = true;
      scaleFactor = (float)strtod( optarg, NULL );
      if( scaleFactor < 0.0f || scaleFactor > 1.0f )
        throw pfs::Exception("incorrect contrast scale factor, accepted range is (0..1)");
      break;
//     case 'i':
//       interpolate_method = atoi( optarg );
//       if (interpolate_method < 1 || interpolate_method > 3)
//         throw pfs::Exception("incorrect interpolation method, accepted values are 1, 2 or 3.");
//       break;
//     case 'm':
//       itmax = atoi( optarg );
//       if (itmax < 1)
//         throw pfs::Exception("incorrect maximum number of iterations.");
//       break;
//     case 't':
//       tol = (float)strtod( optarg, NULL );
//       if( tol < 0.0f || tol > 1.0f )
//         throw pfs::Exception("incorrect convergence tolerance, accepted range is (0..1)");
//      break;
    case '?':
      throw QuietException();
    case ':':
      throw QuietException();
    }
  }

  if( cont_eq && cont_map )
    throw pfs::Exception( "the 'factor' parameter cannot be used in combination with contrast equalization" );

  if( scaleFactor < 0 ) {
    VERBOSE_STR << "algorithm: contrast equalization" << endl;
    VERBOSE_STR << "contrast scale factor = " << -scaleFactor << endl;
  } else {
    VERBOSE_STR << "algorithm: contrast mapping" << endl;
    VERBOSE_STR << "contrast scale factor = " << scaleFactor << endl;
  }
  
  VERBOSE_STR << "saturation factor = " << saturationFactor << endl;
  
  if (bcg)
    {
      VERBOSE_STR << "using biconjugate gradients (itmax = " << itmax << ", tol = " << tol << ")." << endl;
    }
  else
    {
      VERBOSE_STR << "using conjugate gradients (itmax = " << itmax << ", tol = " << tol << ")." << endl;
    }   

  pfs::DOMIO pfsio;
	
  while( true ) {
    pfs::Frame *frame = pfsio.readFrame( stdin );
    if( frame == NULL )
      break;

    pfs::Channel *inX, *inY, *inZ;
	
    frame->getXYZChannels(inX, inY, inZ);
    int cols = frame->getWidth();
    int rows = frame->getHeight();

    pfs::Array2DImpl R( cols, rows );
  
    pfs::transformColorSpace( pfs::CS_XYZ, inX, inY, inZ, pfs::CS_RGB, inX, &R, inZ );

    tmo_mantiuk06_contmap( cols, rows, inX->getRawData(), R.getRawData(), inZ->getRawData(), inY->getRawData(),
      scaleFactor, saturationFactor, bcg, itmax, tol, progress_report );	

    pfs::transformColorSpace( pfs::CS_RGB, inX, &R, inZ, pfs::CS_XYZ, inX, inY, inZ );
    frame->getTags()->setString("LUMINANCE", "RELATIVE");
  
    pfsio.writeFrame( frame, stdout );
    pfsio.freeFrame(frame);
  }
}


int main( int argc, char* argv[] )
{
  try {
    tmo_mantiuk06( argc, argv );
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

