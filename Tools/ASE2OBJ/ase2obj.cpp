#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

struct Vertex
{
	double pos[3];
};

struct TextureVertex
{
	double uvw[3];
};

struct PolygonVertex
{
	int vertexID;
	int texturevertexID;
};

struct Polygon
{
	vector<PolygonVertex> polygonVertex;
	int materialID;
};

struct Object
{
	vector<Vertex> vertex;
	vector<TextureVertex> textureVertex;
	vector<Polygon> polygon;
	bool readASE(istream & in)
	{
		while (true)
		{
			string s;
			if (!(in >> s)) break;
			if (s == "*MESH_VERTEX")
			{
				int v;
				if (!(in >> v)) return false;
				if (vertex.size() <= v)
				{
					vertex.resize(v + 1);
				}
				if (!(in >> vertex[v].pos[0] >> vertex[v].pos[1] >> vertex[v].pos[2])) return false;
				continue;
			}
			if (s == "*MESH_FACE")
			{
				int p;
				if (!(in >> p)) return false;
				if (polygon.size() <= p)
				{
					polygon.resize(p + 1);
				}
				polygon[p].polygonVertex.resize(3);
				while (true)
				{
					if (!(in >> s)) return false;
					if (s != "A:") continue;
					if (!(in >> polygon[p].polygonVertex[0].vertexID)) return false;
					break;
				}
				while (true)
				{
					if (!(in >> s)) return false;
					if (s != "B:") continue;
					if (!(in >> polygon[p].polygonVertex[1].vertexID)) return false;
					break;
				}
				while (true)
				{
					if (!(in >> s)) return false;
					if (s != "C:") continue;
					if (!(in >> polygon[p].polygonVertex[2].vertexID)) return false;
					break;
				}
				while (true)
				{
					if (!(in >> s)) return false;
					if (s != "*MESH_MTLID") continue;
					if (!(in >> polygon[p].materialID)) return false;
					break;
				}
				continue;
			}
			if (s == "*MESH_TVERT")
			{
				int v;
				if (!(in >> v)) return false;
				if (textureVertex.size() <= v)
				{
					textureVertex.resize(v + 1);
				}
				if (!(in >> textureVertex[v].uvw[0] >> textureVertex[v].uvw[1] >> textureVertex[v].uvw[2])) return false;
				continue;
			}
			if (s == "*MESH_TFACE")
			{
				int p;
				if (!(in >> p)) return false;
				if (polygon.size() <= p)
				{
					polygon.resize(p + 1);
				}
				polygon[p].polygonVertex.resize(3);
				if (!(in >> polygon[p].polygonVertex[0].texturevertexID >> polygon[p].polygonVertex[1].texturevertexID >> polygon[p].polygonVertex[2].texturevertexID)) return false;
				continue;
			}
		}
		return true;
	}
	bool writeOBJ(ostream & out)
	{
		out << "# Max2Obj Version 4.0 Mar 10th, 2001" << endl;
		out << "#" << endl;
		out << "# object Object to come ..." << endl;
		out << "#" << endl;
		out.precision(10);
		for (int v = 0; v < vertex.size(); ++ v)
		{
			out << "v  " << vertex[v].pos[0] << " " << vertex[v].pos[2] << " " << -vertex[v].pos[1] << endl;
		}
		out << "# " << vertex.size() << " vertices" << endl << endl;
		for (int tv = 0; tv < textureVertex.size(); ++ tv)
		{
			out << "vt  " << textureVertex[tv].uvw[0] << " " << textureVertex[tv].uvw[1] << " " << textureVertex[tv].uvw[2] << endl;
		}
		out << "# " << textureVertex.size() << " texture vertices" << endl << endl;
		out << "g Object" << endl;
		for (int p = 0; p < polygon.size(); ++ p)
		{
			if (!p || polygon[p].materialID != polygon[p - 1].materialID)
			{
				out << "s " << polygon[p].materialID << endl;
			}
			out << "f ";
			for (int i = 0; i < polygon[p].polygonVertex.size(); ++ i)
			{
				out << polygon[p].polygonVertex[i].vertexID - (int)vertex.size() << "/" << polygon[p].polygonVertex[i].texturevertexID - (int)textureVertex.size() << " ";
			}
			out << endl;
		}
		out << "# " << polygon.size() << " faces" << endl << endl;
		out << "g" << endl;
		return true;
	}
};

int main(int argc, char const * const * const argv)
{
	if (argc < 3)
	{
		cout << "\n\nUsage:\nASE2OBJ infile.ase outfile.obj" << endl << endl;
		return 1;
	}

	ifstream in(argv[1]);
	if (!in)
	{
		cout << "\nCould not open file for reading: " << argv[1] << endl << endl;
		return 1;
	}

	ofstream out(argv[2]);
	if (!out)
	{
		cout << "\nCould not open file for writing: " << argv[2] << endl << endl;
		return 1;
	}

	Object o;
	if (!o.readASE(in))
	{
		cout << "\nError reading ASE file: " << argv[1] << endl << endl;
		return 1;
	}
	if (!o.writeOBJ(out))
	{
		cout << "\nError writing OBJ file: " << argv[2] << endl << endl;
		return 1;
	}

	return 0;
}
