/**
 * @brief PFS library - additional utilities
 *
 * This file is a part of PFSTOOLS package.
 * ----------------------------------------------------------------------
 * Copyright (C) 2006 - 2012 Radoslaw Mantiuk, Oliver Barth
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

//TODO slider geht nicht wirklich auf 0
//TODO menu geht nicht mehr ... repost porblem
//TODO alte fenster position merken



#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <getopt.h>

#include <unistd.h>

#include <math.h>

#include <pthread.h>
#include <signal.h>

#include "glenv.h"
#include "picture_io.h"

#include "m_histogram.h"
#include "m_status.h"
#include "m_on_screen_display.h"




#define PROG_NAME "pfsGLview"

GLint win, mainMenu, mappingSubmenu, channelSubmenu, mouse=0;		// for holding menu and window ids
PictureIO* bitmap;
PictureIO* bitmap2;
M_Histogram* m_histogram;
SC_Status* m_status;
M_OnScreenDisplay* m_osd_help;
M_OnScreenDisplay* m_osd_loading;
M_OnScreenDisplay* m_osd_mapping;
GLboolean firstTime = true;

bool verbose = false;

enum MenuIds {
	MENU_SEPARATOR,
	MENU_NONE,
	MENU_HISTOGRAM,
	MENU_MONITOR_LUM,
	MENU_MAX_LUM, 
	MENU_EXIT,
	MENU_MAPPING_LINEAR,
	MENU_MAPPING_LOG,
	MENU_MAPPING_GAMMA_1_4,
	MENU_MAPPING_GAMMA_1_8,
	MENU_MAPPING_GAMMA_2_2,
	MENU_MAPPING_GAMMA_2_6,
	MENU_SAVE,
	MENU_CHANNEL_XYZ,
	MENU_CHANNEL_X,
	MENU_CHANNEL_Y,
	MENU_CHANNEL_Z,
	MENU_CHANNEL,
	MENU_INFO,
	MENU_HELP,
	MENU_FRAME_NEXT,
	MENU_FRAME_PREVIOUS,
	MENU_HIST_RIGHT,
	MENU_HIST_LEFT,
	MENU_HIST_INCREASE,
	MENU_HIST_DECREASE,
	MENU_ZOOM_IN,
	MENU_ZOOM_OUT,
	MENU_ZOOM_RESET,
	MENU_SELECT_POINTS,
	MENU_FIX_CURSOR
};

std::string loadtext[] = {"generating histograms"} ;
std::string maptext[] = {"mapping values"} ;

//std::string helptext2[] = {
//             " ",
//} ;


std::string helptext[] = {
             "keyboard",
             "--------------------------------------------------",
             "F1          - help",
             "arrow keys  - pan view",
             "c           - cycle channel",
             ".           - zoom in",
             ",           - zoom out",
             "r           - reset",
             "-/=         - move dynamic range",
             "]           - increase exposure",
             "[           - decrease exposure",
             "i           - toggle info panel",
             "h           - toggle histogram panel",
             "CTRL-L      - LDR",
             "\\           - fit dynamic range",
             "1           - mapping gamma 1.4",
             "2           - mapping gamma 1.8",
             "3           - mapping gamma 2.2",
             "4           - mapping gamma 2.6",
             "l           - mapping linear",
             "o           - mapping log",
             "s           - set marking points",
             "n           - next frame",
             "p           - previous frame",
              "f           - fix data position",
             "g           - toggle spectrogram Gaussian",
             "m           - toggle spectrogram maximum hue",
             "x           - toggle over and under exposure",
             "space       - toggle left and middle mouse button",
             "q/ESC       - quit program",
             "",
             "mouse",
             "--------------------------------------------------",
             "left button    - pan",
             "middle button  - zoom",
             "wheel          - zoom",
             "",
             "exposure",
             "--------------------------------------------------",
             "red     - all channels under exposed",
             "green   - at least one channel over OR under exposed",
             "magenta - at least one channel over AND one under exposed",
             "blue    - all channels over exposed"
} ;



#define INIT_MIN_LUM 0.1f
#define INIT_MAX_LUM 1.0f

// the area where the M_Status and M_OnScreenDisplay is located
#define INFOAREA_WIDTH 500
#define INFOAREA_HEIGHT 55
#define INFOAREA_POS_X 10
#define INFOAREA_POS_Y 10

#define OnScreenDisplay_PERCENTAGE_MAX .85
#define OnScreenDisplay_PERCENTAGE_MIN .55

#define ZOOM_SCALE_MIN 0.1f
#define ZOOM_SCALE_MAX 100.0f
#define ZOOM_SCALE_STEP 0.005f
#define ZOOM_MOVE_STEP 2.0f

#define MONITOR_LUM_MIN 0.01f
#define MONITOR_LUM_MAX 1.0f

#define SLIDER_STEP 5
#define PAN_STEP 10

const char* channel ;

/** Mouse listener
*/
int cursorPosX = -1;
int cursorPosY = -1;

int cursorPosX_fixed = -1;
int cursorPosY_fixed = -1;


int cursorPosX_fix = -1 ;
int cursorPosY_fix = -1 ;

int pfs_wait = 0;

void resetHistogram(void);
void loadPicture();
void sliderMoveMax( int shift, int hwidth);
void sliderMoveMin( int shift, int hwidth);
void setWindowTitle( void);

//============================================

struct ZOOM {
	float scale;
	float scale_default;
	float x;
	float y;
	int vX, vY, vSizeX, vSizeY;
	int prevX, prevY;
	bool pan;
} szoom;	


struct point{
	float x;
	float y;
};

point points[1000] ;
int point_counter = 0;

float zoom_akku_x = 0 ;
float zoom_akku_y = 0 ;

int winWidth ;
int winHeight ;

int wait_to_start = 0 ;

int select_points = 0 ;
int fix_cursor = 0 ;

int resetFMax = 0 ;

int ident = 1 ;



struct VIEWPORT
{
	GLint xPos,yPos;
	GLsizei xSize, ySize;
} viewport;

float ***image_original;
/**
*/
void zoomReset(void) {
	dbs("zoomReset") ;
	szoom.scale = szoom.scale_default;
	szoom.x = 0;
	szoom.y = 0;
	szoom.prevX = -1;
	szoom.prevY = -1;
	szoom.pan = true;


	// TODO getting the width and height here will freak out the program
	// and exit with a sigsev
	int height = winHeight -Y_BAR - m_histogram->getHeight() -Y_BAR/2;
	int width = winWidth   -X_BAR ;
//
	float ratio = (float)bitmap->getHeight() / (float)bitmap->getWidth();
	float ratio2 = (float)height/(float)width ;
	szoom.scale = 1.0f; // default scale coefficient

	if( ratio < ratio2) { // vertical
		if( bitmap->getWidth() > width)
			szoom.scale = (float)(width) / (float)bitmap->getWidth();
	}
	else { // horizontal
		if( bitmap->getHeight()  > height)
			szoom.scale = (float)(height) / (float)bitmap->getHeight();
	}
	szoom.scale_default = szoom.scale;
	m_histogram->setVirtualWidth(1.0) ;
	m_histogram->setVirtualOffset(0.5) ;
	dbe("zoomReset") ;
}




void zoomIncrease(void) {
	dbs("zoomIncrease") ;
	float dd = ZOOM_SCALE_STEP * szoom.scale;
	szoom.scale += (100) * dd;

	if( szoom.scale > ZOOM_SCALE_MAX)
		szoom.scale = ZOOM_SCALE_MAX;
	dbe("zoomIncrease") ;
}
void zoomDecrease(void) {
	dbs("zoomDecrease") ;
	float dd = ZOOM_SCALE_STEP * szoom.scale;
	szoom.scale += (-100) * dd;

	if( szoom.scale < ZOOM_SCALE_MIN)
							szoom.scale = ZOOM_SCALE_MIN;
	dbe("zoomDecrease") ;
}




