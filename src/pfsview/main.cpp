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
 * $Id: main.cpp,v 1.19 2014/10/13 09:27:38 rafm Exp $
 */

#include <config.h>

#include "main.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include <qcombobox.h>
#include <qlabel.h>
#include <qtoolbutton.h>
#include <qstatusbar.h>
#include <qtooltip.h>
#include <qcursor.h>
#include <qclipboard.h>
#include <QApplication>
#include <QFileDialog>
#include <QImageWriter>
#include <QToolBar>
#include <pfs.h>
#include <QShortcut>


#include "pfsview_widget.h"
#include "luminancerange_widget.h"

#define PROGNAME "pfsview"

//QApplication *qApp;

PFSViewMainWin::PFSViewMainWin(  float window_min, float window_max ):
  QMainWindow( 0 )
{
  currentFrame = frameList.end();
  
  QScrollArea *pfsViewArea = new PFSViewWidgetArea( this );
  
  pfsView = (PFSViewWidget*)pfsViewArea->widget();
  
  setCentralWidget( pfsViewArea );

  setWindowIcon( QIcon( ":icons/appicon.png" ) );

  QAction *nextFrameAct = new QAction( tr( "&Next frame" ), this );
  nextFrameAct->setStatusTip( tr( "Load next frame" ) );
  nextFrameAct->setShortcut( Qt::Key_PageDown );
  connect( nextFrameAct, SIGNAL(triggered()), this, SLOT(gotoNextFrame()) );

  QAction *previousFrameAct = new QAction( tr( "&Previous frame" ), this );
  previousFrameAct->setStatusTip( tr( "Load previous frame" ) );
  previousFrameAct->setShortcut( Qt::Key_PageUp );
  connect( previousFrameAct, SIGNAL(triggered()), this, SLOT(gotoPreviousFrame()) );
  
  QToolBar *toolBar = addToolBar( tr( "Navigation" ) );
//  toolBar->setHorizontalStretchable( true );

  QToolButton *previousFrameBt = new QToolButton( toolBar );
  previousFrameBt->setArrowType( Qt::LeftArrow );
  previousFrameBt->setMinimumWidth( 15 );
  connect( previousFrameBt, SIGNAL(clicked()), this, SLOT(gotoPreviousFrame()) );
  previousFrameBt->setToolTip( "Goto previous frame" );
  toolBar->addWidget( previousFrameBt );
  
  QToolButton *nextFrameBt = new QToolButton( toolBar );
  nextFrameBt->setArrowType( Qt::RightArrow );
  nextFrameBt->setMinimumWidth( 15 );
  connect( nextFrameBt, SIGNAL(clicked()), this, SLOT(gotoNextFrame()) );
  nextFrameBt->setToolTip( "Goto next frame" );
  toolBar->addWidget( nextFrameBt );

  QLabel *channelSelLabel = new QLabel( "&Channel", toolBar );
  channelSelection = new QComboBox( toolBar );
  channelSelLabel->setBuddy( channelSelection );
  connect( channelSelection, SIGNAL( activated( int ) ),
    this, SLOT( setChannelSelection(int) ) );
  toolBar->addWidget( channelSelLabel );
  toolBar->addWidget( channelSelection );
  
  toolBar->addSeparator();

  QLabel *mappingMethodLabel = new QLabel( "&Mapping", toolBar );
  mappingMethodLabel->setAlignment(  Qt::AlignRight | Qt::AlignVCenter ); // | 
//				    Qt::TextExpandTabs | Qt::TextShowMnemonic );
  mappingMethodCB = new QComboBox( toolBar );
  mappingMethodLabel->setBuddy( mappingMethodCB );
  mappingMethodCB->addItem( "Linear" );
  mappingMethodCB->addItem( "Gamma 1.4" );
  mappingMethodCB->addItem( "Gamma 1.8" );
  mappingMethodCB->addItem( "Gamma 2.2" );
  mappingMethodCB->addItem( "Gamma 2.6" );
  mappingMethodCB->addItem( "Logarithmic" );
  mappingMethodCB->setCurrentIndex( 3 );
  connect( mappingMethodCB, SIGNAL( activated( int ) ),
    this, SLOT( setLumMappingMethod(int) ) );
  toolBar->addWidget( mappingMethodLabel );
  toolBar->addWidget( mappingMethodCB );
  
//  addToolBar( Qt::BottomToolBarArea, toolBar );  
  
  QToolBar *toolBarLR = addToolBar( tr( "Histogram" ) );
  lumRange = new LuminanceRangeWidget( toolBarLR );
  connect( lumRange, SIGNAL( updateRangeWindow() ), this,
    SLOT( updateRangeWindow() ) );
  toolBarLR->addWidget( lumRange );
//  addToolBar( toolBar );

  
  pointerPosAndVal = new QLabel( statusBar() );
  statusBar()->addWidget( pointerPosAndVal );
//  QFont fixedFont = QFont::defaultFont();
//  fixedFont.setFixedPitch( true );
//  pointerPosAndVal->setFont( fixedFont );
  zoomValue = new QLabel( statusBar() );
  statusBar()->addWidget( zoomValue );
  exposureValue = new QLabel( statusBar() );
  statusBar()->addWidget( exposureValue );

  connect( pfsView, SIGNAL(updatePointerValue()),
             this, SLOT(updatePointerValue()) );



  QMenu *frameMenu = menuBar()->addMenu( tr( "&Frame" ) );
  frameMenu->addAction( nextFrameAct );
  frameMenu->addAction( previousFrameAct );
  frameMenu->addSeparator();
  frameMenu->addAction( "&Save image...", this, SLOT(saveImage()), QKeySequence::Save ); 
  frameMenu->addAction( "&Copy image to clipboard", this, SLOT(copyImage()), QKeySequence::Copy ); 
  frameMenu->addSeparator();
  frameMenu->addAction( "&Quit", qApp, SLOT(quit()), Qt::Key_Q ); //QKeySequence::Quit
  QShortcut *shortcut = new QShortcut( QKeySequence::Close, this );
  connect( shortcut, SIGNAL(activated()), qApp, SLOT(quit()) );
  
  
  QAction *act;
  QMenu *viewMenu = menuBar()->addMenu( tr( "&View" ) );

  act = viewMenu->addAction( "&Zoom in", pfsView, SLOT(zoomIn()), Qt::Key_Period ); // QKeySequence::ZoomIn -- not doing it -- silly binding under Linux
  connect( act, SIGNAL(triggered()), this, SLOT(updateZoomValue()) );
  act = viewMenu->addAction( "Zoom &out", pfsView, SLOT(zoomOut()), Qt::Key_Comma ); 
  connect( act, SIGNAL(triggered()), this, SLOT(updateZoomValue()) );
  act = viewMenu->addAction( "Zoom &1:1", pfsView, SLOT(zoomOriginal()), Qt::Key_N ); 
  connect( act, SIGNAL(triggered()), this, SLOT(updateZoomValue()) );
  viewMenu->addAction( "&Fit window to content", this, SLOT(updateViewSize()), Qt::Key_C ); 

  viewMenu->addSeparator();  

  QMenu *infnanMenu = viewMenu->addMenu( "NaN and &Inf values" );
  QActionGroup *infnanActGrp = new QActionGroup( this );
  infnanActGrp->setExclusive( true );
  QAction *infnanHideAct = new QAction( tr( "&Hide" ), this );
  infnanHideAct->setCheckable(true);
  infnanHideAct->setData(0);
  infnanActGrp->addAction( infnanHideAct );
  infnanMenu->addAction( infnanHideAct );
  QAction *infnanMarkAct = new QAction( tr( "Mark with &red color" ), this );
  infnanMarkAct->setCheckable(true);
  infnanMarkAct->setData(1);
  infnanActGrp->addAction( infnanMarkAct );
  infnanMenu->addAction( infnanMarkAct );
  infnanMarkAct->setChecked( true );
  connect( infnanActGrp, SIGNAL(triggered(QAction*)), pfsView, SLOT(setInfNaNTreatment(QAction*)) );

  QMenu *colorClipMenu = viewMenu->addMenu( "&Color clipping" );
  QActionGroup *colorClipActGrp = new QActionGroup( this );
  colorClipActGrp->setExclusive( true );
  QAction *colorClipSimpleAct = new QAction( tr( "&Simple clipping" ), this );
  colorClipSimpleAct->setCheckable(true);
  colorClipSimpleAct->setData(CLIP_SIMPLE);
  colorClipSimpleAct->setShortcut( Qt::CTRL + Qt::Key_H );
  colorClipActGrp->addAction( colorClipSimpleAct );
  colorClipMenu->addAction( colorClipSimpleAct );
  QAction *colorClipCodedAct = new QAction( tr( "&Color-coded clipping" ), this );
  colorClipCodedAct->setCheckable(true);
  colorClipCodedAct->setShortcut( Qt::CTRL + Qt::Key_J );
  colorClipCodedAct->setData(CLIP_COLORCODED);
  colorClipActGrp->addAction( colorClipCodedAct );
  colorClipMenu->addAction( colorClipCodedAct );
  QAction *colorClipBriHueAct = new QAction( tr( "&Keep brightness and hue" ), this );
  colorClipBriHueAct->setCheckable(true);
  colorClipBriHueAct->setShortcut( Qt::CTRL + Qt::Key_K );
  colorClipBriHueAct->setData(CLIP_KEEP_BRI_HUE);
  colorClipActGrp->addAction( colorClipBriHueAct );
  colorClipMenu->addAction( colorClipBriHueAct );
  colorClipSimpleAct->setChecked( true );
  connect( colorClipActGrp, SIGNAL(triggered(QAction*)), pfsView, SLOT(setRGBClippingMethod(QAction*)) );

  QMenu *negativeMenu = viewMenu->addMenu( "&Negative values" );
  QActionGroup *negativeActGrp = new QActionGroup( this );
  negativeActGrp->setExclusive( true );
  act = new QAction( tr( "&Black" ), this );
  act->setCheckable(true);
  act->setData(NEGATIVE_BLACK);
  act->setShortcut( Qt::ALT + Qt::Key_B );
  negativeActGrp->addAction( act );
  negativeMenu->addAction( act );
  act->setChecked( true );
  act = new QAction( tr( "Mark with &red color" ), this );
  act->setCheckable(true);
  act->setData(NEGATIVE_MARK_AS_RED);
  act->setShortcut( Qt::ALT + Qt::Key_R );
  negativeActGrp->addAction( act );
  negativeMenu->addAction( act );
  act = new QAction( tr( "Use &green color scale" ), this );
  act->setCheckable(true);
  act->setData(NEGATIVE_GREEN_SCALE);
  act->setShortcut( Qt::ALT + Qt::Key_G );
  negativeActGrp->addAction( act );
  negativeMenu->addAction( act );
  act = new QAction( tr( "Use &absolute values" ), this );
  act->setCheckable(true);
  act->setData(NEGATIVE_ABSOLUTE);
  act->setShortcut( Qt::ALT + Qt::Key_A );
  negativeActGrp->addAction( act );
  negativeMenu->addAction( act );
  connect( negativeActGrp, SIGNAL(triggered(QAction*)), pfsView, SLOT(setNegativeTreatment(QAction*)) );
  
  viewMenu->addSeparator();
  
  QMenu *colorCoordMenu = viewMenu->addMenu( "Color coo&rdinates" );
  QActionGroup *colorCoordActGrp = new QActionGroup( this );
  colorCoordActGrp->setExclusive( true );
  act = new QAction( tr( "&RGB" ), this );
  act->setCheckable(true);
  act->setData(CC_RGB);
  act->setShortcut( Qt::SHIFT + Qt::ALT + Qt::Key_R );
  colorCoordActGrp->addAction( act );
  colorCoordMenu->addAction( act );
  act->setChecked( true );
  act = new QAction( tr( "&XYZ" ), this );
  act->setCheckable(true);
  act->setData(CC_XYZ);
  act->setShortcut( Qt::SHIFT + Qt::ALT + Qt::Key_X );
  colorCoordActGrp->addAction( act );
  colorCoordMenu->addAction( act );
  act = new QAction( tr( "Y&u'v'" ), this );
  act->setCheckable(true);
  act->setData(CC_Yupvp);
  act->setShortcut( Qt::SHIFT + Qt::ALT + Qt::Key_U );
  colorCoordActGrp->addAction( act );
  colorCoordMenu->addAction( act );
  act = new QAction( tr( "Yx&y" ), this );
  act->setCheckable(true);
  act->setData(CC_Yxy);
  act->setShortcut( Qt::SHIFT + Qt::ALT + Qt::Key_Y );
  colorCoordActGrp->addAction( act );
  colorCoordMenu->addAction( act );
  connect( colorCoordActGrp, SIGNAL(triggered(QAction*)), this, SLOT(setColorCoord(QAction*)) );
  

  QMenu *mappingMenu = menuBar()->addMenu( tr( "&Tone mapping" ) );
  
  
  mappingMenu->addAction( "Increase exposure", lumRange, 
			SLOT(increaseExposure()), Qt::Key_Minus ); 
  mappingMenu->addAction( "Decrease exposure", lumRange, 
			SLOT(decreaseExposure()), Qt::Key_Equal );
  mappingMenu->addAction( "Extend dynamic range", lumRange, 
			SLOT(extendRange()), Qt::Key_BracketRight ); 
  mappingMenu->addAction( "Shrink dynamic range", lumRange, 
			SLOT(shrinkRange()), Qt::Key_BracketLeft );
  mappingMenu->addAction( "Fit to dynamic range", lumRange, 
			SLOT(fitToDynamicRange()), Qt::Key_Backslash );
  mappingMenu->addAction( "Low dynamic range", lumRange, 
			SLOT(lowDynamicRange()), Qt::ALT + Qt::Key_L );


  QMenu *mapfuncMenu = mappingMenu->addMenu( "&Mapping function" );
  QActionGroup *mapfuncActGrp = new QActionGroup( this );
  mapfuncActGrp->setExclusive( true );
  mappingAct[0] = act = new QAction( tr( "&Linear" ), this );
  act->setCheckable(true);
  act->setData(0);
  act->setShortcut( Qt::Key_L );
  mapfuncActGrp->addAction( act );
  mapfuncMenu->addAction( act );
  mappingAct[1] = act = new QAction( tr( "Gamma 1.&4" ), this );
  act->setCheckable(true);
  act->setData(1);
  act->setShortcut( Qt::Key_1 );
  mapfuncActGrp->addAction( act );
  mapfuncMenu->addAction( act );
  mappingAct[2] = act = new QAction( tr( "Gamma 1.&8" ), this );
  act->setCheckable(true);
  act->setData(2);
  act->setShortcut( Qt::Key_2 );
  mapfuncActGrp->addAction( act );
  mapfuncMenu->addAction( act );
  mappingAct[3] = act = new QAction( tr( "Gamma 2.&2" ), this );
  act->setCheckable(true);
  act->setData(3);
  act->setChecked( true );
  act->setShortcut( Qt::Key_3 );
  mapfuncActGrp->addAction( act );
  mapfuncMenu->addAction( act );
  mappingAct[4] = act = new QAction( tr( "Gamma 2.&6" ), this );
  act->setCheckable(true);
  act->setData(4);
  act->setShortcut( Qt::Key_4 );
  mapfuncActGrp->addAction( act );
  mapfuncMenu->addAction( act );
  mappingAct[5] = act = new QAction( tr( "L&ogarithmic" ), this );
  act->setCheckable(true);
  act->setData(5);
  act->setShortcut( Qt::Key_O );
  mapfuncActGrp->addAction( act );
  mapfuncMenu->addAction( act );
  connect( mapfuncActGrp, SIGNAL(triggered(QAction*)), this, SLOT(setLumMappingMethod(QAction*)) );

  
  QMenu *helpMenu = menuBar()->addMenu( tr( "&Help" ) );
  helpMenu->addAction( "&About", this, SLOT(showAboutdialog()) );
  
  colorCoord = CC_RGB;
  
  //Window should not be larger than desktop
  // TODO: how to find desktop size - gnome taksbars
//  setMaximumSize( QApplication::desktop()->width(), QApplication::desktop()->height() );

  try {
    
    if( !readNextFrame() ) 
      throw PFSViewException();
    
    if( window_min < window_max )
      lumRange->setRangeWindowMinMax( window_min, window_max );  
    
  } catch( pfs::Exception ex ) {
    QMessageBox::critical( this, "pfsview error", ex.getMessage() );
    throw PFSViewException();
  }

}



