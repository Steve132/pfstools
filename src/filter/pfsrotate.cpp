/**
 * @brief Resize images in PFS stream
 * 
 * This file is a part of PFSTOOLS package.
 * ---------------------------------------------------------------------- 
 * Copyright (C) 2003,2004 Rafal Mantiuk and Grzegorz Krawczyk,
 *  Alexander Efremov
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
 * @author Alexander Efremov, <aefremov@mpi-sb.mpg.de>
 *
 * $Id: pfsrotate.cpp,v 1.1 2005/06/15 13:36:54 rafm Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>

#include <pfs.h>

#include <sstream>

#define PROG_NAME "pfsrotate"

class QuietException 
{
};

void rotateArray( const pfs::Array2D *in, pfs::Array2D *out, bool clockwise );

void printHelp()
{
  fprintf( stderr, PROG_NAME " [-r] [--help]\n"
    "See man page for more information.\n" );
}

void rotateFrames( int argc, char* argv[] )
{
  pfs::DOMIO pfsio;

  static struct option cmdLineOptions[] = {
    { "help", no_argument, NULL, 'h' },
    { "r", no_argument, NULL, 'r' },
    { NULL, 0, NULL, 0 }
  };

  bool clockwise = true;
  int xSize = -1;
  int ySize = -1;
  
  int optionIndex = 0;
  while( 1 ) {
    int c = getopt_long (argc, argv, "r", cmdLineOptions, &optionIndex);
    if( c == -1 ) break;
    switch( c ) {
    case 'h':
      printHelp();
      throw QuietException();
    case 'r':
      clockwise = false;
      break;
    case '?':
      throw QuietException();
    case ':':
      throw QuietException();
    }
  }

  bool firstFrame = true;
  pfs::Frame *resizedFrame = NULL;
  
  while( true ) {
    pfs::Frame *frame = pfsio.readFrame( stdin );
    if( frame == NULL ) break; // No more frames

    pfs::Channel *X, *Y, *Z;
    frame->getXYZChannels( X, Y, Z );

    pfs::Channel *dX, *dY, *dZ;
    
    if( firstFrame ) {
      xSize = frame->getHeight();
      ySize = frame->getWidth();
      resizedFrame = pfsio.createFrame( xSize, ySize );
      firstFrame = false;
    }

    pfs::ChannelIterator *it = frame->getChannels();
    while( it->hasNext() ) {
      pfs::Channel *originalCh = it->getNext();
      pfs::Channel *newCh = resizedFrame->createChannel( originalCh->getName() );

      rotateArray( originalCh, newCh, clockwise );
    }

    pfs::copyTags( frame, resizedFrame );
    pfsio.writeFrame( resizedFrame, stdout );
    pfsio.freeFrame( frame );        
  }
  pfsio.freeFrame( resizedFrame );
}

void rotateArray(const pfs::Array2D *in, pfs::Array2D *out, bool clockwise)
{
  int outRows = out->getRows();
  int outCols = out->getCols();
  
  for( int i=0; i<outCols; i++ )
    for( int j=0; j<outRows; j++ )
      if( clockwise )
        (*out)( i, j ) = (*in)( j, outCols - i - 1 );          
      else
        (*out)( i, j ) = (*in)( outRows - j - 1, i );
}

int main( int argc, char* argv[] )
{
  try {
    rotateFrames( argc, argv );
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
