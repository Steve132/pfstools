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
 * $Id: pfsview_widget.cpp,v 1.26 2014/10/13 09:27:38 rafm Exp $
 */

#include <config.h>

#include <stdio.h>
#include <math.h>
#include <pfs.h>

#include <qmessagebox.h>
#include <qcolor.h>
#include <qcursor.h>
#include <qapplication.h>
#include <QMouseEvent>
#include <QEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QList>

//#include <algorithm>

#include "pfsview_widget.h"

//#include "pfsglviewwidget.h"


#include <assert.h>

#define min(x,y) ( (x)<(y) ? (x) : (y) )
#define max(x,y) ( (x)>(y) ? (x) : (y) )


#define D65_LUM_R 0.212656f
#define D65_LUM_G 0.715158f
#define D65_LUM_B 0.072186f


inline float clamp( float val, float min, float max )
{
  if( val < min ) return min;
  if( val > max ) return max;
  return val;
}

inline int clamp( int val, int min, int max )
{
  if( val < min ) return min;
  if( val > max ) return max;
  return val;
}


const char* COLOR_CHANNELS = "Color";

PFSViewWidgetArea::PFSViewWidgetArea( QWidget *parent ) :
  QScrollArea( parent )
{
  PFSViewWidget *pfsview = new PFSViewWidget( this );
  setWidget( pfsview );
  setWidgetResizable( true );
  
  setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );  
}

QSize PFSViewWidgetArea::sizeHint() const
{
  QSize sz = widget()->sizeHint();
//  printf( "widget: %d %d\n", sz.width(), sz.height() );
  sz += QSize( frameWidth()*2, frameWidth()*2 );  
//  printf( "area: %d %d\n", sz.width(), sz.height() );
  return sz;
}


PFSViewWidget::PFSViewWidget( QWidget *parent ) :
  QWidget( parent ), image( NULL ),
  minValue( 1.f ), maxValue( 100.f ), zoom( 1.f ),
  mappingMethod( MAP_GAMMA2_2 ), visibleChannel( COLOR_CHANNELS ),
  negativeTreatment( NEGATIVE_BLACK ), infNaNTreatment( INFNAN_MARK_AS_RED ),
  updateMappingRequested( true ), clippingMethod(CLIP_SIMPLE)
{

  setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
  
  
  setMouseTracking( true );
  setCursor( QCursor( Qt::CrossCursor ) );

//name, Qt::WNoAutoErase
  
//  setResizePolicy( Manual );
  
  pfsFrame = NULL;
  workArea[0] = workArea[1] = workArea[2] = NULL;

  selection += QRegion( 10, 10, 100, 100 );

  pointerValue.valid = false;    
  
}

PFSViewWidget::~PFSViewWidget()
{
  delete pfsFrame;
  delete workArea[0];
  delete workArea[1];
  delete workArea[2];
}

// ======================= Set current frame  ==========================

void PFSViewWidget::setFrame( pfs::Frame *frame )
{  
  pfsFrame = frame;    
  if( frame == NULL ) return;
  
  // If selected channel not available
  if( ( visibleChannel == COLOR_CHANNELS && !hasColorChannels( frame ) ) ||
    ( visibleChannel != COLOR_CHANNELS && frame->getChannel( visibleChannel ) == NULL ) ) 
  {
    // Chose first available channel
    pfs::ChannelIterator *it = frame->getChannels();
    if( !it->hasNext() )      // TODO: failover
      throw new pfs::Exception( "No channels available!" );
    visibleChannel = it->getNext()->getName();
  } else if( visibleChannel != COLOR_CHANNELS ) {
    // Get a new pointer, as the old frame object
    // can be delete after returning from this method
    visibleChannel = frame->getChannel( visibleChannel )->getName(); 
  }

  int desktopWidth, desktopHeight;
  desktopWidth = QApplication::desktop()->width();
  desktopHeight = QApplication::desktop()->height();
  if( frame->getWidth() > desktopWidth ||
      frame->getHeight() > desktopHeight ) {
    zoom = min( (float)desktopWidth / (float)frame->getWidth(),
      (float)desktopHeight / (float)frame->getHeight() );
  }
  
  updateZoom();
  setPointer();  
}

//void PFSViewWidget::drawContents( QPainter * p, int clipx, int clipy, int clipw, int cliph )

