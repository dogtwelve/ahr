#include <windows.h>
#include "wavwin.h"
//#include "..\M3D\HQM3DPredefine.h"
#define WF_OFFSET_FORMATTAG			20
#define WF_OFFSET_CHANNELS			22
#define WF_OFFSET_SAMPLESPERSEC		24
#define WF_OFFSET_AVGBYTESPERSEC	28
#define WF_OFFSET_BLOCKALIGN		32
#define WF_OFFSET_BITSPERSAMPLE		34
#define WF_OFFSET_DATASIZE			40
#define WF_OFFSET_DATA				44
#define WF_HEADER_SIZE WF_OFFSET_DATA


void CALLBACK	WaveOutProc( HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);


CWinWave::CWinWave()
{
	m_data = NULL;
	m_length = 0;
	m_hwo = 0;
}

// init from a wav file loaded in memory : NOTE THAT YOU SHOUDN'T FREE the data (they are not copied)
void	CWinWave::InitFromMemory( void* ptr, int fileLength )
{
	unsigned char*	aHeader = (unsigned char*)ptr;

	m_format.cbSize = 0;
	m_format.wFormatTag = *((WORD*) (aHeader + WF_OFFSET_FORMATTAG));
	m_format.nChannels = *((WORD*) (aHeader + WF_OFFSET_CHANNELS));
	m_format.nSamplesPerSec = *((DWORD*) (aHeader + WF_OFFSET_SAMPLESPERSEC));
	m_format.nAvgBytesPerSec = *((DWORD*) (aHeader + WF_OFFSET_AVGBYTESPERSEC));
	m_format.nBlockAlign = *((WORD*) (aHeader + WF_OFFSET_BLOCKALIGN));
	m_format.wBitsPerSample = *((WORD*) (aHeader + WF_OFFSET_BITSPERSAMPLE));

	m_length = fileLength - WF_HEADER_SIZE;

	m_data = aHeader + WF_HEADER_SIZE;

	// prepare header
	memset( &m_whdr, 0, sizeof(WAVEHDR) );
	m_whdr.dwBufferLength = m_length;
	m_whdr.lpData = (char*)m_data;
}

// play the wav at the volume (0-255), will only play one sound at a time, and will fail if a file
// is already playing
int		CWinWave::Play( int volume )
{
//	DEBUG("CWinWave::Play( int volume )");
	UINT		devId;
	MMRESULT	mmres;

	if (m_data == NULL)
		return -1;

	// Open audio device
	for (devId = 0; devId < waveOutGetNumDevs(); devId++) 
	{
		mmres = waveOutOpen(&m_hwo, devId, &m_format, (DWORD)WaveOutProc, (ULONG)this, CALLBACK_FUNCTION);
		if (mmres == MMSYSERR_NOERROR) 
			break;
	}
	if (mmres != MMSYSERR_NOERROR) 
		return -1;

	// set volume
	waveOutSetVolume(m_hwo, (volume << 8) | (volume << 24));

	mmres = waveOutPrepareHeader(m_hwo, &m_whdr, sizeof(WAVEHDR));	
	if (mmres != MMSYSERR_NOERROR) 
		return -1;

	mmres = waveOutWrite(m_hwo, &m_whdr, sizeof(WAVEHDR));	
	if (mmres != MMSYSERR_NOERROR)
		return -1;

	return 0;
}

void CALLBACK	WaveOutProc( HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2) 
{ 
	CWinWave*	wave = (CWinWave*)dwInstance;

	switch(uMsg) 
	{ 
		case WOM_OPEN: // connection opened 
			break; 

		case WOM_DONE: // buffer finished playing 

			waveOutUnprepareHeader(wave->m_hwo, &wave->m_whdr, sizeof(WAVEHDR));

			break; 

		case WOM_CLOSE: // connection closed 
			break; 
	} 
}

