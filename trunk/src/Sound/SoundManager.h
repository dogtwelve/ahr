////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _SOUND_MANAGER_
#define _SOUND_MANAGER_

////////////////////////////////////////////////////////////////////////////////////////////////////
#include "A4_3d_platform_def.h"

#ifdef IPHONE
#include <pthread.h>
#endif
#include <stdio.h>

#ifdef NGI
#include "../_nGage/src/Sound/SoundWrap.h"
#elif __SYMBIAN32__
#include "../_Symbian/src/Sound/SoundWrap.h"
#elif IPHONE
#include "../iPhone/Sound/SoundWrap.h"
#else
#include "SoundWrap.h"
#endif // __SYMBIAN32__

#include "SoundsTable.h"
#include "Globals.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////

#define NO_ACTOR_KEY			(0)
#define NO_DELAY				(0)
#define NO_REPLAY_DELAY			(0)
#define NO_CUSTOM_PRIORITY		(0)

#define SOUND_VSCRIPT_DE_BASE_INDEX				SOUND_VSCRIPT_DE_ADVANCE_TO_NEXT_POSITION
#define SOUND_VSCRIPT_DE_LAST_INDEX				SOUND_VSCRIPT_DE_THEY_ARE_TOO_MANY

#define SOUND_VCODE_DE_BASE_INDEX				SOUND_VCODE_DE_FIND_SOME_COVER
#define SOUND_VCODE_DE_LAST_INDEX				SOUND_VCODE_DE_HOLD_POSIOTION

#define SOUND_VSCRIPT_US_BASE_INDEX				SOUND_VSCRIPT_US_FIX_THEM
#define SOUND_VSCRIPT_US_LAST_INDEX				SOUND_VSCRIPT_US_WERE_PULLIN_OUT

class SoundManager
{
public:
	bool soundsLoaded;
	void* snd;
	unsigned char** m_SoundData;
	int lengthsSound [NUM_SOUNDS_TOTAL];
	//bool m_bSoundInitialize;
	long m_soundStartTime;
	CSoundWrap* m_soundWrap;
	bool m_bSuspended;
	bool m_bPlaySounds;

	unsigned char mMasterVolume;
	unsigned char mSfxVolume;
	unsigned char mMusicVolume;

	SoundManager();
	~SoundManager();

	int m_indexLoadedSound;
	//int loadAllSounds();
	//int freeAllSounds();
	bool loadCurrentSound();

	bool m_bAreEffectsLoaded;

	//avoid destroy the sounds from interrupt when a load/start is excuted
	bool m_bSoundAndMusicProcessing;
	bool m_bFreeRequest;

	void loadAllEffect();
	void releaseAllEffect();
	void initialize();
	void destroy();

//wrappers
	void RemoveAllStoppedSamples();
	void SampleStart(int soundId, bool repeat);
	void SampleSetVolume(int soundId, int volume);
	void SampleFrequencySet( int idx, int freq, int interpol = 0 );
	int	 SampleGetTime( int idx );
	void SetMasterVolume( unsigned char vol );
	void SetSfxVolume( unsigned char vol );
	void SetMusicVolume( unsigned char vol );

	unsigned char GetMasterVolume();
	unsigned char GetSfxVolume();
	unsigned char GetMusicVolume();

	void EnterPhoneCall();
	void ExitPhoneCall();

//unsolved
	bool MusicPlaying(){return 1;}
	void MusicPause( const bool pause ) {}
	void SampleResume(int a){}

	//void playSound(int soundId);
	//void playSound(int soundId, int loopCount, int actorKey, int delay, int replayDelay = NO_REPLAY_DELAY, int priorityMod = NO_CUSTOM_PRIORITY);
	void SampleStop(int soundId, int actorKey = -1);
	void stopAllSounds(bool bUseLockProcessing = true); //usefull parameter in synchronize ... avoi recursive mutex
	void setMasterVolume(unsigned char);
	bool isPlaying();
	bool SamplePlaying(int id);

	int		suspend(bool bIGM = false);
	int		resume(bool bIGM = false);
	int		isSilence();
	inline bool isSuspended();

	void setVolumeSoundId(int sndId, int vol);

	void	update();

	inline	bool useMultipleChannels();
	void setMultipleChannels(bool value);

	void	refreshLongSongs(bool bStartIfOFF);

	//Queries the device volume
	void	queryDeviceVolume();
	void	setDeviceVolume(int vol);

	//Returns the game's SoundManager instance
	//static SoundManager* getInstance();

	void getAudio(int*, int len);
	void StartMixer();
	void StopMixer();

	inline void SetSoundStatus(bool bPlaySounds) { m_bPlaySounds = bPlaySounds;}
	inline bool IsSoundOn() { return m_bPlaySounds; }
	
#ifdef USE_SKIP_PLAY_SOUNDS_AFTER_INTERRUPT
	inline void skipPlaySoundsAfterInterrupt(){ m_soundWrap->m_bSkipPlaySoundsAfterInterrupt = true;}
#endif

#ifdef ENABLE_VIBRATION
	void vibrate();
#endif

#ifdef IPHONE
private:
	pthread_mutex_t s_mutex;

public:
	inline void lockProcessing() 
	{ 
		pthread_mutex_lock(&s_mutex);
		//printf("lock\n");		
	}
	inline void unlockProcessing() 
	{ 
		//printf("unlock\n");
		pthread_mutex_unlock(&s_mutex);
	}
#else
public:
	inline void lockProcessing() {}
	inline void unlockProcessing(){}
#endif
};


////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //EOF

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