void PFSViewWidget::paintEvent( QPaintEvent *event )
{
  assert( pfsFrame != NULL );

  QRect clip = event->rect();
  int clipx = clip.x();
  int clipy = clip.y();
  int clipw = clip.width();
  int cliph = clip.height();
  
  
  if( updateMappingRequested )
    updateMapping();
  
  QPainter p( this );
//  if( image != NULL ) p->drawImage( 0, 0, *image );
  if( image != NULL ) {
      p.drawImage( clipx, clipy, *image, clipx, clipy, clipw, cliph );

    //Erase area outside image
    if( clipx+clipw > image->width() )
      p.eraseRect( image->width(), 0, clipx+clipw - image->width(), image->height() );
    if( clipy+cliph > image->height() )
      p.eraseRect( 0, image->height(), image->width(), clipy+cliph - image->height() );
    if( clipx+clipw > image->width() && clipy+cliph > image->height() )
      p.eraseRect( image->width(), image->height(),
        clipx+clipw - image->width(), clipy+cliph - image->height() );
    
  }
  
  
}

// ======================= Update ===========================

static void scaleCopyArray(const pfs::Array2D *from, pfs::Array2D *to)
{
  assert( from->getRows() >= to->getRows() );
  assert( from->getCols() >= to->getCols() );
  
  float sx, sy;
  float dx = (float)from->getCols() / (float)to->getCols();
  float dy = (float)from->getRows() / (float)to->getRows();

  int x, y, i = 0;
  for( sy = 0, y = 0; y < to->getRows(); y++, sy += dy ) {
    for( sx = 0, x = 0; x < to->getCols(); x++, sx += dx ) {
      (*to)(i++) = (*from)((int)(sx), (int)(sy) );
    }
  }
  
}

static void transformImageToWorkArea( pfs::Array2D **workArea, float zoom, bool color,
  pfs::Array2D *X, pfs::Array2D *Y = NULL, pfs::Array2D *Z = NULL )
{
  int origCols, origRows;
  origCols = X->getCols();
  origRows = X->getRows();

  int workCols, workRows;
  workCols = min( (int)((float)X->getCols() * zoom), X->getCols() );
  workRows = min( (int)((float)X->getRows() * zoom), X->getRows() );

  int requiredChannels = color ? 3 : 1;
  
  // Reallocate work area arrays to fit new size
  {
    for( int c = 0; c < requiredChannels; c++ ) {
      if( workArea[c] == NULL || workArea[c]->getCols() != workCols ||
        workArea[c]->getRows() != workRows ) {
        if( workArea[c] != NULL ) delete workArea[c];
        workArea[c] = new pfs::Array2DImpl( workCols, workRows );        
      }
    }
  }

  //Copy | rescale & tranform image to work area
  if( color )
  {
    if( workCols == origCols && workRows == origRows ) {
      copyArray( X, workArea[0] );
      copyArray( Y, workArea[1] );
      copyArray( Z, workArea[2] );
    } else {
      scaleCopyArray( X, workArea[0] );
      scaleCopyArray( Y, workArea[1] );
      scaleCopyArray( Z, workArea[2] );
    } 
    pfs::transformColorSpace( pfs::CS_XYZ, workArea[0], workArea[1], workArea[2],
      pfs::CS_RGB, workArea[0], workArea[1], workArea[2] );
  } else {
    if( workCols == origCols && workRows == origRows ) 
      copyArray( X, workArea[0] );
    else 
      scaleCopyArray( X, workArea[0] );
  }
  
}

#define LUTSIZE 256

inline int binarySearchPixels( float x, const float *lut, const int lutSize )
{
  int l = 0, r = lutSize;
  while( true ) {
    int m = (l+r)/2;
    if( m == l ) break;
    if( x < lut[m] )
      r = m;
    else
      l = m;
  }
  return l;
}

static float getInverseMapping( LumMappingMethod mappingMethod,
  float v, float minValue, float maxValue )
{
  switch( mappingMethod ) {
  case MAP_GAMMA1_4:
    return powf( v, 1.4 )*(maxValue-minValue) + minValue;
  case MAP_GAMMA1_8:
    return powf( v, 1.8 )*(maxValue-minValue) + minValue;
  case MAP_GAMMA2_2:
    return powf( v, 2.2 )*(maxValue-minValue) + minValue;
  case MAP_GAMMA2_6:
    return powf( v, 2.6 )*(maxValue-minValue) + minValue;
  case MAP_LINEAR:
    return v*(maxValue-minValue) + minValue;
  case MAP_LOGARITHMIC:
    return powf( 10, v * (log10f(maxValue) - log10f(minValue)) + log10f( minValue ) );
  default:
    assert(0);
    return 0;
  }
}

