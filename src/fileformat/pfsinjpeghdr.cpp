/**
 * @brief Read files in JPEG-HDR format
 * 
 * This file is a part of PFSTOOLS package.
 * ---------------------------------------------------------------------- 
 * Copyright (C) 2003-2005 Rafal Mantiuk and Grzegorz Krawczyk
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
 * $Id: pfsinjpeghdr.cpp,v 1.3 2005/11/04 08:54:27 rafm Exp $
 */

#include <config.h>

#include <iostream>

#include <stdio.h>
#include <pfs.h>
#include <getopt.h>
#include <stdlib.h>
#include <math.h>

#define PROG_NAME "pfsinhdrjpeg"


extern "C" {
#include <jpeghdr.h>
}
/* #include "jhcomp.c" 
#include "jhdecomp.c"  
#include "jhresamp.c"  
#include "jhtonemap.c"*/


class QuietException 
{
};

void printHelp()
{
  fprintf( stderr, PROG_NAME " [--verbose] [--help]\n"
    "See man page for more information.\n" );
}


void readFrames( int argc, char* argv[] )
{
  pfs::DOMIO pfsio;
  
  JHSAMPLE   *hdrscan;
  JSAMPLE    *ldrscan;
  
  jpeghdr_decompress_struct jhinf;
  struct jpeg_error_mgr     jerr;

  bool verbose = false;

  // Parse command line parameters
  static struct option cmdLineOptions[] = {
    { "help", no_argument, NULL, 'h' },
    { "verbose", no_argument, NULL, 'v' },
    { "linear", no_argument, NULL, 'l' },
    { NULL, 0, NULL, 0 }
  };
  static const char optstring[] = "";
    
  pfs::FrameFileIterator it( argc, argv, "rb", NULL, stdin,
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
      std::cerr << PROG_NAME << " warning: linearize option ignored for an HDR input!"
                << std::endl;
      break;
    case '?':
      throw QuietException();
    case ':':
      throw QuietException();
    }
  }

 
  while( true )
  {
    pfs::FrameFile ff = it.getNextFrameFile();
    if( ff.fh == NULL ) break; // No more frames

    jhinf.cinfo.err = jpeg_std_error(&jerr);
    // Reassign error handling functions as desired
    jpeghdr_create_decompress(&jhinf);
    jpeg_stdio_src(&jhinf.cinfo, ff.fh);

    VERBOSE_STR << "reading file '" << ff.fileName << "'" << std::endl;
    
    pfs::Frame *frame;
     
    switch (jpeghdr_read_header(&jhinf)) {
    case JPEG_HEADER_HDR:  // HDR image          
      jpeghdr_start_decompress(&jhinf);
      frame = pfsio.createFrame( jhinf.cinfo.output_width, jhinf.cinfo.output_height );
      hdrscan = (JHSAMPLE *)malloc(jhinf.cinfo.output_width * jhinf.cinfo.output_height *
        sizeof(JHSAMPLE)*3);
      // Important: test jhinf.output_scanline, not jhinf.cinfo
      int index ;
      index = 0;
      while (jhinf.output_scanline < jhinf.cinfo.output_height) {
        jpeghdr_read_scanline(&jhinf, (JHSAMPLE *)hdrscan + index );
        index += jhinf.cinfo.output_width * 3; 
      }
       
      pfs::Channel *X, *Y, *Z;
      frame->createXYZChannels( X, Y, Z );
      //printf("bbb");
      //exit(0);
      for(int i=0;i<jhinf.cinfo.output_width * jhinf.cinfo.output_height;i++)
      {
        (X->getRawData())[i] = hdrscan[3*i];
        (Y->getRawData())[i] = hdrscan[3*i + 1];
        (Z->getRawData())[i] = hdrscan[3*i + 2];
      }       
      pfs::transformColorSpace( pfs::CS_RGB, X, Y, Z, pfs::CS_XYZ, X, Y, Z );	  
      free((void *)hdrscan);
      break;
    case JPEG_HEADER_OK:  // LDR image
    case JPEG_SUSPENDED:
    case JPEG_HEADER_TABLES_ONLY:
    default:
      throw pfs::Exception( "Can handle only JPEG-HDR images" );
      break;
    }
    it.closeFrameFile( ff );
    
    frame->getTags()->setString("LUMINANCE", "RELATIVE");

    const char *fileNameTag = strcmp( "-", ff.fileName )==0 ? "stdin" : ff.fileName;
    frame->getTags()->setString( "FILE_NAME", fileNameTag );

    jpeghdr_destroy_decompress(&jhinf);
    
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
    fprintf( stderr, PROG_NAME " error: %s\n", ex.getMessage() );
    return EXIT_FAILURE;
  }
  catch( QuietException  ex ) {
    return EXIT_FAILURE;
  }
  
  return EXIT_SUCCESS;
}
