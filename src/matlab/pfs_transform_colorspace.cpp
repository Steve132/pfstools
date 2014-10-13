/**
 * @brief Tranform between color spaces using pfs library in MATLAB
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
 * $Id: pfs_transform_colorspace.cpp,v 1.10 2014/02/16 19:02:30 rafm Exp $
 */

#include "compatibility.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "mex.h"
#include "mex_utils.h"

#define SCRIPT_NAME "pfs_transform_colorspace"
#define error mexErrMsgTxt

#include <pfs.h>


static pfs::ColorSpace findColorSpace( const char *name );

void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[])
{     
  /* Check for proper number of arguments. */
  if( (nrhs != 5 || 
	  !mxIsChar( prhs[0] ) ||
	  !( mxIsDouble( prhs[1] ) || mxIsSingle( prhs[1] ) ) ||
	  !( mxIsDouble( prhs[2] ) || mxIsSingle( prhs[2] ) ) ||
	  !( mxIsDouble( prhs[3] ) || mxIsSingle( prhs[3] ) ) ||
	  !mxIsChar( prhs[4] )) &&
	  (nrhs != 3 || 
	  !mxIsChar( prhs[0] ) ||
	  !( mxIsDouble( prhs[1] ) || mxIsSingle( prhs[1] ) ) || 
	  mxGetNumberOfDimensions( prhs[1] ) != 3 ||
	  !mxIsChar( prhs[2] ))	  
	  )
	    error(SCRIPT_NAME ": Wrong number or type of input parameters!");

  if( nlhs != 3 && nlhs != 1 )
      error(SCRIPT_NAME ": Wrong number of output parameters!");
      
  
  try {
        pfs::ColorSpace inCS = findColorSpace( get_mex_string( prhs[0] ) );
        pfs::ColorSpace outCS = findColorSpace( get_mex_string( prhs[nrhs-1] ) );

        const mwSize *in_dim = mxGetDimensions( prhs[1] );
        const int rows = in_dim[0], cols = in_dim[1];

//		printf( "%d x %d\n", rows, cols );
        
        pfs::Array2DImpl c1Buf( cols, rows ), c2Buf( cols, rows ), c3Buf( cols, rows );
       	
		if( nrhs == 5 ) {
			copy_mex_to_pfsarray( prhs[1], &c1Buf );
			copy_mex_to_pfsarray( prhs[2], &c2Buf );
			copy_mex_to_pfsarray( prhs[3], &c3Buf );
		} else {
			copy_mex_to_pfsarray( prhs[1], &c1Buf, &c2Buf, &c3Buf );
		}

        pfs::transformColorSpace( inCS, &c1Buf, &c2Buf, &c3Buf, outCS, &c1Buf, &c2Buf, &c3Buf );

		if( nlhs == 3 ) {
			plhs[0] = mxCreateNumericMatrix( rows, cols, mxDEFAULT_ARRAY_CLASS, mxREAL );
			plhs[1] = mxCreateNumericMatrix( rows, cols, mxDEFAULT_ARRAY_CLASS, mxREAL );
			plhs[2] = mxCreateNumericMatrix( rows, cols, mxDEFAULT_ARRAY_CLASS, mxREAL );

			copy_pfsarray_to_mex( &c1Buf, plhs[0] );
			copy_pfsarray_to_mex( &c2Buf, plhs[1] );
			copy_pfsarray_to_mex( &c3Buf, plhs[2] );
		} else if( nlhs == 1 ) {
			mwSize dims[] = { rows, cols, 3 };
			plhs[0] = mxCreateNumericArray( 3, dims, mxDEFAULT_ARRAY_CLASS, mxREAL );

			copy_pfsarray_to_mex( &c1Buf, &c2Buf, &c3Buf, plhs[0] );
		}

    }
    catch( pfs::Exception ex )
    {
        char error_message[100];
        sprintf( error_message, "%s: %s", SCRIPT_NAME, ex.getMessage() );
        error( error_message );      
    }    
}

static pfs::ColorSpace findColorSpace( const char *name )
{
    if( !strcasecmp( name, "XYZ" ) ) return pfs::CS_XYZ;
    else if( !strcasecmp( name, "RGB" ) ) return pfs::CS_RGB;
    else if( !strcasecmp( name, "SRGB" ) ) return pfs::CS_SRGB;
    else if( !strcasecmp( name, "YUV" ) ) return pfs::CS_YUV;
    else if( !strcasecmp( name, "YXY" ) ) return pfs::CS_Yxy;

    throw pfs::Exception( "Not recognized color space" );
}

