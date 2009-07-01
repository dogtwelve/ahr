#include "SSEdriver.h"
//#include "..\M3D\HQM3DPredefine.h"
//#include "windows.h"
#include <stdlib.h>
#include "config.h"


//-------------------------------------------------------------------

CSSESample::CSSESample()
{
	ptr = NULL;
	data = NULL;
}

//-------------------------------------------------------------------

CSSESample::~CSSESample()
{
	if (ptr)
		SSE_FREE( ptr );
}

//-------------------------------------------------------------------

u8	CSSESample::InitSSEFromMem( void* _ptr )
{
	SSE_ASSERT( ptr == NULL );
	
	ptr = _ptr;

	SSE_SAMPLE_HEADER*	p = (SSE_SAMPLE_HEADER*)ptr;

	is16Bits = p->is16Bits;
	sampleRate = p->sampleRate;
	
	// compute sample rate ratio from mixing sample rate
	sampleRateMixRatio = (sampleRate * (1 << SSE_SAMPLERATERATIO_SHIFT)) / SSE_MIXING_SAMPLE_RATE;

	length = p->size;// - sizeof(SSE_SAMPLE_HEADER));
	if (is16Bits)
		length /= 2;
	lengthFixed = length << SSE_POSITION_SHIFT;

	data = p + 1;

	return 1;
}

//-------------------------------------------------------------------

// header of wav file
typedef struct
{
   char		rID[4];            // 'RIFF'
   s32		rLen;
   char		wID[4];            // 'WAVE'
   char		fId[4];            // 'fmt '
   s32		pcm_header_len;   // varies...
   s16		wFormatTag;
   s16		nChannels;      // 1,2 for stereo data is (l,r) pairs
   s32		nSamplesPerSec;
   s32		nAvgBytesPerSec;
   s16		nBlockAlign;      
   s16		nBitsPerSample;
}   WAV_HDR;
   
// header of wav file
typedef struct
{
   char		dId[4];            // 'data' or 'fact'
   s32		dLen;
//   unsigned char *data;
}   CHUNK_HDR;

//-------------------------------------------------------------------

u8	CSSESample::InitWAVFromMem( void* _ptr )
{
	SSE_ASSERT( ptr == NULL );
	
	ptr = _ptr;

	WAV_HDR*	whdr = (WAV_HDR*)_ptr;
	CHUNK_HDR*	chdr = (CHUNK_HDR*)(whdr+1);
	u8*			dataptr = (u8*)(chdr+1);

	if (whdr->nBitsPerSample == 8)
	{
		// convert to signed 8 bits
		u8*		in = (u8*)dataptr;
		s8*		out = (s8*)in;

		for (int i = 0; i < chdr->dLen; i++)
		{
			*out = (*in) - 128;
			out++;
			in++;
		}
	}
	
	if (whdr->nBitsPerSample == 16)
		is16Bits = 1;
	else
		is16Bits = 0;

	sampleRate = whdr->nSamplesPerSec;
	
	// compute sample rate ratio from mixing sample rate
	sampleRateMixRatio = (sampleRate * (1 << SSE_SAMPLERATERATIO_SHIFT)) / SSE_MIXING_SAMPLE_RATE;

	length = chdr->dLen;
	if (is16Bits)
		length /= 2;
	lengthFixed = length << SSE_POSITION_SHIFT;

	data = dataptr;

	return 1;
}

//-------------------------------------------------------------------

