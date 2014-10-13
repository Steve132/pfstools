/**
 * @brief Perform projective transformations of spherical images
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
 * @author Miloslaw Smyk, <thorgal@wfmh.org.pl>
 *
 * $Id: pfspanoramic.cpp,v 1.5 2014/04/05 22:04:12 rafm Exp $
 */

#include <map>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>

#include <pfs.h>

#define PROG_NAME "pfspanoramic"
using namespace std;

class QuietException 
{
};

const double EPSILON=1e-7;

const double PI = 3.1415926535897932384626433;
const double ONE_PI = 0.31830988618379067154; // 1/PI

static void errorCheck( bool condition, const char *string )
{
    if( !condition ) {
  fprintf( stderr, PROG_NAME " error: %s\n", string );
  throw QuietException();
    }
}


class Vector3D
{
  public:
  double x, y, z;

  Vector3D(double phi, double theta)
  {
    x = cos(phi) * sin(theta);
    y = sin(phi) * sin(theta);
    z = cos(theta);
  }

  Vector3D(double x, double y, double z)
  {
    this->x = x;
    this->y = y;
    this->z = z;

    normalize();
  }
  
  double magnitude(void)
  {
    return sqrt( x * x + y * y + z * z );
  }

  void normalize(void)
  {
    double len = magnitude();

    x = x / len;
    y = y / len;
    z = z / len;
  }

  double dot(Vector3D *v)
  {
    return x * v->x + y * v->y + z * v->z;
  }

  //TODO: optimize rotations by precomputing sines and cosines
  void rotateX(double angle)
  {
    angle *= (PI / 180.);

    double c = cos(angle);
    double s = sin(angle);

    double y2 =  c * y + -s * z;
    double z2 =  s * y + c * z;

    y = y2;
    z = z2;
  }

  void rotateY(double angle)
  {
    angle *= (PI / 180.);

    double c = cos(angle);
    double s = sin(angle);

    double x2 =  c * x + s * z;
    double z2 = -s * x + c * z;

    x = x2;
    z = z2;
  }

  void rotateZ(double angle)
  {
    angle *= (PI / 180);

    double c = cos(angle);
    double s = sin(angle);

    double x2 =  c * x + -s * y;
    double y2 =  s * x + c * y;

    x = x2;
    y = y2;
  }
};

class Point2D
{
  public:
    double x, y;

  Point2D(double x, double y)
  {
    this->x = x;
    this->y = y;
  }
};
class Projection
{
  protected:
  const char *name;
  public:
    
    virtual Vector3D *uvToDirection(double u, double v) = 0;
    virtual Point2D *directionToUV(Vector3D *direction) = 0;
    virtual bool isValidPixel(double u, double v) = 0;
    virtual double getSizeRatio(void) = 0;
    virtual ~Projection()
    {
    }

    virtual void setOptions(char *opts)
    {
    }

    virtual const char *getName(void)
    {
      return name;
    }
};


//class Projection;
typedef Projection*(*ProjectionCreator)(void);

class ProjectionFactory
{
    static ProjectionFactory singleton;

    ProjectionFactory(bool init)
    {
    }

  public:
    map<string, ProjectionCreator> projections;

    static void registerProjection(const char *name, ProjectionCreator ptr)
    {
      singleton.projections[ string( name ) ] = ptr;
    }

    static Projection *getProjection(char *name)
    {
      char *opts;
      
      if(opts = strchr(name, '/'))
      {
        *opts++ = '\0';
      }

      ProjectionCreator projectionCreator = singleton.projections.find(string(name))->second;

      if(projectionCreator != NULL)
      {
        Projection *projection = projectionCreator();

        if(opts != NULL)
          projection->setOptions(opts);

        return projection;
      }
      else
        return NULL;
    }

    //FIXME: Lame. Should return an iterator over the names. No time for this now. :/
    static void listProjectionNames(void)
    {
      map<string, ProjectionCreator>::iterator i = singleton.projections.begin();

      while(i != singleton.projections.end())
      {
        fprintf( stderr, "%s\n", (*i).first.c_str());
        i++;
      }
    }
};

ProjectionFactory ProjectionFactory::singleton = true;

class MirrorBallProjection : public Projection
{
  static MirrorBallProjection singleton;

  MirrorBallProjection(bool initialization)
  {
    name = "mirrorball";

    if(initialization)
      ProjectionFactory::registerProjection(name, this->create);
  }

  public:
  static Projection* create()
  {
    return new MirrorBallProjection(false);
  }

