#ifndef _BOARD3D_H_
#define _BOARD3D_H_

#include "config.h"

#include <limits.h>

#include "vector.h"
#include "Vertex.h"
#include "Constants.h"

#include "Rasterize.h"

#define USE_TABLE 1
//#define USE_TABLE 0

class CHighGear;

namespace Lib3D
{
class TTexture;
class TVxuv;
class TFace;
class CLib3D;
class CColor;
class CFogShader;

// -----------------------------------------------------------------------------------------------------
// Board3D class
// -----------------------------------------------------------------------------------------------------
class CBoard3D
{
	enum
	{
		DX_BITS  =Z_BUFFER_BITS,
		DX_RANGE =(1 << DX_BITS),
		DIVTBL_MASK = (DX_RANGE - 1),	// mask to access to division table
		INVTBL_SHIFT = 18              // Inverse table contain 2^22/x values
	};

	typedef	unsigned short TPixel;
public:	
	//#ifdef USE_ROAD_FX
		int m_roadFxMax;
	//#endif
	void			SetFrameEven( bool b )	{ 
		m_FrameEven = b; 
	}
	//bool			IsFrameEven()			{ return m_FrameEven; }


	friend void	Lib3D::DrawScanLineZCorrected(CBoard3D&,int yDraw,const LineParamZ&,int xStop);
	friend void	Lib3D::DrawScanLineZCorrectedCar(CBoard3D&,int yDraw,const LineParamZ&,int xStop);

	friend void Lib3D::DrawScanLine(CBoard3D&,int yDraw,const LineParamNoZ&,int xStop);


	friend void Lib3D::DrawScanLineAdditive(CBoard3D&,int yDraw,const LineParamNoZ&,int xStop);
	friend void Lib3D::DrawScanLineAdditiveAlpha(CBoard3D&,int yDraw,const LineParamNoZ&,int xStop);
	friend void Lib3D::DrawScanLineAlphaMask(CBoard3D&,int yDraw,const LineParamNoZ&,int xStop);
	friend void Lib3D::DrawScanLineFlat(CBoard3D&,int yDraw,const LineParamNoZ&,int xStop);
	friend void Lib3D::DrawScanLineFlatTrans(CBoard3D&,int yDraw,const LineParamNoZ&,int xStop);
	friend void Lib3D::DrawScanLineFlatTransNeon(CBoard3D&,int yDraw,const LineParamNoZ&,int xStop);
	friend void Lib3D::DrawScanLineSubstractive(Lib3D::CBoard3D& board,int yDraw,const LineParamNoZ& i_param,int xStop);
	friend void Lib3D::DrawScanLineShadow(Lib3D::CBoard3D& board,int yDraw,const LineParamNoZUV& i_param,int xStop);

	// Do not change this enum without changing the shader table in the CPP (kShaders)
	enum RenderMode	
	{
		RM_Flat,
		RM_Facet,
		RM_FacetBinAlpha,
		RM_SkyBox,
		RM_DebugGreen,
		RM_DebugYellow,
		RM_DebugRed,
		kNbRM
	};

	enum{ kClearColour = (-1 * FAR_CLIP_MAX) << 16  };

	CBoard3D();      // Init Rasterization parameters
	~CBoard3D();

	// define 3D -> screen projection
	inline void DefProjection(const Vector4s *Src, Vector4s *target,int fov)
	{
		if(Src->z > -64)
			DefProjectionNear(Src,target,fov,7);
		else if(Src->z > -128)
			DefProjectionNear(Src,target,fov,6);
		else if(Src->z > -256)
			DefProjectionNear(Src,target,fov,5);
		else if(Src->z > -512)
			DefProjectionNear(Src,target,fov,4);
		else
			DefProjectionFar(Src,target,fov);
	}

	// wireframe
	void DrawEdgeHV(int xa, int ya, int xb, int yb);
	void DrawPolyEdge (const Vector4s *VxA, const Vector4s *VxB, const Vector4s *VxC);
  
	void DrawFace(const TFace *F,unsigned int tface_flag );

#define BILLBOARD_TRANS_NONE		0x00
#define BILLBOARD_TRANS_MID			0x01 // 0x10
#define BILLBOARD_TRANS_ADD			0x02 // 0x20
#define BILLBOARD_TRANS_ALPHA		0x03 // 0x40
#define BILLBOARD_TRANS_STENCIL     0x04 // 0x100

