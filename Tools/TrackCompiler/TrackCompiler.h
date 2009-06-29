#ifndef _TRACKCOMPILER_H_
#define _TRACKCOMPILER_H_



class ASEMap;


// ---------------------------------------------------------------------------
// Arg! a singleton!!!!!
// ---------------------------------------------------------------------------
class CTextureIndex
{
private:
	CTextureIndex();

public:
	static CTextureIndex& Get();
	
	int		Get(const ASEMap*);

	void Save(OutFile& out) const 
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



	


private:
	typedef std::map<const ASEMap*,int>	Map;
	Map	m_map;
};














#endif // _TRACKCOMPILER_H_
