#include "Camera.h"
#include "IMath.h"
#include "Constants.h"

#include <math.h>

using namespace Lib3D;


CCameraAsphalt::CCameraAsphalt()
{
	m_matrix.LoadIdentity();
	m_position.Init(0,0,0);
	m_rotation.Init(0,0,0);
}

void CCameraAsphalt::Set(const Vector4s& pos,const Vector4s& rot, const CMatrix44& fxmat)
{
	m_position = pos;
	m_rotation = rot;

	m_matrix.DefRotateX(-rot.x);	// [NOTE] Departing from conventions and using Yaw/Roll/Pitch instead of the usual Yaw/Pitch/Roll...
	m_matrix.RotateZ(-rot.z);		//        I'd fix it but that would require re-tweaking all the camera motions.
	m_matrix.RotateY(-rot.y);
	m_matrix.Mult(&fxmat);

	m_matrix.Translate(-pos.x, -pos.y, -pos.z);
}