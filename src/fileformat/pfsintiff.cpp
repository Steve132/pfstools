/**
 * @brief Read files in TIFF format
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
 * $Id: pfsintiff.cpp,v 1.3 2008/01/01 13:01:21 rafm Exp $
 */

#include <config.h>

#include <cstdlib>

#include <iostream>

#include <getopt.h>
#include <pfs.h>

#include "hdrtiffio.h"

#define PROG_NAME "pfsintiff"

class QuietException 
{
};

void printHelp()
{
  std::cerr << PROG_NAME " [--linear] [--expmode] [--verbose] [--help]" << std::endl
            << "See man page for more information." << std::endl;
}



void readFrames( int argc, char* argv[] )
{
  pfs::DOMIO pfsio;

  bool verbose = false;
  bool opt_linear=false;
  bool opt_exponential_mode = false;
  
  // Parse command line parameters
  static struct option cmdLineOptions[] = {
    { "help", no_argument, NULL, 'h' },
    { "verbose", no_argument, NULL, 'v' },
    { "linear", no_argument, NULL, 'l' },
    { "expmode", no_argument, NULL, 'e' },
    { NULL, 0, NULL, 0 }
  };
  static const char optstring[] = "elhv";
    
  pfs::FrameFileIterator it( argc, argv, "rb", NULL, stdin,
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
    case 'l':
      opt_linear = true;
      break;
    case 'e':
      opt_exponential_mode = true;
      break;
    case '?':
      throw QuietException();
    case ':':
      throw QuietException();
    }
  }

  VERBOSE_STR << "linearize input image: " << (opt_linear ? "yes" : "no") << std::endl;
  VERBOSE_STR << "exponential mode (Lars3 HDR camera): "
              << (opt_exponential_mode ? "yes" : "no") << std::endl;  
  
  while( true ) {
    pfs::FrameFile ff = it.getNextFrameFile();
    if( ff.fh == NULL ) break; // No more frames

    it.closeFrameFile( ff );
    HDRTiffReader reader( ff.fileName );

    if( opt_exponential_mode )
      reader.setExponentialMode();

    VERBOSE_STR << "reading file (" << reader.getFormatString() << ") '"
                << ff.fileName << "'" << std::endl;

    pfs::Frame *frame = pfsio.createFrame( reader.getWidth(),
			reader.getHeight() );
    pfs::Channel *X, *Y, *Z;
    frame->createXYZChannels( X, Y, Z );

    //Store RGB data temporarily in XYZ channels
    reader.readImage( X, Y, Z );
    if( opt_linear && !reader.isRelative() )
    {
      pfs::transformColorSpace( pfs::CS_SRGB, X, Y, Z, pfs::CS_XYZ, X, Y, Z );
      frame->getTags()->setString("LUMINANCE", "RELATIVE");
    }
    else
    {
      if( !reader.isColorspaceXYZ() )
        pfs::transformColorSpace( pfs::CS_RGB, X, Y, Z, pfs::CS_XYZ, X, Y, Z );
      
      if( reader.isRelative() )
        frame->getTags()->setString("LUMINANCE", "RELATIVE");
      else
        frame->getTags()->setString("LUMINANCE", "DISPLAY");
    }

    const char *fileNameTag = strcmp( "-", ff.fileName )==0 ? "stdin" : ff.fileName;
    frame->getTags()->setString( "FILE_NAME", fileNameTag );
    
    pfsio.writeFrame( frame, stdout );
    pfsio.freeFrame( frame );
  }
}


int main( int argc, char* argv[] )
{
  try {
    readFrames( argc, argv );
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