  const char *getName(void)
  {
    return name;
  }

  double getSizeRatio(void)
  {
    return 1;
  }

  bool isValidPixel(double u, double v)
  {
    // check if we are not in a boundary region (outside a circle)
    if((u - 0.5) * (u - 0.5) + (v - 0.5) * (v - 0.5) > 0.25)
      return false;
    else
      return true;
  }

  Vector3D* uvToDirection(double u, double v)
  {
    u = 2 * u - 1;
    v = 2 * v - 1;

    double phi = atan2( v, u );
    double theta = 2 * asin( sqrt( u * u + v * v ) );

    Vector3D *direction = new Vector3D(phi, theta);
    double t;

    direction->y = -direction->y;

    return direction;
  }

  Point2D* directionToUV(Vector3D *direction)
  {
    double u, v;

    direction->y = -direction->y;

    if(fabs(direction->x) > 0 || fabs(direction->y) > 0)
    {
      double distance = sqrt(direction->x * direction->x + direction->y * direction->y);

      double r = 0.5 * (sin(acos(direction->z) / 2)) / distance;

      u = direction->x * r + 0.5;
      v = direction->y * r + 0.5;
    }
    else
    {
      u = v = 0.5;
    }

    return new Point2D(u, v);
  }
};

class AngularProjection : public Projection
{
  static AngularProjection singleton;
  double totalAngle;

  AngularProjection(bool initialization)
  {
    name = "angular";

    if(initialization)
      ProjectionFactory::registerProjection(name, this->create);
  }

  public:
  static Projection* create()
  {
    AngularProjection *p = new AngularProjection(false);
    p->totalAngle = 360;

    return (Projection *)p;
  }

  void setOptions(char *opts)
  {
    char *delimiter;
    static const char *OPTION_ANGLE = "angle";

    while(*opts)
    {
      //fprintf(stderr,"option: %s\n", opts);
      //if(delimiter = strchr(name, '/'))
        //*delimiter++ = '\0';


      if(strncmp(opts, OPTION_ANGLE, strlen(OPTION_ANGLE)) == 0)
      {
        totalAngle = strtod(opts + strlen(OPTION_ANGLE) + 1, &delimiter);
      //  fprintf(stderr,"angle: %g\n", totalAngle);

        if(0 >= totalAngle || totalAngle > 360)
        {
          fprintf( stderr, PROG_NAME " error: angular projection: angle must be in (0,360] degrees range.\n" );
          throw QuietException();
        }
      }
      else
      {
        fprintf( stderr, PROG_NAME " error: angular projection: unknown option: %s\n", opts );
        throw QuietException();
      }

      opts = delimiter + 1;
    }
  }


  const char *getName(void)
  {
    return name;
  }

  double getSizeRatio(void)
  {
    return 1;
  }

  bool isValidPixel(double u, double v)
  {
    // check if we are not in a boundary region (outside a circle)
    if((u - 0.5) * (u - 0.5) + (v - 0.5) * (v - 0.5) > 0.25)
      return false;
    else
      return true;
  }

  Vector3D* uvToDirection(double u, double v)
  {
    u = 2 * u - 1;
    v = 2 * v - 1;

    u *= totalAngle / 360;
    v *= totalAngle / 360;

    double phi = atan2( v, u );
    double theta = PI * sqrt( u * u + v * v );

    Vector3D *direction = new Vector3D(phi, theta);
    double t;

    direction->y = -direction->y;

    return direction;
  }

  Point2D* directionToUV(Vector3D *direction)
  {
    double u, v;

    direction->y = -direction->y;

    if(fabs(direction->x) > 0 || fabs(direction->y) > 0)
    {
      double distance = sqrt(direction->x * direction->x + direction->y * direction->y);

      double r = (1 / (2 * PI)) * acos(direction->z) / distance;

      u = direction->x * r + 0.5;
      v = direction->y * r + 0.5;
    }
    else
    {
      u = v = 0.5;
    }

    return new Point2D(u, v);
  }
};

class CylindricalProjection : public Projection
{
  Vector3D *pole;
  Vector3D *equator;
  Vector3D *cross;

  static CylindricalProjection singleton;

  CylindricalProjection(bool initialization)
  {
    name = "cylindrical";

    if(initialization)
      ProjectionFactory::registerProjection(name, this->create);

    pole = new Vector3D(0, 1, 0);
    equator = new Vector3D(0, 0, -1);
    cross = new Vector3D(1, 0, 0);
  }

