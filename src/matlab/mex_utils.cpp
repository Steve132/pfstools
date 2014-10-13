#include <string.h>

#include "mex_utils.h"


mxArray *mxCreateLogicalScalar( bool val )
{
  mxArray *array = mxCreateNumericMatrix( 1, 1, mxDOUBLE_CLASS, mxREAL );
  double *df_ptr = (double*)mxGetData( array );
  *df_ptr = val;
  return array;
}


mxArray *create_mex_double( double val )
{
  mxArray *array = mxCreateNumericMatrix( 1, 1, mxDOUBLE_CLASS, mxREAL );
  double *df_ptr = (double*)mxGetData( array );
  *df_ptr = val;
  return array;
}

mxArray *create_mex_string( const char *str )
{
	char *buf = (char*)mxCalloc( strlen( str )+1, sizeof(char) );
	strcpy( buf, str );
	return mxCreateString( buf );
}

char *get_mex_string( const mxArray *arg )
{ 
  /* Input must be a string. */
  if (mxIsChar(arg) != 1)
    mexErrMsgTxt("Input must be a string.");

  /* Input must be a row vector. */
  if (mxGetM(arg) != 1)
    mexErrMsgTxt("Input must be a row vector.");
    
  /* Get the length of the input string. */
  int buflen = (mxGetM(arg) * mxGetN(arg)) + 1;

  /* Allocate memory for input and output strings. */
  char *str = (char*)mxCalloc(buflen, sizeof(char));
  
  /* Copy the string data from prhs[0] into a C string 
   * input_buf. */
  int status = mxGetString(arg, str, buflen);

  if (status != 0) 
	mexWarnMsgTxt("Not enough space. String is truncated.");
 
  return str;
}

double get_mex_double( const mxArray *arg )
{
	double *data = mxGetPr( arg );   
	return data[0];
}

float get_mex_float( const mxArray *arg )
{
	float* data = (float*)mxGetPr( arg );   
	return data[0];
}


int set_mex_field( mxArray *arg, const char *field_name, mxArray *value )
{ 
	int fnum = mxGetFieldNumber( arg, field_name );
	if( fnum == -1 ) {
		fnum = mxAddField( arg, field_name );
	}
	mxSetFieldByNumber( arg, 0, fnum, value );

	return fnum;
}

bool is_mex_scalar( const mxArray *arg )
{
  if( arg == NULL ) return false;
  return mxIsNumeric( arg ) && mxGetN( arg ) == 1 && mxGetM( arg ) == 1;
}

bool is_mex_fid( const mxArray *arg )
{
  if( arg == NULL ) return false;
  return mxIsNumeric( arg ) && (mxGetN( arg ) == 1 || mxGetN( arg ) == 2) && mxGetM( arg ) == 1;
}

void copy_mex_to_pfsarray( const mxArray *arg, pfs::Array2D *array )
{
        assert( mxIsSingle( arg ) || mxIsDouble( arg ) );
        if ( mxIsDouble( arg ) )
                copy_mex_to_pfsarray_T<double>( arg, array );
        else
	        copy_mex_to_pfsarray_T<float>( arg, array );
}

void copy_mex_to_pfsarray( const mxArray *arg, pfs::Array2D *d1, pfs::Array2D *d2, pfs::Array2D *d3 )
{
        assert( mxIsSingle( arg ) || mxIsDouble( arg ) );
        if ( mxIsDouble( arg ) )
                copy_mex_to_pfsarray_T<double>( arg, d1, d2, d3 );
        else
	        copy_mex_to_pfsarray_T<float>( arg, d1, d2, d3 );
}

void copy_pfsarray_to_mex( const pfs::Array2D *array, mxArray *arg )
{
        assert( mxIsSingle( arg ) || mxIsDouble( arg ) );
        if ( mxIsDouble( arg ) )
                copy_pfsarray_to_mex_T<double>( array, arg );
        else
	        copy_pfsarray_to_mex_T<float>( array, arg );
}

void copy_pfsarray_to_mex( const pfs::Array2D *d1, const pfs::Array2D *d2, const pfs::Array2D *d3, mxArray *arg )
{
        assert( mxIsSingle( arg ) || mxIsDouble( arg ) );
        if ( mxIsDouble( arg ) )
                copy_pfsarray_to_mex_T<double>( d1, d2, d3, arg );
        else
	        copy_pfsarray_to_mex_T<float>( d1, d2, d3, arg );
}

void copy_mex_to_pfschannel( const mxArray *arg, pfs::Channel *pfsChannel )
{
        assert( mxIsSingle( arg ) || mxIsDouble( arg ) );
        if ( mxIsDouble( arg ) )
                copy_mex_to_pfschannel_T<double>( arg, pfsChannel );
        else
	        copy_mex_to_pfschannel_T<float>( arg, pfsChannel );
}

void copy_pfschannel_to_mex( const pfs::Channel *pfsChannel, mxArray *arg )
{
        assert( mxIsSingle( arg ) || mxIsDouble( arg ) );
        if ( mxIsDouble( arg ) )
                copy_pfschannel_to_mex_T<double>( pfsChannel, arg );
        else
	        copy_pfschannel_to_mex_T<float>( pfsChannel, arg );
}