u8	CSSESample::InitRAWFromMem( void* _ptr, u32 _length, u32 _sampleRate, bool _is16bits )
{
	SSE_ASSERT( ptr == NULL );
	
	ptr = _ptr;

	u8*			dataptr = (u8*)(ptr);

	if (_is16bits == false)
	{
		// convert to signed 8 bits
		u8*		in = (u8*)dataptr;
		s8*		out = (s8*)in;

		for (u32 i = 0; i < _length; i++)
		{
			*out = (*in) - 128;
			out++;
			in++;
		}
	}

	if (_is16bits)
		is16Bits = 1;
	else
		is16Bits = 0;

	sampleRate = _sampleRate;
	
	// compute sample rate ratio from mixing sample rate
	sampleRateMixRatio = (sampleRate * (1 << SSE_SAMPLERATERATIO_SHIFT)) / SSE_MIXING_SAMPLE_RATE;

	length = _length;
	if (is16Bits)
		length /= 2;
	lengthFixed = length << SSE_POSITION_SHIFT;

	data = dataptr;

	return 1;
}

//-------------------------------------------------------------------

CSSEMixEngine::CSSEMixEngine( u32 _maxSamples )
{
	// alloc sample array
	m_maxSamples = _maxSamples;
	m_samples = SSE_NEW CSSESample[m_maxSamples];
	// clear voice array
	u32	i;
	for (i = 0; i < SSE_MAX_VOICES; i++)
	{
		m_voices[i].id = -1;
		m_voices[i].psample = NULL;
	}

	// set start voice id
	m_curVoiceId = 0;

	// set master volume to max
	m_masterVolume = SSE_VOLUME_ONE / 2;	// set to half by default to avoid saturation


	m_stream = NULL;
}

//-------------------------------------------------------------------

CSSEMixEngine::~CSSEMixEngine()
{
	// free sample array
	SSE_DELETE_ARRAY m_samples;
}

//-------------------------------------------------------------------

void	CSSEMixEngine::SetMasterVolume( u16 volume )
{
	m_masterVolume = volume;
}

//-------------------------------------------------------------------

#if SSE_USE_FX_INTERPOLATION

