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
 * $Id: ppmio.cpp,v 1.5 2009/05/25 19:24:49 rafm Exp $
 */

#include "ppmio.h"

extern "C" {
 #include <ppm.h>
}

#include <math.h>
#include <assert.h>

struct PPMData
{
    pixval maxPV;
    int formatP;
};

static inline float clamp( const float v, const float minV, const float maxV )
{
    if( v < minV ) return minV;
    if( v > maxV ) return maxV;
    return v;
}

PPMReader::PPMReader( const char *program_name, FILE *fh ) : fh(fh)
{
  pm_init(program_name, 0);
  
  data = new PPMData;

  ppm_readppminit(  fh, &width, &height, &data->maxPV, &data->formatP );
}

bool PPMReader::readImage( pfs::Array2D *X, pfs::Array2D *Y, pfs::Array2D *Z )
{
    pixel *ppmRow;
    ppmRow = ppm_allocrow( width );
    assert( ppmRow != NULL );

    float normalization_factor = 1.f / (float)data->maxPV;
    
    for( int y = 0; y < height; y++ ) { // For each row of the image

      ppm_readppmrow( fh, ppmRow, width, data->maxPV, data->formatP );

      for( int x = 0; x < width; x++ ) {
        (*X)(x,y) = (float)PPM_GETR(ppmRow[x]) * normalization_factor;
        (*Y)(x,y) = (float)PPM_GETG(ppmRow[x]) * normalization_factor;
        (*Z)(x,y) = (float)PPM_GETB(ppmRow[x]) * normalization_factor;
      }
      
    }
    ppm_freerow( ppmRow );

    int eofP;
    ppm_nextimage( fh, &eofP);

    return eofP!=0;
}

int PPMReader::getBitDepth()
{
  return (int)ceil( log2( (float)data->maxPV ) );
}



PPMReader::~PPMReader()
{
    delete data;
}


PPMWriter::PPMWriter( const char *program_name, FILE *fh, int bit_depth ) :
   fh(fh), bit_depth( bit_depth ) 
{
   pm_init(program_name, 0);
}


void PPMWriter::writeImage( pfs::Array2D *X, pfs::Array2D *Y, pfs::Array2D *Z )
{
    pixel *ppmRow;
    int width = X->getCols();
    int height =  X->getRows();
    ppmRow = ppm_allocrow( width );
    assert( ppmRow != NULL );

    const int max_val = (1<<bit_depth)-1;
    const float max_valf = max_val;
    ppm_writeppminit( fh, width, height, max_val, false );

    for( int y = 0; y < height; y++ ) { // For each row of the image

        for( int x = 0; x < width; x++ ) {
            PPM_ASSIGN( ppmRow[x],
                (pixval)( clamp( (*X)( x, y )*max_valf, 0.f, max_valf) ),
                (pixval)( clamp( (*Y)( x, y )*max_valf, 0.f, max_valf) ),
                (pixval)( clamp( (*Z)( x, y )*max_valf, 0.f, max_valf) ) );
        }
        ppm_writeppmrow( fh, ppmRow, width, max_val, false );

    }
    ppm_freerow( ppmRow );

}
