
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>

using namespace std;

#define WRITE_OUT_MATRIX 1
#define ANIM_POS_CORRECTION 1 // moves animation up/down to make the first at normal game car height

double const Pi = 3.1415926535897932384626;

struct RotSample
{
	double t, x, y, z, s;
};

struct PosSample
{
	double t, x, y, z;
};

#if WRITE_OUT_MATRIX

struct RotMatrix
{
	double m[3][3];
	void Mult(RotMatrix const & rhs)
	{
		double A = m[0][0];
		double B = m[0][1];
		double C = m[0][2];

		m[0][0] = A * rhs.m[0][0] + B * rhs.m[1][0] + C * rhs.m[2][0];
		m[0][1] = A * rhs.m[0][1] + B * rhs.m[1][1] + C * rhs.m[2][1];
		m[0][2] = A * rhs.m[0][2] + B * rhs.m[1][2] + C * rhs.m[2][2];

		A = m[1][0];
		B = m[1][1];
		C = m[1][2];

		m[1][0] = A * rhs.m[0][0] + B * rhs.m[1][0] + C * rhs.m[2][0];
		m[1][1] = A * rhs.m[0][1] + B * rhs.m[1][1] + C * rhs.m[2][1];
		m[1][2] = A * rhs.m[0][2] + B * rhs.m[1][2] + C * rhs.m[2][2];

		A = m[2][0];
		B = m[2][1];
		C = m[2][2];

		m[2][0] = A * rhs.m[0][0] + B * rhs.m[1][0] + C * rhs.m[2][0];
		m[2][1] = A * rhs.m[0][1] + B * rhs.m[1][1] + C * rhs.m[2][1];
		m[2][2] = A * rhs.m[0][2] + B * rhs.m[1][2] + C * rhs.m[2][2];
	}
	void DefRotateY(double a)
	{
		double const s = sin(a);
		double const c = cos(a);

		m[0][0] = c;
		m[0][1] = 0;  
		m[0][2] = s;

		m[1][0] = 0;
		m[1][1] = 1;  
		m[1][2] = 0;

		m[2][0] = -s; 
		m[2][1] = 0;  
		m[2][2] = c;
	}
	static double Component(double v0, double v1, double v2, double c, double s)
	{
		return ((v0 * v1) * (1.0 - c)) + (v2 * s);
	}
	void DefRotateV(RotSample const & v)
	{
		double const s = sin(v.s);
		double const c = cos(v.s);

		double const xx = v.x * v.x;
		double const yy = v.y * v.y;
		double const zz = v.z * v.z;

		m[0][0] = xx + (c * (1.0 - xx));
		m[0][1] = Component(v.x, v.y, v.z, c, s);
		m[0][2] = Component(v.x, v.z, -v.y, c, s);

		m[1][0] = Component(v.x, v.y, -v.z, c, s);
		m[1][1] = yy + (c * (1.0 - yy));  
		m[1][2] = Component(v.y, v.z, v.x, c, s);

		m[2][0] = Component(v.x, v.z, v.y, c, s);
		m[2][1] = Component(v.y, v.z, -v.x, c, s);  
		m[2][2] = zz + (c * (1.0 - zz));
	}
};

#endif

int Round(double x)
{
	if (x < 0)
	{
		return int(x - 0.5);
	}
	return int(x + 0.5);
}

