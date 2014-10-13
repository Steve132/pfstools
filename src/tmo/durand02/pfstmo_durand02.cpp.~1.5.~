/**
 * @file pfstmo_durand02.cpp
 * @brief Tone map XYZ channels using Durand02 model
 *
 * Fast Bilateral Filtering for the Display of High-Dynamic-Range Images.
 * F. Durand and J. Dorsey.
 * In ACM Transactions on Graphics, 2002.
 *
 * 
 * This file is a part of PFSTMO package.
 * ---------------------------------------------------------------------- 
 * Copyright (C) 2003,2004 Grzegorz Krawczyk
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
 * $Id: pfstmo_durand02.cpp,v 1.5 2009/02/23 19:09:41 rafm Exp $
 */

#include <config.h>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>

#include <pfs.h>

#include <tmo_durand02.h>

using namespace std;

#define PROG_NAME "pfstmo_durand02"

/// verbose mode
bool verbose = false;
bool quiet = false;

#define FRAME_NAME_MAX 30
char frame_name[FRAME_NAME_MAX+1];

int progress_report( int progress )
{
  if( !quiet ) {  
    fprintf( stderr, "\r'%s' completed %d%%", frame_name, progress );
    if( progress == 100 )
      fprintf( stderr, "\n" );    
  }
  return PFSTMO_CB_CONTINUE;
}

class QuietException 
{
};

void printHelp()
{
  fprintf( stderr, PROG_NAME " (" PACKAGE_STRING ") : \n"
    "\t[--sigma-s <val>] [--sigma-r <val>] \n"
    "\t[--base-contrast <val>] \n"
    "\t[--original] \n"
    "\t[--quiet] [--verbose] [--help] \n"
    "See man page for more information.\n" );
}

void pfstmo_durand02( int argc, char* argv[] )
{
  pfs::DOMIO pfsio;

  //--- default tone mapping parameters;
#ifdef HAVE_FFTW3F
  float sigma_s = 40.0f;
#else
  float sigma_s = 8.0f;
#endif
  float sigma_r = 0.4f;
  float baseContrast = 5.0f;
  int downsample=1;
  bool original_algorithm = false;

  static struct option cmdLineOptions[] = {
    { "help", no_argument, NULL, 'h' },
    { "verbose", no_argument, NULL, 'v' },
    { "original", no_argument, NULL, 'g' },
    { "sigma-s", required_argument, NULL, 's' },
    { "sigma-r", required_argument, NULL, 'r' },
    { "base-contrast", required_argument, NULL, 'c' },
//    { "downsampling", required_argument, NULL, 'z' },
    { "quiet", no_argument, NULL, 'q' },    
    { NULL, 0, NULL, 0 }
  };

  int optionIndex = 0;
  while( 1 ) {
    int c = getopt_long (argc, argv, "hvs:r:c:qg", cmdLineOptions, &optionIndex);
    if( c == -1 ) break;
    switch( c ) {
    case 'q':
      quiet = true;
      break;
    case 'h':
      printHelp();
      throw QuietException();
    case 'v':
      verbose = true;
      break;
    case 'g':
      original_algorithm = true;
      break;
    case 's':
      sigma_s = (float)strtod( optarg, NULL );
      if( sigma_s<=0.0f )
        throw pfs::Exception("sigma_s value out of range, should be >0");
      break;
    case 'r':
      sigma_r = (float)strtod( optarg, NULL );
      if( sigma_r<=0.0f )
        throw pfs::Exception("sigma_r value out of range, should be >0");
      break;
    case 'c':
      baseContrast = (float)strtod( optarg, NULL );
      if( baseContrast<=0.0f )
        throw pfs::Exception("base contrast value out of range, should be >0");
      break;
//     case 'z':
//       downsample = atoi( optarg );
//       if( downsample<1 || downsample>20 )
//         throw pfs::Exception("down sampling factor should be in 1:20 range");
//       break;
    case '?':
      throw QuietException();
    case ':':
      throw QuietException();
    }
  }

  VERBOSE_STR << "sigma_s: " << sigma_s << endl;
  VERBOSE_STR << "sigma_r: " << sigma_r << endl;
  VERBOSE_STR << "base contrast: " << baseContrast << endl;
//  VERBOSE_STR << "down sampling factor: " << downsample << endl;
#ifdef HAVE_FFTW3F
  VERBOSE_STR << "fast bilateral filtering (fftw3)" << endl;
#else
  VERBOSE_STR << "conventional bilateral filtering" << endl;
#endif

  int frame_no = 1;
  while( true )    
  {
    pfs::Frame *frame = pfsio.readFrame( stdin );
    if( frame == NULL ) break; // No more frames

    pfs::Channel *X, *Y, *Z;
    frame->getXYZChannels( X, Y, Z );
    frame->getTags()->setString("LUMINANCE", "RELATIVE");
    //---

    if( Y==NULL || X==NULL || Z==NULL)
      throw pfs::Exception( "Missing X, Y, Z channels in the PFS stream" );
        
    // tone mapping
    int w = Y->getCols();
    int h = Y->getRows();

    const char *file_name = frame->getTags()->getString( "FILE_NAME" );
    if( file_name == NULL )
      sprintf( frame_name, "frame #%d", frame_no );
    else {
      int len = strlen( file_name );
      if( len > FRAME_NAME_MAX ) // In case file name is too long
        len = FRAME_NAME_MAX-3;
      strcpy( frame_name, "..." );
      strncpy( frame_name+3, file_name + strlen( file_name ) - len, len+1 );
    }
    

    pfs::transformColorSpace( pfs::CS_XYZ, X, Y, Z, pfs::CS_RGB, X, Y, Z );
    tmo_durand02( w, h, X->getRawData(), Y->getRawData(), Z->getRawData(), sigma_s, sigma_r, baseContrast, downsample, !original_algorithm, progress_report );
    pfs::transformColorSpace( pfs::CS_RGB, X, Y, Z, pfs::CS_XYZ, X, Y, Z );

    //---
    pfsio.writeFrame( frame, stdout );
    pfsio.freeFrame( frame );

    frame_no++;
  }
}

int main( int argc, char* argv[] )
{
  try {
    pfstmo_durand02( argc, argv );
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
