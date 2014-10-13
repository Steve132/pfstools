/**
 * @brief Cut a rectangle out of images in PFS stream
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
 * @author Dorota Zdrojewska, <dzdrojewska@wi.ps.pl>
 *
 */

#include <pfs.h>
#include <climits>
#include <cstdlib>
#include <iostream>
#include <getopt.h>


#define PROG_NAME "pfscut"
#define UNSP INT_MAX
#define MIN 0
#define MAX 1
#define SIZE 2


using namespace std;


class QuietException
{
};


static struct option cmdLineOptions[] = {
    {"help", no_argument, NULL, 'h'},
    {"left", required_argument, NULL, 'l'},
    {"right", required_argument, NULL, 'r'},
    {"top", required_argument, NULL, 't'},
    {"bottom", required_argument, NULL, 'b'},
    {"width", required_argument, NULL, 'W'},
    {"height", required_argument, NULL, 'H'},
    {NULL, 0, NULL, 0}
};


void printHelp()
{
fprintf( stderr, PROG_NAME ": \n" 
	"\t[--left <val>]\n"
	"\t[--right <val>]\n"
	"\t[--top <val>]\n"
	"\t[--bottom <val>]\n"
	"\t[--width <val>]\n"
	"\t[--height <val>]\n"
	"\t[--help]\n"
        "\t[x_ul y_ul x_br y_br]\n"
	"See man page for more information.\n");
}


void calcBorders(int min, int max, int size, int inSize, int flag, int* out)
{
if (min<0 || max<0) throw pfs::Exception("You cannot cut negative number of pixels");
if (min>=inSize && min!=UNSP) throw pfs::Exception("You cannot cut number of pixels greater than size of an image");
if (max>=inSize && max!=UNSP) throw pfs::Exception("You cannot cut number of pixels greater than size of an image");
if (size>inSize && size!=UNSP) throw pfs::Exception("You cannot specify size greater than size of an input image");
	
 if (min!=UNSP && max!=UNSP && size!=UNSP) {
    if (flag) throw pfs::Exception("You cannot specify all width, left and right options");
    else throw pfs::Exception("You cannot specify all height, top and bottom options");
 }

if (min!=UNSP && max!=UNSP && size==UNSP) {
  if((min+max)>=inSize) {
    	if (flag) throw pfs::Exception("Too large value for left or right option");
        else throw pfs::Exception("Too large value for top or bottom option");
  }
    out[MIN]=min;
    out[MAX]=inSize-max-1;
    out[SIZE]=inSize-min-max;
    }

if (min!=UNSP && max==UNSP && size==UNSP) {
    out[MIN]=min;
    out[MAX]=inSize-1;
    out[SIZE]=inSize-min;
    }

if (min==UNSP && max!=UNSP && size==UNSP) {
    out[MIN]=0;
    out[MAX]=inSize-max-1;
    out[SIZE]=inSize-max;
    }

if (min!=UNSP && max==UNSP && size!=UNSP) {
  if((min+max)>inSize) {
      if(flag) throw pfs::Exception("Too large value for left or width option");
      else throw pfs::Exception("Too large value for top or height option");
  }
    out[MIN]=min;
    out[MAX]=size+min-1;
    out[SIZE]=size;
    }

if (min==UNSP && max!=UNSP && size!=UNSP) {
  if((max+size)>inSize) {
       if(flag) throw pfs::Exception("Too large value for right or width option");
      else throw pfs::Exception("Too large value for bottom or height option");
  }
    out[MIN]=inSize-max-size;
    out[MAX]=inSize-max-1;
    out[SIZE]=size;
    }

if (min==UNSP && max==UNSP && size!=UNSP) {
    int diff=inSize-size;
    out[MIN]=diff/2;
    out[MAX]=inSize-(diff/2)-1;
    if(diff%2) out[MAX]--;
    out[SIZE]=size;
    }

if (min==UNSP && max==UNSP && size==UNSP) {
    out[MIN]=0;
    out[MAX]=inSize-1;
    out[SIZE]=inSize;
    }

}