/**
 * Copy pixels in work area to QImage using proper clipping and mapping
 * method. Handle also expansion if image in zoomed in.
 */
static void mapFrameToImage( pfs::Array2D *R, pfs::Array2D *G, pfs::Array2D *B, 
  QImage *img,
  float minValue, float maxValue,
  RGBClippingMethod clippingMethod,
  LumMappingMethod mappingMethod,
  InfNaNTreatment infNaNTreatment,
  NegativeTreatment negativeTreatment )
{
  assert( R != NULL );
  assert( img != NULL );
  int rows = R->getRows();
  int cols = R->getCols();
  bool expand = cols != img->width() || rows != img->height(); // Zoom in

  float imgDeltaRow, imgDeltaCol;
  if( expand ) {
    imgDeltaRow = (float)(img->height()) / (float)rows;
    imgDeltaCol = (float)(img->width()) / (float)cols;
  } else {
    imgDeltaRow = imgDeltaCol = 1;
  }

  bool color = (G != NULL);
  assert( !color || (color && B != NULL) );

  
  float lutPixFloor[257*2];
  QRgb lutPixel[257*2];
  int lutSize;
  if( !color && ( negativeTreatment == NEGATIVE_GREEN_SCALE ||
        negativeTreatment == NEGATIVE_ABSOLUTE ) ) { // Handle negative numbers
    lutSize = 257*2+1;
    for( int p = 256; p >= 0; p-- ) {
      float p_left = (float)p/255.f;
      lutPixFloor[256-p+1] = -getInverseMapping( mappingMethod, p_left, minValue, maxValue );
      if( p != 0 ) lutPixel[256-p+1] = negativeTreatment == NEGATIVE_GREEN_SCALE ?
                     QColor( 0, p-1, 0 ).rgb() : QColor( p-1, p-1, p-1 ).rgb();
    }
    for( int p = 0; p < 257; p++ ) {
      float p_left = (float)p/255.f;
      lutPixFloor[257+p+1] = getInverseMapping( mappingMethod, p_left, minValue, maxValue );
      if( p != 256 ) lutPixel[257+p+1] = QColor( p, p, p ).rgb();
    }

    if( clippingMethod == CLIP_COLORCODED ) {
      lutPixel[0] = negativeTreatment == NEGATIVE_GREEN_SCALE ?
        QColor( 0, 255, 255 ).rgb() : QColor( 255, 255, 0 ).rgb();
      lutPixel[257] = QColor( 0, 0, 255 ).rgb();
      lutPixel[257*2-1] = QColor( 255, 255, 0 ).rgb();
    } else {
      lutPixel[0] = lutPixel[1];
      lutPixel[257] = QColor( 0, 0, 0 ).rgb();
      lutPixel[257*2-1] = QColor( 255, 255, 255 ).rgb();
    }  
    
  } else {                      // clip negative numbers
    int neg_offset = ((!color && negativeTreatment == NEGATIVE_MARK_AS_RED) ? 1 : 0);
    lutSize = 257+1 + neg_offset;            // +1 - for lower bound
    for( int p = 1+neg_offset; p <= LUTSIZE+1; p++ ) {
      float p_left = ((float)p - 1.f - (float)neg_offset)/255.f; // Should be -1.5f, but we don't want negative nums
      lutPixFloor[p] = getInverseMapping( mappingMethod, p_left, minValue, maxValue );
      if( !color && p < LUTSIZE+1+neg_offset ) lutPixel[p] = QColor( p-1-neg_offset, p-1-neg_offset, p-1-neg_offset ).rgb();      
//      printf( "p = %d\tl = %g\n", p, lookupTable[p] );
    }
    if( clippingMethod == CLIP_COLORCODED ) {
      lutPixel[neg_offset] = QColor( 0, 0, 255 ).rgb();
      lutPixel[LUTSIZE+1+neg_offset] = QColor( 255, 255, 0 ).rgb();
    } else {
      lutPixel[neg_offset] = QColor( 0, 0, 0 ).rgb();
      lutPixel[LUTSIZE+1+neg_offset] = QColor( 255, 255, 255 ).rgb();
    }
    if( negativeTreatment == NEGATIVE_MARK_AS_RED && !color) {
      lutPixFloor[1] = 0;
      lutPixel[0] = QColor( 255, 0, 0 ).rgb();
    }
  }

//  bool once = true;
  
 // #pragma omp parallel for private (index)
  #pragma omp parallel for
  for( int r = 0; r < rows; r++ ) {
	float imgRow, imgCol;
	int index = r*cols;
	imgRow = (float)r * imgDeltaRow;
    QRgb* line = (QRgb*)img->scanLine( (int)imgRow );
    imgCol = 0;
    for( int c = 0; c < cols; c++, index++, imgCol += imgDeltaCol ) {
      QRgb pixel;
      if( color ) {
        // Color channels
        int pr, pg, pb;
        pr = binarySearchPixels( (*R)(index), lutPixFloor, lutSize );        
        pg = binarySearchPixels( (*G)(index), lutPixFloor, lutSize );        
        pb = binarySearchPixels( (*B)(index), lutPixFloor, lutSize );        

        // Clipping
        if( clippingMethod == CLIP_COLORCODED  ) {
          if( pr == 0 || pg == 0 || pb == 0 ) 
            pixel = lutPixel[0];
          else if( pr == lutSize-1 || pg == lutSize-1 || pb == lutSize-1 )
            pixel = lutPixel[LUTSIZE+1];
          else
            pixel = QColor( pr-1, pg-1, pb-1 ).rgb();
        } else if( clippingMethod == CLIP_KEEP_BRI_HUE ) {
          if( pr == lutSize-1 || pg == lutSize-1 || pb == lutSize-1 ||
            pr == 0 || pg == 0 || pb == 0 ) {
            float p[3];
            p[0] = (*R)(index); p[1] = (*G)(index); p[2] = (*B)(index);
            float gray = (p[0]+p[1]+p[2])/3.f;
//            float gray = D65_LUM_R*p[0]+D65_LUM_G*p[1]+D65_LUM_B*p[2];
            float t;

            if( gray >= maxValue ) {
              pixel = QColor( 255, 255, 255 ).rgb();
            } else if( gray <= minValue ) {
              pixel = QColor( 0, 0, 0 ).rgb();
            } else {
              int i;
              for( i = 0; i < 3; i++ ) {
                t = (maxValue - p[i])/(gray - p[i]);             
                if( t >= 0 && t <= 1 ) {
                  break;
                }              
                t = (minValue - p[i])/(gray - p[i]);             
                if( t >= 0 && t <= 1 ) {
                  break;
                }              
              }
            
              if( i == 3 ) 
                pixel = QColor( 255, 255, 255 ).rgb();
              else {
                for( int i = 0; i < 3; i++ )
                  p[i] = gray*t + p[i]*(1-t);
                
//               if( once ) {
//                 printf( "min = %g max = %g\n", minValue, maxValue );
//                 printf( "r = %g g = %g b = %g; t = %g\n", p[0], p[1], p[2], t );
//                 once = false;
//               }              
              
                pr = binarySearchPixels( p[0], lutPixFloor, lutSize );        
                pg = binarySearchPixels( p[1], lutPixFloor, lutSize );        
                pb = binarySearchPixels( p[2], lutPixFloor, lutSize );
                pixel = QColor( clamp( pr-1, 0, 255 ),
                  clamp( pg-1, 0, 255 ),
                  clamp( pb-1, 0, 255 ) ).rgb();
              }
            }  
          } else {
            pixel = QColor( clamp( pr-1, 0, 255 ),
              clamp( pg-1, 0, 255 ),
              clamp( pb-1, 0, 255 ) ).rgb();
          }
        } else {
          pixel = QColor( clamp( pr-1, 0, 255 ),
            clamp( pg-1, 0, 255 ),
            clamp( pb-1, 0, 255 ) ).rgb();
        }
        if( infNaNTreatment == INFNAN_MARK_AS_RED ) {
          if( !isfinite( (*R)(index) ) || !isfinite( (*G)(index) ) || !isfinite( (*B)(index) ) )
          {   // x is NaN or Inf 
            pixel = QColor( 255, 0, 0 ).rgb();
          }
        }
        if( negativeTreatment == NEGATIVE_MARK_AS_RED ) {
          if( (*R)(index)<0 || (*G)(index)<0 || (*B)(index)<0 )
          {   // x is negative
            pixel = QColor( 255, 0, 0 ).rgb();
          }
        }
        
      } else {
        // Single channel
        int p = binarySearchPixels( (*R)(index), lutPixFloor, lutSize );        
        pixel = lutPixel[p];
        if( infNaNTreatment == INFNAN_MARK_AS_RED && (p == 0 || p == LUTSIZE+1)) 
          if( !isfinite( (*R)(index) ) ) {   // x is NaN or Inf 
            pixel = QColor( 255, 0, 0 ).rgb();
          }
        
      }
      
      line[(int)imgCol] = pixel;

      if( expand ) {
        for( int ec = (int)imgCol + 1; ec < (int)(imgCol+imgDeltaCol); ec++ ) {
          line[ec] = pixel;
        }
      }
	  	  
    }

    if( expand ) {
      for( int er = (int)(imgRow + 1); er < (int)(imgRow + imgDeltaRow); er++ ) {
        QRgb* eLine = (QRgb*)img->scanLine( er );
        memcpy( eLine, line, sizeof( QRgb )*img->width() );
      }
      
    }

  }
  
}

