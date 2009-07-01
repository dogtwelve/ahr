#ifndef _KEYFRAMED_H_
#define _KEYFRAMED_H_



namespace Lib3D
{
	class TVertex;
	class TTexture;
	class CMesh;
	class CLib3D;
}



// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class TKeyFramed
{
public:
	class SubMesh;

	TKeyFramed(int code);
	~TKeyFramed();

	void Draw(Lib3D::CLib3D&,int sectionId);


private:

	void	Load(const char* fileName,const char* textureName,const char*,int);

private:

	


	enum
	{
		kMaxMesh = 10,
	};
	
	
	SubMesh*			m_subMesh[kMaxMesh];
	int					m_nbMesh;
	
};


#endif // _KEYFRAMED_H_
