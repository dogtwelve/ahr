#ifndef _ASEMESH_H_
#define _ASEMESH_H_

#include <string>
#include <vector>
#include <memory.h>
#include <assert.h>

class ASEParser;
class ASEMaterialList;
class ASEMap;
class ASEMaterial;




// ---------------------------------------------------------------------------
//	Can represent any set of 3 values (vertex, normal or uvw values)
// ---------------------------------------------------------------------------
class ASEVector
{
	typedef double Type;
public:
	ASEVector()	{::memset(m_vals,0,sizeof(m_vals));}

	void Parse(ASEParser&);

	bool IsNull() const {return X()==Type(0) && Y()==Type(0) && Z()==Type(0);}

	bool operator==(const ASEVector& v) const
	{
		const Type kEpsilon = 0.001;

		for(int i=0;i<3;i++)
		{
			const Type delta = m_vals[i] - v[i];
			if(delta > kEpsilon || delta < -kEpsilon)
				return false;			
		}
		return true;
	}

	inline Type X() const {return m_vals[0];}
	inline Type Y() const {return m_vals[1];}
	inline Type Z() const {return m_vals[2];}

	Type operator[](int i) const {assert(i>=0 && i<3);return m_vals[i];}

private:
	Type m_vals[3];
};



// ---------------------------------------------------------------------------
//	
// ---------------------------------------------------------------------------
class ASEVectorList
	:public std::vector<ASEVector>
{
	ASEVectorList(const ASEVectorList&);
	void operator=(const ASEVectorList&);
public:

	ASEVectorList(){};

	void Parse(ASEParser&);	
};


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class ASEFace
{
public:
	ASEFace()
	{
		m_vertex[0]=0;m_vertex[1]=0;m_vertex[2]=0;	
		m_tVertex[0]=0;m_tVertex[1]=0;m_tVertex[2]=0;
		m_map = NULL;
	}

	// Vertex at each corner of the face
	const ASEVector*	GetVertex(int i)			const {assert(i>=0 && i<3);return m_vertex[i];}

	// texture coordinate at each corner of the face
	const ASEVector*	GetTextureCoordinates(int i)const {assert(i>=0 && i<3);return m_tVertex[i];}

	// Normal at each vertex of the face (as calculated by Max)
	const ASEVector*	GetVertexNormal(int i)		const {assert(i>=0 && i<3);return &m_normals[i];}

	// Normal of the face (as calculated by Max)
	const ASEVector*	GetFaceNormal()				const;

	// Texture Map associated with the face
	const ASEMap*		GetMap()					const {return m_map;}


	// Not to be used by the application; reserved to the usege of the ASEMesh class
	void SetMap(const ASEMaterial&);
	void ParseFace(ASEParser&,const ASEVectorList&,int index);
	void ParseTFace(ASEParser&,const ASEVectorList&,int index);
	void ParseNormal(ASEParser&,const ASEVectorList&,int index);

private:
	const ASEVector*	m_vertex[3];	
	ASEVector			m_faceNormal;
	ASEVector			m_normals[3];
	const ASEVector*	m_tVertex[3];	// texture coordinate 
	int					m_materialId;
	const ASEMap*		m_map;
};

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class ASEFaceList
	:public std::vector<ASEFace>
{
public:

	// reserved to the usage of the ASEMash class
	void Parse(ASEParser&,			const ASEVectorList& vertexList);
	void ParseTFaceList(ASEParser&,	const ASEVectorList& tvList);
	void ParseNormals(ASEParser&,	const ASEVectorList& vertexList);
};



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class ASEMesh
{	
	ASEMesh(const ASEMesh&);
	void operator=(const ASEMesh&);
public:
	ASEMesh(ASEParser&,const ASEMaterialList*);
	~ASEMesh();

	const std::string& GetName() const {return m_name;}


	// Face access, in stl style
	typedef ASEFaceList::const_iterator FaceIterator;

	FaceIterator	FaceBegin() const {return m_faceList.begin();}
	FaceIterator	FaceEnd()   const {return m_faceList.begin();}
	int				NbFaces()	const {return m_faceList.size();}



	typedef ASEVectorList::const_iterator VertexIterator;

	const ASEVectorList&	GetVertexList() const	{return m_vertexList;}

	VertexIterator			VertexBegin()	const {return m_vertexList.begin();}
	VertexIterator			VertexEnd()		const {return m_vertexList.end();}
	int						NbVertex()		const {return m_vertexList.size();}


private:
	void ParseNodeTM(ASEParser&);
	void ParseMesh(ASEParser&);

private:
	std::string						m_name;
	ASEVector						m_position;

	ASEVectorList					m_vertexList;
	ASEVectorList					m_textureVertexList;
	ASEFaceList						m_faceList;
	int								m_materialIndex;
	bool							m_normals;
	const ASEMaterialList*	const	m_materialList;
};


#endif // _ASEMESH_H_