  public:
  static Projection* create()
  {
    return new CylindricalProjection(false);
  }

  ~CylindricalProjection()
  {
    delete pole;
    delete equator;
    delete cross;
  }

  double getSizeRatio(void)
  {
    return 2;
  }

  bool isValidPixel(double u, double v)
  {
    return true;
  }

  Vector3D* uvToDirection(double u, double v)
  {
    u = 0.75 - u;

    u *= PI * 2;

    v = acos( 1 - 2 * v );

    Vector3D *direction = new Vector3D(u, v);

    double temp = direction->z;
    direction->z = direction->y;
    direction->y = temp;

    return direction;
  }

  Point2D* directionToUV(Vector3D *direction)
  {
    double u, v;
    double lat = direction->dot(pole);

    v = ( 1 - lat ) / 2;

    if(v < EPSILON || fabs(1 - v) < EPSILON)
      u = 0;
    else
    {
      double ratio = equator->dot( direction ) / sin( acos( lat ) );

      if(ratio < -1)
        ratio = -1;
      else
        if(ratio > 1)
          ratio = 1;

      double lon = acos(ratio) / (2 * PI);

      if(cross->dot(direction) < 0)
        u = lon;
      else
        u = 1 - lon;

      if(u == 1)
        u = 0;

      if(v == 1)
        v = 0;
    }

  //  if ( 0 > v || v >= 1 ) fprintf(stderr, "u: %f (%f,%f,%f)\n", v, direction->x, direction->y, direction->z);
  //  assert ( -0. <= u && u < 1 );
  //  assert ( -0. <= v && v < 1 );
    return new Point2D(u, v);
  }
};

class PolarProjection : public Projection
{
  Vector3D *pole;
  Vector3D *equator;
  Vector3D *cross;

  static PolarProjection singleton;

  PolarProjection(bool initialization)
  {
    name = "polar";

    if(initialization)
      ProjectionFactory::registerProjection(name, this->create);

    pole = new Vector3D(0, 1, 0);
    equator = new Vector3D(0, 0, -1);
    cross = new Vector3D(1, 0, 0);
  }

  public:
  static Projection* create()
  {
    return new PolarProjection(false);
  }

  ~PolarProjection()
  {
    delete pole;
    delete equator;
    delete cross;
  }

  double getSizeRatio(void)
  {
    return 2;
  }

  bool isValidPixel(double u, double v)
  {
    return true;
  }

  Vector3D* uvToDirection(double u, double v)
  {
    u = 0.75 - u;

    u *= PI * 2;
    v *= PI;

    Vector3D *direction = new Vector3D(u, v);

    double temp = direction->z;
    direction->z = direction->y;
    direction->y = temp;

    return direction;
  }

  Point2D* directionToUV(Vector3D *direction)
  {
    double u, v;
    double lat = acos(direction->dot(pole));

    v = lat * ONE_PI;

    if(v < EPSILON || fabs(1 - v) < EPSILON)
      u = 0;
    else
    {
      double ratio = equator->dot(direction) / sin(lat);

      if(ratio < -1)
        ratio = -1;
      else
        if(ratio > 1)
          ratio = 1;

      double lon = acos(ratio) / (2 * PI);

      if(cross->dot(direction) < 0)
        u = lon;
      else
        u = 1 - lon;

      if(u == 1)
        u = 0;

      if(v == 1)
        v = 0;
    }

  //  if ( 0 > v || v >= 1 ) fprintf(stderr, "u: %f (%f,%f,%f)\n", v, direction->x, direction->y, direction->z);
  //  assert ( -0. <= u && u < 1 );
  //  assert ( -0. <= v && v < 1 );
    return new Point2D(u, v);
  }
};

PolarProjection PolarProjection::singleton = true;
CylindricalProjection CylindricalProjection::singleton = true;
AngularProjection AngularProjection::singleton = true;
MirrorBallProjection MirrorBallProjection::singleton = true;

class TransformInfo
{
  public:
  double xRotate;
  double yRotate;
  double zRotate;
  int oversampleFactor;
  bool interpolate;
  Projection *srcProjection;
  Projection *dstProjection;

  TransformInfo()
  {
    xRotate = yRotate = zRotate = 0;
    oversampleFactor = 1;
    interpolate = false;
    srcProjection = dstProjection = NULL;
  }
};

void transformArray( const pfs::Array2D *in, pfs::Array2D *out, TransformInfo *transformInfo);

