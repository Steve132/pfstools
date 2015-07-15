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
#include <vector>
#include <algorithm>
#include <math.h>

#include "pfs.h"

#include "m_histogram.h"

#define BORDER_SIDE 20
#define BORDER_BOTTOM 30
#define BORDER_TOP 5
#define SLIDER_WIDTH 16
#define SCALE_BORDER_TOP 3
#define SCALE_BORDER_BOTTOM 3
#define MIN_LUM 0.00000001f
#define MAX_LUM 100000000.0f

// WHITE
#define W_R 1.0
#define W_G 1.0
#define W_B 1.0
#define W_A 0.45

// RED
#define R_R 1.0
#define R_G 0.1
#define R_B 0.1
#define R_A 0.9

// GREEN
#define G_R 0.1
#define G_G 0.75
#define G_B 0.1
#define G_A 0.9

// BLUE
#define B_R 0.25
#define B_G 0.25
#define B_B 1.0
#define B_A 0.9




/**
*/
M_Histogram::M_Histogram() : Module() {

        // set inital height
        height = 150 ;

	// set background color of the widget to black
	winBackgroundColor = new float[4];
	for(int i=0; i<3; i++)
	  winBackgroundColor[i] = 0.0f;
	// no transparency please
	winBackgroundColor[3] = 1.0f;

	// array for the frequency analysis
	frequencyValues = NULL ;

	// set a fixed size of bins
	bins = 20000 ;

	// set sigma values for pre- and postsmoothing
//	gauss_data = (float)bins/1000 ; // take this for an adaptive approach , experimental
	gauss_data = 2.5 ;
	gauss_display = 2.5 ;

	// enabled smoothing of the intensity scale values by default
	is_scale_gaussed = 1 ;

	// disable smoothing of the intensity scale values
	is_scale_show_max_hue = 0 ;
	// amplifiy the intensity values by default for better visibility
	scale_amplifier = 0.5 ;

	// set the default channel
	channel = "XYZ" ;
	
	// invalidate the calculated maximum frequency
	frequencyMax = -1;

	// set the inital slider position
	sliderPosMin = 0;
	sliderPosMax = 0;
	
	// set the predefined luminance range
	lumMin = MIN_LUM;
	lumMax = MAX_LUM;
	logLumMin = log10(lumMin);
	logLumMax = log10(lumMax);	
	
	// set inital hover state
	hoverState = NONE;

	virtualWidth = 1.0 ;
	virtualOffset = 0.5 ;

	abort_update = 0;
}


   const float hist_color_max   = 1.0f;
   const float hist_color_min   = 0.2f;
   const float hist_color_alpha = 0.9f;


M_Histogram::~M_Histogram()
{
	delete [] frequencyValues;
}

void M_Histogram::redraw(void) {
	if(frequencyValues == NULL) return ;

 	Module::redrawStart();
	
	drawHistogram();
	drawScale();
	drawSlider();
	drawStatistic();
	
	Module::redrawEnd();
}



