#include "Sort.h"


#include "Texture.h"
#include "Board3d.h"

using namespace Lib3D;


CFSort::CFSort()
{
	for (int i=0; i < YROW_COUNT; i++)
		RowEntry[i] = NULL;
	
	Min = YROW_COUNT;
	Max = 0;
}

/*
void CFSort::SortFace(TFace *F)
{
	int z = (F->ScrVectorA().z + F->ScrVectorB().z + F->ScrVectorC().z) >> 2;   // z average sum
	
	assert( z < YROW_COUNT );

	F->next_face = RowEntry[z];
	RowEntry[z] = F;

	if (z < Min) Min = z;                          // adjust scan limits
	if (z > Max) Max = z;
}



// draw far to near
void CFSort::DrawSortedList(CBoard3D& board, bool envmap)
{
	int		i = (Max - Min) + 1;
	TFace**	entry = RowEntry + Max;
	
	// empty ?
	if (Max == 0)	return;

#ifdef _ENVMAP_
	if (envmap)
		board.SetTextureFXIdx( 1 );
#endif

	while (i--)
	{
		TFace*	f = *entry;
		if (f)
		{
			*entry = NULL;
			while (f)
			{
				board.DrawFace(f,0);

				f = f->next_face;
			}
		}
		entry--;
	}

#ifdef _ENVMAP_
	if (envmap)
		board.SetTextureFXIdx( 0 );
#endif

	Min = YROW_COUNT;
	Max = 0;
}
*/
