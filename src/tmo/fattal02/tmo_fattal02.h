/**
 * @file tmo_fattal02.h
 * @brief TMO: Gradient Domain High Dynamic Range Compression (header)
 *
 * Implementation of Gradient Domain High Dynamic Range Compression
 * by Raanan Fattal, Dani Lischinski, Michael Werman.
 *
 * 
 * This file is a part of PFSTMO package.
 * ---------------------------------------------------------------------- 
 * Copyright (C) 2003,2004 Grzegorz Krawczyk
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
 * @author Grzegorz Krawczyk, <krawczyk@mpi-sb.mpg.de>
 *
 * $Id: tmo_fattal02.h,v 1.4 2012/03/31 16:49:49 tk255 Exp $
 */

#ifndef _tmo_fattal02_h_
#define _tmo_fattal02_h_

/**
 * @brief Gradient Domain High Dynamic Range Compression
 *
 * Implementation of Gradient Domain High Dynamic Range Compression
 * by Raanan Fattal, Dani Lischinski, Michael Werman.
 *
 * @param width image width
 * @param height image height
 * @param Y [in] image luminance values
 * @param L [out] tone mapped values
 * @param alfa parameter alfa (refer to the paper)
 * @param beta parameter beta (refer to the paper)
 * @param noise gradient level of noise (extra parameter)
 * @param cut_min percentile cutoff luminosity to be excluded from final image
 * @param cut_max percentile cutoff luminosity to be excluded from final image
 * @param fftsolver whether to use the fft-solver instead of the multi-grid
 */

void tmo_fattal02(unsigned int width, unsigned int height,
                  const float* nY, float* nL, float alfa, float beta,
                  float gamma, float noise, int detail_level,
                  float black_point, float white_point, bool fftsolver);

#endif