void adjust(M_OnScreenDisplay* win, int width, int height){
  int dw = win->getDesiredWinWidth() ;
    int dh = win->getDesiredWinHeight() ;

    if(dw < (float)width * OnScreenDisplay_PERCENTAGE_MIN)
             dw = (float)width * OnScreenDisplay_PERCENTAGE_MIN ;
     if(dh < (float)height * OnScreenDisplay_PERCENTAGE_MIN)
             dh = (float)height * OnScreenDisplay_PERCENTAGE_MIN ;

     if(dw > (float)width * OnScreenDisplay_PERCENTAGE_MAX)
             dw = (float)width * OnScreenDisplay_PERCENTAGE_MAX ;
     if(dh > (float)height * OnScreenDisplay_PERCENTAGE_MAX)
             dh = (float)height * OnScreenDisplay_PERCENTAGE_MAX ;



    win->setPosition( (float)width/2.0 - dw/2.0, (float)height/2.0 -dh/2.0);
    win->setSize( dw, dh);
}






void updateStatus(void) {
	m_status->setMapping( lumMappingName[bitmap->getMappingMethod()]);
	m_status->setMaxFreq( m_histogram->getMaxFrequency()); 	
	m_status->setFrameNo( bitmap->getFrameNo());
	m_status->setChannel( bitmap->getVisibleChannel());
	if(!szoom.pan)
	  m_status->setNavMode("ZOOM");
	else
	  m_status->setNavMode("PAN");
	m_status->setZoomScale(szoom.scale);
}

void refreshWinStat_PixelData(){
	int x = cursorPosX ;
	int y = cursorPosY ;

	int curPX = cursorPosX ;
	int curPY = cursorPosY ;

	float val;
	glReadPixels( x, glutGet(GLUT_WINDOW_HEIGHT) - y, 1, 1, GL_RED, GL_FLOAT, &val);
	int r = (int)(val * 255);
	glReadPixels( x, glutGet(GLUT_WINDOW_HEIGHT) - y, 1, 1, GL_GREEN, GL_FLOAT, &val);
	int g = (int)(val * 255);
	glReadPixels( x, glutGet(GLUT_WINDOW_HEIGHT) - y, 1, 1, GL_BLUE, GL_FLOAT, &val);
	int b = (int)(val * 255);
	m_status->setPixelData( x, y, r, g, b);
}


void refreshWinStat_RawData(){
	int xPosition, yPosition;
	int x, y ;
	if(!fix_cursor){
		xPosition = cursorPosX - szoom.vX;
//		if(histogram->getFlag())
//			yPosition = cursorPosY - (glutGet(GLUT_WINDOW_HEIGHT) - szoom.vY) -HISTOGRAM_HEIGHT/2 -Y_BAR/4;
//		else
			yPosition = cursorPosY - (glutGet(GLUT_WINDOW_HEIGHT) - szoom.vY)  ;

		x = floor( xPosition / szoom.scale);
		y = floor( yPosition / szoom.scale);

		cursorPosX_fix = x;
		cursorPosY_fix = y;
	}else{
		x = cursorPosX_fix ;
		y = cursorPosY_fix ;
	}



	float X, Y, Z;
	if((m_histogram->getSubComponentHoverState() == M_Histogram::NONE || fix_cursor)  && !bitmap->getRawData( x, y, X, Y, Z)){
		m_status->setRawData( x, y, X, Y, Z);
		m_histogram->setRawData( x, y, X, Y, Z);
	}
	else{
		m_status->setRawData( -1, -1, 0, 0, 0);
		m_histogram->setRawData( -1, -1, 0, 0, 0);
	}
}


void updateHistogram(void) {
	dbs("updateHistogram") ;
	const char* ch = bitmap->getVisibleChannel();
	m_histogram->setChannel(ch) ;
	m_histogram->setSliderPosMinMax( m_histogram->lum2pos(bitmap->getLumMin())
			, m_histogram->lum2pos(bitmap->getLumMax()));
	dbe("updateHistogram") ;
}		
	


void updateMapping_callback (){
  //TODO macht menu kaputt

	if(glutGetWindow() !=  0) glutPostRedisplay() ;
}




pthread_t          updateMappingThread ;
pthread_mutex_t    updateMappingThread_count_mutex  = PTHREAD_MUTEX_INITIALIZER;
int                updateMappingThread_count        = 0 ;
int                updateMappingThread_running      = 0 ;
pthread_mutex_t    updateMappingThread_update_mutex = PTHREAD_MUTEX_INITIALIZER;





void *updateMapping_thread(void *argument)
 {
    // TODO hier bitte echte argumente uebergeben
    int wait_to_start = *((int*)argument) ;

    int thread_num ;
    pthread_mutex_lock( &updateMappingThread_count_mutex );
    if(updateMappingThread_running < updateMappingThread_count) {
//        printf("RETURN %d %d\n", thread_running,thread_count);
        pthread_mutex_unlock( &updateMappingThread_count_mutex );
        return (NULL);
    }
    thread_num = ++updateMappingThread_count;
    bitmap->abort_update = 1 ;
    pthread_mutex_unlock( &updateMappingThread_count_mutex );

    //TODO argument
    if(wait_to_start > 0){
//        printf("WAIT %d %d \n", thread_num, wait_to_start);
        usleep(wait_to_start * 1000) ;
    }

    int r = pthread_mutex_lock( &updateMappingThread_update_mutex );
    pthread_mutex_lock( &updateMappingThread_count_mutex );
//    printf("threadnum: %d \n", thread_num);
//    printf("threadcou: %d \n", thread_count);
    if(thread_num == updateMappingThread_count){
//        printf("START %d %d\n", thread_num,thread_count);
        bitmap->abort_update = 0 ;
        updateMappingThread_running = thread_num ;
        pthread_mutex_unlock( &updateMappingThread_count_mutex );
//        winMapping->setVisible(true);
        bitmap->updateMapping(updateMapping_callback) ;
//        winMapping->setVisible(false);
        updateHistogram() ;
        updateStatus();
        updateMapping_callback() ;
    }else{
//        printf("STOP %d %d\n", thread_num,thread_count);
    }
    pthread_mutex_unlock( &updateMappingThread_count_mutex );
    pthread_mutex_unlock( &updateMappingThread_update_mutex );

    return NULL;
 }




void updateMapping(int* wait_to_start) {
	dbs("updateMapping") ;
//	char* string = {"normal run\n"};
//	char* string = {"wait\n"};
	pthread_create (&updateMappingThread, NULL, updateMapping_thread, wait_to_start);
	pthread_detach( updateMappingThread );
//	bitmap->updateMapping(CB) ;
//	redrawHistogram() ;
	dbe("updateMapping") ;
}



void updateMapping() {
	dbs("updateMapping") ;
	//      char* string = {"normal run\n"};
	//      char* string = {"wait\n"};
	pthread_create (&updateMappingThread, NULL, updateMapping_thread, &wait_to_start);
	pthread_detach( updateMappingThread );
	//      bitmap->updateMapping(CB) ;
	//      redrawHistogram() ;
	dbe("updateMapping") ;
}







/** Change mapping or redraw after changing luminance range.
*/
// not used anymore
//void changeMapping( float lumMin, float lumMax) {
//
//  // update slider
//  histogram->setSliderPosMinMax( histogram->lum2pos(lumMin), histogram->lum2pos(lumMax));
//  bitmap->changeMapping( lumMin, lumMax);
//  updateMapping() ;
//}

void sliderMoveMax( int shift, int hwidth) {
	dbs("sliderMoveMax") ;
	m_histogram->setSliderPosMax( m_histogram->getSliderPosMax() + (float)shift / hwidth);
	bitmap->setMaxLum( m_histogram->pos2lum(m_histogram->getSliderPosMax()));
	dbe("sliderMoveMax") ;
}	
void sliderMoveMin( int shift, int hwidth) {
	dbs("sliderMoveMin") ;
	m_histogram->setSliderPosMin( m_histogram->getSliderPosMin() + (float)shift / hwidth);
	bitmap->setMinLum( m_histogram->pos2lum(m_histogram->getSliderPosMin()));
	dbe("sliderMoveMin") ;
}










pthread_t computeHistogramThread ;
pthread_mutex_t    computeHistogramThread_count_mutex = PTHREAD_MUTEX_INITIALIZER;
int                computeHistogramThread_count = 0 ;
int                computeHistogramThread_running = 0 ;
pthread_mutex_t    computeHistogramThread_update_mutex = PTHREAD_MUTEX_INITIALIZER;
int   incdec = 0 ;





void computeHistogram_callback (){
  //TODO macht menu kaputt

	resetFMax = 1 ;
	if(glutGetWindow() !=  0) glutPostRedisplay() ;
}






