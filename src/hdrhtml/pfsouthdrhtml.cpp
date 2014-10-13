/**
 * @brief Create a web page with an HDR viewer
 * 
 * This file is a part of PFSTOOLS package.
 * ---------------------------------------------------------------------- 
 * Copyright (C) 2009 Rafal Mantiuk
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
 * $Id: pfsouthdrhtml.cpp,v 1.5 2010/06/13 14:45:55 rafm Exp $
 */

#include "hdrhtml.h"

#include <config.h>

#include <cstdlib>
#include <iostream>
#include <string>

#include <stdio.h>
#include <pfs.h>
#include <getopt.h>


#define PROG_NAME "pfsouthdrhtml"

class QuietException 
{
};

#define min(x,y) ( (x)<(y) ? (x) : (y) )

void printHelp()
{
  fprintf( stderr, PROG_NAME " [<html_page_name>] [--quality <1-5>] [--image-dir <directory_name>] [--page-template <template_file>] [--image-template <template_file>] [--object-output <file_name.js>] [--html-output <file_name.html>] [--verbose] [--help]\n"
    "See the manual page for more information.\n" );
}


void generate_hdrhtml( int argc, char* argv[] )
{
  bool verbose = false;
  int quality = 2;

  // Parse command line parameters
  static struct option cmdLineOptions[] = {
    { "help", no_argument, NULL, 'h' },
    { "verbose", no_argument, NULL, 'v' },
    { "quality", required_argument, NULL, 'q' },
    { "image-dir", required_argument, NULL, 'd' },
    { "object-output", required_argument, NULL, 'o' },
    { "html-output", required_argument, NULL, 'l' },
    { "page-template", required_argument, NULL, 'p' },
    { "image-template", required_argument, NULL, 'i' },
    { NULL, 0, NULL, 0 }
  };
  static const char optstring[] = "q:hvd:o:l:p:i:";
  const char *page_name = NULL;
  const char *image_dir = NULL;
  const char *object_output = NULL;
  const char *html_output = NULL;
  const char *page_template = PKG_DATADIR "/hdrhtml_default_templ/hdrhtml_page_templ.html";
  const char *image_template = PKG_DATADIR "/hdrhtml_default_templ/hdrhtml_image_templ.html";
  
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
      if( quality < 1 || quality > 5 )
        throw pfs::Exception( "The quality must be between 1 (worst) and 5 (best)." );
      break;
    case 'd':
      image_dir = optarg;
      break;
    case 'o':
      object_output = optarg;
      break;
    case 'p':
      page_template = optarg;
      break;
    case 'i':
      image_template = optarg;
      break;
    case 'l':
      html_output = optarg;
      break;
    case '?':
      throw QuietException();
    case ':':
      throw QuietException();
    }
  }

  if( optind < argc ) {
    page_name = argv[optind++];    
  }
  if( optind < argc ) {
    throw pfs::Exception( "Too many parameters." );     
  }
  
  pfs::DOMIO pfsio;
 
  bool first_frame = true;

  HDRHTMLSet image_set( page_name, image_dir );
  
  while( true ) {
    pfs::Frame *frame = pfsio.readFrame( stdin );
    if( frame == NULL ) {
      break; // No more frames
    }

    // Get RGB color channels
    pfs::Channel *R= NULL, *B = NULL; // for clarity of the code
    pfs::Channel *Y;
    pfs::Array2DImpl G( frame->getWidth(), frame->getHeight() );
    {
      pfs::Channel *X, *Z;
      
      frame->getXYZChannels( X, Y, Z );
      
      if( X != NULL ) {       // Has color
        R = X;
        B = Z;
        pfs::transformColorSpace( pfs::CS_XYZ, X, Y, Z,
          pfs::CS_RGB, R, &G, B );
      } else {                  // Monochromatic
        Y = frame->getChannel( "Y" );
        if( Y == NULL )
          throw pfs::Exception( "Missing color and luminance information in one of the input frames." );     
        R = Y;
        pfs::copyArray( Y, &G );        
        B = Y;
      }      
    }

    // Get base_name if needed
    std::string base_name;
    {
      const char *filename = frame->getTags()->getString( "FILE_NAME" );
      if( filename == NULL ) {
        if( first_frame && page_name != NULL )
          base_name = page_name;
        else {
          if( first_frame )
            throw pfs::Exception( "Specify page name if FILE_NAME information in missing in the pfsstream" );
          else
            throw pfs::Exception( "Cannot handle multiple images if FILE_NAME information in missing in the pfsstream" );
        }
      } else {
        std::string tmp_str( filename );

        // Remove extension
        int dot_pos = tmp_str.find_last_of( '.' );
        if( dot_pos != std::string::npos & dot_pos > 0 )
          tmp_str = tmp_str.substr( 0, dot_pos );

        // Remove path
        int slash_pos = tmp_str.find_last_of( "/\\" );
        if( slash_pos != std::string::npos )
          tmp_str = tmp_str.substr( slash_pos+1, std::string::npos );
        
        // Substitute invalid characters
        while( true ) {
          int invalid_pos = tmp_str.find_last_of( "-! #@()[]{}`." );
          if( invalid_pos == std::string::npos )
            break;
          tmp_str.replace( invalid_pos, 1, 1, '_' );
        }        
      
        base_name = tmp_str;
      }
      
    }

    std::cerr << "Adding image " << base_name << " to the web page\n";

    image_set.add_image( frame->getWidth(), frame->getHeight(), R->getRawData(), G.getRawData(), B->getRawData(), Y->getRawData(), base_name.c_str(), quality );
    
    pfsio.freeFrame( frame );
    first_frame = false;
  }

  image_set.generate_webpage( page_template, image_template, object_output, html_output );
}


int main( int argc, char* argv[] )
{
  try {
    generate_hdrhtml( argc, argv );
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
