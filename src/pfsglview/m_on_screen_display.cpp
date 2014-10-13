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
#include <string>

#include "m_on_screen_display.h"

#define FONT         GLUT_BITMAP_8_BY_13
#define FONT_WIDTH   8
#define FONT_HEIGHT  13
#define ROW_SPACING  2
#define BORDER_OUTER 5
#define BORDER_INNER 20
#define BORDER       ((hasBorder)?(BORDER_OUTER + BORDER_INNER):0)





M_OnScreenDisplay::M_OnScreenDisplay(std::string* s, int row_count) : Module() {
	isVisible = GL_FALSE;
	hasBorder = true ;
	text = s ;

	winBackgroundColor = new float[4];
	for(int i=0; i<3; i++){
		winBackgroundColor[i] = 0.2f;
	}
	winBackgroundColor[3] = 0.95f;


	this->row_count = row_count ;
	max_row_length = 0 ;
	for(int i=0; i < row_count; i++){
		if(max_row_length < text[i].length()) max_row_length = text[i].length() ;
	}
}



M_OnScreenDisplay::~M_OnScreenDisplay() {
}



void M_OnScreenDisplay::redraw(void) {

	if( Module::redrawStart())
		return;

	float offset_y = ((height-2*BORDER)/(FONT_HEIGHT + ROW_SPACING)-row_count)/2.0    ;
	offset_y = offset_y > 0 ? offset_y : 0;

	start_y = offset_y ;
	glColor3f(0.5f, 1.0f, 0.5f);

	if(hasBorder){
	    glBegin(GL_LINE_STRIP);
	    glVertex3f((BORDER_OUTER-1)                                     , BORDER_OUTER                                          , -0.5f);
	    glVertex3f(BORDER_OUTER + width-(BORDER_OUTER-1)-BORDER_OUTER, BORDER_OUTER                                          , -0.5f);
	    glVertex3f(BORDER_OUTER + width-(BORDER_OUTER-1)-BORDER_OUTER, BORDER_OUTER + height-(BORDER_OUTER-1)-BORDER_OUTER, -0.5f);
	    glVertex3f(BORDER_OUTER                                         , BORDER_OUTER + height-(BORDER_OUTER-1)-BORDER_OUTER, -0.5f);
	    glVertex3f(BORDER_OUTER                                         , (BORDER_OUTER-1)                                      , -0.5f);
	    glEnd();
	}

	glEnable(GL_SCISSOR_TEST);
	glScissor( pos_x + BORDER, pos_y + BORDER, width - 2 * BORDER, height - 2 * BORDER);
	
	for(int i=0; i < row_count; i++){
		println(text[i]) ;
	}

	glDisable( GL_SCISSOR_TEST);
	Module::redrawEnd();
}



void M_OnScreenDisplay::println(std::string s){


	int offset_x = (width - 2 * BORDER  - (max_row_length) * FONT_WIDTH)/2 ;
	int offset_y = height  - start_y * (FONT_HEIGHT + ROW_SPACING) -FONT_HEIGHT + ROW_SPACING  ;
	offset_x = offset_x > 0 ? offset_x : 0;
	offset_x += BORDER ;
	offset_y -= BORDER ;
	glRasterPos2f(offset_x, offset_y);

	for (int i = 0; i < (int) s.length(); i++){
		glutBitmapCharacter(FONT, s[i]);
	}
	start_y++ ;
}

int M_OnScreenDisplay::getDesiredWinWidth(){
	return (max_row_length) * FONT_WIDTH + 2 * BORDER ;
}

int M_OnScreenDisplay::getDesiredWinHeight(){
   return (FONT_HEIGHT + ROW_SPACING) * row_count + 2 * BORDER;
}

void M_OnScreenDisplay::setHasBorder(bool _hasBorder){
  hasBorder = _hasBorder ;
}

void M_OnScreenDisplay::setBackgroundColor(float _r, float _g, float _b, float _a){
  winBackgroundColor[0] = _r;
  winBackgroundColor[1] = _g;
  winBackgroundColor[2] = _b;
  winBackgroundColor[3] = _a;
}