void *computeHistogram_thread(void *argument)
 {
	  // TODO hier bitte echte argumente uebergeben
	    int wait_to_start = *((int*)argument) ;

	    int thread_num ;
	    pthread_mutex_lock( &computeHistogramThread_count_mutex );
	    if(computeHistogramThread_running < computeHistogramThread_count) {
//	        printf("RETURN %d %d\n", thread2_running,thread2_count);
	        pthread_mutex_unlock( &computeHistogramThread_count_mutex );
	        return (NULL);
	    }
	    thread_num = ++computeHistogramThread_count;
	    m_histogram->abort_update = 1 ;
	    pthread_mutex_unlock( &computeHistogramThread_count_mutex );

	    //TODO argument
	    if(wait_to_start > 0){
//	        printf("WAIT %d %d \n", thread2_num, wait_to_start);
	        usleep(wait_to_start * 1000) ;
	    }

	    int r = pthread_mutex_lock( &computeHistogramThread_update_mutex );
	    pthread_mutex_lock( &computeHistogramThread_count_mutex );
//	    printf("threadnum: %d \n", thread2_num);
//	    printf("threadcou: %d \n", thread2_count);
	    if(thread_num == computeHistogramThread_count){
//	        printf("START %d %d\n", thread2_num,thread2_count);
	    	m_histogram->abort_update = 0 ;
	        computeHistogramThread_running = thread_num ;
	        pthread_mutex_unlock( &computeHistogramThread_count_mutex );


	        pthread_mutex_lock( &updateMappingThread_count_mutex );
	        bitmap->abort_update = 1 ;
	        pthread_mutex_lock( &updateMappingThread_update_mutex );
	        if(incdec == 1)
	        	bitmap->gotoNextFrame();
	        if(incdec == -1)
	        	bitmap->gotoPreviousFrame();
	        pthread_mutex_unlock( &updateMappingThread_update_mutex );
	        pthread_mutex_unlock( &updateMappingThread_count_mutex );

	        zoomReset();
	        bitmap->setVisibleChannel( bitmap->CHANNEL_XYZ);
	        bitmap->changeMapping( MAP_GAMMA2_2);



	        m_histogram->clearChannel(0,computeHistogram_callback);
	        m_histogram->clearChannel(1,computeHistogram_callback);
	        m_histogram->clearChannel(2,computeHistogram_callback);
	        m_histogram->clearChannel(3,computeHistogram_callback);
	        m_histogram->clearChannel(4,computeHistogram_callback);

	        m_osd_loading->setVisible(true);

	        m_histogram->computeFrequency(0,bitmap->getPrimaryChannel(),computeHistogram_callback);
	        channel = bitmap->getVisibleChannel() ;
	        //	            histogram->resetFrequencyMax() ;
	        resetHistogram(); // compute histogram for a given image

	        m_histogram->computeFrequency(1, bitmap->getFrame()->getChannel("X"),computeHistogram_callback);
	        m_histogram->computeFrequency(2, bitmap->getFrame()->getChannel("Y"),computeHistogram_callback);
	        m_histogram->computeFrequency(3, bitmap->getFrame()->getChannel("Z"),computeHistogram_callback);
	        m_histogram->computeFrequency(0, NULL,computeHistogram_callback);
	        m_osd_loading->setVisible(false);
	        //	            histogram->resetFrequencyMax() ;
	        resetFMax = 1 ;

	        //	            if(thread_count == 1){
	        resetHistogram(); // compute histogram for a given image
	        //	            }
	        //	                printf("END %d %d\n", thread2_num,thread2_count);
	    }else{
	        //	        printf("STOP %d %d\n", thread2_num,thread2_count);
	    }
	    pthread_mutex_unlock( &computeHistogramThread_count_mutex );
	    pthread_mutex_unlock( &computeHistogramThread_update_mutex );

	    return NULL;
 }




