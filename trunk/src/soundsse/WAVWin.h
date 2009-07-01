// just a simple play wav lib

class CWinWave
{
public:
	CWinWave();

	// init from a wav file loaded in memory : NOTE THAT YOU SHOUDN'T FREE the data (they are not copied)
	void	InitFromMemory( void* ptr, int fileLength );

	// play the wav at the volume (0-255), will only play one sound at a time, and will fail if a file
	// is already playing
	int		Play( int volume );

	void*			m_data;
	int				m_length;
	WAVEFORMATEX	m_format;
	HWAVEOUT		m_hwo;
	WAVEHDR			m_whdr;
};
