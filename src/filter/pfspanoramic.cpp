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

  Vector3D(double tx, double ty, double tz):x(tx),y(ty),z(tz)
  {
    normalize();
  }
  
  double magnitude(void) const
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

  double dot(const Vector3D& v) const
  {
    return x * v.x + y * v.y + z * v.z;
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

  Point2D(double tx, double ty):x(tx),y(ty)
  {}
};
class Projection
{
  protected:
  const char *name;
  public:
    
    virtual Vector3D uvToDirection(double u, double v) = 0;
    virtual Point2D directionToUV(const Vector3D& direction) = 0;
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
      
      if( (opts = strchr(name, '/')) )
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

  Vector3D uvToDirection(double u, double v)
  {
    u = 2 * u - 1;
    v = 2 * v - 1;

    double phi = atan2( v, u );
    double theta = 2 * asin( sqrt( u * u + v * v ) );

    Vector3D direction(phi, theta);
    double t;

    direction.y = -direction.y;

    return direction;
  }

  Point2D directionToUV(const Vector3D& d)
  {
    double u, v;
    Vector3D direction(d.x,d.y,d.z);
    direction.y = -direction.y;

    if(fabs(direction.x) > 0 || fabs(direction.y) > 0)
    {
      double distance = sqrt(direction.x * direction.x + direction.y * direction.y);

      double r = 0.5 * (sin(acos(direction.z) / 2)) / distance;

      u = direction.x * r + 0.5;
      v = direction.y * r + 0.5;
    }
    else
    {
      u = v = 0.5;
    }

    return Point2D(u, v);
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

  Vector3D uvToDirection(double u, double v)
  {
    u = 2 * u - 1;
    v = 2 * v - 1;

    u *= totalAngle / 360;
    v *= totalAngle / 360;

    double phi = atan2( v, u );
    double theta = PI * sqrt( u * u + v * v );

    Vector3D direction(phi, theta);
    double t;

    direction.y = -direction.y;

    return direction;
  }

  Point2D directionToUV(const Vector3D& d)
  {
    double u, v;

    Vector3D direction(d.x,d.y,d.z);
    direction.y = -direction.y;

    if(fabs(direction.x) > 0 || fabs(direction.y) > 0)
    {
      double distance = sqrt(direction.x * direction.x + direction.y * direction.y);

      double r = (1 / (2 * PI)) * acos(direction.z) / distance;

      u = direction.x * r + 0.5;
      v = direction.y * r + 0.5;
    }
    else
    {
      u = v = 0.5;
    }

    return Point2D(u, v);
  }
};

class CylindricalProjection : public Projection
{
  Vector3D pole;
  Vector3D equator;
  Vector3D cross;

  static CylindricalProjection singleton;

  CylindricalProjection(bool initialization):
	pole(0, 1, 0),
	equator(0, 0, -1),
	cross(1, 0, 0)
  {
    name = "cylindrical";

    if(initialization)
      ProjectionFactory::registerProjection(name, this->create);

    
  }

  public:
  static Projection* create()
  {
    return new CylindricalProjection(false);
  }

  ~CylindricalProjection()
  {
  }

  double getSizeRatio(void)
  {
    return 2;
  }

  bool isValidPixel(double u, double v)
  {
    return true;
  }

  Vector3D uvToDirection(double u, double v)
  {
    u = 0.75 - u;

    u *= PI * 2;

    v = acos( 1 - 2 * v );

    Vector3D direction(u, v);

    double temp = direction.z;
    direction.z = direction.y;
    direction.y = temp;

    return direction;
  }

  Point2D directionToUV(const Vector3D& direction)
  {
    double u, v;
    double lat = direction.dot(pole);

    v = ( 1 - lat ) / 2;

    if(v < EPSILON || fabs(1 - v) < EPSILON)
      u = 0;
    else
    {
      double ratio = equator.dot( direction ) / sin( acos( lat ) );

      if(ratio < -1)
        ratio = -1;
      else
        if(ratio > 1)
          ratio = 1;

      double lon = acos(ratio) / (2 * PI);

      if(cross.dot(direction) < 0)
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
    return Point2D(u, v);
  }
};

class PolarProjection : public Projection
{
  Vector3D pole;
  Vector3D equator;
  Vector3D cross;

  static PolarProjection singleton;
  bool issquare;

  PolarProjection(bool initialization):
   pole(0, 1, 0),
    equator(0, 0, -1),
    cross(1, 0, 0)
  {
    name = "polar";
    
    if(initialization)
      ProjectionFactory::registerProjection(name, this->create);

  }

