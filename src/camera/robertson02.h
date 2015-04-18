/**
 * @brief Robertson02 algorithm for automatic self-calibration.
 *
 * 
 * This file is a part of PFS CALIBRATION package.
 * ---------------------------------------------------------------------- 
 * Copyright (C) 2004 Grzegorz Krawczyk
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
 * @author Grzegorz Krawczyk, <gkrawczyk@users.sourceforge.net>
 *
 * $Id: robertson02.h,v 1.4 2011/02/15 15:46:27 ihrke Exp $
 */

#ifndef _robertson02_h_
#define _robertson02_h_

#include <responses.h>

/**
 * @brief Calculate camera response using Robertson02 algorithm
 *
 * @param xj [out]  estimated luminance values
 * @param imgs reference to vector containing source exposures
 * @param I [out] array to put response function
 * @param w weights
 * @param M max camera output (no of discrete steps)
 * @return number of saturated pixels in the HDR image (0: all OK)
 */
int robertson02_getResponse( pfs::Array2D*       output,       /* output image (gray scale) */
			     const ExposureList &imgs,         /* differrent exposures */
			     float              *rcurve,       /* response curve */
			     const float        *weights,      /* weights */
			     int                 M             /* number of values in rcurve/weights */
			     );


/**
 * @brief Create HDR image by applying response curve to given images
 * taken with different exposures
 *
 * @param xj [out] HDR image
 * @param imgs reference to vector containing source exposures
 * @param I camera response function (array size of M)
 * @param w weighting function for camera output values (array size of M)
 * @param M number of camera output levels
 * @return number of saturated pixels in the HDR image (0: all OK)
 */
int robertson02_applyResponse( pfs::Array2D* xj, 
			       const ExposureList &imgs, 
			       const float* I, 
			       const float* w, 
			       int M );


int robertson02_applyResponseRGB( pfs::Array2D *rgb_out[],         
  const ExposureList *imgs[], 
  const float *resp_curve[], 
  const float *weights, 
  int M );

  
#endif /* #ifndef _robertson02_h_ */
