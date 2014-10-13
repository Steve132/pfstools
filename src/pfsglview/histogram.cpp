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
 *
 */


#include <stdio.h>
#include <vector>
#include <algorithm>
#include <math.h>

#include "glenv.h"
#include "pfs.h"
#include "rmglwin.h"
#include "histogram.h"

#define SIDE_BAR 5
#define BOTTOM_BAR 14
#define SCALE_BAR 10	
#define MIN_LUM 0.000001f
#define MAX_LUM 100000000.0f

/**
* @param xPos starting x position of histogram
* @param yPos starting y position of histogram
* @param width width of histogram (in points/pixels), the number of bins is equal to this value
* @param height height of histogram (in points/pixels)
*/
Histogram::Histogram( int xPos, int yPos, int width, int height) : RMGLWin() {

	this->xPos = xPos;
	this->yPos = yPos;
	this->width = width - SIDE_BAR*2;
	this->height = height - BOTTOM_BAR;
	
	// background color
	backgroundColor = new float[3];
	for(int i=0; i<3; i++)
		backgroundColor[i] = 1.0f;	
	
	frequencyMax = -1;
	sliderPosMin = 0;
	sliderPosMax = 0;
	
	this->lumMin = MIN_LUM;
	this->lumMax = MAX_LUM;
	logLumMin = log10(lumMin);
	logLumMax = log10(lumMax);	
	
	selectionState = NONE;
}

Histogram::~Histogram()
{
	delete [] frequencyValues;
	delete [] backgroundColor;
}

/** Redraw window
*/
void Histogram::redraw(void) {

	RMGLWin::redrawStart();
	
	drawHistogram();
	drawScale();
	drawStatistic();
	drawSlider();
	
	RMGLWin::redrawEnd();
}

/** Draws histogram.
*/
void Histogram::drawHistogram() {

	if(frequencyValues == NULL)
		return;
		
	if( frequencyMax == -1)	
		frequencyMax = getHighFrequency(); // not max but high to remove very large picks (e.g. near very light bins)

	int offsetX = SIDE_BAR; 
	int offsetY = height / 4;

	glColor3f(1.0f, 0.0f, 0.0f);
	float barHeight = 0;
	
	for(int lum = 0; lum< width; lum++) {
	
		float hh = frequencyValues[lum] / frequencyMax;
		if( hh > 1)
			hh = 1;
		barHeight = (float)height * hh;
		
		if( barHeight > 0 && barHeight < 1.0)
			barHeight = 1.0;
		
		if(barHeight > 0)
			glRectf(offsetX + lum, offsetY, offsetX+lum + 1, offsetY + barHeight);
	}
}


/** Draws background
*/
void Histogram::drawBackground()
{
	glColor3fv(backgroundColor);
	glRectf( xPos/2, yPos, width + SIDE_BAR*2 + xPos/2, yPos - height - BOTTOM_BAR - SCALE_BAR); 
}

/** Draws scale
*/
void Histogram::drawScale() {

	glColor3f(0.1f, 0.1f, 0.1f);
	
	float pos;
	int offsetX = SIDE_BAR; 
	int offsetY = height / 4;//yPos - height;
	int k1 = 0;
	for( int i = (int)logLumMin; i <= (int)logLumMax; i++) {
	
		pos = lum2pos( pow(10, (float)i)) * (float)width;

		glRectf( offsetX + pos, offsetY, offsetX + pos + 1, offsetY + height/2);
		
		if( i < 0) {
			glRasterPos2f( offsetX + pos - 5, offsetY - 10);
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, '-');
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, '6' - (k1++));
		}
		else {
			glRasterPos2f( offsetX + pos - 2, offsetY - 10);
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, '0' + i);
		}	

	} 
	glRectf( offsetX, offsetY, offsetX+pos + 1, offsetY-1); 
}
	
	
/** Draws statistic of a current image.
*/	
void Histogram::drawStatistic() {

	int offsetX = winWidth - 130;
	int offsetY = height ;

	glRasterPos2f( offsetX, offsetY);
	
	char ss[200];
	sprintf( ss, "logLum.:<%.3f, %.3f>"
			, log10(pos2lum(sliderPosMin)), log10(pos2lum(sliderPosMax)));
	
	int len = (int) strlen(ss);
	for (int i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, ss[i]); 
	}
}	


