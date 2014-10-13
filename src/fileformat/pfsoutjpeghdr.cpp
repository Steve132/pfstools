/**
 * @brief Write files in JPEG-HDR format
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
 * @author Jan Otop, <jotop@mpi-sb.mpg.de>
 *
 * $Id: pfsoutjpeghdr.cpp,v 1.4 2005/11/04 08:54:27 rafm Exp $
 */

#include <config.h>

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <pfs.h>
#include <getopt.h>


extern "C" {
#include <jpeghdr.h>
}

#define PROG_NAME "pfsoutjpeghdr"

#define PFMEOL "\x0a"

class QuietException 
{
};

#define min(x,y) ( (x)<(y) ? (x) : (y) )

void printHelp()
{
  fprintf( stderr, PROG_NAME " [--quality] [--correct] [--verbose] [--help]\n"
    "See man page for more information.\n" );
}

void writeHDRJPEGGray( FILE *fh, int width, int height,
  float *Y, short quality, JHCorrMethod correction  )
{
   jpeghdr_compress_struct jhinf;
   struct jpeg_error_mgr   jerr;
   float *image;

   jhinf.cinfo.err = jpeg_std_error(&jerr);
   // Reassign error handling functions as desired
   jpeghdr_create_compress(&jhinf);
   jhinf.quality = quality;
   jhinf.correction = correction;
   jpeg_stdio_dest(&jhinf.cinfo, fh);
   
   image = (JHSAMPLE *)malloc(width * height *
          sizeof(JHSAMPLE)*3);
   
   for(int i=0;i<width * height;i++)
   {
      image[i*3    ] = Y[i];
      image[i*3 + 1] = Y[i];
      image[i*3 + 2] = Y[i];
   }     
  
   jpeghdr_src_floatRGB(&jhinf, image, width, height);

   // Override jhinf.hdr.h_stride and jhinf.hdr.v_stride if non-standard.
   // Assign jhinf.samp2nits appropriately.
   // Reset jhinf.quality and jhinf.correction if desired.
   // Assign jhinf.alpha and jhinf.beta to modify saturation.
   // Change other JPEG encoding defaults as desired.
   // Assign gamma for tone-mapping if non-standard.
   jpeghdr_tonemap_default(&jhinf);
   // Or, assign jhinf.tmi 8-bit grayscale values in scanline order
   jpeghdr_do_compress(&jhinf);
   jpeghdr_destroy_compress(&jhinf);
   fclose(fh);
}

void writeHDRJPEGRGB( FILE *fh, int width, int height,
  float *R, float *G, float *B, short quality, JHCorrMethod correction,
  float alpha, float beta )
{
   jpeghdr_compress_struct jhinf;
   struct jpeg_error_mgr   jerr;
   float *image;

   jhinf.cinfo.err = jpeg_std_error(&jerr);
   // Reassign error handling functions as desired
   jpeghdr_create_compress(&jhinf);
   jhinf.quality = quality;
   jhinf.correction = correction;
   jhinf.alpha = alpha;
   jhinf.beta = beta;
   jpeg_stdio_dest(&jhinf.cinfo, fh);
   
   image = (JHSAMPLE *)malloc(width * height *
          sizeof(JHSAMPLE)*3);
   
   for(int i=0;i<width * height;i++)
   {
      image[i*3    ] = R[i];
      image[i*3 + 1] = G[i];
      image[i*3 + 2] = B[i];
   }     
   
   jpeghdr_src_floatRGB(&jhinf, image, width, height);

   // Override jhinf.hdr.h_stride and jhinf.hdr.v_stride if non-standard.
   // Assign jhinf.samp2nits appropriately.
   // Reset jhinf.quality and jhinf.correction if desired.
   // Assign jhinf.alpha and jhinf.beta to modify saturation.
   // Change other JPEG encoding defaults as desired.
   // Assign gamma for tone-mapping if non-standard.
   jpeghdr_tonemap_default(&jhinf);
   
   // Or, assign jhinf.tmi 8-bit grayscale values in scanline order
   jpeghdr_do_compress(&jhinf);
   jpeghdr_destroy_compress(&jhinf);

   free( image );
}

void writeFrames( int argc, char* argv[] )
{
  bool verbose = false;
  short quality = 90;
  float alpha = 1, beta = 1;
  JHCorrMethod correction = JHprecorr;
  
  // Parse command line parameters
  static struct option cmdLineOptions[] = {
    { "help", no_argument, NULL, 'h' },
    { "verbose", no_argument, NULL, 'v' },
    { "linear", no_argument, NULL, 'l' },
    { "quality", required_argument, NULL, 'q' },
    { "alpha", required_argument, NULL, 'a' },
    { "beta", required_argument, NULL, 'b' },
    { "correction", required_argument, NULL, 'c' },
    { NULL, 0, NULL, 0 }
  };
  static const char optstring[] = "hlvq:c:a:b:";

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
    case 'q':
      quality = strtol( optarg, NULL, 10 );
      break;
    case 'a':
      alpha = strtof( optarg, NULL );
      break;
    case 'b':
      beta = strtof( optarg, NULL );
      break;
    case 'c':
      if( !strcmp( optarg, "precorrect" ) )
        correction = JHprecorr;
      else if( !strcmp( optarg, "postcorrect" ) )
        correction = JHpostcorr;
      else if( !strcmp( optarg, "fullsamp" ) )
        correction = JHfullsamp;
      else
        throw pfs::Exception( "wrong correction method. Available methods: precorrect, postcorrect, fullsamp" );      
      break;
    case '?':
      throw QuietException();
    case ':':
      throw QuietException();
    }
  }
 
  pfs::DOMIO pfsio;

  VERBOSE_STR << "JPEG-HDR quality: " << quality << "\n";
  VERBOSE_STR << "color saturaction corection alpha: " << alpha << "\n";
  VERBOSE_STR << "color saturaction corection beta: " << beta << "\n";
  
  bool firstFrame = true;
 
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
    
    VERBOSE_STR << "writing file (HDR) '" << ff.fileName << "'" << std::endl;

    pfs::Channel *X, *Y, *Z;
    frame->getXYZChannels( X, Y, Z );
    if( X != NULL ) {       // Has color
      pfs::Channel *R= NULL, *G = NULL, *B = NULL; // for clarity of the code
      R = X;
      G = Y;
      B = Z;
      pfs::transformColorSpace( pfs::CS_XYZ, X, Y, Z,
        pfs::CS_RGB, R, G, B );
      
      writeHDRJPEGRGB(ff.fh, frame->getWidth(), frame->getHeight(),
        R->getRawData(), G->getRawData(), B->getRawData(), quality, correction,
        alpha, beta );
    } else {
      Y = frame->getChannel( "Y" );
      if( Y == NULL )
        throw pfs::Exception( "Can not find color or grayscale channels in the pfs stream" );
      writeHDRJPEGGray( ff.fh, frame->getWidth(), frame->getHeight(),
        Y->getRawData(), quality, correction );      
    }
    
    it.closeFrameFile( ff );
    pfsio.freeFrame( frame );
  }  
}


int main( int argc, char* argv[] )
{
  try {
    writeFrames( argc, argv );
  }
  catch( pfs::Exception ex ) {
    fprintf( stderr, PROG_NAME " error: %s\n", ex.getMessage() );
    return EXIT_FAILURE;
  }
  catch( QuietException  ex ) {
    return EXIT_FAILURE;
  }
  
  return EXIT_SUCCESS;
}
