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
 * $Id: hdrtiffio.cpp,v 1.7 2014/01/03 12:52:20 rafm Exp $
 */

#include <config.h>

#include <iostream>

#include <math.h>
#include <assert.h>

#include <pfs.h>

#include "hdrtiffio.h"

using namespace std;

int TIFFWriteIFD(FILE* file, unsigned short tag, unsigned short type,
		 unsigned long count, unsigned long value);

//---
// HDR TIFF IO classes implementation

HDRTiffReader::HDRTiffReader( const char* filename )
{
  // default values for constants
  exponential_mode = false;
  relative_values = false;
  xyz_colorspace = false;

  // read header containing width and height from file
  tif = TIFFOpen(filename, "r");
  if( !tif )
    throw pfs::Exception("TIFF: could not open file for reading.");

  //--- image size
  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);

  if( width*height<=0 )
  {
    TIFFClose(tif);
    throw pfs::Exception("TIFF: illegal image size");
  }

  DEBUG_STR << "TIFF file \"" << filename << "\" ("
	    << width << "x" << height << ")" << endl;

	//--- image parameters
  if(!TIFFGetField(tif, TIFFTAG_COMPRESSION, &comp)) // compression type
    comp = COMPRESSION_NONE;

	// type of photometric data
  if(!TIFFGetFieldDefaulted(tif, TIFFTAG_PHOTOMETRIC, &phot))
    throw pfs::Exception("TIFF: unspecified photometric type");

  uint16 * extra_sample_types=0;
  uint16 extra_samples_per_pixel=0;
  switch(phot)
  {
    case PHOTOMETRIC_LOGLUV:
      DEBUG_STR << "Photometric data: LogLuv" << endl;
      if (comp != COMPRESSION_SGILOG && comp != COMPRESSION_SGILOG24)
      {
	TIFFClose(tif);
	throw pfs::Exception("TIFF: only support SGILOG compressed LogLuv data");
      }
      TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &nSamples);
      // set decoder to output in float XYZ
      TIFFSetField(tif, TIFFTAG_SGILOGDATAFMT, SGILOGDATAFMT_FLOAT);
      xyz_colorspace = true;
      TypeOfData = FLOAT;      
      strcpy(format_string,"linear LogLuv XYZ");
      relative_values=true;
      break;
    case PHOTOMETRIC_RGB:
      DEBUG_STR << "Photometric data: RGB" << endl;
     // read extra samples (# of alpha channels)
      if (TIFFGetField( tif, TIFFTAG_EXTRASAMPLES,
                        &extra_samples_per_pixel, &extra_sample_types )!=1)
      {
          extra_samples_per_pixel=0;
      }


      TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &nSamples);
      bps = nSamples - extra_samples_per_pixel;
      if (bps!=3)
      {
	TIFFClose(tif);
	throw pfs::Exception("TIFF: unsupported samples per pixel for RGB");
      }
      if (!TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bps) || (bps!=8 && bps!=16 && bps!=32))
      {
	TIFFClose(tif);
	throw pfs::Exception("TIFF: unsupported bits per sample for RGB");
      }

      if( bps==8 )
      {
	TypeOfData = BYTE;
	DEBUG_STR << "8bit per channel" << endl;
        strcpy(format_string,"linear 8bit RGB");
        relative_values=false; //!! TODO: verify if 8bit is always gamma corrected
      }
      else if( bps==16 )
      {
	TypeOfData = WORD;
	DEBUG_STR << "16bit per channel" << endl;
        strcpy(format_string,"linear 16bit RGB");
        relative_values=true;   //!! TODO: verify this case!!
      }
      else
      {
	TypeOfData = FLOAT;
	DEBUG_STR << "32bit float per channel" << endl;
        strcpy(format_string,"linear 32bit float RGB");
        relative_values=true; 
      }
      break;
    case PHOTOMETRIC_MINISBLACK: // HDR video camera format (grayscale)
      DEBUG_STR << "Photometric data: MINISBLACK (hdrv camera)" << endl;
      if (!TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &bps) || bps!=1)
      {
	TIFFClose(tif);
	throw pfs::Exception("TIFF: Unsupported samples per pixel for "
			     "grayscale image");
      }
      if (!TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bps) || bps!=16)
      {
	DEBUG_STR << "Detected bits per sample: " << bps << endl;
	TIFFClose(tif);
	throw pfs::Exception("TIFF: unsupported bits per sample for "
			     "grayscale image.");
      }
      TypeOfData = GRAYSCALE16;
      strcpy(format_string,"linear 16bit");
      relative_values=true;
      break;
    default:
      DEBUG_STR << "Unsupported photometric type: " << phot << endl;
      TIFFClose(tif);
      strcpy(format_string,"unknown");
      relative_values=false;
      throw pfs::Exception("TIFF: unsupported photometric type");
  }

  if (!TIFFGetField(tif, TIFFTAG_STONITS, &stonits))
    stonits = 1.;
}

