/**
 * @brief 
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
 * $Id: ppmio.h,v 1.5 2009/05/25 19:24:49 rafm Exp $
 */

#ifndef PPMIO_H
#define PPMIO_H

#include <stdio.h>
#include <array2d.h>

struct PPMData;

class PPMReader 
{
    FILE *fh;
    int width, height;
    PPMData *data;
public:
    PPMReader( const char *program_name, FILE *fh );    
    ~PPMReader();

    int getWidth() const 
        {
            return width;
        }
    int getHeight() const
        {
            return height;
        }

     int getBitDepth();

     bool readImage( pfs::Array2D *X, pfs::Array2D *Y, pfs::Array2D *Z );

 };

 class PPMWriter
 {
     FILE *fh;
     const int bit_depth;
     
 public:
     PPMWriter( const char *program_name, FILE *fh, int bit_depth = 8 );
    
     void writeImage( pfs::Array2D *X, pfs::Array2D *Y, pfs::Array2D *Z );
    
};



#endif