void	CSSEMixEngine::Mix( void* _buffer, u32 _bufferSize )
{
	// count the number of current active m_voices
	u32	activeNb = 0;
	u32	i;
	for (i = 0; i < SSE_MAX_VOICES; i++)
	{
		if (m_voices[i].psample)
			activeNb++;
	}
	// count the stream in active voices
	if (m_stream && m_streamVolume)
		activeNb++;

	if (activeNb == 0)
	{
		// No active m_voices ! memset to 0, and return :)
#if SSE_MIXING_TO_16_BITS
	#if SSE_CONVERT_16B_TO_UNSIGNED == 0
		SSE_MEMSET( _buffer, 0, _bufferSize );
	#else
		// empty should be 0x8000 for each 16bit value, but 0x8080 will do it (the memset uses only the lower 8bits value to fill)
		SSE_MEMSET( _buffer, 0x80808080, _bufferSize );
	#endif
#else
	#if SSE_CONVERT_8B_TO_UNSIGNED == 0
		SSE_MEMSET( _buffer, 0, _bufferSize );
	#else
		// empty should be 0x8000 for each 16bit value, but 0x8080 will do it (the memset uses only the lower 8bits value to fill)
		SSE_MEMSET( _buffer, 0x80808080, _bufferSize );
	#endif
#endif

		return;
	}

#if SSE_MIXING_TO_16_BITS
	// mixing to 16 bits

	// check buffer size, we use interpolation, so it needs to be a multiple of interpol steps
	SSE_ASSERT( ((_bufferSize>>1) & (SSE_INTERPOLATION_STEPS-1)) == 0 );

	s16*	pout = (s16*)_buffer;

	for (s32 inter = 0; inter < SSE_INTERPOLATION_STEPS; inter++)
	{
		u32		n = (_bufferSize >> 1) >> SSE_INTERPOLATION_STEPS_SHIFT;

		while (n--)
		{
			register s32		out = 0;

			i = SSE_MAX_VOICES;
			SSEVoice*	voice = m_voices;
			while (i--)
			{
				CSSESample*	sample = voice->psample;
				if (sample)
				{
					// add

					if (sample->is16Bits)
					{
						// 16 bits sample
						out += (((s16*)(sample->data))[voice->positionFixed >> SSE_POSITION_SHIFT] * (voice->volume2Fixed >> SSE_INTERPOLATION_VOLUME_FIXED_SHIFT));
					}
					else
					{
						// 8 bits sample
						out += ((((s8*)(sample->data))[voice->positionFixed >> SSE_POSITION_SHIFT] * (voice->volume2Fixed >> SSE_INTERPOLATION_VOLUME_FIXED_SHIFT)) << 8);
					}

					// update position
					voice->positionFixed += voice->positionFixedInc2;

					// end ?
					if (voice->positionFixed >= sample->lengthFixed)
					{
						if (voice->isLooping)
						{
							// looping sample go to start
							voice->positionFixed = 0;	
						}
						else
						{
							// remove sample from voice
							voice->psample = NULL;
							voice->id = -1;
						}
					}
				}

				voice++;
			}

			// stream handling
			if (m_stream && m_streamVolume)
			{
				out += m_stream->m_buffer[m_streamPositionFixed >> SSE_POSITION_SHIFT] * m_streamVolume;
				m_streamPositionFixed += m_streamPositionFixedInc;
				if (m_streamPositionFixed >= m_streamLengthFixed)
				{
					if (m_stream->ReadData( m_stream->m_bufferSize ) == 0)
					{
						// end of non looping stream -> remove it
						m_stream = NULL;
					}
					m_streamPositionFixed = 0;
				}
			}

			// remove volume shift
			out >>= (SSE_VOLUME_SHIFT); //+ SSE_MAX_VOICES_SHIFT
			// clip out
			if (out > 32767)
				out = 32767;
			if (out < -32768)
				out = -32768;
#if SSE_CONVERT_16B_TO_UNSIGNED == 0
			*pout++ = out;
#else
			*pout++ = 32768 + out;		// conv to unsigned
#endif
		}

		// update interpolation
		i = SSE_MAX_VOICES;
		SSEVoice*	voice = m_voices;
		while (i--)
		{
			CSSESample*	sample = voice->psample;
			if (sample)
			{
				if (voice->interpolateVolume)
					voice->volume2Fixed += voice->volumeFixedStep;
				if (voice->interpolatePitch)
					voice->positionFixedInc2 += voice->positionFixedIncStep;
			}

			voice++;
		}

	}
#else
	// mixing to 8 bits

	// check buffer size, we use interpolation, so it needs to be a multiple of interpol steps
	SSE_ASSERT( (_bufferSize & (SSE_INTERPOLATION_STEPS-1)) == 0 );

	s8*		pout = (s8*)_buffer;

	for (s32 inter = 0; inter < SSE_INTERPOLATION_STEPS; inter++)
	{
		u32		n = _bufferSize >> SSE_INTERPOLATION_STEPS_SHIFT;
		while (n--)
		{
			register s32		out = 0;

			i = SSE_MAX_VOICES;
			SSEVoice*	voice = m_voices;
			while (i--)
			{
				CSSESample*	sample = voice->psample;
				if (sample)
				{
					// add

					if (sample->is16Bits)
					{
						// 16 bits sample
						out += (((s16*)(sample->data))[voice->positionFixed >> SSE_POSITION_SHIFT] * (voice->volume2Fixed >> SSE_INTERPOLATION_VOLUME_FIXED_SHIFT)) / 256;
					}
					else
					{
						// 8 bits sample
						out += (((s8*)(sample->data))[voice->positionFixed >> SSE_POSITION_SHIFT] * (voice->volume2Fixed >> SSE_INTERPOLATION_VOLUME_FIXED_SHIFT));
					}

					// update position
					voice->positionFixed += voice->positionFixedInc2;

					// end ?
					if (voice->positionFixed >= sample->lengthFixed)
					{
						if (voice->isLooping)
						{
							// looping sample go to start
							voice->positionFixed = 0;	
						}
						else
						{
							// remove sample from voice
							voice->psample = NULL;
							voice->id = -1;
						}
					}
				}

				voice++;
			}

			// stream handling
			if (m_stream && m_streamVolume)
			{
				out += (m_stream->m_buffer[m_streamPositionFixed >> SSE_POSITION_SHIFT] * m_streamVolume) / 256;
				m_streamPositionFixed += m_streamPositionFixedInc;
				if (m_streamPositionFixed >= m_streamLengthFixed)
				{
					if (m_stream->ReadData( m_stream->m_bufferSize ) == 0)
					{
						// end of non looping stream -> remove it
						m_stream = NULL;
					}
					m_streamPositionFixed = 0;
				}
			}
			
			// remove volume shift
			out >>= (SSE_VOLUME_SHIFT);//+ SSE_MAX_VOICES_SHIFT
			// clip out
			if (out > 127)
				out = 127;
			if (out < -128)
				out = -128;
#if SSE_CONVERT_8B_TO_UNSIGNED == 0
			*pout++ = out;
#else
			*pout++ = 128 + out;	// conv to unsigned
#endif
		}

		// update interpolation
		i = SSE_MAX_VOICES;
		SSEVoice*	voice = m_voices;
		while (i--)
		{
			CSSESample*	sample = voice->psample;
			if (sample)
			{
				if (voice->interpolateVolume)
					voice->volume2Fixed += voice->volumeFixedStep;
				if (voice->interpolatePitch)
					voice->positionFixedInc2 += voice->positionFixedIncStep;
			}

			voice++;
		}

	}
#endif

}

