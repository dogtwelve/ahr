#include "ASEReader.h"
#include "ASEException.h"
#include "ASEMesh.h"
#include "ASEMap.h"

#include <iostream>



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	try
	{
		ASEReader reader("TestObject.ase");



		for(int i=0;i<reader.NbMeshes();++i)
		{
			const ASEMesh& mesh = reader.GetMesh(i);








			std::cout << "Mesh " << mesh.Name() << "\n";

			for(ASEMesh::FaceIterator faceIter = mesh.FaceBegin();faceIter!=mesh.FaceEnd();++faceIter)
			{
				const ASEFace& face = *faceIter;






				std::cout << "   F ";
			}
		}

		int a=0;
	}
	catch(ASEException& e)
	{
		std::cerr << e.GetMessage() << "\n";
	}
	return 0;
}

