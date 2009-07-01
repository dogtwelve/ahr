#include "soundwrap.h"
#ifdef __WIN32__
#include "..\SoundSSE\SSEDriver.h"
#endif // __WIN32__
#include "SoundManager.h"
#include "SoundsTable.h"
//#include "MemoryAllocation.h"

#define ADD_SAMPLE_TO_LIST(x,y,z)
#define REMOVE_SAMPLE_FROM_LIST(x)
#define SAMPLE_NONE		0xFFFFFFFF
#define SOUND_MAX NUM_SOUNDS_TOTAL
#define MM_DELETE delete

static bool loaded[NUM_SOUNDS_TOTAL];


CSoundWrap::CSoundWrap()
{
	for(int index = 0; index < NUM_SOUNDS_TOTAL; ++index)
	{
		loaded[index] = false;
	}

#if DISABLE_SFX
    return;
#endif

	for (int i = 0; i < SOUND_MAX; i++)
	{
		m_SamplesIdx[i] = -1;
		m_SamplesTime[i] = 0;
	}
	m_pSoundDriver = NULL;
}

CSoundWrap::~CSoundWrap()
{
#if DISABLE_SFX == 0
	m_pSoundDriver->GetSoundEngine()->StopAllVoices();
	m_pSoundDriver->SoundStreamDeInit();
	SAFE_DELETE(m_pSoundDriver);
#endif // DISABLE_SFX
}


bool CSoundWrap::Init()
{
	m_bIsResumed = true;
	m_bIsStarted = false;
	m_bInPhoneCall = false;
	m_bMuteWhenInCall = false;
	m_bStoppedVol = false;

#if DISABLE_SFX
    return true;
#endif

	m_pSoundDriver = NEW CSSEWinDriver();
	m_pSoundDriver->SoundStreamInit( SOUND_MAX, NULL );
	return true;
}

