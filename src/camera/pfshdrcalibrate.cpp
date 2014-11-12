/**
 * @brief Create an HDR image or calibrate a response curve from a set
 * of differently exposed images supplied in PFS stream
 *
 * 
 * This file is a part of PFS CALIBRATION package.
 * ---------------------------------------------------------------------- 
 * Copyright (C) 2004 Grzegorz Krawczyk
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
 * @author Ivo Ihrke, <ihrke@mmci.uni-saarland.de>
 *
 * $Id: pfshdrcalibrate.cpp,v 1.16 2011/02/24 17:35:59 ihrke Exp $
 */

#include <config.h>

#include <iostream>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>

#include <pfs.h>

#include <responses.h>
#include <robertson02.h>
#include <mitsunaga99.h>

using namespace std;

#define PROG_NAME "pfshdrcalibrate"


inline float max3( float a, float b, float c )
{
  float max = (a>b) ? a : b;
  return (c>max) ? c : max;
}

inline float min3( float a, float b, float c )
{
  // ignore zero values
  if( int(a)==0 ) a=1e8;
  if( int(b)==0 ) b=1e8;
  if( int(c)==0 ) c=1e8;
  
  float min = (a<b) ? a : b;
  return (c<min) ? c : min;
}

//---------------------------------------------------
//--- standard PFS stuff
bool verbose = false;

class QuietException 
{
};


void printHelp()
{
  fprintf( stderr, PROG_NAME ": \n"
    "\t[--calibration <type>] [--luminance]\n"
    "\t[--response <type>] [--response-file <filename.m>] \n"
    "\t[--save-response <filename.m>] \n"
    "\t[--multiplier <val>] \n"
    "\t[--bpp <val>] \n"
    "\t[--verbose] [--help]\n"
    "See man page for more information.\n" );
}