int main(int argc, char const * const * argv)
{
	if (argc < 3)
	{
		cerr << "\n\nUsage: convertASEtoAsphalt infile outfile\n\n";
		return 0;
	}

	ifstream in(argv[1]);
	if (!in)
	{
		cerr << "\n\nERROR: Could not open file for input: " << argv[1] << "\n\n";
		return 0;
	}

	ofstream out(argv[2]);
	if (!out)
	{
		cerr << "\n\nERROR: Could not open file for output: " << argv[2] << "\n\n";
		return 0;
	}

	vector<RotSample> rotSamples;
	vector<PosSample> posSamples;

	// input the ASE
	while (true)
	{
		string s;
		if (!(in >> s))
		{
			break;
		}
		if (s == "*CONTROL_POS_SAMPLE")
		{
			PosSample r;
			in >> r.t >> r.x >> r.y >> r.z;
			posSamples.push_back(r);
		}
		else if (s == "*CONTROL_ROT_SAMPLE")
		{
			RotSample r;
			in >> r.t >> r.x >> r.y >> r.z >> r.s;
			rotSamples.push_back(r);
		}
	}

	while (posSamples.size() < rotSamples.size())
	{
		PosSample r = posSamples.back();
		posSamples.push_back(r);
	}

	while (posSamples.size() > rotSamples.size())
	{
		RotSample r = rotSamples.back();
		r.x = 1;
		r.y = r.z = r.s = 0;
		rotSamples.push_back(r);
	}


	int i;

#if WRITE_OUT_MATRIX

	// preprocess the rotations

	vector<RotMatrix> rotMatrices;

	RotMatrix rotation;
	rotation.DefRotateY(Pi / 2.0);

	for (i = 0; i < (int)rotSamples.size(); ++ i)
	{
		// apply the rotation step
		RotMatrix deltaRotation;

		// convert from Max to Asphalt
		RotSample converted;
		converted.x = -rotSamples[i].x;
		converted.y = rotSamples[i].z;
		converted.z = rotSamples[i].y;
		converted.s = rotSamples[i].s;

		deltaRotation.DefRotateV(converted);
		deltaRotation.Mult(rotation);
		rotation = deltaRotation;

		rotMatrices.push_back(rotation);
	}

#if ANIM_POS_CORRECTION
	const double kInitialAnimationPosZ = 20;
	const double posSamplesZOffset = kInitialAnimationPosZ - posSamples[0].z;
	for (i = 0; i < (int)posSamples.size(); ++ i)
	{
		posSamples[i].z += posSamplesZOffset; 
	}
#endif

	// output the anim
	out << "\t\t//// " << argv[1] << " ////\n";
	out << "\t\t{\n";
	out << "\t\t\t// frames\n";
	out << "\t\t\t" << (int)posSamples.size() << ",\n";

	for (i = 0; i < (int)posSamples.size(); ++ i)
	{
		for (int r = 0; r < 3; ++ r)
		{
			out << "\t\t\t";
			for (int c = 0; c < 3; ++ c)
			{
				out << Round(rotMatrices[i].m[c][r] * 16384) << ", ";
			}
			out << endl;
		}
		out << "\t\t\t0, 0, 0," << endl;		// last row
		out << "\t\t\t" << Round(-posSamples[i].x * 0.5) << ", " << Round(posSamples[i].z * 0.5)
			 << ", " << Round(-posSamples[i].y * 0.5) << "," << endl;
	}

	out << "\t\t},\n";

#else

	// output the anim
	out << "\t\t//// " << argv[1] << " ////\n";
	out << "\t\t{\n";
	out << "\t\t\t// frames\n";
	out << "\t\t\t" << (int)posSamples.size() << ",\n";
	out << "\t\t\t// rotation\n";
	out << "\t\t\t{\n";

	for (i = 0; i < (int)rotSamples.size(); ++ i)
	{
		out << "\t\t\t\t" << Round(rotSamples[i].x * 16384) << ", " << Round(rotSamples[i].y * 16384)
			 << ", " << Round(rotSamples[i].z * 16384) << ", " << Round(rotSamples[i].s * 1024 / Pi) << "," << endl;
	}

	out << "\t\t\t},\n";
	out << "\t\t\t// translation\n";
	out << "\t\t\t{\n";

   	for (i = 0; i < (int)posSamples.size(); ++ i)
	{
		out << "\t\t\t\t" << Round(posSamples[i].x * 256) << ", " << Round(posSamples[i].y * 256)
			 << ", " << Round(posSamples[i].z * 256) << "," << endl;
	}

	out << "\t\t\t}\n";
	out << "\t\t},\n";

#endif

	return 0;
}