/** Draws histogram.
*/
void M_Histogram::drawHistogram() {

	if(frequencyValues == NULL)
		return;
		
	if( frequencyMax == -1)	
		frequencyMax = getMaxFrequency();
	int offsetX = BORDER_SIDE; 
	int offsetY = BORDER_BOTTOM;
//	if(virtualWidth <= 1) virtualOffset = 0.5 ;
	float offset =  -virtualOffset * (virtualWidth -1.0) * (float)bins ;
	float barHeight = 0;
	float center = 0 ;
	float strech = ((float)width- BORDER_SIDE*2)/(float)bins ;
	float relative_height ;
	float c_pos = lum2pos(center) ;

	glEnable(GL_BLEND);

	int array_pos ;
	int ch = 0 ;
	if(!strcmp(channel, "XYZ") == 0)
			switch(channel[0]) {
			case 'X': ch = 1 ; break ;
			case 'Y': ch = 2 ; break ;
			case 'Z': ch = 3 ; break ;
			}


	float valX = log10(rawX);
	float valY = log10(rawY);
	float valZ = log10(rawZ);
	int indexX = (int)round((valX - logLumMin) / (logLumMax - logLumMin) * (float)bins);
	int indexY = (int)round((valY - logLumMin) / (logLumMax - logLumMin) * (float)bins);
	int indexZ = (int)round((valZ - logLumMin) / (logLumMax - logLumMin) * (float)bins);
	if(rawPosX == -1 || rawPosY == -1){
		indexX = -1 ;
		indexY = -1 ;
		indexZ = -1 ;
	}


	for(int pos = offsetX; pos< width-BORDER_SIDE; pos++) {


		switch(ch) {
		case 0:	glColor4f(W_R, W_G, W_B, W_A); break ;
		case 1:	glColor4f(R_R, R_G, R_B, R_A); break ;
		case 2:	glColor4f(G_R, G_G, G_B, G_A); break ;
		case 3:	glColor4f(B_R, B_G, B_B, B_A); break ;
		}


		int ch_offset = 0 ;
		if(!is_scale_gaussed) ch_offset = 4 ;

		// GAUSS DISPLAY HIST
		float relative_height = 0 ;
		float his_width =  (float)(width-BORDER_SIDE*2) ;
		float painter = (float)(pos-BORDER_SIDE) / his_width ;
		float fbins = (float)bins ;

		float ext_g_weight = 0 ;
		float ext_g_sigma =  gauss_display ;
		if(!is_scale_gaussed) ext_g_sigma = 0.01 ;
		float ext_g_width = ceil(ext_g_sigma * 3) ;

		for(int k = -ext_g_width ; k <= ext_g_width ; k++){
			array_pos =(int)  (painter * fbins - offset + k * fbins / his_width) /virtualWidth  ;
			if(array_pos < 0 || array_pos >= bins) continue ;
			float ext_gaussian = 1.0/(ext_g_sigma*sqrt(2.0*M_PI)) * exp(-0.5*pow((float)k/ext_g_sigma,2.0)) ;
			ext_g_weight += ext_gaussian ;
			relative_height +=   ((float)frequencyValues[ch + ch_offset][array_pos]) / frequencyMax * ext_gaussian;
		}

		relative_height /= ext_g_weight;

		if( relative_height > 1){
			relative_height = 1;
		}
		barHeight = ((float)height - BORDER_BOTTOM -BORDER_TOP) * relative_height;

		// DRAW HIST
		if(barHeight > 0)
			glRectf(pos, offsetY, pos+1, offsetY + barHeight);



		// DRAW SCALE
		array_pos = floor((painter * fbins - offset)/virtualWidth) ;
		int array_pos_min =floor( ((float)(pos-BORDER_SIDE) / his_width * fbins - offset)/virtualWidth) ;
		int array_pos_max =floor(  ((float)(pos+1-BORDER_SIDE) / his_width * fbins - offset)/virtualWidth) ;
		if(array_pos_min == array_pos_max) array_pos_max++ ;

		int r = 0 ;
		int g = 0 ;
		int b = 0 ;

		if(ch == 0){
		if(indexX >= array_pos_min && indexX < array_pos_max) r = 1.0 ;
		if(indexY >= array_pos_min && indexY < array_pos_max) g = 1.0 ;
		if(indexZ >= array_pos_min && indexZ < array_pos_max) b = 1.0 ;
		}else{
			r = 1.0 ;
			g = 1.0 ;
			b = 1.0 ;
		}

		if(        (indexX >= array_pos_min && indexX < array_pos_max && (ch == 0 || ch == 1))
				|| (indexY >= array_pos_min && indexY < array_pos_max && (ch == 0 || ch == 2))
				|| (indexZ >= array_pos_min && indexZ < array_pos_max && (ch == 0 || ch == 3))
		){
			glColor4f(r, g, b, 0.75);
			glRectf(pos, offsetY, pos+1, offsetY + (float)height - BORDER_BOTTOM -BORDER_TOP);
		}

		if(array_pos > 0 && array_pos < bins) {

			// for ch == 0
			float cr, cg, cb, sum, weight ;

			// for ch > 0
			float value ;
			float hue = 0 ;
			if(ch > 0) value = frequencyValues[ch + ch_offset][array_pos] ;
			// dont waste time, go to the next round
			if(ch > 0 && value == 0) continue ;
			if(ch > 0) hue = is_scale_show_max_hue?1.0:pow(value / frequencyMax,scale_amplifier) ;

			switch(ch) {
			case 0:
				cr = frequencyValues[1 + ch_offset][array_pos] ;
				cg = frequencyValues[2 + ch_offset][array_pos] ;
				cb = frequencyValues[3 + ch_offset][array_pos] ;
				sum = (cr + cg +cb) / 3.0 ;
				// dont waste time, go to the next round
				if(sum == 0) continue ;
				weight = cr  ;
				if(cg > weight) weight = cg  ;
				if(cb > weight) weight = cb  ;
				cr /= weight ;
				cg /= weight ;
				cb /= weight ;
				hue = is_scale_show_max_hue?1.0:pow(sum / frequencyMax,scale_amplifier) ;
				glColor4f(cr, cg, cb, hue);
				break ;
			case 1:	glColor4f(R_R, R_G, R_B, hue);	break ;
			case 2:	glColor4f(G_R, G_G, G_B, hue); break ;
			case 3:	glColor4f(B_R, B_G, B_B, hue);	break ;
			}
			glRectf(pos, SCALE_BORDER_BOTTOM, pos+1, 0 + BORDER_BOTTOM-1 -SCALE_BORDER_TOP);
		}
	}

	glDisable(GL_BLEND);
}



