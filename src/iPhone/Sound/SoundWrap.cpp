#include "./SoundWrap.h"
#include "../../Sound/SoundManager.h"

#ifdef ENABLE_SOUND

#include "./SoundEngine.h"


#define ADD_SAMPLE_TO_LIST(x,y,z)
#define REMOVE_SAMPLE_FROM_LIST(x)
#define SAMPLE_NONE					0xFFFFFFFF
#define SOUND_MAX					NUM_SOUNDS
#define MM_DELETE					delete

extern char g_AppPath[];
extern void debug_out(const char* x, ...);


CSoundWrap::CSoundWrap()
{
#ifndef USE_IPHONE_INTRO_VIDEO
//	SoundEngine_Initialize(0.0f);
#endif // USE_IPHONE_INTRO_VIDEO
	
	for (int i = 0; i < NUM_SOUNDS; i++)
		m_bLoaded[i] = false;
	m_bIsSoundAndMusicInitialized = false;

#ifdef USE_SKIP_PLAY_SOUNDS_AFTER_INTERRUPT	
	m_bSkipPlaySoundsAfterInterrupt = false;	
#endif	
}

CSoundWrap::~CSoundWrap()
{
	SoundEngine_Teardown();
}


//////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------------------------------

int	CSoundWrap::SampleGetTime( int idx )
{	
#if DISABLE_SFX
    return 10;
#else
	#ifdef USE_SKIP_PLAY_SOUNDS_AFTER_INTERRUPT
		if( m_bSkipPlaySoundsAfterInterrupt )
			return 10;
	#endif
	return (0);
#endif
}

// Setting sound volume
void CSoundWrap::SetMasterVolume( unsigned char vol )
{
    SoundEngine_SetMasterVolume(((float)(vol - MIN_VOLUME)) / (MAX_VOLUME - MIN_VOLUME));
}

void CSoundWrap::SetSfxVolume( unsigned char vol )
{
#ifdef USE_SKIP_PLAY_SOUNDS_AFTER_INTERRUPT
	if( m_bSkipPlaySoundsAfterInterrupt )
		return;
#endif
	SoundEngine_SetEffectsVolume(((float)(vol - MIN_VOLUME)) / (MAX_VOLUME - MIN_VOLUME));
}

void CSoundWrap::SetMusicVolume( unsigned char vol )
{
	SoundEngine_SetBackgroundMusicVolume(((float)(vol - MIN_VOLUME)) / (MAX_MUSIC_VOLUME_DEVICE - MIN_VOLUME));
}

// loading sounds
void CSoundWrap::SampleLoad( int idx, char* data, unsigned long length )
{
#ifdef USE_SKIP_PLAY_SOUNDS_AFTER_INTERRUPT
	if( m_bSkipPlaySoundsAfterInterrupt )
		return;
#endif
	unsigned long id;
	SoundEngine_LoadEffect(data, length, &id);
	m_usSoundMapping[idx] = id;
	m_bLoaded[idx] = true;
}

void CSoundWrap::SampleFree( int idx )
{
#ifdef USE_SKIP_PLAY_SOUNDS_AFTER_INTERRUPT
	if( m_bSkipPlaySoundsAfterInterrupt )
		return;
#endif
	
    SoundEngine_UnloadEffect(m_usSoundMapping[idx]);
	m_bLoaded[idx] = false;
}

void CSoundWrap::SampleStart( int idx, bool loop )
{
#ifdef USE_SKIP_PLAY_SOUNDS_AFTER_INTERRUPT
	if( m_bSkipPlaySoundsAfterInterrupt )
		return;
#endif
	
	SoundEngine_StartEffect(m_usSoundMapping[idx], loop);
}

void CSoundWrap::SampleStop( int idx )
{
#ifdef USE_SKIP_PLAY_SOUNDS_AFTER_INTERRUPT
	if( m_bSkipPlaySoundsAfterInterrupt )
		return;
#endif
	
	SoundEngine_StopEffect (m_usSoundMapping[idx]);
}


void CSoundWrap::SampleStopFade( int idx, unsigned char endvol, int duration )
{
#ifdef USE_SKIP_PLAY_SOUNDS_AFTER_INTERRUPT
	if( m_bSkipPlaySoundsAfterInterrupt )
		return;
#endif
	
    SoundEngine_StopEffect (m_usSoundMapping[idx]);
}

void CSoundWrap::SampleSetVolume( int idx, unsigned char vol, int interpol )
{
#ifdef USE_SKIP_PLAY_SOUNDS_AFTER_INTERRUPT
	if( m_bSkipPlaySoundsAfterInterrupt )
		return;
#endif
	
    SoundEngine_SetEffectLevel (m_usSoundMapping[idx], ((float)(vol - MIN_VOLUME)) / (MAX_VOLUME - MIN_VOLUME));
}

void CSoundWrap::SampleFrequencySet( int idx, int freq, int interpol )
{
#ifdef USE_SKIP_PLAY_SOUNDS_AFTER_INTERRUPT
	if( m_bSkipPlaySoundsAfterInterrupt )
		return;
#endif
	
	SoundEngine_SetEffectPitch(m_usSoundMapping[idx], (2.0f * freq) / MAX_FREQUENCY);
}

bool CSoundWrap::SamplePlaying( int idx )
{
#ifdef USE_SKIP_PLAY_SOUNDS_AFTER_INTERRUPT
	if( m_bSkipPlaySoundsAfterInterrupt )
		return false;
#endif
	
    return SoundEngine_IsEffectPlaying (m_usSoundMapping[idx]);
}

