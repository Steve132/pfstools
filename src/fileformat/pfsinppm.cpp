/**
 * @brief Read files in PPM format
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
 * @author Grzegorz Krawczyk, <krawczyk@mpi-sb.mpg.de>
 *
 * $Id: pfsinppm.cpp,v 1.6 2009/05/13 17:26:14 rafm Exp $
 */

#include <config.h>

#include <cstdlib>

#include <iostream>

#include <getopt.h>
#include <pfs.h>

#include "ppmio.h"

#define PROG_NAME "pfsinppm"


class QuietException 
{
};

void printHelp()
{
  std::cerr << PROG_NAME " [--linear] [--absolute <max_lum>] [--verbose] [--help]" << std::endl
            << "See man page for more information." << std::endl;
}



void readFrames( int argc, char* argv[] )
{
  pfs::DOMIO pfsio;

  bool verbose = false;
  bool opt_linear=false;
  float absoluteMaxLum = 0;
  
  // Parse command line parameters
  static struct option cmdLineOptions[] = {
    { "help", no_argument, NULL, 'h' },
    { "verbose", no_argument, NULL, 'v' },
    { "linear", no_argument, NULL, 'l' },
    { "absolute", required_argument, NULL, 'a' },
    { NULL, 0, NULL, 0 }
  };
  static const char optstring[] = "lhva:";
    
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
    case 'a':
      absoluteMaxLum = (float)strtod( optarg, NULL );
      break;
    case '?':
      throw QuietException();
    case ':':
      throw QuietException();
    }
  }

  if( absoluteMaxLum != 0 && opt_linear )
    throw pfs::Exception( "'absolute' and 'linear' are conflicting options" );
  
  if( absoluteMaxLum < 0 )
    throw pfs::Exception( "maximum absolute luminance must be > 0" );

  VERBOSE_STR << "linearize input image: " << ((opt_linear || absoluteMaxLum!=0) ? "yes" : "no") << std::endl;
  if( absoluteMaxLum != 0 )
    VERBOSE_STR << "maximum absolute luminance: " << absoluteMaxLum << std::endl;

  bool eofP = false;
  while( !eofP ) {
    pfs::FrameFile ff = it.getNextFrameFile();
    if( ff.fh == NULL ) break; // No more frames

    PPMReader reader( PROG_NAME, ff.fh );

    VERBOSE_STR << "reading file '" << ff.fileName << "'" << std::endl;

    pfs::Frame *frame = pfsio.createFrame( reader.getWidth(),
      reader.getHeight() );
    pfs::Channel *X, *Y, *Z;
    frame->createXYZChannels( X, Y, Z );

    //Store sRGB data temporarily in XYZ channels        
    eofP = reader.readImage( X, Y, Z );
    if( opt_linear || absoluteMaxLum != 0 )
    {
      pfs::transformColorSpace( pfs::CS_SRGB, X, Y, Z, pfs::CS_XYZ, X, Y, Z );
      if( absoluteMaxLum != 0 ) {
        // Rescale to absolute luminance level
        const int pixCount = X->getWidth()*X->getHeight();
        for( int i = 0; i < pixCount; i++ ) {
          (*X)(i) *= absoluteMaxLum;
          (*Y)(i) *= absoluteMaxLum;
          (*Z)(i) *= absoluteMaxLum;
        }        
        frame->getTags()->setString("LUMINANCE", "ABSOLUTE");
      } else
        frame->getTags()->setString("LUMINANCE", "RELATIVE");
    }
    else
    {
      pfs::transformColorSpace( pfs::CS_RGB, X, Y, Z, pfs::CS_XYZ, X, Y, Z );
      frame->getTags()->setString("LUMINANCE", "DISPLAY");
    }

    // This is the luminance / luma perceived as reference white
    // Some tone-mappers may need this
    frame->getTags()->setString("WHITE_Y", "1");

    char strbuf[3];
    snprintf( strbuf, 3, "%d", reader.getBitDepth() );
    frame->getTags()->setString("BITDEPTH", strbuf );    

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
    std::cerr << PROG_NAME << " error: " << ex.getMessage() << std::endl;
    return EXIT_FAILURE;
  }        
  catch( QuietException  ex ) {
    return EXIT_FAILURE;
  }        
  return EXIT_SUCCESS;
}
