/**
 * @brief Write files in OpenEXR format
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
 * @author Grzegorz Krawczyk, <krawczyk@mpi-sb.mpg.de>
 * @author Rafal Mantiuk, <mantiuk@mpi-sb.mpg.de>
 *
 * $Id: pfsoutexr.cpp,v 1.4 2013/12/21 19:42:28 rafm Exp $
 */

#include <config.h>

#include <cstdlib>

#include <iostream>
#include <string>

#include <stdio.h>
#include <pfs.h>
#include <getopt.h>

#include <ImfHeader.h>
#include <ImfChannelList.h>
#include <ImfOutputFile.h>
#include <ImfRgbaFile.h>
#include <ImfStringAttribute.h>
#include <ImfStandardAttributes.h>

using namespace Imf;
using namespace Imath;

#define PROG_NAME "pfsoutexr"

class QuietException 
{
};

#define min(x,y) ( (x)<(y) ? (x) : (y) )

void printHelp()
{
  fprintf( stderr, PROG_NAME " [--compression <method>] [--float32] [--clamp-halfmax] [--linear] [--verbose] [--help]\n"
    "See man page for more information.\n" );
}

bool isColorChannel( pfs::Channel *ch )
{
  return !strcmp( ch->getName(), "X" ) ||
    !strcmp( ch->getName(), "Y" ) ||
    !strcmp( ch->getName(), "Z" );

}

// Change from pfs channel name to OpenEXR channel name
const char *exrChannelName( const char *pfsChannelName )
{
  if( !strcmp( pfsChannelName, "DEPTH" ) ) {
    return "Z";
  }
  return pfsChannelName;
}


void writeFrames( int argc, char* argv[] )
{
  Compression exrCompression = PIZ_COMPRESSION;
  bool verbose = false;
  bool keepXYZ = false;
  bool fixHalfMax = false;
  bool float32 = false;
  bool clampHalfMax = false;

  // Parse command line parameters
  static struct option cmdLineOptions[] = {
    { "help", no_argument, NULL, 'h' },
    { "verbose", no_argument, NULL, 'v' },
    { "compression", required_argument, NULL, 'c' },
    { "keep-xyz", no_argument, NULL, 'k' },
    { "fix-halfmax", no_argument, NULL, 'f' },
    { "clamp-halfmax", no_argument, NULL, 'p' },
    { "float32", no_argument, NULL, '3' },
    { "linear", no_argument, NULL, 'l' },
    { NULL, 0, NULL, 0 }
  };
  static const char optstring[] = "hvc:kf3pl";

  pfs::FrameFileIterator it( argc, argv, "wb", NULL, NULL,
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
    case 'k':
      keepXYZ = true;
      break;
    case 'f':
      std::cerr << PROG_NAME << " warning: fix-halfmax is the default behavior starting from 2.0.3. This option is depreciated and will be removed in the future.";
      break;
    case '3':
      float32 = true;
      break;
    case 'p':
      clampHalfMax = true;
      break;
    case 'c':
      if( !strcasecmp( optarg, "NO" ) ) {
        exrCompression = NO_COMPRESSION;
      } else if( !strcasecmp( optarg, "RLE" ) ) {
        exrCompression = RLE_COMPRESSION;
      } else if( !strcasecmp( optarg, "ZIPS" ) ) {
        exrCompression = ZIPS_COMPRESSION;
      } else if( !strcasecmp( optarg, "ZIP" ) ) {
        exrCompression = ZIP_COMPRESSION;
      } else if( !strcasecmp( optarg, "PIZ" ) ) {
        exrCompression = PIZ_COMPRESSION;
      } else if( !strcasecmp( optarg, "PXR24" ) ) {
        exrCompression = PXR24_COMPRESSION;
      } else {
        throw pfs::Exception( "Unknown compression method. Possible values: NO, RLE, ZIPS, ZIP, PIZ, PXR24" );     
      }
      break;
    case 'l':
      std::cerr << PROG_NAME << " warning: linearize option ignored for an HDR output!"
                << std::endl;
      break;
    case '?':
      throw QuietException();
    case ':':
      throw QuietException();
    }
  }