/*
void PFSViewMainWin::changeClippingMethod( int clippingMethod )
{
  for( int i = 0; i < CLIP_COUNT; i++ )
    viewMenu->setItemChecked( clippingMethodMI[i], clippingMethod == i );

  statusBar()->message( viewMenu->text(clippingMethodMI[clippingMethod]), 1000 );  
}

void PFSViewMainWin::setInfNaNTreatment( int method )
{
  infnanMenu->setItemChecked( infNaNMI[0], method == 0 );
  infnanMenu->setItemChecked( infNaNMI[1], method == 1 );
}

void PFSViewMainWin::setNegativeTreatment( int method )
{
  for( int i = 0; i < NEGATIVE_COUNT; i++ )
    negativeMenu->setItemChecked( negativeMI[i], method == i );
}
*/

void PFSViewMainWin::setLumMappingMethod( int method )
{
  mappingMethodCB->setCurrentIndex( method );
  pfsView->setLumMappingMethod( method );  
  mappingAct[method]->setChecked( true );  
}

void PFSViewMainWin::setLumMappingMethod( QAction *action )
{
  int method = action->data().toUInt();
  mappingMethodCB->setCurrentIndex( method );
  pfsView->setLumMappingMethod( method );
}

struct ColorSpaceLabel
{
  const char *comboBoxLabel;
  const char *c1, *c2, *c3;
};

