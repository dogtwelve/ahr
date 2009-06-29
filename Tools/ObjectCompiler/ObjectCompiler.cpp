//#include "../../Src/DisableStlWarnings.h"

#include "parser.h"
#include "Exception.h"
#include "misc.h"


#include <stdio.h>
#include <math.h>

#include <string>
#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream>

#include <limits.h>
#include <assert.h>


static const std::string kGeomObjevtBlock	= "*GEOMOBJECT";
static const std::string kNodeName			= "*NODE_NAME";
static const std::string kMesh				= "*MESH";
static const std::string kMeshVertexList	= "*MESH_VERTEX_LIST";
static const std::string kMeshVertex		= "*MESH_VERTEX";
static const std::string kMeshFaceList		= "*MESH_FACE_LIST";
static const std::string kMeshFace			= "*MESH_FACE";
static const std::string kMeshTVertexList	= "*MESH_TVERTLIST";
static const std::string kMeshTVertex		= "*MESH_TVERT";
static const std::string kMeshTFaceList		= "*MESH_TFACELIST";
static const std::string kMeshTFace			= "*MESH_TFACE";
static const char*		 kMeshMTLId			= "*MESH_MTLID";

const int				kBlockSizeShift		= 7;
const unsigned long		kMultiBlock			= 0x00FFFFFF;

const int				kNormalShift	= 12;// For normalization

