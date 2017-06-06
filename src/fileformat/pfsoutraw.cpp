/**
 * @brief Write files in Radiance's RGBE format
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
 * @author Rafal Mantiuk, <mantiuk@mpi-sb.mpg.de>
 *
 * $Id: pfsoutrgbe.cpp,v 1.3 2014/06/17 21:57:09 rafm Exp $
 */

#include <config.h>

#include <cstdlib>
#include <math.h>

#include <iostream>

#include <getopt.h>
#include <pfs.h>


#define PROG_NAME "pfsoutraw"

class QuietException 
{
};

void printHelp()
{
  std::cerr << PROG_NAME " [--linear] [--verbose] [--radiance] [--quiet] [--help]" << std::endl
            << "See man page for more information." << std::endl;
}


typedef unsigned char Trgbe;
struct Trgbe_pixel
{
    /// @name RGB values and their exponent
    //@{
    Trgbe r;
    Trgbe g;
    Trgbe b;
    Trgbe e;
    //@}
    
};

void rgb2rgbe( float r, float g, float b, Trgbe_pixel& rgbe)
{
    //  r /= WHITE_EFFICACY;
    //  g /= WHITE_EFFICACY;
    //  b /= WHITE_EFFICACY;
    
    double v = r;	// max rgb value
    if( v < g)
        v = g;
    if( v < b )
        v = b;
    
    if( v < 1e-32 )
    {
        rgbe.r = rgbe.g = rgbe.b = rgbe.e = 0;
    }
    else
    {
        int e;	// exponent
        
        v = frexp(v,&e) * 256.0/v;
        rgbe.r = Trgbe( v*r );
        rgbe.g = Trgbe( v*g );
        rgbe.b = Trgbe( v*b );
        rgbe.e = Trgbe( e+128 );
    }
}



void writeImage( FILE *file, pfs::Array2D *X, pfs::Array2D *Y, pfs::Array2D *Z )
{
    int width = X->getCols();
    int height = X->getRows();
    
    
    
    if( Y->getCols() != width || Y->getRows() != height ||
       Z->getCols() != width || Z->getRows() != height )
    {
        throw pfs::Exception( "RGBE: RGB layers have different size");
    }
  
    
    int strip_len = width*4;
    int bytes_count = 0;		//bytes to write
    
    unsigned char* strip = new unsigned char[strip_len];
    
    for( int y=0 ; y<height ; y++ )
    {
        
        bytes_count=0;
        
        for( int x=0 ; x<width ; x++ )
        {
            
            Trgbe_pixel p;
            rgb2rgbe((*X)(x, y), (*Y)(x, y), (*Z)(x, y), p);
            
            
            strip[bytes_count++] = (unsigned char) p.r;
            strip[bytes_count++] = (unsigned char) p.g;
            strip[bytes_count++] = (unsigned char) p.b;
            strip[bytes_count++] = (unsigned char) p.e;
        }
        
        fwrite(strip, sizeof(unsigned char), bytes_count, file);
    }
}



void writeFrames( int argc, char* argv[] )
{
  pfs::DOMIO pfsio;

  bool verbose = false;
  bool radiance_compatibility = false;
  bool quiet = false;

  // Parse command line parameters
  static struct option cmdLineOptions[] = {
    { "help", no_argument, NULL, 'h' },
    { "verbose", no_argument, NULL, 'v' },
    { "linear", no_argument, NULL, 'l' },
  { "radiance", no_argument, NULL, 'r' },
  { "quiet", no_argument, NULL, 'q' },
  { NULL, 0, NULL, 0 }
  };
  static const char optstring[] = "lhvrq";
    
  pfs::FrameFileIterator it( argc, argv, "wb", NULL, stdout,
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
    case 'l':
      std::cerr << PROG_NAME << " warning: linearize option ignored for an HDR output!"
                << std::endl;
      break;
  case 'r':
    radiance_compatibility = true;
    break;
  case 'q':
    quiet = true;
    break;
  case '?':
      throw QuietException();
    case ':':
      throw QuietException();
    }
  }

  if (!quiet && !radiance_compatibility)
    std::cerr << PROG_NAME << " warning: starting from pfstools 1.9.0, .hdr files are written without correcting for WHITE_EFFICIENCY,"
    " which makes the absolute values incompatible with Radiance and previos versions of pfstools but compatible with most software."
    " Use --radiance option to retain compatibility with Radiance and earlier versions of pfstools.  Use --quiet to suppress this message."
    " Check the manual pages for pfsoutrgbe for details." << std::endl;

  while( true ) {
    pfs::Frame *frame = pfsio.readFrame( stdin );
    if( frame == NULL ) {
      break; // No more frames
    }
    
    const char* luminanceTag = frame->getTags()->getString("LUMINANCE");
    if( luminanceTag!=NULL && 0==strcmp(luminanceTag,"DISPLAY") )
    {
      static bool show_once = false;
      if( !show_once )
      {
        std::cerr << PROG_NAME << " warning: "
                  << "display profiled content written to an HDR output" << std::endl;
        show_once=true;
      }
    }
    
    pfs::FrameFile ff = it.getNextFrameFile();
    if( ff.fh == NULL ) {
      pfsio.freeFrame( frame );
      break; // No more frames
    }
    

    //RGBEWriter writer( ff.fh, radiance_compatibility );       
        
    pfs::Channel *X, *Y, *Z;
    frame->getXYZChannels( X, Y, Z );    
    if( X == NULL ) {       // No color
      throw pfs::Exception( "Missing X, Y, Z channels in the PFS stream" );
    }

    VERBOSE_STR << "writing file (HDR) '" << ff.fileName << "'" << std::endl;
    pfs::transformColorSpace( pfs::CS_XYZ, X, Y, Z, pfs::CS_RGB, X, Y, Z );


    writeImage( ff.fh, X, Y, Z );

    pfsio.freeFrame( frame );
        
    it.closeFrameFile( ff );
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
  catch( QuietException  ex ) {
    return EXIT_FAILURE;
  }        
  return EXIT_SUCCESS;
}
