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

#ifndef M_ON_SCREEN_DISPLAY_H
#define M_ON_SCREEN_DISPLAY_H

#include <string>

#include "module.h"


class M_OnScreenDisplay : public Module {
	
				
private:
	float start_y ;
	int max_row_length ;
	int row_count ;
	bool hasBorder  ;

	std::string* text  ;


	public:
	M_OnScreenDisplay(std::string* s, int row_count);
	~M_OnScreenDisplay();
		
	void redraw(void);
	void println(std::string s) ;
	int getDesiredWinHeight() ;
	int getDesiredWinWidth() ;

	void setHasBorder(bool has_border) ;
	void setBackgroundColor(float _r, float _g, float _b, float _a) ;
};

#endif
