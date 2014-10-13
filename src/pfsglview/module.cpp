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

#include<stdio.h>

#include "module.h"



Module::Module() {

	isVisible = GL_TRUE;
	pos_x = 10;
	pos_y = 10;
	width = 100;
	height = 40;

	// background color
	winBackgroundColor = new float[4];
	for(int i=0; i<3; i++)
		winBackgroundColor[i] = 1.0f;
	winBackgroundColor[3] = 0.8f;
}



Module::~Module() {

	delete [] winBackgroundColor;
}	



void Module::drawBackground(void) {

	if( isVisible == GL_FALSE)
		return;

	// draw background
	glEnable(GL_BLEND);
	glColor4fv(winBackgroundColor);
	glRecti( 0, 0, width, height);
	glDisable(GL_BLEND);
}	



int Module::redrawStart(void) {

	if( isVisible == GL_FALSE)
		return 1;

	glGetIntegerv( GL_VIEWPORT, param); // remember old vieport

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	//glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glViewport( pos_x, pos_y, width, height);
	glOrtho( 0, width, 0, height, -1.0f, 1.0f);

	drawBackground();

	return 0;
}



void Module::redrawEnd(void) {

	if( isVisible == GL_FALSE)
		return;

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glViewport( param[0], param[1], param[2], param[3]); // restore viewport
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}



void Module::setPosition( int _x, int _y) {
	pos_x = _x;
	pos_y = _y;
}



void Module::setSize( int _width, int _height) {
	width = _width;
	height = _height;
}



int Module::getWidth() {
	return width ;
}



int Module::getHeight() {
	return height ;
}



void Module::setWidth(int _width){
	width = _width;
}



void Module::setHeight(int _height){
	height = _height;
}



void Module::setVisible(bool _isVisible) {

	isVisible = _isVisible;
}



bool Module::getVisible(void) {
	return isVisible;
}



// not used anymore
/*
int RMGLWin::processSelection(int xCoord, int yCoord) {

	if( isVisible == GL_FALSE)
		return 1;

	redrawStart();

	// Hits counter and viewport martix
	GLint hits, viewp[4];
	// Get actual viewport
	glGetIntegerv(GL_VIEWPORT, viewp);

	#define BUFFER_SIZE 64
	// Table for selection buffer data
	GLuint selectionBuffer[BUFFER_SIZE];
	// Prepare selection buffer
	glSelectBuffer(BUFFER_SIZE, selectionBuffer);

	// Change rendering mode
	glRenderMode(GL_SELECT);
	// Initializes the Name Stack
	glInitNames();
	// Push 0 (at least one entry) Onto the Stack
	glPushName(0);
	// Set new projection matrix as a box around xPos, yPos
	glLoadIdentity();

	int hh = glutGet(GLUT_WINDOW_HEIGHT);
	// Picking matrix at position xCoord, windowSize - yCoord (fliped window Y axis)
	// and size of 4 units in depth
	gluPickMatrix(xCoord, hh - yCoord, 4, 4, viewp);

	glOrtho(0.0f, viewp[2], 0.0f, viewp[3], -10.0f, 10.0f); // last 1.0 -1.0 it's enough
	drawBackground(); // draw only picked parts

	int ret = 0;

	hits = glRenderMode(GL_RENDER);

	if(hits > 0) {
		ret = 1;
	}

	redrawEnd();

	return ret;
}
 */









