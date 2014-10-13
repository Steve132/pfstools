/**
 * @file pfstmo_pattanaik00.cpp
 * @brief Tone map XYZ channels using Pattanaik00 model
 *
 * Time-Dependent Visual Adaptation for Realistic Image Display
 * S.N. Pattanaik, J. Tumblin, H. Yee, and D.P. Greenberg
 * In Proceedings of ACM SIGGRAPH 2000
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
 * $Id: pfstmo_pattanaik00.cpp,v 1.3 2008/09/04 12:46:49 julians37 Exp $
 */

#include <config.h>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>

#include <pfs.h>

#include <tmo_pattanaik00.h>

using namespace std;

#define PROG_NAME "pfstmo_pattanaik00"

class QuietException 
{
};

void multiplyChannels( pfs::Array2D* X, pfs::Array2D* Y, pfs::Array2D* Z, float mult );


void printHelp()
{
  fprintf( stderr, PROG_NAME " (" PACKAGE_STRING ") : \n"
    "\t[--time-dependence] [--local] \n"
    "\t[--mul <multiplier>] \n"
    "\t[--cone <val>] [--rod <val>] \n"
    "\t[--verbose] [--help]\n"
    "See man page for more information.\n" );
}

void pfstmo_pattanaik00( int argc, char* argv[] )
{
  pfs::DOMIO pfsio;

  //--- default tone mapping parameters;
  bool timedependence = false;
  bool local = false;
  float multiplier = 1.0f;
  float Acone = -1.0f;
  float Arod  = -1.0f;
  float fps = 16.0f;

  //--- process command line args
  bool verbose = false;

  static struct option cmdLineOptions[] = {
    { "help", no_argument, NULL, 'h' },
    { "verbose", no_argument, NULL, 'v' },
    { "time-dependence", no_argument, NULL, 't' },
    { "fps", required_argument, NULL, 'f' },
    { "local", no_argument, NULL, 'l' },
    { "mul", required_argument, NULL, 'm' },
    { "cone", required_argument, NULL, 'c' },
    { "rod", required_argument, NULL, 'r' },
    { NULL, 0, NULL, 0 }
  };

  int optionIndex = 0;
  while( 1 ) {
    int c = getopt_long (argc, argv, "hvtf:lm:c:r:", cmdLineOptions, &optionIndex);
    if( c == -1 ) break;
    switch( c ) {
    case 'h':
      printHelp();
      throw QuietException();
    case 'v':
      verbose = true;
      break;
    case 't':
      timedependence = true;
      break;
    case 'l':
      local = true;
      break;
    case 'f':
      fps = (float)strtod( optarg, NULL );
      if( fps<=0.0f )
        throw pfs::Exception("incorrect frames per second value, should be non-zero positive");
      break;
    case 'm':
      multiplier = (float)strtod( optarg, NULL );
      if( multiplier<=0.0f )
        throw pfs::Exception("incorrect multiplier value, should be non-zero positive");
      break;
    case 'c':
      Acone = (float)strtod( optarg, NULL );
      if( Acone<=0.0f )
        throw pfs::Exception("incorrect cone adaptation value, should be non-zero positive");
      if( Arod==-1.0f )
        Arod=Acone;
      break;
    case 'r':
      Arod = (float)strtod( optarg, NULL );
      if( Arod<=0.0f )
        throw pfs::Exception("incorrect rod adaptation value, should be non-zero positive");
      break;
    case '?':
      throw QuietException();
    case ':':
      throw QuietException();
    }
  }

  VERBOSE_STR << "time-dependence: " << (timedependence ? "yes" : "no") << endl;
  if( timedependence )
    VERBOSE_STR << "frames per sec.: " << fps << endl;
  VERBOSE_STR << "local:           " << (local ? "yes" : "no") << endl;
  VERBOSE_STR << "multiplier:      " << multiplier << endl;

   
  VisualAdaptationModel* am = new VisualAdaptationModel();

  bool firstFrame = true;
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
        
    // adaptation model
    if( multiplier!=1.0f )
      multiplyChannels( X, Y, Z, multiplier );

    if( !local )
    {
      if( firstFrame || !timedependence )
      {
        if( Acone!=-1.0f )
          am->setAdaptation(Acone,Arod);
        else
          am->setAdaptation(Y);

        firstFrame = false;
      }
      else
        am->calculateAdaptation(Y, 1.0f/fps);
      
      VERBOSE_STR << "adaptation cone: " << am->getAcone() << endl;
      VERBOSE_STR << "adaptation rod:  " << am->getArod() << endl;
      VERBOSE_STR << "bleaching cone:  " << am->getBcone() << endl;
      VERBOSE_STR << "bleaching rod:   " << am->getBrod() << endl;
    }

    // tone mapping
    int w = Y->getCols();
    int h = Y->getRows();
    pfs::Array2DImpl* R = new pfs::Array2DImpl(w,h);
    pfs::Array2DImpl* G = new pfs::Array2DImpl(w,h);
    pfs::Array2DImpl* B = new pfs::Array2DImpl(w,h);

    pfs::transformColorSpace( pfs::CS_XYZ, X, Y, Z, pfs::CS_RGB, R, G, B );
    tmo_pattanaik00( w, h, R->getRawData(), G->getRawData(), B->getRawData(), Y->getRawData(), am, local );
    pfs::transformColorSpace( pfs::CS_RGB, R, G, B, pfs::CS_XYZ, X, Y, Z );

    delete R;
    delete G;
    delete B;

    //---
    pfsio.writeFrame( frame, stdout );
    pfsio.freeFrame( frame );        
  }

  delete am; // delete visual adaptation model
}

void multiplyChannels( pfs::Array2D* X, pfs::Array2D* Y, pfs::Array2D* Z, float mult )
{
  int size = Y->getCols() * Y->getRows();

  for( int i=0 ; i<size; i++ )
  {
    (*X)(i) *= mult;
    (*Y)(i) *= mult;
    (*Z)(i) *= mult;
  }
}


int main( int argc, char* argv[] )
{
  try {
    pfstmo_pattanaik00( argc, argv );
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
