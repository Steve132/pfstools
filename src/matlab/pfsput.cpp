/**
 * @brief Write frame to pfs stream in MATLAB
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
 * $Id: pfsput.cpp,v 1.6 2008/05/06 17:11:35 rafm Exp $
 */

#include "compatibility.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <list>

#include "mex.h"
#include "mex_utils.h"

#define SCRIPT_NAME "pfsput"
#define error mexErrMsgTxt

#include <pfs.h>


void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[])
{     

  /* Check for proper number of arguments. */
  if( nrhs != 1 || !mxIsStruct( prhs[0] ) ) 
    error(SCRIPT_NAME ": Improper usage!");
 
  const mxArray *pfs_stream = prhs[0];
  
  mxArray *f_fid = mxGetField( pfs_stream, 0, "FID" );
  if( f_fid == NULL || !is_mex_scalar( f_fid ) )
  {
    error( SCRIPT_NAME ": FH field missing in the structure or it has wrong type");
  }
  
  int fid = (int)get_mex_double( f_fid );

  // Check mode
  {                             
	mxArray *f_mode = mxGetField( pfs_stream, 0, "MODE" );
	if( f_mode == NULL ) {
      error( SCRIPT_NAME ": MODE field missing in the structure or it has wrong type");
    }
    if( strcmp( "W", get_mex_string( f_mode ) ) ) {
      error( SCRIPT_NAME ": Can not write to the stream that is open for reading." );
    }
  }

  // Get width & height
  int width, height;
  {                             
	mxArray *f_cols, *f_rows;
	f_cols = mxGetField( pfs_stream, 0, "columns" );
	f_rows = mxGetField( pfs_stream, 0, "rows" );

    if( !is_mex_scalar( f_cols ) || !is_mex_scalar( f_rows ) )
    {
      error( SCRIPT_NAME ": 'rows' and 'columns' fields missing in the structure or it have wrong type");
    }
    width = (int)get_mex_double( f_cols );
    height = (int)get_mex_double( f_rows );
  }

  // Get channels
  int ch_count;
  mxArray *f_channels;
  {
	f_channels = mxGetField( pfs_stream, 0, "channels" );
	if( f_channels == NULL || !mxIsStruct( f_channels ) ) 
      error( SCRIPT_NAME ": 'channels' field missing in the structure or it has wrong type");
    ch_count = mxGetNumberOfFields( f_channels );
  }  
  
  try {
    pfs::DOMIO ctx;
    pfs::Frame *frame = ctx.createFrame( width, height );

    // For each channel in the 'channels' map
    for( int ch = 0; ch < ch_count; ch++ ) {
      const char *channelName = mxGetFieldNameByNumber( f_channels, ch );

	  mxArray *f_data = mxGetFieldByNumber( f_channels, 0, ch );

      if( !(mxIsDouble( f_data ) || mxIsSingle( f_data )) || mxIsComplex( f_data ) ) {
        throw pfs::Exception( "all channels must be given as real matrices" );
      }
      
      if( mxGetM( f_data ) != height || mxGetN( f_data ) != width ) {
        throw pfs::Exception( "size of the channel must be the same as given in pfsopen" );
      }
      
      pfs::Channel *pfsChannel = frame->createChannel( channelName );

      // Copy matrix to pfs::Channel
      copy_mex_to_pfschannel( f_data, pfsChannel );
    }

    // Copy frame tags
    {
      mxArray *tags, *f_rows;
      tags = mxGetField( pfs_stream, 0, "tags" );
      if( tags != NULL ) {        
        if( !mxIsStruct( f_channels ) ) 
          error( SCRIPT_NAME ": 'tags' field has wrong type");
        int tag_count = mxGetNumberOfFields( tags );

        for( int t = 0; t < tag_count; t++ ) {
          const char *tag_name = mxGetFieldNameByNumber( tags, t );
          mxArray *tag_value = mxGetFieldByNumber( tags, 0, t );

//        printf( "tag: %s = %s\n", tag_name, get_mex_string( tag_value ) );
          frame->getTags()->setString( tag_name, get_mex_string( tag_value ) );
        
        }
      }
      
      
    }
    
    /* Not implemented - hopefuly nobody uses this

    // Copy channel tags
    {
      Octave_map::const_iterator itChTags = pfsStream.seek( "channelTags" );
      if( itChTags != pfsStream.end() ) {
        if( !pfsStream.contents( itChTags )(0).is_map() )
        {
          throw pfs::Exception( "'channelTags' field must be a structure" );  
        }
        Octave_map tagChannels = pfsStream.contents( itChTags )(0).map_value();
        for( Octave_map::iterator itCh = tagChannels.begin(); itCh != tagChannels.end(); itCh++ ) {
          std::string channelName = tagChannels.key(itCh);
          if( !tagChannels.contents( itCh )(0).is_map() ) {
            throw pfs::Exception( "each channelTags file must be a structure" );  
          }
          pfs::Channel *pfsChannel = frame->getChannel( channelName.c_str() );
          if( pfsChannel == NULL ) {
            throw pfs::Exception( "can not set channel tag if channel is missing" );
          }
          
          Octave_map tags = tagChannels.contents( itCh )(0).map_value();
          for( Octave_map::iterator itTag = tags.begin(); itTag != tags.end(); itTag++ ) {
            std::string tagName = tags.key(itTag);
            if( !tags.contents( itTag )(0).is_string() ) 
              throw pfs::Exception( "all channel tags must be given as strings" );
            std::string tagValue = tags.contents( itTag )(0).string_value();
            pfsChannel->getTags()->setString( tagName.c_str(), tagValue.c_str() );
          }
        }
      }
      
    }
*/


//    FILE *fh = fdopen( fid, "a" );
    FILE *fh = fdopen( dup( fid ), "w" );
    if( fh == NULL ) 
      error( SCRIPT_NAME ": Cannot open file for writting" );
    
    ctx.writeFrame( frame, fh );
    ctx.freeFrame( frame );
    fclose( fh );
    fsync( fid );

  }
  catch( pfs::Exception ex )
  {
    char error_message[100];
    sprintf( error_message, "%s: %s", SCRIPT_NAME, ex.getMessage() );
    error( error_message );      
  }    
    
}


