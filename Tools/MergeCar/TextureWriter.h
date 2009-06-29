#ifndef _TEXTUREWRITER_H_
#define _TEXTUREWRITER_H_

#include <string>
#include <fstream>

class CSurface;

class CTextureWriter
{
	public:

	CTextureWriter();
	CTextureWriter::~CTextureWriter();

	bool Write(const std::string&, CSurface& surface);

private:

	std::string m_FileName;
	std::ofstream m_File;

};

#endif