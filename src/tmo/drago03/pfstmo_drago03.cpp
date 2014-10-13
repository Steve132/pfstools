/**
 * @brief Adaptive logarithmic tone mapping
 * 
 * Adaptive logarithmic mapping for displaying high contrast
 * scenes. 
 * F. Drago, K. Myszkowski, T. Annen, and N. Chiba. In Eurographics 2003.
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
 * @author Grzegorz Krawczyk, <krawczyk@mpi-inf.mpg.de>
 *
 * $Id: pfstmo_drago03.cpp,v 1.4 2009/04/15 11:49:32 julians37 Exp $
 */

#include <config.h>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>

#include <pfs.h>

#include "tmo_drago03.h"

#define PROG_NAME "pfstmo_drago03"

using namespace std;

class QuietException 
{
};


void printHelp()
{
  fprintf( stderr, PROG_NAME " (" PACKAGE_VERSION ") : \n"
    "\t [--bias <val>] [--verbose] [--help] \n"
    "See man page for more information. \n" );
}

void tmo_drago03( int argc, char* argv[] )
{
  pfs::DOMIO pfsio;

  //--- default tone mapping parameters;
  float biasValue = 0.85f;

  //--- process command line args
  bool verbose = false;

  static struct option cmdLineOptions[] = {
    { "help", no_argument, NULL, 'h' },
    { "verbose", no_argument, NULL, 'v' },
    { "bias", required_argument, NULL, 'b' },
    { NULL, 0, NULL, 0 }
  };

  int optionIndex = 0;
  while( 1 ) {
    int c = getopt_long (argc, argv, "vhb:", cmdLineOptions, &optionIndex);
    if( c == -1 ) break;
    switch( c ) {
    case 'h':
      printHelp();
      throw QuietException();
    case 'v':
      verbose = true;
      break;
    case 'b':
      biasValue = (float)strtod( optarg, NULL );
      if( biasValue<0.0f || biasValue>1.0f )
        throw pfs::Exception("incorrect bias value, accepted range is (0..1)");
      break;
    case '?':
      throw QuietException();
    case ':':
      throw QuietException();
    }
  }

  VERBOSE_STR << ": bias: " << biasValue << endl;
   
  while( true ) {
    pfs::Frame *frame = pfsio.readFrame( stdin );
    if( frame == NULL ) break; // No more frames

    pfs::Channel *X, *Y, *Z;
    frame->getXYZChannels( X, Y, Z );
    frame->getTags()->setString("LUMINANCE", "RELATIVE");
    //---

    if( Y == NULL )
      throw pfs::Exception( "Missing X, Y, Z channels in the PFS stream" );

    int w = Y->getCols();
    int h = Y->getRows();
        
    float maxLum,avLum;
    calculateLuminance( w, h, Y->getRawData(), avLum, maxLum );
    VERBOSE_STR << ": maximum luminance: " << maxLum << endl;
    VERBOSE_STR << ": average luminance: " << avLum << endl;

    pfs::Array2DImpl* L = new pfs::Array2DImpl(w,h);
    tmo_drago03(w, h, Y->getRawData(), L->getRawData(), maxLum, avLum, biasValue);
		
    for( int y=0 ; y<h ; y++ )
    {
      for( int x=0 ; x<w ; x++ )
      {
        float scale = (*L)(x,y) / (*Y)(x,y);
        (*Y)(x,y) *= scale;
        (*X)(x,y) *= scale;
        (*Z)(x,y) *= scale;
      }
    }

    delete L;
    
    //---
    pfsio.writeFrame( frame, stdout );
    pfsio.freeFrame( frame );        
  }
}


int main( int argc, char* argv[] )
{
  try {
    tmo_drago03( argc, argv );
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
