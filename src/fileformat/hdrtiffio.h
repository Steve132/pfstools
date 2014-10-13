/**
 * @brief IO operations on High Dynamic Range TIFF file format
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
 * $Id: hdrtiffio.h,v 1.3 2007/03/13 09:25:49 gkrawczyk Exp $
 */

#ifndef _HDRTIFFIO_H_
#define _HDRTIFFIO_H_

#include <tiffio.h>
#include <array2d.h>


class HDRTiffReader
{
  TIFF* tif;
  uint32 width, height;

  uint16 comp;                  /// compression type
  uint16 phot;                  /// type of photometric data
  enum {FLOAT, WORD, BYTE, GRAYSCALE16} TypeOfData;
  uint16 bps;                   /// bits per sample
  uint16 nSamples;              /// number of channels in tiff file (only 1-3 are used)
  double stonits;               /// scale factor to get nit values

  bool exponential_mode;        /// if true, grayscale data come from LarsIII camera
  bool relative_values;         /// if true, values are linearized (hdr data)
  bool xyz_colorspace;          /// if true, values are in XYZ colorspace

  char format_string[255];       /// for verbose output 
public:
  HDRTiffReader(  const char* filename );
  ~HDRTiffReader();

  int getWidth() const
	{
		return width;
	}

  int getHeight() const
	{
		return height;
	}

  void readImage( pfs::Array2D *X, pfs::Array2D *Y, pfs::Array2D *Z );

  void setExponentialMode()
    { exponential_mode = true; relative_values=true; }

  const char* getFormatString()
    { return format_string; }

  bool isRelative()
    { return relative_values; }
  
  bool isColorspaceXYZ()
    { return xyz_colorspace; }
};


class HDRTiffWriter
{
  FILE *file;
public:
  HDRTiffWriter( FILE *fh ) : file(fh)
	{
	}

  void writeImage( pfs::Array2D *X, pfs::Array2D *Y, pfs::Array2D *Z );
};

#endif
