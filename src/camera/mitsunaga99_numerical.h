/**
 * @brief Algorithm for automatic HDR images capture.
 *
 * 
 * This file is a part of PFS CALIBRATION package.
 * ---------------------------------------------------------------------- 
 * Copyright (C) 2006 Radoslaw Mantiuk
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
 * @author Radoslaw Mantiuk, <radoslaw.mantiuk@gmail.com>
 *
 */



#ifndef _mitsunaga99_numerical_h_
#define _mitsunaga99_numerical_h_

#define NR_LINEAR_EQUATIONS_MAX 20

class MitsunagaNumerical {

public:
	static int linearEquationsSystem( int n, float a[][NR_LINEAR_EQUATIONS_MAX], float b[]);

};

#endif
