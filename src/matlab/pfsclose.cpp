/**
 * @brief Close pfs stream (MATLAB interface)
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
 * $Id: pfsclose.cpp,v 1.5 2007/03/01 14:00:44 rdmantiuk Exp $
 */

#include "compatibility.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <list>

#include <pfs.h>

#include "mex.h"
#include "mex_utils.h"

#define SCRIPT_NAME "pfsclose"
#define error mexErrMsgTxt



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
  int ret = close( fid );
  if( ret != 0 )
	  error( SCRIPT_NAME ": Cannot close the file. Perhaps it is already closed.");
}