	void DrawBillboard(const Vector4s& screenSpaceCentre,const TTexture&,int scale,int fov, const int transparency=BILLBOARD_TRANS_NONE, const int alpha=0);
	void DrawBillboard(	const Vector4s& screenSpaceCentre,const TTexture& tex,int i_scale_x, int i_scale_y,	int fov,const int transparency, int tex_start_x, int tex_end_x);

	void DrawLine(Vector4s vector1,Vector4s vector2,int color1,int color2,const CColor&, bool yesZ=true);
	
	inline int DivideShift16(int num,int denom) const// return (num/denom) << 16
	{
		//assert(denom >=0 && denom < DX_RANGE);
		CHK_MULT(num,InverseTable(denom));
		return (num * InverseTable(denom)) >> (INVTBL_SHIFT - 16);
	}

	void			Clear();
	void			Clear(unsigned short clearColor);

inline	const TPixel*	GetImageBufferFull() const				{return m_screen;/*m_screenFull*/}
inline	TPixel*			GetImageBufferFull()					{return m_screen;/*m_screenFull*/}
inline	TPixel*			GetImageBuffer()						{return m_screen;}
inline	const TPixel*	GetImageBuffer() const					{return m_screen;}

		void			recalcScreenPtr();

inline	void			SetWireFrame(int col = 0x0F00)			{m_wireframe = col | 0xF000;}
inline	void			ClearWireFrame()						{m_wireframe = 0x00;}

inline	const TTexture*	GetBindTexture() const					{return m_bindTexture;}

#if USE_STENCIL_BUFFER
	void			ClearStencil(bool force=false);
	void			FlushStencilBuffer();
#endif // USE_STENCIL_BUFFER

	enum ETextureFxMode
	{
		kTextureFxModeNormal,
		kTextureFxModeEnvMap,
		kTextureFxModeOldEnvMap,
		kTextureFxModeGhost,
		kTextureFxModeRoad,
	};

	inline ETextureFxMode	GetTextureFXIdx( ) const {return m_TextureFXIdx;}
	inline void				SetTextureFXIdx( ETextureFxMode mode ) {m_TextureFXIdx = mode; }

inline	void SetTextureFX( const TTexture* texture, int fxparam1, int fxparam2 ) {m_bindTextureFX = texture; 	m_TextureFXParam1 = fxparam1; 	m_TextureFXParam2 = fxparam2;}
inline	void SetEnvmapFX( unsigned short* screen, unsigned short* coord )	{	m_bindFxEnvmapScreen = screen; m_bindFxEnvmapCoord = coord;	}

inline	void SetGhostModeColour(int fxColour){		m_GhostColour = fxColour;}

private:
	

	inline void DefProjectionNear(const Vector4s *Src, Vector4s *target,int fov,int shift)
	{
		target->x = m_dispX >> 1;
		target->y = m_dispY >> 1;
		target->z = -Src->z;

		const int inv = InvRhTable(target->z << shift);
		#if WIN_DEBUG
			double _invError = double(1<<22) / double(target->z << shift);
		#endif
		
		CHK_MULT(inv,fov);
		
		const int k = (inv*fov) >> (22 - 16);  // 22 is shifted value of InvRhTable
		
		CHK_MULT(k,Src->x<< shift);
		CHK_MULT(k,Src->y<< shift);

		const int kRoundBias = 1<<15;
		
		target->x += ((k*(Src->x << shift) + kRoundBias) >> 16);
		target->y -= ((k*(Src->y << shift) + kRoundBias) >> 16);		
	}

	inline void DefProjectionFar(const Vector4s *Src, Vector4s *target,int fov)
	{
		target->x = m_dispX >> 1;
		target->y = m_dispY >> 1;
		target->z = -Src->z;
				
		const int inv =InvRhTable(target->z & (DX_RANGE-1));
		CHK_MULT(inv,fov);
		
		const int k = (inv*fov) >> (22 - 16);  // 22 is shifted value of InvRhTable
		
		CHK_MULT(k,Src->x);
		CHK_MULT(k,Src->y);

		const int kRoundBias = 1<<15;
		
		target->x += ((k*Src->x + kRoundBias) >> 16);
		target->y -= ((k*Src->y + kRoundBias) >> 16);		
	}

