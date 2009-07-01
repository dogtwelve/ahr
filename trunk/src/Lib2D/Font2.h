#ifndef _FONT_H_
#define _FONT_H_

#include <string>
#include "Lib2D/Rect.h"
#include "UniqueResource.h" 
//#include <string>

//namespace Lib2D {

// Forward
class CLib2D;

#ifdef __WIN32__
	using namespace std;
#endif


/*class CFontContext {
	// Tors
	CFontContext(const string& name);
	CFontContext(L2Font*);
	CFontContext(const CFontContext&);
	CFontContext& operator = (const CFontContext&);
	

public:
	void SetColor(int color){m_Color=color};
	void SetOpacity(int opacity){m_Opacity=opacity;}
	void SetClip(bool val){m_Clip=val;}
	void SetClipEntireChar(bool val){m_ClipEntireChar=val;}
	void SetWrap(bool val){m_Wrap=val;}
	void SetLineSpacing(int val){m_LineSpacing=val;}
	void SetCharacterSpacing(int val){m_CharacterSpacing=val;}
	void SetAlign(ETextAlign align){m_Align=align;}
	void SetVerticalAlign(ETextVerticalAlign align){m_VerticalAlign=align;}
	void SetClipRect(const ARect* rect);

	int GetColor(){return m_Color;}
	int GetOpcaity(){return m_Opacity;}
	bool GetClip(){return m_Clip;}
	bool GetClipEntireChar(){return m_ClipEntireChar;}
	bool GetWrap(){return m_Wrap;}
	int GetLineSpacing(){return m_LineSpacing;}
	int GetCharacterSpacing(){return m_CharacterSpacing;}
	ETextAlign GetAlign(){return m_Align;}
	ETextVerticalAlign GetVerticalAlign(){return m_VerticalAlign;}
	const ARect& GetClipRect(){return m_ClipRect;}


protected:


	// Print options
	int m_LineSpacing;
	int m_CharacterSpacing;
	ETextAlign m_Align;
	ETextVerticalAlign m_VerticalAlign;
	bool m_Clip;
	bool m_Wrap;
	bool m_ClipEntireChar;
	ARect m_ClipRect;
	ARect m_DrawTextRect;
	int m_Opacity;
	int m_Color;
	
	L2Font* m_Font;

};*/