void HDRTiffReader::readImage( pfs::Array2D *X, pfs::Array2D *Y, pfs::Array2D *Z )
{
  //--- scanline buffer with pointers to different data types
  union {
    float* fp;
    uint16* wp;
    uint8* bp;
    void* vp;
  } buf;

  //--- image length
  uint32 imagelength;
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &imagelength);
  DEBUG_STR << "Image length: " << imagelength << endl;

  //--- image scanline size
  uint32 scanlinesize = TIFFScanlineSize(tif);
  buf.vp = _TIFFmalloc(scanlinesize);
//    DEBUG_STR << "Scanline size: " << scanlinesize << endl;


  //--- read scan lines
  const int image_width = X->getCols();
  for(uint32 row = 0; row < imagelength; row++)
  {
    switch(TypeOfData)
    {
      case FLOAT:
	TIFFReadScanline(tif, buf.fp, row);
	for( int i=0; i < image_width; i++ )
	{
	  (*X)(i,row) = buf.fp[i*nSamples];
	  (*Y)(i,row) = buf.fp[i*nSamples+1];
	  (*Z)(i,row) = buf.fp[i*nSamples+2];
	}
	break;
      case WORD:
	TIFFReadScanline(tif, buf.wp, row);
	for( int i=0; i<image_width; i++ )
	{
	  (*X)(i,row) = buf.wp[i*nSamples]/65536.f;
	  (*Y)(i,row) = buf.wp[i*nSamples+1]/65536.f;
	  (*Z)(i,row) = buf.wp[i*nSamples+2]/65536.f;
	}
	break;
      case BYTE:
	TIFFReadScanline(tif, buf.bp, row);
	for( int i=0; i<image_width; i++ )
	{
	  (*X)(i,row) = buf.bp[i*nSamples]/255.0f;
	  (*Y)(i,row) = buf.bp[i*nSamples+1]/255.0f;
	  (*Z)(i,row) = buf.bp[i*nSamples+2]/255.0f;
// 	  (*X)(i,row) = pow( buf.bp[i*nSamples]/255.0, 2.2 );
// 	  (*Y)(i,row) = pow( buf.bp[i*nSamples+1]/255.0, 2.2 );
// 	  (*Z)(i,row) = pow( buf.bp[i*nSamples+2]/255.0, 2.2 );
//				(*X)(i,row) = buf.bp[i*3];
//				(*Y)(i,row) = buf.bp[i*3+1];
//				(*Z)(i,row) = buf.bp[i*3+2];
	}
	break;
      case GRAYSCALE16:
	TIFFReadScanline(tif, buf.wp, row);

        if( !exponential_mode )
        {
          for( int i=0; i<image_width ; i++ )
          {
            float lum = buf.wp[i];
            // D65 observer XYZ = (95.047,100,108.883);
            (*X)(i,row) = 0.95047f * lum;
            (*Y)(i,row) = lum;
            (*Z)(i,row) = 1.08883f * lum;
          }
        }
        else
        {
          //!! this is for exponential tiffs
          for( int i=0; i<image_width ; i++ )
          {
            static float pow2[] = {1,2,4,8,16,32,64,128,256,
                                   512,1024,2048,4096,8192,16384,32768};
            //--- 16bit value = a:4bit..b:12bit, lum = b*2^a
            float value = buf.wp[i] & 0x0fff;
            unsigned char expo = ( buf.wp[i] & 0xf000 ) >> 12;
            float lum = value*pow2[expo];
            (*X)(i,imagelength-row-1) = lum;
            (*Y)(i,imagelength-row-1) = lum;
            (*Z)(i,imagelength-row-1) = lum;
          }
        }
	break;
    }
  }

  //--- free buffers and close files
  _TIFFfree(buf.vp);
  TIFFClose(tif);
}

