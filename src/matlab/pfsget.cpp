/**
 * @brief Read frame from pfs stream in Matlab
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
 * $Id: pfsget.cpp,v 1.5 2008/05/06 17:11:35 rafm Exp $
 */

#include "compatibility.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <list>

#include "mex.h"
#include "mex_utils.h"

#define SCRIPT_NAME "pfsget"
#define error mexErrMsgTxt

#include <pfs.h>


void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[])
{     

  /* Check for proper number of arguments. */
  if( nrhs != 1 || !mxIsStruct( prhs[0] ) ) 
    error(SCRIPT_NAME ": Improper usage!");
 
  const mxArray *pfs_stream_in = prhs[0];
  mxArray *pfs_stream = mxCreateStructMatrix( 1, 1, 0, NULL );
  
  mxArray *f_fid = mxGetField( pfs_stream_in, 0, "FID" );
  if( f_fid == NULL || !is_mex_scalar( f_fid ) )
  {
    error( SCRIPT_NAME ": FH field missing in the structure or it has wrong type");
  }
  
  int fid = (int)get_mex_double( f_fid );

  set_mex_field( pfs_stream, "FID", mxDuplicateArray( f_fid ) );

  // Check mode
  {                             
	mxArray *f_mode = mxGetField( pfs_stream_in, 0, "MODE" );
	if( f_mode == NULL ) {
      error( SCRIPT_NAME ": MODE field missing in the structure or it has wrong type");
    }
    if( strcmp( "R", get_mex_string( f_mode ) ) ) {
      error( SCRIPT_NAME ": Can not read from the stream that is open for writing." );
    }
	set_mex_field( pfs_stream, "MODE", mxDuplicateArray( f_mode ) );
  }

  
  //mxRemoveField( pfs_stream, mxGetFieldNumber( pfs_stream, "channels" ) );
  //mxRemoveField( pfs_stream, mxGetFieldNumber( pfs_stream, "tags" ) );
  //mxRemoveField( pfs_stream, mxGetFieldNumber( pfs_stream, "channelTags" ) );
  
  FILE *fh = fdopen( dup( fid ), "r" );
  if( fh == NULL ) 
		error( SCRIPT_NAME ": Cannot open file for reading" );

  try {
    pfs::DOMIO ctx;

    pfs::Frame *frame = ctx.readFrame( fh );
    if( frame == NULL ) {         // No more frames
          
		set_mex_field( pfs_stream, "EOF", mxCreateLogicalScalar( true ) );
      
    } else {                    // Not EOF

      // Set width and height
      {
		  set_mex_field( pfs_stream, "columns", create_mex_double( frame->getWidth() ) );
		  set_mex_field( pfs_stream, "rows", create_mex_double( frame->getHeight() ) );
      }
    
      // Add channels as matrices to pfs stream struct
      {
        mxArray *channels = mxCreateStructMatrix( 1, 1, 0, NULL );
        
        pfs::ChannelIteratorPtr cit( frame->getChannelIterator() );
        while( cit->hasNext() ) {
          pfs::Channel *ch = cit->getNext();

	  mxArray *mat = mxCreateNumericMatrix( ch->getRows(), ch->getCols(), mxDEFAULT_ARRAY_CLASS, mxREAL );
	  copy_pfschannel_to_mex( ch, mat );
	  set_mex_field( channels, ch->getName(), mat );
        }
        set_mex_field( pfs_stream, "channels", channels );
      }

      //Add tags (only frame tags, no channel tags)
      {
        mxArray *tags = mxCreateStructMatrix( 1, 1, 0, NULL );
        pfs::TagIteratorPtr it( frame->getTags()->getIterator() );        
        while( it->hasNext() ) {
          const char *tagName = it->getNext();
          mxArray *tag_value = create_mex_string( frame->getTags()->getString( tagName ) );          
          set_mex_field( tags, tagName, tag_value );          
        }
        set_mex_field( pfs_stream, "tags", tags );        
      }
      
      //Add tags
/*      {
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
        
      }*/
      
    
    }
    ctx.freeFrame( frame );

	plhs[0] = pfs_stream;
  }
  catch( pfs::Exception ex )
  {
    char error_message[100];
    sprintf( error_message, "%s: %s", SCRIPT_NAME, ex.getMessage() );
    error( error_message );      
  }
  if( fh != NULL ) fclose( fh );        
 }
