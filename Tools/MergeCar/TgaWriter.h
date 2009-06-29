#ifndef _TGAWRITER_H_
#define _TGAWRITER_H_

#include <string>
#include <fstream>

class CSurface;

class CTgaWriter
{
	public:
	struct SHeader
	{
		SHeader();
		void Write(std::ofstream&);

		char  IdentSize;		//size of ID field that follows 18 byte header (0 usually)
		char  ColorMapType;		//type of colour map 0=none, 1=has palette
		char  ImageType;		//type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed
		short ColorMapStart;	//first colour map entry in palette
		short ColorMapLength;	//number of colours in palette
		char  ColorMapBits;		//number of bits per palette entry 15,16,24,32
		short XStart;			//image x origin
		short YStart;			//image y origin
		unsigned short Width;	//image width in pixels
		short Height;			//image height in pixels
		char  Bits;				//image bits per pixel 8,16,24,32
		char  Descriptor;		//image descriptor bits (vh flip bits)
        
	};

	CTgaWriter();
	CTgaWriter::~CTgaWriter();

	bool Write(const std::string&, CSurface& surface);

	short GetWidth() const {return(m_Header.Width);}
	short GetHeight() const {return(m_Header.Height);}
	char  GetDepth() const {return(m_Header.Bits);}

private:

	std::string m_FileName;
	SHeader m_Header;
	std::ofstream m_File;

	//CSurface& m_Surface;
};

#endif