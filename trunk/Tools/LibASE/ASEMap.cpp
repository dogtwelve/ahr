#include "ASEMap.h"
#include "ASEParser.h"
#include <assert.h>




// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
ASEMap::ASEMap(ASEParser& parser)
{
	assert(parser=="*MAP_DIFFUSE");

	parser.ReadToken(ASEParser::TOpenBlock);
	while(parser.ReadToken()!=ASEParser::TCloseBlock)
	{
		if(parser=="*BITMAP")
			m_fileName = parser.ReadString();
		else
			parser.SkipBlock();
	}
}

