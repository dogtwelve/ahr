#ifndef _IMAGEPROCESS_H_
#define _IMAGEPROCESS_H_

class CSurface;

class CImageProcess {
public:
	enum EGradiant {
		EGradiant_Vertical,
		EGradiant_Horizontal,
	};

	enum {
		kMapRange=255
	};
	
	static void Colorize(CSurface& dest, CSurface& grayscale, int darkColor=0, int lightColor=0xffffffff);
	static void Modulate(CSurface& dest, CSurface& grayscale);
	static void AlphaBlend(CSurface& dest, CSurface& fore);
	static void Compose(CSurface& dest, CSurface& grayscale);

	static void SolidFill(CSurface& dest, int color);
	static void GradiantFill(CSurface& dest, EGradiant type, int color1, int color2, int start=0, int end=255);
protected:
	static inline int LerpRGB(int c1, int c2, int x);
	static inline int LerpARGB(int c1, int c2, int x);
};


#endif