/**
 * @brief Extract selected channels from pfs stream
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
 * $Id: pfsextractchannels.cpp,v 1.2 2014/04/05 22:04:12 rafm Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>
#include <set>

#include <pfs.h>

#define PROG_NAME "pfsextractchannels"


class QuietException 
{
};

void printHelp()
{
  fprintf( stderr, PROG_NAME " channel_name [Channel_name...] [--help]\n"
    "See man page for more information.\n" );
}

static void errorCheck( bool condition, const char *string )
{
    if( !condition ) {
	fprintf( stderr, PROG_NAME " error: %s\n", string );
	throw QuietException();
    }
}

struct ltstr
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) < 0;
  }
};


void extractChannels( int argc, char* argv[] )
{
  pfs::DOMIO pfsio;

  static struct option cmdLineOptions[] = {
    { "help", no_argument, NULL, 'h' },
    { NULL, 0, NULL, 0 }
  };

  std::set<const char*, ltstr> keepChannels;

  int optionIndex = 0;
  while( 1 ) {
    int c = getopt_long (argc, argv, "h", cmdLineOptions, &optionIndex);
    if( c == -1 ) break;
    switch( c ) {
    case 'h':
      printHelp();
      throw QuietException();
    case '?':
      throw QuietException();
    case ':':
      throw QuietException();
    }
  }

  errorCheck( optind < argc, "At least one channel must be specified" );

  for( ;optind < argc; optind++ )
    keepChannels.insert( argv[optind] );

  
  while( true ) {
    pfs::Frame *frame = pfsio.readFrame( stdin );
    if( frame == NULL ) break; // No more frames

    {                           // Check if the listed channels exist
      std::set<const char*, ltstr>::iterator it;
      for( it = keepChannels.begin(); it != keepChannels.end(); it++ )
        if( frame->getChannel( *it ) == NULL ) {
          fprintf( stderr, PROG_NAME " error: Channel %s does not exist\n", *it );
          throw QuietException();
        }
      
    }
    
    {                           // Remoe not listed channels
      pfs::ChannelIterator *it = frame->getChannels();
      while( it->hasNext() ) {
        pfs::Channel *channel = it->getNext();
        if( keepChannels.find(channel->getName() ) == keepChannels.end() )
          frame->removeChannel( channel );
      }
    }    
    
    pfsio.writeFrame( frame, stdout );
    pfsio.freeFrame( frame );        
  }
}


int main( int argc, char* argv[] )
{
  try {
    extractChannels( argc, argv );
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