static const ColorSpaceLabel scLabels[] = {
  { "Color XYZ", "X", "Y", "Z" }    
};

int scLabelsCount = sizeof( scLabels ) / sizeof( ColorSpaceLabel );

void PFSViewMainWin::updatePointerValue()
{
  const PointerValue &pv = pfsView->getPointerValue();

  if( pv.valid ) {
    char pointerValueStr[256];
    const PointerValue &pv = pfsView->getPointerValue();
    sprintf( pointerValueStr, "(x,y)=(%4d,%4d) ", pv.x, pv.y );
    QString label( pointerValueStr );

    int channelSel = channelSelection->currentIndex();
    if( pv.valuesUsed == 3 ) { // Color
      assert( channelSel < scLabelsCount );

      pfs::Array2DImpl X(1,1), Y(1,1), Z(1,1);
      X(0) = pv.value[0];
      Y(0) = pv.value[1];
      Z(0) = pv.value[2];

      const char *l1, *l2, *l3;
      switch( colorCoord ) {
      case CC_RGB:
        l1 = "R"; l2 = "G"; l3 = "B";
        pfs::transformColorSpace( pfs::CS_XYZ, &X, &Y, &Z, pfs::CS_RGB, &X, &Y, &Z );
        break;
      case CC_XYZ:
        l1 = "X"; l2 = "Y"; l3 = "Z";
        break;
      case CC_Yupvp:
        pfs::transformColorSpace( pfs::CS_XYZ, &X, &Y, &Z, pfs::CS_YUV, &X, &Y, &Z );
        l1 = "Y"; l2 = "u'"; l3 = "v'";
        break;
      case CC_Yxy:
        pfs::transformColorSpace( pfs::CS_XYZ, &X, &Y, &Z, pfs::CS_Yxy, &X, &Y, &Z );
        l1 = "Y"; l2 = "x"; l3 = "y";
        break;
      case CC_COUNT:
	assert( 0 );
	break;
      };
      
      sprintf( pointerValueStr, "%s=%07.4g %s=%07.4g %s=%07.4g",
        l1, X(0),
        l2, Y(0),
        l3, Z(0)
        );
      label += pointerValueStr;
      lumRange->showValuePointer( log10(pv.value[1]) );
      
    } else {                    // Single channel
      const QString &name = channelSelection->itemText( channelSel );
      sprintf( pointerValueStr, "%s=%07.4g", (const char*)name.toAscii(), pv.value[0] );
      label += pointerValueStr;
      lumRange->showValuePointer( log10(pv.value[0]) );
    }    
    
    pointerPosAndVal->setText( label );
  } else {
    pointerPosAndVal->setText( "(?,?)" );
    lumRange->hideValuePointer();
  }
  
  
}


