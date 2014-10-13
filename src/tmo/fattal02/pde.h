/**
 * @file pde.h
 * @brief Solving Partial Differential Equations
 *
 * Full Multigrid Algorithm and Successive Overrelaxation.
 *
 * @author Grzegorz Krawczyk
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
 * $Id: pde.h,v 1.4 2012/03/31 16:49:49 tk255 Exp $
 */

#ifndef _fmg_pde_h_
#define _fmg_pde_h_

#include "pfstmo.h"

/// limit of iterations for successive overrelaxation
#define SOR_MAXITS 5001

/**
 * @brief solve pde using full multrigrid algorithm
 *
 * @param F array with divergence
 * @param U [out] solution
 */
void solve_pde_multigrid(pfstmo::Array2D *F, pfstmo::Array2D *U);

/**
 * @brief solve pde using successive overrelaxation
 *
 * @param F array with divergence
 * @param U [out] solution
 * @param maxits limit of iterations
 */
void solve_pde_sor(pfstmo::Array2D *F, pfstmo::Array2D *U, int maxits=SOR_MAXITS);


/**
 * @brief solve poisson pde (Laplace U = F) using discrete cosine transform
 *
 * @param F array of the right hand side (contains div G in this example)
 * @param U [out] solution
 * @param adjust_bound, adjust boundary values of F to make pde solvable 
 */
void solve_pde_fft(pfstmo::Array2D *F, pfstmo::Array2D *U, bool adjust_bound=false);

/**
 * @brief returns the residual error of the solution U, ie norm(Laplace U - F) 
 *
 * @param F [in] right hand side
 * @param U [in] solution
 */
float residual_pde(pfstmo::Array2D *U, pfstmo::Array2D *F);

/**
 * @brief component wise error estimate of the fft solver given an image size
 *
 * @param width
 * @param height
 */
double error_estim_pde_fft(unsigned int width, unsigned int height);
double error_estim_pde_fft_d(unsigned int width, unsigned int height);


#endif

