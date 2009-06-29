#ifndef _ASEREADER_H_
#define _ASEREADER_H_

#include "DisableStlWarnings.h"



#include <vector>
#include <string>
#include <assert.h>

class ASEParser;
class ASEMesh;
class ASEMaterialList;




// ---------------------------------------------------------------------------
//	Read and provide access to the information contained in an ASE file.
//	
//	Any error encoutered are reported by throwing an ASEException error
//
// ---------------------------------------------------------------------------
class ASEReader	
{
private:	
	typedef std::vector<ASEMesh*>				Meshes;

public:
	ASEReader(const char* fileName);
	~ASEReader();

	int					NbMeshes()		const {return m_meshes.size();}
	const ASEMesh&		GetMesh(int i)	const {assert(i>=0 && i< NbMeshes()); return *m_meshes[i];}

private:
	typedef std::vector<ASEMesh*> Meshes;
	
	ASEMaterialList*	m_materials;
	Meshes				m_meshes;	
};

#endif // _ASEREADER_H_