void PFSViewMainWin::setColorCoord( QAction *action )
{
  colorCoord = (ColorCoord)action->data().toUInt();
  updatePointerValue();
}



void PFSViewMainWin::updateZoomValue()
{
  char strBuf[20];
  sprintf( strBuf, "%d%%", (int)(pfsView->getZoom()*100) );
  zoomValue->setText( strBuf );
}

void PFSViewMainWin::updateViewSize()
{
//  QSize sz = sizeHint();
//  printf( "main window: %d %d\n", sz.width(), sz.height() );

  centralWidget()->updateGeometry();
  resize( sizeHint() );   
}

void PFSViewMainWin::updateRangeWindow()
{
  pfsView->setRangeWindow( pow( 10, lumRange->getRangeWindowMin() ), pow( 10, lumRange->getRangeWindowMax() ) );

  char ev_str[100];
  sprintf( ev_str, "EV = %+.2g f-stops   %+.2g D   %.4g",  -lumRange->getRangeWindowMax()*log(2)/log(10),
    -lumRange->getRangeWindowMax(), 1/pow( 10, lumRange->getRangeWindowMax() ) );
  exposureValue->setText( ev_str );
}

void PFSViewMainWin::updateChannelSelection()
{
  channelSelection->clear();

  const char *visibleChannel = pfsView->getVisibleChannel();
  
  if( pfsView->hasColorChannels() ) {  
    channelSelection->addItem( "Color XYZ" );
    if( !strcmp( visibleChannel, "Color" ) )
      channelSelection->setCurrentIndex( 0 );
  }  

  /*  channelSelection->insertItem( "Color RGB" );
  channelSelection->insertItem( "Color XYZ" );
  channelSelection->insertItem( "Color L*ab" );
  channelSelection->insertItem( "Luminance Y" );*/
  
  QList<const char*> channelNames = pfsView->getChannels();
  for( int c = 0; c < channelNames.size(); c++ ) {
    channelSelection->addItem( channelNames[c] );
    if( !strcmp( channelNames[c], visibleChannel ) )
      channelSelection->setCurrentIndex( channelSelection->count()-1 );
  }
  

}

