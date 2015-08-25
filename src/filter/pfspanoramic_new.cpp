#include<string>
#include<iostream>
#include<algorithm>
#include<cstring>
#include<map>
#include<vector>

class Vector3D
{
public:
	double data[3];
	double& x;
	double& y;
	double& z;
	Vector3D(const double tx=0.0,const double ty=0.0,const double tz=0.0):
		data({tx,ty,tz}),
		x(data[0]),y(data[1]),z(data[2])
	{}
	double magnitude() const
	{
		return sqrt(dot(*this));
	}
	void normalize()
	{
		operator*=(1.0/magnitude());
	}
	Vector3D& operator*=(const double scale)
	{
		x*=scale;y*=scale;z*=scale;
		return *this;
	}
	double dot(const Vector3D& o) const
	{
		return data[0]*o.data[0]+data[1]*o.data[1]+data[2]*o.data[2];
	}
};

class Point2D
{
public:
	double x, y;

	Point2D(const double tx,const double ty):x(tx),y(ty)
	{}
};

struct projection_size
{
	int width;
	int height;
	int latitude_frequency;
	projection_size(int w=-1,int h=-1,int l=-1):width(w),height(h),latitude_frequency(l)
	{}
};
class Projection
{
public:
	virtual Vector3D uv_to_sphere(double u, double v) const = 0;
	virtual Point2D sphere_to_uv(const Vector3D& direction) const = 0;
	
	virtual bool is_valid_pixel(double u, double v) const = 0;
	virtual projection_size get_projection_size(const projection_size& options=projection_size()) const = 0;
	virtual std::string guess_valid_string(int w,int h) const = 0;

	virtual ~Projection()
	{}

	virtual void setOptions(const std::map<std::string,std::string>& options)
	{}
};

vector<std::string> split(const std::string& s,const std::string& delimiters)
{
	std::vector<std::string> result;
	size_t current;
	size_t next = -1;
	do
	{
		current = next + 1;
		next = s.find_first_of( delimiters, current );
		result.push_back( s.substr( current, next - current ) );
	}
	while (next != std::string::npos);
	return result;
}

class ProjectionRegistry
{
public:
	map<std::string,Projection*> projections;
private:
	template<class T>
	void register_projection(const std::string& name)
	{
		projections[name]=new T;
	}
public:
	ProjectionRegistry()
	{
		register_projection<MirrorBallProjection>("mirrorball");
		register_projection<AngularProjection>("angular");
		register_projection<PolarProjection>("polar");
		register_projection<PolarProjection>("latlong");
		register_projection<CubeProjection>("cube");
		register_projection<CylindricalProjection>("cylindrical");
	}
	~ProjectionRegistry()
	{
		for(std::map<std::string>::iterator i=projections_in.begin(),i!=projections_in.end();++i)
		{
			delete i->second;
		}
	}
	Projection* parse_projstring(const std::string& projstring)
	{
		std::vector<std::string> slashiesplit=split(projstring,"/");
		
		string name=slashiesplit[0]
		std::transform(name.begin(),name.end(),name.begin(),std::tolower);
		std::map<std::string,Projection*>::iterator found=projections.find(name);
		if(found == projections.end())
		{
			throw std::runtime_error(std::string("No projection with the name \"")+name+"\" found.");
		}
		std::map<std::string,std::string> options;
		for(size_t i=1;i<slashiesplit.size();i++)
		{
			std::vector<std::string> nvpairs=split(projstring,"=");
			if(nvpairs.size()==1)
			{
				options[nvpairs[0]]="";
			}
			else
			{
				options[nvpairs[0]]=nvpairs[1];
			}
		}
		found->second->setOptions(options);
		return found->second;
	}
	std::string guess_projstring(int x,int y)
	{
		if(x <= 0 || y <= 0)
		{
			throw std::runtime_error("No projection can be determined automatically from the given size parameters.  Please specify a projection or size parameters");
		}
		std::string guess="";
		for(map<std::string,Projection*>::iterator it=projections.begin();it!=projections.end();++it)
		{
			std::string cguess=it->second->guess_projstring(x,y);
			if(cguess != "")
			{
				if(guess=="")
				{
					guess=cguess;
				}
				else
				{
					throw std::runtime_error(std::string("Cannot determine projection automatically from size parameters..given parameters could be ")+cguess+" or "+guess);
				}
			}
		}
		if(guess=="")
		{
			throw std::runtime_error("No projection could be determined automatically from the given size parameters.  Please specify a projection.");
		}
	}
};

