// MergeCar.cpp : Defines the entry point for the console application.
//
#pragma warning (disable:4786)
#include "stdafx.h"
#include "stdio.h"
#include "TgaReader.h"
#include "TgaWriter.h"
#include "ImageProcess.h"
#include "Surface.h"

#include "windows.h"
#include "CarExporter.h"

int main(int argc, char* argv[])
{
	std::string name;
	std::string inPath;
	std::string outPath;
	std::string inFile;
	std::string outFile;
	
	if(argc<2){
		printf("Usage: MergeCar <car name> [input path [output path]]\n");
		return 1;
	}else{
		if(argc>=2){
			name=argv[1];
		}
		if(argc>=3){
			inPath=argv[2];
		}else{
			char str [1024];
			::GetCurrentDirectory(1024,str);
			inPath=str;
		}
		if(argc>=4){
			outPath=argv[3];
		}else{
			outPath=inPath;
		}
	}
	

	inFile=name+".txt";
	outFile=name+".car";

	CCarExporter exporter(inPath, outPath);
	exporter.Export(inFile, outFile);


	return 0;

}





/*

class CLayer {
	virtual void Apply(CSurface& surface);
};



class CLayerSet : public CLayer {
public:
	virtual void Apply(CSurface& surface);
protected:
	vector<CLayer*> m_Layers;
};



class CNormalLayer : public CLayer {
public:
	virtual void Apply(CSurface& surface);
protected:
	string m_File;
};



class CMaskLayer : public CLayer {
public:
	virtual void Apply(CSurface& surface);
protected:
	string m_File;
	int m_Color1;
	int m_Color2;
};



class COverlayLayer : public CLayer {
public:
	virtual void Apply(CSurface& surface);
protected:
	string m_File;
};




class CBackgroundLayer : public CLayer {
public:
	virtual void Apply(CSurface& surface);
protected:
	int m_Color1;
	int m_Color2;
};


*/