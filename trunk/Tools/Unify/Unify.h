#pragma warning (disable:4786)

#ifndef _UNIFY_H_
#define _UNIFY_H_


#include <string>
#include <vector>
#include <map>
#include <cfloat>
#include <assert.h>

namespace Unify {

enum EValueType {
	EValueType_String=0,
	EValueType_Int,
	EValueType_Float
};

/*enum ENodeType {
	ENodeType_Any=0,
	ENodeType_Level,
	ENodeType_Room,
	ENodeType_Block,
	ENodeType_Actor,
	ENodeType_Other
};*/

class CValue;




class CUnifyException {
public:
	CUnifyException(const char * description="",int line=0):
	  m_Description(description),m_Line(line),Description(m_Description),Line(m_Line){}

	const std::string& Description;
	const int& Line;

	CUnifyException& operator = (const CUnifyException& e){
		m_Description=e.m_Description;
		m_Line=e.m_Line;
		return *this;
	}

protected:
	std::string m_Description;
	int m_Line;
};



class CParameter;
class CNode;


class CValue {
public:
	static const float k_fNan;

public:


	CValue(): m_fFloatValue(k_fNan), m_bValidString(false) {}
	~CValue(){}

	CValue(const CValue& value){
		operator=(value);		
	}

	CValue& operator=(const CValue& value){
		m_fFloatValue=value.m_fFloatValue;
		m_StringValue=value.m_StringValue;
		m_bValidString=value.m_bValidString;
		return *this;
	}

	void CValue::Copy(CValue* target) const
	{
		// Copy string
		if (!_isnan(m_fFloatValue))
		{
			target->SetFloat(GetFloat());
		}
		else
		{
			target->SetString(GetString());
		}
		
		// Copy type
		
	}

	bool GetBool() const {return GetString() != "false" && GetString() != "0";}
	int GetInt() const {return GetFloat();}
	float GetFloat()  const
	{
		if (_isnan(m_fFloatValue))
		{
			m_fFloatValue = atof(m_StringValue.c_str());
			m_StringValue="";
			m_bValidString = false;
		}
		return m_fFloatValue;
	}
	const std::string& GetString() const
	{
		if (!m_bValidString && !_isnan(m_fFloatValue))
		{
			char str [256];
			sprintf(str,"%.3f",m_fFloatValue);
			m_StringValue = str;
			m_bValidString = true;
		}
		return m_StringValue;
	}

	
	void SetInt(int val)  {SetFloat(val);}
	void SetString(const std::string& val)
	{
		m_StringValue = val;
		m_fFloatValue = k_fNan;
	}
	void SetBool(bool in_b)
	{
		SetString(in_b ? "true" : "false");
	}
	void SetFloat(float val)
	{
		m_fFloatValue = val;
		m_StringValue="";
		m_bValidString = false;
	}


	//CParameter* GetParent()  {return m_Parent;}

protected:

	//CParameter* m_Parent;

	mutable float m_fFloatValue;
	mutable std::string m_StringValue;
	mutable bool m_bValidString;
};




class CParameter {
public:
	CParameter(const std::string& name, CNode* parent=0):m_Name(name), m_Parent(parent){}
	~CParameter(){
		Clear();
	}

	void Clear(){
		/*std::vector<CValue*>::iterator it;
		for(it=m_Values.begin();it<m_Values.end();it++){
			delete *it;
		}
		m_Values.resize(0);*/
		m_Values.clear();
	}

	void Copy(CParameter* target) const {

		target->Clear();

		// Copy values
		target->m_Values.reserve(m_Values.size());
		for(int i=0;i<GetValueCount();++i){
			GetValue(i)->Copy(target->NewValue());
		}
	}


	int GetValueCount() const {return m_Values.size();}
	CValue* GetValue(int i=0)  {return &m_Values[i];}
	const CValue* GetValue(int i=0) const  {return &m_Values[i];}

	CValue* NewValue(){m_Values.push_back(CValue()); return &m_Values.back();};


	
	int GetInt(int v=0) const {return m_Values[v].GetInt();}
	bool GetBool(int v=0) const {return m_Values[v].GetBool();}
	float GetFloat(int v=0) const {return m_Values[v].GetFloat();}