  public:
  static Projection* create()
  {
    PolarProjection* pp=new PolarProjection(false);
    pp->issquare=false;
    return pp;
  }

  ~PolarProjection()
  {
  }

  double getSizeRatio(void)
  {
    return this->issquare ? 1.0 : 2.0;
  }

  bool isValidPixel(double u, double v)
  {
    return true;
  }
  void setOptions(char *opts)
  {
    static const char *OPTION_SQUARE = "square";
    char* delimiter;
    while(*opts)
    {
      if(strncmp(opts, OPTION_SQUARE, strlen(OPTION_SQUARE)) == 0)
      {
        delimiter=opts+strlen(OPTION_SQUARE);
	this->issquare=true;
      }
      else
      {
        fprintf( stderr, PROG_NAME " error: angular projection: unknown option: %s\n", opts );
        throw QuietException();
      }

      opts = delimiter + 1;
    }
  }

  Vector3D uvToDirection(double u, double v)
  {
    u = 0.75 - u;

    u *= PI * 2;
    v *= PI;

    Vector3D direction(u, v);

    double temp = direction.z;
    direction.z = direction.y;
    direction.y = temp;

    return direction;
  }

  Point2D directionToUV(const Vector3D& direction)
  {
    double u, v;
    double lat = acos(direction.dot(pole));

    v = lat * ONE_PI;

    if(v < EPSILON || fabs(1 - v) < EPSILON)
      u = 0;
    else
    {
      double ratio = equator.dot(direction) / sin(lat);

      if(ratio < -1)
        ratio = -1;
      else
        if(ratio > 1)
          ratio = 1;

      double lon = acos(ratio) / (2 * PI);

      if(cross.dot(direction) < 0)
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
    return Point2D(u, v);
  }
};

class CubeProjection : public Projection
{
  static CubeProjection singleton;
  enum { LAYOUT_VERTICAL, LAYOUT_CROSS, LAYOUT_HORIZONTAL } layout;
  
  CubeProjection(bool initialization)
  {
    name = "cube";

    if(initialization)
      ProjectionFactory::registerProjection(name, this->create);
  }

  public:
  static Projection* create()
  {
    CubeProjection *p = new CubeProjection(false);
    p->layout=LAYOUT_HORIZONTAL;
    return (Projection *)p;
  }

  void setOptions(char *opts)
  {
    char *delimiter;
    static const char *OPTION_LAYOUT = "layout";

    while(*opts)
    {
      //fprintf(stderr,"option: %s\n", opts);
      //if(delimiter = strchr(name, '/'))
        //*delimiter++ = '\0';

      fprintf(stderr,"%s\n\n",opts);
      if(strncmp(opts, OPTION_LAYOUT, strlen(OPTION_LAYOUT)) == 0)
      {
	char* lstr=opts+strlen(OPTION_LAYOUT)+1;
	
        if(strncmp(lstr,"horizontal",strlen("horizontal"))==0)
	{
		this->layout=LAYOUT_HORIZONTAL;
		delimiter=lstr+strlen("horizontal");
	}
	else if(strncmp(lstr,"vertical",strlen("vertical"))==0)
	{
		this->layout=LAYOUT_VERTICAL;
		delimiter=lstr+strlen("vertical");
	}
	else if(strncmp(lstr,"cross",strlen("cross"))==0)
	{
		this->layout=LAYOUT_CROSS;
		delimiter=lstr+strlen("cross");
	}
	else
        {
          fprintf( stderr, PROG_NAME " error: cube projection: layout must be \"cross\",\"horizontal\", or \"vertical\".\n" );
          throw QuietException();
        }
      }
      else
      {
        fprintf( stderr, PROG_NAME " error: cube projection: unknown option: %s\n", opts );
        throw QuietException();
      }

      opts = delimiter+1;
    }
  }

  double getSizeRatio(void)
  {
	switch(this->layout)
	{
		case LAYOUT_CROSS:
			return 3.0/4.0;
		case LAYOUT_HORIZONTAL:
			return 6.0;
		case LAYOUT_VERTICAL:
			return 1.0/6.0;
	};
  }

  bool isValidPixel(double u, double v)
  {
	if(layout != LAYOUT_CROSS)
	{
		return true;
	}
	int u3=3.0*u;
	int v4=4.0*v;
	return (u3==2) || (v4 == 2);
  }

