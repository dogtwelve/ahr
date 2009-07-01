#ifndef _SSESTREAM_H_
#define _SSESTREAM_H_

// 16 Bits PCM streaming 

class CSSEStreamPCM
{
public:
	CSSEStreamPCM()		{};
	
	void	Init( 	void* fileManager,
					void* (*fileOpen)( void* fileManager, char* name ),
					int	(*fileRead)( void* fileManager, void* file, void* outPtr, int length ),
					void (*fileSeekStart)( void* fileManager, void* file ),
					void (*fileClose)( void* fileManager, void* file ),
					int bufferSize );
	void	DeInit();

	int		Open( char* name, bool loop );
	void	Close();

	int		ReadData( int length );	// returns 0 when ended if not looped

	int		m_bufferSize;
	s16*	m_buffer;

private:
	bool	m_loop;
	void*	m_file;
	void*	m_fileManager;
	void*	(*m_fileOpen)( void* fileManager, char* name );
	int		(*m_fileRead)( void* fileManager, void* file, void* outPtr, int length );
	void	(*m_fileSeekStart)( void* fileManager, void* file );
	void	(*m_fileClose)( void* fileManager, void* file );

};


#endif