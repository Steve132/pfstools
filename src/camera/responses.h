/**
 * @brief Standard response functions
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
 * $Id: responses.h,v 1.5 2011/02/24 17:35:59 ihrke Exp $
 */


#ifndef _std_responses_h_
#define _std_responses_h_

#include <stdio.h>

#include <vector>
#include <array2d.h>

/**
 * @brief Container for images taken with different exposures
 */
class Exposure
{
public:
  float ti;			// exposure value (eg time)
  pfs::Array2D* yi;		// exposed pixel value (camera output)
  
  // to be able to sort exposures using stl
  static bool msort( Exposure const & a, Exposure const & b)  
  { return a.ti < b.ti; }
};


/**
 * @brief Container for a list of exposures
 */
typedef std::vector<Exposure> ExposureList;


/**
 * @brief Weighting function with gaussian distribution
 *
 * @param w [out] weights (array size of M)
 * @param M number of camera output levels
 * @param Mmin minimum registered camera output level
 * @param Mmax maximum registered camera output level
 * @param sigma sigma value for gaussian
 */
void weightsGauss( float* w, int M, int Mmin, int Mmax, float sigma );

/**
 * @brief Composite weighting function - dark regions
 * gamma curve, dark to medium - Gaussian, medium to high linear
 * topmost values steep Gaussian fall-off
 *
 * @param w [out] weights (array size of M)
 * @param M number of camera output levels
 * @param Mmin minimum registered camera output level
 * @param Mmax maximum registered camera output level
 * @param sigma sigma value for gaussian
 */
void weightsComposite( float* w, int M, int Mmin, int Mmax, float sigma );


/**
 * @brief Create gamma response function
 *
 * @param I [out] camera response function (array size of M)
 * @param M number of camera output levels
 */
void responseGamma( float* I, int M );


/**
 * @brief Create linear response function
 *
 * @param I [out] camera response function (array size of M)
 * @param M number of camera output levels
 */
void responseLinear( float* I, int M );


/**
 * @brief Create logarithmic response function
 *
 * @param I [out] camera response function (array size of M)
 * @param M number of camera output levels
 */
void responseLog10( float* I, int M );


/**
 * @brief Save response curve to a MatLab file for further reuse
 *
 * @param file file handle to save response curve
 * @param I camera response function (array size of M)
 * @param M number of camera output levels
 * @param name matrix name for use in Octave or Matlab
 */
void responseSave( FILE* file, const float* I, int M, const char* name);


/**
 * @brief Save response curve to a MatLab file for further reuse
 *
 * @param file file handle to save response curve
 * @param w weights (array size of M)
 * @param M number of camera output levels
 * @param name matrix name for use in Octave or Matlab
 */
void weightsSave( FILE* file, const float* w, int M, const char* name);


/**
 * @brief Load response curve (saved with responseSave();)
 *
 * @param file file handle to load response curve
 * @param I [out] camera response function (array size of M)
 * @param M number of camera output levels
 * @return false means file has different output levels or is wrong for some other reason
 */
bool responseLoad( FILE* file, float* I, int M);


/**
 * @brief Load response curve (saved with responseSave();)
 *
 * @param file file handle to save response curve
 * @param w [out] weights (array size of M)
 * @param M number of camera output levels
 * @return false means file has different output levels or is wrong for some other reason
 */
bool weightsLoad( FILE* file, float* w, int M);


/**
 * @brief Fill missing (zero) values in the response curve
 *
 * The weighting function will be set to the minimum weight value for
 * the places where the response curve was filled in.
 *
 * @param I [in/out] camera response function (array size of M)
 * @param w [in/out] weighting function (array size of M)
 * @param M number of camera output levels
 * @return number of filled values in the response curve
 */
int responseFillGaps( float* I, float* w, int M );

#endif