void CSoundWrap::SampleSuspend( int idx)
{
#ifdef USE_SKIP_PLAY_SOUNDS_AFTER_INTERRUPT
	if( m_bSkipPlaySoundsAfterInterrupt )
		return;
#endif
	
	if (idx < 0)
	{
		for (int i = 0; i < NUM_SOUNDS; i++)
			if (m_bLoaded[i])
				SoundEngine_PauseEffect (m_usSoundMapping[i]);
	}
	else
		SoundEngine_PauseEffect (m_usSoundMapping[idx]);
}

void CSoundWrap::SampleResume( int idx )
{
#ifdef USE_SKIP_PLAY_SOUNDS_AFTER_INTERRUPT
	if( m_bSkipPlaySoundsAfterInterrupt )
		return;
#endif
	
	if (idx < 0)
	{
		for (int i = 0; i < NUM_SOUNDS; i++)
			if (m_bLoaded[i])
				SoundEngine_ResumeEffect (m_usSoundMapping[i]);
	}
	else
		SoundEngine_ResumeEffect (m_usSoundMapping[idx]);	
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
/// Music functions 
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

//
// ------------------------------------------------------------------------
// MusicPlaying()
// Append new music to music list 
// For now if other music was loaded previously it has been stopped
// ------------------------------------------------------------------------
//
bool CSoundWrap::MusicLoad( const char* name ) 
{
	if(!m_bIsSoundAndMusicInitialized) return false;
	
	char filePath[1024];
	strcpy(filePath, g_AppPath);
	strcat(filePath, name);
	
	//if(SoundEngine_LoadBackgroundMusicTrack(filePath, false, false) != noErr)
	//	return false;	
	SoundEngine_LoadBackgroundMusicTrack(filePath, false, false);	
	
	return true;		
}	

//
// ------------------------------------------------------------------------
// MusicStart()
// ------------------------------------------------------------------------
//
void CSoundWrap::MusicStart( bool loop ) 
{
	if(!m_bIsSoundAndMusicInitialized) return;
	SoundEngine_StartBackgroundMusic();
}

//
// ------------------------------------------------------------------------
// MusicStop()
// ------------------------------------------------------------------------
//
void CSoundWrap::MusicStop( ) 
{
	if(!m_bIsSoundAndMusicInitialized) return;
	SoundEngine_StopBackgroundMusic(false);
}		

//void CSoundWrap::MusicStopAtEnd( ) 
//{
//	if(!m_bIsSoundAndMusicInitialized) return;
//	SoundEngine_StopBackgroundMusic(true);
//}		

//
// ------------------------------------------------------------------------
// MusicPause()
// ------------------------------------------------------------------------
//
void CSoundWrap::MusicPause( const bool pause ) 
{
}

//
// ------------------------------------------------------------------------
// MusicPlaying()
// Return true, if music was playing.
// ------------------------------------------------------------------------
//
bool CSoundWrap::MusicPlaying( )
{
	if(!m_bIsSoundAndMusicInitialized) return false;
	return SoundEngine_IsMusicPlaying();
}

//
// ------------------------------------------------------------------------
// MusicFree()
// ------------------------------------------------------------------------
//
void CSoundWrap::MusicFree() 
{
	if(!m_bIsSoundAndMusicInitialized) return;
	SoundEngine_UnloadBackgroundMusicTrack();
}	



#else /* ENABLE_SOUND */

CSoundWrap::CSoundWrap()
{
}

CSoundWrap::~CSoundWrap()
{
}


//////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------------------------------

int	CSoundWrap::SampleGetTime( int idx )
{
    return 10;
}

// Setting sound volume
void CSoundWrap::SetMasterVolume( unsigned char vol )
{
}

void CSoundWrap::SetSfxVolume( unsigned char vol )
{
}

void CSoundWrap::SetMusicVolume( unsigned char vol )
{
}

// loading sounds
void CSoundWrap::SampleLoad( int idx, char* data, unsigned long length )
{
}

void CSoundWrap::SampleFree( int idx )
{
 
}

void CSoundWrap::SampleStart( int idx, bool loop )
{
}



void CSoundWrap::SampleStop( int idx )
{
}


void CSoundWrap::SampleStopFade( int idx, unsigned char endvol, int duration )
{
}

void CSoundWrap::SampleSetVolume( int idx, unsigned char vol, int interpol )
{
}

void CSoundWrap::SampleFrequencySet( int idx, int freq, int interpol )
{
}

bool CSoundWrap::SamplePlaying( int idx )
{
	return false;
}

void CSoundWrap::SampleSuspend( int idx)
{
}

void CSoundWrap::SampleResume( int idx )
{
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
/// Music functions 
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

//
// ------------------------------------------------------------------------
// MusicPlaying()
// Append new music to music list 
// For now if other music was loaded previously it has been stopped
// ------------------------------------------------------------------------
//
bool CSoundWrap::MusicLoad( const char* name ) 
{
}	

//
// ------------------------------------------------------------------------
// MusicStart()
// ------------------------------------------------------------------------
//
void CSoundWrap::MusicStart( bool loop ) 
{
}

//
// ------------------------------------------------------------------------
// MusicStop()
// ------------------------------------------------------------------------
//
void CSoundWrap::MusicStop( ) 
{
}		

//
// ------------------------------------------------------------------------
// MusicPause()
// ------------------------------------------------------------------------
//
void CSoundWrap::MusicPause( const bool pause ) 
{
}

//
// ------------------------------------------------------------------------
// MusicPlaying()
// Return true, if music was playing.
// ------------------------------------------------------------------------
//
bool CSoundWrap::MusicPlaying( )
{
	return false;
}

//
// ------------------------------------------------------------------------
// MusicFree()
// ------------------------------------------------------------------------
//
void CSoundWrap::MusicFree() 
{
}	

#endif /* ENABLE_SOUND */
