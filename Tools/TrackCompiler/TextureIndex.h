#ifndef _TEXTUREINDEX_H_
#define _TEXTUREINDEX_H_

#include "DisableStlWarnings.h"
			

#include <map>


class ASEMap;
class OutFile;



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
	void	Save(OutFile& out) const;

private:
	typedef std::map<const ASEMap*,int>	Map;
	Map	m_map;
};

#endif // _TEXTUREINDEX_H_
