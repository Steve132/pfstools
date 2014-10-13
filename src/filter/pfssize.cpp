/**
 * @brief Resize images in PFS stream
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
 * $Id: pfssize.cpp,v 1.4 2009/01/29 00:44:30 rafm Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>

#include <pfs.h>

#include <sstream>

#include "config.h"

#define PROG_NAME "pfssize"


#define ROUNDING_ERROR 0.000001

class QuietException 
{
};

class ResampleFilter
{
public:
  /**
   * Size of the filter in samples.
   */
  virtual float getSize() = 0;
  /**
   * Get value of the filter for x. x is always positive.
   */
  virtual float getValue( const float x ) = 0;

  virtual ~ResampleFilter()
  {
  }
  
};

void resize( const pfs::Array2D *src, pfs::Array2D *dest );
void resampleMitchell( const pfs::Array2D *in, pfs::Array2D *out );
void resampleArray( const pfs::Array2D *in, pfs::Array2D *out, ResampleFilter *filter );

inline float max( float a, float b )
{
  return a > b ? a : b;
}

inline float min( float a, float b )
{
  return a < b ? a : b;
}

// --------- Filters --------

class MitchellFilter : public ResampleFilter
{
public:
  float getSize() { return 2; }
  float getValue( const float x ) 
  {
    const float B = 0.3333f, C = 0.3333f;
    const float x2 = x*x;
    if( x < 1 )
      return 1.f/6.f * (
        (12-9*B-6*C)*x2*x +
        (-18+12*B+6*C)*x2 +
        (6-2*B) );
    if( x >=1 && x < 2 )
      return 1.f/6.f * (
        (-B-6*C)*x2*x +
        (6*B + 30*C)*x2 +
        (-12*B-48*C)*x +
        (8*B+24*C) );
    return 0;
  }
};

class LinearFilter : public ResampleFilter
{
public:
  float getSize() { return 1; }
  float getValue( const float x ) 
  {
    if( x < 1 ) return 1 - x;
    return 0;
  }
};


class BoxFilter : public ResampleFilter
{
public:
  float getSize() { return 0.5; }
  float getValue( const float x ) 
  {
    return 1;
  }
};

void printHelp()
{
  fprintf( stderr, PROG_NAME " [--x <pixels>] [--y <pixels>] [--ratio <ratio>] [--verbose] [--help]\n"
    "See man page for more information.\n" );
}

static void errorCheck( bool condition, const char *string )
{
    if( !condition ) {
	fprintf( stderr, PROG_NAME " error: %s\n", string );
	throw QuietException();
    }
}

static int getIntParam( const char *str, const char *paramName )
{
  char *endp;
  int val = strtol( optarg, &endp, 10 );
  if( endp == optarg ) {
    std::ostringstream oss;
    oss << "Bad value for " << paramName << " argument";
    throw pfs::Exception( oss.str().c_str() );
  }
  return val;
}

static float getFloatParam( const char *str, const char *paramName )
{
  char *endp;
  float val = strtof( optarg, &endp );
  if( endp == optarg ) {
    std::ostringstream oss;
    oss << "Bad value for " << paramName << " argument";
    throw pfs::Exception( oss.str().c_str() );
  }
  return val;
}

