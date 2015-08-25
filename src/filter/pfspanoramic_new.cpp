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
	projection_size():width(-1),height(-1),latitude_frequency(-1)
	{}
};
class Projection
{
public:
	virtual Vector3D uv_to_sphere(double u, double v) const = 0;
	virtual Point2D sphere_to_uv(const Vector3D& direction) const = 0;
	
	virtual bool is_valid_pixel(double u, double v) const = 0;
	virtual projection_size get_projection_size(const projection_size& options=projection_size()) const = 0;

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
};

int panoramic()
{
	ProjectionRegistry inprojections,outprojections;
	std::vector<std::string> inoutpstring=split(pstring,"+");
}