void printHelp()
{
  fprintf( stderr, PROG_NAME " <source projection>+<target projection> [--width <val>] [--height <val>] [--oversample <val>] [--interpolate] [--xrotate <angle>] [--yrotate <angle>] [--zrotate <angle>] [--verbose] [--help]\n"
    "See man page for more information.\n" );
}

void panoramic( int argc, char* argv[] )
{
  pfs::DOMIO pfsio;

  // default parameters
  int xSize = -1;
  int ySize = -1;
 
  bool verbose = false;
  TransformInfo transformInfo;

  static struct option cmdLineOptions[] = {
    { "help", no_argument, NULL, 'e' },
    { "verbose", no_argument, NULL, 'v' },
    { "width", required_argument, NULL, 'w' },
    { "height", required_argument, NULL, 'h' },
    { "oversample", required_argument, NULL, 'o' },
    { "interpolate", no_argument, NULL, 'i' },
    { "xrotate", required_argument, NULL, 'x' },
    { "yrotate", required_argument, NULL, 'y' },
    { "zrotate", required_argument, NULL, 'z' },
    { NULL, 0, NULL, 0 }
  };

  int optionIndex = 0;
  while( true ) {
    int c = getopt_long( argc, argv, "w:h:o:ix:y:z:", cmdLineOptions, &optionIndex );
    if( c == -1 ) break;
    switch( c ) {
    case 'e':
      printHelp();
      throw QuietException();
    case 'v':
      verbose = true;
      break;
    case 'w':
      xSize = strtol( optarg, NULL, 10 );
      break;
    case 'h':
      ySize = strtol( optarg, NULL, 10 );
      break;
    case 'o':
      transformInfo.oversampleFactor = strtol( optarg, NULL, 10 );
      break;
    case 'i':
      transformInfo.interpolate = true;
      break;
    case 'x':
      transformInfo.xRotate = strtod( optarg, NULL );
      break;
    case 'y':
      transformInfo.yRotate = strtod( optarg, NULL );
      break;
    case 'z':
      transformInfo.zRotate = strtod( optarg, NULL );
      break;
    case '?':
      throw QuietException();
    case ':':
      throw QuietException();
    }
  } 

  if( optind < argc )
  {
    bool set = false;

    while( optind < argc )
    {
      char *destination;

      if( set || ( destination = strchr( argv[optind], '+' ) ) == NULL )
      {
        fprintf( stderr, PROG_NAME " error: unknown parameter: %s\n", argv[optind]);
        throw QuietException();
      }

      // replace 'plus' sign with a string terminator,
      // thus separating input and output filter names.
      *destination++ = '\0';

      transformInfo.srcProjection = ProjectionFactory::getProjection( argv[optind] );
      transformInfo.dstProjection = ProjectionFactory::getProjection( destination );
      set = true;

      optind++;
    }
  }

  if( transformInfo.srcProjection == NULL || transformInfo.dstProjection == NULL )
  {
    fprintf( stderr, PROG_NAME " error: unknown projection. Possible values:\n" );
    ProjectionFactory::listProjectionNames();
    throw QuietException();
  }

  errorCheck(transformInfo.oversampleFactor > 0, "Oversample factor must be > 0");


  if( verbose )
  {
    fprintf( stderr, PROG_NAME ": reprojecting %s to %s", transformInfo.srcProjection->getName(), transformInfo.dstProjection->getName() );

    if( transformInfo.interpolate == true )
      fprintf( stderr, " with bilinear interpolation" );

    if(transformInfo.oversampleFactor > 1)
    {
      if( transformInfo.interpolate == true )
        fprintf( stderr, " and" );
      else
        fprintf( stderr, " with" );

      fprintf( stderr, " %dx%d oversampling", transformInfo.oversampleFactor, transformInfo.oversampleFactor );
    }

    if( transformInfo.xRotate != 0 || transformInfo.yRotate != 0 || transformInfo.zRotate != 0 )
    {
      fprintf( stderr, ", while rotating by %f, %f and %f degrees around X, Y and Z respectively", transformInfo.xRotate, transformInfo.yRotate, transformInfo.zRotate);
    }

    fprintf( stderr, ".\n");
  }

  pfs::Frame *transformedFrame = NULL;
  bool firstFrame = true;

  while( true ) {
    pfs::Frame *frame = pfsio.readFrame( stdin );
    if( frame == NULL ) break; // No more frames
        
    pfs::Channel *X, *Y, *Z;
    frame->getXYZChannels( X, Y, Z );

    pfs::Channel *dX, *dY, *dZ;
 

    if(xSize == -1 && ySize == -1)
    {
      xSize = frame->getWidth();
      ySize = (int)(xSize / transformInfo.dstProjection->getSizeRatio());
    }
    else
    {
      if(xSize == -1)
        xSize = (int)(ySize * transformInfo.dstProjection->getSizeRatio());

      if(ySize == -1)
        ySize = (int)(xSize / transformInfo.dstProjection->getSizeRatio());
    }

		if( firstFrame )
		{
			transformedFrame = pfsio.createFrame( xSize, ySize );
			firstFrame = false;
		}
      
		pfs::ChannelIterator *it = frame->getChannels();
		while( it->hasNext() ) {
			pfs::Channel *originalCh = it->getNext();
			pfs::Channel *newCh = transformedFrame->createChannel( originalCh->getName() );

			//TODO: major optimization: transform all channels in one go.
			transformArray( originalCh, newCh, &transformInfo );
		}
      

    pfs::copyTags( frame, transformedFrame );

    pfsio.writeFrame( transformedFrame, stdout );
    pfsio.freeFrame( frame );        
  }
  pfsio.freeFrame( transformedFrame );

  delete transformInfo.dstProjection;
  delete transformInfo.srcProjection;
}

