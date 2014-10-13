/**
 * @brief Write files in TIFF format
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
 * @author Grzegorz Krawczyk, <krawczyk@mpi-sb.mpg.de>
 *
 * $Id: pfsouttiff.cpp,v 1.3 2008/10/22 22:20:29 rafm Exp $
 */

#include <config.h>

#include <cstdlib>

#include <iostream>

#include <getopt.h>
#include <pfs.h>

#include "hdrtiffio.h"


#define PROG_NAME "pfsouttiff"


class QuietException 
{
};

void printHelp()
{
  std::cerr << PROG_NAME " [--linear] [--verbose] [--help]" << std::endl
            << "See man page for more information." << std::endl;
}


void writeFrames( int argc, char* argv[] )
{
  pfs::DOMIO pfsio;

  bool verbose = false;
  bool opt_srgb=false;
  
  // Parse command line parameters
  static struct option cmdLineOptions[] = {
    { "help", no_argument, NULL, 'h' },
    { "verbose", no_argument, NULL, 'v' },
    { "srgb", no_argument, NULL, 's' },
    { NULL, 0, NULL, 0 }
  };
  static const char optstring[] = "shv";
    
  pfs::FrameFileIterator it( argc, argv, "wb", NULL, stdout,
    optstring, cmdLineOptions );
    
  int optionIndex = 0;
  while( 1 ) {
    int c = getopt_long (argc, argv, optstring, cmdLineOptions, &optionIndex);
    if( c == -1 ) break;
    switch( c ) {
    case 'h':
      printHelp();
      throw QuietException();
    case 'v':
      verbose = true;
      break;
    case 's':
      opt_srgb = true;
      break;
    case '?':
      throw QuietException();
    case ':':
      throw QuietException();
    }
  }
   
  while( true ) {
    pfs::Frame *frame = pfsio.readFrame( stdin );
    if( frame == NULL ) {
      break;
    }

    pfs::FrameFile ff = it.getNextFrameFile();
    if( ff.fh == NULL ) {
      pfsio.freeFrame( frame );
      break; // No more frames
    }    

    HDRTiffWriter writer( ff.fh );


    pfs::Channel *X, *Y, *Z;
    frame->getXYZChannels( X, Y, Z );
    if( X == NULL ) {       // No color
      throw pfs::Exception( "Missing X, Y, Z channels in the PFS stream" );
    }

    const char* luminanceTag = frame->getTags()->getString("LUMINANCE");
    if( luminanceTag!=NULL && strcmp(luminanceTag,"ABSOLUTE")==0 )
        std::cerr << PROG_NAME << " warning: This file format cannot store absolute luminance values\n";    
    if( opt_srgb )
    {
      if( luminanceTag!=NULL && strcmp(luminanceTag,"DISPLAY")==0 )
        std::cerr << PROG_NAME << " warning: This image seems to be display referred thus there is no need for applying the sRGB non-linearity\n";
      pfs::transformColorSpace( pfs::CS_XYZ, X, Y, Z, pfs::CS_SRGB, X, Y, Z );        
      VERBOSE_STR << "writing file (sRGB corrected) '" << ff.fileName << "'" << std::endl;
    }
    else
    {      
      pfs::transformColorSpace( pfs::CS_XYZ, X, Y, Z, pfs::CS_RGB, X, Y, Z );        
      VERBOSE_STR << "writing file (linear) '" << ff.fileName << "'" << std::endl;
    }        
    
    writer.writeImage( X, Y, Z );

    pfsio.freeFrame( frame );

    it.closeFrameFile( ff );
  }
}


int main( int argc, char* argv[] )
{
  try {
    writeFrames( argc, argv );
  }
  catch( pfs::Exception ex ) {
    std::cerr << PROG_NAME << " error: " << ex.getMessage() << std::endl;
    return EXIT_FAILURE;
  }        
  catch( QuietException  ex ) {
    return EXIT_FAILURE;
  }        
  return EXIT_SUCCESS;
}