/** Draws scale
*/
void M_Histogram::drawScale() {

	glColor3f(1.0f, 1.0f, 1.0f);
	
	float pos;
	int offsetX = BORDER_SIDE ;
	int offsetY = BORDER_BOTTOM;
	int k1 = 0;

//	if(virtualWidth <= 1) virtualOffset = 0.5 ;
	float offset =  -virtualOffset * (virtualWidth -1.0) * (width- BORDER_SIDE*2) ;

	for( float i = (float)logLumMin; i <= (float)logLumMax; i=i+0.25) {
	
		pos = lum2pos( pow(10, (float)i)) * ((float)width- BORDER_SIDE*2);

		pos *= virtualWidth ;
		pos += offset ;

		int hh = 2;
		if( round(i) == i) hh = 4;
		glRectf( offsetX + pos, offsetY - hh, offsetX + pos + 1, offsetY + hh -1);
		
		if( round(i) == i) {
		  if( i < 0 ) {
		    glRasterPos2f( offsetX + pos - 9, offsetY - 15);
		    glutBitmapCharacter(GLUT_BITMAP_8_BY_13, '-');
		    glutBitmapCharacter(GLUT_BITMAP_8_BY_13, '8' - (k1++));
		  }
		  else if( i == 0 ) {
		    glRasterPos2f( offsetX + pos - 1, offsetY - 15);
		    glutBitmapCharacter(GLUT_BITMAP_8_BY_13, '0' + i);
		  }	
		  else {
		    glRasterPos2f( offsetX + pos - 2, offsetY - 15);
		    glutBitmapCharacter(GLUT_BITMAP_8_BY_13, '0' + i);
		  }	
		}

	} 
	glRectf( offsetX + (0 * virtualWidth) + offset, offsetY, offsetX + pos + 1, offsetY-1);
}
	


