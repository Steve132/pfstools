/**
 * @brief Read frame from pfs stream in GNU Octave
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
 * $Id: pfsget.cpp,v 1.1 2005/06/15 13:36:54 rafm Exp $
 */

#include <octave/oct.h>
#include <octave/oct-stream.h>
#include <octave/ov-struct.h>
#include <octave/Cell.h>

//#include "../../config.h"				// conflicts with config.h from octave distribution

#include <string>
#include <pfs.h>

#define SCRIPT_NAME "pfsget"

static const char *helpString =
#include "pfsget_help.h"

DEFUN_DLD( pfsget, args, , helpString)
{
  octave_value_list retval;
	
  int nargin = args.length();

  if( nargin != 1 || !args(0).is_map() )
  {
    error( SCRIPT_NAME ": Improper usage!");
    return retval;
  }

  Octave_map pfsStream = args(0).map_value();

  Octave_map::const_iterator itFH = pfsStream.seek( "FH" );
  if( itFH == pfsStream.end() ||
    !pfsStream.contents( itFH )(0).is_real_scalar() )
  {
    error( SCRIPT_NAME ": FH field missing in the structure or it has wrong type");
    return retval;
  }  
  FILE *fh = (FILE*)((long)(pfsStream.contents( itFH )(0).double_value()));

  Octave_map::const_iterator itMode = pfsStream.seek( "MODE" );
  if( itMode == pfsStream.end() || !pfsStream.contents( itMode )(0).is_string() )
  {
    error( SCRIPT_NAME ": MODE field missing in the structure or it has wrong type");
    return retval;
  }
  if( pfsStream.contents( itMode )(0).string_value() != "R" ) {
    error( SCRIPT_NAME ": Can not read from stream open for writing." );
    return retval;    
  }

  pfsStream.del( "channels" );
  pfsStream.del( "tags" );
  pfsStream.del( "channelTags" );
  
  try {
    pfs::DOMIO ctx;
    pfs::Frame *frame = ctx.readFrame( fh );
    if( frame == NULL ) {         // No more frames
                                  
      pfsStream.assign( "EOF", octave_value(true) );
      
    } else {                    // Not EOF

      // Set width and height
      {
        pfsStream.assign( "columns", octave_value(frame->getWidth()) );
        pfsStream.assign( "rows", octave_value(frame->getHeight()) );
      }
    
      // Add channels as matrices to pfs stream struct
      {
        Octave_map channels;
        
        pfs::ChannelIteratorPtr cit( frame->getChannelIterator() );
        while( cit->hasNext() ) {
          pfs::Channel *ch = cit->getNext();

          Matrix mat( ch->getRows(), ch->getCols() );
          int index = 0;
          for( int r = 0; r < ch->getRows(); r++ ) // Copy channel data to Octave matrix
            for( int c = 0; c < ch->getCols(); c++ ) {
              mat(r,c) = (*ch)(index++);
            }      
      
          channels.assign( ch->getName(), mat );
        }
        pfsStream.assign( "channels", channels );
      }

      //Add tags
      {
        Octave_map tags;
        
        pfs::TagIteratorPtr it( frame->getTags()->getIterator() );        
        while( it->hasNext() ) {
          const char *tagName = it->getNext();         
          tags.assign( tagName, octave_value( frame->getTags()->getString( tagName )) );
        }
        pfsStream.assign( "tags", tags );
        
        Octave_map channelTagList;

        //Copy all channel tags
        pfs::ChannelIteratorPtr cit( frame->getChannelIterator() );
        while( cit->hasNext() ) {
          pfs::Channel *ch = cit->getNext();

          Octave_map channelTags;
          
          pfs::TagIteratorPtr tit( ch->getTags()->getIterator() );        
          while( tit->hasNext() ) {
            const char *tagName = tit->getNext();
            channelTags.assign( tagName, octave_value(ch->getTags()->getString( tagName )) );
          }
          channelTagList.assign( ch->getName(), channelTags );
        }        
        pfsStream.assign( "channelTags", channelTagList );
        
      }
      
    
    }
    ctx.freeFrame( frame );

  }
  catch( pfs::Exception ex )
  {
    char error_message[100];
    sprintf( error_message, "%s: %s", SCRIPT_NAME, ex.getMessage() );
    error( error_message );      
  }
//    if( fh != stdin ) fclose( fh );    
    
  retval.append(pfsStream);
    
  return retval;
}
