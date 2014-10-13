#ifndef MEX_UTILS_H
#define MEX_UTILS_H

#include "mex.h"
#include "array2d.h"
#include <pfs.h>

namespace pfs {
	class Array2D;
}

#define mxDEFAULT_ARRAY_CLASS mxSINGLE_CLASS

mxArray *create_mex_double( double val );
mxArray *create_mex_string( const char *str );
char *get_mex_string( const mxArray *arg );
double get_mex_double( const mxArray *arg );
float get_mex_float( const mxArray *arg );
bool is_mex_scalar( const mxArray *arg );
bool is_mex_fid( const mxArray *arg );
int set_mex_field( mxArray *arg, const char *field_name, mxArray *value );
void copy_mex_to_pfsarray( const mxArray *arg, pfs::Array2D *array );
void copy_pfsarray_to_mex( const pfs::Array2D *array, mxArray *arg );

void copy_mex_to_pfsarray( const mxArray *arg, pfs::Array2D *d1, pfs::Array2D *d2, pfs::Array2D *d3 );
void copy_pfsarray_to_mex( const pfs::Array2D *d1, const pfs::Array2D *d2, const pfs::Array2D *d3, mxArray *arg );

void copy_mex_to_pfschannel( const mxArray *arg, pfs::Channel *pfsChannel );
void copy_pfschannel_to_mex( const pfs::Channel *pfsChannel, mxArray *arg );

mxArray *mxCreateLogicalScalar( bool value );



template <class T>
void copy_mex_to_pfsarray_T( const mxArray *arg, pfs::Array2D *array )
{
	assert( array->getCols() == mxGetN( arg ) && array->getRows() == mxGetM( arg ) );
	const T *data = (T *)mxGetPr( arg );
	for( int c = 0; c < array->getCols(); c++ ) 
		for( int r = 0; r < array->getRows(); r++ ) {
			(*array)(c,r) = (float)*(data++);
        }		
}

template <class T>
void copy_mex_to_pfsarray_T( const mxArray *arg, pfs::Array2D *d1, pfs::Array2D *d2, pfs::Array2D *d3 )
{
	assert( d1->getCols() == mxGetN( arg ) && d1->getRows() == mxGetM( arg ) );
	T *o1 = (T *)mxGetPr( arg );
	const long sdsize = d1->getCols()*d1->getRows();
	T *o2 = o1 + sdsize;
	T *o3 = o2 + sdsize;
	for( int c = 0; c < d1->getCols(); c++ ) 
		for( int r = 0; r < d1->getRows(); r++ ) {
			(*d1)(c,r) = (float)*(o1++);
			(*d2)(c,r) = (float)*(o2++);
			(*d3)(c,r) = (float)*(o3++);
        }		
}

template <class T>
void copy_pfsarray_to_mex_T( const pfs::Array2D *array, mxArray *arg )
{
	assert( array->getCols() == mxGetN( arg ) && array->getRows() == mxGetM( arg ) );
	T *data = (T *)mxGetPr( arg );
	for( int c = 0; c < array->getCols(); c++ ) 
		for( int r = 0; r < array->getRows(); r++ ) {
			*(data++) = (*array)(c,r);
        }		
}

template <class T>
void copy_pfsarray_to_mex_T( const pfs::Array2D *d1, const pfs::Array2D *d2, const pfs::Array2D *d3, mxArray *arg )
{
	assert( d1->getCols() == mxGetN( arg ) && d1->getRows() == mxGetM( arg ) );
	T *o1 = (T *)mxGetPr( arg );
	const long sdsize = d1->getCols()*d1->getRows();
	T *o2 = o1 + sdsize;
	T *o3 = o2 + sdsize;
	for( int c = 0; c < d1->getCols(); c++ ) 
		for( int r = 0; r < d1->getRows(); r++ ) {
			*(o1++) = (*d1)(c,r);
			*(o2++) = (*d2)(c,r);
			*(o3++) = (*d3)(c,r);
        }		
}

template <class T>
void copy_mex_to_pfschannel_T( const mxArray *arg, pfs::Channel *pfsChannel )
{
        int index = 0;
	T *data = (T *)mxGetPr( arg );
	for( int c = 0; c < pfsChannel->getCols(); c++ ) 
	        for( int r = 0; r < pfsChannel->getRows(); r++ ) {
		        (*pfsChannel)(c,r) = *(data++);
	}
}

template <class T>
void copy_pfschannel_to_mex_T( const pfs::Channel *pfsChannel, mxArray *arg )
{
        int index = 0;
	T *data = (T *)mxGetPr( arg );
	for( int c = 0; c < pfsChannel->getCols(); c++ ) 
	        for( int r = 0; r < pfsChannel->getRows(); r++ ) {
		        *(data++) = (*pfsChannel)(c,r);
	}
}

#endif
