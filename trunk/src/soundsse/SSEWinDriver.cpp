
#include "SSEWinDriver.h"

//#include "..\M3D\HQM3DPredefine.h"

#define SSE_BUFFER_PER_SECOND	16
#define SSE_BUFFER_SIZE			(SSE_MIXING_SAMPLE_RATE / SSE_BUFFER_PER_SECOND)

//--------------------------------------------------------------------

class CSSEMixEngine*	CSSEWinDriver::SoundStreamInit( int maxSamples, void* nothing  )
{
    u32 i;

    if (m_open)
        return false;

	
	
	m_reallyStarted = 0;
	m_started = 0;

	// SOUNDENGINE
	m_soundEngine = NEW CSSEMixEngine( maxSamples );



	m_bufmixframe = SSE_BUFFER_SIZE;

    m_freqmix = SSE_MIXING_SAMPLE_RATE;
    m_bufmixtotal = m_bufmixframe*(MULTI_BUFFER_NUMBER+1);
#if SSE_MIXING_TO_16_BITS
    m_bufmix = new u16[m_bufmixtotal];
#else
    m_bufmix = new u8[m_bufmixtotal];
#endif
    if (m_bufmix == 0)
        return false;

#if SSE_MIXING_TO_16_BITS
    // 16b mixbuffer
    for (i = 0; i < m_bufmixtotal; i++)
        m_bufmix[i] = 0;
#else
    // 8b mixbuffer
    for (i = 0; i < m_bufmixtotal; i++)
        (*(u8 *)((u32)m_bufmix + i)) = 128;
#endif

    // multi buffer setup
    m_dmix = 1;
	m_pbufmix = &m_bufmix[m_dmix*m_bufmixframe];


    InitializeCriticalSection(&m_critSecRtp);

    m_thread = CreateThread(NULL, 0, ThreadUpdate, (void *)this, CREATE_SUSPENDED, &m_threadId);
    if (m_thread == NULL)
        return false;

    m_killEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_exitThread = false;

    MMRESULT res;

    m_waveformatx.wFormatTag = WAVE_FORMAT_PCM;
    m_waveformatx.nSamplesPerSec = m_freqmix;
    m_waveformatx.nChannels = 1;	// 2 for stereo
#if SSE_MIXING_TO_16_BITS
    m_waveformatx.wBitsPerSample = 16;
#else
    m_waveformatx.wBitsPerSample = 8;
#endif
    m_waveformatx.nBlockAlign = m_waveformatx.nChannels * (m_waveformatx.wBitsPerSample/8);
    m_waveformatx.nAvgBytesPerSec = m_waveformatx.nSamplesPerSec * m_waveformatx.nChannels;
    m_waveformatx.cbSize = 0; // no extra information

    for (i = 0; i < waveOutGetNumDevs(); i++)  
	{
        res = waveOutOpen(&m_hwaveout, i, &m_waveformatx, m_threadId, (DWORD)this, CALLBACK_THREAD);

        if (res == MMSYSERR_NOERROR)
            break;
    }
    if (res != MMSYSERR_NOERROR) 
	{
        CloseHandle(m_killEvent);
        CloseHandle(m_thread);
        delete [] m_bufmix;
		m_bufmix = NULL; // rax
        return false;
    }
    

    waveOutPause(m_hwaveout);

    for (i = 0; i < MULTI_BUFFER_NUMBER; i++) 
	{
        ZeroMemory(&m_wavehdr[i], sizeof(WAVEHDR));
        m_wavehdr[i].dwBufferLength = 8;
        m_wavehdr[i].lpData = (char *)m_bufmix;

        waveOutPrepareHeader(m_hwaveout, &m_wavehdr[i], sizeof(WAVEHDR));
        waveOutWrite(m_hwaveout, &m_wavehdr[i], sizeof(WAVEHDR));
    }


    SetThreadPriority(m_thread, THREAD_PRIORITY_HIGHEST);
	Sleep(100);
    ResumeThread(m_thread);

    waveOutRestart(m_hwaveout);

    m_open = true;

    return m_soundEngine;
}

//--------------------------------------------------------------------

void	CSSEWinDriver::SoundStreamDeInit() 
{
    if (m_open) 
	{
        m_exitThread = true;

        WaitForSingleObject(m_killEvent, 5000);

        CloseHandle(m_killEvent);
        CloseHandle(m_thread);

        DeleteCriticalSection(&m_critSecRtp);

	    waveOutReset(m_hwaveout);

	    for (u32 i = 0; i < MULTI_BUFFER_NUMBER; i++)
		    waveOutUnprepareHeader(m_hwaveout, &m_wavehdr[i], sizeof(WAVEHDR));

        waveOutClose(m_hwaveout);

        if (m_bufmix)
            delete [] m_bufmix;

		m_bufmix = NULL;
        m_open = false;
    }
}

//--------------------------------------------------------------------

DWORD WINAPI CSSEWinDriver::ThreadUpdate(LPVOID lpParameter) 
{
    CSSEWinDriver	*snd;
    MSG				msg;
    WAVEHDR			*pwavehdr;

    snd = (CSSEWinDriver*)lpParameter;
	if (snd == NULL)
	    return -1;

	while ((WaitForSingleObject(snd->m_killEvent, 5) != 0) && (!snd->m_exitThread)) 
	{
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) 
		{
            if (msg.message == MM_WOM_DONE) 
			{  
                pwavehdr = (WAVEHDR *)msg.lParam;
			    if (pwavehdr == NULL)
				    break;

                waveOutUnprepareHeader(snd->m_hwaveout, pwavehdr, sizeof(WAVEHDR));

                ZeroMemory(pwavehdr, sizeof(WAVEHDR));
#if SSE_MIXING_TO_16_BITS
                pwavehdr->dwBufferLength = snd->m_bufmixframe*2;
#else
                pwavehdr->dwBufferLength = snd->m_bufmixframe;
#endif
                pwavehdr->lpData = (char *)snd->m_pbufmix;

                waveOutPrepareHeader(snd->m_hwaveout, pwavehdr, sizeof(WAVEHDR));

                waveOutWrite(snd->m_hwaveout, pwavehdr, sizeof(WAVEHDR));


                snd->m_dmix++;
                if (snd->m_dmix >= (MULTI_BUFFER_NUMBER+1))
                    snd->m_dmix = 0;

                snd->m_pbufmix = &snd->m_bufmix[snd->m_dmix*snd->m_bufmixframe];

//                EnterCriticalSection(&snd->m_critSecRtp);
  
				// MIX 
				snd->SoundStreamSetReallyStarted();

#if SSE_MIXING_TO_16_BITS
				snd->GetSoundEngine()->Mix( snd->m_pbufmix, snd->m_bufmixframe*2 );
#else
				snd->GetSoundEngine()->Mix( snd->m_pbufmix, snd->m_bufmixframe );
#endif

//                LeaveCriticalSection(&snd->m_critSecRtp);

            }
	   }
    }

    SetEvent(snd->m_killEvent);
    return 0;
}

//--------------------------------------------------------------------

void CSSEWinDriver::SoundStreamSuspend() 
{
    if (!m_open)
        return;

    waveOutPause(m_hwaveout);
    SuspendThread(m_thread);
}

//--------------------------------------------------------------------

void CSSEWinDriver::SoundStreamResume() 
{
    if (!m_open)
        return;

    ResumeThread(m_thread);
    waveOutRestart(m_hwaveout);
}


