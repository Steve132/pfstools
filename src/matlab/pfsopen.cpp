/**
 * @brief Open pfs stream for reading or writting in MATLAB
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
 * $Id: pfsopen.cpp,v 1.6 2010/06/05 18:24:24 rafm Exp $
 */

#include "compatibility.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "mex.h"
#include "matrix.h"
#include "mex_utils.h"

#define SCRIPT_NAME "pfsopen"
#define error mexErrMsgTxt

// Because there is no O_BINARY under UNIX 
#ifndef O_BINARY
#define O_BINARY 0
#endif

#include <pfs.h>



void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[])
{     
  /* Check for proper number of arguments. */
  if (nrhs < 1 || nrhs > 3) 
    error(SCRIPT_NAME ": Improper usage!");
 
  if( !mxIsChar(prhs[0]) && !is_mex_fid( prhs[0] ) ) {
    error( SCRIPT_NAME ": expected file name or file descriptor as the first argument!");
  }

  int width, height;
  bool writeMode = false;
    
  if( nrhs == 3 ) {
    if( !is_mex_scalar( prhs[1]) || !is_mex_scalar( prhs[2] ) ) {
      error( SCRIPT_NAME ": expected frame dimmensions as argument 2 and 3");
    }
    height = (int)*mxGetPr( prhs[1] );
    width = (int)*mxGetPr( prhs[2] );
    writeMode = true;      
  }
  if( nrhs == 2 ) {
    if( !mxIsNumeric( prhs[1] ) || mxGetM( prhs[1] ) != 1 || mxGetN( prhs[1] ) != 2) {
      error( SCRIPT_NAME ": expected 2-column matrix as argument 2");
    }
	double *dim = mxGetPr( prhs[1] );   
    height = (int)dim[0];
    width = (int)dim[1];
    writeMode = true;
  }

  if( writeMode && (width < 1 || height < 1 || width > 65535 || height > 65535 ) ) {
    error( SCRIPT_NAME ": Illegal frame size");
  }

  // Open file for reading or writing
    
  int fid;
  if( mxIsChar( prhs[0] ) ) {
    // File name given
    char *fileName = get_mex_string( prhs[0] );
    if( writeMode ) {
      if( !strcmp( "stdout", fileName ) ) 
        fid = 1;
      else {
        fid = open( fileName, O_CREAT | O_TRUNC | O_RDWR | O_BINARY, S_IREAD | S_IWRITE );
        if( fid == -1 ) {
          error( SCRIPT_NAME ": cannot open file for writing!");
        }
      }    
    } else {
      if( !strcmp( "stdin", fileName ) ) 
        fid = 0;
      else {
        fid = open( fileName, O_RDONLY | O_BINARY );
        if( fid == -1 ) {
          error( SCRIPT_NAME ": cannot open file for reading!");
        }
      }
    }
    mxFree( fileName );

  } else {
    // File descriptor given
    double *p_fid = mxGetPr( prhs[0] );
    fid = dup( (int)p_fid[0] );
    if( fid == -1 ) 
      error( SCRIPT_NAME ": Failed to use the file descriptor" );
  }

  mxArray *pfs_stream = mxCreateStructMatrix( 1, 1, 0, NULL );
  plhs[0] = pfs_stream;

  int fnum;
  fnum = mxAddField( pfs_stream, "FID" );
  mxSetFieldByNumber( pfs_stream, 0, fnum, create_mex_double( fid ) );

  fnum = mxAddField( pfs_stream, "MODE" );
  if( writeMode )
    mxSetFieldByNumber( pfs_stream, 0, fnum, create_mex_string("W") );
  else
    mxSetFieldByNumber( pfs_stream, 0, fnum, create_mex_string("R") );

  fnum = mxAddField( pfs_stream, "EOF" );
  mxSetFieldByNumber( pfs_stream, 0, fnum, mxCreateLogicalScalar( false ) );

  if( writeMode ) {
	fnum = mxAddField( pfs_stream, "columns" );
	mxSetFieldByNumber( pfs_stream, 0, fnum, create_mex_double( width ) );
	fnum = mxAddField( pfs_stream, "rows" );
	mxSetFieldByNumber( pfs_stream, 0, fnum, create_mex_double( height ) );

	fnum = mxAddField( pfs_stream, "channels" );
	mxArray *channels = mxCreateStructMatrix( 1, 1, 0, NULL );
	mxSetFieldByNumber( pfs_stream, 0, fnum, channels );

  }
}


