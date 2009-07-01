#include "Keyframed.h"
#include "File.h"
#include "Face.h"
#include "Lib3d.h"
#include "Mesh.h"
#include "devutil.h"
#include <vector>

#ifndef NULL
#define NULL 0L
#endif

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void CreateFace(File& file,Lib3D::TFace& face,Lib3D::TVertex* vertex,const TTexture* texture)
{
	int iA = file.ReadInt();

	face.VxA = (Vector4s*)(vertex + iA);
	face.uA = file.ReadInt();
	face.vA = file.ReadInt();


	int iB = file.ReadInt();
	face.VxB = (Vector4s*)(vertex + iB);
	face.uB = file.ReadInt();
	face.vB = file.ReadInt();

	int iC = file.ReadInt();
	face.VxC = (Vector4s*)(vertex + iC);
	face.uC  = file.ReadInt();
	face.vC  = file.ReadInt();

	// the normal is ignored
	file.ReadInt();
	file.ReadInt();
	file.ReadInt();

	face.m_texture = texture;

	//	Faces are not realy double-sided, but the ordinary face
	//	culling test can't be used for an object that has it's 
	//	own transformation without transforming the normal
	//SetDoubleSided(true);
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void ShiftUV(Lib3D::TFace& face,int shift)
{
	face.uA>>=shift;
	face.vA>>=shift;
	
	face.uB>>=shift;
	face.vB>>=shift;

	face.uC>>=shift;
	face.vC>>=shift;
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class TKeyFramed::SubMesh
	:public Lib3D::CMesh
{
public:
	SubMesh(const char* fileName,const char* textureName,const char* keyFrameFileName,int startFrame)
		:CMesh("KeyFramed",0,0,0,0,0,0,0,false),
		m_posIndex(0),
		m_startFrame(startFrame),
		m_rotIndex(0),
		m_started(false)
	{	
		int i;

		File file(fileName);

		i = file.ReadInt();
		A_ASSERT(i==1);

		i = file.ReadInt();	// unused blockId

		// Vertexes
		m_nbVertex = file.ReadInt();
		m_vertex = NEW TVertex[m_nbVertex];

		for(i=0;i<m_nbVertex;i++)
		{
			Vector4s& v = m_vertex[i].InitialPos;

			v.x = file.ReadInt() >> 1;
			v.y = file.ReadInt() >> 1;
			v.z = file.ReadInt() >> 1;

		}

		m_texture = NEW TTexture(textureName, false);

		// submeshes
		int nbSubMesh = file.ReadInt();
		A_ASSERT(nbSubMesh==1);

		
		int meshId		= file.ReadInt();
		int doubleFace	= file.ReadInt();
		

		m_nbFaces = file.ReadInt();		
		m_face = NEW Lib3D::TFace[m_nbFaces];

		for(i=0;i<m_nbFaces;i++)
		{
			CreateFace(file,m_face[i],m_vertex,m_texture);
		}

		SetUV(m_texture);

		file.Close();


		if(keyFrameFileName)
		{
			File keyFrame(keyFrameFileName);

			int nbPos = keyFrame.ReadInt();

			m_positions.resize(nbPos);
			for(i=0;i<nbPos;i++)
			{
				m_positions[i].x = -keyFrame.ReadInt()>>1;
				m_positions[i].y = keyFrame.ReadInt()>>1;
				m_positions[i].z = -keyFrame.ReadInt()>>1;
				
			}

			nbPos = keyFrame.ReadInt();

			m_rotations.resize(nbPos);
			for(i=0;i<nbPos;i++)
			{
				m_rotations[i].x = keyFrame.ReadInt();
				m_rotations[i].y = 1024 - keyFrame.ReadInt();
				m_rotations[i].z = keyFrame.ReadInt();			

			}
		}
	}
	
	virtual ~SubMesh()
	{
		MM_DELETE m_texture;
	}


	void SetUV(const Lib3D::TTexture* texture)
	{
		int res = texture->SizeX();

		if(res==256)
			return;

		A_ASSERT(res<256);
		int shift = 0;
		A_ASSERT(res);
		while(res != 256)
		{
			res <<=1;
			shift++;
		}
		
		for(int i = 0;i<m_nbFaces;i++)
			ShiftUV(m_face[i],shift);
	}

	virtual void Draw(Lib3D::CLib3D& lib3d,int sectionId)
	{
		if(m_startFrame==sectionId)
			m_started=true;
		
		if(m_started)
		{	
			m_posIndex++;
			m_rotIndex++;

			if((unsigned int)m_posIndex >= m_positions.size())
				m_posIndex = 0;

			if((unsigned int)m_rotIndex >= m_rotations.size())
				m_rotIndex = 0;
		}
		
		CMatrix44& matrix =lib3d.PushMatrix();		
			
		SetPosition(matrix);
		SetRotation(matrix);

		lib3d.DrawMeshOTFace(*this,0);

		lib3d.PopMatrix();
	}



	virtual void SetPosition(CMatrix44& matrix)
	{
		if(m_positions.size()>0)
		{
			const Vector4s& v = m_positions[m_posIndex];
			matrix.Translate(v);
		}
	}

	virtual void SetRotation(CMatrix44& matrix)
	{
		if(m_rotations.size()>0)
		{
			const Vector4s& euler = m_rotations[m_rotIndex];

			matrix.RotateY(euler.y);
			matrix.RotateX(euler.x);
			matrix.RotateZ(euler.z);
		}
	}

private:
	Lib3D::TTexture* m_texture;

	int m_startFrame;
	int m_posIndex;
	int m_rotIndex;

	bool m_started;


	std::vector<Vector4s> m_positions;
	std::vector<Vector4s> m_rotations;	
};






class Train
	:public TKeyFramed::SubMesh
{
public:

	Train():TKeyFramed::SubMesh("objects\\bag_train.vobj","textures\\track10\\Bag_Locomotive.RLE","objects\\bag_train_anim.kframe",0){}

	virtual void SetRotation(CMatrix44& matrix)
	{
		//matrix.RotateY(1024);
		SubMesh::SetRotation(matrix);
	}
};



class Hummer
	:public TKeyFramed::SubMesh
{
public:
	Hummer():TKeyFramed::SubMesh("objects\\Bag_Hummer.vobj","textures\\track10\\Bag_Hummer.RLE","objects\\bag_hummer_anim.kframe",0){}

	virtual void SetPosition(CMatrix44& matrix)
	{
		//matrix.Translate(0,-90,0);
		SubMesh::SetPosition(matrix);
	}
};



class Chopper
	:public TKeyFramed::SubMesh
{
public:
	Chopper():TKeyFramed::SubMesh("objects\\Bag_Chopper.vobj","textures\\track10\\Bag_Helicopter.RLE","objects\\Bag_chopper_anim.kframe",0){}
};


class ChopperRotor
	:public TKeyFramed::SubMesh
{
public:
	ChopperRotor():TKeyFramed::SubMesh("objects\\Bag_Chopper_Rotor.vobj","textures\\track10\\Bag_Helicopter.RLE","objects\\Bag_chopper_anim.kframe",0)
	{
		m_rotate=0;
	}


	virtual void SetRotation(CMatrix44& matrix)
	{
		matrix.RotateY(m_rotate);

		m_rotate+=192;
		if(m_rotate>2048)
			m_rotate=0;

		SubMesh::SetRotation(matrix);
	}

private:
	int m_rotate;
};


class Tank
	:public TKeyFramed::SubMesh
{
public:

	Tank():TKeyFramed::SubMesh("objects\\bag_tank.vobj","textures\\track10\\Bag_Vehicule_Tank.RLE","objects\\bag_tank_anim.kframe",0)
	{}



	virtual void SetRotation(CMatrix44& matrix)
	{
		matrix.RotateY(720);		
	}

	virtual void Draw(Lib3D::CLib3D& lib3d,int sectionId)
	{
		SubMesh::Draw(lib3d,sectionId);
	}
};

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
TKeyFramed::TKeyFramed(int code)
	:m_nbMesh(0)
{
	switch(code)
	{
	case 10:
//		m_subMesh[m_nbMesh] = new Hummer;
//		m_nbMesh++;

		//m_subMesh[m_nbMesh] = new Chopper;
		//m_nbMesh++;

		//m_subMesh[m_nbMesh] = new ChopperRotor;
		//m_nbMesh++;
		
		//m_subMesh[m_nbMesh] = new Tank;
		//m_nbMesh++;

		//m_subMesh[m_nbMesh] = new Train;
		//m_nbMesh++;

		break;

	default:
		//assert(false);
		break;
	}
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
TKeyFramed::~TKeyFramed()
{
	for(int i=0;i<m_nbMesh;i++)
		MM_DELETE m_subMesh[i];
}



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void TKeyFramed::Load(const char* fileName,const char* textureName,const char* keyFrameFileName,int startFrame)
{
	m_subMesh[m_nbMesh] = NEW SubMesh(fileName,textureName,keyFrameFileName,startFrame);
	m_nbMesh++;
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void TKeyFramed::Draw(Lib3D::CLib3D& lib3d,int sectionId)
{
	for(int i=0;i<m_nbMesh;i++)
		m_subMesh[i]->Draw(lib3d,sectionId);
}









