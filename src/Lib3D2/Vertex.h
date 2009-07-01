#ifndef _VERTEX_H_
#define _VERTEX_H_

#include "vector.h"


namespace Lib3D
{
	enum eVextexVxIndex
	{
		VEC_SCR = 0,                                 // offset to access to screen vector into TVertex struct
		VEC_WRD = 1,                                 // offset to access to world transformed position into TVertex struct
		VEC_INI = 2,                                 // offset to access to import initial position into TVertex struct
	};

	// vertex definition						    // !!! do not change order of these vectors or insert variables between due to some cast
	class TVertex								    // ScreenPos & WorldTPos must be in first position, due to some cast
	{
	public:
		Vector4s ScreenPos;                          // 0 screen transformed position + rendering light intensity (s)
		Vector4s WorldTPos;                          // 1 world transformed position (relative to camera position)
		Vector4s InitialPos;                         // 2 origin export position + origin gouraud intensity


inline		void	ClearVMask()		{m_visibilityMask=0;}
inline		void	AddVMask(int vm)	{m_visibilityMask |= vm; }
inline		void	SetVMask(int vm)	{m_visibilityMask = vm; }

inline		int		Vmask() const		{return m_visibilityMask;}

	private:
		unsigned int m_visibilityMask;
	};

}// namespace


#endif // _VERTEX_H_
