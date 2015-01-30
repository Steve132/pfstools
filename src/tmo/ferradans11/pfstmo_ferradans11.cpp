/**
 * @file pfstmo_ferradans11.cpp
 * @brief Tone map RGB channels using Ferradans11 model
 *
 * An Analysis of Visual Adaptation and Contrast Perception for Tone Mapping
 * S. Ferradans, M. Bertalmio, E. Provenzi, V. Caselles
 * In IEEE Trans. Pattern Analysis and Machine Intelligence 2011
 *
 * 
 * This file is a part of PFSTMO package.
 * ---------------------------------------------------------------------- 
 * Copyright (C) 2013 Sira Ferradans
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
 * @author Sira Ferradans,
 *
 */

#include <stdlib.h>
 
#include <getopt.h>
#include <pfs.h>
#include "pfstmo.h"

#include "Imagen.h"

#define PROG_NAME "pfstmo_ferradans11"

/// verbose mode
bool verbose = false;

class QuietException 
{
};

void printHelp()
{
  fprintf( stderr, PROG_NAME " (" PACKAGE_STRING ") : \n"
    "\t[--rho <val>] related to overall intensity of the final output. Its range is approx (-10,10) although it might depend on the image. \n"
    "\t[--inv_alpha <val>] related to the contrast resolution. The bigger the more local contrast. For a good constrast resolution we suggest the value 0.2\n"
    "See man page for more information.\n" );
}

void pfstmo_ferradans11( int argc, char* argv[] )
{
  pfs::DOMIO pfsio;

  //--- default tone mapping parameters;
  float rho = -4;
  float inv_alpha = 0.1;
    

  static struct option cmdLineOptions[] = {
    { "help", no_argument, NULL, 'h' },
    { "rho", required_argument, NULL, 'r' },
    { "inv_alpha", required_argument, NULL, 'a' },
    { NULL, 0, NULL, 0 }
  };

  static const char optstring[] = "hr:a:";
    
  int optionIndex = 0;
  while( 1 ) {
   // int c = getopt_long (argc, argv, "hvma:b:g:s:n:d:w:k:", cmdLineOptions, &optionIndex);
      int c = getopt_long (argc, argv, optstring, cmdLineOptions, &optionIndex);
     
      if( c == -1 ){ break;}
      
    switch( c ) {
    case 'h':
      printHelp();
      throw QuietException();
    case 'v':
      verbose = true;
      break;
    case 'r':
      rho = (float)strtod( optarg, NULL );
      break;
    case 'a':
      inv_alpha = (float)strtod( optarg, NULL );
      if( inv_alpha<=0.0f )
        throw pfs::Exception("inv_alpha value out of range, should be >0");
      break;
   case '?':
      printHelp();
      throw QuietException();
      
   case ':':
      printHelp();
      throw QuietException();
      
   }
  }
  
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
      int w = Z->getCols();
      int h = Z->getRows();

    // in-place color space transform
      pfs::Array2DImpl* R = new pfs::Array2DImpl(w,h);
      pfs::Array2DImpl* G = new pfs::Array2DImpl(w,h);
      pfs::Array2DImpl* B = new pfs::Array2DImpl(w,h);
      
      pfs::transformColorSpace( pfs::CS_XYZ, X, Y, Z, pfs::CS_RGB, R, G, B );
     
      tmo_ferradans11(w, h, R->getRawData(), G->getRawData(), B->getRawData(),
                      rho, inv_alpha);
   
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
    pfstmo_ferradans11( argc, argv );
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
