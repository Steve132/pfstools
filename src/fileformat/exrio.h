/**
 * @brief IO operations on OpenEXR file format
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
 * @author Grzegorz Krawczyk, <krawczyk@mpi-sb.mpg.de>
 *
 * $Id: exrio.h,v 1.1 2005/06/15 13:36:54 rafm Exp $
 */

#ifndef _EXR_IO_H_
#define _EXR_IO_H_

#include <array2d.h>
#include <ImfRgbaFile.h>


class OpenEXRReader
{
  Imf::RgbaInputFile* file;		/// OpenEXR file object
  Imath::Box2i dw;			/// data window
  
  int width, height;

public:
  OpenEXRReader(  const char* filename );
  ~OpenEXRReader();

  int getWidth() const
    {
      return width;
    }

  int getHeight() const
    {
      return height;
    }

  void readImage( pfs::Array2D *R, pfs::Array2D *G, pfs::Array2D *B );
};


class OpenEXRWriter
{
  char fileName[1024];
  
public:
  OpenEXRWriter( const char* filename );

  void writeImage( pfs::Array2D *R, pfs::Array2D *G, pfs::Array2D *B );
};

#endif
