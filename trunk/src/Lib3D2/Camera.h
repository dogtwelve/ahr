#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "Matrix.h"

namespace Lib3D
{

class CCameraAsphalt
{
public:
	CCameraAsphalt();

	void				Set(const Vector4s& pos,const Vector4s& rot, const CMatrix44& fxmat);

inline	const CMatrix44&	GetMatrix() const	{return m_matrix;}
inline	const Vector4s&		GetRotation() const	{return m_rotation;}
inline	const Vector4s&		GetPosition() const	{return m_position;}

private:
	CMatrix44			m_matrix;
	int					m_fov;
	Vector4s			m_position;
	Vector4s			m_rotation;
};


}//namespace

#endif // _CAMERA_H_
