#ifndef OCTAVE_UTIL_H
#define OCTAVE_UTIL_H

/**
 * @brief Utility functions for interfacing pfs::Array2D with Octave
 * 
 * This file is a part of PFSTOOLS package.
 * ---------------------------------------------------------------------- 
 * Copyright (C) 2003,2004 Rafal Mantiuk and Grzegorz Krawczyk
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
 * @author Rafal Mantiuk, <mantiuk@mpi-sb.mpg.de>
 *
 * $Id: octave_util.h,v 1.1 2005/06/15 13:36:54 rafm Exp $
 */

#include <octave/oct.h>
#include <array2d.h>

inline void copy( const Matrix &in, pfs::Array2D *out ) 
{
    int index = 0;
    for( int r = 0; r < in.rows(); r++ ) 
        for( int c = 0; c < in.cols(); c++ ) {
            (*out)(index++) = (float)in(r,c);
        }    
}

inline void copy( const pfs::Array2D *in, Matrix &out )
{
    int index = 0;
    for( int r = 0; r < in->getRows(); r++ ) 
        for( int c = 0; c < in->getCols(); c++ ) {
            out(r,c) = (double)(*in)(index++);
        }
}



#endif

