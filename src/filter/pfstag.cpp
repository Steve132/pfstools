/**
 * @brief Add/Remove tags to frames in pfs stream
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
 * $Id: pfstag.cpp,v 1.1 2005/06/15 13:36:54 rafm Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>

#include <pfs.h>

#include <iostream>
#include <list>
#include <string>

using namespace std;

#define PROG_NAME "pfstag"

class QuietException 
{
};


void printHelp()
{
  fprintf( stderr, PROG_NAME " [--set \"[channel:]name=value\"] [--remove \"[channel:]name]\" [--verbose] [--help]\n"
    "See man page for more information.\n" );
}

struct TagOperation
{
  bool remove;
  string name, value;
  string channel;
};

typedef list<TagOperation> ListOfTags;

TagOperation parseTagOperation( const char *tag, bool remove )
{
  TagOperation tagop;
  const char *equalSign = strstr( tag, "=" );
  const char *colonSign = strstr( tag, ":" );
  if( remove ) equalSign = tag + strlen( tag );
  if( colonSign > equalSign || colonSign == tag ) colonSign = NULL; // ":" only before "="
  
  if( !remove && (equalSign == NULL ) ) // = sign missing
    throw pfs::Exception( "Tag must contain '=' sign" );

  if( (colonSign == NULL && equalSign == tag) ||
    (colonSign != NULL && (equalSign-colonSign) <= 1 ) )
    throw pfs::Exception( "Missing tag name" );
  
  tagop.name = colonSign == NULL ? string( tag, (equalSign-tag) ) :
    string( colonSign+1, (equalSign-colonSign-1) );
  if( !remove ) tagop.value = string( equalSign+1 );
  // No channel -> frame tag
  if( colonSign != NULL )       
    tagop.channel = string( tag, (colonSign-tag) );

  tagop.remove = remove;
  
  return tagop;
}

void setTagsOnFrames( int argc, char* argv[] )
{
  pfs::DOMIO pfsio;

  ListOfTags setTags;
  
  bool verbose = false;

  static struct option cmdLineOptions[] = {
    { "help", no_argument, NULL, 'h' },
    { "verbose", no_argument, NULL, 'v' },
    { "add", required_argument, NULL, 's' },
    { "set", required_argument, NULL, 's' },
    { "remove", required_argument, NULL, 'r' },
    { NULL, 0, NULL, 0 }
  };

  int optionIndex = 0;
  while( 1 ) {
    int c = getopt_long (argc, argv, "m:g:s:r:", cmdLineOptions, &optionIndex);
    if( c == -1 ) break;
    switch( c ) {
    case 'h':
      printHelp();
      throw QuietException();
    case 'v':
      verbose = true;
      break;
    case 's':
      setTags.push_back( parseTagOperation( optarg, false ) );
      break;
    case 'r':
      setTags.push_back( parseTagOperation( optarg, true ) );
      break;
    case '?':
      throw QuietException();
    case ':':
      throw QuietException();
    }
  } 

  if( verbose ) {
    ListOfTags::iterator it;
    for( it = setTags.begin(); it != setTags.end(); it++ ) {
      TagOperation &tagop = *it;
      if( tagop.remove )       
        cerr << PROG_NAME ": remove tag '" << tagop.name << "'\n";
      else
        cerr << PROG_NAME ": set tag '" << tagop.name << "' to '" << tagop.value << "'\n";
    }
    
  }
  
  
  while( true ) {
    pfs::Frame *frame = pfsio.readFrame( stdin );
    if( frame == NULL ) break; // No more frames

    
    ListOfTags::iterator it;
    for( it = setTags.begin(); it != setTags.end(); it++ ) {
      TagOperation &tagop = *it;

      pfs::TagContainer *tags;
      if( tagop.channel.empty() ) tags = frame->getTags();
      else {
        pfs::Channel *channel = frame->getChannel( tagop.channel.c_str() );
        if( channel == NULL ) throw pfs::Exception( "Channel not found" );
        tags = channel->getTags();
      }
      

      if( tagop.remove )
        tags->removeTag( tagop.name.c_str() );
      else
        tags->setString( tagop.name.c_str(), tagop.value.c_str() );
    }    
    
    pfsio.writeFrame( frame, stdout );
    pfsio.freeFrame( frame );        
  }
}


int main( int argc, char* argv[] )
{
  try {
    setTagsOnFrames( argc, argv );
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
