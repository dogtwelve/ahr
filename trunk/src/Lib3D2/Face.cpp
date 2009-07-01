#include "config.h"
#include "Face.h"
#include "IMath.h"
#include "Texture.h"
#include "Lib3D.h"
#include "Render.h"
#include <math.h>
#include "Performance.h"

using namespace Lib3D;

#ifdef WIN32
	extern int gNbFacesDrawn;
#endif

TFace::TFace(TVertex* v)
	:m_texture(NULL),
	uA(0),vA(0),VxA((Vector4s *)v),
	uB(0),vB(0),VxB((Vector4s *)(v+1)),
	uC(0),vC(0),VxC((Vector4s *)(v+2))
{
}


TFace::TFace(	const TTexture* texture,
							TVertex& vA,UVType u1,UVType v1,
							TVertex& vB,UVType u2,UVType v2,
							TVertex& vC,UVType u3,UVType v3,char shading)
	:m_texture(texture),
	uA(u1),vA(v1),VxA((Vector4s *)&vA),

	uB(u2),vB(v2),VxB((Vector4s *)&vB),

	uC(u3),vC(v3),VxC((Vector4s *)&vC)
{
}


// ---------------------------------------------------------------------------
// virtual inherited from the COTObject parent class
// ---------------------------------------------------------------------------

#ifdef USE_OGL

void TFace::AddToOGLRenderingPipeline(int& nVtxAdded, OTGroupInfo*& pGroupInfo, f32*& pVtx, f32*& pTex, int startVtxIdx, u16*& pIndices, u8*& pColor)
{
	u16 faceFlagsForGrpInfo = 0;
	
	bool isFlat	= (bool) (m_flag & TFACE_FLAG_FLAT);
	faceFlagsForGrpInfo  |= isFlat ? FLAG_GRP_INFO_HAS_COLOR : FLAG_GRP_INFO_HAS_TEXTURE;

	if (m_flag & TFACE_FLAG_ENVMAP)
		faceFlagsForGrpInfo |= FLAG_GRP_INFO_USE_POLYGON_OFFSET;
	
	if( !isFlat )
	{
		//texture
		bool useTrans	 = m_texture->m_bHasAlphaChannel;
		bool useAdditive = (m_flag & TFACE_FLAG_TRANS_ADDITIVE) || ( m_flag & TFACE_FLAG_TRANS_ADDITIVE_ALPHA );
		bool useGlobalAlpha = (m_texture->m_globalAlpha != 0xFF); 
		
		if( useTrans )
			faceFlagsForGrpInfo |= FLAG_GRP_INFO_USE_TRANS;

		if( useAdditive )
			faceFlagsForGrpInfo |= FLAG_GRP_INFO_USE_ADDITIVE;

		if( useGlobalAlpha )
			faceFlagsForGrpInfo |= FLAG_GRP_INFO_USE_GLB_ALPHA;
	}
	else
	{
		//only opaque	
	}

	//BEGIN Attach faces to groups
	bool createNewGroup = true; //include the first group case
	
	if( pGroupInfo->m_startVtxIdx == 0 && pGroupInfo->m_vtxCount == 0 )
	{
		//first group in array
		createNewGroup = false;
		pGroupInfo->m_flags = faceFlagsForGrpInfo;
		pGroupInfo->m_pTexOrColor = m_texture;
	}
	else
	{
		//avoid first group
		if( pGroupInfo->m_flags == faceFlagsForGrpInfo )
		{			
			if( pGroupInfo->m_pTexOrColor == m_texture )
				createNewGroup = false;		
		}	
	}

	if(createNewGroup)
	{		
		//add a new group
		u32 newVtxIdx = pGroupInfo->m_startVtxIdx + pGroupInfo->m_vtxCount;
		
		pGroupInfo++;			
		pGroupInfo->m_startVtxIdx = newVtxIdx;
		pGroupInfo->m_vtxCount = 0;

		pGroupInfo->m_flags = faceFlagsForGrpInfo;
		pGroupInfo->m_pTexOrColor = m_texture;
	}	
	//END Attach faces to groups	


	//vertex A
	*pVtx++ = (f32)VectorA(VEC_WRD).x;
	*pVtx++ = (f32)VectorA(VEC_WRD).y;
	*pVtx++ = (f32)VectorA(VEC_WRD).z;
	
	//vertex B
	*pVtx++ = (f32)VectorB(VEC_WRD).x;
	*pVtx++ = (f32)VectorB(VEC_WRD).y;
	*pVtx++ = (f32)VectorB(VEC_WRD).z;
	
	//vertex C
	*pVtx++ = (f32)VectorC(VEC_WRD).x;
	*pVtx++ = (f32)VectorC(VEC_WRD).y;
	*pVtx++ = (f32)VectorC(VEC_WRD).z;	

	//compute tex coordinates
	if(CHECK_GRP_INFO_HAS_TEXTURE(pGroupInfo->m_flags))
	{
		f32 aspectRatioW = 1.0f / ( this->m_texture->m_pow2Width * ( 1 << 4 ) );
		f32 aspectRatioH = 1.0f / ( this->m_texture->m_pow2Height * ( 1 << 4 ) );	
			
		//tex for vertex A
		*pTex++ = uA * aspectRatioW;
		*pTex++ = vA * aspectRatioH;

		//tex for vertex B
		*pTex++ = uB * aspectRatioW;
		*pTex++ = vB * aspectRatioH;

		//tex for vertex C
		*pTex++ = uC * aspectRatioW;
		*pTex++ = vC * aspectRatioH;
	}
	else
	{
		pTex += 6;
	}

	if(CHECK_GRP_INFO_HAS_COLOR(pGroupInfo->m_flags))
	{
		//col for vertex A
		*pColor++ = 0xFF;	//R
		*pColor++ = 0x0;	//G
		*pColor++ = 0x0;	//B
		*pColor++ = 0xFF;	//A

		//col for vertex B
		*pColor++ = 0x0;	//R
		*pColor++ = 0xFF;	//G
		*pColor++ = 0x0;	//B
		*pColor++ = 0xFF;	//A

		//col for vertex C
		*pColor++ = 0x0;	//R
		*pColor++ = 0x0;	//G
		*pColor++ = 0xFF;	//B
		*pColor++ = 0xFF;	//A
	}
	else
	{
		pColor += 12;
	}
	
	
	*pIndices++ = startVtxIdx;
	*pIndices++ = startVtxIdx + 1;
	*pIndices++ = startVtxIdx + 2;

	nVtxAdded = 3;

	//add this face to the group
	pGroupInfo->m_vtxCount += 3; //3 indices
}

