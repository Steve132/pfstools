/**
 * @brief Concatenate frames in PFS stream
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
#include <iostream>
#include <getopt.h>
#include <fcntl.h>
#include <stdlib.h>

#define PROG_NAME "pfscat"
#define UNSP INT_MAX
#define HORIZONTAL 0
#define VERTICAL 1
#define MIN 0
#define MAX 1
#define PREV_MIN 2
#define PREV_MAX 3
#define X 0
#define Y 1
#define Z 2

using namespace std;

enum just {J_MIN, J_MAX, J_CENTER};

class QuietException
{
};


static struct option cmdLineOptions[] = {
    {"help", no_argument, NULL, 'h'},
    {"horizontal", no_argument, NULL, 'H'},
    {"vertical", no_argument, NULL, 'V'},
    {"justify", required_argument, NULL, 'j'},
    {"R", required_argument, NULL, 'R'},
    {"G", required_argument, NULL, 'G'},
    {"B", required_argument, NULL, 'B'},
    {"Y", required_argument, NULL, 'Y'},
    {NULL, 0, NULL, 0}

};


void printHelp()
{
fprintf( stderr, PROG_NAME ": \n"
	"\t[--vertical]\n"
	"\t[--horizontal]\n"
	"\t[-j <type>]\n"
	"\t[-R <val>]\n"
	"\t[-G <val>]\n"
	"\t[-B <val>]\n"
	"\t[-Y <val>]\n"
	"\t[--help]\n"
	"See man page for more information.\n");
}


void justification (enum just just, int inSize, int prevSize, int outSize, int *out)
{
switch (just) {

    case J_CENTER:
	
	if(inSize<prevSize)	{
    
    	    int diff=outSize-inSize;
    	    out[MIN]=diff/2;
    	    out[MAX]=outSize-(diff/2)-1;
    	    if (diff%2) out[MAX]--;
    	    out[PREV_MIN]=0;
    	    out[PREV_MAX]=outSize-1;
    	    }
    	else {
    	
	    int diff=outSize-prevSize;
    	    out[PREV_MIN]=diff/2;
    	    out[PREV_MAX]=outSize-(diff/2)-1;
    	    if (diff%2) out[PREV_MAX]--;
    	    out[MIN]=0;
    	    out[MAX]=outSize-1;
    	    }
	break;

    case J_MIN:	    
	    
	out[MIN]=0;
	out[MAX]=inSize-1;
	out[PREV_MIN]=0;
	out[PREV_MAX]=prevSize-1;
    	break;

    case J_MAX:	   
	    
	out[MIN]=outSize-inSize;
	out[MAX]=outSize-1;
	out[PREV_MIN]=outSize-prevSize;
	out[PREV_MAX]=outSize-1;
	break;
    }
}


float clamp(float val, float min, float max)
{
float out;
out = (val<min) ? min : val;
out = (val>max) ? max : val;
return out;
}


void pfscat (int argc, char* argv[])
{
pfs::DOMIO pfsio;

static const char optString[] =  "hHVj:R:G:B:Y:";
enum just just=J_CENTER; // type of justification
int alignment=UNSP; //vertical or horizontal alignment of frames in output image

float R=1e-4, G=1e-4, B=1e-4, luminance=1.0; // background color in RGB or luminance mode
float XYZ[3];
bool optLuminanceMode=false, RGB=false, optLuminance=false;
bool pipes = true;

int optionIndex=0;

while (1)
    {
    int c=getopt_long(argc, argv,optString, cmdLineOptions, &optionIndex);
    if(c==-1) break;

    switch(c)
	{
	case 'h':
	    printHelp();
	    throw QuietException();
	    break;
	case 'H':
    	    if (alignment==VERTICAL) throw pfs::Exception("You cannot specify both horizontal and vertical alignment");
    	    else alignment=HORIZONTAL;
    	    break;
	case 'V':
    	    if (alignment==HORIZONTAL) throw pfs::Exception("You cannot specify both horizontal and vertical alignment");
            else alignment=VERTICAL;
	    break;
	case 'j':
            if (!strcmp(optarg,"min")) just=J_MIN;
    	    else if (!strcmp(optarg,"max")) just=J_MAX;
	    else if (!strcmp(optarg,"center")) just=J_CENTER;
    	    else throw pfs::Exception("Unknown type of justification. Accepted types: 'min', 'max' or 'center')");
	    break;
	case 'R':
    	    R=atof(optarg);
	    if(R<=(1e-4)) R=1e-4;
	    RGB=true;
    	    break;
	case 'G':
    	    G=atof(optarg);
	    if(G<=(1e-4)) G=1e-4;
	    RGB=true;
    	    break;
	case 'B':
    	    B=atof(optarg);
	    if(B<=(1e-4)) B=1e-4;
	    RGB=true;
    	    break;
	case 'Y':
    	    luminance=atof(optarg);
	    if(luminance<=(1e-4)) luminance=1e-4;
	    optLuminance=true;
	    break;
	}
    }

if (alignment==UNSP) throw pfs::Exception("You must specify an alignment - --horizontal or --vertical option");

pfs::FrameFileIterator it ( argc, argv, "rb", NULL, NULL, optString, cmdLineOptions);
int pipe_no = 0;
pfs::FrameFile * ff;
ff = (pfs::FrameFile *) malloc ((pipe_no+1) * sizeof(pfs::FrameFile));

while (1) {
    ff[pipe_no] = it.getNextFrameFile();
    if (ff[pipe_no].fh == NULL) break; // no more files
    pipe_no++;
    ff=(pfs::FrameFile *) realloc(ff, (pipe_no+1)*sizeof(pfs::FrameFile));
    }
    
if(pipe_no == 0) { // no named pipes
    pipe_no = 1;
    pipes = false;
}

//color transformation
if(optLuminance && !RGB) {
    R=1;
    G=1;
    B=1;
    }

XYZ[X]=luminance*(0.4124*R + 0.3576*G + 0.1805*B);
XYZ[Y]=luminance*(0.2126*R + 0.7152*G + 0.0722*B);
XYZ[Z]=luminance*(0.0193*R + 0.1192*G + 0.9505*B);
for (int i=0; i<3; i++) XYZ[i]=clamp(XYZ[i], 1e-4, 100000000.0);

bool finish = false;
int frame_no=0;
int tRow=0, bRow=0, lCol=0, rCol=0, prevtRow=0, prevbRow=0, prevlCol=0, prevrCol=0;
int inWidth=0, inHeight=0, outWidth=0, outHeight=0, prevWidth=0, prevHeight=0;
pfs::Frame *inFrame, *outFrame, *prevFrame;
pfs::Channel *inX, *inY, *inZ, *outX, *outY, *outZ, *prevX, *prevY, *prevZ;

while (1) {
    
    if(pipes) {
	inWidth=0;   inHeight=0; 
	outWidth=0;  outHeight=0; 
	prevWidth=0; prevHeight=0;
	}
    
    for (int i=0; i<pipe_no; i++) {

	if(pipes) inFrame = pfsio.readFrame(ff[i].fh);    
	else inFrame = pfsio.readFrame(stdin);
	
	if (inFrame==NULL) {
	    finish = true;	
	    break; // no more frames
	    }

	inFrame->getXYZChannels(inX, inY, inZ);
	inWidth=inFrame->getWidth();
	inHeight=inFrame->getHeight();

        if (frame_no) { // save previous frame
	
	    prevWidth=outWidth;
            prevHeight=outHeight;
	    prevFrame=pfsio.createFrame(prevWidth, prevHeight);
	    if (!optLuminanceMode) prevFrame->createXYZChannels(prevX, prevY, prevZ);
    	    else prevY=prevFrame->createChannel("Y");
	    int size=prevWidth*prevHeight;
	
	    for (int i=0; i<size; i++) {
		(*prevY)(i)=(*outY)(i);
        	if (!optLuminanceMode) {
		    (*prevX)(i)=(*outX)(i);
    		    (*prevZ)(i)=(*outZ)(i);
		    }
    		}
	    pfsio.freeFrame(outFrame);
	    }
        
	int leftRight[4], topBottom[4];
	
        if (alignment==VERTICAL) {
	
	    if(inWidth>outWidth) outWidth=inWidth;
	    outHeight+=inHeight;
	
	    justification (just, inWidth, prevWidth, outWidth, leftRight);

	    lCol=leftRight[MIN];
	    rCol=leftRight[MAX];
	    tRow=outHeight-inHeight;
	    bRow=outHeight-1;
	    prevlCol=leftRight[PREV_MIN];
	    prevrCol=leftRight[PREV_MAX];
	    prevtRow=0;
	    prevbRow=prevHeight-1;
	    }

	else if (alignment==HORIZONTAL) {
	
	    outWidth+=inWidth;
	    if(inHeight>outHeight) outHeight=inHeight;

	    justification (just, inHeight, prevHeight, outHeight, topBottom);
	    lCol=outWidth-inWidth;
	    rCol=outWidth-1;
	    tRow=topBottom[MIN];
	    bRow=topBottom[MAX];
	    prevlCol=0;
	    prevrCol=prevWidth-1;
	    prevtRow=topBottom[PREV_MIN];
	    prevbRow=topBottom[PREV_MAX];
	    }
     
	if(inX != NULL) { //XYZ mode

	    if (frame_no==0) optLuminanceMode=false; // first frame
	
	    if(!optLuminanceMode) {
	
		outFrame = pfsio.createFrame(outWidth, outHeight);
		outFrame->createXYZChannels(outX, outY, outZ);
    
		for (int i=0; i<outHeight; i++)
		    for (int j=0; j<outWidth; j++) {
	    
		    if (i>=tRow && i<=bRow && j>=lCol && j<=rCol) {
	    		(*outX)(j,i)=(*inX)(j-lCol,i-tRow);
    			(*outY)(j,i)=(*inY)(j-lCol,i-tRow);
    			(*outZ)(j,i)=(*inZ)(j-lCol,i-tRow);
    			}
	
		    else if (i>=prevtRow && i<=prevbRow && j>=prevlCol && j<=prevrCol) {
    			(*outX)(j,i)=(*prevX)(j-prevlCol,i-prevtRow);
    			(*outY)(j,i)=(*prevY)(j-prevlCol,i-prevtRow);
    			(*outZ)(j,i)=(*prevZ)(j-prevlCol,i-prevtRow);
    			}
		
		    else {
			(*outX)(j, i)=XYZ[X];
			(*outY)(j, i)=XYZ[Y];
			(*outZ)(j, i)=XYZ[Z];
			}
		    
		    }
		}
	    else {
		outFrame = pfsio.createFrame(prevWidth, prevHeight);
		outY=outFrame->createChannel("Y");
		int size=prevWidth*prevHeight;
		for (int i=0; i<size; i++ ) (*outY)(i)=(*prevY)(i);
		outHeight=prevHeight;
		outWidth=prevWidth;
		}
	    }
	else if (inY=inFrame->getChannel("Y")) {// luminance mode
    	
	    if (frame_no==0) optLuminanceMode=true; // first frame
	
	    if (optLuminanceMode) {
		outFrame = pfsio.createFrame(outWidth, outHeight);
		outY=outFrame->createChannel("Y");
	
		for (int i=0; i<outHeight; i++)
		    for (int j=0; j<outWidth; j++) {
	    
			if (i>=tRow && i<=bRow && j>=lCol && j<=rCol) 
	    		    (*outY)(j,i)=(*inY)(j-lCol,i-tRow);
    		    
			else if (i>=prevtRow && i<=prevbRow && j>=prevlCol && j<=prevrCol) 
    			    (*outY)(j,i)=(*prevY)(j-prevlCol,i-prevtRow);
    		
			else (*outY)(j, i)=luminance;
			}
		}
	    else {
		outFrame = pfsio.createFrame(prevWidth, prevHeight);
		outFrame->createXYZChannels(outX, outY, outZ);
		int size=prevWidth*prevHeight;
		for (int i=0; i<size; i++ ) {
		    (*outX)(i)=(*prevX)(i);
	    	    (*outY)(i)=(*prevY)(i);
	    	    (*outZ)(i)=(*prevZ)(i);
		    }
		outHeight=prevHeight;
		outWidth=prevWidth;
		}
	    }
	else throw pfs::Exception("Missing X, Y, Z channels in the PFS stream");

        pfsio.freeFrame(inFrame);
	if(frame_no) pfsio.freeFrame(prevFrame);
	frame_no++;
   
	} // end for

    if(finish) break;
    if(pipes) {
	pfsio.writeFrame(outFrame, stdout);
	pfsio.freeFrame(outFrame);
	frame_no=0;
	}
    } // end while

if(!pipes) {
    pfsio.writeFrame(outFrame, stdout);
    pfsio.freeFrame(outFrame);
    }

for (int i=0; i<pipe_no; i++) it.closeFrameFile(ff[i]);
free(ff);
}


int main (int argc, char* argv[])
{
try {
    pfscat(argc, argv);
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
