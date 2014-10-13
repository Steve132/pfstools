/**
 * @brief Retime animation sequence
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
 * @author Rafal Mantiuk, <mantiuk@gmail.com>
 *
 * $Id: pfsretime.cpp,v 1.1 2013/01/29 22:15:01 rafm Exp $
 */

#include <config.h>

#include <iostream>
#include <sstream>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>

#include <pfs.h>

#define PROG_NAME "pfsretime"

class QuietException
{
};


void applyGamma( pfs::Array2D *array, const float exponent, const float multiplier );

void printHelp()
{
    fprintf( stderr, PROG_NAME " [--in-fps <val>] [--out-fps <val>] [--speedup <val>] [--verbose] [--help]\n"
             "See man page for more information.\n" );
}

void retimeFrames( int argc, char* argv[] )
{
    pfs::DOMIO pfsio;

    float in_fps = 30.f;
    float out_fps = 30.f;
    float speedup = -1.f;

    bool verbose = false;

    static struct option cmdLineOptions[] = {
        { "help", no_argument, NULL, 'h' },
        { "verbose", no_argument, NULL, 'v' },
        { "in-fps", required_argument, NULL, 'i' },
        { "out-fps", required_argument, NULL, 'o' },
        { "speedup", required_argument, NULL, 's' },
        { NULL, 0, NULL, 0 }
    };

    int optionIndex = 0;
    while( 1 ) {
        int c = getopt_long (argc, argv, "i:o:s:hv", cmdLineOptions, &optionIndex);
        if( c == -1 ) break;
        switch( c ) {
        case 'h':
            printHelp();
            throw QuietException();
        case 'v':
            verbose = true;
            break;
        case 's':
            speedup = (float)strtod( optarg, NULL );
            break;
        case 'i':
            in_fps = (float)strtod( optarg, NULL );
            break;
        case 'o':
            out_fps = (float)strtod( optarg, NULL );
            break;
        case '?':
            throw QuietException();
        case ':':
            throw QuietException();
        }
    }

    std::ostringstream fps_tag;
    bool first_frame = true;
    float in_pos = 0;
    bool get_new_frame = true;
    pfs::Frame *frame = NULL;

    while( true ) {

        if( in_pos >= 1 ) {
            get_new_frame = true;
            in_pos -= 1;
        }

        if( get_new_frame ) {
            frame = pfsio.readFrame( stdin );
            if( frame == NULL ) break; // No more frames


            if( first_frame ) {

                const char *fps_str = frame->getTags()->getString("FPS");
                if( fps_str != NULL ) {
                    in_fps = (float)strtod( fps_str, NULL );
                }

                if( speedup != -1.f ) {
                    out_fps = in_fps;
                    in_fps /= speedup;
                }

                VERBOSE_STR << "in-fps: " << in_fps << " out-fps: " << out_fps<< std::endl;

                fps_tag << out_fps;

                first_frame = false;

            }

            frame->getTags()->setString("FPS", fps_tag.str().c_str() );

            get_new_frame = false;

        }

        if( in_pos >= 1 ) // skip input frames
            continue;

        pfsio.writeFrame( frame, stdout );

        in_pos += in_fps / out_fps;

    }

    if( frame != NULL )
        pfsio.freeFrame( frame );

}


int main( int argc, char* argv[] )
{
    try {
        retimeFrames( argc, argv );
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
