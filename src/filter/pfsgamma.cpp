/**
 * @brief Apply gamma correction the the pfs stream
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
 * $Id: pfsgamma.cpp,v 1.4 2013/01/29 22:15:01 rafm Exp $
 */

#include <config.h>

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>

#include <pfs.h>

#define PROG_NAME "pfsgamma"

class QuietException 
{
};


void applyGamma( pfs::Array2D *array, const float exponent, const float multiplier );

void printHelp()
{
  fprintf( stderr, PROG_NAME " [--gamma <val> | --inverse-gamma <val>] [--mul <val>] [--verbose] [--help]\n"
    "See man page for more information.\n" );
}

void retimeFrames( int argc, char* argv[] )
{
  pfs::DOMIO pfsio;

  float gamma = 1.0f;
  bool opt_setgamma = false;
  float multiplier = 1;

  bool verbose = false;

  static struct option cmdLineOptions[] = {
    { "help", no_argument, NULL, 'h' },
    { "verbose", no_argument, NULL, 'v' },
    { "gamma", required_argument, NULL, 'g' },
    { "inverse-gamma", required_argument, NULL, 'i' },
    { "mul", required_argument, NULL, 'm' },
    { NULL, 0, NULL, 0 }
  };

  int optionIndex = 0;
  while( 1 ) {
    int c = getopt_long (argc, argv, "m:g:i:hv", cmdLineOptions, &optionIndex);
    if( c == -1 ) break;
    switch( c ) {
    case 'h':
      printHelp();
      throw QuietException();
    case 'v':
      verbose = true;
      break;
    case 'g':
      gamma = (float)strtod( optarg, NULL );
      opt_setgamma = true;
      break;
    case 'i':
      gamma = 1/(float)strtod( optarg, NULL );
      opt_setgamma = true;
      break;
    case 'm':
      multiplier = (float)strtod( optarg, NULL );
      break;
    case '?':
      throw QuietException();
    case ':':
      throw QuietException();
    }
  } 

  VERBOSE_STR << "multiplier: " << multiplier << " gamma: " << gamma << std::endl;
  if( gamma > 1.0f )
    VERBOSE_STR << "luminance content will be changed to 'display'" << std::endl; 

  bool first_frame = true;
  while( true ) {
    pfs::Frame *frame = pfsio.readFrame( stdin );
    if( frame == NULL ) break; // No more frames


    if( first_frame ) {
      const char *lum_type = frame->getTags()->getString("LUMINANCE");
      if( lum_type ) {
        if( !strcmp( lum_type, "DISPLAY" ) && gamma > 1.0f )
          std::cerr << PROG_NAME " warning: applying gamma correction to a display referred image" << std::endl;
        if( !strcmp( lum_type, "RELATIVE" ) && gamma < 1.0f )
          std::cerr << PROG_NAME " warning: applying inverse gamma correction to a linear luminance or radiance image" << std::endl;
        if( !strcmp( lum_type, "ABSOLUTE" ) && multiplier == 1 )
          std::cerr << PROG_NAME " warning: an image should be normalized to 0-1 before applying gamma correction" << std::endl;
      }
    }
    
    pfs::Channel *X, *Y, *Z;
    frame->getXYZChannels( X, Y, Z );

    if( X != NULL ) {           // Color, XYZ
      
      pfs::transformColorSpace( pfs::CS_XYZ, X, Y, Z, pfs::CS_RGB, X, Y, Z );
      // At this point (X,Y,Z) = (R,G,B)
        
      applyGamma( X, 1/gamma, multiplier );
      applyGamma( Y, 1/gamma, multiplier );
      applyGamma( Z, 1/gamma, multiplier );

      pfs::transformColorSpace( pfs::CS_RGB, X, Y, Z, pfs::CS_XYZ, X, Y, Z );
      // At this point (X,Y,Z) = (X,Y,Z)
      
    } else if( (Y = frame->getChannel( "Y" )) != NULL ) {
      // Luminance only

      applyGamma( Y, 1/gamma, multiplier );
      
    } else
      throw pfs::Exception( "Missing X, Y, Z channels in the PFS stream" );

    if( opt_setgamma && gamma > 1.0f )
      frame->getTags()->setString("LUMINANCE", "DISPLAY");
    else if( opt_setgamma && gamma < 1.0f )
      frame->getTags()->setString("LUMINANCE", "RELATIVE");

    first_frame = false;
    
    pfsio.writeFrame( frame, stdout );
    pfsio.freeFrame( frame );        
  }
}


void applyGamma( pfs::Array2D *array, const float exponent, const float multiplier )
{
  int imgSize = array->getRows()*array->getCols();
  for( int index = 0; index < imgSize ; index++ ) {    
    float &v = (*array)(index);
    if( v < 0 ) v = 0;
    v = powf( v*multiplier, exponent );
  }    

}


int main( int argc, char* argv[] )
{
  try {
    retimeFrames( argc, argv );
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
