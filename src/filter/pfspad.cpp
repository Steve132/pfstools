/**
 * @brief Add borders to images in PFS stream
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


#define PROG_NAME "pfspad"
#define UNSP INT_MAX
#define MIN 0
#define MAX 1
#define SIZE 2
#define X 0
#define Y 1
#define Z 2

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
    {"R", required_argument, NULL, 'R'},
    {"G", required_argument, NULL, 'G'},
    {"B", required_argument, NULL, 'B'},
    {"Y", required_argument, NULL, 'Y'}, 
    {NULL, 0, NULL, 0}
};


void printHelp()
{
fprintf(stderr, PROG_NAME ":\n"
	"\t[--left <val>]\n"
	"\t[--right <val>]\n"
	"\t[--top <val>]\n"
	"\t[--bottom <val>]\n"
	"\t[--width <val>]\n"
	"\t[--height <val>\n"
	"\t[--R <val> \n"
	"\t[--G <val> \n"
	"\t[--B <val> \n"
	"\t[--Y <val>\n"
	"\t[--help]\n"
	"See man page for more information.\n");
}


void calcBorders (int min, int max, int size, int inSize, int flag, int *out)
{
if (min<0 || max<0) throw pfs::Exception("You cannot add negative number of pixels");
if (size<inSize) throw pfs::Exception("You cannot specify size smaller than size of an input image");

 if (min!=UNSP && max!=UNSP && size!=UNSP) {
    if (flag) throw pfs::Exception("You cannot specify all width, left and right options");
    else throw pfs::Exception("You cannot specify all height, top and bottom options");
 }

if (min!=UNSP && max!=UNSP && size==UNSP) {
    out[SIZE]=inSize+min+max;
    out[MIN]=min;
    out[MAX]=out[SIZE]-max-1;
    }

if (min!=UNSP && max==UNSP && size==UNSP) {
    out[SIZE]=inSize+min;
    out[MIN]=min;
    out[MAX]=out[SIZE]-1;
    }

if (min==UNSP && max!=UNSP && size==UNSP) {
    out[SIZE]=inSize+max;
    out[MIN]=0;
    out[MAX]=out[SIZE]-max-1;
    }

if (min!=UNSP && max==UNSP && size!=UNSP) {
  if(size<(inSize+min)) {
	if (flag) throw pfs::Exception("Wrong value for left or width option");
	else throw pfs::Exception("Wrong value for top or height option");
  }
    out[SIZE]=size;
    out[MIN]=min;
    out[MAX]=inSize+min-1;
    }

if (min==UNSP && max!=UNSP && size!=UNSP) {
  if(size<(inSize+max)) {
	if (flag) throw pfs::Exception("Wrong value for right or width option");
	else throw pfs::Exception("Wrong value for bottom or height option");
  }
    out[SIZE]=size;
    out[MIN]=out[SIZE]-max-inSize;
    out[MAX]=out[SIZE]-max-1;
    }

if (min==UNSP && max==UNSP && size!=UNSP) {
    out[SIZE]=size;
    int diff=out[SIZE]-inSize;
    out[MIN]=diff/2;
    out[MAX]=out[SIZE]-(diff/2)-1;
    if(diff%2) out[MAX]--;
    }

if (min==UNSP && max==UNSP && size==UNSP) {
    out[SIZE]=inSize;
    out[MIN]=0;
    out[MAX]=out[SIZE]-1;
    }

}

float clamp(float val, float min, float max)
{
float out;
out = (val<min) ? min : val;
out = (val>max) ? max : val;
return out;
}

void pfspad (int argc, char* argv[])
{

pfs::DOMIO pfsio;

int left=UNSP, right=UNSP, top=UNSP, bottom=UNSP, //size of borders (in pixels)
    width=UNSP, height=UNSP; // size of an output image
float R=1e-4, G=1e-4, B=1e-4, luminance=1.0; // color of borders in RGB or luminance mode
float XYZ[3];
bool optLuminanceMode=false, RGBMode=false;

int optionIndex=0;

while (1)
    {
    int c=getopt_long(argc, argv, "hl:r:t:b:W:H:R:G:B:Y:", cmdLineOptions, &optionIndex);
    if(c==-1) break;
    
    switch(c) 
	{
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
	case 'R':
	    R=atof(optarg);
	    RGBMode=true;
	    if(R<=(1e-4)) R=1e-4;
	    break;
	case 'G':
	    G=atof(optarg);
	    RGBMode=true;
	    if(G<=(1e-4)) G=1e-4;
	    break;
	case 'B':
	    B=atof(optarg);
	    RGBMode=true;
	    if(B<=(1e-4)) B=1e-4;
	    break;
	case 'Y':
	    luminance=atof(optarg);
	    optLuminanceMode = true;
	    if(luminance<=(1e-4)) luminance=1e-4;
	    break;
	}
    }
    
if(optLuminanceMode && !RGBMode) {
    R=1;
    G=1;
    B=1;
    }
    
//transform RGB colors to YXZ color space and multiply by luminance
XYZ[X]=luminance*(0.4124*R + 0.3576*G + 0.1805*B);
XYZ[Y]=luminance*(0.2126*R + 0.7152*G + 0.0722*B);
XYZ[Z]=luminance*(0.0193*R + 0.1192*G + 0.9505*B);

XYZ[X]=clamp(XYZ[X],1e-4,100000000.0);
XYZ[Y]=clamp(XYZ[Y],1e-4,100000000.0);
XYZ[Z]=clamp(XYZ[Z],1e-4,100000000.0);

pfs::Channel *inX, *inY, *inZ, *outX, *outY, *outZ;

while (1) {

    pfs::Frame *inFrame = pfsio.readFrame(stdin);
    if (inFrame==NULL) break; // no more frames

    inFrame->getXYZChannels(inX, inY, inZ);

    int inWidth=inFrame->getWidth();
    int inHeight=inFrame->getHeight();

    //calculate borders to add
    int leftRight[3], topBottom[3];
    calcBorders(left, right, width, inWidth, 1, leftRight);
    calcBorders(top, bottom, height, inHeight, 0, topBottom);
    
    int lCol=leftRight[MIN];
    int rCol=leftRight[MAX];
    int tRow=topBottom[MIN];
    int bRow=topBottom[MAX];

    int outWidth=leftRight[SIZE];
    int outHeight=topBottom[SIZE];

    pfs::Frame *outFrame = pfsio.createFrame(outWidth, outHeight);

    if (inX != NULL) {  //XYZ mode
	outFrame->createXYZChannels(outX, outY, outZ);

	for (int i=0; i<outHeight; i++)
    	    for (int j=0; j<outWidth; j++) 
    		if (i>=tRow && i<=bRow && j>=lCol && j<=rCol) {
		    (*outX)(j, i)=(*inX)(j-lCol,i-tRow);
	            (*outY)(j, i)=(*inY)(j-lCol,i-tRow);
		    (*outZ)(j, i)=(*inZ)(j-lCol,i-tRow);
		    }
		else {
		    (*outX)(j, i)=XYZ[X];
	    	    (*outY)(j, i)=XYZ[Y];
		    (*outZ)(j, i)=XYZ[Z];
		    }
	}
    else if ( (inY=inFrame->getChannel("Y")) != NULL ) {  // only luminance
	outY=outFrame->createChannel("Y");	
	
	for (int i=0; i<outHeight; i++)
    	    for (int j=0; j<outWidth; j++) 
    		if (i>=tRow && i<=bRow && j>=lCol && j<=rCol) 
	            (*outY)(j, i)=(*inY)(j-lCol,i-tRow);
		else (*outY)(j, i)=luminance;
		
	}
    else throw pfs::Exception("Missing X, Y, Z channels in the PFS stream");
	
    pfs::copyTags(inFrame, outFrame);
    pfsio.writeFrame(outFrame, stdout);
    pfsio.freeFrame(inFrame);
    pfsio.freeFrame(outFrame);
    
    }
}


int main (int argc, char* argv[])
{
try {
    pfspad(argc, argv);
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
