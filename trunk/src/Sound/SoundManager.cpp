// SoundManager.cpp
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "SoundManager.h"
#include "File.h"
#include "File_iPhone.h"

#ifdef IPHONE
#include <string.h>
	#include "iPhone/Sound/SoundEngine.h"
#endif

//#ifdef __WIN32__
////#include <windows.h>
//#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
//Returns the game's SoundManager instance

////////////////////////////////////////////////////////////////////////////////////////////////////

#define	USE_DUAL_CHANNEL	(1)

////////////////////////////////////////////////////////////////////////////////////////////////////

#define NORMAL_DELAY			(2)

//Loop constants
#define LOOP_INFINITE	(0x7FFFFFFF)
#define LOOP_ONCE		(0x00000001)
#define LOOP_TWICE		(0x00000002)

SoundManager::SoundManager()
{
	m_soundWrap = NEW CSoundWrap();
	m_soundWrap->Init();
	m_soundWrap->Start();

	m_bSuspended = false;
	soundsLoaded = false;
	m_bPlaySounds = false; // Must call SetSoundStatus(true) to activate playing sounds

	m_indexLoadedSound = 0;

	m_SoundData = NULL;
	m_bAreEffectsLoaded = false;

	m_bSoundAndMusicProcessing = false;
	m_bFreeRequest = false;
#ifdef IPHONE
	pthread_mutex_init(&s_mutex, NULL);
#endif
}

SoundManager::~SoundManager()
{
#if defined __SYMBIAN32__ && !defined NGI
#else
	if (m_SoundData)
	{
		for (int i = 0; i < NUM_SOUNDS_TOTAL; i++)
		{
			SAFE_DELETE_ARRAY(m_SoundData[i]);
			//m_soundWrap->SampleFree(i);
		}
	}
	SAFE_DELETE_ARRAY(m_SoundData);
	SAFE_DELETE(m_soundWrap);

#ifdef IPHONE
	pthread_mutex_destroy(&s_mutex);
#endif
#endif
}
/*
int SoundManager::freeAllSounds()
{
	if (m_SoundData)
	{
		for (int i = 0; i < NUM_SOUNDS_TOTAL; i++)
		{
			SAFE_DELETE(m_SoundData[i]);
			//m_soundWrap->SampleFree(i);
		}
	}
	SAFE_DELETE_ARRAY(m_SoundData);

	m_indexLoadedSound = 0;

	return 0;
}

int SoundManager::loadAllSounds()
{
#if DISABLE_SFX
    return 0;
#else 

	if (soundsLoaded == true)
	{
		return S_OK;
	}
	soundsLoaded = true;
	
	m_SoundData = NEW unsigned char*[NUM_SOUNDS_TOTAL];

	int *nLengths = NEW int[NUM_SOUNDS_TOTAL];

	for(int nIndex = 0; nIndex < NUM_SOUNDS_TOTAL; nIndex++)
	{
		char szFilePath[512];
		
		if (nIndex < NUM_SOUNDS)
		{
			strcpy(szFilePath, "Sounds\\");
			strcat(szFilePath, SOUND_FILE[nIndex]);
		}
		else
		{
			strcpy(szFilePath, "Musics\\");
			strcat(szFilePath, MUSIC_FILE[nIndex - NUM_SOUNDS]);
		}
		//rax - the files already have extension (.wav, .znd, etc)
		//strcat(szFilePath, ".wav");

		A_IFile *pFile = A_IFile::Open( szFilePath, A_IFile::OPEN_READ | A_IFile::OPEN_BINARY, false);

		if(!pFile)
		{
			return E_FAIL;
		}

		int nLength = pFile->GetSize();
		m_SoundData[nIndex] = NEW unsigned char[nLength];
		//char* buffer = NEW char[size];		
		if(!m_SoundData[nIndex])
		{
			return E_FAIL;
		}
		
		pFile->Read(m_SoundData[nIndex], nLength);
		A_IFile::Close(pFile);

		nLengths[nIndex] = nLength;
	}


	for(int nIndex = 0; nIndex < NUM_SOUNDS_TOTAL; nIndex++)
	{
		m_soundWrap->SampleLoad(nIndex, (char*) m_SoundData[nIndex], nLengths[nIndex]);
	}
	
	MM_DELETE nLengths;

	return S_OK;
#endif 
}
*/