void computeHistogram() {
	dbs("computeHistogram") ;
	pthread_create (&computeHistogramThread, NULL, computeHistogram_thread, &wait_to_start);
	pthread_detach( computeHistogramThread );
	dbe("computeHistogram") ;
}






	
/** Main display routine
*/
void display(void) {



	glClear(GL_COLOR_BUFFER_BIT);

	// If the picture was loaded we need to rescale it to window size	 
	if(bitmap != NULL) 	{
		GLint viewp[4];
		glGetIntegerv( GL_VIEWPORT, viewp);	
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		
		 winWidth = glutGet(GLUT_WINDOW_WIDTH);
		 winHeight = glutGet(GLUT_WINDOW_HEIGHT);
		


		szoom.vX = (int)(winWidth*0.5 - ((bitmap->getWidth() - szoom.x) * szoom.scale)*0.5);
		if(m_histogram->getVisible() == GL_TRUE) {
		  szoom.vY = (int)(winHeight*0.5 + ((bitmap->getHeight() + szoom.y) * szoom.scale)*0.5 -m_histogram->getHeight()/2-Y_BAR/4) ;
//		  szoom.vY = (int)(winHeight/2 + ((bitmap->getHeight() + szoom.y) * szoom.scale)/2 - HISTOGRAM_HEIGHT/2);
		} else {
		  szoom.vY = (int)(winHeight*0.5 + ((bitmap->getHeight() + szoom.y) * szoom.scale)*0.5);
		}
		szoom.vSizeX = (int)(bitmap->getWidth() * szoom.scale);
		szoom.vSizeY = (int)(bitmap->getHeight() * szoom.scale);

		glEnable( GL_SCISSOR_TEST);

		if(m_histogram->getVisible() == GL_TRUE) {
//		  glScissor( X_BAR/2, Y_BAR/2, winWidth - X_BAR, winHeight - Y_BAR                             ); 
		  glScissor( X_BAR/2, Y_BAR/2, winWidth - X_BAR, winHeight - Y_BAR - m_histogram->getHeight() - Y_BAR/2);
		} else {
		  glScissor( X_BAR/2, Y_BAR/2, winWidth - X_BAR, winHeight - Y_BAR                             ); 
		}
		/*
                glViewport(0 , bitmap->getHeight(), 0, 0);

	              glPixelZoom( 1, -1);
	                  glRasterPos2i(0, 0);

	                  glDrawPixels(bitmap->getWidth(), bitmap->getHeight(),
	                          GL_RGBA,
	                          GL_UNSIGNED_BYTE,
	                          bitmap->getImageData()
	                          );
*/



//		if(histogram->getFlag() == GL_TRUE) {
//		glViewport( szoom.vX, szoom.vY-HISTOGRAM_HEIGHT/2-Y_BAR/4, szoom.vSizeX, szoom.vSizeY);
//		}
//		else{
		    glViewport( szoom.vX, szoom.vY, szoom.vSizeX, szoom.vSizeY);
//		}
		glPixelZoom( szoom.scale, -szoom.scale);
		glRasterPos2i(0,0);

		glDrawPixels(bitmap->getWidth(), bitmap->getHeight(),
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			bitmap->getImageData()
			);

//printf("%d %d %d %d\n", szoom.vX, szoom.vY, szoom.vSizeX, szoom.vSizeY) ;


//		int nw = szoom.vSizeX  ;
//		int nh = szoom.vSizeY  ;
//
////		if(histogram->getFlag() == GL_TRUE) {
////			glViewport( szoom.vX, szoom.vY-HISTOGRAM_HEIGHT/2-Y_BAR/4-(float)bitmap->getHeight()*szoom.scale, nw, nh);
////			}
////			else{
//			    glViewport( szoom.vX, szoom.vY-bitmap->getHeight()*szoom.scale, nw, nh);
////			}
//
//
//		glEnable(GL_BLEND);
//
//
//
//		 glEnable (GL_TEXTURE_2D) ;
//
//		glBindTexture(GL_TEXTURE_2D, 13);
//		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
////		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
////		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
//		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, bitmap->getWidth(), bitmap->getHeight(), 0, GL_RGBA, GL_FLOAT, bitmap->getImageData());
//
////
//
////
//
//		int ww =  bitmap->getWidth();
//				int hh = bitmap->getHeight();
//				float ar = ww / hh * 100;
////
////		printf("%d %d %d %d\n", szoom.vSizeX, szoom.vSizeY, winWidth, winHeight ) ;
////
////
////
//		glBindTexture (GL_TEXTURE_2D, 13);
////		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
////		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
////		glLoadIdentity();
//		//glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
//		glBegin(GL_QUADS);
//		glTexCoord2f (0.0, 1.0);
//		glVertex3f(0,0 , -0.5f);
//
//		glTexCoord2f (0.0, 0.0);
//		glVertex3f(0,100, -0.5f);
//
//		glTexCoord2f (1.0, 0.0);
//		glVertex3f(ar,100, -0.5f);
//
//		glTexCoord2f (1.0, 1.0);
//		glVertex3f(ar,0, -0.5f);
//		glEnd();
//
//
//		glDisable (GL_TEXTURE_2D) ;
//
//		glDisable(GL_BLEND);





		glViewport( viewp[0], viewp[1], viewp[2], viewp[3]);



//
//
//		int xPos, yPos;
//			xPos = cursorPosX - szoom.vX;
//			yPos = cursorPosY - (glutGet(GLUT_WINDOW_HEIGHT) - szoom.vY) -HISTOGRAM_HEIGHT/2 ;
//
//			int xx = floor( xPos / szoom.scale);
//			int yy = floor( yPos / szoom.scale);




			float xPos, yPos;
		for (int i = 0 ; i < point_counter ; i++){
			xPos = (points[i].x) * szoom.scale  + szoom.vX;
//			if(histogram->getFlag()){
//			yPos = glutGet(GLUT_WINDOW_HEIGHT) - ((points[i].y) * szoom.scale  +  (glutGet(GLUT_WINDOW_HEIGHT) - szoom.vY) +HISTOGRAM_HEIGHT/2 +Y_BAR/4 ) ;
//			}
//			else{
				yPos = glutGet(GLUT_WINDOW_HEIGHT) - ((points[i].y) * szoom.scale  +  (glutGet(GLUT_WINDOW_HEIGHT) - szoom.vY) ) ;
//			}

			// only for BEAUTY, to center the dot in the middle of the cross cursor

			xPos=floor(xPos) ;
			yPos=floor(yPos) ;


			float mark_size = szoom.scale/4 ;
			if (mark_size < 6) {
				mark_size = 6;

			}

//				glColor4f(1.0f, 1.0f, 0.0f, 0.33f);
//				glBegin(GL_QUADS);
//				glVertex3f(xPos-mark_size, yPos-mark_size, -0.5f);
//				glVertex3f(xPos+mark_size, yPos-mark_size, -0.5f);
//				glVertex3f(xPos+mark_size, yPos+mark_size, -0.5f);
//				glVertex3f(xPos-mark_size, yPos+mark_size, -0.5f);
//				glEnd();

//				glEnable(GL_BLEND);
//				glColor4f(1.0f, 0.0f, 0.0f, 0.8f);
//				glBegin(GL_LINES);
//				glVertex3f(xPos-mark_size, yPos-mark_size, -0.5f);
//				glVertex3f(xPos+mark_size, yPos+mark_size, -0.5f);
//				glVertex3f(xPos+mark_size, yPos-mark_size, -0.5f);
//				glVertex3f(xPos-mark_size, yPos+mark_size, -0.5f);
//				glEnd();
//				glDisable(GL_BLEND);


				glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
				glPointSize(2);
				glBegin(GL_POINTS);
				glVertex3f(xPos, yPos, -0.5f);
				glEnd();

				glBegin(GL_LINE_LOOP);
				glVertex3f(xPos-mark_size, yPos-mark_size, -0.5f);
				glVertex3f(xPos+mark_size+1, yPos-mark_size, -0.5f);
				glVertex3f(xPos+mark_size+1, yPos+mark_size+1, -0.5f);
				glVertex3f(xPos-mark_size, yPos+mark_size+1, -0.5f);
				glEnd();



			}


		glDisable( GL_SCISSOR_TEST);
	}
	
	refreshWinStat_RawData() ;
	refreshWinStat_PixelData() ;

	// Histogram drawing (and slider)
	if(m_histogram->getVisible() == GL_TRUE) {
		if(m_histogram != NULL) {
			if(resetFMax){
				m_histogram->resetFrequencyMax() ;
				resetFMax = 0 ;
			}
			m_histogram->redraw();
		}
	}

	m_status->redraw();
	m_osd_help->redraw();
	m_osd_loading->redraw();
	m_osd_mapping->redraw();


	glMatrixMode(GL_COLOR);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glFlush();
	glutSwapBuffers();
}


/** Resizes main window.
*/
void resizeWindow(int width, int height) {


	#ifdef __APPLE__ // "Think different" for OS/X :)
	glutReshapeWindow(width, height);
	#endif

	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, width, 0.0f, height, -10.0f, 10.0f); // last 1.0 -1.0 it's enough

	glMatrixMode(GL_MODELVIEW);
	viewport.ySize = height;
	viewport.xSize = width;


	int newHeight = m_histogram->getHeight() ;
	if(m_histogram->getHeight() > height - INFOAREA_HEIGHT -Y_BAR - Y_BAR/2)  newHeight = height - INFOAREA_HEIGHT  -Y_BAR -Y_BAR/2;
	if(newHeight < 50)  newHeight = 50 ;
	m_histogram->setSize(width - X_BAR, newHeight ) ;
	m_histogram->setPosition( X_BAR/2, height - Y_BAR/2 - m_histogram->getHeight());


	adjust(m_osd_help, width, height) ;

	m_osd_loading->setPosition( glutGet(GLUT_WINDOW_WIDTH) - INFOAREA_WIDTH/2 - INFOAREA_POS_X, INFOAREA_POS_Y);
	m_osd_loading->setSize( INFOAREA_WIDTH/2, INFOAREA_HEIGHT);
	m_osd_loading->setBackgroundColor(0, 0, 0, 0.75) ;

	m_osd_mapping->setPosition( glutGet(GLUT_WINDOW_WIDTH) - INFOAREA_WIDTH/2 - INFOAREA_POS_X, INFOAREA_POS_Y);
	m_osd_mapping->setSize( INFOAREA_WIDTH/2, INFOAREA_HEIGHT);
	m_osd_mapping->setBackgroundColor(0, 0, 0, 0.75) ;

	glutPostRedisplay();
}



void setCursor(int cursor){
	if(cursor > 0){
		glutSetCursor(cursor);
	} else {


		if(m_histogram->getVisible()){
			m_histogram->processPointerPosition(cursorPosX, cursorPosY, szoom.pan);
		}

		if(!m_histogram->getVisible() || m_histogram->getSubComponentHoverState() == M_Histogram::NONE)
		{
			if(select_points)
				glutSetCursor(GLUT_CURSOR_CROSSHAIR);
			else if(szoom.pan)
				glutSetCursor(GLUT_CURSOR_INFO);
			else
				glutSetCursor(GLUT_CURSOR_LEFT_RIGHT);
		}



	}
}





