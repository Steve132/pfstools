/**
 * @brief Display Adaptive TMO
 *
 * From:
 * Rafal Mantiuk, Scott Daly, Louis Kerofsky.
 * Display Adaptive Tone Mapping.
 * To appear in: ACM Transactions on Graphics (Proc. of SIGGRAPH'08) 27 (3)
 * http://www.mpi-inf.mpg.de/resources/hdr/datmo/
 *
 * This file is a part of PFSTMO package.
 * ---------------------------------------------------------------------- 
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
 * @author Rafal Mantiuk, <mantiuk@gmail.com>
 *
 * $Id: pfstmo_mantiuk08.cpp,v 1.19 2013/12/28 14:00:54 rafm Exp $
 */

//#include <config.h>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>
#include <fcntl.h>
#include <iostream>
#include <memory>

#include <sys/time.h>

#include <pfs.h>

#include "pfstmo_config.h"

#include "display_adaptive_tmo.h"


#define PROG_NAME "pfstmo_mantiuk08"

class QuietException 
{
};


class Timing
{
  timeval t1;
public:
  Timing() {
    gettimeofday(&t1, NULL);
  }

  void report( const char *activity )
  {
    timeval t2;
    gettimeofday(&t2, NULL);
    unsigned int t = (t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec);

    fprintf( stderr, "Activity %s took %g seconds.\n", activity,  (float)t / 1000000.f );
  }
};

using namespace std;

const char *temp_file_2pass = "datmo_tone_curves.tmp";
bool read_tone_curve( FILE *fh, datmoToneCurve *tc, double **x_scale );

void printHelp()
{
  fprintf( stderr, PROG_NAME " (" PACKAGE_STRING ") : \n"
    "\t[--display-function <df-spec>] [--display-size=<size-spec>]\n"
    "\t[--color-saturation <float>] [--contrast-enhancement <float>]\n"
    "\t[--white-y=<float>]\n"
    "\t[--verbose] [--quiet] [--help]\n"
    "See man page for more information.\n" );
}


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



