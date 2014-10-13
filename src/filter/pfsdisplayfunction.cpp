/**
 * @brief Apply display function or inverse display function the the pfs stream
 * 
 * This file is a part of PFSTOOLS package.
 * ---------------------------------------------------------------------- 
 * Copyright (C) 2003,2004 Rafal Mantiuk and Grzegorz Krawczyk
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
 * @author Rafal Mantiuk, <mantiuk@mpi-sb.mpg.de>
 *
 * $Id: pfsdisplayfunction.cpp,v 1.2 2008/07/29 16:14:29 rafm Exp $
 */

#include <config.h>

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>

#include <pfs.h>

#include "display_function.h"

#define PROG_NAME "pfsdisplayfunction"

class QuietException 
{
};

void applyDisplayFunction( pfs::Array2D *array, DisplayFunction *df, int model_dir );

void printHelp()
{
  fprintf( stderr, PROG_NAME " [--gamma <val> | --inverse-gamma <val>] [--mul <val>] [--verbose] [--help]\n"
    "See man page for more information.\n" );
}

void processFrames( int argc, char* argv[] )
{
  pfs::DOMIO pfsio;

  DisplayFunction *df = NULL;
  bool verbose = false;

  int model_dir = 0;            // -1 to-pixels; 1 to-luminance; 0 unspecified

  df = createDisplayFunctionFromArgs( argc, argv );
  if( df == NULL )
    df = new DisplayFunctionGGBA( "lcd" );
  
  static struct option cmdLineOptions[] = {
    { "help", no_argument, NULL, 'h' },
    { "verbose", no_argument, NULL, 'v' },
    { "to-pixels", no_argument, NULL, 'p' },
    { "to-luminance", no_argument, NULL, 'l' },
    { NULL, 0, NULL, 0 }
  };
  
  int optionIndex = 0;
  while( 1 ) {
    int c = getopt_long (argc, argv, "hvpl", cmdLineOptions, &optionIndex);
    if( c == -1 ) break;
    switch( c ) {
    case 'h':
      printHelp();
      throw QuietException();
    case 'v':
      verbose = true;
      break;
    case 'l':
      model_dir = 1;
      break;
    case 'p':
      model_dir = -1;
      break;
    case '?':
      throw QuietException();
    case ':':
      throw QuietException();
    }
  }

  if( verbose ) {
    df->print( stderr );
  }
  
  bool first_frame = true;
  while( true ) {
    pfs::Frame *frame = pfsio.readFrame( stdin );
    if( frame == NULL ) break; // No more frames

    if( first_frame ) {
      const char *lum_type = frame->getTags()->getString("LUMINANCE");
      
      if( lum_type ) {

        if( model_dir == 0 ) {    // Determine model direction from the LUMINANCE tag
          if( !strcmp( lum_type, "DISPLAY" ) )
            model_dir = 1;
          else if( !strcmp( lum_type, "ABSOLUTE" ) )
            model_dir = -1;
        }      
        
        if( !strcmp( lum_type, "DISPLAY" ) && model_dir == -1 )
          std::cerr << PROG_NAME " warning: applying inverse display function to a display referred image" << std::endl;
        if( !strcmp( lum_type, "ABSOLUTE" ) && model_dir == 1 )
          std::cerr << PROG_NAME " warning: applying display function to a linear luminance or radiance image" << std::endl;
        if( !strcmp( lum_type, "RELATIVE" ) )
          std::cerr << PROG_NAME " warning: input image should be in absolute luminance / radiance units" << std::endl;
      }
    }

    if( model_dir == 0 )
      throw pfs::Exception( "specify --to-pixels or --to-luminance mapping" );    
    
    pfs::Channel *X, *Y, *Z;
    frame->getXYZChannels( X, Y, Z );

    if( X != NULL ) {           // Color, XYZ
      
      pfs::transformColorSpace( pfs::CS_XYZ, X, Y, Z, pfs::CS_RGB, X, Y, Z );
      // At this point (X,Y,Z) = (R,G,B)
        
      applyDisplayFunction( X, df, model_dir );
      applyDisplayFunction( Y, df, model_dir );
      applyDisplayFunction( Z, df, model_dir );

      pfs::transformColorSpace( pfs::CS_RGB, X, Y, Z, pfs::CS_XYZ, X, Y, Z );
      // At this point (X,Y,Z) = (X,Y,Z)
      
    } else if( (Y = frame->getChannel( "Y" )) != NULL ) {
      // Luminance only

      applyDisplayFunction( Y, df, model_dir );
      
    } else
      throw pfs::Exception( "Missing X, Y, Z channels in the PFS stream" );

    if( model_dir == -1 )
      frame->getTags()->setString("LUMINANCE", "DISPLAY");
    else if( model_dir == 1 )
      frame->getTags()->setString("LUMINANCE", "ABSOLUTE");

    first_frame = false;
    
    pfsio.writeFrame( frame, stdout );
    pfsio.freeFrame( frame );        
  }
}


void applyDisplayFunction( pfs::Array2D *array, DisplayFunction *df, int model_dir )
{
  int imgSize = array->getRows()*array->getCols();
  for( int index = 0; index < imgSize ; index++ ) {    
    float &v = (*array)(index);
    if( model_dir == -1 )
      v = df->inv_display( v );
    else 
      v = df->display( v );
  }
}


int main( int argc, char* argv[] )
{
  try {
    processFrames( argc, argv );
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
