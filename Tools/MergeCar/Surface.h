#ifndef _SURFACE_H_
#define _SURFACE_H_



class CPalette;

class CSurface {

public:
	enum EFormat {
		EFormat_Unknown=0,

		// 8 bits
		EFormat_S8,
		
		// 16 bits
		EFormat_X4R4G4B4,
		EFormat_S4R4G4B4,
		
		// 24 bits
		EFormat_R8G8B8,

		// 32 bits
		EFormat_A8R8G8B8,
		EFormat_X8R8G8B8,
		EFormat_D16S4R4G4B4,

		EFormat_Invalid = -1
	};




	// Tors
	CSurface();
	CSurface(int width, int height, EFormat depth);
	explicit CSurface(const CSurface& surface);

	void Reset(int width, int height, EFormat format);

	~CSurface();

	void Release();


	// Attributes
	int GetWidth(){return m_Width;}
	int GetHeight(){return m_Height;}
	int GetPitch(){return m_Pitch;}
	int GetDepth(){return m_Depth;}
	int GetSize(){return (m_Width*m_Height*m_Depth);}
	EFormat GetFormat(){return m_Format;}
	unsigned char* GetDataPointer(){return m_Data;}
	CPalette* GetPalette(){return m_Palette;}

protected:

	int m_Width;
	int m_Height;
	int m_Pitch;
	int m_Depth;
	EFormat m_Format;
	unsigned char * m_Data;
	CPalette* m_Palette;

	static int GetFormatDepth(EFormat format);
};


class CPalette {
public:
	CPalette(int size=0);
	~CPalette();
	unsigned int * GetDataPointer(){return m_Data;}
	int GetSize(){return m_Size;}
	void Reset(int size);

protected:
	void Release();

	int m_Size;
	unsigned int * m_Data;

};

#endif