HDRTiffReader::~HDRTiffReader()
{
}

void HDRTiffWriter::writeImage( pfs::Array2D *X, pfs::Array2D *Y, pfs::Array2D *Z )
{
  // image size
  int width = X->getCols();
  int height = X->getRows();

  // Offset in tiff image
  unsigned long tiff_offset=0;

  //--- Tiff header
  unsigned char head[4];
  head[0] = 'I';	// byte order
  head[1] = 'I';
  head[2] = 0x2a;	// tiff version
  head[3] = 0x00;
  fwrite(head, sizeof(unsigned char), 4, file);
  tiff_offset+=4;

  //--- Tiff image data
  int rows_per_strip = 3;
  int no_of_strips = ((height+rows_per_strip-1) / rows_per_strip);
  unsigned long* strip_offsets = new unsigned long[no_of_strips];
  unsigned long* strip_byte_counts = new unsigned long[no_of_strips];

  //--- first IFD pointer
  // (67 - ifd values block)
  unsigned long first_ifd_offs = width*height*3+no_of_strips*4*2+67;

  fwrite(&first_ifd_offs, sizeof(unsigned long), 1, file);
  tiff_offset+=4;

  //--------------------------------------------------------------------
  //--- writing image data
  int strip_len = rows_per_strip*width*3;
  int bytes_count = 0;		//bytes to write
  int strip_no = 0;	// no of strip
  unsigned char* strip = new unsigned char[strip_len];

  for( int y=0 ; y<height ; y++ )
  {
    for( int x=0 ; x<width ; x++ )
    {
      //!! gamma correction performed in transformColorSpace in pfsouttiff.cpp
//       float r = pow( (*X)(x,y),1.0f/2.2f ) * 255.0f;
//       float g = pow( (*Y)(x,y),1.0f/2.2f ) * 255.0f;
//       float b = pow( (*Z)(x,y),1.0f/2.2f ) * 255.0f;
      float r = (*X)(x,y) * 255.0f;
      float g = (*Y)(x,y) * 255.0f;
      float b = (*Z)(x,y) * 255.0f;

      r = (r>0) ? ( (r<255.0f)?r:255.0f) : 0;
      g = (g>0) ? ( (g<255.0f)?g:255.0f) : 0;
      b = (b>0) ? ( (b<255.0f)?b:255.0f) : 0;

      strip[bytes_count++] = (unsigned char) r;
      strip[bytes_count++] = (unsigned char) g;
      strip[bytes_count++] = (unsigned char) b;
    }

    if( y%rows_per_strip == rows_per_strip-1 )
    {
      // write strip
      strip_offsets[strip_no] = tiff_offset;
      strip_byte_counts[strip_no] = bytes_count;
      fwrite(strip, sizeof(unsigned char), bytes_count, file);

      tiff_offset+=bytes_count;
      strip_no++;
      bytes_count=0;
    }
  }

  // write remaining strip
  if( bytes_count>0 )
  {
    // write strip
    strip_offsets[strip_no] = tiff_offset;
    strip_byte_counts[strip_no] = bytes_count;
    fwrite(strip, sizeof(unsigned char), bytes_count, file);

    tiff_offset+=bytes_count;
    strip_no++;
    bytes_count=0;
  }

  delete[] strip;
  //--------------------------------------------------------------------

  //--- Tiff data
  unsigned char software[] = "PFSTOOLS tiff io operations";
  unsigned long software_offs = tiff_offset;
  int software_len = 37;
  fwrite(software, sizeof(unsigned char), software_len, file);
  tiff_offset+=37;

  unsigned short bits_per_sample[3] = {8,8,8};
  unsigned long bits_per_sample_offs=tiff_offset;
  fwrite(bits_per_sample, sizeof(unsigned short), 3, file);
  tiff_offset+=3*2;

  unsigned long strip_byte_counts_offs = tiff_offset;
  fwrite(strip_byte_counts, sizeof(unsigned long), no_of_strips, file);
  tiff_offset+=no_of_strips*4;

  unsigned long strip_offsets_offs = tiff_offset;
  fwrite(strip_offsets, sizeof(unsigned long), no_of_strips, file);
  tiff_offset+=no_of_strips*4;

  unsigned long xresolution[] = { 72, 1 };		// cecha i mantysa
  unsigned long xresolution_offs = tiff_offset;
  fwrite(xresolution, sizeof(unsigned long), 2, file);
  tiff_offset+=2*4;

  unsigned long yresolution[] = { 72, 1 };		// cecha i mantysa
  unsigned long yresolution_offs = tiff_offset;
  fwrite(yresolution, sizeof(unsigned long), 2, file);
  tiff_offset+=2*4;

  //-- not used tiff data
  delete[] strip_offsets;
  delete[] strip_byte_counts;

  //--- check if offset is ok
  assert( first_ifd_offs==tiff_offset );

  //--- image file directory
  unsigned short ifd_count=15;	// ifd entry count
  fwrite(&ifd_count, sizeof(unsigned short), 1, file);
  tiff_offset+=2;

  // NewSubfileType: 0 - full resolution image
  TIFFWriteIFD(file, 0x00fe, 4, 1, 0);
  // ImageWidth
  TIFFWriteIFD(file, 0x0100, 4, 1, width);
  // ImageLength (Height)
  TIFFWriteIFD(file, 0x0101, 4, 1, height);
  // BitsPerSample
  TIFFWriteIFD(file, 0x0102, 3, 3, bits_per_sample_offs);
  // Compression: 1 - no compression
  TIFFWriteIFD(file, 0x0103, 3, 1, 1);
  // PhotometricInterpretation: 2 - RGB image
  TIFFWriteIFD(file, 0x0106, 3, 1, 2);
  // StripOffsets
  TIFFWriteIFD(file, 0x0111, 4, no_of_strips, strip_offsets_offs);
  // SamplesPerPixel: 3
  TIFFWriteIFD(file, 0x0115, 3, 1, 3);
  // RowsPerStrip
  TIFFWriteIFD(file, 0x0116, 4, 1, rows_per_strip);
  // StripByteCounts
  TIFFWriteIFD(file, 0x0117, 4, no_of_strips, strip_byte_counts_offs);
  // XResolution
  TIFFWriteIFD(file, 0x011a, 5, 1, xresolution_offs);
  // YResolution
  TIFFWriteIFD(file, 0x011b, 5, 1, yresolution_offs);
  // PlanarConfiguration: 1 - single plannar
  TIFFWriteIFD(file, 0x011c, 3, 1, 1);
  // ResolutionUnit: 2 - cm
  TIFFWriteIFD(file, 0x0128, 3, 1, 2);
  // Software
  TIFFWriteIFD(file, 0x0131, 2, software_len, software_offs);
  tiff_offset+=ifd_count*12;

  //--- second and the last IFD pointer
  unsigned long last_ifd_offs = 0;

  fwrite(&last_ifd_offs, sizeof(unsigned long), 1, file);
  tiff_offset+=4;
}


int TIFFWriteIFD(FILE* file, unsigned short tag, unsigned short type,
		 unsigned long count, unsigned long value)
{
  // image file directory flag
  unsigned char ifd[12];

  (unsigned short&)ifd[0] = tag;
  (unsigned short&)ifd[2] = type;
  (unsigned long&)ifd[4] = count;
  (unsigned long&)ifd[8] = 0;

  if( count>1 )
    (unsigned long&)ifd[8] = value;
  else
  {
    switch( type )
    {
      case 1:		// byte
	(unsigned char&)ifd[8] = (unsigned char)value;
	break;
      case 3:		// short
	(unsigned short&)ifd[8] = (unsigned short)value;
	break;
      case 5:		// rational
      case 4:		// long
      case 2:		// ascii
	(unsigned long&)ifd[8] = value;
	break;
    };
  }

  fwrite(ifd, sizeof(unsigned char), 12, file);
  return 0;
}