/** Menu listener
*/
void menuListener(int menuId)
{
	dbs("menuListener") ;
	int hwidth = m_histogram->getInnerWidth() ;

	switch(menuId) {
	case MENU_FIX_CURSOR:
			fix_cursor = !fix_cursor ;
			break ;

	    case MENU_SELECT_POINTS:
	    	select_points = !select_points ;
	        break;

	    case MENU_ZOOM_IN:
			zoomIncrease();
			updateStatus();
			break;

		case MENU_ZOOM_OUT:
			zoomDecrease();
			updateStatus();
			break;

		case MENU_ZOOM_RESET:
			zoomReset();
			bitmap->setVisibleChannel( bitmap->CHANNEL_XYZ);
			bitmap->changeMapping( MAP_GAMMA2_2);
			resetHistogram();
			break;

		case MENU_HIST_RIGHT:
			sliderMoveMax( SLIDER_STEP, hwidth);
			sliderMoveMin( SLIDER_STEP, hwidth);
			updateMapping() ;
			break;

		case MENU_HIST_LEFT:
			sliderMoveMin( -SLIDER_STEP, hwidth);
			sliderMoveMax( -SLIDER_STEP, hwidth);
			updateMapping() ;
			break;

		case MENU_HIST_INCREASE:
			sliderMoveMin( -SLIDER_STEP, hwidth);
			sliderMoveMax( SLIDER_STEP, hwidth);
			updateMapping() ;
			break;

		case MENU_HIST_DECREASE:
			sliderMoveMin( SLIDER_STEP, hwidth);
			sliderMoveMax( -SLIDER_STEP, hwidth);
			updateMapping() ;
			break;
										
		case MENU_FRAME_NEXT: 
			incdec = 1 ;
			setWindowTitle();
			// TODO not safe because of running threads;
//			thread_count = 0 ;
			pthread_mutex_lock( &computeHistogramThread_count_mutex );
			m_histogram->abort_update = 1 ;
			pthread_mutex_unlock( &computeHistogramThread_count_mutex );
			computeHistogram() ;
			break;
			
		case MENU_FRAME_PREVIOUS: 
			incdec = -1 ;
			setWindowTitle();
			// TODO not safe ;
//			thread_count = 0 ;
			pthread_mutex_lock( &computeHistogramThread_count_mutex );
			m_histogram->abort_update = 1 ;
			pthread_mutex_unlock( &computeHistogramThread_count_mutex );
			computeHistogram() ;
			break;

		case MENU_HISTOGRAM :
         		m_histogram->setVisible( !m_histogram->getVisible());
			break;

		case MENU_INFO:
			m_status->setVisible( !m_status->getVisible());
			break;

		case MENU_HELP:
		        m_osd_help->setVisible( !m_osd_help->getVisible());
			break;
			
		case MENU_MONITOR_LUM :	
			bitmap->changeMapping( MONITOR_LUM_MIN, MONITOR_LUM_MAX);
			updateMapping() ;
			break;
			
		case MENU_MAX_LUM :
			float min, max;
			min = bitmap->computeLumMin();
			max = bitmap->computeLumMax();
			bitmap->changeMapping( min, max);
			updateMapping() ;
			break;

		case MENU_MAPPING_LINEAR: 
			bitmap->changeMapping( MAP_LINEAR);
			updateMapping() ;
			break;

		case MENU_MAPPING_GAMMA_1_4: 
			bitmap->changeMapping( MAP_GAMMA1_4);
			updateMapping() ;
			break;

		case MENU_MAPPING_GAMMA_1_8: 
			bitmap->changeMapping( MAP_GAMMA1_8);
			updateMapping() ;
			break;

		case MENU_MAPPING_GAMMA_2_2: 
			bitmap->changeMapping( MAP_GAMMA2_2);
			updateMapping() ;
			break;

		case MENU_MAPPING_GAMMA_2_6: 
			bitmap->changeMapping( MAP_GAMMA2_6);
			updateMapping() ;
			break;

		case MENU_MAPPING_LOG: 
			bitmap->changeMapping( MAP_LOGARITHMIC);
			updateMapping() ;
			break;
			
		case MENU_CHANNEL_XYZ: 
			bitmap->setVisibleChannel( bitmap->CHANNEL_XYZ);
			updateMapping() ;
			channel = bitmap->getVisibleChannel() ;
			break;

		case MENU_CHANNEL_X: 
			bitmap->setVisibleChannel( bitmap->CHANNEL_X);
			updateMapping() ;
			channel = bitmap->getVisibleChannel() ;
			break;		

		case MENU_CHANNEL_Y: 
			bitmap->setVisibleChannel( bitmap->CHANNEL_Y);
			updateMapping() ;
			channel = bitmap->getVisibleChannel() ;
			break;		

		case MENU_CHANNEL_Z: 
			bitmap->setVisibleChannel( bitmap->CHANNEL_Z);
			updateMapping() ;
			channel = bitmap->getVisibleChannel() ;
			break;	
		
		case MENU_SAVE: // save current bitmap data (8-bits RGB) to stdout
			bitmap->save();
			exit(0);
			break;
					
		case MENU_EXIT: 
			exit(0); 
			break;
			
	}
	setCursor(0) ;
	glutPostRedisplay();
	dbe("menuListener") ;
}



void mouseListener(int button, int state, int x, int y)
{
	dbs("mouseListener") ;

	// this one is a matter of taste
	// It changes the behaviour after mouse button release over the histogram.
	// If enabled, mouse over histogram is sufficient to trigger histogram.
	// If disabled first you have to move the mouse to enable histogram actions
	// after a button release over the histogram.

	// if(histogram->getFlag() == GL_TRUE)
	//   histogram->processSliderSelection(x, y);

	cursorPosX = x; // required for mouse motion listener
	cursorPosY = y;




	// slider was moved, the bitmap should be recalculated
	if( m_histogram->getVisible() == GL_TRUE 
			&& state == GLUT_UP
			&& (button != 3 && button != 4)
			&&
			(m_histogram->getSubComponentHoverState() == M_Histogram::LEFT_BAR
			|| m_histogram->getSubComponentHoverState() == M_Histogram::RIGHT_BAR
			|| m_histogram->getSubComponentHoverState() == M_Histogram::WHOLE_SLIDER
			)
	) {

//		bitmap->changeMapping();
//		updateMapping() ;
//		histogram->processSliderSelection(x, y);
	}

	if(button == GLUT_MIDDLE_BUTTON) {
		szoom.pan = !szoom.pan;
		updateStatus();
	}


	if(select_points && button == GLUT_LEFT_BUTTON && state == GLUT_UP
			&&(cursorPosX_fixed == cursorPosX && cursorPosY_fixed == cursorPosY)) {



		float xPos, yPos;
			xPos = cursorPosX - szoom.vX;
//			if(histogram->getFlag()){
//			yPos = cursorPosY - (glutGet(GLUT_WINDOW_HEIGHT) - szoom.vY) -HISTOGRAM_HEIGHT/2 -Y_BAR/4 ;
//			}
//			else{
				yPos = cursorPosY - (glutGet(GLUT_WINDOW_HEIGHT) - szoom.vY)  ;
//			}

			float xx =  xPos / szoom.scale;
			float yy =  yPos / szoom.scale;

		points[point_counter].x = xx ;
		points[point_counter].y = yy ;
		printf("point: %f %f\n", xx, yy) ;
		point_counter++ ;
	}



	// reset cursor position (for zooming)
	szoom.prevX = -1;
	szoom.prevY = -1;


	if((button == 3 || button == 4)  && state == GLUT_DOWN && m_histogram->getSubComponentHoverState() == M_Histogram::NONE ) {


        //*********************
		float oxPos, oyPos;

		oxPos = (float)winWidth*0.5 - (((float)bitmap->getWidth() - szoom.x) * szoom.scale)*0.5;
		if(m_histogram->getVisible() == GL_TRUE) {
			oyPos = (float)winHeight*0.5 + (((float)bitmap->getHeight() + szoom.y) * szoom.scale)*0.5 -m_histogram->getHeight()/2-Y_BAR/4 ;
		} else {
			oyPos = (float)winHeight*0.5 + (((float)bitmap->getHeight() + szoom.y) * szoom.scale)*0.5;
		}
		float xPos, yPos;
		xPos = (float)cursorPosX - oxPos;
		yPos = (float)cursorPosY - (float) glutGet(GLUT_WINDOW_HEIGHT) + oyPos  ;

		float xx =  xPos / szoom.scale;
		float yy =  yPos / szoom.scale;
		//----
		float dd = ZOOM_SCALE_STEP * szoom.scale;
		if(button == 3)	     szoom.scale -= 50 * dd;
		else if(button == 4) szoom.scale += 50 * dd;

		if( szoom.scale < ZOOM_SCALE_MIN)
			szoom.scale = ZOOM_SCALE_MIN;
		if( szoom.scale > ZOOM_SCALE_MAX)
			szoom.scale = ZOOM_SCALE_MAX;
        //*********************
		oxPos = (float)cursorPosX - xx * szoom.scale ;
		 szoom.x  = ((2 * oxPos - (float)winWidth)/szoom.scale + (float)bitmap->getWidth()) ;

		 oyPos = -(float)cursorPosY + (float) glutGet(GLUT_WINDOW_HEIGHT) + yy * szoom.scale ;
		 if(m_histogram->getVisible() == GL_TRUE) {
			 szoom.y  = ((2.0 * oyPos - (float)winHeight +m_histogram->getHeight()+Y_BAR/2)/szoom.scale - (float)bitmap->getHeight()) ;
		 }else{
			 szoom.y  = ((2.0 * oyPos - (float)winHeight)/szoom.scale - (float)bitmap->getHeight()) ;
		 }
		 //----


		updateStatus();
	}

	if((button == 3 || button == 4)  && state == GLUT_DOWN && m_histogram->getSubComponentHoverState() != M_Histogram::NONE ) {


//	    float spos =   (float)(cursorPosX-X_BAR-10)/(float)hwidth ;

//	    printf("spos: %f\n", spos) ;

//	    if(histogram->getSliderSelectionState() == BACK ){

        //*********************
		 float hwidth = (float) m_histogram->getInnerWidth() ;
		 float vwidth = m_histogram->getVirtualWidth() ;
		 float voff =  m_histogram->getVirtualOffset()  ;
		 float rel_offset = ((vwidth-1.0)*hwidth)/(vwidth * hwidth)*voff + 1/vwidth*(((float)cursorPosX-X_BAR/2-m_histogram->getSidebarWidth())/hwidth)  ;
		 //-----------------

	    float dd = ZOOM_SCALE_STEP * vwidth;
            if(button == 3)      vwidth -= 25 * dd;
            else if(button == 4) vwidth += 25 * dd;

	  if(vwidth > ZOOM_SCALE_MAX ) vwidth = ZOOM_SCALE_MAX ;
	  else if(vwidth < 1) vwidth = 1;
	  m_histogram->setVirtualWidth(vwidth) ;

      //*********************
	  if(vwidth > 1){
	  voff =  (rel_offset - 1/vwidth*(((float)cursorPosX-X_BAR/2-m_histogram->getSidebarWidth())/hwidth)) * (vwidth * hwidth)/((vwidth-1.0)*hwidth) ;

	  if(voff<0 || rel_offset<0) voff = 0 ;
	  if(voff>1 || rel_offset>1) voff = 1 ;
	  m_histogram->setVirtualOffset(voff)  ;
	  }
//-----------------
	  //	    }
//	    else{
//	          float dd = 1;
//	            if(button == 3)      dd *= -1;
//
//	        sliderMoveMin( (dd), vwidth*hwidth);
//	        sliderMoveMax( -(dd), vwidth*hwidth);
//	        bitmap->changeMapping();
//	  updateMapping() ;
//	    }
	}

	setCursor(0) ;
	cursorPosX_fixed = x; // required for mouse motion listener
	cursorPosY_fixed = y;

	resizeWindow( glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT) );
	glutPostRedisplay();
	dbe("mouseListener") ;
}



