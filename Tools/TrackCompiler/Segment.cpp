#include "Segment.h"
#include "OutFile.h"
#include "TextureIndex.h"


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------

CSegment::CSegment()
	:m_end(false)
{
	int i;
	for(i = 0;i<kSectionWidth;i++)
	{
		m_textureId[i]=0;
		m_vertex[i]=NULL;
	}
	m_vertex[kSectionWidth]=NULL;
}




// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void CSegment::SetVertex(ASEMesh::VertexIterator& vertexIter)
{
	for(int i = 0;i<(kSectionWidth+1);i++)
	{
		m_vertex[i] = &(*vertexIter);
		vertexIter++;
	}
}



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void CSegment::SetFace(ASEMesh::FaceIterator& faceIter)
{
	for(int i = 0;i<kSectionWidth;i++)
	{			
		m_textureId[i] = CTextureIndex::Get().Get(faceIter->GetMap());

		faceIter++;
		faceIter++;
	}
}



void CSegment::Save(OutFile& out) const
{
	out.Write(m_textureId,sizeof(m_textureId));
}

