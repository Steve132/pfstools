/**
 * @brief Algorithm for automatic HDR images capture.
 *
 * 
 * This file is a part of PFS CALIBRATION package.
 * ---------------------------------------------------------------------- 
 * Copyright (C) 2006 Radoslaw Mantiuk
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
 * @author Radoslaw Mantiuk, <radoslaw.mantiuk@gmail.com>
 *
 */


#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#define NRANSI
#include "nrutil.h"

#include "mitsunaga99_numerical.h"


#define TINY 1.0e-20;
void ludcmp(float **a, int n, int *indx, float *d)
{
	int i,imax,j,k;
	float big,dum,sum,temp;
	float *vv;

	vv=vector(1,n);
	*d=1.0;
	for (i=1;i<=n;i++) {
		big=0.0;
		for (j=1;j<=n;j++)
			if ((temp=fabs(a[i][j])) > big) big=temp;
		if (big == 0.0) nrerror("Singular matrix in routine ludcmp");
		vv[i]=1.0/big;
	}
	
	for (j=1;j<=n;j++) {
		for (i=1;i<j;i++) {
			sum=a[i][j];
			for (k=1;k<i;k++) sum -= a[i][k]*a[k][j];
			a[i][j]=sum;
		}
		big=0.0;
		for (i=j;i<=n;i++) {
			sum=a[i][j];
			for (k=1;k<j;k++)
				sum -= a[i][k]*a[k][j];
			a[i][j]=sum;
			if ( (dum=vv[i]*fabs(sum)) >= big) {
				big=dum;
				imax=i;
			}
		}
		if (j != imax) {
			for (k=1;k<=n;k++) {
				dum=a[imax][k];
				a[imax][k]=a[j][k];
				a[j][k]=dum;
			}
			*d = -(*d);
			vv[imax]=vv[j];
		}
		indx[j]=imax;
		if (a[j][j] == 0.0) a[j][j]=TINY;
		if (j != n) {
			dum=1.0/(a[j][j]);
			for (i=j+1;i<=n;i++) a[i][j] *= dum;
		}
	}
	free_vector(vv,1,n);
}
#undef TINY

void lubksb(float **a, int n, int *indx, float b[])
{
	int i,ii=0,ip,j;
	float sum;

	for (i=1;i<=n;i++) {
		ip=indx[i];
		sum=b[ip];
		b[ip]=b[i];
		if (ii)
			for (j=ii;j<=i-1;j++) sum -= a[i][j]*b[j];
		else if (sum) ii=i;
		b[i]=sum;
	}
	for (i=n;i>=1;i--) {
		sum=b[i];
		for (j=i+1;j<=n;j++) sum -= a[i][j]*b[j];
		b[i]=sum/a[i][i];
	}
}


/** Solves the system of linear equations. The result is returned in b[].
* @param n number of equations (equal to number of variables)
* @param a[][] coefficients of variables
* @param b[] free coefficients
*/
int MitsunagaNumerical::linearEquationsSystem( int n, float a[][NR_LINEAR_EQUATIONS_MAX], float b[]) {

	if( n >= NR_LINEAR_EQUATIONS_MAX) {
		fprintf( stderr, "WARNING: too many equations (nr_solver_linear_equations())\n");
		return 1;
	}

	float dd;
	
	int N = n+1;
	
	int indx[N];
	float bb[N];
	float** aa;
	aa = (float**)malloc(sizeof(float*) * N);
	for(int i = 0; i < N; i++) 
		aa[i] = (float*)malloc(sizeof(float) * N);
	
	for( int i = 0; i < n; i++) {
		bb[i+1] = b[i];
		
		for( int j = 0; j < n; j ++) {
			aa[i+1][j+1] = a[i][j];
		}	
	}		
		
	ludcmp( aa, n, indx, &dd); 
	lubksb( aa, n, indx, bb);

	for(int i = 0; i < N; i++)
		if( aa[i] != NULL)
			free(aa[i]);	
	if( aa != NULL)
		free(aa);
	
	
	for( int i = 0; i < n; i++) {
		b[i] = bb[i+1];
	}	

	return 0;
}




