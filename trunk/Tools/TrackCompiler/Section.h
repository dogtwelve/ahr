#ifndef _SECTION_H_
#define _SECTION_H_
#include "../libase/asemesh.h"

class OutFile;
class CSegment;


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class CSection
{
	CSection(const CSection&);	
public:
	CSection(int index);
	~CSection();

	const ASEVector& GetStartConnectVertex() const;
	const ASEVector& GetEndConnectVertex() const;
	

	bool	IsOnMaintrack() const {return m_isOnMainTrack;}
	void	AddNext(CSection& next);
	int		GetIndex() const {return m_index;}
	void	SetTrack(const ASEMesh&);	
	void	SetFence(const ASEMesh&);
	void	Save(OutFile&) const;
	
private:
	void	Init(int);

private:
	const ASEMesh*	m_trackMesh;
	CSegment*		m_segments;
	int				m_nbSegments;
	bool			m_isOnMainTrack;

	const CSection*	m_nextMain;
	const CSection*	m_nextAlt;

	const CSection*	m_prevMain;
	const CSection*	m_prevAlt;

	const int		m_index;
};
#endif // _SECTION_H_