void resizeFrames( int argc, char* argv[] )
{
  pfs::DOMIO pfsio;

  float ratio = -1;
  int xSize = -1;
  int ySize = -1;
  int minX = -1;
  int maxX = -1;
  int minY = -1;
  int maxY = -1;
  bool verbose = false;
  ResampleFilter *filter = NULL;

  static struct option cmdLineOptions[] = {
    { "help", no_argument, NULL, 'h' },
    { "verbose", no_argument, NULL, 'v' },
    { "x", required_argument, NULL, 'x' },
    { "y", required_argument, NULL, 'y' },
    { "maxx", required_argument, NULL, '1' },
    { "maxy", required_argument, NULL, '2' },
    { "minx", required_argument, NULL, '3' },
    { "miny", required_argument, NULL, '4' },
    { "ratio", required_argument, NULL, 'r' },
    { "filter", required_argument, NULL, 'f' },
    { NULL, 0, NULL, 0 }
  };

  int optionIndex = 0;
  while( 1 ) {
    int c = getopt_long (argc, argv, "x:y:r:f:", cmdLineOptions, &optionIndex);
    if( c == -1 ) break;
    switch( c ) {
    case 'h':
      printHelp();
      throw QuietException();
    case 'v':
      verbose = true;
      break;
    case 'x':
      xSize = getIntParam( optarg, "x" );
      break;
    case 'y':
      ySize = getIntParam( optarg, "y" );
      break;
    case 'r':
      ratio = getFloatParam( optarg, "ratio" );
      break;
    case '1':
      maxX = getIntParam( optarg, "maxx" );
      break;
    case '2':
      maxY = getIntParam( optarg, "maxy" );
      break;
    case '3':
      minX = getIntParam( optarg, "minx" );
      break;
    case '4':
      minY = getIntParam( optarg, "miny" );
      break;
    case 'f':
      if( !strcasecmp( optarg, "LINEAR" ) ) {
        filter = new LinearFilter();
      } else if( !strcasecmp( optarg, "MITCHELL" ) ) {
        filter = new MitchellFilter();
      } else if( !strcasecmp( optarg, "BOX" ) ) {
        filter = new BoxFilter();
      } else {
        throw pfs::Exception( "Unknown filter. Possible values: LINEAR, BOX, MITCHELL" );     
      }
      break;
    case '?':
      throw QuietException();
    case ':':
      throw QuietException();
    }
  }

  if( filter == NULL ) filter = new LinearFilter();

  bool isMinMax = (minX!=-1) || (maxX!=-1) || (minY!=-1) || (maxY!=-1);

  errorCheck( (minX==-1 || maxX == -1 || minX <= maxX) &&
    (minY==-1 || maxY==-1 || minY <= maxY),
    "Min value must be lower than max value" );
  
  errorCheck( (ratio != -1) ^ (xSize != -1 || ySize != -1) ^ isMinMax, "Specify either size or ratio or min/max sizes" );
  errorCheck( ratio == -1 || ratio > 0 , "Wrong scaling ratio" );
  
//  bool firstFrame = true;

  pfs::Frame *resizedFrame = NULL;
  
  while( true ) {
    pfs::Frame *frame = pfsio.readFrame( stdin );
    if( frame == NULL ) break; // No more frames

    pfs::Channel *X, *Y, *Z;
    frame->getXYZChannels( X, Y, Z );

//    pfs::Channel *dX, *dY, *dZ;
    
//    if( firstFrame ) {

    int new_x, new_y;

    
      if( ratio != -1 ) {
        new_x = (int)(frame->getWidth()*ratio);
        new_y = (int)(frame->getHeight()*ratio);
      } else {
        if( isMinMax ) {        // Min/max sizes given
          new_x = frame->getWidth();
          new_y = frame->getHeight();
          float mm_ratio = (float)new_x/(float)new_y;
          if( minX != -1 && new_x < minX ) {
            new_x = minX;
            new_y = (int)((float)new_x/mm_ratio);
          }
          if( minY != -1 && new_y < minY ) {
            new_y = minY;
            new_x = (int)((float)new_y*mm_ratio);
          }
          if( maxX != -1 && new_x > maxX ) {
            new_x = maxX;
            new_y = (int)((float)new_x/mm_ratio);
          }
          if( maxY != -1 && new_y > maxY ) {
            new_y = maxY;
            new_x = (int)((float)new_y*mm_ratio);
          }
        } else {                // Size given
          new_x = xSize;
          new_y = ySize;
          if( new_x == -1 )
            new_x = (int)((float)frame->getWidth() * (float)ySize / (float)frame->getHeight());
          else if( new_y == -1 )
            new_y = (int)((float)frame->getHeight() * (float)xSize / (float)frame->getWidth());
        }
        
      }
      errorCheck( new_x > 0 && new_y > 0 && new_x <= 65536 && new_y <= 65536, "Wrong frame size" );
      errorCheck( ((frame->getWidth() <= new_x) && (frame->getHeight() <= new_y)) ||
        ((frame->getWidth() >= new_x) && (frame->getHeight() >= new_y)),
        "Can upsample / downsample image only in both dimensions simultaneously" );
      
      if( verbose ) fprintf( stderr, "New size: %d x %d \n", new_x, new_y );
      
      resizedFrame = pfsio.createFrame( new_x, new_y );
      
//      firstFrame = false;
//    }

    pfs::ChannelIterator *it = frame->getChannels();
    while( it->hasNext() ) {
      pfs::Channel *originalCh = it->getNext();
      pfs::Channel *newCh = resizedFrame->createChannel( originalCh->getName() );

      resampleArray( originalCh, newCh, filter );
    }

    pfs::copyTags( frame, resizedFrame );
    
    pfsio.writeFrame( resizedFrame, stdout );
    pfsio.freeFrame( frame );        
  }
  pfsio.freeFrame( resizedFrame );
  delete filter;
}