// Returns true when the last sound was loaded
bool SoundManager::loadCurrentSound()
{
#if DISABLE_SFX
    return 0;
#else /* DISABLE_SFX */

	if (m_indexLoadedSound == NUM_SOUNDS_TOTAL)
	{
		return true;
	}
	
	if (m_SoundData == NULL)
	{
		m_SoundData = NEW unsigned char*[NUM_SOUNDS_TOTAL];
		memset(m_SoundData, 0, NUM_SOUNDS_TOTAL * sizeof(char*));
	}

	int nLength;

	int nIndex = m_indexLoadedSound;
	m_indexLoadedSound++;

	char szFilePath[512];
		
	if (nIndex < NUM_SOUNDS)
	{
		strcpy(szFilePath, "sfx\\");
		strcat(szFilePath, SOUND_FILE[nIndex]);
	}
	else
	{
		strcpy(szFilePath, "bgm\\");
		strcat(szFilePath, MUSIC_FILE[nIndex - NUM_SOUNDS]);
	}
	//rax - the files already have extension (.wav, .znd, etc)
	//strcat(szFilePath, ".wav");

	A_IFile *pFile = A_IFile::Open( szFilePath, A_IFile::OPEN_READ | A_IFile::OPEN_BINARY, false);

	if (!pFile)
	{
		return false; // E_FAIL;
	}

	nLength = pFile->GetSize();
	
	SAFE_DELETE(m_SoundData[nIndex]);
	m_SoundData[nIndex] = NEW unsigned char[nLength];	
	lengthsSound[nIndex]=nLength;

	if(!m_SoundData[nIndex])
	{
		return false; // E_FAIL;
	}
	
	pFile->Read(m_SoundData[nIndex], nLength);
	A_IFile::Close(pFile);
#if defined IPHONE
	if(false)
#endif
	{
		m_soundWrap->SampleLoad(nIndex, (char*) m_SoundData[nIndex], nLength);
	}
	
	return (m_indexLoadedSound == NUM_SOUNDS_TOTAL);
#endif /* DISABLE_SFX */
}



void SoundManager::initialize()
{
	//printf("initialize\n");
	if(!m_soundWrap->m_bIsSoundAndMusicInitialized)
	{
#ifdef IPHONE
		SoundEngine_Initialize(0.0f);
#endif
		m_soundWrap->m_bIsSoundAndMusicInitialized = true;
	}
}

void SoundManager::destroy()
{
	//printf("destroy\n");
	if(!m_soundWrap->m_bIsSoundAndMusicInitialized)
	{
		return;
	}

	lockProcessing();

	if(m_bSoundAndMusicProcessing)
	{
		unlockProcessing();
		m_bFreeRequest = true;	
		return;
	}

	#ifdef IPHONE
	SoundEngine_Teardown();
	#endif
	m_soundWrap->m_bIsSoundAndMusicInitialized = false;
	m_bAreEffectsLoaded = false;
	
	unlockProcessing();
}

void SoundManager::loadAllEffect()
{
	lockProcessing();
	m_bSoundAndMusicProcessing = true;

	initialize();
	
	if(!m_bAreEffectsLoaded)
	{
		for (int nIndex = 0; nIndex < NUM_SOUNDS_TOTAL; nIndex++)
		{
			m_soundWrap->SampleLoad(nIndex, (char*) m_SoundData[nIndex], lengthsSound[nIndex]);
		}
		m_bAreEffectsLoaded = true;
	}
	
	m_bSoundAndMusicProcessing = false;
	unlockProcessing();

	//a destroy request was made ( maybe an interrupt)
	if(m_bFreeRequest)
	{
		destroy();
		m_bFreeRequest= false;
	}
}
void SoundManager::releaseAllEffect()
{
	//for (int nIndex = 0; nIndex < NUM_SOUNDS_TOTAL; nIndex++)
	//{
	//	m_soundWrap->SampleFree(nIndex);
	//}		
}
//wrappers
int	SoundManager::SampleGetTime( int idx )
{
	return  m_soundWrap->SampleGetTime(idx);
}

void SoundManager::SetMasterVolume( unsigned char vol )
{
#ifdef USE_SOUND_CORRECTION
	if (R320x320 || is_HTC_P5500)
	{
		vol = vol * 7 / 5;
	}
#endif

	mMasterVolume = vol;
	m_soundWrap->SetMasterVolume(vol);
}

void SoundManager::SetSfxVolume( unsigned char vol )
{
	mSfxVolume = vol;
	m_soundWrap->SetSfxVolume(vol);
}

void SoundManager::SetMusicVolume( unsigned char vol )
{
	mMusicVolume = vol;
	m_soundWrap->SetMusicVolume(vol);
}

unsigned char SoundManager::GetMasterVolume()
{
	//return m_soundWrap->GetMasterVolume();
	return mMasterVolume;
}

unsigned char SoundManager::GetSfxVolume()
{
	//return m_soundWrap->GetSfxVolume();
	return mSfxVolume;
}