void PFSViewWidget::postUpdateMapping()
{
  updateMappingRequested = true;

  update( 0, 0, image->width(), image->height() );
  //updateContents( 0, 0, image->width(), image->height() );
}


void PFSViewWidget::updateMapping()
{ 
  assert( image != NULL );

  QApplication::setOverrideCursor( Qt::WaitCursor );
  
  if( visibleChannel == COLOR_CHANNELS ) {
    
    assert( workArea[0] != NULL && workArea[1] != NULL && workArea[2] != NULL );
    mapFrameToImage( workArea[0], workArea[1], workArea[2], image,
      minValue, maxValue, clippingMethod, mappingMethod,
      infNaNTreatment, negativeTreatment );
    
  } else {
    
    assert( workArea[0] != NULL );
    mapFrameToImage( workArea[0], NULL, NULL, image,
      minValue, maxValue, clippingMethod, mappingMethod,
      infNaNTreatment, negativeTreatment );
  }  
  updateMappingRequested = false;

  QApplication::restoreOverrideCursor();
}

void PFSViewWidget::updateZoom()
{
  assert( pfsFrame != NULL );

  QApplication::setOverrideCursor( Qt::WaitCursor );
  
  if( visibleChannel == COLOR_CHANNELS ) {
    pfs::Channel *X, *Y, *Z;
    pfsFrame->getXYZChannels( X, Y, Z );
    transformImageToWorkArea( workArea, zoom, true, X, Y, Z );
  } else {
    pfs::Channel *X = pfsFrame->getChannel( visibleChannel );
    transformImageToWorkArea( workArea, zoom, false, X );    
  }  

  int zoomedWidth = max( workArea[0]->getCols(), (int)((float)(pfsFrame->getWidth())*zoom) );
  int zoomedHeight = max( workArea[0]->getRows(), (int)((float)(pfsFrame->getHeight())*zoom) );    
  
  if( image != NULL ) delete image;
  image = new QImage( zoomedWidth, zoomedHeight, QImage::Format_RGB32 );
  assert( image != NULL );
  
  postUpdateMapping();

  resize( zoomedWidth, zoomedHeight );
  
//  resizeContents( zoomedWidth, zoomedHeight );
//  updateContents( 0, 0, zoomedWidth, zoomedHeight );
  update( 0, 0, zoomedWidth, zoomedHeight );

  QApplication::restoreOverrideCursor();  
}

