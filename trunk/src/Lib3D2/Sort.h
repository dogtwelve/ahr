#ifndef _SORT_H_
#define _SORT_H_

#include "Constants.h"
#include "Face.h"

#define MAX_SORT_COUNT 1024			// max sort polys by objects

namespace Lib3D
{
	class CBoard3D;

// ---------------------------------------------------------------------------

class CFSort
{
	enum
	{
		YROW_COUNT  = (FAR_CLIP_MAX * -3)>>2  // use z1+z2+z3 >> 2 sort test
	};
	CFSort();	
public:

	
	void	SortFace(TFace *F);
	void	DrawSortedList(CBoard3D&,bool envmap);                     // use this method if mixed type of faces are sorted (opaque + alpha)
	
private:
	
	int		Min, Max;                                  // hach table scan range
	TFace*	RowEntry[YROW_COUNT];                // hach table entry count
};


}//namepsace
#endif // _SORT_H_

