/**
 * @brief Write files in PFM format
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
 * The format description was based on:
 * http://netpbm.sourceforge.net/doc/pfm.html
 *
 * however the order of the lines in pfm file seems to be reversed -
 * from bottom to top
 *
 * $Id: pfsoutpfm.cpp,v 1.1 2005/06/15 13:36:54 rafm Exp $
 */

#include <config.h>

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <pfs.h>
#include <getopt.h>

#define PROG_NAME "pfsoutpfm"

#define PFMEOL "\x0a"

class QuietException 
{
};

#define min(x,y) ( (x)<(y) ? (x) : (y) )

void printHelp()
{
  fprintf( stderr, PROG_NAME " [--verbose] [--help]\n"
    "See man page for more information.\n" );
}

void writePFMFileColor( FILE *fh, int width, int height,
  float *R, float *G, float *B )
{
  // Write header
  fprintf( fh, "PF" PFMEOL "%d %d" PFMEOL "-1" PFMEOL, width, height );
  
  const int lineSize = width*3;
  float *line = new float[lineSize];
  
  for( int l = 0; l < height; l++ ) {
    for( int x = 0; x < width; x++ ) {
      const int lineOffset = l*width;
      line[x*3+0] = R[lineOffset+x];
      line[x*3+1] = G[lineOffset+x];
      line[x*3+2] = B[lineOffset+x];
    }
    int written = fwrite( line, sizeof( float ), lineSize, fh );
    if( written != lineSize )
      throw new pfs::Exception( "Unable to write data" );
  }
  delete[] line;  
}

void writePFMFileGrayscale( FILE *fh, int width, int height, float *Y )
{
  // Write header
  fprintf( fh, "Pf" PFMEOL "%d %d" PFMEOL "-1" PFMEOL, width, height );
  
  const int lineSize = width;
  float *line = new float[lineSize];
  
  for( int l = 0; l < height; l++ ) {
    for( int x = 0; x < width; x++ ) {
      const int lineOffset = l*width;
      line[x] = Y[lineOffset+x];
    }
    int written = fwrite( line, sizeof( float ), lineSize, fh );
    if( written != lineSize )
      throw new pfs::Exception( "Unable to write data" );
  }
  delete[] line;  
}

void writeFrames( int argc, char* argv[] )
{
  bool verbose = false;

  // Parse command line parameters
  static struct option cmdLineOptions[] = {
    { "help", no_argument, NULL, 'h' },
    { "verbose", no_argument, NULL, 'v' },
    { "linear", no_argument, NULL, 'l' },
    { NULL, 0, NULL, 0 }
  };
  static const char optstring[] = "hlv";

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
    case 'l':
      std::cerr << PROG_NAME << " warning: linearize option ignored for an HDR output!"
                << std::endl;
      break;
    case '?':
      throw QuietException();
    case ':':
      throw QuietException();
    }
  }
 
  pfs::DOMIO pfsio;
 
  bool firstFrame = true;
 
  while( true ) {
    pfs::Frame *frame = pfsio.readFrame( stdin );
    if( frame == NULL ) {
      break; // No more frames
    }

    const char* luminanceTag = frame->getTags()->getString("LUMINANCE");
    if( luminanceTag!=NULL && 0==strcmp(luminanceTag,"DISPLAY") )
    {
      static bool show_once = false;
      if( !show_once )
      {
        std::cerr << PROG_NAME << " warning: "
                  << "display profiled content written to an HDR output" << std::endl;
        show_once=true;
      }
    }
    
    pfs::FrameFile ff = it.getNextFrameFile();
    if( ff.fh == NULL ) {
      pfsio.freeFrame( frame );
      break; // No more frames
    }
    
    VERBOSE_STR << "writing file (HDR) '" << ff.fileName << "'" << std::endl;

    pfs::Channel *X, *Y, *Z;
    frame->getXYZChannels( X, Y, Z );
    if( X != NULL ) {       // Has color
      pfs::Channel *R= NULL, *G = NULL, *B = NULL; // for clarity of the code
      R = X;
      G = Y;
      B = Z;
      pfs::transformColorSpace( pfs::CS_XYZ, X, Y, Z,
        pfs::CS_RGB, R, G, B );

      writePFMFileColor( ff.fh, frame->getWidth(), frame->getHeight(),
        R->getRawData(), G->getRawData(), B->getRawData() );
    } else {
      Y = frame->getChannel( "Y" );
      if( Y == NULL )
        throw pfs::Exception( "Can not find color or grayscale channels in the pfs stream" );
      writePFMFileGrayscale( ff.fh, frame->getWidth(), frame->getHeight(),
        Y->getRawData() );      
    }
    
    it.closeFrameFile( ff );
    pfsio.freeFrame( frame );
  }  
}


int main( int argc, char* argv[] )
{
  try {
    writeFrames( argc, argv );
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
