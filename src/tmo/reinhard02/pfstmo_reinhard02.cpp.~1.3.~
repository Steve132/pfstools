/**
 * @file pfstmo_reinhard02.cpp
 * @brief Tone map XYZ channels using Reinhard02 model
 *
 * Photographic Tone Reproduction for Digital Images.
 * E. Reinhard, M. Stark, P. Shirley, and J. Ferwerda.
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
 * $Id: pfstmo_reinhard02.cpp,v 1.3 2008/09/04 12:46:49 julians37 Exp $
 */

#include <config.h>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>

#include <pfs.h>

#include <tmo_reinhard02.h>

using namespace std;

#define PROG_NAME "pfstmo_reinhard02"

/// verbose mode
bool verbose = false;

class QuietException 
{
};

void printHelp()
{
  fprintf( stderr, PROG_NAME " (" PACKAGE_STRING ") : \n"
    "\t[--scales] \n"
    "\t[--key <val>] [--phi <val>] \n"
    "\t[--range <val>] [--lower <val>] [--upper <val>] \n"
    "\t[--temporal-coherent]\n"
    "\t[--verbose] [--help] \n"
    "See man page for more information.\n" );
}

void pfstmo_reinhard02( int argc, char* argv[] )
{
  pfs::DOMIO pfsio;

  //--- default tone mapping parameters;
  float key = 0.18;
  float phi = 1.0;
  int num = 8;
  int low = 1;
  int high = 43;
  bool use_scales = false;
  bool temporal_coherent = false;  

  static struct option cmdLineOptions[] = {
    { "help", no_argument, NULL, 'h' },
    { "verbose", no_argument, NULL, 'v' },
    { "scales", no_argument, NULL, 's' },
    { "key", required_argument, NULL, 'k' },
    { "phi", required_argument, NULL, 'p' },
    { "range", required_argument, NULL, 'r' },
    { "lower", required_argument, NULL, 'l' },
    { "upper", required_argument, NULL, 'u' },
    { "temporal-coherent", no_argument, NULL, 't' },
    { NULL, 0, NULL, 0 }
  };

  int optionIndex = 0;
  while( 1 ) {
    int c = getopt_long (argc, argv, "hvsk:p:r:l:u:t", cmdLineOptions, &optionIndex);
    if( c == -1 ) break;
    switch( c ) {
    case 'h':
      printHelp();
      throw QuietException();
    case 'v':
      verbose = true;
      break;
    case 's':
      use_scales = true;
      break;
    case 'k':
      key = (float)strtod( optarg, NULL );
      if( key<=0.0f || key>1.0f )
        throw pfs::Exception("key value out of range, should be <0..1>");
      break;
    case 'p':
      phi = (float)strtod( optarg, NULL );
      if( phi<=0.0f )
        throw pfs::Exception("phi value out of range, should be >0.0");
      break;
    case 'r':
      num = (int)strtod( optarg, NULL );
      if( num<1 )
        throw pfs::Exception("range size value out of range, should be >1");
      break;
    case 'l':
      low = (int)strtod( optarg, NULL );
      if( low<1 )
        throw pfs::Exception("lower scale size out of range, should be >1");
      break;
    case 'u':
      high = (int)strtod( optarg, NULL );
      if( high<1 )
        throw pfs::Exception("upper scale size out of range, should be >1");
      break;
    case 't':
      temporal_coherent = true;
      break;
    case '?':
      throw QuietException();
    case ':':
      throw QuietException();
    }
  }

  VERBOSE_STR << "use scales: " << (use_scales ? "yes" : "no" ) << endl;
  VERBOSE_STR << "key value: " << key << endl;
  VERBOSE_STR << "phi value: " << phi << endl;
  if( use_scales )
  {
    VERBOSE_STR << "number of scales: " << num << endl;
    VERBOSE_STR << "lower scale size: " << low << endl;
    VERBOSE_STR << "upper scale size: " << high << endl;
#ifndef HAVE_ZFFT
    VERBOSE_STR << "approximate implementation of scales" << endl;
#endif
  }

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
    pfs::Array2DImpl* L = new pfs::Array2DImpl(w,h);

    tmo_reinhard02( w, h, Y->getRawData(), L->getRawData(), use_scales, key, phi, num, low, high, temporal_coherent );

    for( int x=0 ; x<w ; x++ )
      for( int y=0 ; y<h ; y++ )
      {
        float scale = (*L)(x,y) / (*Y)(x,y);
        (*Y)(x,y) *= scale;
        (*X)(x,y) *= scale;
        (*Z)(x,y) *= scale;
      }

    delete L;

    //---
    pfsio.writeFrame( frame, stdout );
    pfsio.freeFrame( frame );        
  }
}

int main( int argc, char* argv[] )
{
  try {
    pfstmo_reinhard02( argc, argv );
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
