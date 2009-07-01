#ifndef _SSEWINDRIVER_H_
#define _SSEWINDRIVER_H_

#include <stdlib.h>
#include "config.h"
#include <windows.h>

// OS/types Configuration :

#define SSE_NEW				NEW
#define SSE_DELETE			delete
#define SSE_DELETE_ARRAY	delete[]
#define SSE_MALLOC			malloc
#define SSE_FREE			free
#define SSE_ASSERT( a)		//if (!(a)) DBGPRINTF("ASSERT line %d", __LINE__)
#define SSE_MEMSET			memset

typedef unsigned int	u32;
typedef signed int		s32;
typedef unsigned short	u16;
typedef signed short	s16;
typedef unsigned char	u8;
typedef signed char		s8;

#define NULL			0


#include "SSEMixEngine.h"



// configure multi buffer
#define MULTI_BUFFER_NUMBER		4


class CSSEWinDriver 
{
public:
    CSSEWinDriver()			{ m_open = false; m_bufmix = NULL; }
    ~CSSEWinDriver()		
	{ 
		// SAFE_DELETE(m_soundEngine);
		SAFE_DELETE_ARRAY(m_bufmix);
	}

    class CSSEMixEngine*	SoundStreamInit( int maxSamples, void* nothing  );
    void					SoundStreamDeInit();

	void	SoundStreamStart()	{};
	void	SoundStreamStop()	{};

	void	SoundStreamSuspend();
	void	SoundStreamResume();

	class CSSEMixEngine*	GetSoundEngine()	{ return m_soundEngine; }

	void	SoundStreamSetReallyStarted()		{ m_reallyStarted++; }
	int		SoundStreamIsReallyStarted()		{ return m_reallyStarted; }

	int		SoundStreamIsStarted()		{ return m_started; }


private:
    static DWORD WINAPI ThreadUpdate(LPVOID lpParameter);

	// waveout
    HWAVEOUT			m_hwaveout;
    WAVEFORMATEX		m_waveformatx;
    WAVEHDR				m_wavehdr[MULTI_BUFFER_NUMBER];

	// thread
    HANDLE				m_thread;
    DWORD				m_threadId;
    CRITICAL_SECTION	m_critSecRtp;
    HANDLE				m_killEvent;
    bool				m_exitThread;
    bool				m_open;

    // buffer mix
#if SSE_MIXING_TO_16_BITS
    u16				*m_bufmix;
    u16				*m_pbufmix;
#else
    u8				*m_bufmix;
    u8				*m_pbufmix;
#endif

    u32				m_bufmixframe;
    u32				m_bufmixtotal;
    u32				m_dmix;
    u32				m_freqmix;


	class CSSEMixEngine*	m_soundEngine;
	int						m_reallyStarted;
	int						m_started;
};





#endif