	const std::string& GetString(int v=0) const {return m_Values[v].GetString();}

//	D3DXVECTOR3 GetVector3() const {return D3DXVECTOR3(m_Values[0].GetFloat(),m_Values[1].GetFloat(),m_Values[2].GetFloat());}
//	D3DXVECTOR2 GetVector2() const {return D3DXVECTOR2(m_Values[0].GetFloat(),m_Values[1].GetFloat());}

	CNode* GetParent() const {return m_Parent;}
	const std::string& GetName() const {return m_Name;}

protected:
	std::vector<CValue> m_Values;
	CNode* m_Parent;
	std::string m_Name;

};




class CNode {
public:
	typedef std::map<std::string,CParameter*> Parameters;
	typedef Parameters::iterator ParameterIterator;
	typedef Parameters::const_iterator ConstParameterIterator;


	CNode(CNode* parent=0):m_Parent(parent){}
	~CNode();

	void Clear();
	void Copy(CNode*) const;


	int GetChildCount() const {return m_Nodes.size();}

	//ENodeType	GetType()					{return m_Type;}
	//void		SetType(ENodeType type)		{m_Type = type;}

	CNode * GetChild(int i)		{assert(i>=0 && i < m_Nodes.size());return m_Nodes[i];}
	const CNode * GetChild(int i) const	{assert(i>=0 && i < m_Nodes.size());return m_Nodes[i];}
	CNode * GetChildByParamValue(const std::string& param, const std::string& value);
	const CNode * GetChildByParamValue(const std::string& param, const std::string& value) const;
	CNode * GetChildByName(const std::string& name) { return GetChildByParamValue("name", name);}
	const CNode * GetChildByName(const std::string& name) const { return GetChildByParamValue("name", name);}

	void DetachChildren(){m_Nodes.clear();}
	void AddChild(CNode* node){m_Nodes.push_back(node);}
	CNode * NewChild(){m_Nodes.push_back(CreateChildNode());return m_Nodes.back();}
	void DetachChild(CNode* node);
	//int Find(CNode* node);

	const CParameter * GetParameter(const std::string& name) const;
	CParameter * GetParameter(const std::string& name);

	CParameter * NewParameter(const std::string& name)  
	{
		CParameter *p = new CParameter(name,this);
		m_Parameters[name]= p;
		return p;
	}

	int GetParameterCount() const {return m_Parameters.size();}
	
	ParameterIterator ParameterBegin() {return m_Parameters.begin();}
	ParameterIterator ParameterEnd() {return m_Parameters.end();}

	ConstParameterIterator ParameterBegin() const {return m_Parameters.begin();}
	ConstParameterIterator ParameterEnd() const {return m_Parameters.end();}


	ParameterIterator RemoveParameter(ParameterIterator);


	void RemoveParameter(const std::string& name);


	// Remove a parameter without deleting it
	CParameter * ReleaseParameter(const std::string& name);
	
	CNode* GetParent()  {return m_Parent;}

	// Merge
	void AddParametersFrom(CNode* in_pNode);
	void GetParameterValuesFrom(CNode* in_pNode);

protected:
	/////////////////////////////////////////////////////////////////////////////
	// Overridable child creation function
	// Useful if deriving from this class (see CLevelNode, CRoomNode, etc)
	virtual CNode * CreateChildNode()
	{
		return new CNode(this);
	}

protected:
	
	//ENodeType m_Type;
	std::vector<CNode*> m_Nodes;

	

	Parameters m_Parameters;

	CNode* m_Parent;
};





class CUnifySync {
public:
	virtual void SetNode(Unify::CNode * node){m_Node=node;}
	virtual Unify::CNode * GetNode(){return m_Node;}
	virtual void SaveToNode(){}
	virtual void LoadFromNode(){}

protected:
	Unify::CNode * m_Node;

};



class CValueReference {
public:
	CValueReference(Unify::CNode* node, const std::string& parameterName, int valueIndex):
	m_Node(node),
	m_ParameterName(parameterName),
	m_ValueIndex(valueIndex){}

	Unify::CValue*	GetValuePointer(){
		Unify::CParameter* param=m_Node->GetParameter(m_ParameterName);
		if(param){
			return param->GetValue(m_ValueIndex);
		}else{
			return 0;
		}
	}

private:
	CNode* m_Node;
	std::string  m_ParameterName;
	int m_ValueIndex;

};





}




#endif
