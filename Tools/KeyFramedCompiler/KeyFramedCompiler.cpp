#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <math.h>

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
struct Position
{
public:
	Position(float ix,float iy,float iz):x(ix),y(iz),z(-iy){};
	Position(){};

public:
	int x,y,z;
};


#define COS_SIN_SHIFT 14                         // shifted 2^n value for sinus/cosinus result
#define COS_SIN_MUL (1 << COS_SIN_SHIFT)         // sinus/cosinus max value (==1)



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
struct Orientation
	:public Position
{
public:
	Orientation(double ix,double iy,double iz,double angle)
	{
		double s=sin(angle);
		double c=cos(angle);
		double t=1-c;

		double heading,attitude,bank;

		const double PI = 3.1415926535897932384626433832795;

		//angle = angle * 2.0*PI;

		if ((ix*iy*t + iz*s) > 0.998) 
		{ // north pole singularity detected
			heading = 2*atan2(ix*sin(angle/2),cos(angle/2));
			attitude = PI/2;
			bank = 0;
		}
		else if ((x*y*t + z*s) < -0.998) 
		{ // south pole singularity detected
			heading = -2*atan2(ix*sin(angle/2),cos(angle/2));
			attitude = -PI/2;
			bank = 0;		
		}
		else
		{
			heading = atan2(iy * s- ix * iz * t , 1 - (iy*iy+ iz*iz ) * t);
			attitude = asin(ix * iy * t + iz * s) ;
			bank = atan2(ix * s - iy * iz * t , 1 - (ix*ix + iz*iz) * t);
		}

		y = heading/(2*PI)  * 2048.0;
		x = attitude/(2*PI) * 2048.0;
		z = bank    /(2*PI)* 2048.0;


		//std::cerr << angle << "  |  " << x << " " << y << " " << z <<"\n";
	}
};


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
template<class T>
void Save(FILE* out,const std::vector<T>& v)
{
	const int nb = v.size();
	fwrite(&nb,1,sizeof(nb),out);
	if(nb)
		fwrite(&v[0],sizeof(T),nb,out);
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
int main(int argc,char** argv)
{
	std::string fileName="anim.ase";

	char buffer[1024];

	if(argc < 2)
	{
		std::cerr << "Bad # Arguments\n";
		//return 0;
	}
	else
	{
		fileName = argv[1];
	}

	typedef std::vector<Position> Positions;
	typedef std::vector<Orientation> Orientations;

	Positions		positions;
	Orientations	orientations;

	std::ifstream in(fileName.c_str());

	float angle = 0.0;

	while(in.good())
	{
		in.getline(buffer,1024);
		
		char buffer2[256];

		int w;
		float x,y,z,a;

		sscanf(buffer,"%s",buffer2);
		if(strcmp(buffer2,"*CONTROL_ROT_SAMPLE")==0)
		{
			if(sscanf(buffer,"%s %d %f %f %f %f",buffer2,&w,&x,&y,&z,&a)==6)
			{
				angle += a;
				orientations.push_back( Orientation(x,z,-y,angle) );
			}
		}
		else if(strcmp(buffer2,"*CONTROL_POS_SAMPLE")==0)
		{
			if(sscanf(buffer,"%s %d %f %f %f",buffer2,&w,&x,&y,&z)==5)
				positions.push_back( Position(x,y,z) );
		}
	}

	if(argc > 2)
	{
		FILE* out = fopen(argv[2],"wb");
		if(out)
		{
			Save(out,positions);
			Save(out,orientations);			
			fclose(out);
		}
	}
	return 0;
}