void pfscut (int argc, char* argv[])
{
pfs::DOMIO pfsio;

//numbers of pixels to cut from each border of an image 
int left=UNSP, right=UNSP, top=UNSP, bottom=UNSP; 
int width=UNSP, height=UNSP; //size of an output image
int x_ul=UNSP, y_ul=UNSP, x_br=UNSP, y_br=UNSP;
    
int optionIndex=0;

while (1) {

    int c=getopt_long(argc, argv, "hl:r:t:b:W:H:", cmdLineOptions, &optionIndex);
    if(c==-1) break;
    
    switch(c) {
	case 'h':  
	    printHelp();
	    throw QuietException();
	    break;
	case 'l':  
	    left=atoi(optarg);
	    break;
	case 'r':  
	    right=atoi(optarg);
	    break;
	case 't':
	    top=atoi(optarg);
	    break;
	case 'b':
	    bottom=atoi(optarg);
	    break;
	case 'W':
	    width=atoi(optarg);
	    if (width<=0) throw pfs::Exception("Size of an output image cannot be negative or equal to 0");
	    break;
	case 'H':
	    height=atoi(optarg);
	    if (height<=0) throw pfs::Exception("Size of an output image cannot be negative or equal to 0");
	    break;
	}
    }



if( optind != argc ) {
  if( optind != (argc - 4) )
    throw pfs::Exception( "You must specify x and y coordinates of both upper left and lower right corner" );
  x_ul = strtol( argv[optind++], NULL, 10 );
  y_ul = strtol( argv[optind++], NULL, 10 );
  x_br = strtol( argv[optind++], NULL, 10 );
  y_br = strtol( argv[optind++], NULL, 10 );
}

while (1) {

    pfs::Frame *inFrame = pfsio.readFrame(stdin);
    if (inFrame==NULL) break;  // no more frames

    int inWidth=inFrame->getWidth();
    int inHeight=inFrame->getHeight();

    int leftRight[3], topBottom[3];
    if( x_ul != UNSP ) {
      leftRight[MIN] = x_ul;
      leftRight[MAX] = x_br;
      leftRight[SIZE] = x_br - x_ul + 1;
      topBottom[MIN] = y_ul;
      topBottom[MAX] = y_br;
      topBottom[SIZE] = y_br - y_ul + 1;
    } else {
      //calculate edge columns and rows of an input image to be in an output image  
      calcBorders(left, right, width, inWidth, 1, leftRight);
      calcBorders(top, bottom, height, inHeight, 0, topBottom);
    }
    
    if( topBottom[SIZE] < 1 || leftRight[SIZE] < 1 )
      throw pfs::Exception( "The resulting image has zero or negative size" );
    
    if( topBottom[MIN] < 0 || leftRight[MIN] < 0 ||
      topBottom[MAX] >= inHeight || leftRight[MAX] >= inWidth )
      throw pfs::Exception( "The specified coordinates are outsize the image boundaries" );
    
    int lCol=leftRight[MIN];
    int rCol=leftRight[MAX];
    int tRow=topBottom[MIN];
    int bRow=topBottom[MAX];

    int outWidth=leftRight[SIZE];
    int outHeight=topBottom[SIZE];
  
    pfs::Frame *outFrame = pfsio.createFrame(outWidth, outHeight);

    pfs::ChannelIterator *it = inFrame->getChannels();
    
    while (it->hasNext()) {
	
	pfs::Channel *inCh = it->getNext();
	pfs::Channel *outCh = outFrame->createChannel(inCh->getName());
	
	for (int i=tRow; i<=bRow; i++)
	    for (int j=lCol; j<=rCol; j++) 
		(*outCh)(j-lCol, i-tRow)=(*inCh)(j,i);
	
    }
    
    pfs::copyTags(inFrame, outFrame);
    pfsio.writeFrame(outFrame, stdout);
    pfsio.freeFrame(inFrame);
    pfsio.freeFrame(outFrame);

    }

}


int main (int argc, char* argv[])
{
try {
    pfscut(argc, argv);
    }

catch (pfs::Exception ex) {
    fprintf (stderr, PROG_NAME " error: %s\n", ex.getMessage());
    return EXIT_FAILURE;
    }

catch (QuietException ex) {
    return EXIT_FAILURE;
    }
    
return EXIT_SUCCESS;

}
