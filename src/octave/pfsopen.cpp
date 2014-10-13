/**
 * @brief Open pfs stream for reading or writting in GNU Octave
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
 * $Id: pfsopen.cpp,v 1.5 2008/05/06 18:01:32 rafm Exp $
 */

#include <octave/oct.h>
#include <octave/oct-stream.h>
#include <octave/ov-struct.h>

//#include "../../config.h"				// conflicts with config.h from octave distribution

#include <string>
#include <unistd.h>
#include <pfs.h>

#define SCRIPT_NAME "pfsopen"

static const char *helpString =
#include "pfsopen_help.h"

DEFUN_DLD( pfsopen, args, , helpString)
{
  octave_value_list retval;

  int nargin = args.length();

  // Get arguments and check if they are legal
    
  if( nargin < 1 || nargin > 3 )
  {
    error( SCRIPT_NAME ": Improper usage!");
    return retval;
  }

  if( !args(0).is_string() && !args(0).is_real_scalar() ) {
    error( SCRIPT_NAME ": expected file name or file descriptor as the first argument!");
    // file descriptors are represented as integers (stored as doubles) in Octave 3.0
    return retval;
  }

  int width, height;
  bool writeMode = false;
    
  if( nargin == 3 ) {
    if( !args(1).is_real_scalar() || !args(2).is_real_scalar() ) {
      error( SCRIPT_NAME ": expected frame dimmensions as argument 2 and 3");
      return retval;
    }
    height = (int)args(1).double_value();
    width = (int)args(2).double_value();
    writeMode = true;      
  }
  if( nargin == 2 ) {
    if( !args(1).is_real_matrix() ) {
      error( SCRIPT_NAME ": expected matrix as argument 2");
      return retval;
    }
    Matrix dim = args(1).matrix_value();
    if( dim.rows() != 1 || dim.columns() != 2 ) {
      error( SCRIPT_NAME ": expected 1x2 matrix with frame size as argument 2");
      return retval;
    }
    height = (int)dim(0,0);
    width = (int)dim(0,1);
    writeMode = true;
  }

  if( writeMode && (width < 1 || height < 1 || width > 65535 || height > 65535 ) ) {
    error( SCRIPT_NAME ": Illegal frame size");
    return retval;      
  }

  // Open file for reading or writing
    
  FILE *fh;
  if( args(0).is_string() ) {
    // File name given
    const char *fileName = args(0).string_value().c_str();
    if( writeMode ) {
      if( !strcasecmp( "stdout", fileName ) ) 
        fh = stdout;
      else {
        fh = fopen( fileName, "wb" );
        if( fh == NULL ) {
          error( SCRIPT_NAME ": cannot open file for writing!");
          return retval;
        }
      }    
    } else {
      if( !strcasecmp( "stdin", fileName ) ) 
        fh = stdin;
      else {
        fh = fopen( fileName, "rb" );
        if( fh == NULL ) {
          error( SCRIPT_NAME ": cannot open file for reading!");
          return retval;
        }
      }
    }
  } else {
    // File descriptor given
    int fd = dup( (int) args(0).scalar_value() );
    if( writeMode ) {
      fh = fdopen( fd, "wb" );
      if( fh == NULL ) {
        error( SCRIPT_NAME ": cannot open file for writing!");
        return retval;
      }
    } else {
      fh = fdopen( fd, "rb" );
      if( fh == NULL ) {
        error( SCRIPT_NAME ": cannot open file for reading!");
        return retval;
      }
    }    
  }

  Octave_map pfsStream;
  pfsStream.assign( "FH", octave_value((double)((long)fh)) );
  pfsStream.assign( "MODE", writeMode ? octave_value("W") : octave_value("R") );
  pfsStream.assign( "EOF", octave_value(false) );

  if( writeMode ) {
    pfsStream.assign( "columns", octave_value(width) );
    pfsStream.assign( "rows", octave_value(height) );
    Octave_map channels;
    pfsStream.assign( "channels", octave_value(channels) );    
  }
  
  retval.append(pfsStream);
    
  return retval;
}
