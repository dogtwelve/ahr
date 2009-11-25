#ifndef _RASTERIZE_H_
#define _RASTERIZE_H_

class CHighGear;

namespace Lib3D
{
	class CBoard3D;

	// ---------------------------------------------------------------------------
	//
	// ---------------------------------------------------------------------------
	struct LineParamNoZ
	{
	public:
		LineParamNoZ(){};

		inline		LineParamNoZ(int ix,int iz,int iu,int iv) :x(ix),z(iz),u(iu),v(iv)	{};

		inline void Set(int ix,int iz,int iu,int iv)
		{
			x=ix;	z=iz;	u=iu;	v=iv;
		}

		inline void Substact(int mult,const LineParamNoZ& delta)
		{
			x -= mult * delta.x;
			z -= mult * delta.z;

			u -= mult * delta.u;
			v -= mult * delta.v;
		}

		inline void AddDelta(const LineParamNoZ& delta)
		{
			x+=delta.x;
			z+=delta.z;		
			u+=delta.u;
			v+=delta.v;
		}


	public:
		int x;
		int z;

		int	u;
		int v;
	};

	struct LineParamNoZUV
	{
	public:
		LineParamNoZUV(){};

		inline		LineParamNoZUV(int ix,int iz) :x(ix),z(iz)	{};

		inline void Set(int ix,int iz)
		{
			x=ix;	z=iz;
		}

		inline void Substact(int mult,const LineParamNoZUV& delta)
		{
			x -= mult * delta.x;
			z -= mult * delta.z;
		}

		inline void AddDelta(const LineParamNoZUV& delta)
		{
			x+=delta.x;
			z+=delta.z;		
		}


	public:
		int x;
		int z;
	};


	// ---------------------------------------------------------------------------
	//
	// ---------------------------------------------------------------------------
	struct LineParamZ
		:public LineParamNoZ
	{
	public:	
		typedef LineParamNoZ inherited;

		inline LineParamZ(){};

		inline LineParamZ(int ix,int iz,int ih,int iu,int iv) :inherited(ix,iz,iu,iv),h(ih)  {}

		inline void Set(int ix,int iz,int ih,int iu,int iv)
		{
			inherited::Set(ix,iz,iu,iv);
			h=ih;
		}

		inline void Substact(int mult,const LineParamZ& delta)
		{
			inherited::Substact(mult,delta);
			h -= mult * delta.h;
		}

		inline void AddDelta(const LineParamZ& delta)
		{
			inherited::AddDelta(delta);		
			h+=delta.h;		
		}

	public:
		int h;
	};

	
	void DrawScanLineZCorrected(CBoard3D&,int yDraw,const LineParamZ&,int xStop);
	void DrawScanLineZCorrectedCar(CBoard3D& board,int yDraw,const LineParamZ& i_param, int xStop); // TEST

	#if OPT_ASM

		//to choose the optimised function when compile for N-gage, otherwise C++ function
		#ifdef MARM
			extern "C" void DrawScanLine(CBoard3D&,int yDraw,const LineParamNoZ&,int xStop);
		#else
			void DrawScanLine(CBoard3D&,int yDraw,const LineParamNoZ&,int xStop);
		#endif

	#else

	#if PLAYERCARFRESNEL
		void DrawScanLine(CBoard3D&,int yDraw,const LineParamNoZ&,int xStop, int i_alpha);
	#else
		void DrawScanLine(CBoard3D&,int yDraw,const LineParamNoZ&,int xStop);
	#endif
		void DrawScanLineAdditive(CBoard3D&,int yDraw,const LineParamNoZ&,int xStop);
		void DrawScanLineAdditiveAlpha(Lib3D::CBoard3D& board,int yDraw,const LineParamNoZ& i_param,int xStop);
		void DrawScanLineAlphaMask(CBoard3D&,int yDraw,const LineParamNoZ&,int xStop);
		void DrawScanLineSubstractive(Lib3D::CBoard3D& board,int yDraw,const LineParamNoZ& i_param,int xStop);
		void DrawScanLineFlat(CBoard3D&,int yDraw,const LineParamNoZ&,int xStop);
		void DrawScanLineFlatTrans(CBoard3D&,int yDraw,const LineParamNoZ&,int xStop);
		void DrawScanLineFlatTransNeon(Lib3D::CBoard3D& board,int yDraw,const LineParamNoZ& i_param,int xStop);

		void DrawScanLineShadow(Lib3D::CBoard3D& board,int yDraw,const LineParamNoZUV& i_param,int xStop);

		//void DrawScanLineZCorrected_OLD(CBoard3D& board,int yDraw,const LineParamZ& i_param,int xStop);
	#endif


}




#endif // _RASTERIZE_H_
