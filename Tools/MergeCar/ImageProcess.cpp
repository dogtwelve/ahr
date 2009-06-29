
#include "ImageProcess.h"
#include "Surface.h"



void CImageProcess::Colorize(CSurface& dest, CSurface& grayscale, int darkColor, int lightColor)
{
	int w = dest.GetWidth();
	int h = dest.GetHeight();

	unsigned int * destPix = (unsigned int*)  dest.GetDataPointer();
	unsigned char* grayPix = (unsigned char*) grayscale.GetDataPointer();

	for(int i=0;i<w*h;i++){
		

		// Read scale from grayscale surface
		int scale = *grayPix;

		int color;
		if(scale>=128){
			// Overscale to white
			scale-=128;
			scale<<=1;

			color = LerpRGB(*destPix,lightColor,scale);
		
		}else{
			// Modulation
			scale<<=1;
			
			color = LerpRGB(darkColor,*destPix,scale);
		}

		// Write result to destination surface
		*destPix=(*destPix&0xff000000)|color;

		destPix++;
		grayPix++;
	}
	
}


void CImageProcess::Modulate(CSurface& dest, CSurface& grayscale)
{
	int w = dest.GetWidth();
	int h = dest.GetHeight();

	// Initialize surfaces pointers
	unsigned int * destPix = (unsigned int*) dest.GetDataPointer();
	unsigned int * grayPix = (unsigned int*) grayscale.GetDataPointer();

	for(int i=0;i<w*h;i++){
		
		// Read scale from grayscale surface
		int scale = *grayPix;

		// Write result to destination surface
		*destPix=(*destPix&0xff000000)|LerpRGB(0,*destPix,scale);

		destPix++;
		grayPix++;
	}
}


void CImageProcess::AlphaBlend(CSurface& dest, CSurface& fore)
{
	int w = dest.GetWidth();
	int h = dest.GetHeight();

	// Initialize surfaces pointers
	unsigned int * destPix = (unsigned int*) dest.GetDataPointer();
	unsigned int * forePix = (unsigned int*) fore.GetDataPointer();

	for(int i=0;i<w*h;i++){
		
		// Read scale from grayscale surface
		int scale=((*forePix)>>24)&0xff;

		// Write result to destination surface
		*destPix=(*destPix&0xff000000)|LerpRGB(*destPix,*forePix,scale);

		destPix++;
		forePix++;
	}


}



// Compose a D{ARGB} surface from a S{RGB} and T{A} surface
// Da = Ta
// Dr = Sr
// Dg = Sg
// Db = Sb
void CImageProcess::Compose(CSurface& dest, CSurface& gray)
{
	int w = dest.GetWidth();
	int h = dest.GetHeight();

	// Initialize surfaces pointers
	unsigned int * destPix = (unsigned int*)  dest.GetDataPointer();
	unsigned char* grayPix = (unsigned char*) gray.GetDataPointer();

	for(int i=0;i<w*h;i++){
		
		*destPix=(*grayPix<<24) | (*destPix&0x00ffffff);

		destPix++;
		grayPix++;
	}
}

void CImageProcess::SolidFill(CSurface& dest, int color)
{
	int w = dest.GetWidth();
	int h = dest.GetHeight();

	// Initialize surfaces pointers
	unsigned int * destPix = (unsigned int*)  dest.GetDataPointer();

	for(int i=0;i<w*h;i++){
		*destPix=color;
		destPix++;
	}
}


void CImageProcess::GradiantFill(CSurface& dest, EGradiant type, int color1, int color2, int start, int end)
{
	int w = dest.GetWidth();
	int h = dest.GetHeight();


	// Initialize surfaces pointers
	unsigned int * destPix = (unsigned int*)  dest.GetDataPointer();

	
	int scale = 0;
	for(int y=0;y<h;y++){
		if(type==EGradiant_Vertical){
			scale = -start + y*h/(end-start);
			if(scale<0)scale=0;
			if(scale>kMapRange)scale=kMapRange;
		}
		for(int x=0;x<w;x++){
			
			if(type==EGradiant_Horizontal){
				scale = -start + x*w/(end-start);
				if(scale<0)scale=0;
				if(scale>kMapRange)scale=kMapRange;
			}

			// Write result to destination surface
			*destPix=LerpARGB(color1,color2,scale);

			destPix++;
		}
	}

}

int CImageProcess::LerpRGB(int c1, int c2, int x)
{
	const int r1 = (c1>>16) & 0xff;
	const int g1 = (c1>>8) & 0xff;
	const int b1 = (c1) & 0xff;

	const int r2 = (c2>>16) & 0xff;
	const int g2 = (c2>>8) & 0xff;
	const int b2 = (c2) & 0xff;

	const int r = r1 + (x*(r2-r1)) / 255;
	const int g = g1 + (x*(g2-g1)) / 255;
	const int b = b1 + (x*(b2-b1)) / 255;

	return (r<<16)|(g<<8)|(b);
}

int CImageProcess::LerpARGB(int c1, int c2, int x)
{
	const int a1 = (c1>>24) & 0xff;
	const int r1 = (c1>>16) & 0xff;
	const int g1 = (c1>>8) & 0xff;
	const int b1 = (c1) & 0xff;

	const int a2 = (c2>>24) & 0xff;
	const int r2 = (c2>>16) & 0xff;
	const int g2 = (c2>>8) & 0xff;
	const int b2 = (c2) & 0xff;

	const int a = a1 + (x*(a2-a1)) / 255;
	const int r = r1 + (x*(r2-r1)) / 255;
	const int g = g1 + (x*(g2-g1)) / 255;
	const int b = b1 + (x*(b2-b1)) / 255;

	return (a<<24)|(r<<16)|(g<<8)|(b);
}