void mouseMotionListener(int x, int y) {

bool is_control_down = false;
//	printf("XX %d\n", glutGetModifiers()) ;
	 is_control_down = glutGetModifiers() & GLUT_ACTIVE_CTRL  ;


	if(select_points && (cursorPosX_fixed != cursorPosX || cursorPosY_fixed != cursorPosY)){
//		select_points = 0 ;
		setCursor(GLUT_CURSOR_INFO) ;
//		select_points = 1 ;
	}




  float hwidth = (float) m_histogram->getInnerWidth() ;
  float vwidth = m_histogram->getVirtualWidth() ;

	if(m_histogram->getVisible() == GL_TRUE) // slider
	{
//	printf("%d\n" , selectionState) ;
		switch(m_histogram->getSubComponentHoverState())
		{
			case M_Histogram::LEFT_BAR:

				sliderMoveMin( x - cursorPosX, vwidth*hwidth);
//				cursorPosX = x;
				
//				glutSetCursor(GLUT_CURSOR_LEFT_SIDE);
				  updateMapping(&pfs_wait) ;
				break;
				
			case M_Histogram::RIGHT_BAR:

				sliderMoveMax( x - cursorPosX, vwidth*hwidth);
//				cursorPosX = x;

//				glutSetCursor(GLUT_CURSOR_RIGHT_SIDE);
                                updateMapping(&pfs_wait) ;
				break;
				



			case M_Histogram::WHOLE_SLIDER:
			case M_Histogram::BACK:
				if(is_control_down){
					if(!szoom.pan){
						sliderMoveMin( -(x - cursorPosX), vwidth*hwidth);
						sliderMoveMin( -(cursorPosY -y), vwidth*hwidth);
						sliderMoveMax( (x - cursorPosX), vwidth*hwidth);
						sliderMoveMax( (cursorPosY- y), vwidth*hwidth);

					}else{
						if (x - cursorPosX < 0){
							sliderMoveMin( x - cursorPosX, vwidth*hwidth);
							sliderMoveMax( x - cursorPosX, vwidth*hwidth);
						}
						else{
							sliderMoveMax( x - cursorPosX, vwidth*hwidth);
							sliderMoveMin( x - cursorPosX, vwidth*hwidth);
						}
					}
					//				cursorPosX = x;

					//				glutSetCursor(GLUT_CURSOR_LEFT_RIGHT);

					updateMapping(&pfs_wait) ;
					break;
				}else{
				}

				if(!szoom.pan){
					//*********************
					//                         	 float hwidth = (float) histogram->getWidth() ;
					//                         		 float vwidth = histogram->getVirtualWidth() ;
					float voff =  m_histogram->getVirtualOffset()  ;
					float rel_offset = ((vwidth-1.0)*hwidth)/(vwidth * hwidth)*voff + 1/vwidth*(((float)cursorPosX_fixed-X_BAR/2-m_histogram->getSidebarWidth())/hwidth)  ;
					if(rel_offset<0) rel_offset = 0 ;
					if(rel_offset>1) rel_offset = 1 ;
					//-----------------



					float dd = ZOOM_SCALE_STEP * vwidth;
					vwidth += (x-cursorPosX) * dd;
					vwidth += (cursorPosY-y) * dd;
					if(vwidth > ZOOM_SCALE_MAX ) vwidth = ZOOM_SCALE_MAX ;
					else if(vwidth < 1) vwidth = 1;
					m_histogram->setVirtualWidth(vwidth) ;


					//*********************
					if(vwidth > 1){
						voff =  (rel_offset - 1/vwidth*(((float)cursorPosX_fixed-X_BAR/2-m_histogram->getSidebarWidth())/hwidth)) * (vwidth * hwidth)/((vwidth-1.0)*hwidth) ;
						if(voff<0) voff = 0 ;
						if(voff>1) voff = 1 ;
						m_histogram->setVirtualOffset(voff)  ;
					}
					//-----------------


				}else{
					float offset = m_histogram->getVirtualOffset() ;
					offset -= (x-cursorPosX) * 1/((vwidth-1.0)*hwidth)  ;
					if(offset > 1.0) offset = 1.0;
					else if(offset < -0.0) offset = 0.0;
					else if( isnan(offset) ) offset = 0.0;

					//                              printf("setting offset: %f\n", offset) ;
					m_histogram->setVirtualOffset(offset) ;
				}

				if(m_histogram->getSubComponentHoverState() == M_Histogram::WHOLE_SLIDER) break;



			case M_Histogram::BOTTOM:
								int newHeight = m_histogram->getHeight() + (y-cursorPosY)  ;
								if(newHeight < 75)  newHeight = 75 ;
								if(newHeight > glutGet(GLUT_WINDOW_HEIGHT) - INFOAREA_HEIGHT -Y_BAR - Y_BAR/2)  newHeight = glutGet(GLUT_WINDOW_HEIGHT) - INFOAREA_HEIGHT  -Y_BAR -Y_BAR/2;

				//				cursorPosY = y;
								m_histogram->setHeight(newHeight) ;
								m_histogram->setPosition( X_BAR/2, glutGet(GLUT_WINDOW_HEIGHT) - Y_BAR/2 - m_histogram->getHeight());

								//glutPostRedisplay();
								break ;


		}

	}
	
	if(m_histogram->getSubComponentHoverState() == M_Histogram::NONE) {
	// zooming 
//			if(y > (HISTOGRAM_HEIGHT + Y_BAR)) {
	
			if( szoom.prevX != -1){
				if( szoom.pan) {
					float dd = ZOOM_MOVE_STEP / szoom.scale; 
					szoom.x += ((x - szoom.prevX) * dd);
					szoom.y += ((szoom.prevY - y) * dd);
//					zoom_akku_x += ((x - szoom.prevX) * dd);
//					zoom_akku_y += ((szoom.prevY - y) * dd);
//					if(fabs(zoom_akku_x) > 1){
//					  szoom.x += (zoom_akku_x>0?floor(zoom_akku_x):ceil(zoom_akku_x)) ;
//					  zoom_akku_x = zoom_akku_x -  (zoom_akku_x>0?floor(zoom_akku_x):ceil(zoom_akku_x)) ;
//					}
//					if(fabs(zoom_akku_y) > 1){
//					  szoom.y += (zoom_akku_y>0?floor(zoom_akku_y):ceil(zoom_akku_y)) ;
//					  zoom_akku_y = zoom_akku_y -  (zoom_akku_y>0?floor(zoom_akku_y):ceil(zoom_akku_y)) ;
//					}


				}
				else {


                    //*********************
						float oxPos, oyPos;

						oxPos = (float)winWidth*0.5 - (((float)bitmap->getWidth() - szoom.x) * szoom.scale)*0.5;
						if(m_histogram->getVisible() == GL_TRUE) {
							oyPos = (float)winHeight*0.5 + (((float)bitmap->getHeight() + szoom.y) * szoom.scale)*0.5 -m_histogram->getHeight()/2-Y_BAR/4 ;
						} else {
							oyPos = (float)winHeight*0.5 + (((float)bitmap->getHeight() + szoom.y) * szoom.scale)*0.5;
						}
						float xPos, yPos;
						xPos = (float)cursorPosX_fixed - oxPos;
						yPos = (float)cursorPosY_fixed - (float) glutGet(GLUT_WINDOW_HEIGHT) + oyPos  ;

						float xx =  xPos / szoom.scale;
						float yy =  yPos / szoom.scale;
						//----


					float dd = ZOOM_SCALE_STEP * szoom.scale;
					szoom.scale -= ( szoom.prevX - x) * dd;
					szoom.scale += ( szoom.prevY - y) * dd;
	
					if( szoom.scale < ZOOM_SCALE_MIN)
						szoom.scale = ZOOM_SCALE_MIN;
					if( szoom.scale > ZOOM_SCALE_MAX)
						szoom.scale = ZOOM_SCALE_MAX;	


                    //*********************
							oxPos = (float)cursorPosX_fixed - xx * szoom.scale ;
							 szoom.x  = ((2 * oxPos - (float)winWidth)/szoom.scale + (float)bitmap->getWidth()) ;

							 oyPos = -(float)cursorPosY_fixed + (float) glutGet(GLUT_WINDOW_HEIGHT) + yy * szoom.scale ;
							 if(m_histogram->getVisible() == GL_TRUE) {
								 szoom.y  = ((2.0 * oyPos - (float)winHeight +m_histogram->getHeight()+Y_BAR/2)/szoom.scale - (float)bitmap->getHeight()) ;
							 }else{
								 szoom.y  = ((2.0 * oyPos - (float)winHeight)/szoom.scale - (float)bitmap->getHeight()) ;
							 }
							 //----


					updateStatus();
				}	
			}	
			szoom.prevX = x;
			szoom.prevY = y;
}

	cursorPosX = x; // required for mouse motion listener
	cursorPosY = y;
	glutPostRedisplay();
}


