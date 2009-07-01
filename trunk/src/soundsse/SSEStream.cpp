#include "SSEDriver.h"
#include "SSEStream.h"

//--------------------------------------------------------------

void	CSSEStreamPCM::Init( 	void* fileManager,
							void* (*fileOpen)( void* fileManager, char* name ),
							int	(*fileRead)( void* fileManager, void* file, void* outPtr, int length ),
							void (*fileSeekStart)( void* fileManager, void* file ),
							void (*fileClose)( void* fileManager, void* file ),
							int bufferSize )
{
	m_bufferSize = bufferSize;

	m_buffer = (s16*)SSE_MALLOC( bufferSize );

	m_loop = false;
	m_file = NULL;
	m_fileManager = fileManager;
	m_fileOpen = fileOpen;
	m_fileRead = fileRead;
	m_fileSeekStart = fileSeekStart;
	m_fileClose = fileClose;
}

//--------------------------------------------------------------

void	CSSEStreamPCM::DeInit()
{
	m_bufferSize = 0;
	SSE_FREE( m_buffer );
	m_buffer = 0;
}

//--------------------------------------------------------------

int		CSSEStreamPCM::Open( char* name, bool loop )
{
	m_loop = loop;

	// open file
	m_file = m_fileOpen( m_fileManager, name );
	if (m_file == 0)
		return 0;

	// intial read :)
	int	n = m_fileRead( m_fileManager, m_file, m_buffer, m_bufferSize );

	if (n != m_bufferSize)
		return 0;	// should not happen unless there's a READ error, or the file is smaller than the buffer

	return 1;
}

//--------------------------------------------------------------

void		CSSEStreamPCM::Close()
{
	if (m_file)
	{
		m_fileClose( m_fileManager, m_file );
		m_file = 0;
	}
}

//--------------------------------------------------------------

// returns 0 when ended if not looped
int		CSSEStreamPCM::ReadData( int length )
{
	SSE_ASSERT( length <= m_bufferSize );

	int	n = m_fileRead( m_fileManager, m_file, m_buffer, length );
	
	// end of the file reached ?
	if (n < length)
	{
		if (m_loop)
		{
			// go to the start of the file
			m_fileSeekStart( m_fileManager, m_file );

			// read what we need to complete the buffer
			int	n2 = m_fileRead( m_fileManager, m_file, ((unsigned char*)m_buffer) + n, length - n );
			
			if (n2 < (length-n))
			{
				// should not happen (unless the file is smaller than the buffer !)
				SSE_ASSERT( 0 );
			}
		}
		else
		{
			// fill the end of the buffer with empty sound value 
			// empty should be 0x8000 for each 16bit value, but 0x8080 will do it (the memset uses only the lower 8bits value to fill)
			SSE_MEMSET( ((unsigned char*)m_buffer) + n, 0x80808080, length - n );

			// return END of file if we didn't read anything 
			if (n == 0)
				return 0;
		}
	}

	return 1;
}