/** Draws slider
*/
void M_Histogram::drawSlider()
{

//    if(virtualWidth <= 1) virtualOffset = 0.5 ;
         float offset =  -virtualOffset * (virtualWidth -1.0) * (width- BORDER_SIDE*2) ;

	float minValue = (width- BORDER_SIDE*2) * sliderPosMin;
	float maxValue = (width- BORDER_SIDE*2) * sliderPosMax;
	int sliderHeight = height -BORDER_BOTTOM -BORDER_TOP ;

	int offsetX = BORDER_SIDE;
	int offsetY = height -BORDER_TOP;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); 
	glMatrixMode(GL_MODELVIEW) ;

	// BOTTOM
	glPushMatrix();
	glLoadName(BOTTOM);
	glColor4f(1.0f, 0.0f, 0.0f, 0.8f);
	glBegin(GL_QUADS);
	glVertex3f(offsetX-BORDER_SIDE,           0,        -0.5f);
	glVertex3f(offsetX-BORDER_SIDE,           -Y_BAR/2, -0.5f);
	glVertex3f(offsetX + width - BORDER_SIDE, -Y_BAR/2, -0.5f);
	glVertex3f(offsetX + width - BORDER_SIDE, 0,        -0.5f);
	glEnd();
	glPopMatrix();

	offsetX += offset ;
	minValue *= virtualWidth ;
	maxValue *= virtualWidth ;


	// SLIDER
	if(minValue-SLIDER_WIDTH>0){
		glLoadName(WHOLE_SLIDER);
		glColor4f(1.0f, 0.0f, 0.0f, 0.25f);
		glBegin(GL_QUADS);
		glVertex3f(offsetX + 0, offsetY, -0.5f);
		glVertex3f(offsetX + 0, offsetY - sliderHeight, -0.5f);
		glVertex3f(offsetX + minValue-SLIDER_WIDTH, offsetY - sliderHeight, -0.5f);
		glVertex3f(offsetX + minValue-SLIDER_WIDTH, offsetY, -0.5f);
		glEnd();
	}

	if(maxValue+SLIDER_WIDTH <  (width-2*BORDER_SIDE) * virtualWidth ){
		glLoadName(WHOLE_SLIDER);
		glColor4f(1.0f, 0.0f, 0.0f, 0.25f);
		glBegin(GL_QUADS);
		glVertex3f(offsetX + maxValue+SLIDER_WIDTH, offsetY, -0.5f);
		glVertex3f(offsetX + maxValue+SLIDER_WIDTH, offsetY - sliderHeight, -0.5f);
		glVertex3f(offsetX + (width-2*BORDER_SIDE) * virtualWidth , offsetY - sliderHeight, -0.5f);
		glVertex3f(offsetX + (width-2*BORDER_SIDE) * virtualWidth , offsetY, -0.5f);
		glEnd();
	}

	glLoadName(WHOLE_SLIDER);
	glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
	glBegin(GL_QUADS);
	glVertex3f(offsetX + minValue, offsetY, -0.5f);
	glVertex3f(offsetX + minValue, offsetY - sliderHeight, -0.5f);
	glVertex3f(offsetX + maxValue, offsetY - sliderHeight, -0.5f);
	glVertex3f(offsetX + maxValue, offsetY, -0.5f);
	glEnd();

	// LEFT
	glPushMatrix();
	glLoadName(LEFT_BAR);
	glColor4f(1.0f, 1.0f, 1.0f, 0.3f);
	glBegin(GL_QUADS);
	glVertex3f(offsetX + minValue-SLIDER_WIDTH, offsetY, -0.5f);
	glVertex3f(offsetX + minValue-SLIDER_WIDTH, offsetY - sliderHeight, -0.5f);
	glVertex3f(offsetX + minValue, offsetY - sliderHeight, -0.5f);
	glVertex3f(offsetX + minValue, offsetY, -0.5f);
	glEnd();

	// RIGHT
	glPushMatrix();
	glLoadName(RIGHT_BAR);
	glColor4f(1.0f, 1.0f, 1.0f, 0.3f);
	glBegin(GL_QUADS);
	glVertex3f(offsetX + maxValue, offsetY, -0.5f);
	glVertex3f(offsetX + maxValue, offsetY - sliderHeight, -0.5f);
	glVertex3f(offsetX + maxValue+SLIDER_WIDTH, offsetY - sliderHeight, -0.5f);
	glVertex3f(offsetX + maxValue+SLIDER_WIDTH, offsetY, -0.5f);
	glEnd();
	glPopMatrix();



	// BACK + SCALE
	offsetX = BORDER_SIDE;
	offsetY = height  ;
	glPushMatrix();
	glLoadName(BACK);
	glColor4f(0.0f, 1.0f, 0.0f, 0.0f);
	glBegin(GL_QUADS);
	glVertex3f(0,                          0,       -0.5f);
	glVertex3f(0,                          offsetY, -0.5f);
	glVertex3f(offsetX + width - BORDER_SIDE, offsetY, -0.5f);
	glVertex3f(offsetX + width - BORDER_SIDE, 0,       -0.5f);
	glEnd();
	glPopMatrix();


	glDisable(GL_BLEND);
}

	
/** Draws statistic of a current image.
*/
void M_Histogram::drawStatistic() {

	int offsetX = width - 24*8 -BORDER_SIDE;
	int offsetY = height - 13  -BORDER_TOP ;

	glColor3f(0.5f, 1.0f, 0.5f);

	glRasterPos2f( offsetX, offsetY );

	char ss[200];
	sprintf( ss, "logLum.:<%.3f, %.3f>"
			, log10(pos2lum(sliderPosMin)), log10(pos2lum(sliderPosMax)));

	int len = (int) strlen(ss);



	for (int i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, ss[i]);
	}
}

