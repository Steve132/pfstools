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
 * @author Oliver Barth <obarth@mpi-inf.mpg.de>
 */

#include <stdio.h>
#include <string.h>

#include "m_status.h"


/*
 *
 */
SC_Status::SC_Status() : Module() {

	valueR = -1;
	valueG = -1;
	valueB = -1;
	frameNo = -1;
	channel = "";
	rawPositionX = -1;
	zoom_scale = 1 ;


	winBackgroundColor = new float[4];
	for(int i=0; i<3; i++)
		winBackgroundColor[i] = 0.0f;
	winBackgroundColor[3] = 0.75f;

}



SC_Status::~SC_Status() {
}



void SC_Status::redraw(void) {

	if( Module::redrawStart())
		return;

	char buffer[256];
	glColor3f(0.5f, 1.0f, 0.5f);

	glRasterPos2f( 5.0f, 41.0f);

	if( frameNo != -1) {
		sprintf( buffer, "Frame: %d", frameNo);
		for (int i = 0; i < (int) strlen(buffer); i++) 
			glutBitmapCharacter(GLUT_BITMAP_8_BY_13, buffer[i]);
	}

	sprintf( buffer, "  %s  %s   %s", mappingMode, channel, navigationMode);
	for (int i = 0; i < (int) strlen(buffer); i++) 
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, buffer[i]);

	glRasterPos2f( 5.0f, 29.0f);
	sprintf( buffer, "zoom scale: %.6f", zoom_scale);
	for (int i = 0; i < (int) strlen(buffer); i++)
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, buffer[i]);

	glRasterPos2f( 5.0f, 17.0f);
	if( valueR >= 0) {
		sprintf( buffer, "screen    : %4d, %4d : %8d, %8d, %8d", positionX, positionY, valueR, valueG, valueB);
		for (int i = 0; i < (int) strlen(buffer); i++) 
			glutBitmapCharacter(GLUT_BITMAP_8_BY_13, buffer[i]);
	}

	glRasterPos2f( 5.0f, 5.0f);		
	if( rawPositionX >=0) {
		sprintf( buffer, "data      : %4d, %4d : %5.2e, %5.2e, %5.2e", rawPositionX, rawPositionY, rawValueX, rawValueY, rawValueZ);
		for (int i = 0; i < (int) strlen(buffer); i++) 
			glutBitmapCharacter(GLUT_BITMAP_8_BY_13, buffer[i]);
	}

	Module::redrawEnd();
}



void SC_Status::setPixelData( int x, int y, int r, int g, int b) {

	positionX = x;
	positionY = y;
	valueR = r;
	valueG = g;
	valueB = b;
}



void SC_Status::setRawData( int x, int y, float X, float Y, float Z) {
	rawPositionX = x;
	rawPositionY = y;
	rawValueX = X;
	rawValueY = Y;
	rawValueZ = Z;
}



void SC_Status::setMaxFreq(float _max_frequency) {
	lumMax = _max_frequency;
}



void SC_Status::setMapping(const char* _mappingMode) {
	mappingMode = _mappingMode;
}



void SC_Status::setNavMode(const char* _navigationMode) {
	navigationMode = _navigationMode;
}



void SC_Status::setFrameNo(int _frameNo) {
	frameNo = _frameNo;
}	



void SC_Status::setChannel(const char* _channel) {
	channel = _channel;
}	



void SC_Status::setZoomScale(float zoom_scale) {
	this->zoom_scale = zoom_scale;
}



