/**
 * @brief Read RGBE files
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
 * $Id: pfs_read_rgbe.cpp,v 1.2 2007/03/01 14:00:44 rdmantiuk Exp $
 */

#include "compatibility.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "mex.h"
#include "mex_utils.h"

#include <rgbeio.h>

#define SCRIPT_NAME "pfs_read_rgbe"
#define error mexErrMsgTxt

#include <pfs.h>


void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[])
{     
  /* Check for proper number of arguments. */
  if( nrhs != 1 || 
	  !mxIsChar( prhs[0] ) 
	  )
	    error(SCRIPT_NAME ": Improper usage!");
 
  try {

		const char *file_name = get_mex_string( prhs[0] );
		FILE *fh = fopen( file_name, "rb" );
		if( fh == NULL ) {
		    error(SCRIPT_NAME ": Cannot open file for reading.");
		}

		RGBEReader reader( fh );

		const int rows = reader.getHeight(), cols = reader.getWidth();
		const int dims[] = { rows, cols, 3 };
		plhs[0] = mxCreateNumericArray( 3, dims, mxDOUBLE_CLASS, mxREAL );


        pfs::Array2DImpl ch1( cols, rows ), ch2( cols, rows ), ch3( cols, rows );
		reader.readImage( &ch1, &ch2, &ch3 );
		//pfs::transformColorSpace( pfs::CS_XYZ, &ch1, &ch2, &ch3, pfs::CS_RGB, &ch1, &ch2, &ch3 );

		copy_pfsarray_to_mex( &ch1, &ch2, &ch3, plhs[0] );

    }
    catch( pfs::Exception ex )
    {
        char error_message[100];
        sprintf( error_message, "%s: %s", SCRIPT_NAME, ex.getMessage() );
        error( error_message );      
    }    
}