void M_Histogram::setChannel(const char* ch) { 
	channel = ch;
}	




void M_Histogram::clearChannel(int ch, void  CB(void)) {

	if(frequencyValues == NULL) return ;



#pragma omp parallel for
for( int i = 0; i < bins; i++ ){
	if(abort_update == 1) continue ;
		frequencyValues[ch][i] = 0;
		frequencyValues[ch+4][i] = 0;
	}

#pragma omp parallel for
for( int i = 0; i < bins; i++ ){
	if(abort_update == 1) continue ;
	frequencyValues[8][i] = 0;
}
	CB() ;

}








/** Creates histogram. Histogram will have 'width' bins.
*/
void M_Histogram::computeFrequency(int ch, const pfs::Array2D *image,void  CB(void)) {

	dbs("Histogram::computeFrequency") ;

	if(ch == 0 && image == NULL){


        #pragma omp parallel for
	    for( int i = 0; i < bins; i++ ){
	    	if(abort_update == 1) continue ;
	        frequencyValues[0][i] =
	            frequencyValues[1][i]
	           +frequencyValues[2][i]
                   +frequencyValues[3][i] ;
	        frequencyValues[0][i] = ceil((float)frequencyValues[0][i] / 3.0) ;

                frequencyValues[4][i] =
                    frequencyValues[5][i]
                   +frequencyValues[6][i]
                   +frequencyValues[7][i] ;
                frequencyValues[4][i] = (float)frequencyValues[4][i] / 3.0 ;
                if(i % 10 == 0) CB() ;
	    }
	    return ;
	}


	const int size = image->getRows()*image->getCols();


	if(frequencyValues == NULL){
//		binborder = new float[bins+1];
		frequencyValues = new int*[9];

		frequencyValues[0] = new int[bins];
		frequencyValues[1] = new int[bins];
		frequencyValues[2] = new int[bins];
		frequencyValues[3] = new int[bins];

		frequencyValues[4] = new int[bins];
		frequencyValues[5] = new int[bins];
		frequencyValues[6] = new int[bins];
		frequencyValues[7] = new int[bins];

		frequencyValues[8] = new int[bins];
//		for( int i = 0; i < bins; i++ ){
//			frequencyValues[0][i] = 0;
//			frequencyValues[1][i] = 0;
//			frequencyValues[2][i] = 0;
//			frequencyValues[3][i] = 0;
//
//			frequencyValues[4][i] = 0;
//                        frequencyValues[5][i] = 0;
//                        frequencyValues[6][i] = 0;
//                        frequencyValues[7][i] = 0;
//		}

//		for(int k = 0 ; k <= bins ; k++){
//			binborder[k] = pow10( (double)((logLumMax - logLumMin) / (double)bins * (double)k + logLumMin));
//		}
	}



//	int lastbin = -1 ;
	#pragma omp parallel for
	for( int i = 0; i < size; i=i+1 ) {
		if(abort_update == 1) continue ;
	

		float val = (*image)(i);
		if( val <= 0 || val > lumMax) {
			//fprintf( stderr, "WARNING: pixel luminance out of range (Histogram::computeFrequency())\n");
			continue;
		}


		val = log10(val);

//		 Calculate frequency value index in table: <0, with>
		int index = (int)round((val - logLumMin) / (logLumMax - logLumMin) * (float)bins);


//		int index = -1 ;
//
//		if(lastbin != -1){
////			printf("lb: %d\n", lastbin) ;
//			int lastbin_b = lastbin -4 ;
//			int lastbin_e = lastbin +4 ;
//			if(lastbin_b < 0) lastbin_b = 0 ;
//			if(lastbin_e > bins) lastbin_e = bins ;
//
//			index = findBin(val, lastbin_b, lastbin_e) ;
//		}
//		if(index == -1)
//		index = findBin(val, 0, bins) ;
//
//		if(index != -1) lastbin = index ;



		if( index > bins || index < 0 ) 
			continue;
		if( index == bins ) 
			index = bins - 1;

		// Increase the counter of this value
		frequencyValues[ch+4][index] += 1;
		frequencyValues[ch][index] += 1;

		if(i % 10 == 0) {
			CB() ;
		}



	}





	float his_width =  (float)(width-BORDER_SIDE*2) ;
	float fbins = (float)bins ;

	 for(int k = 0 ; k < bins ; k++){
		 if(abort_update == 1) continue ;
	     float rel = 0 ;
	     float g_weight = 0 ;
	     float g_sigma =  gauss_data ;
	     float g_width = ceil(g_sigma * 3) ;
	     for(int j = -g_width ; j <= g_width ; j++){
	         if(k +j < 0 || k +j >= bins) continue ;
	         float gaussian = 1.0/(g_sigma*sqrt(2.0*M_PI)) * exp(-0.5*pow((float)j/g_sigma,2.0)) ;
	         g_weight += gaussian ;
	         rel += (float)frequencyValues[ch+4][k+j] * gaussian;
	     }
//	     printf("weight: %f\n", g_weight) ;
	     rel /= g_weight ;
	     frequencyValues[ch][k] = rel ;
	 	if(k % 10 == 0){
	 		CB() ;
	 	}
	 }


	 dbe("Histogram::computeFrequency") ;
}