unsigned char SoundManager::GetMusicVolume()
{
	//return m_soundWrap->GetMusicVolume();
	return mMusicVolume;
}

void RemoveStoppedSamples()
{
#ifdef IPHONE
	SoundEngine_RemoveStoppedEffects();
#endif
}

void SoundManager::SampleStart(int soundId, bool repeat)
{
	if (!m_bPlaySounds)
		return;
	
	lockProcessing();
	m_bSoundAndMusicProcessing = true;

	m_soundWrap->SampleStart(soundId,repeat);

	//a destroy request was made ( maybe an interrupt)	
	m_bSoundAndMusicProcessing = false;
	unlockProcessing();

	if(m_bFreeRequest)
	{
		destroy();
		m_bFreeRequest = false;
	}
}

void SoundManager::SampleSetVolume(int soundId, int volume)
{
	lockProcessing();
	m_bSoundAndMusicProcessing = true;
	
	setVolumeSoundId(soundId, volume);
	
	//a destroy request was made ( maybe an interrupt)	
	m_bSoundAndMusicProcessing = false;
	unlockProcessing();
	
	if(m_bFreeRequest)
	{
		destroy();
		m_bFreeRequest = false;
	}	
}

//void SoundManager::playSound(int soundId)
//{
//	playSound(soundId, LOOP_ONCE, NO_ACTOR_KEY, NO_DELAY, NO_REPLAY_DELAY, NO_CUSTOM_PRIORITY);
//}

//void SoundManager::playSound(int soundId, int loopCount, int actorKey, int delay, int replayDelay, int priorityMod)
//{
//	m_soundWrap->SampleStart(soundId,loopCount==LOOP_INFINITE?true:false);
//}

void SoundManager::SampleStop(int soundId, int actorKey)
{
	m_soundWrap->SampleStop(soundId);
}

void SoundManager::stopAllSounds(bool bUseLockProcessing)
{
	if(bUseLockProcessing)
		lockProcessing();
	
	for(int i=0;i<NUM_SOUNDS_TOTAL;i++)
	{
		m_soundWrap->SampleStop(i);
	}
	
	if(bUseLockProcessing)
		unlockProcessing();
}

void SoundManager::setMasterVolume(unsigned char vol)
{
	m_soundWrap->SetMasterVolume(vol);
	for(int i=0;i<NUM_SOUNDS_TOTAL;i++)
	{
		m_soundWrap->SampleSetVolume(i,vol);
	}
}

void SoundManager::SampleFrequencySet( int idx, int freq, int interpol )
{
	m_soundWrap->SampleFrequencySet(idx, freq, interpol);
}

bool SoundManager::SamplePlaying(int id)
{
	return m_soundWrap->SamplePlaying(id);
}

void SoundManager::EnterPhoneCall()
{
	return m_soundWrap->EnterPhoneCall();
}

void SoundManager::ExitPhoneCall()
{
	return m_soundWrap->ExitPhoneCall();
}

bool SoundManager::isPlaying()
{ 
	for(int i=0;i<NUM_SOUNDS_TOTAL;i++)
	{
		if(m_soundWrap->SamplePlaying(i))
		{
			return true;
		}
	}
	return false;
}

int	SoundManager::suspend(bool bIGM)		
{ 
	m_bSuspended = true;
	stopAllSounds();
	return 0; 
}

int	SoundManager::resume(bool bIGM)		
{ 
	m_bSuspended = false;
	return 0; 
}
	
int	SoundManager::isSilence()						
{ 
	return 0;
}											
	
inline bool SoundManager::isSuspended()			
{ 
	return m_bSuspended;
}

void SoundManager::setVolumeSoundId(int sndId, int vol) 
{
	m_soundWrap->SampleSetVolume(sndId,vol);
}

void SoundManager::update()						
{ 
	m_soundWrap->Update();
}

inline	bool SoundManager::useMultipleChannels()	
{ 
	return false; 
}
	
void SoundManager::setMultipleChannels(bool value)	
{ 
}

void	SoundManager::refreshLongSongs(bool bStartIfOFF)				
{ 
}

//Queries the device volume
void	SoundManager::queryDeviceVolume() 
{
}
	
void	SoundManager::setDeviceVolume(int vol) 
{
}

void SoundManager::getAudio(int*, int len) 
{
}

void SoundManager::StartMixer() 
{
}

void SoundManager::StopMixer() 
{
}

#ifdef ENABLE_VIBRATION
	void SoundManager::vibrate()
	{
	#ifdef IPHONE
		SoundEngine_Vibrate();
	#endif
	}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