void pfshdrcalibrate( int argc, char* argv[] )
{

  /* ------------------------------------------------------------------------------------------------ */
  /* -------------------------------- initialization start ------------------------------------------ */
  /* ------------------------------------------------------------------------------------------------ */

  pfs::DOMIO pfsio;

  enum TCalibration 
    { NONE, CALIBRATE } opt_calibration = CALIBRATE;
  enum TResponse
    { FROM_FILE, LINEAR, GAMMA, LOG10 } opt_response = LINEAR;
  enum TMethod
    { ROBERTSON_METHOD, MITSUNAGA_METHOD } opt_method = ROBERTSON_METHOD;
	
  /* defaults */
  float         input_multiplier              = 1.0f;
  FILE         *responseFile                  = NULL;
  FILE         *responseSaveFile              = NULL;
  int           opt_bpp                       = 8;
  bool          opt_fillgaps                  = false;   /* todo remove */
  bool          opt_luminance                 = false;
  float         opt_gauss                     = 0.2; 
  int           opt_maxresponse               = -1;
  int           opt_minresponse               = -1;
  unsigned long mitsunaga_sample_no           = MITSUNAGA_SAMPLES_NO;
  
  /* helper */
  int c;

  static struct option cmdLineOptions[] = {
    { "help", no_argument, NULL, 'h' },
    { "verbose", no_argument, NULL, 'v' },
    { "luminance", no_argument, NULL, 'Y' },
    //{ "fillin-response", no_argument, NULL, 'F' },      /* todo: remove - Robertson does not work well anyway */
    { "method", required_argument, NULL, 'c' },
    { "gauss", required_argument, NULL, 'g' },
    { "max-response", required_argument, NULL, 'A' },
    { "min-response", required_argument, NULL, 'S' },
    { "response", required_argument, NULL, 'r' },
    { "response-file", required_argument, NULL, 'f' },
    { "save-response", required_argument, NULL, 's' },
    { "multiplier", required_argument, NULL, 'm' },
    { "bpp", required_argument, NULL, 'b' },
    { "samples", required_argument, NULL, 'p' },	
    { NULL, 0, NULL, 0 }
  };

  int optionIndex = 0;
  while( ( c =  getopt_long( argc, 
			     argv, 
			     "hvYFc:g:r:f:s:m:b:p:x", 
			     cmdLineOptions, 
			     &optionIndex) ) 
	 != -1 ) 
    {
      
      switch( c ) 
	{
	  /* help */
	case 'h':
	  printHelp();
	  throw QuietException();

	  /* verbose */
	case 'v':
	  verbose = true;
	  break;

	  /* gray scale */
	case 'Y':
	  opt_luminance = true;
	  break;

	  /* REMOVE: fix for Robertson -> remove in the future */
	case 'F':
	  opt_fillgaps = true;
	  break;
		
	  /* calibration method */
	case 'c':
	  if( strcmp( optarg, "mitsunaga" ) == 0 ) 
	    {
	      opt_calibration = CALIBRATE;
	      opt_method = MITSUNAGA_METHOD;
	    }
	  else 
	    if( strcmp( optarg, "robertson" ) == 0 ) 
	      {
		opt_calibration = CALIBRATE;	
		opt_method = ROBERTSON_METHOD;
	      }
	    else 
	      if( strcmp( optarg, "none" ) == 0 ) 
		{
		  opt_calibration = NONE;	
		  opt_method = ROBERTSON_METHOD;
		}
	      else
		throw pfs::Exception("unsupported automatic self-calibration method");        
	  break;
		
	  /* support of Gaussian weighting curve */
	  /* REMOVE: support 16 bit raw formats, in the long run switch to normalized 0-1 */
	case 'g':
	  opt_gauss = atof(optarg);
	  if( opt_gauss <= 0.0f or opt_gauss > 1.0f)
	    throw pfs::Exception("sigma value for Gaussian out of range. accepted range 0:1");
	  break;

	  /* predefined response curve */
	case 'r':
	  if( strcmp( optarg, "linear" ) == 0 ) 
	    opt_response = LINEAR;
	  else 
	    if( strcmp( optarg, "gamma" ) == 0 ) 
	      opt_response = GAMMA;
	    else 
	      if( strcmp( optarg, "log" ) == 0 ) 
		opt_response = LOG10;
	      else
		throw pfs::Exception("unknown standard response (check the manpage or use default)");

	      opt_calibration = NONE;
	  break;

	  
	  /* response curve from file */
	case 'f':
	  opt_response    = FROM_FILE;
	  opt_calibration = NONE;
	  responseFile    = fopen(optarg, "r");
	  if( !responseFile )
	    throw pfs::Exception("could not open file with response curve");
	  break;

	  /* response curve to file */
	case 's':
	  responseSaveFile = fopen(optarg,"w");
	  if( !responseSaveFile )
	    throw pfs::Exception("could not open file to save response curve");
	  break;

	  /* REMOVE: this is a hack option as well */
	case 'm':
	  input_multiplier = atof(optarg);
	  if( input_multiplier<=0.0f )
	    throw pfs::Exception("input multiplier value out of range. accepted range >0");
	  break;
	  
	  /* bit depth of input files */
	case 'b':
	  opt_bpp = atoi(optarg);
	  if( opt_bpp<8 || opt_bpp>32)
	    throw pfs::Exception("bits per pixel value out of range. accepted range >=8");
	  break;

	  /* internal parameters of Mitsunaga & Nayar's method  */
	case 'p':
	  mitsunaga_sample_no = (unsigned long)atoll(optarg);
	  if( mitsunaga_sample_no<10 || mitsunaga_sample_no >= (1 << 31))
	    throw pfs::Exception("too many samples");
	  break;	  


	case 'A':                   // max response
	  opt_maxresponse = atoi(optarg);
	  if( opt_maxresponse<=opt_minresponse )
	    throw pfs::Exception("max response should be higher than min response");
	  break;

	case 'S':                   // min response
	  opt_minresponse = atoi(optarg);
	  if( opt_minresponse<0 )
	    throw pfs::Exception("min response should be >0");
	  break;

	case '?':
	  throw QuietException();

	case ':':
	  throw QuietException();
	}
    } 


  /* FIX: this is most important, use fixed 0-1 range */
  //!! FIX
  // in PFS streams, 8bit data are mapped to 0:1 range

  //Ivo: this seems to be the problem of >, code seems to assume 0:255 range of values
  //     but 16bit values do not get multiplied, however, out of pfs they also come
  //     in the range between 0:1, 
  //     proper solution: change code to work on the 0:1 range
  //  if( opt_bpp == 8 )
  //    input_multiplier = 255;
  //Ivo end

  if( opt_method == ROBERTSON_METHOD )
    {
      input_multiplier  = float( 1 << opt_bpp ) - 1;
      opt_gauss        *= input_multiplier;
    }

  
  if( opt_method == MITSUNAGA_METHOD )
    input_multiplier = 1.0f;
  

  //--- verbose information and load initialization data
  VERBOSE_STR << "Assuming " << opt_bpp << " Bits per pixel in the LDR images (use --bpp to change this)" << endl;

  VERBOSE_STR << "calibrating channels: "
              << (opt_luminance ? "LUMINANCE" : "RGB") << endl;

  switch( opt_response )
    {
      
    case FROM_FILE:
      VERBOSE_STR << "response curve from file" << endl;
      break;
      
    case LINEAR:
      VERBOSE_STR << "initial response: linear" << endl;
      break;
      
    case GAMMA:
      VERBOSE_STR << "initial response: gamma" << endl;
      break;

    case LOG10:
      VERBOSE_STR << "initial response: logarithmic" << endl;
      break;
      
    default:
      throw pfs::Exception("undefined standard response");
      break;
    }
  
	

//   VERBOSE_STR << "interpolate missing parts of response: "
//               << (opt_fillgaps ? "yes" : "no") << endl;
  
  if( responseSaveFile != NULL )
    VERBOSE_STR << "save response curve to a file (do not generate HDR image)" << endl;


  // number of input levels
  int M = (int) powf( 2.0f, opt_bpp );
  VERBOSE_STR << "number of input levels: " << M << endl;
  VERBOSE_STR << "input multiplier: " << input_multiplier << endl;
  

  /* ------------------------------------------------------------------------------------------------ */
  /* -------------------------------- initialization done ------------------------------------------- */
  /* ------------------------------------------------------------------------------------------------ */

  /* ------------------------------------------------------------------------------------------------ */
  /* ---------------------------- preparation of images start --------------------------------------- */
  /* ------------------------------------------------------------------------------------------------ */

  //--- read frames from pfs stream
  int frame_no = 1;
  int width    = 0;
  int height   = 0;
  int size     = 0;

  float minResponse = M;
  float maxResponse = 0.0f;

  // collected exposures
  ExposureList imgsY;
  ExposureList imgsR;
  ExposureList imgsG;
  ExposureList imgsB;

  while( true ) 
    {
      pfs::Frame *frame = pfsio.readFrame( stdin );

      if( frame == NULL ) 
	break; // No more frames
        
      pfs::Channel *X = NULL;
      pfs::Channel *Y = NULL;
      pfs::Channel *Z = NULL;


      frame -> getXYZChannels( X, Y, Z );

      if( X==NULL || Y==NULL || Z==NULL )
	throw pfs::Exception( "missing XYZ channels in the PFS stream (try to preview your files using pfsview)" );

      const char* exp_str = 
	frame -> getTags() -> getString("BV");

      if( exp_str == NULL )
	throw pfs::Exception( "missing exposure information in the PFS stream (use pfsinhdrgen to input files)" );

      // relate APEX brightness value only as a function of exposure time
      // that is assume aperture=1 and sensitivity=1
      float exp_time = 1.0f / powf(2.0f,atof( exp_str ));

      // absolute calibration: this magic multiplier is a result of my
      // research in internet plus a bit of tweaking with a luminance
      // meter. tested with canon 350d, so might be a bit off for other
      // cameras. to control absolute calibration modify iso values in
      // hdrgen script or use pfsabsolute program.
      exp_time /= 1.0592f * 11.4f / 3.125f;
      VERBOSE_STR << "Exposure:" << exp_time << endl;	

      // frame size
      width  = Y -> getCols();
      height = Y -> getRows();
      size   = width * height;

      // new exposure image
      // in luminance only mode
      if ( opt_luminance )
	{
	  
	  Exposure eY;
	  eY.ti = exp_time;
	  eY.yi = new pfs::Array2DImpl( width, height );
	  
	  if( eY.yi == NULL )
	    throw pfs::Exception( "could not allocate memory for source exposure" );
      
	  for( int i=0 ; i < size ; i++ )
	    {
	      (*eY.yi)( i ) = (*Y)( i ) * input_multiplier;

	      float val = (*eY.yi)( i );

	      if( val>0.0f )          // discard zero values
		{
		  maxResponse = (maxResponse > val) ? maxResponse : val;
		  minResponse = (minResponse < val) ? minResponse : val;
		}
	    }
	  
	  imgsY.push_back( eY );
	}
    
    // new exposure image
    // collect RGB channels if not in luminance mode or we will apply
    // response to image (not in save response mode)
    if( !opt_luminance )
      {

	/* PFS streams are in XYZ */
	pfs::transformColorSpace( pfs::CS_XYZ, X, Y, Z, pfs::CS_RGB, X, Y, Z );
	
	Exposure eR,eG,eB;
	eR.ti = eG.ti = eB.ti = exp_time;
	eR.yi = new pfs::Array2DImpl( width, height );
	eG.yi = new pfs::Array2DImpl( width, height );
	eB.yi = new pfs::Array2DImpl( width, height );

	if( eR.yi==NULL || eG.yi==NULL || eB.yi==NULL )
	  throw pfs::Exception( "could not allocate memory for source exposure" );
      
	for( int i = 0 ; i < size ; i++ )
	  {

	    (*eR.yi)( i ) = (*X)( i ) * input_multiplier;
	    (*eG.yi)( i ) = (*Y)( i ) * input_multiplier;
	    (*eB.yi)( i ) = (*Z)( i ) * input_multiplier;

	    float maxval = max3( (*eR.yi)( i ),
				 (*eG.yi)( i ),
				 (*eB.yi)( i ) );

	    float minval = min3( (*eR.yi)( i ),
				 (*eG.yi)( i ),
				 (*eB.yi)( i ) );

	    maxResponse  = (maxResponse > maxval) ? maxResponse : maxval;
	    minResponse  = (minResponse < minval) ? minResponse : minval;

	  }

	// add to exposures list
	imgsR.push_back( eR );
	imgsG.push_back( eG );
	imgsB.push_back( eB );    
      }
    
    VERBOSE_STR << "frame #" << frame_no << ", BV=" << atof(exp_str) << endl;
    frame_no++;
    
    pfsio.freeFrame( frame );        
    }

  if( frame_no < 2 )
    throw pfs::Exception( "at least one image required for calibration (check paths in hdrgen script?)" );
    

  // some more info on input frames
  VERBOSE_STR << "registered values: min=" << (int) minResponse
              << " max=" << (int) maxResponse << endl;

  if( opt_minresponse == -1 )
    opt_minresponse = (int) minResponse;

  if( opt_maxresponse == -1 )
    opt_maxresponse = (int) maxResponse;

  if( opt_response != FROM_FILE )
    VERBOSE_STR << "camera response range: min=" << opt_minresponse
                << " max=" << opt_maxresponse << endl;
  
  if( maxResponse >= M )
    throw pfs::Exception( "input value higher than defined number of input levels (adjust the number of bits per pixel)" );

  /* ------------------------------------------------------------------------------------------------ */
  /* ---------------------------- preparation of images done ---------------------------------------- */
  /* ------------------------------------------------------------------------------------------------ */

  /* ------------------------------------------------------------------------------------------------ */
  /* ------------------- preparation of weighting and response functions ---------------------------- */
  /* ------------------------------------------------------------------------------------------------ */

  // weighting function representing confidence in accuracy of acquisition
  float* w = new float [ M ];

  if( w == NULL )
    throw pfs::Exception( "could not allocate memory for weighting function" );

  /* 1.4: currently, weights are always Gaussian */
  /* Composite function works better for extreme cases */
  weightsComposite ( w, M, opt_minresponse, opt_maxresponse, opt_gauss );

  // camera response functions for each channel
  float* Iy = new float [ M ];
  float* Ir = new float [ M ];
  float* Ig = new float [ M ];
  float* Ib = new float [ M ];
  
  if( Iy == NULL || Ib == NULL || Ig == NULL || Ib == NULL )
    throw pfs::Exception( "could not allocate memory for camera responses" );

  // initial responses
  switch( opt_response )
    {
      
      /* ------ Response function from file ------ */
    case FROM_FILE:

      if( opt_luminance )
	{

	  bool loadY_ok = responseLoad ( responseFile, Iy, M );
	  bool loadW_ok = weightsLoad  ( responseFile,  w, M );
	  fclose( responseFile );
	  
	  if( !loadY_ok || !loadW_ok )
	    throw pfs::Exception( "could not load response curve from file" );

	}

    else
      {

	// read camera response from file (and also weights)
	bool loadR_ok = responseLoad ( responseFile, Ir, M );
	bool loadG_ok = responseLoad ( responseFile, Ig, M );
	bool loadB_ok = responseLoad ( responseFile, Ib, M );
	bool loadW_ok = weightsLoad  ( responseFile,  w, M );
	fclose( responseFile );

	if( !loadR_ok || !loadG_ok || !loadB_ok || !loadW_ok )
	  throw pfs::Exception( "could not load response curve from file" );
      }

      opt_calibration = NONE;	  
      break;

      /* ------ Response function linear ------ */
    case LINEAR:
      responseLinear( Iy, M );
      responseLinear( Ir, M );
      responseLinear( Ig, M );
      responseLinear( Ib, M );
      break;

      /* ------ Response function gamma ------ */
    case GAMMA:
      responseGamma( Iy, M );
      responseGamma( Ir, M );
      responseGamma( Ig, M );
      responseGamma( Ib, M );
      break;
      
      /* ------ Response function logarithmic ------ */
    case LOG10:
      responseLog10( Iy, M );
      responseLog10( Ir, M );
      responseLog10( Ig, M );
      responseLog10( Ib, M );
      break;

    default:
      throw pfs::Exception( "camera response not initialized" );
      break;
  }

  
  /* ------------------------------------------------------------------------------------------------ */
  /* --------------- preparation of weighting and response functions done --------------------------- */
  /* ------------------------------------------------------------------------------------------------ */


  // create channels for output
  pfs::Frame   *frame  = pfsio.createFrame( width, height );

  pfs::Channel *Xj     = NULL;
  pfs::Channel *Yj     = NULL;
  pfs::Channel *Zj     = NULL;

  frame -> createXYZChannels( Xj, Yj, Zj );

  
  /* REMOVE: -------------------------- */
  // !!! this currently does more bad than good, relevant command line
  // option is disabled
  if( opt_fillgaps )
    {
      if( opt_luminance )
	{

	  int num    = responseFillGaps( Iy, w, M );

	  float perc = 100.0f * num / M;

	  VERBOSE_STR << "interpolated " << perc << "% of the Y response curve..." << endl;
	}
      else
	{
	  int numr   = responseFillGaps ( Ir, w, M );
	  int numg   = responseFillGaps ( Ig, w, M );
	  int numb   = responseFillGaps ( Ib, w, M );

	  float perc = 100.0f * ( numr + numb + numg) / ( 3 * M );

	  VERBOSE_STR << "interpolated " << perc << "% of the RGB response curve..." << endl;
	}
      
    }
  /* REMOVE end: -------------------------- */
	


  if( opt_calibration != NONE ) 
    {


    }
  else 
    VERBOSE_STR << "self-calibration disabled" << endl;


	


  /* ------------------------------------------------------------------------------------------------ */
  /* ---------------------------------------- Calibration  ------------------------------------------ */
  /* ------------------------------------------------------------------------------------------------ */

  /* counter for saturated pixels */
  long sp = 0;

  if ( opt_calibration == CALIBRATE ) 
    {

      switch ( opt_method ) 
	{
	  
	  /* --------------------- Robertson Calibration --------------------- */
	case ROBERTSON_METHOD:

	  VERBOSE_STR << "automatic self-calibration method: robertson" << endl;
	  
	  if( opt_luminance )
	    {
	      VERBOSE_STR << "recovering Y channel..." << endl;
	      sp = robertson02_getResponse( Yj, imgsY, Iy, w, M);
	    }
	  else
	    {
	      VERBOSE_STR << "recovering R channel..." << endl;
	      sp = robertson02_getResponse( Xj, imgsR, Ir, w, M);
	      
	      VERBOSE_STR << "recovering G channel..." << endl;
	      sp += robertson02_getResponse( Yj, imgsG, Ig, w, M);
	      
	      VERBOSE_STR << "recovering B channel..." << endl;
	      sp += robertson02_getResponse( Zj, imgsB, Ib, w, M);
	      
	      sp /= 3;
	    }

	  break;

	  /* --------------------- Mitsunaga-Nayar Calibration ----------------- */
	case MITSUNAGA_METHOD:
	  
	  VERBOSE_STR << "automatic self-calibration method: mitsunaga & nayar" << endl;

	  if( opt_luminance )
	    throw pfs::Exception( "recovering Y channel not implemented, use robertson calibration." );
	  else 
	    {
					
	      VERBOSE_STR << "Mitsunaga & Nayar ( " << mitsunaga_sample_no << " samples)" << endl;
	  
	      HDRCaptureMitsunaga* mits_calibration = new HDRCaptureMitsunaga();		
				
	      mits_calibration -> capture ( imgsR, imgsG, imgsB, 
					    M, 3,
					    Xj, Yj, Zj, 
					    mitsunaga_sample_no, 
					    Ir, Ig, Ib, 
					    w, 1 );
				
	    }	

	  break;

	default:

	  throw pfs::Exception( "Calibration method not implemented" );

	  break;
	}
	  
    }
  

  /* ------------------------------------------------------------------------------------------------ */
  /* ---------------------------------------- Calibration  done ------------------------------------- */
  /* ------------------------------------------------------------------------------------------------ */

  /* ------------------------------------------------------------------------------------------------ */
  /* ---------------------------------------- HDR computation --------------------------------------- */
  /* ------------------------------------------------------------------------------------------------ */

  /*  if( opt_calibration == NONE ) */
    {

      switch ( opt_method )
	{
	  
	  /* --------------------- Robertson Apply Curve --------------------- */	  
	case ROBERTSON_METHOD:
	  
	  if( opt_luminance )
	    {

	      VERBOSE_STR << "applying response to Y channel..." << endl;
	      sp = robertson02_applyResponse( Yj, imgsY, Iy, w, M);

	    }
	  else
	    {

	      VERBOSE_STR << "applying response to R channel..." << endl;
	      sp = robertson02_applyResponse( Xj, imgsR, Ir, w, M);

	      VERBOSE_STR << "applying response to G channel..." << endl;
	      sp += robertson02_applyResponse( Yj, imgsG, Ig, w, M);

	      VERBOSE_STR << "applying response to B channel..." << endl;
	      sp += robertson02_applyResponse( Zj, imgsB, Ib, w, M);

	      sp /= 3;

	      if( sp > 0 )
		{
		  float perc = ceilf( 100.0f * sp / size );
		  VERBOSE_STR << "saturated pixels found in " << perc << "% of the image!" << endl;
		}

	    }
	  break;

	  /* --------------------- Mitsunaga-Nayar Apply Curve --------------------- */	  
	case MITSUNAGA_METHOD:

	  if( opt_luminance )
	    throw pfs::Exception( "recovering Y channel not implemented, use robertson calibration." );
	  else
	    {
	      HDRCaptureMitsunaga* mits_calibration = new HDRCaptureMitsunaga();		
	      
	      mits_calibration -> capture ( imgsR, imgsG, imgsB, 
					    M, 3,
					    Xj, Yj, Zj, 
					    mitsunaga_sample_no, 
					    Ir, Ig, Ib, 
					    w, 0 );

	      
	      // normalization
	      VERBOSE_STR << "Normalization" << endl;

	      float mmax = -1e30;

	      for( int j = 0; j < size; j++ ) 
		{

		  float maxval = max3( (*Xj)( j ),(*Yj)( j ),(*Zj)( j ) );

		  mmax = ( mmax > maxval ) ? mmax : maxval;

		}

	      for( int j = 0; j < size; j++ ) 
		{
		  (*Xj)( j ) /=  mmax;
		  (*Yj)( j ) /=  mmax;
		  (*Zj)( j ) /=  mmax;								
		}

	    }
	  break;
	  
	}
    }


  /* ------------------------------------------------------------------------------------------------ */
  /* ---------------------------------------- HDR computation done----------------------------------- */
  /* ------------------------------------------------------------------------------------------------ */


  /* ------------------------------------------------------------------------------------------------ */
  /* --------------------------------------------- Save Output -------------------------------------- */
  /* ------------------------------------------------------------------------------------------------ */
  

  // save response curve to a given file
  if( responseSaveFile != NULL )
  {
    if( opt_luminance )
      {
	responseSave( responseSaveFile, Iy, M, "IY" );
	weightsSave ( responseSaveFile,  w, M, "W" );
	fclose( responseSaveFile );
      }
    else
      {
	responseSave( responseSaveFile, Ir, M, "IR" );
	responseSave( responseSaveFile, Ig, M, "IG" );
	responseSave( responseSaveFile, Ib, M, "IB" );
	weightsSave ( responseSaveFile,  w, M, "W");
	fclose( responseSaveFile );
    }
  }


  // output PFS stream with calibrated response in any case
  if( opt_luminance )
    for( int i=0 ; i<size ; i++ )
      {
        // D65 observer XYZ = (95.047,100,108.883);
        (*Xj)(i) = 0.95047f * (*Yj)(i);
        (*Zj)(i) = 1.08883f * (*Yj)(i);
      }
  else
    pfs::transformColorSpace( pfs::CS_RGB, Xj, Yj, Zj, pfs::CS_XYZ, Xj, Yj, Zj );
    
  pfsio.writeFrame( frame, stdout );


  /* ------------------------------------------------------------------------------------------------ */
  /* ----------------------------------------   Save Output done ------------------------------------ */
  /* ------------------------------------------------------------------------------------------------ */

  // clean up memory
  pfsio.freeFrame( frame );
  delete[] w;
  delete[] Iy;
  delete[] Ir;
  delete[] Ig;
  delete[] Ib;

  if( opt_luminance )
    for( int i=0 ; i<imgsY.size() ; i++ )
      delete imgsY[i].yi;
  else
    for( int i=0 ; i<imgsR.size() ; i++ )
    {
      delete imgsR[i].yi;
      delete imgsG[i].yi;
      delete imgsB[i].yi;
    }
}


int main( int argc, char* argv[] )
{
  try 
    {
      pfshdrcalibrate( argc, argv );
    }

  catch( pfs::Exception ex ) 
    {
      fprintf( stderr, PROG_NAME " error: %s\n", ex.getMessage() );
      return EXIT_FAILURE;
    }     
   
  catch( QuietException  ex ) 
    {
      return EXIT_FAILURE;
    }   
     
  return EXIT_SUCCESS;
}