	void DrawPolyGT3z (const TVxuv *A, const TVxuv *B, const TVxuv *C);// textured + z correction  
	void DrawPolyGT3zCar(const TVxuv *A, const TVxuv *B, const TVxuv *C);
	void DrawPolyGT3  (const TVxuv *A, const TVxuv *B, const TVxuv *C);// textured without z correction
	void DrawPolyG3  (const TVxuv *A, const TVxuv *B, const TVxuv *C, unsigned int trans = 0, unsigned int neon = 0);// flat
	void DrawPolyGT3Additive(const TVxuv *A, const TVxuv *B, const TVxuv *C);
	void DrawPolyGT3AdditiveAlpha(const TVxuv *A, const TVxuv *B, const TVxuv *C);
	void DrawPolyGT3AlphaMask(const TVxuv *A, const TVxuv *B, const TVxuv *C);
	void DrawPolyGT3Substractive(const TVxuv *A, const TVxuv *B, const TVxuv *C);

	void DrawPolyGT3Shadow(const TVxuv *A, const TVxuv *B, const TVxuv *C);

	bool PrepareFace(const TFace* F,const TVxuv** vxUV);


	// init
	void InitInverseTable(void);

	// wireframe line drawing
	void DrawEdgeH(int xStart, int ya, int xStop, int yb);
	void DrawEdgeV(int xa, int yStart, int xb, int yStop);


	
	#if USE_TABLE
		//	1<<INVTBL_SHIFT / i
		inline int InverseTable(int i) const
		{
			REC_MINMAX(InverseTable,i);

			if(i >= DX_RANGE)
				return m_inverseTable[DX_RANGE-1];	// [NOTE] this is happening in game, so we need it!

			if(i < 0)
				return m_inverseTable[0];	// [NOTE] this is happening in game but we are not yet sure if it's right way 
											// to fix it


			A_ASSERT(i>=0 && i < DX_RANGE);
			/*
			if(i<=0)
				return m_inverseTable[0];
					
			if(i >= DX_RANGE)
				return int( (1<<INVTBL_SHIFT) / i); // + 0.5f;*/

			return m_inverseTable[i];
		}
		
		inline int InvRhTable(int i) const
		{
			if(i<=0)
				return m_invRhTable[0];

			if(i >= DX_RANGE+ 32)
				return m_invRhTable[DX_RANGE+ 31];
			return m_invRhTable[i];
		}
	#else
		static int InverseTable(int i);
		static int InvRhTable(int i);
	#endif	//USE_TABLE

		int				m_256Div16[16];		// for perspective corrected rasterization interp < 16 pixels

public:
	unsigned int		m_RandSeed;	// no need to be intialized (it's a random seed after all ;)
	// must use separated rand seed for the road so we can reinitialize it without any side effects
	unsigned int		m_RoadFXRandSeed;	

#if USE_STENCIL_BUFFER
	TPixel*				m_stencilBuffer;
	unsigned char*		m_stencilAlpha;
	int					m_stencilBeginX;
	int					m_stencilBeginY;
	int					m_stencilEndX;
	int					m_stencilEndY;
#endif

//private:
public:
//	TPixel*				m_screenFullReal;							// contain display data (fullscreen)
//	TPixel*				m_screenFull;								// contain display data (fullscreen)
	TPixel*				m_screen;									// contain display data for 3D -> offset on fullscreen

	bool				m_FrameEven;

	const TTexture*		m_bindTexture;						// current binded texture
	const TTexture*		m_bindTextureFX;					// current binded texture
  
	// for new envmap data
	unsigned short*		m_bindFxEnvmapCoord;
	unsigned short*		m_bindFxEnvmapScreen;



	ETextureFxMode		m_TextureFXIdx;					// for envmap effect
	int					m_TextureFXParam1;
	int					m_TextureFXParam2;
	int					m_GhostColour;

	int					m_wireframe;							// face drawing mode (e_PolyMode)

	int					m_dh;
	int					m_du;
	int					m_dv;
	int					m_dz;
	

	int					m_uvShift;
  
#if USE_TABLE
	int					m_inverseTable[DX_RANGE];       // inverse table allow to replace some divisions with multiplications
	int					m_invRhTable[DX_RANGE + 32];		// used for perspective correction + projections. note: security offset added
#endif

public:
	CHighGear *m_pHG;
	int& m_dispX;
	int& m_dispY;

#if USE_Z_BUFFER

	int*				m_zBuffer;
//	int*				m_zBufferCar;
	int*				m_zBufferReal;
	int					m_zBufferDir;

	void ClearZBuffer();

#endif // USE_Z_BUFFER

};

}//namespace
#endif // _BOARD3D_H_
