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

#ifndef _mitsunaga99_h_
#define _mitsunaga99_h_

#include <list>

using namespace std;

enum { R, G, B };
#define CH 3
#define MITSUNAGA_SAMPLES_NO 50000

class HDRCaptureMitsunaga {

private:
	list<unsigned long> pixelList;
	
public:
	
	int  capture( ExposureList &imgsR,        /* input red images */
		      ExposureList &imgsG,        /* input green images */ 
		      ExposureList &imgsB,        /* input blue images */ 
		      int M,                      /* ?? */
		      int poly_degree,            /* degree of polynomial approximation */
		      pfs::Array2D* Xj,           /* output HDR image red */
		      pfs::Array2D* Yj,           /* output HDR image green */
		      pfs::Array2D* Zj,           /* output HDR image blue */
		      unsigned long samples_no,   /* number of randomly selected pixels to use */
		      float* Ir,                  /* response curve red   in/out depending in comp_response */
		      float* Ig,                  /* response curve green in/out depending in comp_response */
		      float* Ib,                  /* response curve blue  in/out depending in comp_response */
		      float* w,                   /* weight function for HDR computation */
		      int comp_response           /* 0 - do not compute response
						     1 - compute response */
		      );
	
	int getResponse( const ExposureList &imgs, 
			 int poly_degree, 
			 float* coef, 
			 int M );

	void applyResponse( pfs::Array2D* xj, ExposureList &imgs, float* Iresp, int M, float* w) ;	
	void getInverseResponseArray( int c_cc, float* c, int cc, float* Iresp );
	void getDerivativeOfInverseResponseArray( int c_cc, float* c, int cc, float* w );
	
	int randomPixels( const ExposureList &imgs, unsigned long no);
	int getResponseFast( const ExposureList &imgs, int poly_degree, float* coef, int M);
	
};

#endif
