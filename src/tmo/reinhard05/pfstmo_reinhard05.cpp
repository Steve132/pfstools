/**
 * @file pfstmo_reinhard05.cpp
 * @brief Tone map XYZ channels using Reinhard05 model
 *
 * Dynamic Range Reduction Inspired by Photoreceptor Physiology.
 * E. Reinhard and K. Devlin.
 * In IEEE Transactions on Visualization and Computer Graphics, 2005.
 *
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
 * @author Grzegorz Krawczyk, <krawczyk@mpi-sb.mpg.de>
 *
 * $Id: pfstmo_reinhard05.cpp,v 1.2 2008/09/04 12:46:49 julians37 Exp $
 */

#include <config.h>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>

#include <pfs.h>

#include "tmo_reinhard05.h"

using namespace std;

#define PROG_NAME "pfstmo_reinhard05"

/// verbose mode
bool verbose = false;

class QuietException 
{
};

void printHelp()
{
  fprintf( stderr, PROG_NAME  " (" PACKAGE_STRING "): \n"
    "\t[--brightness <val>] [--saturation <val>] [--light <val>]\n"
    "\t[--verbose] [--help] \n"
    "See man page for more information.\n" );
}

void pfstmo_reinhard05( int argc, char* argv[] )
{
  pfs::DOMIO pfsio;

  //--- default tone mapping parameters;
  float brightness = 0.0f;
  float chromaticadaptation = 0.5f;
  float lightadaptation = 0.75f;

  static struct option cmdLineOptions[] = {
    { "help", no_argument, NULL, 'h' },
    { "verbose", no_argument, NULL, 'v' },
    { "brightness", required_argument, NULL, 'b' },
    { "chromatic", required_argument, NULL, 'c' },
    { "light", required_argument, NULL, 'l' },
    { NULL, 0, NULL, 0 }
  };

  int optionIndex = 0;
  while( 1 ) {
    int c = getopt_long (argc, argv, "hvb:c:l:", cmdLineOptions, &optionIndex);
    if( c == -1 ) break;
    switch( c ) {
    case 'h':
      printHelp();
      throw QuietException();
    case 'v':
      verbose = true;
      break;
    case 'b':
      brightness = (float)strtod( optarg, NULL );
      if( brightness<-8.0f || brightness>8.0f )
        throw pfs::Exception("brightness value out of range, should be <-8..8>");
      break;
    case 'c':
      chromaticadaptation = (float)strtod( optarg, NULL );
      if( chromaticadaptation<0.0f || chromaticadaptation>1.0f )
        throw pfs::Exception("chromatic adaptation value out of range, should be <0..1>");
      break;
    case 'l':
      lightadaptation = (float)strtod( optarg, NULL );
      if( lightadaptation<0.0f || lightadaptation>1.0f )
        throw pfs::Exception("light adaptation value out of range, should be <0..1>");
      break;
    case '?':
      throw QuietException();
    case ':':
      throw QuietException();
    }
  }

  VERBOSE_STR << "brightness: " << brightness << endl;
  VERBOSE_STR << "chromatic adaptation: " << chromaticadaptation << endl;
  VERBOSE_STR << "light adaptation: " << lightadaptation << endl;

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
    pfs::Array2DImpl* R = new pfs::Array2DImpl(w,h);
    pfs::Array2DImpl* G = new pfs::Array2DImpl(w,h);
    pfs::Array2DImpl* B = new pfs::Array2DImpl(w,h);

    pfs::transformColorSpace( pfs::CS_XYZ, X, Y, Z, pfs::CS_RGB, R, G, B );

    tmo_reinhard05(w, h, R->getRawData(), G->getRawData(), B->getRawData(), Y->getRawData(), brightness, chromaticadaptation, lightadaptation );
    pfs::transformColorSpace( pfs::CS_SRGB, R, G, B, pfs::CS_XYZ, X, Y, Z );

    delete R;
    delete G;
    delete B;

    //---
    pfsio.writeFrame( frame, stdout );
    pfsio.freeFrame( frame );        
  }
}

int main( int argc, char* argv[] )
{
  try {
    pfstmo_reinhard05( argc, argv );
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