/** Mouse passive listener.
*/
void mousePassiveMotionListener(int x, int y) {
	cursorPosX = x;
	cursorPosY = y;

	setCursor(0) ;
	glutPostRedisplay();
}






/** Keyboard listener.
*/
void keyListener(unsigned char key, int x, int y) {
	dbs("keyListener") ;
	int menuFuncId = MENU_NONE;
	switch (key) {
		case '.': menuFuncId = MENU_ZOOM_IN; break;
		case ',': menuFuncId = MENU_ZOOM_OUT; break;
		case 'r': menuFuncId = MENU_ZOOM_RESET; break;
		case '=': menuFuncId = MENU_HIST_RIGHT;break;
		case '-': menuFuncId = MENU_HIST_LEFT; break;
		case ']': menuFuncId = MENU_HIST_INCREASE; break;
		case '[': menuFuncId = MENU_HIST_DECREASE; break;
		case 'h': menuFuncId = MENU_HISTOGRAM; break;
		case 'i': menuFuncId = MENU_INFO; break;
		case 12 : menuFuncId = MENU_MONITOR_LUM; break;
		case '\\': menuFuncId = MENU_MAX_LUM; break;
		case '1': menuFuncId = MENU_MAPPING_GAMMA_1_4; break;
		case '2': menuFuncId = MENU_MAPPING_GAMMA_1_8; break;
		case '3': menuFuncId = MENU_MAPPING_GAMMA_2_2; break;
		case '4': menuFuncId = MENU_MAPPING_GAMMA_2_6; break;
		case 'l': menuFuncId = MENU_MAPPING_LINEAR; break;
		case 'o': menuFuncId = MENU_MAPPING_LOG; break;
		case 'n': menuFuncId = MENU_FRAME_NEXT; break;	
		case 'p': menuFuncId = MENU_FRAME_PREVIOUS; break;	
		case ' ': 
		  szoom.pan = !szoom.pan;
		  updateStatus();
		  szoom.prevX = -1;
		  szoom.prevY = -1;
		  menuFuncId = MENU_NONE;
		  break;
		case 'c':
		  pthread_mutex_lock( &updateMappingThread_count_mutex );
		  bitmap->abort_update = 1 ;
		  pthread_mutex_unlock( &updateMappingThread_count_mutex );
		  if(strcmp(channel, "XYZ") == 0){
		      menuFuncId = MENU_CHANNEL_X; break;
		  }
		  else{
		      switch(channel[0]) {
		      case 'X':
		      menuFuncId = MENU_CHANNEL_Y; break;
		      case 'Y':
		        menuFuncId = MENU_CHANNEL_Z; break;
		      case 'Z':
		        menuFuncId = MENU_CHANNEL_XYZ; break;
		      }
		  }
		  break ;
		case 'f': menuFuncId = MENU_FIX_CURSOR ; break ;
		case 'g': m_histogram->set_is_scale_gaussed(!m_histogram->get_is_scale_gaussed()) ; break ;
		case 'm': m_histogram->set_is_scale_show_max_hue(!m_histogram->get_is_scale_show_max_hue()) ; break ;
		case 'x': bitmap->set_is_show_clipping(!bitmap->get_is_show_clipping()) ; updateMapping(); break ;
		case 's': menuFuncId = MENU_SELECT_POINTS; break ;
		case 'q':
		case 27: exit(0); break;
	}
	menuListener(menuFuncId);
	dbe("keyListener") ;
}


void specialKeyListener(int key, int x, int y) {
	dbs("specialKeyListener") ;
	int menuFuncId = MENU_NONE;

	switch (key) {
	case GLUT_KEY_F1 :
	   menuFuncId = MENU_HELP; break;
		break;

	case GLUT_KEY_LEFT :
			szoom.x -= PAN_STEP;
			glutPostRedisplay();
			break;
			
		case GLUT_KEY_RIGHT: 
			szoom.x += PAN_STEP;
			glutPostRedisplay();
			break;
			
		case GLUT_KEY_UP:
			szoom.y += PAN_STEP;
			glutPostRedisplay();	
			break;
			
		case GLUT_KEY_DOWN :
			szoom.y -= PAN_STEP;
			glutPostRedisplay();	
			break;
	}	
	menuListener(menuFuncId);
	dbe("specialKeyListener") ;
}


/** Loads the bitmap from disk.  Display an error message if it doesn't load...
*/
void loadPicture() {
	dbs("loadPicture") ;
	bitmap = new PictureIO(MAP_GAMMA2_2, INIT_MIN_LUM, INIT_MAX_LUM);
	dbe("loadPicture") ;
}











/**
*/
void resetHistogram(void) {
	dbs("resetHistogram") ;
	float min, max;
	m_histogram->computeLumRange( min, max); // starting luminance range on the slider (computed based on the histogram shape)

	// mapping method
	bitmap->changeMapping( min, max);
	updateMapping() ;
	zoomReset();
	dbe("resetHistogram") ;
}



/** Changes main window title.
*/
void setWindowTitle( void) {
	 dbs("setWindowTitle") ;
	// set window title
	char title[2000];
	sprintf(title, "PFS GLview v.1.2     %s   %dx%d", bitmap->getCurrentFileName(), bitmap->getWidth(), bitmap->getHeight());
	glutSetWindowTitle(title);
	 dbe("setWindowTitle") ;
}


