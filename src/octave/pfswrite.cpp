/**
 * @brief GNU Octave wrapper - write matrices as channels to the PFS stream
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
 * $Id: pfswrite.cpp,v 1.1 2005/06/15 13:36:54 rafm Exp $
 */
#include <string>

#include <octave/oct.h>
#include <octave/oct-stream.h>

#include <pfs.h>

#define SCRIPT_NAME "pfswrite"

static const char *helpString =
#include "pfswrite_help.h"

DEFUN_DLD( pfswrite, args, , helpString)
{
    octave_value_list retval;
	
    int nargin = args.length();
    
    if( nargin < 3 )
    {
        error( SCRIPT_NAME ": Improper usage!");
        return retval;
    }

    // Check params
    if( !args(0).is_string() ) {
      error( SCRIPT_NAME ": expected file name as the first argument!");
      return retval;
    }
    for( int i = 1; i < nargin; i += 2 ) 
      if( (i+1) >= nargin || !args(i).is_string() || !args(i+1).is_real_matrix() ) {
        error( SCRIPT_NAME ": expected (channelName, matrix) pair argument!");
        return retval;
      }

    FILE *fh;
    const char *fileName = args(0).string_value().c_str();
    if( !strcasecmp( "stdout", fileName ) ) 
      fh = stdout;
    else {
      fh = fopen( fileName, "ab" );
      if( fh == NULL ) {
        error( SCRIPT_NAME ": cannot open file for writting!");
        return retval;
      }
    }
    
    try {
      
        pfs::DOMIO ctx;
        pfs::Frame *frame = NULL;

        int rows, cols;
        
        for( int i = 1; i < nargin; i += 2 ) {
      
            Matrix mat(args(i+1).matrix_value());
            if( frame == NULL ) { // Fist channel
                frame = ctx.createFrame(mat.cols(), mat.rows());
                rows = mat.rows();
                cols = mat.cols();
            }
            else if( mat.rows() != rows || mat.cols() != cols ) {
              // If size of this and the first channel differ
              error( SCRIPT_NAME ": all matrices must be of the same size");
              continue;
            }
            
      
            const char *channelName = args(i).string_value().c_str();
            pfs::Channel *channel = frame->createChannel( channelName );
      
            int index = 0;
            for( int r = 0; r < channel->getRows(); r++ ) // Copy octave matrix to channel
                for( int c = 0; c < channel->getCols(); c++ ) {
                    (*channel)(index++) = mat(r,c);
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

    if( fh != stdout ) fclose( fh );
  
    return retval;
}
