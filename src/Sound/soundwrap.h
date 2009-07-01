#ifndef _SOUND_WRAP_H_
#define _SOUND_WRAP_H_

#ifdef __WIN32__
#include "../SoundSSE/SSEDriver.h"
#endif // __WIN32__
#include "SoundsTable.h"

#ifdef WINDOWS_MOBILE_WAV_MUSIC
#define NUM_SOUNDS_TOTAL (NUM_SOUNDS + NUM_MUSICS)
#else
#define NUM_SOUNDS_TOTAL (NUM_SOUNDS)
#endif



typedef struct
{
	void*			sample;
	unsigned char	vol;
#define SAMPLE_FLAG_LOOP	0x1
#define SAMPLE_FLAG_FADEIN	0x2
#define SAMPLE_FLAG_FADEOUT	0x4
	unsigned char	flags;
	unsigned short	time;
	unsigned char	volstart;
	unsigned char	volend;
	unsigned short	fadetime;
} SAMPLE_INFO;


class CSoundWrap
{
public:
	CSoundWrap();
	~CSoundWrap();

	bool		Init();

	void		Start();
	void		Stop();

	void		Update();

    void		Suspend();
    void		Resume();

    void		EnterPhoneCall();	//BBucur.15.10.2004.
    void		ExitPhoneCall();
	void		SetMuteWhenInCall(bool in_bMuteWhenInCall);

	void		SetMasterVolume( unsigned char vol );
	void		SetSfxVolume( unsigned char vol );
	void		SetMusicVolume( unsigned char vol );

	void		SampleLoad( int idx, char* data, int length );
	void		SampleFree( int idx );
	void		SampleStart( int idx, bool loop );
	void		SampleStartFade( int idx, unsigned char startvol, unsigned char endvol, int duration, bool loop );
	int			SampleGetTime( int idx );
	void		SampleStop( int idx );
	void		SampleStopFade( int idx, unsigned char endvol, int duration );
	void		SampleSetVolume( int idx, unsigned char vol, int interpol = 0 );
	void		SampleFrequencySet( int idx, int freq, int interpol = 0 );
	bool		SamplePlaying( int idx );

	///Stream functions.
	//void		LoadStream(char* inPath);
	void		QueueStream(char* inPath);
	void		PlayStream();
	void		StopStream();
	void		LoopStream(bool inBool);
	void		SetVolumeStream(unsigned short inVol);
	void		SetHalfSpeedStream(bool inHalfSpeed);

	void		MusicStart( bool loop );
	void		MusicStop( );

	void		MusicLoadXM( const char* name );
	void		MusicLoadXTI( const char* name );
	void		MusicLoadXTM( const char* name );
	
	void		MusicFreeXTI( );
	void		MusicFreeXTM( );
	void		MusicFreeXM( );
	bool		m_bIsSoundAndMusicInitialized;
	//void		LoadSound();

//private:
	
	//std::string m_sCurrentStream;
//	bool IsStreamPlaying(std::string s) { return (m_sCurrentStream==s);}

#ifdef __WIN32__
	CSSEWinDriver*		m_pSoundDriver;
#endif // __WIN32__
	int					m_SamplesIdx[NUM_SOUNDS_TOTAL];
	int					m_SamplesTime[NUM_SOUNDS_TOTAL];

	bool				m_bIsResumed; // Suspend / Resume status
	bool				m_bIsStarted; // Start / Stop status
	bool				m_bInPhoneCall;
	bool				m_bMuteWhenInCall;
	unsigned char		m_bMasterVolume;
	bool				m_bStoppedVol;	// when stopped because of master volume set to 0
};


#endif
