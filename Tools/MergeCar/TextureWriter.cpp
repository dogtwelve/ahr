#include "TextureWriter.h"
#include "surface.h"


CTextureWriter::CTextureWriter()
{
}


CTextureWriter::~CTextureWriter()
{
}


bool CTextureWriter::Write(const std::string& fileName, CSurface& surface)
{

	m_FileName=fileName;
	//file = IGenericFile::Open(fileName.c_str(), IGenericFile::OPEN_READ | IGenericFile::OPEN_BINARY | IGenericFile::OPEN_STANDALONE, false, true);
	m_File.open(fileName.c_str(),std::ios::out|std::ios::binary);
	if(!m_File)return false;

	int integer;

	// Width
	integer = surface.GetWidth();
	m_File.write((char*)&integer,sizeof(integer));
	if(!m_File)return false;

	// Height
	integer = surface.GetHeight();
	m_File.write((char*)&integer,sizeof(integer));
	if(!m_File)return false;

	// Format
	integer = (int)surface.GetFormat();
	m_File.write((char*)&integer,sizeof(integer));
	if(!m_File)return false;
	
	// Write data
	m_File.write((char*)surface.GetDataPointer(),surface.GetSize());

	// Close and bail out	
	if(!m_File)return false;
	m_File.close();
	return true;

}

