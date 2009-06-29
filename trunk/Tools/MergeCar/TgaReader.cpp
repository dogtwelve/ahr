#include "TgaReader.h"
#include "surface.h"


CTgaReader::SHeader::SHeader()
{
	memset(this,sizeof(this),0);
}


CTgaReader::CTgaReader():
m_Header()
{
}


CTgaReader::~CTgaReader()
{
}


bool CTgaReader::Read(const std::string& fileName, CSurface& surface)
{

	m_FileName=fileName;
	//file = IGenericFile::Open(fileName.c_str(), IGenericFile::OPEN_READ | IGenericFile::OPEN_BINARY | IGenericFile::OPEN_STANDALONE, false, true);
	m_File.open(fileName.c_str(),std::ios::in|std::ios::binary);

	if(!m_File)return false;
	
	char skip [256];
	
	// Read header
	m_Header.Read(m_File);

	if(!m_File)return false;

	// Skip identifier
	if(m_Header.IdentSize){
		m_File.read(skip,m_Header.IdentSize);
	}

	if(!m_File)return false;
	
	// Build surface
	// + Determine format
	CSurface::EFormat format=CSurface::EFormat_Unknown;
	if(m_Header.ImageType==ETgaType_RGB){
		if(GetDepth()==24 ){
			format=CSurface::EFormat_R8G8B8;
		}else if(GetDepth()==32){
			format=CSurface::EFormat_A8R8G8B8;
		}
	}else if(m_Header.ImageType==ETgaType_Grey){
		format=CSurface::EFormat_S8;
	}
	// + Reset 
	surface.Reset(GetWidth(),GetHeight(),format);


	// Read data
	m_File.read((char*)surface.GetDataPointer(),GetWidth()*GetHeight()*((GetDepth()+7)/8));

	// Close and bail out
	if(!m_File)return false;
	m_File.close();
	return true;

	
}

void CTgaReader::SHeader::Read(std::ifstream& file)
{

	file.read((char*)&(IdentSize), sizeof(IdentSize));
	file.read((char*)&(ColorMapType), sizeof(ColorMapType));
	file.read((char*)&(ImageType), sizeof(ImageType));
	file.read((char*)&(ColorMapStart), sizeof(ColorMapStart));
	file.read((char*)&(ColorMapLength), sizeof(ColorMapLength));
	file.read((char*)&(ColorMapBits), sizeof(ColorMapBits));
	file.read((char*)&(XStart), sizeof(XStart));
	file.read((char*)&(YStart), sizeof(YStart));
	file.read((char*)&(Width), sizeof(Width));
	file.read((char*)&(Height), sizeof(Height));
	file.read((char*)&(Bits), sizeof(Bits));
	file.read((char*)&(Descriptor), sizeof(Descriptor));
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


