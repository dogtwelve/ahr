#include "Config.h"
#include "HG/HighGear.h"
#include "TextureFBO.h"
#include "Lib3DGL/Lib3DGL.h"


#ifndef _SAVESCREENWRAPPER_H_
#define _SAVESCREENWRAPPER_H_

#define TEX_SCREEN_BUFFER_W	512
#define TEX_SCREEN_BUFFER_H 512

extern TextureFBO* g_texScreenBuffer;

extern int	g_sceneViewportW;
extern int	g_sceneViewportH;
extern bool	g_bSaveInScreenBuffer;	


//link gl to a backbuffer screen ( fbo )
void LinkGLToScreenBuffer( s32 viewportX, s32 viewportY, s32 viewportW, s32 viewportH );
bool IsGLLinkedToScreenBuffer();
//link gl to the old frame buffer object
void RestoreGLLinkage();

void InitScreenBufferIndices();

void FlushScreenBuffer();

void DrawScreenBuffer( s32 x, s32 y, s32 w, s32 h, u32 color);

void DrawBlurEffect(u32 colFadeInt, u32 colFadeExt, int offset, int deltaOffset, int numSteps);

//blur effect
extern bool g_bIsBlurActivated;

//optimize ingame_menu
extern bool g_bRefreshScreenBuffer;

#endif //_SAVESCREENWRAPPER_H_

