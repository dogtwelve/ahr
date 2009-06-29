#include "surface.h"
#include <cstdlib>
#include <string>

CSurface::CSurface():
m_Data(0),
m_Palette(0)
{
	Reset(0,0,EFormat_Unknown);
}

CSurface::CSurface(int width, int height, EFormat format):
m_Data(0),
m_Palette(0)
{
	Reset(width,height,format);
}

CSurface::CSurface(const CSurface& source):
m_Data(0),
m_Palette(0)
{
	Reset(source.m_Width, source.m_Height, source.m_Format);
	memcpy(m_Data,source.m_Data,GetSize());
}

CSurface::~CSurface()
{
	Release();
}

void CSurface::Reset(int width, int height, EFormat format)
{
	Release();

	m_Width=width;
	m_Height=height;
	m_Depth=GetFormatDepth(format);
	m_Format=format;

	if(GetSize()){
		m_Data=new unsigned char [GetSize()];
	}
}


int CSurface::GetFormatDepth(EFormat format)
{
	switch(format){
		// 8 bits
		case EFormat_S8:
			return 1;
		
		// 16 bits
		case EFormat_X4R4G4B4:
		case EFormat_S4R4G4B4:
			return 2;
		
		// 24 bits
		case EFormat_R8G8B8:
			return 3;

		// 32 bits
		case EFormat_A8R8G8B8:
		case EFormat_X8R8G8B8:
		case EFormat_D16S4R4G4B4:
			return 4;

		case EFormat_Unknown:
		default:
			return 0;
	}
}



void CSurface::Release()
{
	delete[] m_Data;
	m_Data=0;
}



CPalette::CPalette(int size):
m_Data(0),
m_Size(0)
{
	Reset(size);
}

CPalette::~CPalette()
{
	Release();
}

void CPalette::Reset(int size)
{
	Release();

	if(size){
		m_Data = new unsigned int [size];
	}
}

void CPalette::Release()
{
	delete [] m_Data;
	m_Data=0;
	m_Size=0;
}


