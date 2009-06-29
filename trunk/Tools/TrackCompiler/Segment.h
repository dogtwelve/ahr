#ifndef _SEGMENT_H_
#define _SEGMENT_H_


#include "Fence.h"
#include "../LibASE/ASEMesh.h"



class ASEVector;
class OutFile;




// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class CSegment
{
public:
	enum
	{
		kSectionWidth = 10
	};


	CSegment();
		

	void				MakeEndSegment() {m_end = true;}
	void				SetVertex(ASEMesh::VertexIterator&);
	void				SetFace(ASEMesh::FaceIterator& faceIter);
	bool				IsEnd() const {return m_end;}
	void				Save(OutFile&) const;
	const ASEVector&	GetConnectVertex() const {return *	m_vertex[ kSectionWidth/2 ];}


private:
	const ASEVector*	m_vertex[kSectionWidth+1];
	unsigned char		m_textureId[kSectionWidth];
	CFence				m_fences[2];
	bool				m_end;						// Indicate the dummy section at the end of the track
};

#endif // _SEGMENT_H_