void transformArray( const pfs::Array2D *in, pfs::Array2D *out, TransformInfo *transformInfo )
{
  const double delta = 1. / transformInfo->oversampleFactor;
  const double offset = 0.5 / transformInfo->oversampleFactor;
  const double scaler = 1. / ( transformInfo->oversampleFactor * transformInfo->oversampleFactor );
  
  const int outRows = out->getRows();
  const int outCols = out->getCols();

  const int inRows = in->getRows();
  const int inCols = in->getCols();


  for( int y = 0; y < outRows; y++ )
    for( int x = 0; x < outCols; x++ ) {
      double pixVal = 0;

      if( transformInfo->dstProjection->isValidPixel(( x + 0.5 ) / outCols, ( y + 0.5 ) / outCols ) == true )
      {
        for( double sy = 0, oy = 0; oy < transformInfo->oversampleFactor; sy += delta, oy++ )
          for( double sx = 0, ox = 0; ox < transformInfo->oversampleFactor; sx += delta, ox++ )
          {
            Vector3D *direction = transformInfo->dstProjection->uvToDirection(
                ( x + offset + sx ) / outCols, ( y + offset + sy ) / outRows );

            if(direction == NULL)
              continue;

            // angles below are negated, because we want to rotate
            // the environment around us, not us within the environment.
            if( transformInfo->xRotate != 0 )
              direction->rotateX( -transformInfo->xRotate );

            if( transformInfo->yRotate != 0 )
              direction->rotateY( -transformInfo->yRotate );

            if( transformInfo->zRotate != 0 )
              direction->rotateZ( -transformInfo->zRotate );

            Point2D *p = transformInfo->srcProjection->directionToUV( direction );

            p->x *= inCols;
            p->y *= inRows;

            if( transformInfo->interpolate == true )
            {
              int ix = (int)floor( p->x );
              int iy = (int)floor( p->y );

              double i = p->x - ix;
              double j = p->y - iy;

              // compute pixel weights for interpolation
              double w1 = i * j;
              double w2 = (1 - i) * j;
              double w3 = (1 - i) * (1 - j);
              double w4 = i * (1 - j);

              int dx = ix + 1;
              if(dx >= inCols)
                dx = inCols - 1;

              int dy = iy + 1;
              if(dy >= inRows)
                dy = inRows - 1;

              pixVal += w3 * (*in)(ix, iy) +
                        w4 * (*in)(dx, iy) +
                        w1 * (*in)(dx, dy) +
                        w2 * (*in)(ix, dy);
            }
            else
            {
              int ix = (int)floor(p->x + 0.5);
              int iy = (int)floor(p->y + 0.5);

              if(ix >= inCols)
                ix = inCols - 1;

              if(iy >= inRows)
                iy = inRows - 1;

              pixVal += (*in)(ix, iy);
            }


            delete direction;
            delete p;

            (*out)(x,y) = (float)(pixVal * scaler);
          }
      }
    }
}

int main( int argc, char* argv[] )
{
  try {
    panoramic( argc, argv );
  }
  catch( pfs::Exception ex ) {
    fprintf( stderr, PROG_NAME " error: %s\n", ex.getMessage() );
    return EXIT_FAILURE;
  }        
  catch( QuietException  ex ) {
    return EXIT_FAILURE;
  }        
  return EXIT_SUCCESS;
}
