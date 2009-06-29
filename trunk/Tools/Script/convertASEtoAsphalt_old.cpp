
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>

using namespace std;

struct RotSample
{
	double t, x, y, z, s;
};

struct PosSample
{
	double t, x, y, z;
};

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

	// [FIXME] process the rots

	// output the anim
	out << "\t\t//// " << argv[1] << " ////\n";
	out << "\t\t{\n";
	out << "\t\t\t// frames\n";
	out << "\t\t\t" << (int)posSamples.size() << ",\n";
	out << "\t\t\t// rotation\n";
	out << "\t\t\t{\n";

	// [FIXME] output by structs instead of by arrays

	for (int i = 0; i < (int)rotSamples.size(); ++ i)
	{
		out << "\t\t\t\t" << Round(rotSamples[i].x * 16384) << ", " << Round(rotSamples[i].y * 16384)
			 << ", " << Round(rotSamples[i].z * 16384) << ", " << Round(rotSamples[i].s * 1024 / 3.14159265358979) << "," << endl;
	}

	out << "\t\t\t},\n";
	out << "\t\t\t// translation\n";
	out << "\t\t\t{\n";

   	for (int j = 0; j < (int)posSamples.size(); ++ j)
	{
		out << "\t\t\t\t" << Round(posSamples[j].x * 256) << ", " << Round(posSamples[j].y * 256)
			 << ", " << Round(posSamples[j].z * 256) << "," << endl;
	}

	out << "\t\t\t}\n";
	out << "\t\t},\n";

	return 0;
}