//-------------------------------------------------------------------

#else	// USE_INTERPOLATION

void	CSSEMixEngine::Mix( void* _buffer, u32 _bufferSize )
{
	// count the number of current active m_voices
	u32	activeNb = 0;
	u32	i;
	for (i = 0; i < SSE_MAX_VOICES; i++)
	{
		if (m_voices[i].psample)
			activeNb++;
	}
	// count the stream in active voices
	if (m_stream && m_streamVolume)
		activeNb++;

	if (activeNb == 0)
	{
		// No active m_voices ! memset to 0, and return :)
#if SSE_MIXING_TO_16_BITS
	#if SSE_CONVERT_16B_TO_UNSIGNED == 0
		SSE_MEMSET( _buffer, 0, _bufferSize );
	#else
		// empty should be 0x8000 for each 16bit value, but 0x8080 will do it (the memset uses only the lower 8bits value to fill)
		SSE_MEMSET( _buffer, 0x80808080, _bufferSize );
	#endif
#else
	#if SSE_CONVERT_8B_TO_UNSIGNED == 0
		SSE_MEMSET( _buffer, 0, _bufferSize );
	#else
		// empty should be 0x8000 for each 16bit value, but 0x8080 will do it (the memset uses only the lower 8bits value to fill)
		SSE_MEMSET( _buffer, 0x80808080, _bufferSize );
	#endif
#endif
		return;
	}

#if SSE_MIXING_TO_16_BITS
	// mixing to 16 bits
	u32		n = _bufferSize >> 1;
	s16*	pout = (s16*)_buffer;

	while (n--)
	{
		register s32		out = 0;

		i = SSE_MAX_VOICES;
		SSEVoice*	voice = m_voices;
		while (i--)
		{
			CSSESample*	sample = voice->psample;
			if (sample)
			{
				// add

				if (sample->is16Bits)
				{
					// 16 bits sample
#if SSE_USE_SAMPLE_INTERPOLATION == 0
					out += (((s16*)(sample->data))[voice->positionFixed >> SSE_POSITION_SHIFT] * voice->volume);
#else
					const s32	pos = voice->positionFixed;
					const s32	fac = pos & SSE_POSITION_DECIMAL_MASK;
					u32			posnext = (pos >> SSE_POSITION_SHIFT) + 1;
					if (posnext >= (sample->lengthFixed >> SSE_POSITION_SHIFT))
						posnext = 0;
//					const s32	tmp =	(((s16*)(sample->data))[posnext] * fac) + 
//										(((s16*)(sample->data))[pos >> SSE_POSITION_SHIFT] * ((1<<SSE_POSITION_SHIFT) - fac));
//					out += (tmp >> SSE_POSITION_SHIFT) * voice->volume;
					const s16	v1 = ((s16*)(sample->data))[posnext];
					const s16	v0 = ((s16*)(sample->data))[pos >> SSE_POSITION_SHIFT];
					out += (v0 + (((v1 - v0) * fac) >> SSE_POSITION_SHIFT)) * voice->volume;
#endif
				}
				else
				{
					// 8 bits sample
#if SSE_USE_SAMPLE_INTERPOLATION == 0
					out += ((((s8*)(sample->data))[voice->positionFixed >> SSE_POSITON_SHIFT] * voice->volume) << 8);
#else
					const u32	pos = voice->positionFixed;
					const s32	fac = pos & SSE_POSITION_DECIMAL_MASK;
					u32			posnext = (pos >> SSE_POSITION_SHIFT) + 1;
					if (posnext >= (sample->lengthFixed >> SSE_POSITION_SHIFT))
						posnext = 0;
//					const s32	tmp =	(((s8*)(sample->data))[posnext] * fac) + 
//										(((s8*)(sample->data))[pos >> SSE_POSITION_SHIFT] * ((1<<SSE_POSITION_SHIFT) - fac));
//					out += (tmp >> (SSE_POSITION_SHIFT-8)) * voice->volume;
					const s8	v1 = ((s8*)(sample->data))[posnext];
					const s8	v0 = ((s8*)(sample->data))[pos >> SSE_POSITION_SHIFT];
					out += ((v0 + (((v1 - v0) * fac) >> SSE_POSITION_SHIFT)) * voice->volume) << 8;
#endif
				}

				// update position
				voice->positionFixed += voice->positionFixedInc;

				// end ?
				if (voice->positionFixed >= sample->lengthFixed)
				{
					if (voice->isLooping)
					{
						// looping sample go to start
						voice->positionFixed = 0;	
					}
					else
					{
						// remove sample from voice
						voice->psample = NULL;
						voice->id = -1;
					}
				}
			}

			voice++;
		}

		// stream handling
		if (m_stream && m_streamVolume)
		{
			out += m_stream->m_buffer[m_streamPositionFixed >> SSE_POSITION_SHIFT] * m_streamVolume;
			m_streamPositionFixed += m_streamPositionFixedInc;
			if (m_streamPositionFixed >= m_streamLengthFixed)
			{
				if (m_stream->ReadData( m_stream->m_bufferSize ) == 0)
				{
					// end of non looping stream -> remove it
					m_stream = NULL;
				}
				m_streamPositionFixed = 0;
			}
		}


		// remove volume shift
		out >>= (SSE_VOLUME_SHIFT); //+ SSE_MAX_VOICES_SHIFT
		// clip out
		if (out > 32767)
			out = 32767;
		if (out < -32768)
			out = -32768;
	#if SSE_CONVERT_16B_TO_UNSIGNED == 0
		*pout++ = out;
	#else
		*pout++ = 32768 + out;
	#endif
	}
#else
	// mixing to 8 bits
	u32		n = _bufferSize;
	s8*		pout = (s8*)_buffer;

	while (n--)
	{
		register s32		out = 0;

		i = SSE_MAX_VOICES;
		SSEVoice*	voice = m_voices;
		while (i--)
		{
			CSSESample*	sample = voice->psample;
			if (sample)
			{
				// add

				if (sample->is16Bits)
				{
					// 16 bits sample
#if SSE_USE_SAMPLE_INTERPOLATION == 0
					out += (((s16*)(sample->data))[voice->positionFixed >> SSE_POSITION_SHIFT] * voice->volume) / 256;
#else
					const s32	pos = voice->positionFixed;
					const s32	fac = pos & SSE_POSITION_DECIMAL_MASK;
					u32			posnext = (pos >> SSE_POSITION_SHIFT) + 1;
					if (posnext >= (sample->lengthFixed >> SSE_POSITION_SHIFT))
						posnext = 0;
//					const s32	tmp =	(((s16*)(sample->data))[posnext] * fac) + 
//										(((s16*)(sample->data))[pos >> SSE_POSITION_SHIFT] * ((1<<SSE_POSITION_SHIFT) - fac));
//					out += ((tmp >> SSE_POSITION_SHIFT) * voice->volume) / 256;
					const s16	v1 = ((s16*)(sample->data))[posnext];
					const s16	v0 = ((s16*)(sample->data))[pos >> SSE_POSITION_SHIFT];
					out += ((v0 + (((v1 - v0) * fac) >> SSE_POSITION_SHIFT)) * voice->volume) / 256;
#endif
				}
				else
				{
					// 8 bits sample
#if SSE_USE_SAMPLE_INTERPOLATION == 0
					out += (((s8*)(sample->data))[voice->positionFixed >> SSE_POSITION_SHIFT] * voice->volume);
#else
					const s32	pos = voice->positionFixed;
					const s32	fac = pos & SSE_POSITION_DECIMAL_MASK;
					u32			posnext = (pos >> SSE_POSITION_SHIFT) + 1;
					if (posnext >= (sample->lengthFixed >> SSE_POSITION_SHIFT))
						posnext = 0;
//					const s32	tmp =	(((s8*)(sample->data))[posnext] * fac) + 
//										(((s8*)(sample->data))[pos >> SSE_POSITION_SHIFT] * ((1<<SSE_POSITION_SHIFT) - fac));
//					out += (tmp >> (SSE_POSITION_SHIFT)) * voice->volume;
					const s8	v1 = ((s8*)(sample->data))[posnext];
					const s8	v0 = ((s8*)(sample->data))[pos >> SSE_POSITION_SHIFT];
					out += ((v0 + (((v1 - v0) * fac) >> SSE_POSITION_SHIFT)) * voice->volume);
#endif
				}

				// update position
				voice->positionFixed += voice->positionFixedInc;

				// end ?
				if (voice->positionFixed >= sample->lengthFixed)
				{
					if (voice->isLooping)
					{
						// looping sample go to start
						voice->positionFixed = 0;	
					}
					else
					{
						// remove sample from voice
						voice->psample = NULL;
						voice->id = -1;
					}
				}
			}

			voice++;
		}

		// stream handling
		if (m_stream && m_streamVolume)
		{
			out += (m_stream->m_buffer[m_streamPositionFixed >> SSE_POSITION_SHIFT] * m_streamVolume) / 256;
			m_streamPositionFixed += m_streamPositionFixedInc;
			if (m_streamPositionFixed >= m_streamLengthFixed)
			{
				if (m_stream->ReadData( m_stream->m_bufferSize ) == 0)
				{
					// end of non looping stream -> remove it
					m_stream = NULL;
				}
				m_streamPositionFixed = 0;
			}
		}

		// remove volume shift
		out >>= (SSE_VOLUME_SHIFT);//+ SSE_MAX_VOICES_SHIFT
		// clip out
		if (out > 127)
			out = 127;
		if (out < -128)
			out = -128;
	#if SSE_CONVERT_8B_TO_UNSIGNED == 0
		*pout++ = out;
	#else
		*pout++ = 128 + out;
	#endif
	}
#endif

}
#endif// USE_INTERPOLATION