std::ofstream			gTrace;



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class ReportFile
{
public:
	ReportFile(const std::string& name):m_fileName(name){};


	ReportFile& operator<<(const char* str)	{m_strings.push_back(std::string(str)); return *this;}
	ReportFile& operator<<(const std::string& str)	{m_strings.push_back(str); return *this;}

	void Dump()
	{
		std::ofstream f(m_fileName.c_str(), std::ios_base::app);
		if(f)
		{
			for(Strings::const_iterator i = m_strings.begin();i!=m_strings.end();i++)
				f << *i;
		}
		m_strings.clear();
	}

private:
	typedef std::vector<std::string> Strings;

	const std::string	m_fileName;
	Strings				m_strings;
};



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
bool AlmostEqual(float x,float y)
{
	const float kEpsilon = 0.01f;

	return ::fabs(x-y) < kEpsilon;
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class CBlockId
{

public:
	CBlockId()	{m_long=kMultiBlock;}
	CBlockId(const CBlockId& b):m_long(b.m_long){};
	

	void Set(int i,int i_v)
	{
		const int v = i_v >> kBlockSizeShift;

		assert(i>=0 && i<3);
		//assert(v>= CHAR_MIN && v<= CHAR_MAX);
		if (v < CHAR_MIN)
			m_data[i] = (signed char)CHAR_MIN;
		else if (v > CHAR_MAX)
			m_data[i] = (signed char)CHAR_MAX;
		else m_data[i] = (signed char)v;
	}

	bool operator<(const CBlockId& b) const {return m_long<b.m_long;}
	bool operator==(const CBlockId& b)	const{return m_long==b.m_long;}
	void operator=(const CBlockId& b)	{m_long=b.m_long;}


	void Save(OutFile& out) const	
	{
		gTrace << " "<< int(m_data[0]) << " " << int(m_data[1]) << " " << int(m_data[2]) << "\n";
		out.WriteChar(m_data[0]);
		out.WriteChar(m_data[2]);
		out.WriteChar(-m_data[1]);
		out.WriteChar(m_data[4]);
	}

	static const CBlockId& NulBlock() {static CBlockId b;b.m_long = 0; return b;}


	void Debug() const 
	{
		if(gTrace)
		{
			gTrace << "[";
			for(int i=0;i<3;i++)
				gTrace << int(m_data[i]) << " ";
			gTrace << "]";
		}
	}

private:
	union
	{
		unsigned long	m_long;
		signed char		m_data[4];
	};
};




// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class TFace
{
public:
	TFace(const TFace& f)
	{
		for(int i=0;i<3;i++)
			m_indexes[i] = f.m_indexes[i];
	}

	TFace(Parser& p)
	{
		m_faceIndex  = p.ReadInt();
		m_indexes[0] = p.ReadInt();
		m_indexes[1] = p.ReadInt();
		m_indexes[2] = p.ReadInt();
	}
	static const std::string& BlockName() {return kMeshTFace;}

	int			FaceIndex() const {return m_faceIndex;}
	const int*	Index() const {return m_indexes;}

private:
	int	m_faceIndex;
	int m_indexes[3];	
};
// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class TVertex
{
public:
	TVertex(){};
	TVertex(const TVertex& t)	{m_u=t.m_u;m_v=t.m_v;}
	TVertex(Parser& p)
	{
		p.ReadInt();	//index
		const float u = p.ReadFloat();
		const float v = p.ReadFloat();

		m_u = int((float(1<<12)*u) + 0.5);
		m_v = int((float(1<<12)*v) + 0.5);

		p.SkipBlock();
	}

	void Save(OutFile& out) const	
	{
		out.WriteLong(m_u);
		out.WriteLong(m_v);
	}

	static const std::string& BlockName() {return kMeshTVertex;}
private:
	int m_u;
	int m_v;
};





// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class Vector
{
public:
	Vector(){};
	Vector(const Vector& v){operator=(v);}
	Vector(float x,float y,float z){m_val[0]=x;m_val[1]=y;m_val[2]=z;}
	

	Vector& operator=(const Vector& v){m_val[0]=v.m_val[0];m_val[1]=v.m_val[1];m_val[2]=v.m_val[2];return *this;}
	

	Vector CrossProduct(const Vector& v) const
	{
		return Vector(	 (Y() * v.Z()) - (Z() * v.Y()),
						 (Z() * v.X()) - (X() * v.Z()),
						 (X() * v.Y()) - (Y() * v.X()));
	}

	float Length2() const {return X()*X() + Y() * Y() + Z()*Z();}
	float Length() const {return float( ::sqrt(Length2()));}

	float operator[](int i) const {return m_val[i];}
	void Set(int i,float f) {m_val[i]=f;}

	
	Vector operator+(const Vector& v) const {return Vector(m_val[0]+v.m_val[0],m_val[1]+v.m_val[1],m_val[2]+v.m_val[2]);}
	Vector operator-(const Vector& v) const {return Vector(m_val[0]-v.m_val[0],m_val[1]-v.m_val[1],m_val[2]-v.m_val[2]);}
	Vector operator-() const {return *this * -1.0;}
	Vector operator/(float f) const {if(f==0.0) f=1.0; return Vector(X()/f,Y()/f,Z()/f);}
	Vector operator*(float f) const {if(f==0.0) f=1.0; return Vector(X()*f,Y()*f,Z()*f);}



	float X() const{return m_val[0];}
	float Y() const{return m_val[1];}
	float Z() const{return m_val[2];}

	CBlockId	CalcBlockId() const 
	{
		CBlockId id;
		for(int i=0;i<3;i++)
		{
			const int v = int(m_val[i]);
			id.Set(i,v);
		}
		return id;
	}

	void Debug() const 	{gTrace << "[" << m_val[0] << ";" << m_val[1] << ";" << m_val[2] << ']';}


private:
	float m_val[3];
};









// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class Vertex
	:public Vector
{
	typedef Vector inherited;
public:
	Vertex(Parser& parser)
	{
		m_index	= parser.ReadInt();
		for(int i=0;i<3;i++)
		{
			const float f = parser.ReadFloat();
			inherited::Set(i,f);			
			m_valInt[i] = int(f);
		}
		m_blockId = CalcBlockId();
	}
	Vertex(const Vertex& v)	{operator=(v);}


	void operator=(const Vertex& v)
	{
		inherited::operator=(v);

		m_index		= v.m_index;
		m_blockId	= v.m_blockId;
		for(int i=0;i<3;i++)		
			m_valInt[i] = v.m_valInt[i];		
		
	}

	static const std::string& BlockName() {return kMeshVertex;}

	void Save(OutFile& out) const
	{		
		out.WriteLong(m_valInt[0]);
		out.WriteLong(m_valInt[2]);
		out.WriteLong(-m_valInt[1]);
	}

	CBlockId BlockId() const {return m_blockId;}

	static const char* ClassName() {return "Vertex";}

	void Debug() const 
	{
		gTrace << m_valInt[0] << ";" << m_valInt[1] << ";" << m_valInt[2] << "  B: ";
		m_blockId.Debug();
	}

	int GetInt(int i) const {return m_valInt[i];}

private:
	int				m_index;	
	int				m_valInt[3];
	CBlockId		m_blockId;
};

typedef std::vector<Vertex>		Vertexes;


class VertexCollection;

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class Face
{
public:
	Face(const Face& f)
	{
		m_index		= f.m_index;
		m_blockId	= f.m_blockId;
		m_meshIndex	= f.m_meshIndex;
		for(int i=0;i<3;i++)
		{
			m_vindex[i]	= f.m_vindex[i];
			m_tvertex[i]= f.m_tvertex[i];
			m_vertex[i] = f.m_vertex[i];
		}
		m_normal		= f.m_normal;
		m_doubleSided	= f.m_doubleSided;
		m_materialId	= f.m_materialId;
	}

	Face(Parser& p)
	{	
		p.ReadToken();
		const char* c = p.Get();
	
		if(sscanf(c,"%d:",&m_index)!=1)
			p.Throw("Face Index Invalid:",p.Get());
		
		p.ReadToken("A:");
		m_vindex[0] = p.ReadInt();
		p.ReadToken("B:");
		m_vindex[1] = p.ReadInt();
		p.ReadToken("C:");
		m_vindex[2] = p.ReadInt();

		do
		{
			p.ReadToken();
		} while (::strcmp(p.Get(), kMeshMTLId) != 0);
		
		m_materialId = p.ReadInt();

		//p.SkipBlock();
	}

	void Init(Vertexes& vertexes,int meshId,bool doubleSided)
	{
		m_meshIndex = meshId;
		for(int i=0;i<3;i++)
			m_vertex[i] = &vertexes[m_vindex[i]];

		CalcNormal();


		const int plan = EdgePlan();


		Vector offset(0,0,0);

		if(plan!=-1)
		{
			offset = -m_normal * 2;

			if(gTrace)
			{
				gTrace << "Face " << m_index <<" is on PLAN " << plan << "\n";

				gTrace << "\t";m_vertex[0]->Debug();gTrace << "\n";
				gTrace << "\t";m_vertex[1]->Debug();gTrace << "\n";
				gTrace << "\t";m_vertex[2]->Debug();gTrace << "\n";

				gTrace << "\t";m_normal.Debug(); gTrace << "\n";
			}
		}
		
		{
			const Vector v = ((*m_vertex[0] + *m_vertex[1] + *m_vertex[2]) / 3.0) + offset;

			m_blockId = v.CalcBlockId();

			if(	gTrace)
			{
				/*
				gTrace << "\t Index: " << m_index << "\n";

				gTrace << "\t";m_vertex[0]->Debug();gTrace << "\n";
				gTrace << "\t";m_vertex[1]->Debug();gTrace << "\n";
				gTrace << "\t";m_vertex[2]->Debug();gTrace << "\n";

				gTrace << "\t";m_normal.Debug(); gTrace << "\n";

				gTrace << "res: "; m_blockId.Debug();
				gTrace<< "\n";
				*/
			}
		}

		m_doubleSided = doubleSided;
		//m_materialId = 0;
	}

	void			SetTvertex(int i,const TVertex& tv)	{	m_tvertex[i] = tv;}
	const Vertex*	GetVertex(int i) const {assert(i>=0&&i<3);return m_vertex[i];}

	static const std::string&	BlockName() {return kMeshFace;}

	void			Save(OutFile& out,const VertexCollection& vc) const;
	
	CBlockId		BlockId() const { return m_blockId;}

	bool			operator<(const Face& f) const{return m_blockId < f.m_blockId;}

	int				GetMeshId() const { return m_meshIndex;}

	bool			DoubleSided() const {return m_doubleSided;}


	int 			EdgePlan() const
	{
		for(int i=0;i<3;i++)
		{
			if(		AlmostEqual(V(0)[i] , V(1)[i])  && 
					AlmostEqual(V(0)[i] , V(2)[i]))
			{
				const float v = V(0)[i];

				const int valInt = int(::floor((v/128.0)+0.5) * 128);

				const float dif = ::fabs(v-valInt);
				
				if(dif < 2)
					return i;
			}
		}
		return -1;
	}


	const Vector&	V(int i) const {return *m_vertex[i];}


private:
	void CalcNormal() 
	{
		const Vector v1 = (*m_vertex[1]) - (*m_vertex[0]);
		const Vector v2 = (*m_vertex[2]) - (*m_vertex[0]);
		const Vector cross = v1.CrossProduct(v2);

		m_normal = cross / cross.Length();
	}

private:
	int				m_index;

	int				m_vindex[3];
	TVertex			m_tvertex[3];
	const Vertex*	m_vertex[3];	
	CBlockId		m_blockId;
	int				m_meshIndex;
	Vector			m_normal;
	bool			m_doubleSided;
	unsigned char	m_materialId;
};





typedef std::vector<const Face*>	FacesPtr;

class FaceSortByMesh
{
public:
	bool operator()(const FacesPtr::value_type& face1,
					const FacesPtr::value_type& face2) const
	{
		return face1->GetMeshId() < face2->GetMeshId();
	}
};




// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class VertexCollection
{
public:
	VertexCollection(const FacesPtr& faces)
	{
		for(FacesPtr::const_iterator i = faces.begin();i!=faces.end();i++)
		{
			for(int j=0;j<3;j++)
				AddIndex((*i)->GetVertex(j));
		}
	}

	void AddIndex(const Vertex* v)
	{
		Map::const_iterator i = m_map.find(v);
		if(i==m_map.end())
		{
			int newIndex = m_map.size();
			m_map.insert( Map::value_type(v,newIndex));			
		}		
	}

	int GetIndex(const Vertex* v) const
	{
		Map::const_iterator i = m_map.find(v);
		assert(i!=m_map.end());
		return i->second;		
	}

	void Save(OutFile& out) const
	{
		std::vector<const Vertex*> v(m_map.size());
		for(Map::const_iterator i=m_map.begin();i!=m_map.end();i++)
		{
			assert(v[i->second]==0);
			v[i->second] = i->first;
		}
		out.WriteListPtr(v);	
	}

	static const char* ClassName() {return "VertexCollection";}

private:
	typedef std::map<const Vertex*,int> Map;
	Map	m_map;	
};


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void Face::Save(OutFile& out,const VertexCollection& vc) const	
{
	int i;

	for(i=0;i<3;i++)
	{
		int index = vc.GetIndex(m_vertex[i]);
		out.WriteLong(index);

		m_tvertex[i].Save(out);
	}
	
	const float multiplier = float(1<<kNormalShift);

	const Vector normal = m_normal * multiplier;

	out.WriteLong(normal[0]);
	out.WriteLong(normal[2]);
	out.WriteLong(-normal[1]);

	out.WriteChar(m_materialId);
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class TextureClass
	:public FacesPtr
{
public:

	TextureClass(const TextureClass& t)
		:FacesPtr(t),
		m_meshId(t.m_meshId),
		m_doubleFace(t.m_doubleFace),
		m_vertexes(t.m_vertexes)
	{};

	TextureClass(int meshId,const VertexCollection*	vertexes,bool doubleSided)
		:m_meshId(meshId),
		m_doubleFace(doubleSided),
		m_vertexes(vertexes)
	{};

	void Save(OutFile& out) const
	{
		out.WriteLong(m_meshId);
		out.WriteLong(m_doubleFace ? 1:0);

		out.WriteLong(size());

		gTrace << "Faces " << size() << "\n";

		for(const_iterator i = begin();i!=end();i++)
			(*i)->Save(out,*m_vertexes);
	}

	void Debug() const
	{

	};

	static const char* ClassName() {return "TextureClass";}

private:
	int						m_meshId;
	bool					m_doubleFace;
	const VertexCollection*	m_vertexes;

};


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class Cubes
	:public std::map<CBlockId,FacesPtr>
{
public:

	static const char* ClassName() {return "Cubes";}

	void Save(OutFile& out) const
	{	
		gTrace << "Cubes " << size() << "\n";

		out.WriteLong(size());
		for(const_iterator i = begin();i!=end();i++)
		{
			gTrace << "Cube";
			// Save a cube/GeneralMesh
			i->first.Save(out);	// block Id

			// Vertex List
			const VertexCollection vertexes(i->second);
			vertexes.Save(out);


			// Sub-Mesh List
			// Sort all faces by mesh:
			FacesPtr faces = i->second;
			std::sort(faces.begin(),faces.end(),FaceSortByMesh());

			std::vector<TextureClass> textureClasses;
			textureClasses.reserve(10);
			
			int meshId = -1;faces.front()->GetMeshId();
			for(FacesPtr::iterator f = faces.begin();f!=faces.end();f++)
			{
				if(meshId != (*f)->GetMeshId())
				{
					meshId = (*f)->GetMeshId();
					textureClasses.push_back(TextureClass(meshId,&vertexes,(*f)->DoubleSided()));
				}
				textureClasses.back().push_back(*f);
			}
			// Save the texture classes
			out.WriteList(textureClasses);

			gTrace << "End Cube\n";
		}

	}
};

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class Mesh
{
	Mesh();
public:
	Mesh(int i):m_meshId(i),m_doubleSided(false){};

	void SetName(const char* n)
	{
		m_name = n;::ToLower(m_name);::StripQuotes(m_name);
		if(m_name.length()>3)
		{
			if(m_name.substr(m_name.length()-3)=="_ds")
				m_doubleSided=true;
		}
	}

	const std::string& GetName() const {return m_name;}

	void Add(const TVertex& v)	{m_tvertex.push_back(v);}
	void Add(const Vertex& v)	{m_vertex.push_back(v);}

	void Add(const Face& f)
	{
		m_faces.push_back(f);
		m_faces.back().Init(m_vertex,m_meshId,m_doubleSided);		
	}

	void Add(const TFace& f)
	{
		int faceIndex = f.FaceIndex();
		if(faceIndex>=0 && faceIndex < m_faces.size())
		{
			const int* tindex = f.Index();
			for(int i=0;i<3;i++)
			{
				const int index = tindex[i];
				if(index>=0 && index < m_tvertex.size())
				{
					Face& face = m_faces[faceIndex];
					face.SetTvertex(i,m_tvertex[index]);
				}
				else
					throw Exception("Bad TVertex Index");	
			}
		}
		else
			throw Exception("Bad Face Index");	
	}

	void Fill(Cubes& cubes,bool monoObject) const
	{
		if(monoObject)
		{
			const CBlockId nullBlock = CBlockId::NulBlock();

			for(Faces::const_iterator i = m_faces.begin();i!=m_faces.end();i++)
				cubes[nullBlock].push_back( &(*i));
		}
		else
		{
			for(Faces::const_iterator i = m_faces.begin();i!=m_faces.end();i++)
				cubes[i->BlockId()].push_back( &(*i));
		}
	}


	

	void Debug(){};

	static const char* ClassName() {return "Mesh";}

private:
	typedef std::vector<Face>			Faces;	
	typedef std::vector<TVertex>		TVertexes;

	Faces			m_faces;
	TVertexes		m_tvertex;
	Vertexes		m_vertex;	
	std::string		m_name;
	const int		m_meshId;
	bool			m_doubleSided;
};


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class ASEParser
	:public Parser
{
public:
	ASEParser(const char* fileName):Parser(fileName){};

	template<class T>
	void ReadList(Mesh& mesh,T* dummy)
	{
		ReadToken(TOpenBlock);
		while(ReadToken() != TCloseBlock)
		{
			if(operator!=(T::BlockName()))
				Throw("Expected:" ,T::BlockName().c_str());

			mesh.Add(T(*this));			
		}
	}

	void ReadMesh(Mesh& mesh)
	{
		while(ReadToken()!=TCloseBlock)
		{
			if(IsBlock(kMeshVertexList))
				ReadList(mesh,(Vertex*)0);
			else if(IsBlock(kMeshFaceList))
				ReadList(mesh,(Face*)0);
			else if(IsBlock(kMeshTVertexList))
				ReadList(mesh,(TVertex*)0);
			else if(IsBlock(kMeshTFaceList))
				ReadList(mesh,(TFace*)0);
			else
				SkipBlock();
		}
	}

	Mesh* ReadObject(int meshIndex)
	{
		std::auto_ptr<Mesh>	mesh(new Mesh(meshIndex));

		ReadToken(TOpenBlock);
		while(ReadToken()!=TCloseBlock)
		{
			if(IsBlock(kNodeName))
			{
				ReadToken();
				mesh->SetName(Get());
			}
			else if(IsBlock(kMesh))
			{
				ReadToken(TOpenBlock);
				ReadMesh(*mesh);
			}
			else
				SkipBlock();
		}		
		return mesh.release();
	}
};









// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class MeshCollection
	:public std::vector<Mesh*>
{
public:
	MeshCollection(bool m):m_monoObject(m ){};
	~MeshCollection()
	{
		for(iterator i=begin();i!=end();i++)
			delete *i;
		clear();
	}

	void Save(OutFile& out) const
	{	
		Cubes cubes;

		for(const_iterator i = begin();i!=end();i++)
			(*i)->Fill(cubes,m_monoObject);


		cubes.Save(out);
	}
private:
	const bool m_monoObject;
};



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
int main(int argc,const char** argv)
{
	try
	{
		std::string fileName		= "D:\\sources\\SplinterCell3\\data\\meshes\\ASE\\office_desk.ase";
		std::string outDirName		= "d:";
		std::string repportFileName = "d:\\mesh.txt";

		bool monoObject		 = false;
		bool debug			= false;
	
		if(argc > 3)
		{
			fileName		= argv[1];
			outDirName		= argv[2];			
			repportFileName	= argv[3];
			
			for(int i=4;i<argc;i++)
			{
				const char* c= argv[i];

				if( c[0]=='-')
				{
					if(c[1]=='d')
						debug = true;
					if(c[1]=='o')
						monoObject = true;
				}
			}
			
		}
		else
		{
			#ifdef NDEBUG
				std::cerr << argv[0] << ": Wrong Number of arguments: <ASE File Name> <Destination Directory> <meshFile> [-o (object)] [-d (debug)]\n";
				return 0;		
			#endif
		}

		ToLower(fileName);
		const std::string stripedFileName = StripFileName(fileName);

		if(debug)
			gTrace.open((stripedFileName + ".debug").c_str());

		std::cout << "Processing " << fileName << "                         \r";

		ASEParser parser(fileName.c_str());
			
		ReportFile repportFile(repportFileName);
		
		
		repportFile << "*OBJECT " << stripedFileName << "\n";
		

		MeshCollection meshes(monoObject);
		while(parser.ReadToken()!= TEOF)
		{
			if(parser == TBlockName)
			{
				if(parser == kGeomObjevtBlock)
				{
					meshes.push_back(parser.ReadObject(meshes.size()));					
					repportFile << "\t*MESH " << meshes.back()->GetName() << " *ENDMESH\n";
				}
				else
					parser.SkipBlock();
			}
			else
				parser.Throw("Expecting a Block Name");
		}
		repportFile << "*ENDOBJECT\n";

		OutFile out( outDirName + "\\" + stripedFileName + ".vobj" );
		meshes.Save(out);

		repportFile.Dump();
	}
	catch(Exception& e)
	{		
		std::cerr << "\nError:" << e.m_msg << "\n";
	}
	catch(OutFileException& e2)
	{
		std::cerr << "\nOutFile Exception:" << e2.m_msg << "\n";
	}
	return 0;
}
