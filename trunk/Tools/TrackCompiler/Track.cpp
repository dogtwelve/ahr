#include "Track.h"
#include "Section.h"
#include "../LibASE/ASEReader.h"

#include "TextureIndex.h"
#include "OutFile.h"

#include <iostream>

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
inline static bool IsPrefixed(const std::string& str,const std::string& prefix)
{
	return ::strncmp(str.c_str(),prefix.c_str(),prefix.length())==0;
}






enum ESectionType
{
	kTypeMainTrack,
	kTypeAltTrack,
	kTypeFence,

	kTypeUnknown
};

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
static ESectionType GetSectionType(const std::string& s)
{
	if(IsPrefixed(s,"maintrack_"))
		return kTypeMainTrack;

	if(IsPrefixed(s,"fences_"))
		return kTypeFence;

	return kTypeUnknown;
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
static std::string GetSectionName(const std::string& s)
{
	int p = s.find("_");
	if(p!=std::string::npos)
	{
		return s.substr(p+1);
	}
	return "";
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
CTrack::CTrack(const std::string& fileName)
{	
	ASEReader reader((fileName + ".ase").c_str());

	for(int i=0;i<reader.NbMeshes();++i)
		Add(reader.GetMesh(i));
	
	Finish();
}



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
CTrack::~CTrack()
{
	for(Sections::iterator i = m_sections.begin();i!=m_sections.end();++i)
		delete i->second;
}



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void CTrack::Add(const const ASEMesh& mesh)
{
	const ESectionType type = ::GetSectionType(mesh.GetName());
	if(type==kTypeUnknown)
		std::cerr << "** Warning: Mesh " << mesh.GetName() << " Ignored\n";
	else
	{
		CSection& section = GetSection(mesh.GetName());

		if(type==kTypeMainTrack)
			section.SetTrack(mesh);

		if(type==kTypeFence)
			section.SetFence(mesh);
	}
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------


void CTrack::Save(OutFile& out) const 
{
	CTextureIndex::Get().Save(out);

	std::vector<const CSection*> sections;
	sections.resize(m_sections.size());

	for(Sections::const_iterator i = m_sections.begin();i!=m_sections.end();++i)
		sections[i->second->GetIndex()] = i->second;

	out.SaveElements(sections);
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------

void CTrack::Finish()
{
	for(Sections::iterator sectionIter = m_sections.begin();sectionIter!= m_sections.end();++sectionIter)
	{
		for(Sections::iterator sectionIter2 = m_sections.begin();sectionIter2!= m_sections.end();++sectionIter2)
		{
			if(sectionIter!=sectionIter2)
			{
				CSection& current	= *sectionIter->second;
				CSection& next		= *sectionIter2->second;
				
				if(current.GetEndConnectVertex() == next.GetStartConnectVertex())
					current.AddNext(next);
			}
		}
	}
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------

CSection& CTrack::GetSection(const std::string& s)
{
	const int newIndex = m_sections.size();
	const std::string name = ::GetSectionName(s);

	Sections::referent_type& t = m_sections[name];

	if(t==NULL)
		t = new CSection(newIndex);

	return *t;
}
