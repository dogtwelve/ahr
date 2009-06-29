#include "ASEReader.h"
#include "ASEParser.h"
#include "ASEMesh.h"
#include "ASEMaterial.h"


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
template<class T>
static void Delete(T begin,const T end)
{	
	while(begin!=end)
	{
		delete *begin;
		++begin;
	}
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
template<class T>
static void Delete(T& v)
{
	Delete(v.begin(),v.end());
}




// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
ASEReader::ASEReader(const char* fileName)
	:m_materials(NULL)
{
	ASEParser parser(fileName);

	while(parser.ReadToken()!= ASEParser::TEOF)
	{
		if(parser == ASEParser::TBlockName)
		{
			if(parser == "*GEOMOBJECT")
			{
				m_meshes.push_back(new ASEMesh(parser,m_materials));
			}
			else if(parser == "*MATERIAL_LIST")
			{
				if(m_materials!=NULL)
					parser.Throw("Bad *MATERIAL_LIST");
				m_materials = new ASEMaterialList(parser);
			}
			else
			{
				// Add new type of block here...
				parser.SkipBlock();
			}
		}
		else 
			parser.Throw("Expecting a Block Name");
	}
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
ASEReader::~ASEReader()
{
	delete m_materials;
	::Delete(m_meshes);
}
