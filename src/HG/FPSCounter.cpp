// FPSCounter.cpp: implementation of the CFPSCounter class.
//
//////////////////////////////////////////////////////////////////////

#include "FPSCounter.h"
#include "lib2D/lib2D.h"
#include "lib2D/FontWrapper.h"
#include "str_utils.h"

#ifdef NGI
#include "Game.h"

class CGame;
#endif // NGI

#ifdef CHECK_MEMORY_LEAKS
#include "MemoryManager.h"
#endif

//#include "HighGear.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFPSCounter::CFPSCounter()
{
    m_nCurrentFPS = 0;
    m_nTimeLastSec = 0;
    m_nNumFrames = 0;
	m_fpsInfoString[0] = 0;
	showInfo = false;
	toggledFromUpdateGame = false;
	toggledFromGameLoop = false;
	numUpdates = 0;
	m_font = NULL;
}


CFPSCounter::~CFPSCounter()
{

}

void CFPSCounter::ToggleInfoShow()
{
	showInfo = !showInfo;
}

/*
void CFPSCounter::Begin(int in_nCurrentTimeMillisec)
{
    m_nTimeLastSec = in_nCurrentTimeMillisec;
    m_nNumFrames = 0;
    m_nCurrentFPS = 0;
}
*/

void CFPSCounter::Update()
{
    m_nNumFrames++;    
}

// ---------------------------------------------------------------------------
// same as itoa, but more fast
// ---------------------------------------------------------------------------
void IntToStr(int a, unsigned short* aStr)
{
	static const int Pow10[6] = { 1000000, 100000, 10000, 1000, 100, 10 };       // for binairy -> decimal convertion

  int l, d, s;

  l = 0;
  s = 0;

  if (a < 0)
  {
    aStr[l++] = 'n';   // todo: add '-' character in font
    a = -a;
  }

  for (int i=0; i<6; i++)
  {
    d = a / Pow10[i];
    if (d)
    {
      s = 1;
      a -= d*Pow10[i];
    }
    if (s) 
      aStr[l++] = '0' + d;
  }
 
  aStr[l++] = '0' + a;
  aStr[l] = 0;
}

void CFPSCounter::Draw(int in_nPosX, int in_nPosY, CLib2D& in_Lib2D,int in_nCurrentTimeMillisec)
{
	if((m_nNumFrames & 0x07) == 0)
	{
		int takenRam = 0;
		int peek = 0;

#ifdef CHECK_MEMORY_LEAKS
		peek = CMemoryManager::GetInstance()->GetAllocatedSizePeek();
		takenRam = CMemoryManager::GetInstance()->GetAllocatedSize();
#endif

#ifdef NGI
		in_nCurrentTimeMillisec = CGame::GetTime();
#endif // NGI

		const int nTimeOffset = in_nCurrentTimeMillisec - m_nTimeLastSec;

		if(nTimeOffset > 0)
		{
			//const int fps = (80000*256)/nTimeOffset;	// Note : fps = 10 time real fps (for precision)
#ifdef NGI
			const int fps = 80000000 / nTimeOffset;
#else
			const int fps = ((8 * 10 * 1000) << 8)/nTimeOffset;
#endif // NGI
			unsigned short	fpsTempInt[10];
			unsigned short	fpsTempFrac[10];
			::IntToStr(fps/10, fpsTempInt);
			::IntToStr(fps%10, fpsTempFrac);
			
			//sprintf(m_fpsInfoString,"FPS:%3s.%1s PEEK:%5dKB ALLOC:%5dKB", fpsTempInt, fpsTempFrac, peek /1024, takenRam /1024);			
			sprintf(m_fpsInfoString, "FPS:%s.%s %d", fpsTempInt, fpsTempFrac, numUpdates);
		}
		m_nTimeLastSec = in_nCurrentTimeMillisec;      
	}

	if (/*showInfo &&*/ m_font)
	{
		int fpsMargin =	2;
		int stringWidth = m_font->GetStringWidth(m_fpsInfoString);
		in_Lib2D.DrawRect (in_nPosX, in_nPosY,stringWidth + 2*fpsMargin,20,0xF000/*COLOR_BLACK*/);		
		m_font->DrawString(in_Lib2D, in_nPosX+fpsMargin, in_nPosY+fpsMargin, m_fpsInfoString);
	}
}

