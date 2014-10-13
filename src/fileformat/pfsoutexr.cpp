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
  fprintf( stderr, PROG_NAME " [--compression <method>] [--keep-xyz] [--fix-halfmax] [--linear] [--verbose] [--help]\n"
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

  // Parse command line parameters
  static struct option cmdLineOptions[] = {
    { "help", no_argument, NULL, 'h' },
    { "verbose", no_argument, NULL, 'v' },
    { "compression", required_argument, NULL, 'c' },
    { "keep-xyz", no_argument, NULL, 'k' },
    { "fix-halfmax", no_argument, NULL, 'f' },
    { "linear", no_argument, NULL, 'l' },
    { NULL, 0, NULL, 0 }
  };
  static const char optstring[] = "c:kf";

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
      fixHalfMax = true;
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

  if( verbose && keepXYZ )
    fprintf( stderr, PROG_NAME ": keeping XYZ channels untouched\n" );
  
  pfs::DOMIO pfsio;
 
  bool firstFrame = true;

  half *halfR = NULL, *halfG = NULL, *halfB = NULL;
  
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

      bool colorChannelsToHalf = false;
      pfs::Channel *R= NULL, *G = NULL, *B = NULL; // for clarity of the code
      if( !keepXYZ ) {  
        pfs::Channel *X, *Y, *Z;
        frame->getXYZChannels( X, Y, Z );
        if( X != NULL ) {       // Has color
          R = X;
          G = Y;
          B = Z;
          pfs::transformColorSpace( pfs::CS_XYZ, X, Y, Z,
            pfs::CS_RGB, R, G, B );
          colorChannelsToHalf = true;
        }
      }

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
          if( colorChannelsToHalf && isColorChannel( ch ) )
            continue;

          const char *channelName = exrChannelName( ch->getName() );
          
          header.channels().insert( channelName, Channel(FLOAT) );        
        }      
        if( colorChannelsToHalf ) {
          header.channels().insert( "R", Channel(HALF) );     
          header.channels().insert( "G", Channel(HALF) );		  
          header.channels().insert( "B", Channel(HALF) );
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
          if( colorChannelsToHalf && isColorChannel( ch ) )
            continue;
          frameBuffer.insert( exrChannelName( ch->getName() ), // name
            Slice( FLOAT,			// type	 
              (char*)ch->getRawData(), // base	 
              sizeof(float) * 1,	// xStride
              sizeof(float) * frame->getWidth()) ); // yStride        
        }
        
        if( colorChannelsToHalf ) {

          if( firstFrame ) {
            halfR = new half[frame->getWidth()*frame->getHeight()];
            halfG = new half[frame->getWidth()*frame->getHeight()];
            halfB = new half[frame->getWidth()*frame->getHeight()];
            firstFrame = false;
          }
          
          frameBuffer.insert( "R",				// name
            Slice( HALF,			// type	 
              (char*)halfR,		// base	 
              sizeof(half) * 1,	// xStride
              sizeof(half) * frame->getWidth()) ); // yStride

          frameBuffer.insert( "G",				// name
            Slice( HALF,			// type	 
              (char*)halfG,		// base	 
              sizeof(half) * 1,	// xStride
              sizeof(half) * frame->getWidth()) ); // yStride

          frameBuffer.insert( "B",				// name
            Slice( HALF,			// type	 
              (char*)halfB,		// base	 
              sizeof(half) * 1,	// xStride
              sizeof(half) * frame->getWidth()) ); // yStride

          int pixelCount = frame->getHeight()*frame->getWidth();
          
//          Check if pixel values do not exceed maximum HALF value            
          float maxValue = -1;
          for( int i = 0; i < pixelCount; i++ ) {
            if( (*R)(i) > maxValue ) maxValue = (*R)(i);
            if( (*G)(i) > maxValue ) maxValue = (*G)(i);
            if( (*B)(i) > maxValue ) maxValue = (*B)(i);
          }

          bool maxHalfExceeded = maxValue > HALF_MAX;
          
          if( maxHalfExceeded && !fixHalfMax )
            fprintf( stderr, PROG_NAME " warning: Some pixels exceed maximum value that can be stored in an OpenEXR file (maximum value of HALF-16 float) and will be clamped to that maximum. Use --fix-halfmax switch to rescale the data to the valid range.\n" );
          
          if( fixHalfMax && maxHalfExceeded ) {
//          Rescale and copy pixels to half-type buffers
            float scaleFactor = HALF_MAX/maxValue;
            for( int i = 0; i < pixelCount; i++ ) {
              halfR[i] = (half)((*R)(i)*scaleFactor);
              halfG[i] = (half)((*G)(i)*scaleFactor);
              halfB[i] = (half)((*B)(i)*scaleFactor);          
            }
            // Store scale factor as WhileLuminance standard sttribute
            // in order to restore absolute values later
            addWhiteLuminance( header, 1/scaleFactor );
          } else {
//          Copy pixels to half-type buffers
            for( int i = 0; i < pixelCount; i++ ) {
              halfR[i] = min( (*R)(i), HALF_MAX );
              halfG[i] = min( (*G)(i), HALF_MAX );
              halfB[i] = min( (*B)(i), HALF_MAX );          
            }
            if( luminanceTag != NULL && !strcmp( luminanceTag, "ABSOLUTE" ) )
            {
              addWhiteLuminance( header, 1 );
            }
          }
        }
      }

      OutputFile file(ff.fileName, header);
      
      file.setFrameBuffer (frameBuffer);

      file.writePixels( frame->getHeight() );
      
    }
    pfsio.freeFrame( frame );
  }
  
  delete[] halfR;
  delete[] halfG;
  delete[] halfB;    
  
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