//-------------------------------------------------------------------

u8		CSSEMixEngine::NewSampleFromMemSSE( u32 _sampleIdx, void* _ptr )
{
	SSE_ASSERT( _sampleIdx >= 0 );
	SSE_ASSERT( _sampleIdx < m_maxSamples );

	return (m_samples[_sampleIdx].InitSSEFromMem( _ptr ));
}

//-------------------------------------------------------------------

u8		CSSEMixEngine::NewSampleFromMemWAV( int _sampleIdx, void* _ptr )
{
//	SSE_ASSERT( _sampleIdx >= 0 );
	//SSE_ASSERT( _sampleIdx < m_maxSamples );

	return (m_samples[_sampleIdx].InitWAVFromMem( _ptr ));
}

//-------------------------------------------------------------------

u8		CSSEMixEngine::NewSampleFromMemRAW( u32 _sampleIdx, void* _ptr, u32 _length, u32 _frequency, bool _is16bits )
{
	SSE_ASSERT( _sampleIdx >= 0 );
	SSE_ASSERT( _sampleIdx < m_maxSamples );

	return (m_samples[_sampleIdx].InitRAWFromMem( _ptr, _length, _frequency, _is16bits ));
}

//-------------------------------------------------------------------

void	CSSEMixEngine::DeleteSample( u32 sampleIdx )
{
	SSE_DELETE_ARRAY m_samples[sampleIdx].ptr;

	m_samples[sampleIdx].ptr = NULL;
	m_samples[sampleIdx].data = NULL;
}