void PFSViewMainWin::setChannelSelection( int selectedChannel )
{
  // If channel combo box has not been initialized yet
  if( selectedChannel >= channelSelection->count() ) 
    return;

  const QString &name = channelSelection->itemText( selectedChannel );
  
  if( pfsView->hasColorChannels() && selectedChannel < scLabelsCount ) {
    pfsView->setVisibleChannel( NULL ); // Color
  } else {
    pfsView->setVisibleChannel( (const char*)name.toAscii() );
  }

  updateMenuEnable( );
  lumRange->setHistogramImage(pfsView->getPrimaryChannel());

  updatePointerValue();

}

void PFSViewMainWin::updateMenuEnable( )
{
  bool color = pfsView->getVisibleChannel() == COLOR_CHANNELS;

//  negativeMenu->setItemEnabled( negativeMI[NEGATIVE_GREEN_SCALE], !color );
//  negativeMenu->setItemEnabled( negativeMI[NEGATIVE_ABSOLUTE], !color );  
}


void PFSViewMainWin::showAboutdialog()
{
   QMessageBox::information( this, "pfsview",
     "pfsview " PACKAGE_VERSION "\n"
     "Copyright (C) 2005-2011 Rafal Mantiuk\n\n"
     "See the manual page (man pfsview) and\n"
     "the web page (http://pfstools.sourceforge.net/)\n"
     "for more information"
     );
   
}