/** Draws slider
*/
void Histogram::drawSlider()
{
	float minValue = width * sliderPosMin;
	float maxValue = width * sliderPosMax;
	int sliderHeight = height;

	int offsetX = SIDE_BAR;
	int offsetY = height + height/4 ;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); 
	glColor4f(0.2f, 0.2f, 0.5f, 0.3f);
	
	glLoadName(WHOLE_SLIDER); 
	glBegin(GL_QUADS);
		glVertex3f(offsetX + minValue, offsetY, -0.5f);
		glVertex3f(offsetX + minValue, offsetY - sliderHeight, -0.5f);
		glVertex3f(offsetX + maxValue, offsetY - sliderHeight, -0.5f);
		glVertex3f(offsetX + maxValue, offsetY, -0.5f);
	glEnd();

	glColor4f(0.7f, 0.7f, 0.8f, 0.5f);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
		glLoadIdentity();
		glTranslatef(offsetX + minValue, offsetY - sliderHeight/2, 0.0f);
		glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);		
		glLoadName(LEFT_BAR); 
		glutSolidCone(sliderHeight/2, sliderHeight/3.0, 2, 2);
	glPopMatrix();
	glPushMatrix();
		glLoadIdentity();
		glTranslatef(offsetX + maxValue, offsetY - sliderHeight/2, 0.0f);
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
		glLoadName(RIGHT_BAR); 
		glutSolidCone(sliderHeight/2, sliderHeight/3.0, 2, 2);
	glPopMatrix();
	
	glDisable(GL_BLEND);
}

	
/** Creates histogram. Histogram will have 'width' bins.
*/
void Histogram::computeFrequency(const pfs::Array2D *image) {

	const int size = image->getRows()*image->getCols();
	
	frequencyValues = new int[width];
	for( int i = 0; i < width; i++ )
		frequencyValues[i] = 0;							
											
	for( int i = 0; i < size; i++ ) {
	
		float val = (*image)(i);
		if( val <= 0 || val > lumMax) {
			//fprintf( stderr, "WARNING: pixel luminance out of range (Histogram::computeFrequency())\n");
			continue;
		}
		
		val = log10(val);

		// Calculate frequency value index in table: <0, with>
		int index = (int)((val - logLumMin) / (logLumMax - logLumMin) * (float)width);
		
		if( index > width || index < 0 ) 
			continue;
		if( index == width ) 
			index = width - 1;

		// Increase the counter of this value
		frequencyValues[index] += 1;	
	}
}


/** Computes the maximum frequency in an image based on histogram data.
*/
float Histogram::getMaxFrequency() const {

	int maxFreq = -1;
	for( int i = 0; i < width; i++ ) {
		if( frequencyValues[i] > maxFreq ) {
			maxFreq = frequencyValues[i];
		}    
	}
	return maxFreq;
}


/** Returns frequence of the bin = max_bin - PERCENTIL * number_of_bins. Very light and very dark bins 
*	are not taken into consideration in this computations. It allows to skip very dark (black) and/or
*	very light (white) pixels which can cover large areas of an image and lower the height of histogram for remaining 
*	pixels (undesirable situation: flat histogram with one large pick for very light bin). 
*/
float Histogram::getHighFrequency() const {
	
	std::vector<float> vec;
	for( int i = 0; i < width; i++ ) {
		vec.push_back((float)frequencyValues[i]);
	}    
	std::sort(vec.begin(), vec.end());
	
	#define PERCENTIL 0.99

	float highFreq = vec[(int)((float)vec.size() * PERCENTIL)];
	
	// returns high frequency only if max frequency is 3 or more times higher
	float maxFreq = getMaxFrequency();

	if( (3*highFreq) > maxFreq)
		return maxFreq;
	else	
		return highFreq;
}


/** Converts horizontal position in the histogram into luminance value. The position must be in range <0,1>.
*/
float Histogram::pos2lum( float pos) {

	if( pos < 0)
		pos = 0;
	if( pos > 1)
		pos = 1;

	return pow( 10, pos * (logLumMax - logLumMin) + logLumMin);
}

