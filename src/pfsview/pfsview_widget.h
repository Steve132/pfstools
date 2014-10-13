#ifndef PFSVIEW_WIDGET_H
#define PFSVIEW_WIDGET_H

/**
 * @brief 
 * 
 * This file is a part of PFSTOOLS package.
 * ---------------------------------------------------------------------- 
 * Copyright (C) 2003,2004 Rafal Mantiuk and Grzegorz Krawczyk
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
 * @author Rafal Mantiuk, <mantiuk@mpi-sb.mpg.de>
 *
 * $Id: pfsview_widget.h,v 1.10 2011/04/30 14:01:17 rafm Exp $
 */

#include <qpainter.h>
#include <qimage.h>
#include <QEvent>
#include <QMouseEvent>
#include <QAction>
#include <QScrollArea>

#include <array2d.h>

namespace pfs {
  class DOMIO;
  class Frame;
}

enum RGBClippingMethod {
  CLIP_SIMPLE = 0,
  CLIP_COLORCODED,
  CLIP_KEEP_BRI_HUE,
  CLIP_COUNT
};

enum LumMappingMethod {
  MAP_LINEAR,
  MAP_GAMMA1_4,
  MAP_GAMMA1_8,
  MAP_GAMMA2_2,
  MAP_GAMMA2_6,
  MAP_LOGARITHMIC
};

enum InfNaNTreatment {
  INFNAN_HIDE=0,
  INFNAN_MARK_AS_RED
};

enum NegativeTreatment {
  NEGATIVE_BLACK=0,
  NEGATIVE_MARK_AS_RED,
  NEGATIVE_GREEN_SCALE,
  NEGATIVE_ABSOLUTE,
  NEGATIVE_COUNT
};


struct PointerValue
{
  int x, y;
  float value[3];
  int valuesUsed;
  bool valid;
};


class PFSViewWidgetArea : public QScrollArea {
  Q_OBJECT
public:
  PFSViewWidgetArea( QWidget *parent=0 );

  QSize sizeHint() const;

};


class PFSViewWidget : public QWidget {
  Q_OBJECT
public:
  PFSViewWidget( QWidget *parent=0 );
  ~PFSViewWidget();

public slots:
  void zoomIn();
  void zoomOut();
  void zoomOriginal();
    
  void setRGBClippingMethod( QAction *action );
  void setInfNaNTreatment( QAction *action );
  void setNegativeTreatment( QAction *action );
  void setLumMappingMethod( int method );
  
signals:
  void updatePointerValue();
    
protected:
//  void paintEvent( QPaintEvent * );


  void updateZoom();
  void updateMapping();
  void postUpdateMapping();

  void mouseMoveEvent( QMouseEvent *mouseEvent );  
  void setPointer( int x = -1, int y = -1 );
  void leaveEvent( QEvent * );
  
  void paintEvent( QPaintEvent *event );  
  
private:
  pfs::Frame *pfsFrame;
  const char *visibleChannel;
  QByteArray visibleChannelName;
  
  bool colorChannelsPresent;

  bool updateMappingRequested;

  QImage *image;

  float minValue;
  float maxValue;

  RGBClippingMethod clippingMethod;
  InfNaNTreatment infNaNTreatment;
  NegativeTreatment negativeTreatment;
  LumMappingMethod mappingMethod;

  float zoom;

  pfs::Array2D *workArea[3];

  PointerValue pointerValue;

  QRegion selection;

public:
  QSize sizeHint() const;

  const PointerValue &getPointerValue();
  void setRangeWindow( float min, float max );

  const pfs::Array2D *getPrimaryChannel();  

  const QList<const char*> getChannels();

  void setVisibleChannel( const char *channelName );
  const char *getVisibleChannel();
  
  bool hasColorChannels( pfs::Frame *frame = NULL );

  float getZoom() const
    {
      return zoom;
    }

  void setFrame( pfs::Frame *frame );

  QImage *getDisplayedImage() 
    {
      assert( image != NULL );
      return image;
    }
  
  
};


class PFSViewException
{
};

extern const char* COLOR_CHANNELS;

#endif