// ======================= Loading next frame ==========================

/**
 * GUI action -> goto next frame
 * handle error and eof of frame
 */
void PFSViewMainWin::gotoNextFrame()
{
  try {
    if( !readNextFrame() ) {
      statusBar()->showMessage( "No more frames", 1000 );
    }
  }
  catch( pfs::Exception ex )
    {
      // Display message and keep the old frame
      QMessageBox::critical( this, "pfsview error", ex.getMessage() );
      qApp->quit();
    }
}

void PFSViewMainWin::gotoPreviousFrame()
{
  currentFrame++;
  if( currentFrame == frameList.end() ) {
    currentFrame--;
    statusBar()->showMessage( "No more frames in buffer (buffer holds max 5 frames)", 1000 );
    return;
  }

  setFrame( *currentFrame );
}

void PFSViewMainWin::setFrame( pfs::Frame *frame )
{
  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

  
  pfsView->setFrame( frame );

  lumRange->setHistogramImage(pfsView->getPrimaryChannel());

  updateChannelSelection();
  updateZoomValue();
  
  const char *luminanceTag = frame->getTags()->getString( "LUMINANCE" );
  if( luminanceTag != NULL && !strcmp( luminanceTag, "DISPLAY" ) ) {
    setLumMappingMethod(0);
    lumRange->lowDynamicRange();
  }
  
  updateRangeWindow();

  // Set the window caption to the name of the frame, if it is given
  const char *fileName = frame->getTags()->getString( "FILE_NAME" );
  QString qLuminance( "" );
  if( luminanceTag!=NULL )
    qLuminance=QString(" (") + QString(luminanceTag) + QString(")");
  if( fileName == NULL )
    setWindowTitle( QString( "pfsview" ) );
  else {
    QString qFileName( fileName );
    if( qFileName.length() > 30 )
      setWindowTitle( QString( "pfsview: ... " ) + QString( fileName ).right( 30 ) + qLuminance);
    else
      setWindowTitle( QString( "pfsview: " ) + QString( fileName ) + qLuminance);
  }
  updateMenuEnable( );


  QApplication::restoreOverrideCursor();  
}

