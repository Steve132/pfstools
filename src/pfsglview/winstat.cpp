/**
 * @brief PFS library - additional utilities
 * 
 * This file is a part of PFSTOOLS package.
 * ---------------------------------------------------------------------- 
 * Copyright (C) 2006 Radoslaw Mantiuk
 * 
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * ---------------------------------------------------------------------- 
 * 
 * @author Radoslaw Mantiuk, <radoslaw.mantiuk@gmail.com>
 *
 */

#include <stdio.h>
#include <string.h>

#include "glenv.h"
#include "winstat.h"


WinStat::WinStat() : RMGLWin() {

	colR = -1;
	colG = -1;
	colB = -1;
	frameNo = -1;
	channel = "";
	rawPosX = -1;
	bZoom = true;
}


WinStat::~WinStat() {

}	

void WinStat::redraw(void) {

	if( RMGLWin::redrawStart())
		return;
		
	char ss[200];
	glColor3f( 0.0f, 0.0f, 0.0f);
		
	glRasterPos2f( 5.0f, 29.0f);

	if( frameNo != -1) {
		sprintf( ss, "Frame: %d", frameNo);
		for (int i = 0; i < (int) strlen(ss); i++) 
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, ss[i]); 	
	}
	
	//sprintf( ss, "Max freq: %.0f  %s", lumMax, mappingMode);
	
	char szoom[10];
	if( bZoom == true)
		strcpy( szoom, "ZOOM");
	else
		strcpy( szoom, "PAN");
	
	sprintf( ss, "  %s  %s   %s", mappingMode, channel, szoom);
	for (int i = 0; i < (int) strlen(ss); i++) 
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, ss[i]); 

	glRasterPos2f( 5.0f, 17.0f);
	if( colR >= 0) {
		sprintf( ss, "screen: %d,%d : %d,%d,%d", pixelX, pixelY, colR, colG, colB);
		for (int i = 0; i < (int) strlen(ss); i++) 
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, ss[i]); 
	}
	
	glRasterPos2f( 5.0f, 5.0f);		
	if( rawPosX >=0) {
		sprintf( ss, "data: %d,%d : %.6f,%.6f,%.6f", rawPosX, rawPosY, rawX, rawY, rawZ);
		for (int i = 0; i < (int) strlen(ss); i++) 
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, ss[i]); 	
	}

	
	RMGLWin::redrawEnd();
}

void WinStat::setPixelData( int x, int y, int r, int g, int b) {

	pixelX = x;
	pixelY = y;
	colR = r;
	colB = b;
	colG = g;
}

void WinStat::setMaxFreq(float val) {
	lumMax = val;
}

void WinStat::setMapping(const char* sval) {
	mappingMode = sval;
}

void WinStat::setFrameNo(int no) { 
	frameNo = no;
}	

void WinStat::setChannel(const char* ch) { 
	channel = ch;
}	

void WinStat::setRawData( int x, int y, float X, float Y, float Z) {

	rawPosX = x;
	rawPosY = y;
	rawX = X;
	rawY = Y;
	rawZ = Z;
}

void WinStat::setBZoom(bool bb) { 
	bZoom = bb;
}	