unsigned int TFace::GetGLTextureName()
{ 
	return (m_flag & TFACE_FLAG_FLAT) ? 0 : m_texture->m_glTextureName;
}

void TFace::Draw(CLib3D& lib3d)
{
	#pragma REMINDER("TO DO ... do a check if TFace::Draw(CLib3D& lib3d) is still called")	
}

#else /* USE_OGL */

void TFace::Draw(CLib3D& lib3d)
{
	//PERF_COUNTER(TFace_Draw);

	CBoard3D& board = *lib3d.m_board3d;

	#ifdef _ENVMAP_
		const CBoard3D::ETextureFxMode  oldTextureMode = board.GetTextureFXIdx( );

		if (m_flag & TFACE_FLAG_OLDENVMAP)
			board.SetTextureFXIdx( CBoard3D::kTextureFxModeOldEnvMap );	
		else if (m_flag & TFACE_FLAG_ENVMAP)
			board.SetTextureFXIdx( CBoard3D::kTextureFxModeEnvMap );
		else if (m_flag & TFACE_FLAG_GHOST)
			board.SetTextureFXIdx( CBoard3D::kTextureFxModeGhost );	
		else if (m_flag & TFACE_FLAG_ROAD)
			board.SetTextureFXIdx( CBoard3D::kTextureFxModeRoad );	
	#endif

		//if (m_texture->m_name == "tracks\\track16\\1152_forest.rle")
		//	int a = 0;

#if 1
	// 3d clip for all corrected faces
	if (m_flag & TFACE_FLAG_ZCORRECTED)
	{
		if (m_flag & TFACE_FLAG_SUBDIV)
		{
			lib3d.m_renderer->FrustrumSubdiv( this, board, m_flag );
		}
		else
		{
			if (m_flag & TFACE_FLAG_CULL)
				lib3d.m_renderer->FrustrumSubdiv( this, board, m_flag, true );
			else
				lib3d.m_renderer->FrustrumSubdiv( this, board, m_flag );
		}
	}
	else
	{
		if (m_flag & TFACE_FLAG_SUBDIV)
			lib3d.m_renderer->FrustrumSubdiv( this, board, m_flag );
		else
		{
			if (m_flag & TFACE_FLAG_CULL)
				lib3d.m_renderer->FrustrumDrawCull( this, board, m_flag );
			else
				lib3d.m_renderer->FrustrumDraw( this, board, m_flag );
		}
	}
#else
	if (m_flag & TFACE_FLAG_SUBDIV)
	{
		if (m_flag & TFACE_FLAG_SUBDIV4)
			lib3d.m_renderer->FrustrumSubdiv4Front( this, board, m_flag );
		else
			lib3d.m_renderer->FrustrumSubdiv( this, board, m_flag );
	}
	else
	{
		if (m_flag & TFACE_FLAG_CULL)
			lib3d.m_renderer->FrustrumDrawCull( this, board, m_flag );
		else
		{
			if (m_flag & TFACE_FLAG_SUBDIV4)
				lib3d.m_renderer->FrustrumSubdiv4( this, board, m_flag );
			else
				lib3d.m_renderer->FrustrumDraw( this, board, m_flag );
		}
	}
#endif

	#ifdef _ENVMAP_
		board.SetTextureFXIdx( oldTextureMode );
	#endif
}
#endif /* USE_OGL*/

