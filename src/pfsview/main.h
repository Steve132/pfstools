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
 * $Id: main.h,v 1.11 2011/03/21 16:09:16 rafm Exp $
 */

#ifndef MAIN_H
#define MAIN_H

#include <qapplication.h>
#include <qmessagebox.h>
#include <qlayout.h>
#include <qmenubar.h>
#include <qsplitter.h>
#include <QLabel>
#include <QMainWindow>

#include <list>

#include "pfsview_widget.h"

#define MAX_FRAMES_IN_MEMORY 5

class LuminanceRangeWidget;
class QComboBox;

//class pfs::Frame;


enum ColorCoord { CC_RGB = 0, CC_XYZ, CC_Yupvp, CC_Yxy, CC_COUNT
};

class PFSViewMainWin : public QMainWindow {
  Q_OBJECT
public:
    PFSViewMainWin( float window_min, float window_max );
    
public slots:
//    void changeClippingMethod( int clippingMethod );
    void updatePointerValue();
    void updateRangeWindow();
    void updateChannelSelection();
    void setChannelSelection( int selectedChannel );
    void updateZoomValue();
    void gotoNextFrame();
    void gotoPreviousFrame();
    void saveImage();
    void copyImage();
    void setLumMappingMethod( int method );
    void setLumMappingMethod( QAction *action );
    void setColorCoord( QAction *action );
    void showAboutdialog();
    void updateViewSize();    
private:
    void setFrame( pfs::Frame *frame );

    QAction *mappingAct[6];    
    
    QLabel *pointerPosAndVal, *zoomValue, *exposureValue;
    PFSViewWidget *pfsView;
    LuminanceRangeWidget *lumRange;
    QComboBox *channelSelection;
    QComboBox *mappingMethodCB;
    ColorCoord colorCoord;

    bool readNextFrame();

    void updateMenuEnable( );
    
    std::list<pfs::Frame*> frameList;
    std::list<pfs::Frame*>::iterator currentFrame;
    
public:
    
    PFSViewWidget *getPFSViewWidget() 
      {
        return pfsView;
      }

};

#endif