/** Parses command line.
 */
int pfsglview( int argc, char* argv[]) {
	dbs("pfsglview") ;

	static struct option cmdLineOptions[] = {
			{ "help", no_argument, NULL, 'h' },
			{ "verbose", no_argument, NULL, 'v' },
			{ NULL, 0, NULL, 0 }
	};

	int optionIndex = 0;
	while( 1 ) {
		int c = getopt_long (argc, argv, "hv", cmdLineOptions, &optionIndex);
		if( c == -1 ) break;
		switch( c ) {
		case 'h':
			fprintf( stderr, "pfsglview [--verbose] [--help]\nHigh dynamic range image viewer. Use within pfstools pipe (e.g. pfsin image.hdr | pfsglview).\n");
			return 1;

		case 'v':
			verbose = true;
			break;
		}
	}

	dbe("pfsglview") ;
	return 0;
}








int once = 1 ;
void idle(){
  if(once){
//		char* strings[] = {"compute histograms\n"};

	  computeHistogram() ;
	  zoomReset();
		once = 0 ;
  }
  usleep(1000) ;
//  glutPostRedisplay() ;
}






/** Main routine.
*/
int main( int argc, char* argv[] )
{
	 dbs("main") ;

	if( pfsglview( argc, argv ))
		return 1;

	loadPicture();

	m_histogram = new M_Histogram();
	m_status = new SC_Status();
	m_osd_help = new M_OnScreenDisplay(helptext, sizeof( helptext ) / sizeof( helptext[0] ) );
	m_osd_loading = new M_OnScreenDisplay(loadtext, sizeof( loadtext ) / sizeof( loadtext[0] ) );
	m_osd_mapping = new M_OnScreenDisplay(maptext, sizeof( maptext ) / sizeof( maptext[0] ) );


	glutInit(&argc, argv);
	glutInitWindowPosition(0.1 * glutGet(GLUT_SCREEN_WIDTH), 0.1 * glutGet(GLUT_SCREEN_HEIGHT));
	glutInitWindowSize(0.8 * glutGet(GLUT_SCREEN_WIDTH), 0.8 * glutGet(GLUT_SCREEN_HEIGHT));
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);


	win = glutCreateWindow("PFS GLview v.1.2.1");
	//	  glutPositionWindow(0.25 * glutGet(GLUT_SCREEN_WIDTH), 0.25 * glutGet(GLUT_SCREEN_HEIGHT) );
	//	  glutReshapeWindow(0.5 * glutGet(GLUT_SCREEN_WIDTH), 0.5 * glutGet(GLUT_SCREEN_HEIGHT) );

	m_histogram->setPosition( X_BAR/2, glutGet(GLUT_WINDOW_HEIGHT) - Y_BAR/2 - m_histogram->getHeight());
	//	m_histogram->setSize( glutGet(GLUT_WINDOW_WIDTH) , Y_BAR);
//	m_histogram->setWinSize( HISTOGRAM_WIDTH, HISTOGRAM_HEIGHT);

	m_status->setPosition( INFOAREA_POS_X, INFOAREA_POS_Y);
	m_status->setSize( INFOAREA_WIDTH, INFOAREA_HEIGHT);

	adjust(m_osd_help, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT)) ;

	m_osd_loading->setHasBorder(false) ;
	m_osd_loading->setPosition( glutGet(GLUT_WINDOW_WIDTH) - INFOAREA_WIDTH/2 - INFOAREA_POS_X, INFOAREA_POS_Y);
	m_osd_loading->setSize( INFOAREA_WIDTH/2, INFOAREA_HEIGHT);
	m_osd_mapping->setHasBorder(false) ;
	m_osd_mapping->setPosition( glutGet(GLUT_WINDOW_WIDTH) - INFOAREA_WIDTH/2 - INFOAREA_POS_X, INFOAREA_POS_Y);
	m_osd_mapping->setSize( INFOAREA_WIDTH/2, INFOAREA_HEIGHT);



	setWindowTitle();

	// popup menu
	mappingSubmenu = glutCreateMenu(menuListener);
	glutAddMenuEntry("Gamma 1.4 (1)", MENU_MAPPING_GAMMA_1_4);
	glutAddMenuEntry("Gamma 1.8 (2)", MENU_MAPPING_GAMMA_1_8);
	glutAddMenuEntry("Gamma 2.2 (3)", MENU_MAPPING_GAMMA_2_2);
	glutAddMenuEntry("Gamma 2.6 (4)", MENU_MAPPING_GAMMA_2_6);
	glutAddMenuEntry("Linear (L)", MENU_MAPPING_LINEAR);	
	glutAddMenuEntry("Logarithmic (O)", MENU_MAPPING_LOG);
	
	channelSubmenu = glutCreateMenu(menuListener);
	glutAddMenuEntry("XYZ", MENU_CHANNEL_XYZ);

	
	//std::vector<const char*> vec = bitmap->getChannelNames();
	//for(int i = 0; i < vec.size(); i ++)
	//	glutAddMenuEntry(vec[i], MENU_CHANNEL+MENU_CHANNEL_SHIFT+i);
	
	glutAddMenuEntry("X", MENU_CHANNEL_X);	
	glutAddMenuEntry("Y", MENU_CHANNEL_Y);	
	glutAddMenuEntry("Z", MENU_CHANNEL_Z);
	
	mainMenu = glutCreateMenu(menuListener);

	glutAddMenuEntry("Zoom reset (r)", MENU_ZOOM_RESET);
	glutAddMenuEntry("Zoom in (.)", MENU_ZOOM_IN);
	glutAddMenuEntry("Zoom out (,)", MENU_ZOOM_OUT);
	glutAddMenuEntry("",MENU_SEPARATOR);
	glutAddMenuEntry("Increase exposure (=)", MENU_HIST_RIGHT);
	glutAddMenuEntry("Decrease exposure (-)", MENU_HIST_LEFT);
	glutAddMenuEntry("Extend dynamic range (])", MENU_HIST_INCREASE);
	glutAddMenuEntry("Shrink dynamic range ([)", MENU_HIST_DECREASE);
	glutAddMenuEntry("Low dynamic range (Ctrl-L)", MENU_MONITOR_LUM);
	glutAddMenuEntry("Fit to dynamic range (\\)", MENU_MAX_LUM);
	glutAddMenuEntry("",MENU_SEPARATOR);
	glutAddSubMenu("Choose channel", channelSubmenu);
	glutAddSubMenu("Mapping method", mappingSubmenu);
	glutAddMenuEntry("",MENU_SEPARATOR);
	glutAddMenuEntry("Next frame (n)", MENU_FRAME_NEXT);
	glutAddMenuEntry("Previous frame (p)", MENU_FRAME_PREVIOUS);	
	glutAddMenuEntry("",MENU_SEPARATOR);
	glutAddMenuEntry("Histogram (h)", MENU_HISTOGRAM);
	glutAddMenuEntry("Info (i)", MENU_INFO);
	glutAddMenuEntry("Help (F1)", MENU_HELP);
	glutAddMenuEntry("",MENU_SEPARATOR);	
	glutAddMenuEntry("Save&Quit", MENU_SAVE);
	glutAddMenuEntry("Quit (Q or Esc)", MENU_EXIT);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
	

	float ratio = (float)bitmap->getHeight() / (float)bitmap->getWidth();
	szoom.scale = 1.0f; // default scale coefficient
	szoom.scale_default = szoom.scale;	
	zoomReset();


	// viewport
	GLint viewportSize[4];
	glGetIntegerv(GL_VIEWPORT, viewportSize);

	viewport.xPos = viewportSize[0];
	viewport.yPos = viewportSize[1];
	viewport.xSize = viewportSize[2];
	viewport.ySize = viewportSize[3];

	// mouse listeners
 	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glutMouseFunc(mouseListener);
	glutMotionFunc(mouseMotionListener);
	glutPassiveMotionFunc( mousePassiveMotionListener);
	
	// keyboard listeners
	glutKeyboardFunc(keyListener);
	glutSpecialFunc(specialKeyListener);
	
	glutDisplayFunc(display);
	glutReshapeFunc(resizeWindow);
	glutIdleFunc(idle) ;

	resizeWindow( glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT) );

	glutMainLoop();

	return 0;   
}




