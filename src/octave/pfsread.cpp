/**
 * @brief GNU Octave wrapper - read selected channels from the PFS stream
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
 * $Id: pfsread.cpp,v 1.1 2005/06/15 13:36:54 rafm Exp $
 */

#include <octave/oct.h>
#include <octave/oct-stream.h>

#include <config.h>	// conflicts with config.h from octave distribution

#include <string>

#include <pfs.h>

#define SCRIPT_NAME "pfsread"

static const char *helpString =
#include "pfsread_help.h"

DEFUN_DLD( pfsread, args, , helpString)
{
    octave_value_list retval;
	
    int nargin = args.length();

    if( nargin < 1 )
    {
        error( SCRIPT_NAME ": Improper usage!");
        return retval;
    }

    if( !args(0).is_string() ) {
      error( SCRIPT_NAME ": expected file name as the first argument!");
      return retval;
    }

    FILE *fh;
    const char *fileName = args(0).string_value().c_str();
    if( !strcasecmp( "stdin", fileName ) ) 
      fh = stdin;
    else {
      fh = fopen( fileName, "rb" );
      if( fh == NULL ) {
        error( SCRIPT_NAME ": cannot open file for reading!");
        return retval;
      }
    }

    try {
      
        pfs::DOMIO ctx;
        pfs::Frame *frame = ctx.readFrame( fh );
        if( frame == NULL ) {         // No more frames
            for( int i = 0; i < nargin; i++ )
                retval(i) = std::string( "EOF" );
            return retval;      
        }
  
        for( int i = 1; i < nargin; i++ ) {
            if( !args(i).is_string() ) {
                error( SCRIPT_NAME ": expected string argument!");
                break;
            }
            const char *channelName = args(i).string_value().c_str();
            pfs::Channel *channel = frame->getChannel( channelName );

            if( channel == NULL ) {
                error( SCRIPT_NAME ": channel not found!");
                break;
            }
      
            Matrix mat( channel->getRows(), channel->getCols() );
            int index = 0;
            for( int r = 0; r < channel->getRows(); r++ ) // Copy channel data to Octave matrix
                for( int c = 0; c < channel->getCols(); c++ ) {
                    mat(r,c) = (*channel)(index++);
                }      
            retval(i-1) = mat;      
        }
  
        ctx.freeFrame( frame );

    }
    catch( pfs::Exception ex )
    {
        char error_message[100];
        sprintf( error_message, "%s: %s", SCRIPT_NAME, ex.getMessage() );
        error( error_message );      
    }

    if( fh != stdin ) fclose( fh );    
    
    return retval;
}
