#include "Section.h"
#include "Outfile.h"
#include "Segment.h"
#include "../LibASE/ASEException.h"

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
CSection::CSection(int index)
	:m_segments(NULL),m_nbSegments(0),m_trackMesh(NULL),m_isOnMainTrack(false),
	m_nextMain(NULL),m_nextAlt(NULL),m_prevMain(NULL),m_prevAlt(NULL),m_index(index)
{
}




// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
CSection::~CSection()	
{
	delete[] m_segments;
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void CSection::Save(OutFile& out) const
{		
	out.Write(m_trackMesh->GetVertexList());	// Vertex
	out.Write(m_nbSegments);
	for(int i=0;i<m_nbSegments;i++)
		m_segments[i].Save(out);

	out.Write(m_nextMain!=NULL	? m_nextMain->GetIndex():-1);
	out.Write(m_nextAlt !=NULL	? m_nextAlt->GetIndex() :-1);
	out.Write(m_prevMain!=NULL	? m_prevMain->GetIndex():-1);
	out.Write(m_prevAlt !=NULL	? m_prevAlt->GetIndex() :-1);		
}



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void CSection::AddNext(CSection& next)
{
	if(next.IsOnMaintrack())
	{
		if(m_nextMain!=NULL)
			throw ASEException("Track has two next main track",m_trackMesh->GetName().c_str());
		else
			m_nextMain = &next;
	}
	else
	{
		if(m_nextAlt!=NULL)
			throw ASEException("Track has two next alt track",m_trackMesh->GetName().c_str());
		else
			m_nextAlt = &next;
	}
	if(IsOnMaintrack())
	{
		if(next.m_prevMain!=NULL)
			throw ASEException("Track has two previous main track",next.m_trackMesh->GetName().c_str());

		next.m_prevMain = this;
	}
	else
	{
		if(next.m_prevAlt!=NULL)
			throw ASEException("Track has two previous alt track",next.m_trackMesh->GetName().c_str());
		next.m_prevAlt = this;
	}
}



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void CSection::SetTrack(const ASEMesh& mesh)
{
	if(m_trackMesh!=NULL)
		throw ASEException("Two Section with identical name",mesh.GetName().c_str());

	m_trackMesh = &mesh;
	const int nbSegments = mesh.NbFaces() / (CSegment::kSectionWidth*2);

	if(mesh.NbFaces() != (CSegment::kSectionWidth*2*nbSegments))
		throw ASEException("Bad number of face in mesh",mesh.GetName().c_str());

	Init(nbSegments);

	if(nbSegments != m_nbSegments)
		throw ASEException("Bad number of segments in mesh",mesh.GetName().c_str());


	if(mesh.NbVertex() != ((nbSegments+1) * (CSegment::kSectionWidth + 1)))
		throw ASEException("Bad number of vertex in mesh",mesh.GetName().c_str());

	if(mesh.NbFaces() != nbSegments * CSegment::kSectionWidth * 2)
		throw ASEException("Bad number of faces in mesh",mesh.GetName().c_str());


	ASEMesh::FaceIterator	faceIter	= mesh.FaceBegin();
	ASEMesh::VertexIterator vertexIter	= mesh.VertexBegin();

	for(int segmentIndex = 0;segmentIndex<m_nbSegments;segmentIndex++)
	{
		m_segments[segmentIndex].SetVertex(vertexIter);
		m_segments[segmentIndex].SetFace(faceIter);
	}
	m_segments[m_nbSegments].SetVertex(vertexIter);// There is m_nbSegments+1 serie of vertex in a track

	if(vertexIter!= mesh.VertexEnd())
		throw ASEException("VertexIter Error",mesh.GetName().c_str());	

	if(faceIter!=mesh.FaceEnd())
		throw ASEException("FaceIter Error",mesh.GetName().c_str());
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void CSection::SetFence(const ASEMesh& mesh)
{

}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void CSection::Init(int nbSegments)
{
	if(m_segments==NULL)
	{		
		m_segments = new CSegment[nbSegments+1];
		m_segments[nbSegments].MakeEndSegment();
		m_nbSegments = nbSegments;
	}
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
const ASEVector& CSection::GetStartConnectVertex() const
{
	assert(m_nbSegments >0);
	return m_segments[0].GetConnectVertex();
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
const ASEVector& CSection::GetEndConnectVertex() const
{
	assert(m_nbSegments >0);
	assert(m_segments[m_nbSegments].IsEnd());// this is NOT a bug, there is m_nbSegments+1 segments
	return m_segments[m_nbSegments].GetConnectVertex();	
}