/*
   Simple Sound Engine : MIX ENGINE by dnicolier@gameloft.com

*/

#ifndef __SSEMIXENGINE_H_
#define __SSEMIXENGINE_H_

#include "SSEStream.h"

// --------------------------------------------------------------
// MIX ENGINE CONFIGURATION

#define SSE_MIXING_SAMPLE_RATE			22050
#define SSE_MAX_VOICES_SHIFT			3			// 3 for 8 voices, 4 for 16 etc...
#define SSE_MIXING_TO_16_BITS			1			// 0 for 8bits, 1 for 16bits
#define SSE_USE_FX_INTERPOLATION		0			// to use pitch/volume interpolation (usefull if buffer is too big) (BREW)
#define SSE_USE_SAMPLE_INTERPOLATION	1			// to interpolate samples to the sample rate (greater quality)
													// doesn't work if FX_INTERPOLATION is ON

#define SSE_CONVERT_8B_TO_UNSIGNED		1		// 0 for 8bit signed sample output (BREW), 1 for 8bit unsigned (WINDOWS)
#define SSE_CONVERT_16B_TO_UNSIGNED		0		// 0 for 16bit signed sample output (BREW/WINDOWS), 1 for 16bit unsigned (?)


#if SSE_USE_FX_INTERPOLATION
	// WARNING : special care should be taken when interpolation is ON
    // the mix buffer size needs to be a multiple of SSE_INTERPOLATION_STEPS !
	#define SSE_INTERPOLATION_STEPS_SHIFT	3	// 8 steps
	#define SSE_INTERPOLATION_STEPS			(1<<SSE_INTERPOLATION_STEPS_SHIFT)
#endif

//---------------------------------------------------------------

#define SSE_MAX_VOICES				(1<<SSE_MAX_VOICES_SHIFT)

typedef struct
{
	u32		size;
	u16		sampleRate;
	u8		is16Bits;
	u8		pad;

} SSE_SAMPLE_HEADER;

class CSSESample
{
public:
	CSSESample();
	~CSSESample();

	u8		InitWAVFromMem( void* ptr );
	u8		InitSSEFromMem( void* ptr );
	u8		InitRAWFromMem( void* ptr, u32 length, u32 sampleRate, bool is16bits );

	void*	ptr;	// storage 
	void*	data;	// data (inside storage)
	u32		is16Bits;
	u32		sampleRate;
#define SSE_SAMPLERATERATIO_SHIFT		12
	u32		sampleRateMixRatio;	// ratio from the mix sample rate
	u32		length;
	u32		lengthFixed;
};


typedef struct 
{
	u32				id;

	CSSESample*		psample;	// NULL = inactive

#define SSE_POSITION_SHIFT			12
#define SSE_POSITION_DECIMAL_MASK	((1<<SSE_POSITION_SHIFT)-1)
	u32				positionFixed;		// position in fixed point 20.12
#define SSE_PITCH_ONE		(1<<SSE_POSITION_SHIFT)
	u32				pitch;				// pitch fixed point 20.12 -> 4096 = 1.0 
	u32				positionFixedInc;	// increment of position for the current pitch fixed point 20.12

#define SSE_VOLUME_SHIFT	8
#define SSE_VOLUME_ONE		(1<<SSE_VOLUME_SHIFT)
	s32				volume;		// 0-256
	u32				isLooping;	// 0 or 1
	u32				priority;	// 0-255

	// if interpolation is on, the voice will smooth interpolate from the old value to the new during MIX_BUFFER time
	// needed for platforms where the mix is not done in parallele with the game update and who need
	// a big buffer (> 1/8 sec) to output sound smoothly (BREW)
	u16				interpolatePitch;	
	u16				interpolateVolume;	

#if SSE_USE_FX_INTERPOLATION
	s32				positionFixedIncStep;
	s32				positionFixedInc2;
#define SSE_INTERPOLATION_VOLUME_FIXED_SHIFT		8		// we use a fixed point volume2 and volume2step for better interpolation precision
	s32				volumeFixedStep;
	s32				volume2Fixed;	
#endif

} SSEVoice;


class CSSEMixEngine
{
public:
	CSSEMixEngine( u32 maxSamples );
	~CSSEMixEngine();

	u8		NewSampleFromMemSSE( u32 sampleIdx, void* ptr );	// init a sample from a sample loaded in memory (DON'T FREE THIS POINTER, it will be automaticaly freed on deletesample !)
	u8		NewSampleFromMemWAV( int sampleIdx, void* ptr );	// init a sample from a sample loaded in memory (DON'T FREE THIS POINTER, it will be automaticaly freed on deletesample !)
	u8		NewSampleFromMemRAW( u32 sampleIdx, void* ptr, u32 length, u32 sampleRate, bool is16bits );	// init a sample from a sample loaded in memory (DON'T FREE THIS POINTER, it will be automaticaly freed on deletesample !)

	void	DeleteSample( u32 sampleIdx );
	void	DeleteAllSamples();

	u32		PlaySample( u32 sampleIdx, u16 volume, u32 pitch, u8 loop, u8 priority );

	void	StopAllVoices();
	void	StopVoice( u32 voiceId );
	u32		IsVoicePlaying( u32 voiceId );
	void	SetVoicePitch( u32 voiceId, u32 pitch, u16 interpolate = 0 );
	void	SetVoiceVolume( u32 voiceId, u16 volume, u16 interpolate = 0 );

	void	SetMasterVolume( u16 volume );

	void	Start();		// start sound driver + mixing
	void	Stop();			// stop sound driver + mixing
	void	Mix( void* buffer, u32 bufferSize );

	// to set a stream (set to NULL if not used)
	// should be already init & opened / won't be closed nor deinit
	void	SetStream( CSSEStreamPCM* stream, u32 sampleRate );
	void	SetStreamVolume( u16 volume )		{	m_streamVolume = volume;	}

private:

	SSEVoice		m_voices[SSE_MAX_VOICES];
	u32				m_curVoiceId;
	CSSESample*		m_samples;
	u32				m_maxSamples;
	s32				m_masterVolume;

	CSSEStreamPCM*	m_stream;					// 16 bits PCM stream
	u32				m_streamLengthFixed;
	u32				m_streamPositionFixed;		// position in fixed point 20.12
	u32				m_streamPositionFixedInc;	// increment of position for the current pitch fixed point 20.12
	s32				m_streamVolume;				// 0-256

};

#endif