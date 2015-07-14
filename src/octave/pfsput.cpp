/**
 * @brief Write frame to pfs stream in GNU Octave
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
 * $Id: pfsput.cpp,v 1.2 2006/03/22 10:03:09 rafm Exp $
 */

#include <octave/oct.h>
#include <octave/oct-stream.h>
#include <octave/ov-struct.h>
#include <octave/Cell.h>

//#include "../../config.h"				// conflicts with config.h from octave distribution

#include <string>
#include <pfs.h>

#define SCRIPT_NAME "pfsput"

static const char *helpString =
#include "pfsput_help.h"

DEFUN_DLD( pfsput, args, , helpString)
{
  octave_value_list retval;
	
  int nargin = args.length();

  if( nargin != 1 || !args(0).is_map() )
  {
    error( SCRIPT_NAME ": Improper usage!");
    return retval;
  }
  
  octave_scalar_map pfsStream = args(0).map_value();

  octave_scalar_map::const_iterator itFH = pfsStream.seek( "FH" );
  if( itFH == pfsStream.end() ||
    !pfsStream.contents( itFH )(0).is_real_scalar() )
  {
    error( SCRIPT_NAME ": FH field missing in the structure or it has wrong type");
    return retval;
  }  
  FILE *fh = (FILE*)((long)(pfsStream.contents( itFH )(0).double_value()));

  // Check mode
  {                             
    octave_scalar_map::const_iterator itMode = pfsStream.seek( "MODE" );
    if( itMode == pfsStream.end() || !pfsStream.contents( itMode )(0).is_string() )
    {
      error( SCRIPT_NAME ": MODE field missing in the structure or it has wrong type");
      return retval;
    }
    if( pfsStream.contents( itMode )(0).string_value() != "W" ) {
      error( SCRIPT_NAME ": Can not write to the stream that is open for reading." );
      return retval;    
    }
  }

  // Get width & height
  int width, height;
  {                             
    octave_scalar_map::const_iterator itCols = pfsStream.seek( "columns" );
    octave_scalar_map::const_iterator itRows = pfsStream.seek( "rows" );
    if( itCols == pfsStream.end() || itRows == pfsStream.end() ||
      !pfsStream.contents( itCols )(0).is_real_scalar() ||
      !pfsStream.contents( itRows )(0).is_real_scalar() )
    {
      error( SCRIPT_NAME ": 'rows' and 'columns' fields missing in the structure or it have wrong type");
      return retval;
    }
    width = (int)pfsStream.contents( itCols )(0).double_value();
    height = (int)pfsStream.contents( itRows )(0).double_value(); 
  }

  // Get channels
  octave_scalar_map channels;
  {
    octave_scalar_map::const_iterator itChannels = pfsStream.seek( "channels" );
    if( itChannels == pfsStream.end() ||
      !pfsStream.contents( itChannels )(0).is_map() )
    {
      error( SCRIPT_NAME ": 'channels' field missing in the structure or it has wrong type");
      return retval;
    }
    channels = pfsStream.contents( itChannels )(0).map_value();
  }  
  
  try {
    pfs::DOMIO ctx;
    pfs::Frame *frame = ctx.createFrame( width, height );

    // For each channel in the 'channels' map
    for( octave_scalar_map::iterator itCh = channels.begin(); itCh != channels.end(); itCh++ ) {
      std::string channelName = channels.key(itCh);

      if( !channels.contents( itCh )(0).is_real_matrix() ) {
        throw pfs::Exception( "all channels must be given as real matrices" );
      }
      
      Matrix channelData = channels.contents( itCh )(0).matrix_value();
      if( channelData.rows() != height || channelData.columns() != width ) {
        throw pfs::Exception( "size of the channel must be the same as given in pfsopen" );
      }
      
      pfs::Channel *pfsChannel = frame->createChannel( channelName.c_str() );

      // Copy matrix to pfs::Channel
      int index = 0;
      for( int r = 0; r < pfsChannel->getRows(); r++ ) 
        for( int c = 0; c < pfsChannel->getCols(); c++ ) {
          (*pfsChannel)(index++) = channelData(r,c);
        }
    }

    // Copy frame tags
    {
      octave_scalar_map::const_iterator itTags = pfsStream.seek( "tags" );
      if( itTags != pfsStream.end() ) {
        if( !pfsStream.contents( itTags )(0).is_map() )
        {
          throw pfs::Exception( "'tags' field must be a structure" );  
        }
        
        octave_scalar_map_map tags = pfsStream.contents( itTags )(0).map_value();
        for( octave_scalar_map::iterator itTag = tags.begin(); itTag != tags.end(); itTag++ ) {
          std::string tagName = tags.key(itTag);

          if( !tags.contents( itTag )(0).is_string() ) 
            throw pfs::Exception( "all tags must be given as strings" );
          std::string tagValue = tags.contents( itTag )(0).string_value();
          frame->getTags()->setString( tagName.c_str(), tagValue.c_str() );
        }
      }  
    }

    // Copy channel tags
    {
      octave_scalar_map::const_iterator itChTags = pfsStream.seek( "channelTags" );
      if( itChTags != pfsStream.end() ) {
        if( !pfsStream.contents( itChTags )(0).is_map() )
        {
          throw pfs::Exception( "'channelTags' field must be a structure" );  
        }
        octave_scalar_map tagChannels = pfsStream.contents( itChTags )(0).map_value();
        for( octave_scalar_map::iterator itCh = tagChannels.begin(); itCh != tagChannels.end(); itCh++ ) {
          std::string channelName = tagChannels.key(itCh);
          if( !tagChannels.contents( itCh )(0).is_map() ) {
            throw pfs::Exception( "each channelTags file must be a structure" );  
          }
          pfs::Channel *pfsChannel = frame->getChannel( channelName.c_str() );
          if( pfsChannel == NULL ) {
            throw pfs::Exception( "can not set channel tag if channel is missing" );
          }
          
          octave_scalar_map tags = tagChannels.contents( itCh )(0).map_value();
          for( octave_scalar_map::iterator itTag = tags.begin(); itTag != tags.end(); itTag++ ) {
            std::string tagName = tags.key(itTag);
            if( !tags.contents( itTag )(0).is_string() ) 
              throw pfs::Exception( "all channel tags must be given as strings" );
            std::string tagValue = tags.contents( itTag )(0).string_value();
            pfsChannel->getTags()->setString( tagName.c_str(), tagValue.c_str() );
          }
        }
      }
      
    }

    ctx.writeFrame( frame, fh );
    ctx.freeFrame( frame );
  }
  catch( pfs::Exception ex )
  {
    char error_message[100];
    sprintf( error_message, "%s: %s", SCRIPT_NAME, ex.getMessage() );
    error( error_message );      
  }    
    
  return retval;
}
