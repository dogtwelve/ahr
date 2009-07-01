#ifndef _FX_H_
#define _FX_H_



namespace Lib3D
{
class CBoard3D;
class CRender;
class CLib3D;


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
class FX
{
public:
	virtual	~FX(){};

	virtual void	Draw(CLib3D&)=0;
	virtual bool	Remove()=0;
};




}//namespace
#endif // _FX_H_
