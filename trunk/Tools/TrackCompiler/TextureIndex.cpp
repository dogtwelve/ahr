#include "TextureIndex.h"
#include "OutFile.h"

#include "../LibASE/ASEMap.h"

#include <vector>



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
CTextureIndex::CTextureIndex()
{
	m_map[0] = 0;
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void CTextureIndex::Save(OutFile& out) const
{
	std::vector<long>	offsets;
	std::vector<char>	data;

	offsets.resize(m_map.size());

	for(Map::const_iterator i = m_map.begin();i!=m_map.end();i++)
	{
		offsets[i->second] = data.size();
		const ASEMap* textureMap = i->first;
		if(textureMap)
		{
			const std::string& textureFileName = textureMap->GetName();
			data.insert(data.end(),textureFileName.begin(),textureFileName.end());
		}
		data.push_back('\0');
	}
	out.Write(offsets);
	out.Write(data);
}



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
CTextureIndex& CTextureIndex::Get()
{
	static CTextureIndex gTextureIndex;
	return gTextureIndex;
}



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
int CTextureIndex::Get(const ASEMap* m)
{
	int index;

	Map::iterator i = m_map.find(m);
	if(i != m_map.end())
		index =i->second;
	else
	{
		index = m_map.size();
		m_map[m] = index;
	}
	return index;
}