  Vector3D uvToDirection(double u, double v)
  {
    static const int linear_sel[6]={-1,1,-2,2,-3,3};
    int axis_sel=0;
    double fu,fv;
    double tmpi;
    switch(layout)
    {
	case LAYOUT_CROSS:
	{
		double u3f=3.0*u;
		double v4f=4.0*v;
		int u3=u3f;
		int v4=v4f;
		fu=modf(u3f,&tmpi);
		fv=modf(v4f,&tmpi);
		static const int cross_sel[4][3]=
			{{ 0, 2, 0},
			 {-1, 3, 1},
			 { 0,-2, 0},
			 { 0,-3, 0}};
		axis_sel=cross_sel[v4][u3];
		break;
	}
	case LAYOUT_HORIZONTAL:
	{
		double u6f=6.0*u;
		int u6=u6f;
		fu=modf(u6f,&tmpi);
		fv=v;
		axis_sel=linear_sel[u6];
		break;
	}
	case LAYOUT_VERTICAL:
	{
		double v6f=6.0*u;
		int v6=v6f;
		fu=u;
		fv=modf(v6f,&tmpi);
		axis_sel=linear_sel[v6];
	}
    };
    double vec[3];
    
    int ma=abs(axis_sel)-1;
    vec[ma]=axis_sel < 0 ? -1.0 : 1.0;
    double tmp;
    vec[(ma+1) % 3] = (2.0*fu-1.0)*vec[ma];
    vec[(ma+2) % 3] = (2.0*fv-1.0)*vec[ma];
    
    return Vector3D(vec[0],vec[1],vec[2]);
  }

  Point2D directionToUV(const Vector3D& direction)
  {
	double vec[3]={direction.x,direction.y,direction.z};
	double avec[3]={fabs(vec[0]),fabs(vec[1]),fabs(vec[2])};
	
	int ma= (avec[0] > avec[1]) ? 0 :
		((avec[1] > avec[2]) ? 1 : 2);
		
	
	double fu=vec[(ma+1) % 3] / vec[ma];
	double fv=vec[(ma+2) % 3] / vec[ma];
	ma+=1;
	ma=(vec[ma] < 0.0) ? -ma : ma; 
	
	double u,v;
	static const double linear_offsets[7]={4.0,5.0,2.0,3.0,0.0,1.0};
	switch(layout)
	{
		case LAYOUT_CROSS:
		{
			static const double offsets[7][2]={
				              {1.0,3.0},//-3
			                      {1.0,2.0},//-2
					      {0.0,1.0},//-1
					      {0.0,0.0},//0 NULL
					      {2.0,1.0},
					      {1.0,2.0},
					      {1.0,1.0}};
			u=(fu+offsets[ma+3][0])/3.0;
			v=(fv+offsets[ma+3][1])/4.0;
			break;
		}
		case LAYOUT_HORIZONTAL:
		{
			u=(fu+linear_offsets[ma+3])/6.0;
			v=fv;
			break;
		}
		case LAYOUT_VERTICAL:
		{
			u=fu;
			v=(fv+linear_offsets[ma+3])/6.0;
			break;
		}
	};

    return Point2D(u, v);
  }
};

PolarProjection PolarProjection::singleton = true;
CylindricalProjection CylindricalProjection::singleton = true;
AngularProjection AngularProjection::singleton = true;
MirrorBallProjection MirrorBallProjection::singleton = true;
CubeProjection CubeProjection::singleton = true;

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
            Vector3D direction = transformInfo->dstProjection->uvToDirection(
                ( x + offset + sx ) / outCols, ( y + offset + sy ) / outRows );

            // angles below are negated, because we want to rotate
            // the environment around us, not us within the environment.
            if( transformInfo->xRotate != 0 )
              direction.rotateX( -transformInfo->xRotate );

            if( transformInfo->yRotate != 0 )
              direction.rotateY( -transformInfo->yRotate );

            if( transformInfo->zRotate != 0 )
              direction.rotateZ( -transformInfo->zRotate );

            Point2D p = transformInfo->srcProjection->directionToUV( direction );

            p.x *= inCols;
            p.y *= inRows;

            if( transformInfo->interpolate == true )
            {
              int ix = (int)floor( p.x );
              int iy = (int)floor( p.y );

              double i = p.x - ix;
              double j = p.y - iy;

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
              int ix = (int)floor(p.x + 0.5);
              int iy = (int)floor(p.y + 0.5);

              if(ix >= inCols)
                ix = inCols - 1;

              if(iy >= inRows)
                iy = inRows - 1;

              pixVal += (*in)(ix, iy);
            }

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
