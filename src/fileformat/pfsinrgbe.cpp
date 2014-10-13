/**
 * @brief Read files in Radiance's RGBE format
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
 * @author Rafal Mantiuk, <mantiuk@mpi-sb.mpg.de>
 *
 * $Id: pfsinrgbe.cpp,v 1.5 2014/06/17 21:57:08 rafm Exp $
 */

#include <config.h>

#include <cstdlib>

#include <iostream>

#include <getopt.h>
#include <pfs.h>

#include "rgbeio.h"

#define PROG_NAME "pfsinrgbe"

class QuietException 
{
};

void printHelp()
{
  std::cerr << PROG_NAME " [--linear] [--radiance] [--verbose] [--help]" << std::endl
            << "See man page for more information." << std::endl;
}


void readFrames( int argc, char* argv[] )
{
  pfs::DOMIO pfsio;

  bool verbose = false;
  bool opt_linear=false;
  bool radiance_compatibility = false;
  bool quiet = false;
  
  // Parse command line parameters
  static struct option cmdLineOptions[] = {
    { "help", no_argument, NULL, 'h' },
    { "verbose", no_argument, NULL, 'v' },
    { "linear", no_argument, NULL, 'l' },
	{ "radiance", no_argument, NULL, 'r' },
	{ "quiet", no_argument, NULL, 'q' },
	{ NULL, 0, NULL, 0 }
  };
  static const char optstring[] = "lhvrq";
    
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
	case 'r':
		radiance_compatibility = true;
		break;
	case 'q':
		quiet = true;
		break;
	case 'l':
      std::cerr << PROG_NAME << " warning: linearize option ignored for an HDR input!"
                << std::endl;
      break;
    case '?':
      throw QuietException();
    case ':':
      throw QuietException();
    }
  }

  if (!quiet && !radiance_compatibility)
	  std::cerr << PROG_NAME << " warning: starting from pfstools 1.9.0, .hdr files are read without correcting for WHITE_EFFICIENCY,"
	  " which makes the absolute values incompatible with Radiance and previos versions of pfstools but compatible with most software."
	  " Use --radiance option to retain compatibility with Radiance and earlier versions of pfstools.  Use --quiet to suppress this message."
	  " Check the manual pages for pfsinrgbe for details." << std::endl;

  while( true ) {
    pfs::FrameFile ff = it.getNextFrameFile();
    if( ff.fh == NULL ) break; // No more frames

    RGBEReader reader( ff.fh, radiance_compatibility );

    VERBOSE_STR << "reading file (linear) '" << ff.fileName << "'" << std::endl;

    pfs::Frame *frame = pfsio.createFrame( reader.getWidth(),
					 reader.getHeight() );
    pfs::Channel *X, *Y, *Z;
    frame->createXYZChannels( X, Y, Z );

    //Store RGB data temporarily in XYZ channels
    reader.readImage( X, Y, Z );
    pfs::transformColorSpace( pfs::CS_RGB, X, Y, Z, pfs::CS_XYZ, X, Y, Z );

    frame->getTags()->setString("LUMINANCE", "RELATIVE");
    const char *fileNameTag = strcmp( "-", ff.fileName )==0 ? "stdin" : ff.fileName;
    frame->getTags()->setString( "FILE_NAME", fileNameTag );

    pfsio.writeFrame( frame, stdout );
    pfsio.freeFrame( frame );

    it.closeFrameFile( ff );
  }
}


int main( int argc, char* argv[] )
{
  try {
    readFrames( argc, argv );
  }
  catch( pfs::Exception ex ) {
    fprintf( stderr, PROG_NAME " error: %s\n", ex.getMessage() );
    return EXIT_FAILURE;
  }        
  catch( QuietException ) {
    return EXIT_FAILURE;
  }        
  return EXIT_SUCCESS;
}