/**
 * Load next frame from the stream.
 * @return false if there are no more frames
 */
bool PFSViewMainWin::readNextFrame()
{
  if( currentFrame != frameList.begin() )
  {
    currentFrame--;
    setFrame( *currentFrame );
    return true;
  }
  
  pfs::DOMIO pfsCtx;
  pfs::Frame *newFrame;
  newFrame = pfsCtx.readFrame( stdin );
  if( newFrame == NULL ) return false; // No more frames
  
  if( frameList.size() == MAX_FRAMES_IN_MEMORY ) {
    // Remove one frame from the back
    pfsCtx.freeFrame( frameList.back() );
    frameList.pop_back();
  }
  frameList.push_front( newFrame );
  currentFrame = frameList.begin();
  
  setFrame( newFrame );  
  return true;
}

// ======================= Saving image ================================

void PFSViewMainWin::saveImage()
{ 
  QString fileName = QFileDialog::getSaveFileName( this, "Save current view as...", "./", "*.png; *.jpg" );
  QByteArray jpeg_format( "jpg" );
  QByteArray png_format( "png" );
  
  if ( !fileName.isNull() ) {                 // got a file name

    QList<QByteArray> file_formats = QImageWriter::supportedImageFormats();
    QList<QByteArray>::iterator it;
    QByteArray *format = NULL;
    for( it = file_formats.begin(); it != file_formats.end(); it++ ) {
      if( fileName.endsWith( (const char*)*it ) ) {
        format = &(*it);
        break;        
      }
    }
    if( format == NULL ) {
      int sel = QMessageBox::question( this, "Select image format", "The file name is missing extension or the extension is not recongnized as an image format. Choose the image format.", "Cancel", "JPEG image", "PNG image" );
      if( sel == 1 ) {
        format = &jpeg_format;
        fileName = fileName + ".jpg";        
      } else if( sel == 2 ) {
        format = &png_format;
        fileName = fileName + ".png";        
      }
    }    

    if( format != NULL ) {
      
      QImage *image = pfsView->getDisplayedImage();
      
      if( !image->save( fileName, format->data(), -1 ) )
      {
        QMessageBox::warning( this, "Saving image problem", "Cannot save image " + fileName );
      }
      else
        statusBar()->showMessage( fileName + QString( "' saved as " ) + QString( format->data() ) + QString( " image" ), 4000 ); 
    }
    
  }
}