void tmo_mantiuk08(int argc, char * argv[])
{

  //--- default tone mapping parameters;
  float contrast_enhance_factor = 1.f;
  float saturation_factor = 1.f;
  float white_y = -2.f;
  float fps = 25;  
  int temporal_filter = 0;
  int itmax = 200;
  float tol = 1e-3;
  DisplayFunction *df = NULL;
  DisplaySize *ds = NULL;
  const char *output_tc = NULL;
  datmoVisualModel visual_model = vm_full;
  double scene_l_adapt = 1000;  

  //--- process command line args

  df = createDisplayFunctionFromArgs( argc, argv );
  if( df == NULL )
    df = new DisplayFunctionGGBA( "lcd" );

  ds = createDisplaySizeFromArgs( argc, argv );
  if( ds == NULL )
    ds = new DisplaySize( 30.f, 0.5f );
  
  static struct option cmdLineOptions[] = {
    { "help", no_argument, NULL, 'h' },
    { "verbose", no_argument, NULL, 'v' },
    { "contrast-enhancement", required_argument, NULL, 'e' },
    { "color-saturation", required_argument, NULL, 'c' },
    { "white-y", required_argument, NULL, 'y' },
    { "fps", required_argument, NULL, 'f' },
    { "output-tone-curve", required_argument, NULL, 'o' },
    { "visual-model", required_argument, NULL, 'm' },
    { "scene-y-adapt", required_argument, NULL, 'a' },
    { "quiet", no_argument, NULL, 'q' },    
    { NULL, 0, NULL, 0 }
  };

  int optionIndex = 0;
  while( 1 ) {
    int c = getopt_long (argc, argv, "vhe:c:y:t:o:qm:f:a:", cmdLineOptions, &optionIndex);
    if( c == -1 ) break;
    switch( c ) {
    case 'h':
      printHelp();
      throw QuietException();
    case 'v':
      verbose = true;
      break;
    case 'q':
      quiet = true;
      break;
    case 'a':
      if( !strcmp( optarg, "auto" ) )
        scene_l_adapt = -1;
      else {        
        scene_l_adapt = (float)strtod( optarg, NULL );
        if( scene_l_adapt <= 0.0f )
        throw pfs::Exception("incorrect scane adaptation luminance. The value must be greater than 0.");
      }      
      break;
    case 'e':
      contrast_enhance_factor = (float)strtod( optarg, NULL );
      if( contrast_enhance_factor <= 0.0f )
        throw pfs::Exception("incorrect contrast enhancement factor, accepted value must be a positive number");
      break;
    case 'c':
      saturation_factor = (float)strtod( optarg, NULL );
      if( saturation_factor < 0.0f || saturation_factor > 2.0f )
        throw pfs::Exception("incorrect saturation factor, accepted range is (0..2)");
      break;
    case 'm':
    {      
      char *saveptr;
      char *token;
      visual_model = vm_none;
      
      token = strtok_r( optarg, ",:", &saveptr );
      while( token != NULL ) {
        
        if( !strcmp( token, "none" ) ) {
          visual_model = vm_none;
        } else if( !strcmp( token, "full" ) ) {      
          visual_model = vm_full;
        } else if( !strcmp( token, "luminance_masking" ) ) {          
          visual_model |= vm_luminance_masking;
        } else if( !strcmp( token, "contrast_masking" ) ) {          
          visual_model |= vm_contrast_masking;
        } else if( !strcmp( token, "csf" ) ) {          
          visual_model |= vm_csf;
        } else 
          throw pfs::Exception("Unrecognized visual model");
        
        token = strtok_r( NULL, ",:", &saveptr );
      }      
    }
    
      break;
      
    case 'y':
      if( !strcmp( optarg, "none" ) )
        white_y = -1;
      else
        white_y = (float)strtod( optarg, NULL );
      if( white_y < 0.0f )
        throw pfs::Exception("incorrect white-y value. The value must be greater than 0");
      break;
    case 'f':
      fps = strtod( optarg, NULL );
      if( fps != 25 && fps != 30 && fps != 60 )
        throw pfs::Exception("Only 3 frame-per-seconds values are supported: 25, 30 and 60.");
      break;
    case 'o':
      output_tc = optarg;
      break;
    case '?':
      throw QuietException();
    case ':':
      throw QuietException();
    }
  }

  if( verbose ) {
    df->print( stderr );
    ds->print( stderr );
    fprintf( stderr, "Frames-per-second: %g\n", fps );    
    fprintf( stderr, "Contrast masking: %d\n", (bool)(visual_model & vm_contrast_masking) );
    fprintf( stderr, "Luminance masking: %d\n", (bool)(visual_model & vm_luminance_masking) );
    fprintf( stderr, "CSF: %d\n", (bool)(visual_model & vm_csf) );
    fprintf( stderr, "Scane adaptation luminance: %g (-1 means auto)\n", scene_l_adapt );  
  }

  Timing tm_entire;

  FILE *tc_FH = NULL;
  if( output_tc != NULL ) {
    tc_FH = fopen( output_tc, "w" );
    if( tc_FH == NULL )
      throw pfs::Exception("cannot open file for writing tone-curve.");
  }

  
  datmoTCFilter rc_filter( fps, log10(df->display(0)), log10(df->display(1)) );
  pfs::DOMIO pfsio;

  size_t frame_no = 0;
  while( true ) {
    pfs::Frame *frame = pfsio.readFrame( stdin );
    if( frame == NULL )
      break;

    pfs::Channel *inX, *inY, *inZ;
	
    frame->getXYZChannels(inX, inY, inZ);
    int cols = frame->getWidth();
    int rows = frame->getHeight();

    const char *file_name = frame->getTags()->getString( "FILE_NAME" );
    if( file_name == NULL )
      sprintf( frame_name, "frame #%d", (int)frame_no );
    else {
      int len = strlen( file_name );
      if( len > FRAME_NAME_MAX ) // In case file name is too long
        len = FRAME_NAME_MAX-3;
      strcpy( frame_name, "..." );
      strncpy( frame_name+3, file_name + strlen( file_name ) - len, len+1 );
    }
    

    {
      pfs::Array2DImpl R( cols, rows );
  
      pfs::transformColorSpace( pfs::CS_XYZ, inX, inY, inZ, pfs::CS_RGB, inX, &R, inZ );

      if( white_y == -2.f ) {       // If not overriden by command line options
        const char *white_y_str = frame->getTags()->getString( "WHITE_Y" );
        if( white_y_str != NULL ) {
          white_y = strtof( white_y_str, NULL );
          if( white_y == 0 ) {
            white_y = -1;
            fprintf( stderr, PROG_NAME ": warning - wrong WHITE_Y in the input image" );        
          }
        }
      }      
      if( verbose && frame_no == 0 ) {
        fprintf( stderr, "Luminance factor of the reference white: " );
        if( white_y < 0 )
          fprintf( stderr, "not specified\n" );
        else
          fprintf( stderr, "%g\n", white_y );      
      }

      const char *lum_data = frame->getTags()->getString("LUMINANCE");
      if( lum_data != NULL && !strcmp( lum_data, "DISPLAY" ) && frame_no == 0 )
        fprintf( stderr, PROG_NAME " warning: input image should be in linear (not gamma corrected) luminance factor units. Use '--linear' option with pfsin* commands.\n" );

      Timing tm_cond_dens;
        std::auto_ptr<datmoConditionalDensity> C = datmo_compute_conditional_density( cols, rows, inY->getRawData(), progress_report );
        if( C.get() == NULL )
          throw pfs::Exception("failed to analyse the image");
	tm_cond_dens.report( "Conditional density" );

	Timing tm_comp_tone_curve;
        int res;
        datmoToneCurve *tc = rc_filter.getToneCurvePtr();        
        res = datmo_compute_tone_curve( tc, C.get(), df, ds, contrast_enhance_factor, white_y, progress_report, visual_model, scene_l_adapt );
        if( res != PFSTMO_OK )
          throw pfs::Exception( "failed to compute a tone-curve" );    

        datmoToneCurve *tc_filt = rc_filter.filterToneCurve();
	tm_comp_tone_curve.report( "Computing a tone-cuve" );
        

        {

	  Timing tm_tonecurve;
        int res;
        res = datmo_apply_tone_curve_cc( inX->getRawData(), R.getRawData(), inZ->getRawData(), cols, rows,
          inX->getRawData(), R.getRawData(), inZ->getRawData(), inY->getRawData(), tc_filt,
          df, saturation_factor );
        if( res != PFSTMO_OK )
          throw pfs::Exception( "failed to tone-map an image" );

	tm_tonecurve.report( "Apply tone-curve" );

        if( tc_FH != NULL ) {
          for( int i=0; i < tc_filt->size; i++ )
            fprintf( tc_FH, "%d,%g,%g,%g\n", (int)frame_no, tc_filt->x_i[i], tc_filt->y_i[i], df->inv_display( (float)pow( 10, tc_filt->y_i[i] ) ) );
        }
    
//    int res;
//    res = datmo_tonemap( inX->getRawData(), R.getRawData(), inZ->getRawData(), cols, rows,
//      inX->getRawData(), R.getRawData(), inZ->getRawData(), inY->getRawData(),
//    df, ds, contrast_enhance_factor, saturation_factor, white_y, progress_report );
        
        progress_report( 100 );        

        pfs::transformColorSpace( pfs::CS_RGB, inX, &R, inZ, pfs::CS_XYZ, inX, inY, inZ );
        frame->getTags()->setString("LUMINANCE", "DISPLAY");
  
        pfsio.writeFrame( frame, stdout );
      }
        
      pfsio.freeFrame(frame);
    }
    

    frame_no++;

  }

      
    if( tc_FH != NULL )
      fclose( tc_FH );
    
  delete df;
  delete ds;

  tm_entire.report( "Entire operation" );
}

bool read_tone_curve( FILE *fh, datmoToneCurve *tc, double **x_scale )
{
  int size, frame_no, read;
  read = fscanf( fh, "%d,%d\n", &frame_no, &size );
  if( read != 2 )
    return false;
  
  if( *x_scale == NULL )
    *x_scale = new double[size];

  tc->init( size, *x_scale );
  
  for( int i=0; i < size; i++ ) {
    float x, y;
    read = fscanf( fh, "%f,%f\n", &x, &y );
    if( read != 2 )
      throw pfs::Exception( "missing data in the 2-pass file"  );
    (*x_scale)[i] = x;
    tc->y_i[i] = y;
  }
  return true;
}



int main( int argc, char* argv[] )
{
  try {
    tmo_mantiuk08( argc, argv );
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

