/**
 * @brief Convert luminance in images to absolute measure
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
 * $Id: pfsabsolute.cpp,v 1.3 2006/08/14 16:48:31 rafm Exp $
 */

#include <config.h>

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>

#include <pfs.h>

#define PROG_NAME "pfsabsolute"

class QuietException 
{
};


inline void multiplyArray(pfs::Array2D *z, const pfs::Array2D *x, const float y)
{
  assert( x->getRows() == z->getRows() );
  assert( x->getCols() == z->getCols() );

  if( y == 1.0f ) return;
  
  const int elements = x->getRows()*x->getCols();
  for( int i = 0; i < elements; i++ )
    (*z)(i) = (*x)(i) * y;
}

void printHelp()
{
  fprintf( stderr, PROG_NAME " <dest Y> [<src Y>] [--verbose] [--help]\n"
    "See man page for more information.\n" );
}

void applyAbsoluteOnFrames( int argc, char* argv[] )
{
  pfs::DOMIO pfsio;

  float destY = 1.0f;
  float srcY = 1.0f;  

  bool verbose = false;

  static struct option cmdLineOptions[] = {
    { "help", no_argument, NULL, 'h' },
    { "verbose", no_argument, NULL, 'v' },
    { NULL, 0, NULL, 0 }
  };

  int optionIndex = 0;
  while( 1 ) {
    int c = getopt_long (argc, argv, "", cmdLineOptions, &optionIndex);
    if( c == -1 ) break;
    switch( c ) {
    case 'h':
      printHelp();
      throw QuietException();
    case 'v':
      verbose = true;
      break;
    case '?':
      throw QuietException();
    case ':':
      throw QuietException();
    }
  } 

  if( optind == argc )
    throw pfs::Exception( "Destination luminance level <dest Y> must be specified" );
  if( optind < (argc - 2) )
    throw pfs::Exception( "Too many arguments" );

  destY = strtof( argv[optind++], NULL );
  if( optind != argc )
    srcY = strtof( argv[optind++], NULL );
  
  VERBOSE_STR << "rescale luminance to: " << destY << std::endl;
  if( srcY != 1.0f )
    VERBOSE_STR << "from: " << srcY << std::endl;

  float multY = destY/srcY;
  
  while( true ) {
    pfs::Frame *frame = pfsio.readFrame( stdin );
    if( frame == NULL ) break; // No more frames

    const char *lumType = frame->getTags()->getString( "LUMINANCE" );
    if( lumType != NULL && !strcmp( lumType, "ABSOLUTE" ) ) {
      VERBOSE_STR << "luminance is already absolute, skipping frame.";
    } else {
    
      pfs::Channel *X, *Y, *Z;
      frame->getXYZChannels( X, Y, Z );
    
      if( X != NULL ) {           // Color, XYZ

        if( lumType != NULL && !strcmp( lumType, "DISPLAY" ) ) {
          VERBOSE_STR << "converting from display-referred to linear luminance.";
          pfs::transformColorSpace( pfs::CS_XYZ, X, Y, Z, pfs::CS_RGB, X, Y, Z );
          pfs::transformColorSpace( pfs::CS_SRGB, X, Y, Z, pfs::CS_XYZ, X, Y, Z );
        }
        multiplyArray( X, X, multY );
        multiplyArray( Y, Y, multY );
        multiplyArray( Z, Z, multY );
        
      } else if( (Y = frame->getChannel( "Y" )) != NULL ) {
        // Luminance only

        if( lumType != NULL && !strcmp( lumType, "DISPLAY" ) ) 
          throw pfs::Exception( PROG_NAME ": Cannot handle gray-level display-referred images." );
        
        multiplyArray( Y, Y, multY );

      } else
        throw pfs::Exception( "Missing color channels in the PFS stream" );

      frame->getTags()->setString("LUMINANCE", "ABSOLUTE");
    }    
    
    pfsio.writeFrame( frame, stdout );
    pfsio.freeFrame( frame );        
  }
}

int main( int argc, char* argv[] )
{
  try {
    applyAbsoluteOnFrames( argc, argv );
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