void CSoundWrap::Start()
{
#if DISABLE_SFX
    return;
#endif

	if (m_bInPhoneCall && m_bMuteWhenInCall)
	{
		m_bIsStarted = true;
		m_bIsResumed = false;

		return;
	}

	if (!m_bIsStarted)
	{
		if (!m_bStoppedVol)
		{
			m_pSoundDriver->SoundStreamStart();
		}
		m_bIsStarted = true;
	}
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//You MUST pass the absolute path of the file
/*void CSoundWrap::LoadStream(char* inPath)
{
	m_sCurrentStream = inPath;
}*/

//////////////////////////////////////////////////////////////////////////

//You MUST pass the absolute path of the file
void CSoundWrap::QueueStream(char* inPath)
{
}


//////////////////////////////////////////////////////////////////////////

void CSoundWrap::PlayStream()
{
}

 //////////////////////////////////////////////////////////////////////////
 
void CSoundWrap::StopStream()
{
}

//////////////////////////////////////////////////////////////////////////

void CSoundWrap::LoopStream(bool inLoop)
{
}

//////////////////////////////////////////////////////////////////////////

void CSoundWrap::SetVolumeStream(unsigned short inVol)
{
}

void CSoundWrap::SetHalfSpeedStream(bool inHalfSpeed)
{
}


//////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------------------------------

void		CSoundWrap::Stop()
{
#if DISABLE_SFX
    return;
#endif

	if (m_bIsStarted)
	{
		m_pSoundDriver->SoundStreamStop();

		m_bIsStarted = false;
		m_bIsResumed = false;
	}
}

int			CSoundWrap::SampleGetTime( int idx )
{
#if DISABLE_SFX
    return 10;
#else
		return (m_SamplesTime[idx]);
#endif
}

void		CSoundWrap::Update()
{
#if DISABLE_SFX
    return;
#else
		if (m_pSoundDriver && m_pSoundDriver->GetSoundEngine())
		{
			// avoid init problems, by checking if really started, and starting it if necessary
			if (m_pSoundDriver->SoundStreamIsStarted())
			{
				if (m_pSoundDriver->SoundStreamIsReallyStarted() <= 1)
				{
					m_pSoundDriver->SoundStreamStart();
				}
			}

			// check all running samples to see if they are finished :
			for (int i = 0; i < SOUND_MAX; i++)
			{
				if (m_SamplesIdx[i] != -1)
				{
					if (!m_pSoundDriver->GetSoundEngine()->IsVoicePlaying( m_SamplesIdx[i] ))
						m_SamplesIdx[i] = -1;	// clear if finished
					else
						m_SamplesTime[i]++;	// inc time
				}
			}
		}
#endif
}

void		CSoundWrap::Suspend()
{
#if DISABLE_SFX
    return;
#endif
	m_pSoundDriver->SoundStreamStop();
	m_pSoundDriver->SoundStreamSuspend();
	m_bIsResumed = false;
}


void		CSoundWrap::Resume()
{
#if DISABLE_SFX
    return;
#endif
//BBucur.20.09.2004. Debug.
	SetMasterVolume(m_bMasterVolume);

	if (!m_bIsResumed)
	{
		// Code copied from Start() function
		// but doesn't affect m_bIsStarted
		if (m_bIsStarted)
		{
			if (!m_bStoppedVol)
			{
				m_pSoundDriver->SoundStreamResume();
				m_pSoundDriver->SoundStreamStart();
			}
		}
		m_bIsResumed = true;
	}
}

void CSoundWrap::EnterPhoneCall()
{
	m_bInPhoneCall	=	true;
	if(m_bMuteWhenInCall)
		if(m_bIsResumed)
			Suspend();
}

void CSoundWrap::ExitPhoneCall()
{
	m_bInPhoneCall = false;

	if(m_bMuteWhenInCall)
	{
		Suspend();
		Resume();
	}
	else
	{
		Suspend();
		Resume();
	}
}

void		CSoundWrap::SetMuteWhenInCall(bool in_bMuteWhenInCall)
{
	if (m_bMuteWhenInCall != in_bMuteWhenInCall)
	{
		m_bMuteWhenInCall = in_bMuteWhenInCall;

		if (m_bInPhoneCall)
		{
			if (m_bMuteWhenInCall)
			{
				Suspend();
			}
			else
			{
				Resume();
			}
		}
	}
}

//Modified by Bogdan Bucur on 11.09.2004
void CSoundWrap::SetMasterVolume( unsigned char vol )
{
#if DISABLE_SFX
    return;
#endif

	m_pSoundDriver->GetSoundEngine()->SetMasterVolume( vol * 2 );
}

void CSoundWrap::SetSfxVolume( unsigned char vol )
{
}

void CSoundWrap::SetMusicVolume( unsigned char vol )
{
}

void CSoundWrap::SampleLoad( int idx, char* _ptr, int length )
{
	loaded[idx] = true;
#if DISABLE_SFX
    return;
#endif

//	A_ASSERT( idx < SOUND_MAX );
	m_pSoundDriver->GetSoundEngine()->NewSampleFromMemWAV( idx, _ptr );
}

void CSoundWrap::SampleFree( int idx )
{
//	A_ASSERT(loaded[idx]);
	loaded[idx] = false;
#if DISABLE_SFX
    return;
#endif

	m_pSoundDriver->GetSoundEngine()->DeleteSample( idx );
}

void		CSoundWrap::SampleStart( int idx, bool loop )
{
//	A_ASSERT(loaded[idx]);
#if DISABLE_SFX
    return;
#else
		if (m_SamplesIdx[idx] != -1)	// play only if not started already !
		{
			if (m_pSoundDriver->GetSoundEngine()->IsVoicePlaying( m_SamplesIdx[idx] ) == 0)
			{
                m_SamplesIdx[idx] = -1;	// clear if finished
			}
		}

		if (m_SamplesIdx[idx] == -1)
		{
			m_SamplesIdx[idx] = m_pSoundDriver->GetSoundEngine()->PlaySample( idx, SSE_VOLUME_ONE, SSE_PITCH_ONE, loop, 1 );
			m_SamplesTime[idx] = 0;
		}
#endif
}

void		CSoundWrap::SampleStartFade( int idx, unsigned char startvol, unsigned char endvol, int duration, bool loop )
{
#if DISABLE_SFX
    return;
#else

#endif
}


void		CSoundWrap::SampleStop( int idx )
{
#if DISABLE_SFX
    return;
#endif

	if (m_SamplesIdx[idx] != -1)
	{
		m_pSoundDriver->GetSoundEngine()->StopVoice( m_SamplesIdx[idx] );
		m_SamplesIdx[idx] = -1;
		m_SamplesTime[idx] = 0;
	}
}


void CSoundWrap::SampleStopFade( int idx, unsigned char endvol, int duration )
{
#if DISABLE_SFX
    return;
#else
#endif
}

void CSoundWrap::SampleSetVolume( int idx, unsigned char vol, int interpol )
{
#if DISABLE_SFX
    return;
#else

	if (m_SamplesIdx[idx] != -1)
		m_pSoundDriver->GetSoundEngine()->SetVoiceVolume( m_SamplesIdx[idx], vol * 2, interpol );	
#endif
}

void CSoundWrap::SampleFrequencySet( int idx, int freq, int interpol )
{
#if DISABLE_SFX
    return;
#endif

	if (m_SamplesIdx[idx] != -1)
	{
		m_pSoundDriver->GetSoundEngine()->SetVoicePitch( m_SamplesIdx[idx], (SSE_PITCH_ONE * freq) / 16000, interpol );	// TEMP we assume that the orginal sound frequency was 16000 because SoundWrap sets the frequency of the sound
	}
}

bool CSoundWrap::SamplePlaying( int idx )
{
#if DISABLE_SFX
    return false;
#endif

	if (m_SamplesIdx[idx] == -1)
	{
		return 0;
	}
	else
	{
		if (m_pSoundDriver->GetSoundEngine()->IsVoicePlaying( m_SamplesIdx[idx] ))
			return true;
		else
		{
			m_SamplesIdx[idx] = -1;	// not playing -> clean
			return false;
		}
	}
}

void		CSoundWrap::MusicStart( bool loop )
{
#if DISABLE_SFX
    return;
#endif
}

void		CSoundWrap::MusicStop()
{
#if DISABLE_SFX
    return;
#endif
}

void		CSoundWrap::MusicLoadXM( const char* name )
{
	// Disabled XM music
	return;
}

void		CSoundWrap::MusicLoadXTI( const char* name )
{
	// Disabled XM music
	return;
}

void		CSoundWrap::MusicLoadXTM( const char* name )
{
	// Disabled XM music
	return;
}
	
void		CSoundWrap::MusicFreeXTI(  )
{
}

void		CSoundWrap::MusicFreeXTM( )
{
	// Disabled XM music
}

void		CSoundWrap::MusicFreeXM( )
{
}

