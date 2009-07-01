#ifndef _SOUND_WRAP_H_
#define _SOUND_WRAP_H_

#include "SoundsTable.h"
#include "Config.h"

#define NUM_SOUNDS_TOTAL (NUM_SOUNDS)


class CSoundWrap
{
public:
	CSoundWrap();
	~CSoundWrap();

	void		SetMasterVolume( unsigned char vol );
	void		SetSfxVolume( unsigned char vol );
	void		SetMusicVolume( unsigned char vol );

	void		SampleLoad( int idx, char* data, unsigned long length );
	void		SampleFree( int idx );
	void		SampleStart( int idx, bool loop );
//	void		SampleStartFade( int idx, unsigned char startvol, unsigned char endvol, int duration, bool loop );
	int			SampleGetTime( int idx );
	void		SampleStop( int idx );
	void		SampleStopFade( int idx, unsigned char endvol, int duration );
	void		SampleSetVolume( int idx, unsigned char vol, int interpol = 0 );
	void		SampleFrequencySet( int idx, int freq, int interpol = 0 );
	bool		SamplePlaying( int idx );
	void		SampleSuspend( int idx = -1);
	void		SampleResume( int idx = -1);

	bool		MusicLoad( const char* name );
	void		MusicStart( bool loop );
	void		MusicStop( );
//	void		MusicStopAtEnd();
	void		MusicPause( const bool pause );
	void		MusicFree();
	bool		MusicPlaying( );
	
	
	bool		Init() { return false; }
	void		Start() {}
	void		Stop() {}
	void		Update() {}
    void		Suspend() {}
    void		Resume() {}
	
    void		EnterPhoneCall() {}
    void		ExitPhoneCall() {}
	void		SetMuteWhenInCall(bool in_bMuteWhenInCall)  {}
	
	///Stream functions.
	void		LoadStream(char* inPath) {};
	void		QueueStream(char* inPath) {};
	void		PlayStream() {};
	void		StopStream() {};
	void		LoopStream(bool inBool) {};
	void		SetVolumeStream(unsigned short inVol) {};
	void		SetHalfSpeedStream(bool inHalfSpeed) {};
	bool		m_bIsSoundAndMusicInitialized;

#ifdef USE_SKIP_PLAY_SOUNDS_AFTER_INTERRUPT
	bool		m_bSkipPlaySoundsAfterInterrupt;
#endif
	
private:
	
	bool			m_bLoaded[NUM_SOUNDS];
	unsigned long	m_usSoundMapping[NUM_SOUNDS];
};


#endif
