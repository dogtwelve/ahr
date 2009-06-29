#include "ASEMesh.h"
#include "ASEParser.h"
#include "ASEMaterial.h"

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void ASEVectorList::Parse(ASEParser& p)
{
	p.ReadToken(ASEParser::TOpenBlock);
	const int s = size();

	for(int i=0;i<s;i++)
	{
		p.ReadToken(); // *MESH_VERTEX or *MESH_TVERTLIST
		int index = p.ReadInt();
		if(index!=i || i>=size())
			p.Throw("Bad Vertex Index");
		operator[](i).Parse(p);
	}
	p.ReadToken(ASEParser::TCloseBlock);
}



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void ASEVector::Parse(ASEParser& p)
{
	m_vals[0] = p.ReadFloat();
	m_vals[1] = p.ReadFloat();
	m_vals[2] = p.ReadFloat();
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void ASEFace::SetMap(const ASEMaterial& material)
{
	m_map = material.GetMap(m_materialId);
}





// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void ASEFace::ParseFace(ASEParser& parser,const ASEVectorList& vertexList,int index)
{
	parser.ReadToken("*MESH_FACE");
	parser.ReadToken();	// index mangled with a ":"
	
	char expectedString[] = "A:";
	for(int i=0;i<3;i++)
	{
		expectedString[0] = 'A' + i;
		parser.ReadToken(expectedString);
		const int index = parser.ReadInt();
		
		if(index <0 && index >= vertexList.size())
			parser.Throw("Bad Vertex Index");
	
		m_vertex[i] = &vertexList[index];
	}

	for(i=0;i<3;i++)
	{
		parser.ReadToken();	//AB:
		parser.ReadInt();
	}

	parser.ReadToken(ASEParser::TBlockName);

	if(parser=="*MESH_SMOOTHING")
	{
		ASEParser::EToken token = parser.ReadToken();
		if(token==ASEParser::TUnknown)	// This is the optional int
			parser.ReadToken("*MESH_MTLID");
	}

	if(parser!="*MESH_MTLID")
		parser.Throw("Bad token");
	
	m_materialId = parser.ReadInt();
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
const ASEVector* ASEFace::GetFaceNormal() const 
{
	if(m_faceNormal.IsNull())
		return NULL; 
	else 
		return &m_faceNormal;
}



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void ASEFace::ParseTFace(ASEParser& parser,const ASEVectorList& textureVertexList,int face_index)
{
	parser.ReadToken("*MESH_TFACE");

	if(parser.ReadInt()!=face_index)
		parser.Throw("Bad TFace Index");
	
	for(int i=0;i<3;i++)
	{
		const int index = parser.ReadInt();
		if(index <0 || index >= textureVertexList.size())
			parser.Throw("Bad TVertexIndex");
		
		m_tVertex[i] = &textureVertexList[index];
	}
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void ASEFace::ParseNormal(ASEParser& parser,const ASEVectorList& vertexList,int face_index)
{
	parser.ReadToken("*MESH_FACENORMAL");
	if(face_index != parser.ReadInt())
		parser.Throw("bad Face normal index");
			
	m_faceNormal.Parse(parser);

	const ASEVector* const firstVertex = &vertexList[0];
		
	for(int i=0;i<3;i++)
	{
		parser.ReadToken("*MESH_VERTEXNORMAL");

		const int vertexIndex = parser.ReadInt();

		if(vertexIndex != m_vertex[i] - firstVertex)
			parser.Throw("Bad normal vertex index");
		
		m_normals[i].Parse(parser);
	}
}



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void ASEFaceList::Parse(ASEParser& parser,const ASEVectorList& vertexList)
{
	parser.ReadToken(ASEParser::TOpenBlock);
	
	int index =0;
	for(iterator i=begin();i!=end();i++)
	{
		i->ParseFace(parser,vertexList,index);
		index++;
	}
	parser.ReadToken(ASEParser::TCloseBlock);
}



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void ASEFaceList::ParseTFaceList(ASEParser& parser,const ASEVectorList& textureVertexList)
{
	parser.ReadToken(ASEParser::TOpenBlock);
	
	int index =0;
	for(iterator i=begin();i!=end();i++)
	{
		i->ParseTFace(parser,textureVertexList,index);
		index++;
	}
	parser.ReadToken(ASEParser::TCloseBlock);
}







// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void ASEFaceList::ParseNormals(ASEParser& parser,const ASEVectorList& vertexList)
{
	if(vertexList.size()==0)
		parser.Throw("Empty Vertex List");



	parser.ReadToken(ASEParser::TOpenBlock);
	
	int index =0;
	for(iterator i=begin();i!=end();i++)
	{
		i->ParseNormal(parser,vertexList,index);
		index++;
	}
	parser.ReadToken(ASEParser::TCloseBlock);
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
ASEMesh::ASEMesh(ASEParser& parser,const ASEMaterialList* materialList)
	:m_materialIndex(-1),
	m_normals(false),
	m_materialList(materialList)
{
	parser.ReadToken(ASEParser::TOpenBlock);

	while(parser.ReadToken()!=ASEParser::TCloseBlock)
	{
		if(parser=="*NODE_NAME")
			m_name = parser.ReadString();
		else if(parser=="*NODE_TM")
		{
			ParseNodeTM(parser);
		}
		else if(parser=="*MESH")
		{
			ParseMesh(parser);
		}
		else if(parser=="*MATERIAL_REF")
		{
			m_materialIndex = parser.ReadInt();			
		}
		else 
			parser.SkipBlock();
	}



	if(materialList)
	{	
		const ASEMaterial* material = materialList->Find(m_materialIndex);
		if(material)
		{
			for(ASEFaceList::iterator faceIter =m_faceList.begin();faceIter!=m_faceList.end();++faceIter)
				faceIter->SetMap(*material);
		}
	}
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
ASEMesh::~ASEMesh()
{

}



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void ASEMesh::ParseNodeTM(ASEParser& parser)
{
	parser.ReadToken(ASEParser::TOpenBlock);
	while(parser.ReadToken()!=ASEParser::TCloseBlock)
	{
		if(parser=="*TM_POS")
			m_position.Parse(parser);
		else
		{
			// TODO: Add other necessary info here
			parser.SkipBlock();
		}
	}	
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void ASEMesh::ParseMesh(ASEParser& parser)
{
	parser.ReadToken(ASEParser::TOpenBlock);
	while(parser.ReadToken()!=ASEParser::TCloseBlock)
	{
		if(parser=="*MESH_NUMVERTEX")
		{
			m_vertexList.resize(parser.ReadInt());
		}
		else if(parser=="*MESH_NUMFACES")
		{
			m_faceList.resize(parser.ReadInt());
		}
		else if(parser=="*MESH_VERTEX_LIST")
		{
			m_vertexList.Parse(parser);
		}
		else if(parser=="*MESH_FACE_LIST")
		{
			m_faceList.Parse(parser,m_vertexList);
		}
		else if(parser=="*MESH_NUMTVERTEX")
		{
			m_textureVertexList.resize(parser.ReadInt());
		}
		else if(parser=="*MESH_TVERTLIST")
		{
			m_textureVertexList.Parse(parser);			
		}
		else if(parser=="*MESH_NUMTVFACES")
		{
			const int numTVFaces = parser.ReadInt();
			if(numTVFaces!= m_faceList.size())
				parser.Throw("Bad number of *MESH_NUMTVFACES");
		}
		else if(parser == "*MESH_TFACELIST")
		{
			m_faceList.ParseTFaceList(parser,m_textureVertexList);			
		}
		else if(parser == "*MESH_NORMALS")
		{
			m_faceList.ParseNormals(parser,m_vertexList);
			m_normals=true;
		}
		else
			parser.SkipBlock();
	}
}
