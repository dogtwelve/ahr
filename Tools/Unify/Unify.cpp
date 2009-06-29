#pragma warning(disable : 4786)
#include "unify.h"
#include <algorithm>
#include <set>
#include <limits>

using namespace Unify;




const float Unify::CValue::k_fNan =  std::numeric_limits<float>::quiet_NaN();


/// CNode ///////////////////////////////////////////


// CNode -> ~CNode
CNode::~CNode()
{
	Clear();
}

void CNode::Clear()
{
	std::vector<CNode*>::iterator it;
	for(it=m_Nodes.begin();it!=m_Nodes.end();++it){
		delete *it;
	}
	m_Nodes.resize(0);
	
	for(ParameterIterator it2=m_Parameters.begin();it2!=m_Parameters.end();++it2)
		delete it2->second;

	m_Parameters.clear();
}


const CNode * CNode::GetChildByParamValue(const std::string& param, const std::string& value) const
{
	for(int i = 0; i < GetChildCount(); ++i)
	{
		const CParameter * pParam = GetChild(i)->GetParameter(param);
		if(pParam && value == pParam->GetValue()->GetString())
		{
			return GetChild(i);
		}
	}
	return 0;
}

CNode * CNode::GetChildByParamValue(const std::string& param, const std::string& value)
{
	return const_cast<CNode *>(const_cast<const CNode*>(this)->GetChildByParamValue(param, value));
}


// CNode -> GetParameter
CParameter * CNode::GetParameter(const std::string& name){
	 
	 ParameterIterator it=m_Parameters.find(name);

	if(it==m_Parameters.end()){
		return 0;
	}else{
		return it->second;
	}
}


const CParameter * CNode::GetParameter(const std::string& name) const 
{
	Parameters::const_iterator i = m_Parameters.find(name);
	if(i==m_Parameters.end())
		return NULL;
	else
		return i->second;
}


CNode::ParameterIterator CNode::RemoveParameter(ParameterIterator i)
{
	delete i->second;
	return m_Parameters.erase(i);
}


// CNode -> RemoveParameter
void CNode::RemoveParameter(const std::string& name)
{
	ParameterIterator i = m_Parameters.find(name);
	if(i!=m_Parameters.end())
		RemoveParameter(i);
}

// Remove a parameter without deleting it
Unify::CParameter * CNode::ReleaseParameter(const std::string& name)
{
	Unify::CParameter* p=NULL;
	ParameterIterator i = m_Parameters.find(name);
	if(i!=m_Parameters.end())
	{
		p = i->second;
		m_Parameters.erase(i);
	}
	return p;
}

// Add the node parameters from another node
void CNode::AddParametersFrom(CNode* in_pNode)
{
	for(ParameterIterator it = in_pNode->ParameterBegin(); it != in_pNode->ParameterEnd(); ++it)
	{
		(*it).second->Copy(NewParameter((*it).second->GetName()));
	}
}

// Merge the node parameters from another node
void CNode::GetParameterValuesFrom(CNode* in_pNode)
{
	for(ParameterIterator it = in_pNode->ParameterBegin(); it != in_pNode->ParameterEnd(); ++it)
	{
		Unify::CParameter * pSourceParam = (*it).second;
		Unify::CParameter * pExistingParam = GetParameter(pSourceParam->GetName());
		if (pExistingParam)
		{
			pSourceParam->Copy(pExistingParam);
		}
		else
		{
			pSourceParam->Copy(NewParameter(pSourceParam->GetName()));
		}
	}
}



void CNode::Copy(Unify::CNode* target) const
{

	target->Clear();

	// Copy parameters
	ConstParameterIterator it;
	for(it=ParameterBegin();it!=ParameterEnd();++it){
		(*it).second->Copy(target->NewParameter((*it).first));
	}

	// Copy sub nodes
	target->m_Nodes.reserve(m_Nodes.size());
	std::vector<CNode*>::const_iterator it2;
	for(it2=m_Nodes.begin();it2!=m_Nodes.end();++it2){
		(*it2)->Copy(target->NewChild());
	}

}


void CNode::DetachChild(CNode* node){
	std::vector<CNode*>::iterator it;
	it=std::find(m_Nodes.begin(),m_Nodes.end(),node);
	if(it!=m_Nodes.end()){
		m_Nodes.erase(it);
	}
}
