/**
 * @brief Tone-mapping optimized for backward-compatible HDR image and video compression
 *
 * From:
 * Mai, Z., Mansour, H., Mantiuk, R., Nasiopoulos, P., Ward, R., & Heidrich, W. 
 * Optimizing a tone curve for backward-compatible high dynamic range image and video compression. 
 * IEEE Transactions on Image Processing, 20(6), 1558 – 1571. doi:10.1109/TIP.2010.2095866, 2011
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
#include <pfstmo.h>

#include <config.h>

#include "compression_tmo.h"

#define PROG_NAME "pfstmo_mai11"

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

void printHelp()
{
  fprintf( stderr, PROG_NAME " (" PACKAGE_STRING ") : \n"
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



void tmo_mai11(int argc, char * argv[])
{

  //--- default tone mapping parameters;

  //--- process command line args

  
  static struct option cmdLineOptions[] = {
    { "help", no_argument, NULL, 'h' },
    { "verbose", no_argument, NULL, 'v' },
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
    }
  }


  Timing tm_entire;

  pfs::DOMIO pfsio;

  CompressionTMO tmo;

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


      tmo.tonemap( inX->getRawData(), R.getRawData(), inZ->getRawData(), cols, rows,
        inX->getRawData(), R.getRawData(), inZ->getRawData(), inY->getRawData(),
		progress_report );
        
        progress_report( 100 );        

        pfs::transformColorSpace( pfs::CS_RGB, inX, &R, inZ, pfs::CS_XYZ, inX, inY, inZ );
        frame->getTags()->setString("LUMINANCE", "DISPLAY");
  
        pfsio.writeFrame( frame, stdout );
     
        
      pfsio.freeFrame(frame);
    }
    

    frame_no++;

  }

  tm_entire.report( "Entire operation" );
}




int main( int argc, char* argv[] )
{
  try {
    tmo_mai11( argc, argv );
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

