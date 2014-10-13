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

#ifndef PICTURE_IO_H
#define PICTURE_IO_H

#include <list>

#include "pfs.h"

#define MAX_FRAMES_IN_MEMORY 10

enum LumMappingMethod {
			MAP_LINEAR,
			MAP_GAMMA1_4,
			MAP_GAMMA1_8,
			MAP_GAMMA2_2,
			MAP_GAMMA2_6,
			MAP_LOGARITHMIC
			};
			
static const char* lumMappingName[] = { // must be consistent with LumMappingMethod
			"Linear",
			"Gamma 1.4",
			"Gamma 1.8",
			"Gamma 2.2",
			"Gamma 2.6",
			"Logarithmic"
			};


class PictureIO
{
	private:
		pfs::Frame *pfsFrame;
		int width;
		int height;
		const char *currentFileName;
		const char *visibleChannel;
		pfs::Array2D *chR, *chG, *chB;
		std::list<pfs::Frame*> frameList;
		std::list<pfs::Frame*>::iterator currentFrame; 
		int frameNo;		

		unsigned char* data;
		LumMappingMethod imageMappingMethod;
		float minLuminance, maxLuminance;
		int is_show_clipping ;

	public:	
		const char* CHANNEL_XYZ;
		const char* CHANNEL_X;
		const char* CHANNEL_Y;
		const char* CHANNEL_Z;	

		int abort_update ;

		PictureIO(LumMappingMethod mappingMethod, float minLuminance, float maxLumuminance);
		~PictureIO();
		void gotoNextFrame();
		void gotoPreviousFrame();

		void changeMapping( LumMappingMethod mappingMethod, float minLum, float maxLum);
		void changeMapping( float minLum, float maxLum);
		void changeMapping( LumMappingMethod mappingMethod);
		void changeMapping( void);
		
		void setMinLum( float val);
		void setMaxLum( float val);
		
		unsigned char* getImageData( void);
		
		void setMappingMethod( LumMappingMethod val);
		LumMappingMethod getMappingMethod( void);
		
		const pfs::Array2D* getPrimaryChannel();
		int loadPicture(const char* location, int width, int height);
		const char *getCurrentFileName();
		const char *getVisibleChannel();
		void setVisibleChannel(const char *channel);
		pfs::Frame *getFrame();
		void setFrame(pfs::Frame *pfsFrame, const char *channel);
		
		int save(void);
		
		int getWidth();
		int getHeight();		
		int getPixel(int ch, int c, int r);
		int getPixelR(int x, int y);
		int getPixelG(int x, int y);
		int getPixelB(int x, int y);
		float getLumMin(void);
		float getLumMax(void);
		float computeLumMin(void);
		float computeLumMax(void);
		int getFrameNo(void);
		float getDynamicRange(void);
		std::vector<const char*> getChannelNames();
		
		int getRawData( int x, int y, float& XX, float& YY, float& ZZ);
		void updateMapping( void CB(void));
		
		int get_is_show_clipping() const ;
		void set_is_show_clipping(int is_show_clipping) ;
		inline int fastMap( LumMappingMethod mappingMethod, float v, float minValue, float maxValue, float ranger) ;
		inline int fastCheck( LumMappingMethod mappingMethod, float v, float minValue, float maxValue, float ranger) ;

	private:
		bool hasColorChannels( pfs::Frame *frame );
		inline int binarySearchPixels(const float lum, const float *lumMap, const int lumSize);
		float getInverseMapping( LumMappingMethod mappingMethod, float v, float minValue, float maxValue );
		bool readNextFrame();
};

class PFSglViewException
{
};
#endif