QSize PFSViewWidget::sizeHint() const
{
  if( pfsFrame != NULL ) 
    return QSize( (int)((float)pfsFrame->getWidth()*zoom), (int)((float)pfsFrame->getHeight()*zoom)  );

//    return QSize( (int)((float)pfsFrame->getWidth()*zoom) + 2 * frameWidth(), (int)((float)pfsFrame->getHeight()*zoom) + 2 * frameWidth() );

  //    return QSize( pfsFrame->getWidth() + 2 * frameWidth(), pfsFrame->getHeight() + 2 * frameWidth() );
  else return QWidget::sizeHint();
}

// ======================= Events ===========================

void PFSViewWidget::setRGBClippingMethod( QAction *action )
{
  clippingMethod = (RGBClippingMethod)action->data().toUInt();
  postUpdateMapping();
}

void PFSViewWidget::setInfNaNTreatment( QAction *action )
{
  infNaNTreatment = (InfNaNTreatment)action->data().toUInt();
  postUpdateMapping();
}

void PFSViewWidget::setNegativeTreatment( QAction *action )
{
  negativeTreatment = (NegativeTreatment)action->data().toUInt();
  postUpdateMapping();
}

void PFSViewWidget::setLumMappingMethod( int method )
{
  mappingMethod = (LumMappingMethod)method;
  postUpdateMapping();
}



