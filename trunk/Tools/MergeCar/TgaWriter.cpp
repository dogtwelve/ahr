#include "TgaWriter.h"
#include "surface.h"


CTgaWriter::SHeader::SHeader()
{
	memset(this,sizeof(this),0);
}


CTgaWriter::CTgaWriter():
m_Header()
{
}


CTgaWriter::~CTgaWriter()
{
}


bool CTgaWriter::Write(const std::string& fileName, CSurface& surface)
{

	m_FileName=fileName;
	//file = IGenericFile::Open(fileName.c_str(), IGenericFile::OPEN_READ | IGenericFile::OPEN_BINARY | IGenericFile::OPEN_STANDALONE, false, true);
	m_File.open(fileName.c_str(),std::ios::out|std::ios::binary);
	if(!m_File)return false;

	memset(&m_Header,0,sizeof(m_Header));
	

	m_Header.Width=surface.GetWidth();
	m_Header.Height=surface.GetHeight();
	m_Header.Bits=surface.GetDepth()<<3;

	if(surface.GetFormat()==CSurface::EFormat_A8R8G8B8 || surface.GetFormat()==CSurface::EFormat_R8G8B8 || surface.GetFormat()==CSurface::EFormat_X8R8G8B8){
		m_Header.ImageType=2;
	}else if(surface.GetFormat()==CSurface::EFormat_S8){
		m_Header.ImageType=3;
	}


	// Write header
	m_Header.Write(m_File);
	if(!m_File)return false;

	// Write data
	m_File.write((char*)surface.GetDataPointer(),surface.GetSize());

	// Close and bail out
	if(!m_File)return false;
	m_File.close();
	return true;


}

void CTgaWriter::SHeader::Write(std::ofstream& file)
{

	file.write((char*)&(IdentSize), sizeof(IdentSize));
	file.write((char*)&(ColorMapType), sizeof(ColorMapType));
	file.write((char*)&(ImageType), sizeof(ImageType));
	file.write((char*)&(ColorMapStart), sizeof(ColorMapStart));
	file.write((char*)&(ColorMapLength), sizeof(ColorMapLength));
	file.write((char*)&(ColorMapBits), sizeof(ColorMapBits));
	file.write((char*)&(XStart), sizeof(XStart));
	file.write((char*)&(YStart), sizeof(YStart));
	file.write((char*)&(Width), sizeof(Width));
	file.write((char*)&(Height), sizeof(Height));
	file.write((char*)&(Bits), sizeof(Bits));
	file.write((char*)&(Descriptor), sizeof(Descriptor));
}

/*
void CTgaReader::ConvertToARGB()
{
	for(int i=0; i<in_nPixels; ++i)
	{
		int nPix = i*bytBytesPerPixel;

		unsigned char temp = in_pArray[nPix + 0];

		m_Data[nPix + 0] = in_pArray[nPix + 2];
		m_Data[nPix + 2] = temp;
	}
}*/


