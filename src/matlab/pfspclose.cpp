#include "compatibility.h"

#include <stdio.h>
#include <stdlib.h>

#if defined( __win32__ )
#include <io.h>
#include <process.h>
#else
#include <unistd.h>
#endif

#include "mex.h"
#include "mex_utils.h"

#define SCRIPT_NAME "pfspclose"

void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[])
{     
  /* Check for proper number of arguments. */
  if (nrhs != 1 && !is_mex_scalar( prhs[0] ) ) 
    mexErrMsgTxt( SCRIPT_NAME ": Expecting one parameter - file descriptor." );

  if( mxGetN(prhs[0]) != 2 )
    mexErrMsgTxt( SCRIPT_NAME ": File descriptor not created with pfspopen." );
  
  double *p_fid = mxGetPr( prhs[0] );
//  int fid = (int)p_fid[0];
  FILE *fh = (FILE*)((unsigned long)p_fid[1]);

//  FILE *fh = fdopen( fid, "r" );
  if( pclose( fh ) == -1 )
    mexErrMsgTxt( SCRIPT_NAME ": pclose has failed.");

//  int res = close( fid );
//  if( res == -1 )
//	mexErrMsgTxt( SCRIPT_NAME ": pclose has failed.");
  
  return;
}