void upsampleArray( const pfs::Array2D *in, pfs::Array2D *out, ResampleFilter *filter )
{
  float dx = (float)in->getCols() / (float)out->getCols();
  float dy = (float)in->getRows() / (float)out->getRows();

  float pad;
  
  float filterSamplingX = max( modff( dx, &pad ), 0.01f );
  float filterSamplingY = max( modff( dy, &pad ), 0.01f );

  const int outRows = out->getRows();
  const int outCols = out->getCols();

  const float inRows = (float)in->getRows();
  const float inCols = (float)in->getCols();

  const float filterSize = filter->getSize();

// TODO: possible optimization: create lookup table for the filter
  
  float sx, sy;
  int x, y;
  for( y = 0, sy = -0.5f + dy/2; y < outRows; y++, sy += dy )
    for( x = 0, sx = -0.5f + dx/2; x < outCols; x++, sx += dx ) {

      float pixVal = 0;
      float weight = 0;
      
      for( float ix = max( 0, ceilf( sx-filterSize ) ); ix <= min( floorf(sx+filterSize), inCols-1 ); ix++ )
        for( float iy = max( 0, ceilf( sy-filterSize ) ); iy <= min( floorf( sy+filterSize), inRows-1 ); iy++ ) {
          float fx = fabs( sx - ix );
          float fy = fabs( sy - iy );

          const float fval = filter->getValue( fx )*filter->getValue( fy );
          
          pixVal += (*in)( (int)ix, (int)iy ) * fval;
          weight += fval;
        }

      if( weight == 0 ) {
        fprintf( stderr, "%g %g %g %g\n", sx, sy, dx, dy );
      }    
//      assert( weight != 0 );
      (*out)(x,y) = pixVal / weight;

    } 
}

void downsampleArray( const pfs::Array2D *in, pfs::Array2D *out )
{
  const float inRows = (float)in->getRows();
  const float inCols = (float)in->getCols();

  const int outRows = out->getRows();
  const int outCols = out->getCols();

  const float dx = (float)in->getCols() / (float)out->getCols();
  const float dy = (float)in->getRows() / (float)out->getRows();

  const float filterSize = 0.5;
  
  float sx, sy;
  int x, y;
  
  for( y = 0, sy = dy/2-0.5f; y < outRows; y++, sy += dy )
    for( x = 0, sx = dx/2-0.5f; x < outCols; x++, sx += dx ) {

      float pixVal = 0;
      float w = 0;
      for( float ix = max( 0, ceilf( sx-dx*filterSize ) ); ix <= min( floorf( sx+dx*filterSize ), inCols-1 ); ix++ )
        for( float iy = max( 0, ceilf( sy-dx*filterSize ) ); iy <= min( floorf( sy+dx*filterSize), inRows-1 ); iy++ ) {
          pixVal += (*in)( (int)ix, (int)iy );
          w += 1;
        }     
      (*out)(x,y) = pixVal/w;      
    }
}

void resampleArray( const pfs::Array2D *in, pfs::Array2D *out, ResampleFilter *filter )
{
  if( in->getCols() == out->getCols() && in->getRows() == out->getRows() )
    pfs::copyArray( in, out );
  else if( in->getCols() < out->getCols() || in->getRows() < out->getRows() )
    upsampleArray( in, out, filter );
  else                          
    downsampleArray( in, out ); 
}

int main( int argc, char* argv[] )
{
  try {
    resizeFrames( argc, argv );
  }
  catch( pfs::Exception ex ) {
    fprintf( stderr, PROG_NAME " error: %s\n", ex.getMessage() );
    return EXIT_FAILURE;
  }        
  catch( QuietException ) {
    return EXIT_FAILURE;
  }        
  return EXIT_SUCCESS;
}