//-------------------------------------------------------------------

void	CSSEMixEngine::DeleteAllSamples()
{
	for (u32 i = 0; i < m_maxSamples; i++)
	{
		if (m_samples[i].ptr)
			DeleteSample( i );
	}
}

//-------------------------------------------------------------------

u32		CSSEMixEngine::PlaySample( u32 _sampleIdx, u16 _volume, u32 _pitch, u8 _loop, u8 _priority )
{
	SSE_ASSERT( _sampleIdx < m_maxSamples );
	SSE_ASSERT( m_samples[_sampleIdx].ptr );

	// find a free voice
	u32	i;
	for (i = 0; i < SSE_MAX_VOICES; i++)
	{
		if (m_voices[i].psample == NULL)
			break;
	}
	
	if (i == SSE_MAX_VOICES) // no voice free ?
	{
		// look for a non looping sample of lower priority
		for (i = 0; i < SSE_MAX_VOICES; i++)
		{
			if ((m_voices[i].priority < _priority) && (m_voices[i].isLooping == 0))
				break;
		}
		if (i == SSE_MAX_VOICES)	// still not found ? 
			return (0);	// id 0 == no voice
	}

	// add this sample 
	SSEVoice* voice = m_voices + i;
	voice->psample = m_samples + _sampleIdx;
	voice->positionFixed = 0;
	voice->volume = (_volume * m_masterVolume) >> 8;	// multiply by master volume
	voice->priority = _priority;
	voice->pitch = _pitch;
	voice->isLooping = _loop;


	// compute position increment
	voice->positionFixedInc = (_pitch * voice->psample->sampleRateMixRatio) >> SSE_SAMPLERATERATIO_SHIFT;


	// interpolation stuff
#if SSE_USE_FX_INTERPOLATION
	voice->interpolatePitch = 0;
	voice->interpolateVolume = 0;
	voice->positionFixedInc2 = voice->positionFixedInc;
	voice->volume2Fixed = voice->volume << SSE_INTERPOLATION_VOLUME_FIXED_SHIFT;
#endif

	m_curVoiceId++;	// inc our unique id

	voice->id = m_curVoiceId;

	return m_curVoiceId;
}