int M_Histogram::findBin(float val, int start, int stop){

//	fprintf(stderr, "d1 %d %d\n", start, stop) ;
//	exit(0) ;

	if(val < binborder[start] && val > binborder[stop]) {
//		fprintf(stderr, "d1") ;
		return -1 ;
	}
	if(start == stop || start == stop-1){
		if(val >= binborder[start] && val < binborder[start+1] ) {
//			fprintf(stderr, "d2") ;
			return start;
		}else{
//			fprintf(stderr, "d3") ;
			return -1 ;
		}
	}

	int left = findBin(val, start, stop - (stop - start)/2) ;
	int right = findBin(val, start + (stop - start)/2, stop) ;

//	fprintf(stderr, "d4") ;
	if(left >= 0 ) return left ;
//	fprintf(stderr, "d5") ;
	if(right >= 0 ) return right ;
//	fprintf(stderr, "d6") ;
	return -1 ;
}



/** Computes the maximum frequency in an image based on histogram data.
*/
float M_Histogram::getMaxFrequency()  {
	if(frequencyValues == NULL)
		return -1;


	int maxFreq = -1;
	for( int i = 0; i < bins; i++ ) {
		if( frequencyValues[0][i] > maxFreq )
			maxFreq = frequencyValues[0][i];
		if( frequencyValues[1][i] > maxFreq )
			maxFreq = frequencyValues[1][i];
		if( frequencyValues[2][i] > maxFreq )
			maxFreq = frequencyValues[2][i];
		if( frequencyValues[3][i] > maxFreq )
			maxFreq = frequencyValues[3][i];
	}

	if(maxFreq == 0){
		 maxFreq = -1;
			for( int i = 0; i < bins; i++ ) {
				if( frequencyValues[4][i] > maxFreq )
					maxFreq = frequencyValues[4][i];
				if( frequencyValues[5][i] > maxFreq )
					maxFreq = frequencyValues[5][i];
				if( frequencyValues[6][i] > maxFreq )
					maxFreq = frequencyValues[6][i];
				if( frequencyValues[7][i] > maxFreq )
					maxFreq = frequencyValues[7][i];
			}
	}

	return maxFreq;
}


