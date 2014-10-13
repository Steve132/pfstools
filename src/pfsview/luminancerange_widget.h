#ifndef LUMINANCERANGE_WIDGET_H
#define LUMINANCERANGE_WIDGET_H

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
 * $Id: luminancerange_widget.h,v 1.4 2013/12/21 19:42:29 rafm Exp $
 */

#include <QMouseEvent>
#include <QPaintEvent>
#include <QFrame>

class Histogram;

namespace pfs 
{
  class Array2D;
}

class LuminanceRangeWidget : public QFrame {
  Q_OBJECT
public:
  LuminanceRangeWidget( QWidget *parent=0 );
  ~LuminanceRangeWidget();

  QSize sizeHint () const;

protected:
  void paintEvent( QPaintEvent * );
  void mouseMoveEvent( QMouseEvent * );
  void mousePressEvent( QMouseEvent * me );
  void mouseReleaseEvent(QMouseEvent *);

  float draggedMin();
  float draggedMax();
  
signals:
  void updateRangeWindow();
public slots:
  void decreaseExposure();
  void increaseExposure();
  void extendRange();
  void shrinkRange();
  void fitToDynamicRange();
  void lowDynamicRange();
  
private:
  float minValue;
  float maxValue;

  float windowMin;
  float windowMax;
  

  static const int DRAGNOTSTARTED = -1;
  int mouseDragStart;
  float dragShift;
  enum DragMode 
    {
      DRAG_MIN, DRAG_MAX, DRAG_MINMAX, DRAG_NO
    };
  DragMode dragMode;
  
  float valuePointer;

  Histogram *histogram;
  const pfs::Array2D *histogramImage;

  bool showVP;

  QRect getPaintRect() const;
  
public:
  float getRangeWindowMin() const
    {
      return windowMin;
    }
  float getRangeWindowMax() const
    {
      return windowMax;
    }

  void setRangeWindowMinMax( float min, float max );
  
  void setHistogramImage( const pfs::Array2D *image );

  void showValuePointer( float value );
  void hideValuePointer();
  
  
};


#endif
