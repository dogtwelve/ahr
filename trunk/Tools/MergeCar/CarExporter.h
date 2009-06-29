#ifndef _CAREXPORTER_H_
#define _CAREXPORTER_H_

#include <string>
#include <vector>

class CCarExporter {
public:
	CCarExporter(const std::string& in, const std::string& out){SetPaths(in,out);}
	void Export(const std::string& inName, const std::string& outName);

	void SetPaths(const std::string& in, const std::string& out){m_InPath=in; m_OutPath=out;}
protected:
	enum ELayerType {
		ELayerType_Normal=0,
		ELayerType_Mask,
		ELayerType_Overlay,
		ELayerType_Set,
		ELayerType_Unknown
	};

	void WriteString(std::ofstream& file, const std::string& str);
	ELayerType GetLayerTypeFromName(const std::string& str);


	bool ExportTexture(const std::string& iFile, const std::string& oFile, ELayerType type);

	std::string m_InPath;
	std::string m_OutPath;
};


#endif