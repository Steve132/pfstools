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
 * $Id: exrio.cpp,v 1.1 2005/06/15 13:36:54 rafm Exp $
 */

#include <config.h>

#include <iostream>

#include <math.h>
#include <assert.h>

#include <pfs.h>
#include "exrio.h"

using namespace Imf;
using namespace Imath;


OpenEXRReader::OpenEXRReader( const char* filename )
{
  //--- read image
  file = new RgbaInputFile(filename);
  dw = file->dataWindow();

  width  = dw.max.x - dw.min.x + 1;
  height = dw.max.y - dw.min.y + 1;
  
  if( width*height<=0 )
  {
    throw pfs::Exception("EXR: illegal image size");
  }

  DEBUG_STR << "OpenEXR file \"" << filename << "\" ("
	    << width << "x" << height << ")" << endl;
}

void OpenEXRReader::readImage( pfs::Array2D *R, pfs::Array2D *G,
			       pfs::Array2D *B )
{
  assert(file!=NULL);
  DEBUG_STR << "Reading OpenEXR file... " << endl;
  
  Imf::Rgba* tmp_img = new Imf::Rgba[width*height];

  assert( dw.min.x - dw.min.y * width<=0 );
  file->setFrameBuffer(tmp_img - dw.min.x - dw.min.y * width, 1, width);
  // read image to memory
  file->readPixels(dw.min.y, dw.max.y);

  // check if supplied matrixes have the same size as the image
  if( R->getCols()!=width || R->getRows()!=height ||
      G->getCols()!=width || G->getRows()!=height ||
      B->getCols()!=width || B->getRows()!=height )
  {
    throw pfs::Exception("EXR: matrixes have different size than image");
  }

  int idx=0;
  for( int y=0 ; y<height ; y++ )
    for( int x=0 ; x<width ; x++ )
    {
      (*R)(x,y) = tmp_img[idx].r;
      (*G)(x,y) = tmp_img[idx].g;
      (*B)(x,y) = tmp_img[idx].b;
      idx++;
    }
  delete tmp_img;
}

OpenEXRReader::~OpenEXRReader()
{
  delete file;
  file==NULL;
}

OpenEXRWriter::OpenEXRWriter(const char* filename)
{
  strcpy(fileName, filename);
}

void OpenEXRWriter::writeImage( pfs::Array2D *R, pfs::Array2D *G,
				pfs::Array2D *B )
{
  // image size
  int width = R->getCols();
  int height = R->getRows();

  Imf::Rgba* tmp_img = new Imf::Rgba[width*height];
  int idx=0;
  for( int y=0 ; y<height ; y++ )
    for( int x=0 ; x<width ; x++ )
    {
      tmp_img[idx].r = (*R)(x,y);
      tmp_img[idx].g = (*G)(x,y);
      tmp_img[idx].b = (*B)(x,y);
      tmp_img[idx].a = 1.0f;
      idx++;
    }

  try
  {
    RgbaOutputFile file( fileName, width, height, WRITE_RGBA );
    file.setFrameBuffer( tmp_img, 1, width );
    file.writePixels( height );
  }
  catch (const std::exception &exc)
  {
    throw pfs::Exception( exc.what() );
  }

  delete tmp_img;
}