void PFSViewMainWin::copyImage()
{
  QClipboard *cb = QApplication::clipboard();

  const QImage *image = pfsView->getDisplayedImage();
  cb->setImage( *image, QClipboard::Clipboard );
  statusBar()->showMessage( "Image copied to clipboard", 2000 );
}


// ======================= main and command line =======================

void printUsage()
{
    fprintf( stderr,
      "Usage: " PROGNAME " [options]\n"
      "\n"
      "Displays high-dynamic range image in pfs format. The image is read from the "
      "standard output.\n"
      "\n"
      "options:\n"
      "\t--help                        : prints this message\n"
      "\t--window_min log_lum          : lower bound of hdr display window, given as "
      "a log10 of luminance.\n"
      "\t--window_min log_lum          : the same as above, but the upper bound\n"
      );
}

static void errorCheck( bool condition, const char *string )
{
    if( !condition ) {
	fprintf( stderr, PROGNAME " error: %s\n", string );
	abort();
    }
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    //    qApp = &app;

    float window_min = 0, window_max = 0;

    for( int i=1 ; i<argc ; i++ )
    {
      if( !strcmp( argv[i], "--help") ) {
        printUsage();
        return 0;
      }
      else if( !strcmp( argv[i], "--window_min") ) {
        i++;
        errorCheck( i < argc, "expected parameter after --window_min" );
        char *checkPtr;
        window_min = (float)strtod( argv[i], &checkPtr );
        errorCheck( checkPtr != NULL && checkPtr[0] == 0 &&
          window_min < 100 && window_min > -100,
          "--window_min expects floating point value between -100 and 100" );  
      }
      else if( !strcmp( argv[i], "--window_max") ) {
        i++;
        errorCheck( i < argc, "expected parameter after --window_max" );
        char *checkPtr;
        window_max = (float)strtod( argv[i], &checkPtr );
        errorCheck( checkPtr != NULL && checkPtr[0] == 0 &&
          window_max < 100 && window_max > -100,
          "--window_max expects floating point value between -100 and 100" );
      } else {
        fprintf( stderr, PROGNAME " error: not recognized parameter '%s'\n", argv[i] );
        printUsage();
        return -1;
      }
    }

    errorCheck( window_min <= window_max, "window_min must be less than window_max" );


    try {
      
      PFSViewMainWin pfsViewMW( window_min, window_max );
//      app.setMainWidget(&pfsViewMW);    
      pfsViewMW.show();
      pfsViewMW.updateViewSize();
      
      return app.exec();
      
    }
    catch( PFSViewException ex )
    {
      QMessageBox::critical( NULL, "pfsview error", "Can not open the first PFS frame. Quitting." );
      return -1;
    }
    
    
}

