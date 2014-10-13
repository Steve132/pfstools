/**
 * @brief Read files using GDAL.
 * 
 * This file is a part of PFSTOOLS package.
 * ---------------------------------------------------------------------- 
 * Copyright (C) 2008 Martin Lambers <marlam@marlam.de>
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
 * @author Martin Lambers, <marlam@marlam.de>
 * 
 * $Id: pfsingdal.cpp,v 1.1 2008/05/15 00:03:56 rafm Exp $
 */

#include <config.h>

#include <cstdio>
#include <cstring>
#include <cfloat>
#include <sstream>
#include <iostream>

#include <getopt.h>
#include <pfs.h>

#include <gdal_priv.h>

#define PROG_NAME "pfsingdal"

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

class QuietException 
{
};

std::string stringify(double x)
{
  std::ostringstream o;
  o.precision( DBL_DIG );
  o << x;
  return o.str();
}

void printHelp()
{
  std::cerr << PROG_NAME " [--verbose] [--help]" << std::endl
            << "See man page for more information." << std::endl;
}



void readFrames( int argc, char* argv[] )
{
  pfs::DOMIO pfsio;

  bool verbose = false;
  
  // Parse command line parameters
  static struct option cmdLineOptions[] = {
    { "help", no_argument, NULL, 'h' },
    { "verbose", no_argument, NULL, 'v' },
    { NULL, 0, NULL, 0 }
  };
  static const char optstring[] = "hv";
    
  pfs::FrameFileIterator it( argc, argv, "rb", NULL, NULL,
    optstring, cmdLineOptions );
    
  int optionIndex = 0;
  while( 1 ) {
    int c = getopt_long (argc, argv, optstring, cmdLineOptions, &optionIndex);
    if( c == -1 ) break;
    switch( c ) {
    case 'h':
      printHelp();
      throw QuietException();
    case 'v':
      verbose = true;
      break;
    case '?':
      throw QuietException();
    case ':':
      throw QuietException();
    }
  }

  GDALAllRegister();

  GDALDataset *poDataset;
  GDALRasterBand *poBand;
  double adfGeoTransform[6];
  size_t nBlockXSize, nBlockYSize, nBands;
  int bGotMin, bGotMax;
  double adfMinMax[2];
  float *pafScanline;

  while( true ) {
    pfs::FrameFile ff = it.getNextFrameFile();
    if( ff.fh == NULL ) break; // No more frames
    it.closeFrameFile( ff );

    VERBOSE_STR << "reading file '" << ff.fileName << "'" << std::endl;
    if( !( poDataset = (GDALDataset *) GDALOpen( ff.fileName, GA_ReadOnly ) ) ) {
      std::cerr << "input does not seem to be in a format supported by GDAL" << std::endl;
      throw QuietException();
    }
    VERBOSE_STR << "GDAL driver: " << poDataset->GetDriver()->GetDescription()
	<< " / " << poDataset->GetDriver()->GetMetadataItem( GDAL_DMD_LONGNAME ) << std::endl;
    nBlockXSize = poDataset->GetRasterXSize();
    nBlockYSize = poDataset->GetRasterYSize();
    nBands = poDataset->GetRasterCount();
    VERBOSE_STR << "Data size " << nBlockXSize << "x" << nBlockYSize << "x" << nBands << std::endl;
    if( poDataset->GetProjectionRef() ) {
      VERBOSE_STR << "Projection " << poDataset->GetProjectionRef() << std::endl;
    }
    if( poDataset->GetGeoTransform( adfGeoTransform ) == CE_None ) {
      VERBOSE_STR << "Origin = (" << adfGeoTransform[0] << ", " << adfGeoTransform[3] << ")" << std::endl;
      VERBOSE_STR << "Pixel Size = (" << adfGeoTransform[1] << ", " << adfGeoTransform[5] << ")" << std::endl;
    }

    if( nBlockXSize==0  || nBlockYSize==0 
	    || ( SIZE_MAX / nBlockYSize < nBlockXSize ) 
	    || ( SIZE_MAX / (nBlockXSize * nBlockYSize ) < 4 ) ) {
      std::cerr << "input data has invalid size" << std::endl;
      throw QuietException();
    }
    if( !(pafScanline = (float *) CPLMalloc( sizeof(float) * nBlockXSize ) ) ) {
      std::cerr << "not enough memory" << std::endl;
      throw QuietException();
    }

    pfs::Frame *frame = pfsio.createFrame( nBlockXSize, nBlockYSize );
    pfs::Channel *C[nBands];
    char channel_name[32];

    frame->getTags()->setString( "X-GDAL_DRIVER_SHORTNAME", poDataset->GetDriver()->GetDescription() );
    frame->getTags()->setString( "X-GDAL_DRIVER_LONGNAME", poDataset->GetDriver()->GetMetadataItem( GDAL_DMD_LONGNAME ) );
    frame->getTags()->setString( "X-PROJECTION", poDataset->GetProjectionRef() );
    if( poDataset->GetGeoTransform( adfGeoTransform ) == CE_None ) {
      frame->getTags()->setString( "X-ORIGIN_X", stringify(adfGeoTransform[0]).c_str() );
      frame->getTags()->setString( "X-ORIGIN_Y", stringify(adfGeoTransform[3]).c_str() );
      frame->getTags()->setString( "X-PIXEL_WIDTH", stringify(adfGeoTransform[1]).c_str() );
      frame->getTags()->setString( "X-PIXEL_HEIGHT", stringify(adfGeoTransform[5]).c_str() );
    }

    for ( size_t band = 1; band <= nBands; band++) {
      size_t nBandXSize, nBandYSize;
      VERBOSE_STR << "Band " << band << ": " << std::endl;
      snprintf( channel_name, 32, "X-GDAL%zu", band );
      C[band - 1] = frame->createChannel( channel_name );
      poBand = poDataset->GetRasterBand( band );
      nBandXSize = poBand->GetXSize();
      nBandYSize = poBand->GetYSize();
      VERBOSE_STR << "    " << nBandXSize << "x" << nBandYSize << std::endl;
      if( nBandXSize != (int)nBlockXSize || nBandYSize != (int)nBlockYSize ) {
        std::cerr << "data in band " << band << " has different size" << std::endl;
        throw QuietException();
      }
      VERBOSE_STR << "    Type " << GDALGetDataTypeName( poBand->GetRasterDataType() ) << ", "
	  << "Color Interpretation " << GDALGetColorInterpretationName( poBand->GetColorInterpretation() ) << std::endl;
      adfMinMax[0] = poBand->GetMinimum( &bGotMin );
      adfMinMax[1] = poBand->GetMaximum( &bGotMax );
      if( ! (bGotMin && bGotMax) ) {
      	  GDALComputeRasterMinMax((GDALRasterBandH)poBand, TRUE, adfMinMax);
      }
      VERBOSE_STR << "    Min " << adfMinMax[0] << ", Max " << adfMinMax[1] << std::endl;
      C[band - 1]->getTags()->setString( "X-TYPE", 
	      GDALGetDataTypeName( poBand->GetRasterDataType() ) );
      C[band - 1]->getTags()->setString( "X-COLOR_INTERPRETATION", 
	      GDALGetColorInterpretationName( poBand->GetColorInterpretation() ) );
      C[band - 1]->getTags()->setString( "X-MIN", stringify(adfMinMax[0]).c_str() );
      C[band - 1]->getTags()->setString( "X-MAX", stringify(adfMinMax[1]).c_str() );
      for( size_t y = 0; y < nBlockYSize; y++ ) {
        if( poBand->RasterIO( GF_Read, 0, y, nBlockXSize, 1, pafScanline, 
		    nBlockXSize, 1, GDT_Float32, 0, 0) != CE_None ) {
          std::cerr << "input error" << std::endl;
          throw QuietException();
	}
	memcpy( C[band - 1]->getRawData() + y * nBlockXSize, pafScanline, nBlockXSize * sizeof(float) );
      }
    }
    CPLFree( pafScanline );
    GDALClose( poDataset );

    const char *fileNameTag = strcmp( "-", ff.fileName )==0 ? "stdin" : ff.fileName;
    frame->getTags()->setString( "FILE_NAME", fileNameTag );
        
    pfsio.writeFrame( frame, stdout );
    pfsio.freeFrame( frame );
  }
}


int main( int argc, char* argv[] )
{
  try {
    readFrames( argc, argv );
  }
  catch( pfs::Exception ex ) {
    std::cerr << PROG_NAME << " error: " << ex.getMessage() << std::endl;
    return EXIT_FAILURE;
  }        
  catch( QuietException  ex ) {
    return EXIT_FAILURE;
  }        
  return EXIT_SUCCESS;
}