//   // FrameFileIterator should process only non-option parameters, but
//   // it assumes that the first parameter is a program name, therefore
//   // argv+optind-1
//   int newArgc = argc-optind+1;

  if( verbose ) {
//	&& keepXYZ )
    //fprintf( stderr, PROG_NAME ": keeping XYZ channels untouched\n" );
	
	fprintf( stderr, PROG_NAME ": Color channel precision: %s\n", float32 ? "32-bit float" : "16-bit float" );
	
   }
  
  pfs::DOMIO pfsio;
 
  bool firstFrame = true;

  half *halfRGB[3] = { NULL, NULL, NULL }; //*halfG = NULL, *halfB = NULL;
//  float *floatRGB[3] = { NULL, NULL, NULL }; //*floatG = NULL, *floatB = NULL;
  size_t pix_count_prev = 0;
  
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
    
    // OpenEXR library can not use FILE, therefore we do not need FH
    it.closeFrameFile( ff );

    VERBOSE_STR << "writing file (HDR) '" << ff.fileName << "'" << std::endl;
    
    
    // Write the frame to EXR file
    {

//      bool colorChannelsToHalf = false;
	  bool storeRGBChannels = false;
      pfs::Channel *R= NULL, *G = NULL, *B = NULL; // for clarity of the code
//      if( !keepXYZ ) {  
		pfs::Channel *X, *Y, *Z;
        frame->getXYZChannels( X, Y, Z );
        if( X != NULL ) {       // Has color
          R = X;
          G = Y;
          B = Z;
          pfs::transformColorSpace( pfs::CS_XYZ, X, Y, Z,
            pfs::CS_RGB, R, G, B );
//          colorChannelsToHalf = true;
		  storeRGBChannels = true;
        }
  //    }

      Header header( frame->getWidth(), frame->getHeight(),
        1,                      // aspect ratio
        Imath::V2f (0, 0),      // screenWindowCenter
        1,                      // screenWindowWidth
        INCREASING_Y,           // lineOrder
        exrCompression
        );

      // Define channels in Header
      {                         
        pfs::ChannelIteratorPtr cit( frame->getChannelIterator() );
        while( cit->hasNext() ) {
          pfs::Channel *ch = cit->getNext();
          if( storeRGBChannels && isColorChannel( ch ) ) // Skip color channels
            continue;

          const char *channelName = exrChannelName( ch->getName() );
          
          header.channels().insert( channelName, Channel(FLOAT) );        
        }      		
        if( storeRGBChannels ) {
			if( float32 ) {
				header.channels().insert( "R", Channel(FLOAT) );     
				header.channels().insert( "G", Channel(FLOAT) );		  
				header.channels().insert( "B", Channel(FLOAT) );
			} else {
				header.channels().insert( "R", Channel(HALF) );     
				header.channels().insert( "G", Channel(HALF) );		  
				header.channels().insert( "B", Channel(HALF) );
			}
        }
      }

      // Copy tags to attributes
      {
        pfs::TagIteratorPtr it( frame->getTags()->getIterator() );
        
        while( it->hasNext() ) {
          const char *tagName = it->getNext();
//           fprintf( stderr, "Tag: %s = %s\n", tagName, frame->getTags()->getString( tagName ) );         
          header.insert( tagName, StringAttribute(frame->getTags()->getString( tagName )) );
        }

        //Copy all channel tags
        pfs::ChannelIteratorPtr cit( frame->getChannelIterator() );
        while( cit->hasNext() ) {
          pfs::Channel *ch = cit->getNext();

          pfs::TagIteratorPtr tit( ch->getTags()->getIterator() );        
          while( tit->hasNext() ) {
            const char *tagName = tit->getNext();
	    std::string channelTagName = ch->getName();
            channelTagName += ":";
            channelTagName += tagName;
            header.insert( channelTagName.c_str(), StringAttribute(ch->getTags()->getString( tagName )) );
          }
          
        }        
      }

      FrameBuffer frameBuffer;      

      // Create channels in FrameBuffer
      {
        pfs::ChannelIterator *it = frame->getChannels();
        while( it->hasNext() ) {
          pfs::Channel *ch = it->getNext();
          if( storeRGBChannels && isColorChannel( ch ) )
            continue;
          frameBuffer.insert( exrChannelName( ch->getName() ), // name
            Slice( FLOAT,			// type	 
              (char*)ch->getRawData(), // base	 
              sizeof(float) * 1,	// xStride
              sizeof(float) * frame->getWidth()) ); // yStride        
        }
        
        if( storeRGBChannels ) {

			static const char *rgb_strings[3] = { "R", "G", "B" };			
			bool whiteLuminanceUsed = false;
			if( float32 ) {			
				frameBuffer.insert( rgb_strings[0], // name
					Slice( FLOAT,			// type	 
					(char*)R->getRawData(), // base	 
					sizeof(float) * 1,	// xStride
					sizeof(float) * frame->getWidth()) ); // yStride        

				frameBuffer.insert( rgb_strings[1], // name
					Slice( FLOAT,			// type	 
					(char*)G->getRawData(), // base	 
					sizeof(float) * 1,	// xStride
					sizeof(float) * frame->getWidth()) ); // yStride        

				frameBuffer.insert( rgb_strings[2], // name
					Slice( FLOAT,			// type	 
					(char*)B->getRawData(), // base	 
					sizeof(float) * 1,	// xStride
					sizeof(float) * frame->getWidth()) ); // yStride        
					
			} else { // Half-float
				const size_t pix_count = frame->getWidth()*frame->getHeight();			
				if( pix_count_prev != pix_count ) { // Reallocate memory if needed			
					for( int cc=0; cc<3; cc++ ) {								
						delete [] halfRGB[cc];
						halfRGB[cc] = new half[pix_count];
					}
					pix_count_prev = pix_count;
				}

				for( int cc=0; cc<3; cc++ ) {
					frameBuffer.insert( rgb_strings[cc],		// name
						Slice( HALF,			// type	 
						(char*)halfRGB[cc],		// base	 
						sizeof(half) * 1,	// xStride
						sizeof(half) * frame->getWidth()) ); // yStride
				}			

	//          Check if pixel values do not exceed maximum HALF value            
				bool maxHalfExceeded = false;
				float maxValue = -1;
				if( !clampHalfMax ) {
					for( int i = 0; i < pix_count; i++ ) {
						if( (*R)(i) > maxValue ) maxValue = (*R)(i);
						if( (*G)(i) > maxValue ) maxValue = (*G)(i);
						if( (*B)(i) > maxValue ) maxValue = (*B)(i);
					}
					maxHalfExceeded = maxValue > HALF_MAX;
				}
          
				if( maxHalfExceeded && verbose && !clampHalfMax )
					fprintf( stderr, PROG_NAME " warning: Some pixels exceed maximum value that can be stored in an OpenEXR file (maximum value of HALF-16 float). The values are scaled and the \"WhiteLuminance\" tag is added to preserve those values.\n" );
          
				if( maxHalfExceeded ) {
		//          Rescale and copy pixels to half-type buffers
					float scaleFactor = HALF_MAX/maxValue;
					for( size_t i = 0; i < pix_count; i++ ) {
						halfRGB[0][i] = (half)((*R)(i)*scaleFactor);
						halfRGB[1][i] = (half)((*G)(i)*scaleFactor);
						halfRGB[2][i] = (half)((*B)(i)*scaleFactor);          
					}
					// Store scale factor as WhileLuminance standard sttribute
					// in order to restore absolute values later
					addWhiteLuminance( header, 1/scaleFactor );
					whiteLuminanceUsed = true;
				} else {
		//          Copy pixels to half-type buffers
					for( size_t i = 0; i < pix_count; i++ ) {
						halfRGB[0][i] = min( (*R)(i), HALF_MAX );
						halfRGB[1][i] = min( (*G)(i), HALF_MAX );
						halfRGB[2][i] = min( (*B)(i), HALF_MAX );          
					}
				}
			}
			if( luminanceTag != NULL && !strcmp( luminanceTag, "ABSOLUTE" ) && !whiteLuminanceUsed )			
			{
				// Use WhiteLuminance tag to signalize absolute values
				addWhiteLuminance( header, 1 );
			}
			
        }
      }

      OutputFile file(ff.fileName, header);
      
      file.setFrameBuffer (frameBuffer);

      file.writePixels( frame->getHeight() );
      
    }
    pfsio.freeFrame( frame );
  }
  
  for( int cc=0; cc<3; cc++ ) {								
	delete [] halfRGB[cc];
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
  catch (const std::exception &exc) // OpenEXR exception
  {
    fprintf( stderr, PROG_NAME " error: %s\n", exc.what() );
    return EXIT_FAILURE;
  }
  catch( QuietException  ex ) {
    return EXIT_FAILURE;
  }
  
  return EXIT_SUCCESS;
}
