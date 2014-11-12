/**
 * @brief Create an HDR image directly from JPEG files
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
 * @author Wenxiang Ying <wilson@mpi-inf.mpg.de>
 *
 * $Id: pfsmergehdr.cpp,v 1.1 2007/03/28 15:18:29 gkrawczyk Exp $
 */

#include <config.h>

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>


using namespace std;

//--- standard PFS stuff
#define PROG_NAME "pfsmergehdr"
bool verbose = false;


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


int main( int argc, char* argv[] )
{
  static struct option cmdLineOptions[] = {
    { "help", no_argument, NULL, 'h' },
    { "verbose", no_argument, NULL, 'v' },
    { NULL, 0, NULL, 0 }
  };

  int optionIndex = 0;
  while( 1 )
  {
    int c = getopt_long (argc, argv, "hv", cmdLineOptions, &optionIndex);
    if( c == -1 ) break;
    switch( c )
    {
    case 'v':
      verbose = true;
      break;
    case 'h':
    case '?':
    case ':':
      printHelp();
      return -1;
    }
  } 

  //--- verbose information and load initialization data
  // use this stream to output useful information for a user, they will
  //see it if they lunch the program with -v option. do not output
  //information in any other way
  VERBOSE_STR << "verbose information"  << endl;

  return 0;
}
