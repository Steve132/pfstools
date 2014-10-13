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

#ifndef M_HISTOGRAM_H
#define M_HISTOGRAM_H

#include "module.h"



class M_Histogram : public Module {




private:
  GLfloat*    backgroundColor;
  int         bins;
  const char* channel ;

  float*      binborder ;
  float       scale_amplifier ;

  int         rawPosX, rawPosY;
  float       rawX, rawY, rawZ;

public:
  int abort_update ;


private:
  void drawScale();
  void drawStatistic();
  void drawSlider();
  int findBin(float val, int start, int stop) ;

public:
  M_Histogram();
  ~M_Histogram();


  // ************************************
  // inherited methods
  // ************************************
  void setVisible(bool _isVisible) ;
  // ************************************



  // ************************************
  // frequency related functions
  // ************************************
private:
  float frequencyMax;
  int** frequencyValues;
public:
  float getHighFrequency() ;
  float getMaxFrequency() ;
  void resetFrequencyMax(void);
  void computeFrequency(int channel, const pfs::Array2D *image, void CB(void));
  // ************************************



  // ************************************
  // dimension realted functions
  // ************************************
public:
  int getInnerWidth( void);
  int getInnerHeight( void);
  int getBackgroundWidth(void);
  int getBackgroundHeight(void);
  // ************************************



  // ************************************
  // luminance related functions
  // ************************************
private:
  float lumMin, lumMax;
  float logLumMin, logLumMax;
public:
  float getLumMin();
  float getLumMax();

  float pos2lum( float pos);
  float lum2pos( float lum);

  void  computeLumRange( float& min, float& max);
  // ************************************



  // ************************************
  // subcomponent related functions
  // ************************************
public:
  enum SubComponent{
    NONE,
    LEFT_BAR,
    RIGHT_BAR,
    WHOLE_SLIDER,
    BOTTOM,
    BACK
  };
private:
  float sliderPosMin, sliderPosMax; // luminance values for start and end of slider
  SubComponent hoverState;
public:
  void         processPointerPosition(int xCoord, int yCoord,int pan);
  int          setSubComponentHoverState(SubComponent newState);
  SubComponent getSubComponentHoverState();

  void  setSliderPosMin( float pos);
  float getSliderPosMin(void);

  void  setSliderPosMax( float pos);
  float getSliderPosMax(void);

  void setSliderPosMinMax( float min, float max);
  // ************************************



  // ************************************
  // virtual size related functions
  // ************************************
private:
  float virtualWidth ;
  float virtualOffset ;

public:
  void  setVirtualWidth(float width) ;
  float getVirtualWidth();
  void  setVirtualOffset(float offset) ;
  float getVirtualOffset();
  // ************************************



  // ************************************
  // visualization related functions
  // ************************************
private:
  float gauss_data;
  float gauss_display;
  int is_scale_gaussed ;
  int is_scale_show_max_hue ;
public:
  // gaussian smoothing of the histogram function values
  int  get_is_scale_gaussed() const ;
  void set_is_scale_gaussed(int is_scale_gaussed) ;

  // gaussian smoothing of the intensity scale values
  int  get_is_scale_show_max_hue() const ;
  void set_is_scale_show_max_hue(int is_scale_show_max_hue) ;
  // ************************************


  void setChannel(const char* ch) ;
  void clearChannel(int channel, void CB(void));
  void drawHistogram();

  void redraw(void);
  int getChannelAsInt() ;
  float getPeak() ;
  void setRawData(int x, int y, float X, float Y, float Z);
  int getSidebarWidth(void) ;
};




#endif

