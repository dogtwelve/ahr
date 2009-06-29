#ifndef _TGAREADER_H_
#define _TGAREADER_H_

#include <string>
#include <fstream>

class IGenericFile;
class CSurface;

class CTgaReader
{
	public:
	enum ETgaType {
		ETgaType_None=0,
		ETgaType_Indexed,
		ETgaType_RGB,
		ETgaType_Grey
	};
	struct SHeader
	{
		SHeader();
		void Read(std::ifstream&);

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

	CTgaReader();
	CTgaReader::~CTgaReader();

	bool Read(const std::string&, CSurface& surface);
	bool ReadHeader();
	bool ReadData();

	short GetWidth() const {return(m_Header.Width);}
	short GetHeight() const {return(m_Header.Height);}
	char  GetDepth() const {return(m_Header.Bits);}
	void ConvertToARGB();


private:

	std::string m_FileName;
	SHeader m_Header;
	std::ifstream m_File;

	//CSurface& m_Surface;
};

#endif