static const std::string PROG_NAME="pfspanoramic";
void printHelp(const std::string& arg0)
{
  cerr << arg0 << " <source projection>+<target projection> [--width <val>] [--height <val>] [--oversample <val>] [--interpolate] [--xrotate <angle>] [--yrotate <angle>] [--zrotate <angle>] [--verbose] [--help]\n" 
	<< "See man page for more information.\n";
}
struct cmdargs
{
	std::string pstring="";
	projection_size size;
	bool verbose=false;
	Vector3D rotation;
	bool interpolate=false;
	int oversample=1;
	
	cmdargs(const std::vector<std::string>& args):
		pstring(""),
		verbose(false),
		interpolate(false),
		oversample(false)
	{
		for(int i=1;i<args.size();i++)
		{
			if(args[i][0]!='-')
			{
				pstring=args[i];
			}
			else if(args[i]=='--help')
			{
				printHelp(args[0]);
				return 0;
			}
			else
			{
				char ch=args[i][1]=='-' ? args[i][2] : args[i][1];
				switch(ch)
				{
				case 'w':
				{
					std::istringstream(args[++i]) >> size.width;
					break;
				}
				case 'h':
				{
					std::istringstream(args[++i]) >> size.height;
					break;
				}
				case 'x':
				{
					std::istringstream(args[++i]) >> rotation.x;
					break;
				}
				case 'y':
				{
					std::istringstream(args[++i]) >> rotation.y;
					break;
				}
				case 'z':
				{
					std::istringstream(args[++i]) >> rotation.z;
					break;
				}
				case 'i':
				{
					interpolate=true;
					break;
				}
				case 'o':
				{
					std::istringstream(args[++i]) >> oversample;
					break;
				}
				default:
				{
					printHelp(args[0]);
					throw std::runtime_error("Invalid arguments");
				}
				};
			}
		}
		if(pstring == "")
		{
			throw std::runtime_error("No projection string detected!");
		}
		if(oversample < 1)
		{
			throw std::runtime_error("Oversampling must be an integer greater than 10");
		}
	}
};

Projection* replace_guess(Projection* proj,const std::string pstr,ProjectionRegistry& reg,int width,int height)
{
	if(proj!=NULL)
		return proj;
	
	if(pstr=="")
	{
		pstr=reg.guess_projstring(width,height);
	}
	return inprojections.parse_projstring(pstr);
}

int panoramic(const std::vector<std::string>& args)
{
	cmdargs argsin(args);
	ProjectionRegistry inprojections,outprojections;
	
	pfs::DOMIO pfsio;
	
	
	std::vector<std::string> inoutpstring=split(pstring,"+");
	
	Projection* inproj=NULL;
	Projection* outproj=NULL;
	
	pfs::Frame *inframe;
	while(inframe=pfsio.readFrame( stdin ))
	{
		inproj=replace_guess(inproj,inoutpstring[0],inprojections,inframe->getWidth(),inframe->getHeight());
		projection_size ips=inproj->get_projection_size(projection_size(inframe->getWidth(),inframe->getHeight()));
		outproj=replace_guess(outproj,inoutpstring[1],outprojections,argsin.size.x,argin.size.y);
		
		projection_size ops;
		if(argin.size.x <=0 || argin.size.y <=0)
		{
			ops=outproj->get_projection_size(ips);
		}
		else
		{
			ops=argin.size;
		}
	}
}



