/**
 * @brief Close pfs stream
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
 * $Id: pfsclose.cpp,v 1.1 2005/06/15 13:36:54 rafm Exp $
 */

#include <octave/oct.h>
#include <octave/oct-stream.h>
#include <octave/ov-struct.h>

//#include "../../config.h"				// conflicts with config.h from octave distribution

#include <string>
#include <pfs.h>

#define SCRIPT_NAME "pfsclose"

static const char *helpString =
#include "pfsclose_help.h"

DEFUN_DLD( pfsclose, args, , helpString )
{
  octave_value_list retval;

  int nargin = args.length();

  // Get arguments and check if they are legal
    
  if( nargin != 1 || !args(0).is_map() )
  {
    error( SCRIPT_NAME ": Improper usage!");
    return retval;
  }

  octave_map pfsStream = args(0).map_value();

  octave_scalar_map::const_iterator itFH = pfsStream.seek( "FH" );
  if( itFH == pfsStream.end() ||
    !pfsStream.contents( itFH )(0).is_real_scalar() )
  {
    error( SCRIPT_NAME ": FH field missing in the structure or it has wrong type");
    return retval;
  }  
  FILE *fh = (FILE*)((long)(pfsStream.contents( itFH )(0).double_value()));

  // close file if not stdin or stdout

  if( fh != stdin && fh != stdout )
    fclose( fh );
    
  return retval;
}