// ---------------------------------------------------------------------------
//	Return TRue if face can be culled; face must have been transformed and
// ---------------------------------------------------------------------------
bool TFace::CullingTest() const
{
	const Vector4s& A = VertexA()->WorldTPos;
	const Vector4s& B = VertexB()->WorldTPos;
	const Vector4s& C = VertexC()->WorldTPos;

	const Vector4s faceNormal = Vector4s::Cross(B - A, C - A);

	const int dot = Vector4s::Dot(A, faceNormal);

	return dot >= 0;
}

//bool TFace::CullingTestBg() const
//{
//	const Vector4s& A = VertexA()->WorldTPos;
//	const Vector4s& B = VertexB()->WorldTPos;
//	const Vector4s& C = VertexC()->WorldTPos;
//
//	Vector4s faceNormal = Vector4s::Cross(B - A, C - A);
//
//	const int dot = Vector4s::Dot(A, faceNormal);
//
//	return dot < 0;
//}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
int TFace::GetZForOrderingTable() const
{
	const int az = VertexA()->WorldTPos.z;
	const int bz = VertexB()->WorldTPos.z;
	const int cz = VertexC()->WorldTPos.z;

	if (cz < bz)
	{
		if (cz < az)
			return -cz;
		else
			return -az;
	}
	else
	{
		if (bz < az)
			return -bz;
		else
			return -az;
	}
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
int TBillboard::GetZForOrderingTable() const
{	
	if (m_flag & TFACE_FLAG_BILBOARD_TRANS_ADD)	// additive face are a bit in front !
		return pos.z - 32;
	else
		return pos.z;
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
#ifdef USE_OGL

void TBillboard::AddToOGLRenderingPipeline(int& nVtxAdded, OTGroupInfo*& pGroupInfo, f32*& pVtx, f32*& pTex, int startVtxIdx, u16*& pIndices, u8*& pColor)
{
	
	u16 faceFlagsForGrpInfo = 0;
	
	faceFlagsForGrpInfo |= FLAG_GRP_INFO_HAS_TEXTURE;	
	
	//mark as billboard
	faceFlagsForGrpInfo |= FLAG_GRP_INFO_IS_BILLBOARD;

	//texture
	bool useTrans	 = m_texture->m_bHasAlphaChannel;
	bool useAdditive = ( m_flag & TFACE_FLAG_BILBOARD_TRANS_ADD ) || ( m_flag & TFACE_FLAG_BILBOARD_TRANS_ALPHA );
	bool useGlobalAlpha = ( m_texture->m_globalAlpha != 0xFF ); 
		
	if( useTrans )
		faceFlagsForGrpInfo |= FLAG_GRP_INFO_USE_TRANS;

	if( useAdditive )
		faceFlagsForGrpInfo |= FLAG_GRP_INFO_USE_ADDITIVE;

	if( useGlobalAlpha )
		faceFlagsForGrpInfo |= FLAG_GRP_INFO_USE_GLB_ALPHA;
	
	if (m_texture->GetFlags() & FLAG_TEXTURE_DISABLE_DEPTH_TEST)
		faceFlagsForGrpInfo |= FLAG_GRP_INFO_DISABLE_DEPTH_TEST;

	//BEGIN Attach faces to groups
	bool createNewGroup = true; //include the first group case
	
	if( pGroupInfo->m_startVtxIdx == 0 && pGroupInfo->m_vtxCount == 0 )
	{
		//first group in array
		createNewGroup = false;
		pGroupInfo->m_flags = faceFlagsForGrpInfo;
		pGroupInfo->m_pTexOrColor = m_texture;
	}
	else
	{
		//avoid first group
		if( pGroupInfo->m_flags == faceFlagsForGrpInfo )
		{			
			if( pGroupInfo->m_pTexOrColor == m_texture )
				createNewGroup = false;		
		}	
	}

	if(createNewGroup)
	{		
		//add a new group
		u32 newVtxIdx = pGroupInfo->m_startVtxIdx + pGroupInfo->m_vtxCount;
		
		pGroupInfo++;			
		pGroupInfo->m_startVtxIdx = newVtxIdx;
		pGroupInfo->m_vtxCount = 0;

		pGroupInfo->m_flags = faceFlagsForGrpInfo;
		pGroupInfo->m_pTexOrColor = m_texture;
	}	
	//END Attach faces to groups	

	s32 half_size = size/2;	

	//vertex TOP_LEFT
	*pVtx++ = (f32)( pos.x - half_size );
	*pVtx++ = (f32)( pos.y + half_size );
	*pVtx++ = (f32) pos.z;
	
	//vertex TOP RIGHT
	*pVtx++ = (f32)( pos.x + half_size );
	*pVtx++ = (f32)( pos.y + half_size );
	*pVtx++ = (f32) pos.z;
	
	//vertex BOTTOM LEFT
	*pVtx++ = (f32)( pos.x - half_size );
	*pVtx++ = (f32)( pos.y - half_size );
	*pVtx++ = (f32) pos.z;

	//vertex BOTTOM RIGHT
	*pVtx++ = (f32)( pos.x + half_size );
	*pVtx++ = (f32)( pos.y - half_size );
	*pVtx++ = (f32) pos.z;

	//compute tex coordinates
	//f32 aspectRatioW = 1.0f / ( m_texture->m_pow2Width * ( 1 << 4 ) );
	//f32 aspectRatioH = 1.0f / ( m_texture->m_pow2Height * ( 1 << 4 ) );	
		
	//texture saved from tga ... fliped by Y

	if( (m_flag & TFACE_FLAG_BILBOARD_STARTEND) != 0 )
	{
		//compute tex coordinates
		f32 aspectRatioW = 1.0f / ( m_texture->m_pow2Width * ( 1 << 4 ) );
		f32 aspectRatioH = 1.0f / ( m_texture->m_pow2Height * ( 1 << 4 ) );

		//tex for TOP_LEFT
		*pTex++ = start_x	* aspectRatioW;//uA * aspectRatioW;
		*pTex++ = end_y		* aspectRatioH;//vA * aspectRatioH;

		//tex for TOP RIGHT
		*pTex++ = end_x		* aspectRatioW;//uB * aspectRatioW;
		*pTex++ = end_y		* aspectRatioH;//vB * aspectRatioH;

		//tex for BOTTOM LEFT
		*pTex++ = start_x	* aspectRatioW;//uC * aspectRatioW;
		*pTex++ = start_y	* aspectRatioH;//vC * aspectRatioH;

		//tex for BOTTOM RIGHT
		*pTex++ = end_x		* aspectRatioW;//uD * aspectRatioW;
		*pTex++ = start_y	* aspectRatioH;//vD * aspectRatioH;
	}
	else
	{
		//tex for TOP_LEFT
		*pTex++ = 0.0f;//uA * aspectRatioW;
		*pTex++ = 1.0f;//vA * aspectRatioH;

		//tex for TOP RIGHT
		*pTex++ = 1.0f;//uB * aspectRatioW;
		*pTex++ = 1.0f;//vB * aspectRatioH;

		//tex for BOTTOM LEFT
		*pTex++ = 0.0f;//uC * aspectRatioW;
		*pTex++ = 0.0f;//vC * aspectRatioH;

		//tex for BOTTOM RIGHT
		*pTex++ = 1.0f;//uD * aspectRatioW;
		*pTex++ = 0.0f;//vD * aspectRatioH;
	}
	
	pColor += 16;	
	
	*pIndices++ = (startVtxIdx);
	*pIndices++ = (startVtxIdx + 2);
	*pIndices++ = (startVtxIdx + 1);

	*pIndices++ = (startVtxIdx + 1);
	*pIndices++ = (startVtxIdx + 2);
	*pIndices++ = (startVtxIdx + 3);

	nVtxAdded = 4;

	//add this face to the group
	pGroupInfo->m_vtxCount += 6;	 //6 indices
}

//void TBillboard::AddToOGLRenderingPipeline(Vector4s& pos, int& nQuadsAdded, f32*& pVtx, f32*& pTex)
//{
///*	int halfSizeX = size * m_texture->SizeX() / 2;
//	int halfSizeY = size * m_texture->SizeY() / 2;
//
//	*pVtx++ = (f32)pos.x - halfSizeX;
//	*pVtx++ = (f32)pos.y - halfSizeY;
//	*pVtx++ = (f32)pos.z;
//
//	*pTex++ = 0.0f;
//	*pTex++ = 1.0f;
//
//	*pVtx++ = (f32)pos.x + halfSizeX;
//	*pVtx++ = (f32)pos.y - halfSizeY;
//	*pVtx++ = (f32)pos.z;
//
//	*pTex++ = 1.0f;
//	*pTex++ = 1.0f;
//
//	*pVtx++ = (f32)pos.x + halfSizeX;
//	*pVtx++ = (f32)pos.y + halfSizeY;
//	*pVtx++ = (f32)pos.z;
//
//	*pTex++ = 1.0f;
//	*pTex++ = 0.0f;
//
//	*pVtx++ = (f32)pos.x - halfSizeX;
//	*pVtx++ = (f32)pos.y + halfSizeY;
//	*pVtx++ = (f32)pos.z;
//
//	*pTex++ = 0.0f;
//	*pTex++ = 0.0f;
//
//	nQuadsAdded = 1;*/
//}
//
#endif /* USE_OGL */

void TBillboard::Draw(CLib3D& lib3d)
{
	PERF_COUNTER(TBillboard_Draw);

	const int fov = lib3d.GetFoV();

	int transparency = BILLBOARD_TRANS_NONE;
	if (m_flag & TFACE_FLAG_BILBOARD_TRANS_MID)
		transparency = BILLBOARD_TRANS_MID;
	else if(m_flag & TFACE_FLAG_BILBOARD_TRANS_ADD)
		transparency = BILLBOARD_TRANS_ADD;
	else if(m_flag & TFACE_FLAG_BILBOARD_TRANS_ALPHA)
		transparency = BILLBOARD_TRANS_ALPHA;
	else if(m_flag & TFACE_FLAG_BILBOARD_STENCIL)
		transparency = BILLBOARD_TRANS_STENCIL;

#ifdef USE_OGL
	return;
#else /* USE_OGL */

	if (m_flag & TFACE_FLAG_BILBOARD_STARTEND)
		lib3d.m_board3d->DrawBillboard(	pos, *m_texture, ((end_x - start_x)*size) >> 6, size, fov,  transparency, start_x, end_x );
	else
		lib3d.m_board3d->DrawBillboard(	pos, *m_texture, size, fov,  transparency, alpha);
#endif /* USE_OGL */

}
