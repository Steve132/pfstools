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


#ifndef M_STATUS_H
#define M_STATUS_H

#include "module.h"


class SC_Status : public Module {

private:
	int    positionX ;
	int    positionY ;
	int    valueR, valueG, valueB ;

	int    rawPositionX ;
	int    rawPositionY ;
	float  rawValueX, rawValueY, rawValueZ ;

	const char* mappingMode;
	const char* navigationMode;
	const char* channel;
	float       lumMax;
	int         frameNo;
	float       zoom_scale ;

public:
	SC_Status();
	~SC_Status();

	// ************************************
	// inherited methods
	// ************************************
	void redraw(void);
	// ************************************

	void setPixelData( int x, int y, int r, int g, int b);
	void setRawData(int x, int y, float X, float Y, float Z);
	void setMaxFreq(float _max_frequency);
	void setMapping(const char* _mappingMode);
	void setNavMode(const char* _navigationMode) ;
	void setFrameNo(int _frameNo);
	void setChannel(const char* _channel) ;
	void setZoomScale(float zoom_scale) ;
};


#endif
