#ifndef _UTIL_H_
#define _UTIL_H_

#include <string>
#include <algorithm>

class Util {
	
public:


	static std::string& Trim(std::string& str){
		long bpos;
		long epos;
		bpos=str.find_first_not_of(" \t\n\r");
		if(bpos==str.npos){
			str="";
			return str;
		}
		epos=str.find_last_not_of(" \t\n\r");
		if(epos==str.npos){
			str="";
			return str;
		}

		return str=str.substr(bpos,epos-bpos+1);
	}

	static std::string& Replace(std::string& str, std::string key, std::string rep){
		long bpos;
		
		bpos=str.find(key);
		while(bpos!=str.npos){
			str=str.replace(bpos,key.size(),rep);
			bpos=str.find(key,bpos+rep.size());
		}

		return str;
	}

	static void SplitLeft(std::string str, std::string& left, std::string& right,const char * tok=" \t\n\r"){
		long pos;
		str=Trim(str);
		pos=str.find_first_of(tok);

		if(pos!=str.npos){
			left=Trim(str.substr(0,pos));
			right=Trim(str.substr(pos+1,str.size()-pos));
		}else{
			left=str;
			right="";
		}
	}

	static void SplitRight(std::string str, std::string& left, std::string& right,const char * tok=" \t\n\r"){
		long pos;
		str=Trim(str);
		pos=str.find_last_of(tok);

		if(pos!=str.npos){
			left=Trim(str.substr(0,pos));
			right=Trim(str.substr(pos+1,str.size()-pos));
		}else{
			left=str;
			right="";
		}
	}

	static bool InStr(const char* in, char c){
		return strchr(in,c) != 0;
	}

	
	static bool InStri(const char* in, char c){
		return (strchr(in,::tolower(c))!=0) || (strchr(in,::toupper(c))!=0);
	}

	static float Thresh (float v1,float v2,float x){
		if(x<=0.0f){
			return v1;
		}else if(x>=1.0f){
			return v2;
		}else{
			return v1+x*(v2-v1);
		}
	}




	template <class T>
	class LessThan {
	public:
		bool operator ()(const T& v1,const T& v2) const{
			return v1<v2;
		}
	};

	/*class LessThan<D3DXVECTOR3> {
	public:
		bool operator ()(const D3DXVECTOR3& v1,const D3DXVECTOR3& v2) const{
			if(v1.x==v2.x){
				if(v1.y==v2.y){
					return v1.z<v2.z;
				}else return v1.y<v2.y;
			}else return v1.x<v2.x;
		}
	};*/

	
	template <class T>
	static inline T Exp4Lerp (T a,T b,float x){
		x=1.0f-pow(1.0f-x,4);
		return  a*(1-x) + b*x;
	}

	class ToLower {
		public:
		void operator()(char& c){	c=::tolower(c);}
	};

	std::string static LowerCase(std::string str){
		std::for_each(str.begin(),str.end(),ToLower());
		return str;
	};


	/*static void NearestAngle(D3DXVECTOR3& io_vAngle1, D3DXVECTOR3& io_vAngle2)
	{
		NearestAngle(io_vAngle1.x,io_vAngle2.x);
		NearestAngle(io_vAngle1.y,io_vAngle2.y);
		NearestAngle(io_vAngle1.z,io_vAngle2.z);
	}*/



	/*static void NearestAngle(float& io_nAngle1, float& io_nAngle2)
	{
		while(io_nAngle1>D3DX_PI*2.0f)io_nAngle1-=D3DX_PI*2.0f;
		while(io_nAngle1<0.0f)io_nAngle1+=D3DX_PI*2.0f;
		while(io_nAngle2>D3DX_PI*2.0f)io_nAngle2-=D3DX_PI*2.0f;
		while(io_nAngle2<0.0f)io_nAngle2+=D3DX_PI*2.0f;

		if (io_nAngle1 < io_nAngle2)
		{
			NearestAngle_Ordered(io_nAngle1, io_nAngle2);
		}
		else
		{
			NearestAngle_Ordered(io_nAngle2, io_nAngle1);
		}
	}*/

	/*static void NearestAngle_Ordered(float& io_nAngle1, float& io_nAngle2)
	{

		float nDiff = io_nAngle2 - io_nAngle1;

		if (nDiff > D3DX_PI)
		{
			io_nAngle1 += 2.0f*D3DX_PI;
		}
	}*/



	class InsensitiveCompare {
		public:
			bool operator()(const std::string& s1,const std::string& s2)const{
				return Util::LowerCase(s1).compare(Util::LowerCase(s2)) < 0;
			}
	};

	static int CharToHex(int c){
		if(c>=0 && c<=9){
			return c+'0';
		}else if(c>=10 && c<=15){
			return c-10+'a';
		}else {
			return '0';
		}
	}

	static int HexToChar(int h){
		h=::tolower(h);
		if(h>='0' && h<='9'){
			return h-'0';
		}else if(h>='a' && h<='f'){
			return h-'a'+10;
		}else{
			return 0;
		}
	}

	template<class T>
	static T Clamp (const T& v, const T& m, const T& M){
		return ( v<m ) ? ( m ) : ( (v>M)?(M):(v) );
	}
};

#endif