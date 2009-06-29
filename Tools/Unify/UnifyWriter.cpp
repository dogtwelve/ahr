#pragma warning(disable:4786)

#include "unify.h"
#include "unifywriter.h"

#include "util.h"

#ifdef WIN32
#include "windows.h"
#endif

namespace Unify {

void CUnifyWriter::Write(const char * filename, Unify::CNode* root, bool brief){
	m_Brief=brief;
	m_Tab=0;
	m_File.open(filename,std::ios::out);

	if( (m_File.rdstate() & std::ios::failbit ) != 0 ){
		MessageBox(0,"Can't open file for writing.\nMake sure it is not read-only.\n","Alice error",MB_OK);
	}else{
		WriteNode(root);
		m_File.close();
	}
}

void CUnifyWriter::WriteNode(Unify::CNode* node){
	bool compact=false;
	bool empty=false;
	std::string tab (m_Tab,'\t');

	/*if(node->GetChildCount()==0 && node->GetParameterCount()<=4)compact=true;
	if(node->GetParameterCount()==0)compact=false;*/

	// If node is a block, compact it
	Unify::CParameter * param;
	param=node->GetParameter("class");
	if(param){
		if(param->GetString()=="block"){
			compact=true;
			if(		node->GetParameter("type")->GetString()=="empty" 
				&&	node->GetParameter("flags")->GetString()=="" )
			{
				empty=true;
			}
		}
	}
	if(!m_Brief)empty=false;
	
	if(node&&!empty){
		m_File << std::endl<<tab << "(";
		
		param=node->GetParameter("class");
		if(param){
			WriteParameter("class",param);
		}
		param=node->GetParameter("name");
		if(param){
			WriteParameter("name",param);
		}

		Unify::CNode::ParameterIterator it=node->ParameterBegin();
		for(int i=0;i<node->GetParameterCount();i++){
			if((*it).first!="class" && (*it).first !="name"){
				if(!compact)
					m_File << std::endl;
				if(!compact)
					m_File << tab ;
				WriteParameter((*it).first,((*it).second));
			}
			it++;

		}

		for(i=0;i<node->GetChildCount();i++){
			m_Tab++;
			WriteNode(node->GetChild(i));
			m_Tab--;
		}


		if(!compact){
			m_File << std::endl << tab << ")";
		}else{
			m_File << ")";
		}
	}
}

void CUnifyWriter::WriteParameter(const std::string& name, Unify::CParameter* param) {
	if(param){
		WriteToken(name.c_str());
		m_File << " ";
		bool first=true;
		for(int i=0;i<param->GetValueCount();i++){
			if(first){
				first=false;
			}else{
				m_File<<",";
			}
			WriteToken(param->GetValue(i)->GetString());
		}
		m_File << " ";
	}
}

void CUnifyWriter::WriteToken(const std::string& str2 ){
	bool quote=false;
	std::string str=str2;
	EscapeString(str);

	if(str.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.+-")!=std::string::npos)
		quote=true;

	if(str=="")
		quote=true;

	if(quote)
		m_File << "\"" ;
	
	//!PATCH (Need to escape quotes)
	m_File << str ;

	if(quote)
		m_File << "\"" ;
		

}

void CUnifyWriter::EscapeString(std::string& str){
	
	Util::Replace(str,"\\","\\\\");

	Util::Replace(str,"\r\n","\\n");
	Util::Replace(str,"\t","\\t");
	Util::Replace(str,"\"","\\\"");

	
}




}