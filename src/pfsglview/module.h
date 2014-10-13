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

#include "glenv.h"

#ifndef MODULE_H
#define MODULE_H

// pixel width of the module border
#define X_BAR 20
#define Y_BAR 20

class Module  {

private:
	GLint param[4];

protected:
	int isVisible;
	int pos_x;
	int pos_y;
	int width;
	int height;
	GLfloat* winBackgroundColor;

	void drawBackground(void);
	int redrawStart(void);
	void redrawEnd(void);

public:

	Module();
	~Module();

	void setPosition( int _x, int _y);
	void setSize(int _width, int _height);

	int getWidth();
	int getHeight();
	void setWidth(int _width);
	void setHeight(int _height);

	void setVisible(bool _isVisible);
	bool getVisible(void);

	// not used anymore
	//		int processSelection(int xCoord, int yCoord);
};

#endif