/*
class L2Font
{
	enum { k_nColorInvalid = -1 };
public:


enum ETextAlign {
	ETextAlign_Left=0,
	ETextAlign_Right,
	ETextAlign_Center,
	ETextAlign_Justify,
	ETextAlign_Default=ETextAlign_Left
};

enum ETextVerticalAlign {
	ETextVerticalAlign_Top=0,
	ETextVerticalAlign_Middle,
	ETextVerticalAlign_Bottom,
	ETextVerticalAlign_Default=ETextVerticalAlign_Top
};

enum EColorMode {
	EColorMode_Normal=0,
	EColorMode_Overlay,
	EColorMode_Fast,
	EColorMode_None,
	EColorMode_Default=EColorMode_Normal
};


	// Construction
	explicit L2Font(const std::string& fontname, CLib2D* lib2d=0, EColorMode nColorMode=EColorMode_Default);

	~L2Font();
	int LoadFont(const char* filename);
	void Unload();

	void Reset();
	
	// Operations
	void DrawChar(int x, int y, unsigned char c);
	void DrawString(int x,int y,const char* s);
	void DrawString(int x,int y,const std::string& s){DrawString(x, y, s.c_str());}
	void DrawStringCentered(int x,int y,const char* s);
	void DrawStringCentered(int x,int y,const std::string& s){DrawStringCentered(x, y, s.c_str());}
	void DrawText(const ARect& rect, const char* s);
	void DrawText(const ARect& rect, const std::string& s){DrawText(rect, s.c_str());}
	void DrawText(const char* s){ DrawText(ARect(0,0,m_ScreenWidth,m_ScreenHeight),s); }
	void DrawText(const std::string& s){DrawText(s);}
	void DrawNumber(int x,int y,int in_nNumber);

	int GetCharWidth(char in_C);
	int GetTextWidth(const char* str);
	int GetTextWidth(const std::string& str){return GetTextWidth(str.c_str());}
	int GetTextHeight(const char* str);
	int GetTextHeight(const std::string& str){return GetTextHeight(str.c_str());}

	void WrapText(char* io_pString, int in_nWidth);
	void WrapText(std::string& io_str, int in_nWidth){WrapText(&*(io_str.begin()), in_nWidth);}
	void TruncText(std::string& io_str, int in_nWidth);


	// Context
	void SetLib2D(const CLib2D& lib2d);
	void SetScreenBuffer(unsigned short * ptr, int w, int h){m_Screen=ptr; m_ScreenWidth=w; m_ScreenHeight=h;}


	// Attributes
	void SetColor(int color, bool forceRefresh=false);
	void SetColors(int color1, int color2, bool forceRefresh=false);
	void SetOpacity(int opacity){m_Opacity=opacity;}
	void SetClip(bool val){m_Clip=val;}
	void SetClipEntireChar(bool val){m_ClipEntireChar=val;}
	void SetWrap(bool val){m_Wrap=val;}
	void SetLineSpacing(int val){m_LineSpacing=val;}
	void SetCharacterSpacing(int val){m_CharacterSpacing=val;}
	void SetAlign(ETextAlign align){m_Align=align;}
	void SetVerticalAlign(ETextVerticalAlign align){m_VerticalAlign=align;}
	void ResetClipRect() {SetClip(false); SetClipRect(0);}
	void SetClipRect(const ARect* rect);
	void SetColorMode(EColorMode mode);


	bool GetClip()const{return m_Clip;}
	bool GetClipEntireChar()const{return m_ClipEntireChar;}
	bool GetWrap()const{return m_Wrap;}
	int GetLineSpacing()const{return m_LineSpacing;}
	int GetCharacterSpacing()const{return m_CharacterSpacing;}
	ETextAlign GetAlign()const{return m_Align;}
	ETextVerticalAlign GetVerticalAlign()const{return m_VerticalAlign;}
	EColorMode GetColorMode()const{return m_ColorMode;}
	const ARect& GetClipRect()const{return m_ClipRect;}

	char QueryLastUnclippedChar()const{return m_QueryLastUnclippedChar;}

	inline char AnsiToLocal(char c)const{return m_AnsiToFont[c];}
	inline char LocalToAnsi(char c)const{return m_ConvertString[c];}



private:

	enum {
		kColorTableSize=64,
		kColorTableShift=6,
		kColorTableHalf=31,
		kColorTableMask=63
	};

	void SetConvertString(const unsigned char* str);
	static inline int LerpARGB(int c1, int c2, int x);
	// Data members
	ARect* m_CharRect;
	int m_CharCount;

	
	// Font data
	unsigned short * m_Data;
	int m_DataWidth;
	int m_DataHeight;
	int m_FontHeight;

	
	// Common data
	std::string m_ConvertString;
	unsigned char m_AnsiToFont[256];
	unsigned char m_AlphaTable[256];

	
	// Context data
	unsigned short * m_Screen;
	int m_ScreenWidth;
	int m_ScreenHeight;
	const CLib2D* m_Lib2D;
	char m_QueryLastUnclippedChar;


	// Print options
	int m_LineSpacing;
	int m_CharacterSpacing;
	ETextAlign m_Align;
	ETextVerticalAlign m_VerticalAlign;
	bool m_Clip;
	bool m_Wrap;
	bool m_ClipEntireChar;
	ARect m_ClipRect;
	ARect m_DrawTextRect;
	int m_Opacity;

	// Colors
	EColorMode m_ColorMode;
	int m_ColorTable[kColorTableSize];
	int m_nCurrentColor1;
	int m_nCurrentColor2;

};
*/

//}

#endif