void PFSViewWidget::zoomIn()
{
  if( zoom >= 10 ) return;
  zoom *= 1.25f;
  updateZoom();
}


void PFSViewWidget::zoomOut()
{
  if( zoom <= 0.05 ) return;
  zoom *= 0.8f;
  updateZoom();
}

void PFSViewWidget::zoomOriginal()
{
  zoom = 1;
  updateZoom();  
}

void PFSViewWidget::setRangeWindow( float min, float max )
{
  minValue = min;
  maxValue = max;
  postUpdateMapping();
}


// ===================== Mouse interaction =========================

void PFSViewWidget::mouseMoveEvent( QMouseEvent *mouseEvent )
{
  assert( pfsFrame != NULL );

  setPointer( (int)((float)mouseEvent->x() / zoom),
    (int)((float)mouseEvent->y() / zoom) );  
}

void PFSViewWidget::setPointer( int x, int y )
{
  assert( pfsFrame != NULL );

  if( x >= 0 ) {    
    pointerValue.x = x;
    pointerValue.y = y;
  }
  
  if( pointerValue.x >= 0 && pointerValue.x < pfsFrame->getWidth() &&
    pointerValue.y >= 0 && pointerValue.y < pfsFrame->getHeight() ) {

    if( visibleChannel == COLOR_CHANNELS ) {
      
      pfs::Channel *X, *Y, *Z;
      pfsFrame->getXYZChannels( X, Y, Z );
    
      pointerValue.value[0] = (*X)( pointerValue.x, pointerValue.y );
      pointerValue.value[1] = (*Y)( pointerValue.x, pointerValue.y );
      pointerValue.value[2] = (*Z)( pointerValue.x, pointerValue.y );

      pointerValue.valuesUsed = 3;
      pointerValue.valid = true;

    } else {

      pfs::Channel *X;
      X = pfsFrame->getChannel( visibleChannel );
    
      pointerValue.value[0] = (*X)( pointerValue.x, pointerValue.y );

      pointerValue.valuesUsed = 1;
      pointerValue.valid = true;      
      
    }
    
    updatePointerValue();
    
  } else {
    pointerValue.valid = false;    
    updatePointerValue();
  }
  
  
}

void PFSViewWidget::leaveEvent ( QEvent * )
{
    pointerValue.valid = false;    
    updatePointerValue();
}

const PointerValue &PFSViewWidget::getPointerValue()
{
  return pointerValue;
}


// ===================== Data access =========================


const pfs::Array2D *PFSViewWidget::getPrimaryChannel()
{
  assert( pfsFrame != NULL );
  if( visibleChannel == COLOR_CHANNELS ) {
    pfs::Channel *X, *Y, *Z;
    pfsFrame->getXYZChannels( X, Y, Z );
    return Y;
  } else {
    return pfsFrame->getChannel( visibleChannel );
  }
  
}

const QList<const char*> PFSViewWidget::getChannels()
{
  assert( pfsFrame != NULL );
  
  QList<const char*> chArray;

  pfs::ChannelIterator *it = pfsFrame->getChannels();
  it = pfsFrame->getChannels();
  while( it->hasNext() )
  {
    pfs::Channel *ch = it->getNext();
    chArray.push_back(ch->getName());
  }

  return chArray;
}

void PFSViewWidget::setVisibleChannel( const char *channelName )
{
  visibleChannelName = channelName;
  
  visibleChannel = channelName != NULL ? (const char*)visibleChannelName : COLOR_CHANNELS;
  
  updateZoom();
  setPointer();
}

const char *PFSViewWidget::getVisibleChannel()
{
  return visibleChannel;
}

bool PFSViewWidget::hasColorChannels( pfs::Frame *frame )
{
  if( frame == NULL ) frame = pfsFrame;
  assert( frame != NULL );
  pfs::Channel *X, *Y, *Z;
  frame->getXYZChannels( X, Y, Z );
  return ( X != NULL );
}

