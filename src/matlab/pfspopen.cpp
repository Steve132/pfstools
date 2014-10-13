#include "compatibility.h"

#include <stdio.h>
#include <stdlib.h>

#include "mex.h"
#include "mex_utils.h"

#define SCRIPT_NAME "pfspopen"

extern void _main();


void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[])
{     
  /* Check for proper number of arguments. */
  if (nrhs != 2) 
    mexErrMsgTxt("Expecting two parameters.");
 
  char *cmd_line = get_mex_string( prhs[0] );
  char *mode = get_mex_string( prhs[1] );

  FILE *in = popen( cmd_line, mode );
  if( in == NULL ) {		
	mexErrMsgTxt("popen has failed.");
  }

  int fd = fileno( in );
//  int fd = dup( fileno( in ) );
//  if( fclose( in ) != 0 ) 
//	  mexErrMsgTxt("fclose has failed.");

  plhs[0] = mxCreateNumericMatrix( 1, 2, mxDOUBLE_CLASS, mxREAL );
  double *df_ptr = (double*)mxGetData( plhs[0] );
  *df_ptr = fd;
  // Very bad hack
  df_ptr[1] = (unsigned long)in;

  mxFree( cmd_line );
  mxFree( mode );

  return;
}