//-------------------------------------------------------------------

void	CSSEMixEngine::StopAllVoices()
{
	u32	i;
	for (i = 0; i < SSE_MAX_VOICES; i++)
		m_voices[i].psample = NULL;

}

//-------------------------------------------------------------------

void	CSSEMixEngine::StopVoice( u32 _voiceId )
{
	u32	i;
	for (i = 0; i < SSE_MAX_VOICES; i++)
	{
		if (m_voices[i].id == _voiceId)
		{
			m_voices[i].id = -1;
			m_voices[i].psample = NULL;
			return;
		}
	}
}

//-------------------------------------------------------------------

u32		CSSEMixEngine::IsVoicePlaying( u32 _voiceId )
{
	u32	i;
	for (i = 0; i < SSE_MAX_VOICES; i++)
	{
		if (m_voices[i].id == _voiceId)
		{
			return 1;
		}
	}
	return 0;
}

//-------------------------------------------------------------------

void		CSSEMixEngine::SetVoiceVolume( u32 _voiceId, u16 _volume, u16 interpolate  )
{
	s32			i = SSE_MAX_VOICES;
	SSEVoice*	voc = m_voices;
	while (i--)
	{
		if (voc->id == _voiceId)
		{
#if SSE_USE_FX_INTERPOLATION
			if (interpolate)
			{
				voc->interpolateVolume = 1;
				//voc->volume2Fixed = voc->volume << SSE_INTERPOLATION_VOLUME_FIXED_SHIFT; NOT NEEDED !
				voc->volume = (_volume * m_masterVolume) >> SSE_INTERPOLATION_VOLUME_FIXED_SHIFT;	// multiply by master volume
				// compute interpolation steps
				voc->volumeFixedStep = ((voc->volume - (voc->volume2Fixed>>SSE_INTERPOLATION_VOLUME_FIXED_SHIFT)) << SSE_INTERPOLATION_VOLUME_FIXED_SHIFT) / SSE_INTERPOLATION_STEPS;
			}
			else
#endif
			{
				voc->volume = (_volume * m_masterVolume) >> SSE_VOLUME_SHIFT;	// multiply by master volume
#if SSE_USE_FX_INTERPOLATION
				voc->volume2Fixed = voc->volume << SSE_INTERPOLATION_VOLUME_FIXED_SHIFT;
#endif
			}

			return;
		}

		voc++;
	}
}

