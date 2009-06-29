#ifndef _TRACK_H_
#define _TRACK_H_


class ASEMesh;
class OutFile;
class CSection;


#include "DisableStlWarnings.h"
#include <string>
#include <map>

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class CTrack
{
public:
	CTrack(const std::string& fileName);
	~CTrack();

	void Add(const const ASEMesh&);
	void Save(OutFile&) const;
	void Finish();
	

private:
	CSection& GetSection(const std::string&);
	
private:
	typedef std::map<std::string,CSection*> Sections;

	Sections	m_sections;
};










#endif // _TRACK_H_
