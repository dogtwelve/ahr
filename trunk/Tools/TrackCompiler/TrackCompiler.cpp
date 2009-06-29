#include "..\Libase\ASEException.h"

#include "OutFile.h"
#include "Track.h"

#include <iostream>

/******************************************************************************
	Track File Format:
 ******************************************************************************

	TRACK
		TEXTURES
		SECTIONS

	TEXTURES
		int						nbrtextures		//	number of textures
		int[nbrtextures]		offsets			//	offset for each texture in the "textureNames" array
		int						nameSize		// size of the array of texture names
		char[nameSize]			textureNames	// all textures file name (null terminated)


	SECTIONS
		VERTEXES				trackVertex
		VERTEXES				fenceVertex
		int						nbSegments
		SEGMENT[nbSegments]
		int						next main track index
		int						next alt track index
		int						next alt track index


	VERTEXES
		int						nbVertex
		VERTEX[nbVertex]		vertexes

	VERTEX
		int						x
		int						y
		int						z

	SEGMENT
		uchar[kSectionWidth]	textureIndex	


******************************************************************************/



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
int main(int argc,const char** argv)
{
	std::string fileName = "D:\\sources\\Asphalt2\\data\\Track\\track0\\track0";

	if(argc>1)
		fileName = argv[1];
	else
	{
		#ifdef NDEBUG
			std::cerr << "*** ERROR: " << argv[0] << "Bad parameters\n";
			exit(1);
		#endif
	}

	try
	{		
		CTrack	track(fileName);
		track.Save(OutFile((fileName + ".vtrack").c_str()));
	}
	catch(const ASEException& e)
	{
		std::cerr << "*** ERROR: " << argv[0] << " " << e.GetMessage() << "\n";
	}	
	return 0;
}



