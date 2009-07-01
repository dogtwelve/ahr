// Rect.h: interface for the CRect2 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RECT_H__60D6FCE1_3A36_43FC_8878_5B8858EF6DB7__INCLUDED_)
#define AFX_RECT_H__60D6FCE1_3A36_43FC_8878_5B8858EF6DB7__INCLUDED_

//cdp iphone ... avoid overriding std::max or std::min
#define MATH_MIN(x,y) (((x)<(y))?(x):(y))
#define MATH_MAX(x,y) (((x)>(y))?(x):(y))

//#ifndef min
//	#define min(x,y) (((x)<(y))?(x):(y))
//#endif
//#ifndef max
//	#define max(x,y) (((x)>(y))?(x):(y))
//#endif


//namespace Lib2D {


class ARect {

    public:
        ARect(int in_x1, int in_y1, int in_x2, int in_y2)

		{
			Set(in_x1, in_y1, in_x2, in_y2);
		}

        ARect()
		{
			Set(0,0,0,0);
		}

#ifdef IPHONE
//cdp iphone ... assure that the compiler will not synthetize a copy-constructor or member wise assignment
		ARect(const ARect& src)
		{
			Set(src.x1, src.y1, src.x2, src.y2);
		}

		ARect& operator = (const ARect& src)
		{
			Set(src.x1, src.y1, src.x2, src.y2);
			return *this;
		}
#endif /* IPHONE */
		
		inline void Set(int in_x1, int in_y1, int in_x2, int in_y2)
		{
			x1 = in_x1;
			y1 = in_y1;
			x2 = in_x2;
			y2 = in_y2;
			Recalc();
		}


        int x1, y1, x2, y2;
        int width, height;

		// Return the intersection between two rectangles
		// It is assumed that the rectangle is ordoned
		static inline ARect Intersect(const ARect& rect1, const ARect& rect2){
			ARect ret(
					MATH_MAX(rect1.x1,rect2.x1),
					MATH_MAX(rect1.y1,rect2.y1),
					MATH_MIN(rect1.x2,rect2.x2),
					MATH_MIN(rect1.y2,rect2.y2)
					);

			if(ret.width<=0 || ret.height <=0){ 
				return ARect(0,0,0,0);
			}else{
				return ret;
			}

    };

		inline void Recalc(){
			width = x2-x1;
			height = y2-y1;
		}

		inline void MoveTo( int x, int y)
		{
			x1 = x;
			y1 = y;
			x2 = x+width;
			y2 = y+height;
		}
		
		//inline ARect Expand(int x)const{
		//	return Expand(x,x);
		//}
		//inline ARect Expand(int x, int y)const{
		//	return ARect(x1-x,y1-y,x2+x,y2+y);
		//}
		//inline ARect Translate(int x, int y)const{
		//	return ARect(x1+x,y1+y,x2+x,y2+y);
		//}
};




//}






#endif // !defined(AFX_RECT_H__60D6FCE1_3A36_43FC_8878_5B8858EF6DB7__INCLUDED_)