//-------------------------------------------------------------------

void		CSSEMixEngine::SetVoicePitch( u32 _voiceId, u32 _pitch, u16 interpolate )
{
	s32			i = SSE_MAX_VOICES;
	SSEVoice*	voc = m_voices;
	while (i--)
	{
		if (voc->id == _voiceId)
		{
			voc->pitch = _pitch;	// information not needed in fact !

#if SSE_USE_FX_INTERPOLATION
			if (interpolate)
			{
				voc->interpolatePitch = 1;
				//voc->positionFixedInc2 = voc->positionFixedInc;// NOT NEEDED !!
				// recompute position increment
				voc->positionFixedInc = (_pitch * voc->psample->sampleRateMixRatio) >> SSE_SAMPLERATERATIO_SHIFT;

				// compute interpolation steps
				voc->positionFixedIncStep = (((s32)voc->positionFixedInc) - voc->positionFixedInc2) / SSE_INTERPOLATION_STEPS;

				//DBGPRINTF("> %d %d %d", voc->positionFixedInc2, voc->positionFixedInc, voc->positionFixedIncStep );
			}
			else
#endif
			{
				// recompute position increment
				voc->positionFixedInc = (_pitch * voc->psample->sampleRateMixRatio) >> SSE_SAMPLERATERATIO_SHIFT;
#if SSE_USE_FX_INTERPOLATION
				voc->positionFixedInc2 = voc->positionFixedInc;
#endif
			}
			return;
		}

		voc++;
	}
}

//-------------------------------------------------------------------

void	CSSEMixEngine::SetStream( CSSEStreamPCM* stream, u32 sampleRate )
{
	m_stream = stream;
	if (stream)
	{
		m_streamLengthFixed = stream->m_bufferSize << SSE_POSITION_SHIFT;
		m_streamPositionFixed = 0;
		m_streamPositionFixedInc = (sampleRate * (1 << SSE_SAMPLERATERATIO_SHIFT)) / SSE_MIXING_SAMPLE_RATE;
	}
}