/** Converts luminance value into position in range <0,1>. 
*/
float Histogram::lum2pos( float lum) {
	
	if( lum < lumMin)
		lum = lumMin;
	if( lum > lumMax)
		lum = lumMax;
	
	return (log10(lum) - logLumMin) / (logLumMax - logLumMin);
}


/** Computes initial position of the slider. The calculation are based on the shape of histogram. 
*/
#define FREQUENCY_EDGE 0.2
void Histogram::computeLumRange( float& min, float& max) {

	float maxFreq = getHighFrequency();

	float freq;
	for( int i = 0; i < width; i++) {
	
		freq = frequencyValues[i] / maxFreq;
		
		if( freq > FREQUENCY_EDGE && freq < 1.0){
			min = pos2lum((float)i / (float)width);
			break;
		}
	}
	
	for( int i = (width-1); i >=0; i--) {
	
		freq = frequencyValues[i] / maxFreq;
		
		if( freq > FREQUENCY_EDGE && freq < 1.0){
			max = pos2lum((float)i / (float)width);
			break;
		}
	}
}

/** Returns width of the histogram in pixels
*/	
int Histogram::getWidth(void) {
	return width;
}
int Histogram::getHeight(void) {
	return height;
}

int Histogram::getBackgroundWidth(void) {
	return (width + SIDE_BAR*2);
}
int Histogram::getBackgroundHeight(void) {
	return (height + BOTTOM_BAR);
}

float Histogram::getLumMin(void) {
	return lumMin;
}
float Histogram::getLumMax(void) {
	return lumMax;
}



void Histogram::setSliderPosMin( float pos) {
	if( pos > sliderPosMax)
		pos = sliderPosMax;
	if( pos < 0)
		pos = 0;
	sliderPosMin = pos;
}

float Histogram::getSliderPosMin(void) {
	return sliderPosMin;
}

void Histogram::setSliderPosMax( float pos) {
	if( pos < sliderPosMin)
		pos = sliderPosMin;
	if( pos > 1.0)
		pos = 1.0;
	sliderPosMax = pos;
}

float Histogram::getSliderPosMax(void) {
	return sliderPosMax;
}

void Histogram::setSliderPosMinMax( float min, float max) {

	if( min < 0)
		min = 0;
	if( max < 0)
		max = 0;
	if( min > 1.0)
		min = 0;
	if( max > 1.0)
		max = 1.0;
		
	if( min > max)
		min = max;

	sliderPosMin = min;
	sliderPosMax = max;

}


/** Checks what part of slider was touched.
*/
void Histogram::processSliderSelection(int xCoord, int yCoord) {

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
	glPushName(NONE);
	// Set new projection matrix as a box around xPos, yPos
	glLoadIdentity();
		
	int hh = glutGet(GLUT_WINDOW_HEIGHT);
	// Picking matrix at position xCoord, windowSize - yCoord (fliped window Y axis)
	// and size of 4 units in depth
	gluPickMatrix(xCoord, hh - yCoord, 4, 4, viewp);

	glOrtho(0.0f, viewp[2], 0.0f, viewp[3], -10.0f, 10.0f); // last 1.0 -1.0 it's enough
	drawSlider(); // draw only picked parts
	
	hits = glRenderMode(GL_RENDER);

	if(hits > 0) {
		for(int i=3; i < BUFFER_SIZE; i+=4) {
			switch(selectionBuffer[i]) {
			
				case 1 :selectionState = LEFT_BAR; break;
				case 2 :selectionState = RIGHT_BAR; break; 
				case 3 :selectionState = WHOLE_SLIDER; break;
				default:selectionState = NONE; break;
			}
			if(selectionState != NONE)
				break;
		}
	}
		
	redrawEnd();
}


int Histogram::setSliderSelectionState(SelectedBar newState)
{
	if(newState == LEFT_BAR || newState == RIGHT_BAR || newState == WHOLE_SLIDER || newState == NONE)
	{
		selectionState = newState;
		return (int)selectionState;
	}
	else
		return -1;
}


SelectedBar Histogram::getSliderSelectionState()
{
	return selectionState;
}

void Histogram::resetFrequencyMax(void) {
	frequencyMax = -1;
}



