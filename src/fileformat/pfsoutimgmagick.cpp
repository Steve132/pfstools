/**
 * @file
 * @brief Writing frames using ImageMagick library
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
 * $Id: pfsoutimgmagick.cpp,v 1.7 2011/03/15 08:47:24 rafm Exp $
 */

#include <config.h>


#include <Magick++.h>

#include <iostream>

#include <getopt.h>
#include <pfs.h>

#define PROG_NAME "pfsoutimgmagick"

class QuietException 
{
};

void printHelp()
{
  std::cerr << PROG_NAME " [--linear] [--quality] [--bit-depth] [--verbose] [--help]" << std::endl
            << "See man page for more information." << std::endl;
}

template<class T>
inline T clamp( const T v, const T minV, const T maxV )
{
    if( v < minV ) return minV;
    if( v > maxV ) return maxV;
    return v;
}

void writeFrames( int argc, char* argv[] )
{
  pfs::DOMIO pfsio;

  bool verbose = false;
  bool opt_srgb=false;
  int quality = 75;
  int opt_bit_depth = -1;
  
  // Parse command line parameters
  static struct option cmdLineOptions[] = {
    { "help", no_argument, NULL, 'h' },
    { "verbose", no_argument, NULL, 'v' },
    { "srgb", no_argument, NULL, 's' },
    { "quality", required_argument, NULL, 'q' },
    { "bit-depth", required_argument, NULL, 'b' },
    { NULL, 0, NULL, 0 }
  };
  static const char optstring[] = "shvq:b:";
    
  pfs::FrameFileIterator it( argc, argv, "wb", NULL, NULL,
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
    case 'q':
      quality = strtol( optarg, NULL, 10 );
      break;
    case 'b':
      opt_bit_depth = strtol( optarg, NULL, 10 );
      break;
    case 's':
      opt_srgb = true;
      break;
    case '?':
      throw QuietException();
    case ':':
      throw QuietException();
    }
  }

  if( quality < 1 || quality > 100 )
      throw pfs::Exception( "'quality' argument must be within 1-100 range" );

  if( opt_bit_depth != -1 && (opt_bit_depth < 8 || opt_bit_depth > 32 ) )
      throw pfs::Exception( "'bit-depth' argument must be within 8-32 range" );
  
  VERBOSE_STR << "quality: " << quality << "%\n";
  
  while( true ) {
    pfs::Frame *frame = pfsio.readFrame( stdin );
    if( frame == NULL ) {
      break;
    }        

    pfs::FrameFile ff = it.getNextFrameFile();
    if( ff.fh == NULL ) {
      pfsio.freeFrame( frame );
      break; // No more frames
    }
    it.closeFrameFile( ff );
    
    pfs::Channel *X, *Y, *Z, *alpha;
        
    frame->getXYZChannels( X, Y, Z );
    if( X == NULL )         // No color
    {
      Y = frame->getChannel( "Y" );
      if( Y == NULL )       // No luminance
        throw pfs::Exception( "Missing X, Y, Z channels in the PFS stream" );
      // Grey levels
      X = frame->createChannel( "X" );
      Z = frame->createChannel( "Z" );
      pfs::transformColorSpace( pfs::CS_RGB, Y, Y, Y, pfs::CS_XYZ, X, Y, Z );
    }

    alpha = frame->getChannel( "ALPHA" );
    if( alpha == NULL )
      alpha = frame->getChannel( "A" );
        
    const char* luminanceTag = frame->getTags()->getString("LUMINANCE");
    if( luminanceTag!=NULL && strcmp(luminanceTag,"ABSOLUTE")==0 )
        std::cerr << PROG_NAME << " warning: This file format cannot store absolute luminance values\n";    
    if( opt_srgb )
    {
      if( luminanceTag!=NULL && strcmp(luminanceTag,"DISPLAY")==0 )
        std::cerr << PROG_NAME << " warning: This image seems to be display referred thus there is no need for applying the sRGB non-linearity\n";
      pfs::transformColorSpace( pfs::CS_XYZ, X, Y, Z, pfs::CS_SRGB, X, Y, Z );        
      VERBOSE_STR << "writing file (sRGB corrected) '" << ff.fileName << "'" << std::endl;
    }
    else
    {      
      pfs::transformColorSpace( pfs::CS_XYZ, X, Y, Z, pfs::CS_RGB, X, Y, Z );        
      VERBOSE_STR << "writing file (linear) '" << ff.fileName << "'" << std::endl;
    }        

    // Determine bit-depth for writing images (bitdepth == -1 for default)
    int bitdepth = -1;
    if( opt_bit_depth != -1 )
      bitdepth = opt_bit_depth;
    else {      
      const char* bitDepthTag = frame->getTags()->getString("BITDEPTH");
      if( bitDepthTag!=NULL ) {
        bitdepth=strtol( bitDepthTag, NULL, 10 );
        if( bitdepth < 8 )
          bitdepth = 8;
        else if( bitdepth > 16 )
          bitdepth = 16;
      }
    }
    
    {
      // Copy image to array that can be accepted by ImageMagick
      const int pixelCount = frame->getWidth()*frame->getHeight();
      unsigned short *imgBuffer =
        new unsigned short[pixelCount*(alpha == NULL ? 3 : 4)];
      int i = 0;
      float maxValue = (float)(1<<16) -1;
      for( int pix = 0; pix < pixelCount; pix++ ) {
        imgBuffer[i++] = (unsigned short)(clamp((*X)(pix),0.f,1.f)*maxValue);
        imgBuffer[i++] = (unsigned short)(clamp((*Y)(pix),0.f,1.f)*maxValue);
        imgBuffer[i++] = (unsigned short)(clamp((*Z)(pix),0.f,1.f)*maxValue);
        if( alpha != NULL )
          imgBuffer[i++] = (unsigned short)(maxValue-clamp((*alpha)(pix),0.f,1.f)*maxValue);
      }
      Magick::Image imImage( frame->getWidth(), frame->getHeight(),
        (alpha == NULL ? "RGB" : "RGBA"), Magick::ShortPixel, imgBuffer );
      imImage.quality( quality );
      if( bitdepth != -1 )
        imImage.depth( bitdepth );
      imImage.write( ff.fileName );
      delete[] imgBuffer;  
    }
    pfsio.freeFrame( frame );
        
  }    
}


int main( int argc, char* argv[] )
{
  try {
    writeFrames( argc, argv );
  }
  catch( pfs::Exception ex ) {
    std::cerr << PROG_NAME << " error: " << ex.getMessage() << std::endl;
    return EXIT_FAILURE;
  }        
  catch( Magick::Exception &ex ) { //This is comming from ImageMagick
    std::cerr << PROG_NAME << " error: " << ex.what() << std::endl;
    return EXIT_FAILURE;
  }
  catch( QuietException  ex ) {
    return EXIT_FAILURE;
  }        
  return EXIT_SUCCESS;
}
