#ifndef _ASEMATERIAL_H_
#define _ASEMATERIAL_H_

#include "DisableStlWarnings.h"
#include <map>
#include <string>
#include <vector>

class ASEParser;
class ASEMap;

typedef unsigned int ASEMaterialId;

// ---------------------------------------------------------------------------
//	Contains information about one material or a submaterial
//
//	Some material are directly useable, some material are in fact a list of
//	sub-material.
//
//	We assume that a normal material can contains 
//	a *MAP_DIFFUSE or some *SUBMATERIAL, but never both
//
//	This sould not be used directly by the application but should remains 
//	hidden in the ASEMesh class 
// ---------------------------------------------------------------------------
class ASEMaterial
{
public:
	ASEMaterial(ASEParser&);
	~ASEMaterial();

	const std::string& GetName() const {return m_name;}

	const ASEMap* GetMap(int subMaterialId) const;	

private:	
	void ParseSubMaterial(ASEParser&);

private:	
	typedef std::vector<ASEMaterial*>	SubMaterials;

	std::string		m_name;
	ASEMap*			m_map;
	SubMaterials	m_subMaterials;
};


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class ASEMaterialList
{
private:
	typedef std::map<ASEMaterialId,ASEMaterial*> Materials;

public:
	ASEMaterialList(ASEParser&);
	~ASEMaterialList();

	const ASEMaterial*	Find(const ASEMaterialId&) const;

private:
	Materials	m_materials;
};


#endif // _ASEMATERIAL_H_