/** Returns frequence of the bin = max_bin - PERCENTIL * number_of_bins. Very light and very dark bins 
*	are not taken into consideration in this computations. It allows to skip very dark (black) and/or
*	very light (white) pixels which can cover large areas of an image and lower the height of histogram for remaining 
*	pixels (undesirable situation: flat histogram with one large pick for very light bin). 
*/
float M_Histogram::getHighFrequency()  {
	if(frequencyValues == NULL)
		return -1;
	
	std::vector<float> vec;
	for( int i = 0; i < bins; i++ ) {
		vec.push_back((float)frequencyValues[0][i]);
		vec.push_back((float)frequencyValues[1][i]);
		vec.push_back((float)frequencyValues[2][i]);
		vec.push_back((float)frequencyValues[3][i]);
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
float M_Histogram::pos2lum( float pos) {

	if( pos < 0)
		pos = 0;
	if( pos > 1)
		pos = 1;

	return pow( 10, pos * (logLumMax - logLumMin) + logLumMin);
}

/** Converts luminance value into position in range <0,1>. 
*/
float M_Histogram::lum2pos( float lum) {
	
	if( lum < lumMin)
		lum = lumMin;
	if( lum > lumMax)
		lum = lumMax;
	
	return (log10(lum) - logLumMin) / (logLumMax - logLumMin);
}


/** Computes initial position of the slider. The calculation are based on the shape of histogram. 
*/
#define FREQUENCY_EDGE 0.05
void M_Histogram::computeLumRange( float& min, float& max) {
	if(frequencyValues == NULL)
		return;

	float maxFreq = getMaxFrequency();

	float freq;
	for( int i = 0; i < bins; i++) {
	
		freq = frequencyValues[0][i] / maxFreq;
		
		if( freq > FREQUENCY_EDGE && freq < 1.0){
			min = pos2lum((float)i / (float)bins);
			break;
		}
	}
	
	for( int i = (bins-1); i >=0; i--) {
	
		freq = frequencyValues[0][i] / maxFreq;
		
		if( freq > FREQUENCY_EDGE && freq < 1.0){
			max = pos2lum((float)i / (float)bins);
			break;
		}
	}
}


float M_Histogram::getPeak() {
        if(frequencyValues == NULL)
                return -1;

        float maxFreq = getMaxFrequency();

        float freq = 0 ;
        float j = -1 ;
        for( int i = 0; i < bins; i++) {

            if(freq < frequencyValues[0][i]){
                freq = frequencyValues[0][i] ;
                j = i ;
            }
        }
        return pos2lum((float)j / (float)bins);
}




int M_Histogram::getSidebarWidth(void) {
	return BORDER_SIDE ;
}


/** Returns width of the histogram in pixels
*/	
int M_Histogram::getInnerWidth(void) {
	return width - BORDER_SIDE * 2;
}
int M_Histogram::getInnerHeight(void) {
	return height - BORDER_BOTTOM - BORDER_TOP;
}

int M_Histogram::getBackgroundWidth(void) {
	return (width);
}
int M_Histogram::getBackgroundHeight(void) {
	return (height);
}

float M_Histogram::getLumMin(void) {
	return lumMin;
}
float M_Histogram::getLumMax(void) {
	return lumMax;
}



void M_Histogram::setSliderPosMin( float pos) {
	if( pos > sliderPosMax)
		pos = sliderPosMax;
	if( pos < 0)
		pos = 0;
	sliderPosMin = pos;
}

float M_Histogram::getSliderPosMin(void) {
	return sliderPosMin;
}

void M_Histogram::setSliderPosMax( float pos) {
	if( pos < sliderPosMin)
		pos = sliderPosMin;
	if( pos > 1.0)
		pos = 1.0;
	sliderPosMax = pos;
}

float M_Histogram::getSliderPosMax(void) {
	return sliderPosMax;
}

void M_Histogram::setSliderPosMinMax( float min, float max) {

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
void M_Histogram::processPointerPosition(int xCoord, int yCoord, int pan) {
	if(frequencyValues == NULL)
		return;

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
	gluPickMatrix(xCoord, hh - yCoord, 1, 1, viewp);
	//gluPickMatrix(xCoord, hh - yCoord, 1, 1, viewp);

	glOrtho(0.0f, viewp[2], 0.0f, viewp[3], -10.0f, 10.0f); // last 1.0 -1.0 it's enough
	drawSlider(); // draw only picked parts
	
	hits = glRenderMode(GL_RENDER);

//	printf("\n\nHITS: %d %d\n", hits, BUFFER_SIZE) ;

	if(hits > 0) {
		for(int i=3; i < hits*4; i+=4) {
//			printf("HIT: %d - %u \n", i-3, selectionBuffer[i-3] ) ;
//			printf("HIT: %d - %u \n", i-2, selectionBuffer[i-2] ) ;
//			printf("HIT: %d - %u \n", i-1, selectionBuffer[i-1] ) ;
//			printf("HIT: %d - %u \n", i, selectionBuffer[i-0] ) ;
			switch(selectionBuffer[i]) {
				case 1 :hoverState = LEFT_BAR; glutSetCursor(GLUT_CURSOR_LEFT_SIDE); break;
				case 2 :hoverState = RIGHT_BAR; glutSetCursor(GLUT_CURSOR_RIGHT_SIDE); break; 
				case 3 :hoverState = WHOLE_SLIDER; !pan?glutSetCursor(GLUT_CURSOR_LEFT_RIGHT):(virtualWidth>1?glutSetCursor(GLUT_CURSOR_INFO):glutSetCursor(GLUT_CURSOR_INHERIT));break;
				case 4 :hoverState = BOTTOM; glutSetCursor(GLUT_CURSOR_BOTTOM_SIDE);break;
				case 5 :hoverState = BACK; !pan?glutSetCursor(GLUT_CURSOR_LEFT_RIGHT):(virtualWidth>1?glutSetCursor(GLUT_CURSOR_INFO):glutSetCursor(GLUT_CURSOR_BOTTOM_SIDE));break;
				default:hoverState = NONE; glutSetCursor(GLUT_CURSOR_INHERIT); break;
			}
			if(hoverState != NONE)
				break;
		}
	} else {
	  hoverState = NONE; //glutSetCursor(GLUT_CURSOR_INHERIT); 	glutSetCursor(GLUT_CURSOR_CROSSHAIR);
        }
	redrawEnd();

}


int M_Histogram::setSubComponentHoverState(SubComponent newState)
{
	if(newState == LEFT_BAR || newState == RIGHT_BAR || newState == WHOLE_SLIDER || newState == NONE)
	{
		hoverState = newState;
		return (int)hoverState;
	}
	else
		return -1;
}


void M_Histogram::setVisible(bool _isVisible) {
        isVisible = _isVisible;
        if(!getVisible())  setSubComponentHoverState(NONE);
}


M_Histogram::SubComponent M_Histogram::getSubComponentHoverState()
{
	return hoverState;
}

void M_Histogram::resetFrequencyMax(void) {
	frequencyMax = -1;
}


void M_Histogram::setVirtualWidth(float width) {
  virtualWidth = width ;
//  printf("VW: %f\n",virtualWidth) ;
}


float M_Histogram::getVirtualWidth() {
  return virtualWidth ;
}

void M_Histogram::setVirtualOffset(float offset) {
  virtualOffset = offset ;
//  printf("VW: %f\n",virtualOffset) ;
}


float M_Histogram::getVirtualOffset() {
  return virtualOffset ;
}



int M_Histogram::getChannelAsInt(){


  if(strcmp(channel, "XYZ") == 0){
      return 0 ;
  } else {
    switch(channel[0]) {
    case 'X':
      return 1 ;
    case 'Y':
      return 2 ;
    default:
    case 'Z':
      return 3 ;
    }
  }
}



int M_Histogram::get_is_scale_gaussed() const
{
	return is_scale_gaussed;
}

void M_Histogram::set_is_scale_gaussed(int is_scale_gaussed)
{
	this->is_scale_gaussed = is_scale_gaussed;
}

int M_Histogram::get_is_scale_show_max_hue() const
{
	return is_scale_show_max_hue;
}

void M_Histogram::set_is_scale_show_max_hue(int is_scale_show_max_hue)
{
	this->is_scale_show_max_hue = is_scale_show_max_hue;
}


void M_Histogram::setRawData( int x, int y, float X, float Y, float Z) {

	rawPosX = x;
	rawPosY = y;
	rawX = X;
	rawY = Y;
	rawZ = Z;
}
