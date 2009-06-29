#include "ASEMaterial.h"
#include "ASEParser.h"
#include "ASEMap.h"

#include <memory>
#include <assert.h>



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
ASEMaterial::ASEMaterial(ASEParser& parser)
	:m_map(NULL)
{
	parser.ReadToken(ASEParser::TOpenBlock);
	while(parser.ReadToken()!=ASEParser::TCloseBlock)
	{
		if(parser=="*MAP_DIFFUSE")
		{
			m_map = new ASEMap(parser);
		}
		else if(parser=="*MATERIAL_NAME")
		{
			m_name = parser.ReadString();
		}		
		else if(parser=="*SUBMATERIAL")
		{
			ParseSubMaterial(parser);
		}
		else
			parser.SkipBlock();
	}
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
ASEMaterial::~ASEMaterial()
{
	delete m_map;
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
const ASEMap* ASEMaterial::GetMap(int subMaterialId) const
{
	
	if(m_subMaterials.size()!=0)
	{
		// If there are sub-material, return the sub-material at the index provided
		assert(m_map==NULL);
		if(subMaterialId>=0 && subMaterialId < m_subMaterials.size())
		{
			const ASEMaterial* subMaterial = m_subMaterials[subMaterialId];
			if(subMaterial)
			{		
				assert(subMaterial->m_subMaterials.size()==0);
				return subMaterial->m_map;
			}
		}
		assert(!"error");
		return NULL;
	}
	else
	{
		// If there are no sub materials, the subMaterialId is ignored
		return m_map;
	}
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void ASEMaterial::ParseSubMaterial(ASEParser& parser)
{
	const int matId = parser.ReadInt();

	if(matId != m_subMaterials.size())
		parser.Throw("Bad Sub Material Id");

	m_subMaterials.push_back(new ASEMaterial(parser));
	
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
ASEMaterialList::ASEMaterialList(ASEParser& parser)
{
	parser.ReadToken(ASEParser::TOpenBlock);
	parser.ReadToken("*MATERIAL_COUNT");

	const int nb = parser.ReadInt();

	for(int i=0;i<nb;++i)
	{
		parser.ReadToken("*MATERIAL");

		const int id = parser.ReadInt();
		

		if(m_materials.find(id)!=m_materials.end())
			parser.Throw("MaterialId already used");

		m_materials[id] = new ASEMaterial(parser);
	}
	parser.ReadToken(ASEParser::TCloseBlock);
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
ASEMaterialList::~ASEMaterialList()
{
	for(Materials::iterator i = m_materials.begin();i != m_materials.end();i++)
		delete i->second;
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
const ASEMaterial* ASEMaterialList::Find(const ASEMaterialId& id) const
{
	Materials::const_iterator i = m_materials.find(id);
	if(i!=m_materials.end())
	{
		return i->second;
	}
	else
		return NULL;
}










