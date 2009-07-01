// File.cpp: implementation of the A_IFile class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable:4786)
	
#include "config.h"

#ifdef __SYMBIAN32__
#ifdef SYMBIAN9
	#include <E32CMN.H>
	using namespace std;
#endif
	#include <e32std.h>
	#include "Application.h"
#endif
#ifdef WINDOWS_MOBILE
	#include <winbase.h>
#endif

#ifdef USE_LZMA_COMPRESSION
	#include "lzma\uncompress.h"
#endif

#include "File.h"
#ifdef __SYMBIAN32__
#include <F32FILE.H>
#endif

#ifndef ASEMANIP_PARSERONLY
#include "HighGear.h"
#endif

#include "GenDef.h"

#include "Throw.h"

#include "ArchivedFileManager.h"


#if defined(WIN32) && !(defined(__BREW__))
#ifndef WINDOWS_MOBILE
#include <direct.h>
#endif
#ifndef USE_LZMA_COMPRESSION
	#include "zlib/zlib.h"
#endif
#endif

#ifdef __BREW__
#include "ZLib/zlib.h"
#include "AEEStdLib.h"
#include "AEEFile.h"
#include "AEEShell.h"
#endif

#ifdef __SYMBIAN32__
#include <ezlib.h>
#endif

#include <string>
#include <stdio.h>

#ifdef CHECK_MEMORY_LEAKS
#ifdef __BREW__
	#define FILE_NEW(filename) new(filename, __LINE__, 0, (USE_MEMORYMANAGER_NEW*)0)
#else
	#define FILE_NEW(filename) ::new(filename, __LINE__, 0, (USE_MEMORYMANAGER_NEW*)0)
#endif
#else

#ifdef BREW_MEMORY_DEBUG
	#define FILE_NEW(filename) new(__LINE__, (char*)filename)
#else
	#define FILE_NEW(filename) STATIC_NEW
#endif
#endif

// TODO: remove temp_buff from WinMain on windowsMobile...
/*SEFU 8 MOVED
#ifdef USE_LZMA_COMPRESSION
	unsigned char temp_buff[16 * 1024];
#endif
*/

#define COMPRESSED_HEADER_SIZE	4

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

// Read operators

inline bool IsBlank(char in_Char)
{
    return (in_Char == ' ' || in_Char == '\n' || in_Char == '\b' || in_Char == '\t' || in_Char == '\r');
}

A_IFile& operator>>(A_IFile& in_File, char * out_pReadValue)
{
    // Skip blank
    char data = in_File.GetChar();
    while(IsBlank(data))
    {
        data = in_File.GetChar();
    }

    // Read the string
    int i = 0;
    while(!IsBlank(data))
    {
        out_pReadValue[i] = data;
        data = in_File.GetChar();
        ++i;
    }
    out_pReadValue[i] = '\0';

    return in_File;
}

A_IFile& operator>>(A_IFile& in_File, int& out_nReadValue)
{
    out_nReadValue = 0;

    // Skip blank
    char data = in_File.GetChar();
    while(IsBlank(data))
    {
        data = in_File.GetChar();
    }

    bool bNegate = false;
    if (data == '-')
    {
        bNegate = true;
        data = in_File.GetChar();
    }

    // Read the integer
    while(!IsBlank(data))
    {
        out_nReadValue *= 10;
        out_nReadValue += (data - '0');
        data = in_File.GetChar();
    }

    if (bNegate)
    {
        out_nReadValue = -out_nReadValue;
    }

    return in_File;
}


//-----------------------------------------------------------------------------------------------------
// ZLib allocator/deallocator

//#ifdef __BREW__



	#ifdef __cplusplus
extern "C" 
{
	#endif

		void* zcalloc( void* global, unsigned items, unsigned size ) 
		{
// TBD!			CHeap * pZLibHeap = (CHeap *)global;
//			return pZLibHeap->heapMalloc( items * size );

			BREW_HEAP*		pheap = ((BREW_HEAP*)global);
			unsigned char*	heap = pheap->ptr;

			pheap->ptr += items * size;	// inc heap
			pheap->pos += items * size;
			A_ASSERT( pheap->pos < HEAP1_SIZE );

			return heap; //MM_STATIC_MALLOC( items * size );
		}
		void zcfree( void* global, void* ptr )
		{
			// NOP. The BrewHeap deallocate all its memory during the destructor
			//MM_FREE( ptr );
		}

	#ifdef __cplusplus
}
	#endif

//#endif

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

#ifdef ASEMANIP_READERONLY

class CArchivedFile :
    public A_IFile
{
public:
    CArchivedFile(const char* pFileName, int iFlags, bool noFail);
    virtual ~CArchivedFile();

    bool IsCompressed();
    bool Exist();

    virtual unsigned int SpecificRead(void * out_pData, unsigned int in_nSize);
    virtual unsigned int SpecificWrite(const void * out_pData, unsigned int in_nSize);
	virtual bool SpecificSeek(unsigned int nOffsetFromStart);
    virtual long SpecificGetSize();
    virtual void SpecificClose();
    virtual void Flush();
    virtual void* GetBuffer();

	virtual void HandleReadError()
	{
		
		TRACE("CArchivedFile::HandleReadError\n");
		A_IFile::HandleReadError();
	}

protected:
	CArchivedFileManager::ArchiveData*	pArchive;
    int  iSeek;
    int  iFileOffset;
    int  iFileSize;
	bool bFileCompressed;
	unsigned char* pFileBuffer;
};


//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------


CArchivedFile::CArchivedFile(const char* pFileName, int iFlags, bool noFail)
	:A_IFile(noFail)
{
//SEFU 8
#ifdef USE_LZMA_COMPRESSION
	unsigned char *temp_buff = new unsigned char[16 * 1024];
#endif
	pArchive = NULL;
	CArchivedFileManager*	arcMan = CArchivedFileManager::GetInstance();

    iFileOffset		= arcMan->GetFileIndex(pFileName);
	iSeek			= iFileOffset;
    iFileSize		= arcMan->GetFileSize(pFileName);
	bFileCompressed = arcMan->GetFileCompression(pFileName);

	pFileBuffer = NULL;

	if (arcMan->CurrentArchive == NULL)
	{
		if (noFail)
		{
			A_ASSERT( arcMan->CurrentArchive );
		}
		else
		{
			delete[] temp_buff;
			return;
		}
	}

	/// Open current archive
	pArchive = arcMan->OpenCurrentArchive();

	if ( pArchive == NULL || pArchive->pDataFile == NULL)
	{
		if (noFail)
		{
			A_ASSERT( pArchive->pDataFile );
		}
		else
		{
			delete[] temp_buff;
			return;
		}
	}

    // If the file wasn't found!
    
    if(noFail && (iFileOffset == -1 || iFileSize == -1))
    {
        //LA:Removed DEBUG_PRINTF_1P("File not found1:%s\n",pFileName);  
        A_ASSERT(false);
		debug_out("ReadError:1:%s",pFileName);
        HandleReadError();
    }

	if (iFileOffset == -1 || iFileSize == -1)
    {
		delete[] temp_buff;
        return;
    }

	//uncompress the file and put the data in a buffer
	if (bFileCompressed)
	{
		unsigned int uiRealFileSize;
		unsigned long ulUncompressSize;

		unsigned char ucSize[COMPRESSED_HEADER_SIZE];

#ifdef USE_FILE_HEAP
		A_ASSERT( iFileSize <= HEAP2_SIZE );
		unsigned char* pBuffer = arcMan->m_heap2;
#else
		unsigned char* pBuffer = NEW unsigned char[iFileSize];
#endif


		//read the uncompressed size of the file
		if (!pArchive->pDataFile->Seek(iSeek + pArchive->iDataFileOffset))
        {
           //LA:Removed DEBUG_PRINTF_1P("File not found2:%s\n",pFileName);        
			debug_out("ReadError:2:%s",pFileName);
            HandleReadError();
        }

		if (!pArchive->pDataFile->Read(ucSize, COMPRESSED_HEADER_SIZE))
        {
            //LA:Removed DEBUG_PRINTF_1P("File not found3:%s\n",pFileName);        
			debug_out("ReadError:3:%s",pFileName);
            HandleReadError();
        }

		//retreive the file size
		uiRealFileSize  = ((unsigned char)(*(ucSize )))      << 24;
		uiRealFileSize |= ((unsigned char)(*(ucSize  + 1)))  << 16;
		uiRealFileSize |= ((unsigned char)(*(ucSize  + 2)))  <<  8;
		uiRealFileSize |= ((unsigned char)(*(ucSize  + 3)));

		//buffer fot the uncompressed file
#ifdef USE_FILE_HEAP
		A_ASSERT( uiRealFileSize <= HEAP3_SIZE );
		pFileBuffer = arcMan->m_heap3;
#else
		pFileBuffer = NEW unsigned char[uiRealFileSize];
#endif


		//read the compressed data
		if (!pArchive->pDataFile->Seek(iSeek+COMPRESSED_HEADER_SIZE + pArchive->iDataFileOffset))
        {
             //LA:Removed DEBUG_PRINTF_1P("File not found4:%s\n",pFileName);        
			debug_out("ReadError:4:%s",pFileName);
            HandleReadError();
        }

		if (!pArchive->pDataFile->Read(pBuffer, iFileSize))
        {
        //LA:Removed DEBUG_PRINTF_1P("File not found5:%s\n",pFileName);        
            HandleReadError();
        }
	


#if defined __BREW__ || defined __WIN32__
		//uncompress the data
		ulUncompressSize = uiRealFileSize;

		BREW_HEAP		pheap;
		pheap.ptr = arcMan->m_heap1;
		pheap.pos = 0;

#ifndef USE_LZMA_COMPRESSION
		int err = uncompress( &pheap, pFileBuffer, &ulUncompressSize, pBuffer, iFileSize );
#else
		int err = uncompress((char *) (pFileBuffer), &ulUncompressSize, (const char *) pBuffer, (unsigned long) iFileSize, temp_buff);
#endif // USE_LZMA_COMPRESSION
#else

		//uncompress the data
		ulUncompressSize = uiRealFileSize;
#ifndef USE_LZMA_COMPRESSION
		int err = uncompress(pFileBuffer, &ulUncompressSize, pBuffer, iFileSize);
#else
		int err = uncompress((char *) (pFileBuffer), &ulUncompressSize, (const char *) pBuffer, (unsigned long) iFileSize, temp_buff);
#endif // USE_LZMA_COMPRESSION
		// TODO - handle error
#ifdef __SYMBIAN32__
        if (err != Z_OK)
        {
			/// Spyder: Fix it
//            User::Leave(KErrNoMemory);
        }
#else
		A_ASSERT( err == Z_OK );
#endif
#endif

#ifndef USE_FILE_HEAP
		DELETE_ARRAY pBuffer;
#endif

		iFileOffset     = 0;
		iSeek			= iFileOffset;
		iFileSize       = uiRealFileSize;
	}

	delete[] temp_buff;
}


//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------


CArchivedFile::~CArchivedFile()
{
    SpecificClose();
}


//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------


bool CArchivedFile::Exist()
{
    return !(iFileOffset == -1 || iFileSize == -1);
}


//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------


unsigned int CArchivedFile::SpecificWrite(const void * out_pData, unsigned int in_nSize)
{
    A_ASSERT(false);
    return 0;
}


//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------


bool CArchivedFile::SpecificSeek(unsigned int nOffsetFromStart)
{
    iSeek = iFileOffset + nOffsetFromStart;

    
    // If the seek is after the file size, stop it to the max value

    if(iSeek >= iFileOffset + iFileSize)
    {
        iSeek = iFileOffset + iFileSize - 1;
    }

    return true;
}


//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------


long CArchivedFile::SpecificGetSize()
{
    return iFileSize;
}


//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

unsigned int CArchivedFile::SpecificRead(void * vpOutBuf, unsigned int iSize)
{
    // If we request to read after the file size, ajust the size to stop at the end of the file

    if(iSeek - iFileOffset + (int)iSize > iFileSize)
    {
        return 0;
    }

    if(!bFileCompressed)
	{
		if (!pArchive->pDataFile->Seek(iSeek + pArchive->iDataFileOffset))
        {
            HandleReadError();
        }

		if (pArchive->pDataFile->Read(vpOutBuf, iSize) == 1)
		{
			iSeek += iSize;

			return 1;
		}
        else
        {
            HandleReadError();
        }
	}
	else
	{
		if(pFileBuffer != NULL)
		{
			memcpy(vpOutBuf, (void *)&pFileBuffer[iSeek], iSize);
			
			iSeek += iSize;
			
			return 1;
		}
	}

    return 0;
}


//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------


void CArchivedFile::SpecificClose()
{
    iSeek = iFileOffset;
	
	if(bFileCompressed)
	{
		if(pFileBuffer != NULL)
		{
#ifndef USE_FILE_HEAP
			DELETE_ARRAY pFileBuffer;
#endif
			pFileBuffer = NULL;
		}
	}
}


//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------


void CArchivedFile::Flush()
{
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------


bool CArchivedFile::IsCompressed()
{
	return bFileCompressed;
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

void* CArchivedFile::GetBuffer()
{
    if(bFileCompressed)
	{
        return pFileBuffer;
    }
    else
    {
        return 0;
    }
}

#endif // ASEMANIP_READERONLY

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

#if defined _WINDOWS && !defined(__BREW__)
class CWinFile :
    public A_IFile
{
public:
    CWinFile(bool noFail);
    bool Create(const char* in_pFileName, int in_nFlags);
    virtual ~CWinFile();

    virtual unsigned int SpecificRead(void * out_pData, unsigned int in_nSize);
    virtual unsigned int SpecificWrite(const void * out_pData, unsigned int in_nSize);
	virtual bool SpecificSeek(unsigned int nOffsetFromStart);
    virtual long SpecificGetSize();
    virtual void SpecificClose();
    virtual void Flush();

protected:
    FILE* m_pFile;
};
#endif

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

#ifdef __SYMBIAN32__

/// Symbian files class
class CSymbianFile  :
    public A_IFile
{
public:
    CSymbianFile(bool noFail);
    bool Create(const char* in_pFileName, int in_nFlags);
    virtual ~CSymbianFile();

    virtual unsigned int SpecificRead(void * out_pData, unsigned int in_nSize);
    virtual unsigned int SpecificWrite(const void * out_pData, unsigned int in_nSize);
	virtual bool SpecificSeek(unsigned int nOffsetFromStart);
    virtual long SpecificGetSize();
    virtual void SpecificClose();
    virtual void Flush();

protected:
    RFile m_File;
    RFs m_FileServer;
};
#endif

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------


#ifdef __BREW__

class CBrewFile :
	public A_IFile
{

public:
	CBrewFile(bool noFail);
	bool Create(const char* in_pFileName, int in_nFlags);
	virtual ~CBrewFile();

	virtual unsigned int SpecificRead(void * out_pData, unsigned int in_nSize);
	virtual unsigned int SpecificWrite(const void * out_pData, unsigned int in_nSize);
	virtual bool SpecificSeek(unsigned int nOffsetFromStart);
	virtual long SpecificGetSize();
	virtual void SpecificClose();
	virtual void Flush();

protected:
	IFile * m_pFile;
};

#endif

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

class CStreamBuffer :
    public A_IFile
{
public:
	CStreamBuffer();
    virtual ~CStreamBuffer();

    virtual unsigned int SpecificRead(void * out_pData, unsigned int in_nSize);
    virtual unsigned int SpecificWrite(const void * out_pData, unsigned int in_nSize);
	virtual bool SpecificSeek(unsigned int nOffsetFromStart);
    virtual long SpecificGetSize(){return m_nCreatedBufferSize;}
    virtual void SpecificClose(){}
    virtual void Flush(){}
    virtual void* GetBuffer(){return m_pCreatedBuffer;}
	virtual int Tell() {return 0;}// TODO; there seem to be some bugs in this class: if in_pBuffer (in ctor) is null,m_pReadPos will be null also

protected:
    unsigned char * m_pCreatedBuffer;
    unsigned int m_nCreatedBufferSize;
    const unsigned char * m_pReadPos;
};

CStreamBuffer::CStreamBuffer() :
    A_IFile(true),
    m_pCreatedBuffer(0),
    m_nCreatedBufferSize(0),
    m_pReadPos(0)
{
        m_nCreatedBufferSize = 0;
        m_pCreatedBuffer = (unsigned char*) MM_MALLOC(m_nCreatedBufferSize);
		m_pReadPos = m_pCreatedBuffer;
    }

CStreamBuffer::~CStreamBuffer()
{
    MM_FREE(m_pCreatedBuffer);
}

unsigned int CStreamBuffer::SpecificRead(void * out_pData, unsigned int in_nSize)
{
    memcpy(out_pData, m_pReadPos, in_nSize);
    m_pReadPos += in_nSize;
    return 1;
}

unsigned int CStreamBuffer::SpecificWrite(const void * out_pData, unsigned int in_nSize)
{
	// Always write at end of stream
	unsigned int nAppendOffset = m_nCreatedBufferSize;
    m_nCreatedBufferSize += in_nSize;
    m_pCreatedBuffer = (unsigned char*) MM_REALLOC(m_pCreatedBuffer, m_nCreatedBufferSize);
    memcpy(m_pCreatedBuffer + nAppendOffset, out_pData, in_nSize);
    return 1;
}

bool CStreamBuffer::SpecificSeek(unsigned int nOffsetFromStart)
{
	// Can seek only on stream that were created (opened in write)
	A_ASSERT(m_pCreatedBuffer);
	if (nOffsetFromStart < 0) nOffsetFromStart = 0;
	if (nOffsetFromStart > m_nCreatedBufferSize - 1) nOffsetFromStart = m_nCreatedBufferSize - 1;
	m_pReadPos = m_pCreatedBuffer + nOffsetFromStart;
	return true;
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

A_IFile::~A_IFile()
{
	if (m_pCache)
		MM_DELETE m_pCache;
    m_pCache = 0;
	if (m_pWriteCache)
		DELETE_ARRAY m_pWriteCache;
	m_pWriteCache=0;
}


// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
/*A_IFile* A_IFile::OpenECP(ECPFileId fileId)
{	
	return CArchivedFileManager::GetInstance()->OpenECPFile(fileId);
}*/






//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

//Closes the last buffer opened by A_IFile::Open
void A_IFile::CloseLastBuffer()
{
/*	A_IFile	*	iLastFile	=	(A_IFile*)*((TInt32*)Dll::Tls());

	if(iLastFile)
		Close(iLastFile);*/
}

A_IFile* A_IFile::Open(const char* in_pFilePath, int in_nFlags, bool nofail, bool in_bDataPath)
{
//	FUNCTIONTRACKER( TRACE_GRP1, A_IFile::Open );
//	TRACEA(( TRACE_GRP1, "file name: %s %d", in_pFilePath, in_nFlags ));
#ifdef ASEMANIP_READERONLY
    if( !(in_nFlags & OPEN_STANDALONE) )
    {
#if (defined(_WINDOWS) || defined(__WINS__)) && !defined __BREW__
        // First try without using archive
    	if( in_pFilePath[0] == 'u' ) // for user.dat 
   		{
	        A_IFile* pStandAloneFile = A_IFile::Open(in_pFilePath, in_nFlags | OPEN_STANDALONE, false, in_bDataPath);
	        if (pStandAloneFile)
			{
				//Dll::SetTls((TAny*)pStandAloneFile);
	            return pStandAloneFile;
			}
   		}
#endif
        A_ASSERT( in_bDataPath && !(in_nFlags & OPEN_WRITE) && !(in_nFlags & OPEN_APPEND) );
        A_ASSERT( !(in_pFilePath[0] != '\0' && in_pFilePath[1] == ':') );
	
		/// Create new archive 
		CArchivedFile* pFile = FILE_NEW(in_pFilePath) CArchivedFile(in_pFilePath, in_nFlags, nofail);

        if (!pFile->Exist())
	    {
	    	/// Too many debug information
//			DEBUG_PRINTF_1P( "ERROR: %s file doesn't exist\n", in_pFilePath );
		    MM_DELETE pFile;
		    pFile = NULL;
	    }

        if (pFile && (in_nFlags & OPEN_CACHE) && !(pFile->IsCompressed()))
        {
            pFile->CacheFile();
        }
/*		*((TInt32*)(Dll::Tls()))	=	(TInt32)pFile;*/
		//Dll::SetTls((TAny*)pFile);
	    return pFile;
    }
#endif

#if defined _WINDOWS && !defined(__BREW__)
    CWinFile* pFile = FILE_NEW(in_pFilePath) CWinFile(nofail);
#else
#ifdef __SYMBIAN32__
	CSymbianFile* pFile = FILE_NEW(in_pFilePath) CSymbianFile(nofail);
#else
#ifdef __BREW__

//	CBrewFile* pFile = NEW CBrewFile(nofail);
	CBrewFile* pFile = FILE_NEW(in_pFilePath) CBrewFile(nofail);

#else
	#error PLATFORM NOT DEFINED
#endif
#endif
#endif

#ifndef ASEMANIP_PARSERONLY
	/*const */char* path;


	if(in_pFilePath[0] != '\0' && in_pFilePath[1] == ':')
    {
        // Absolute path
		path = (char*)in_pFilePath;
    }
	else
    {
        // Relative path
        if(in_bDataPath)
		{
            path = CHighGear::GetInstance()->Gapi().MakePath(in_pFilePath);
		}
        else
        {
            path = CHighGear::GetInstance()->Gapi().MakePath(in_pFilePath);

            // Ensure that the path exist
            if ((in_nFlags&OPEN_WRITE) && strchr(in_pFilePath, '\\'))
            {
#ifdef __SYMBIAN32__
                int nStrLen = strlen(path);
                unsigned short * wszPath = STATIC_NEW unsigned short[nStrLen+1];
                for(int i = 0; i < nStrLen+1; ++i) wszPath[i] = path[i];
                TPtr16 ptr16(wszPath, nStrLen, nStrLen);
                RFs fileServer;
                fileServer.Connect();
                fileServer.MkDirAll(ptr16);
                DELETE_ARRAY wszPath;
#else
                {
	#ifdef __BREW__
						char * pPathEnd = STRRCHR(path, '\\');
						A_ASSERT(pPathEnd);
						// Temporary remove the file name
						*pPathEnd = '\0';
						IFileMgr * pIFileMgr = (IFileMgr *)CHighGear::GetInstance()->GetFileMgr();
						IFILEMGR_MkDir( pIFileMgr, path );
						// Restore the file name
						*pPathEnd = '\\';
	#else
						char * pPathEnd = strrchr(path, '\\');
						A_ASSERT(pPathEnd);
						// Temporary remove the file name
						*pPathEnd = '\0';
						#ifndef WINDOWS_MOBILE
							_mkdir(path);
						#else
							int len = strlen(path)+1;
							wchar_t *wText = new wchar_t[len];
							//if ( wText == 0 )
							//	  return;
							memset(wText,0,len);
							::MultiByteToWideChar(  CP_ACP, NULL,path, -1, wText,len );
							
							CreateDirectory(wText,NULL);					  
							// when finish using wText dont forget to delete it
							delete []wText;						

						#endif
						// Restore the file name
						*pPathEnd = '\\';
	#endif
                }
#endif
            }
        }
    }
#else//ASEMANIP_PARSERONLY
	const char* path = in_pFilePath;
#endif //ASEMANIP_PARSERONLY
	
	if(pFile)
	{
		if (!pFile->Create(path, in_nFlags))
		{
			MM_DELETE pFile;
			pFile = NULL;
		}
		
		if (pFile && (in_nFlags&OPEN_CACHE))
		{
			A_ASSERT(!(in_nFlags&OPEN_WRITE) && !(in_nFlags&OPEN_APPEND));
			pFile->CacheFile();
		}

		if (pFile && (in_nFlags&OPEN_WRITECACHE))
		{
			
			//ASSERT(!(in_nFlags&OPEN_READ) && (in_nFlags&OPEN_APPEND));
			//pFile->CacheFile();
			pFile->m_nCachePosition = 0;
			pFile->m_nCacheSize = k_nWriteCacheSize;
			if (pFile->m_nCacheSize > 0)
			{
				pFile->m_pWriteCache = STATIC_NEW unsigned char[pFile->m_nCacheSize];
			}
		}

		if (pFile && (in_nFlags&OPEN_CHECKSUM))
		{
			pFile->m_UseChecksum=true;
		}

	}

/*	*((TInt32*)(Dll::Tls()))	=	(TInt32)pFile;*/
//	Dll::SetTls((TAny*)pFile);
	return pFile;
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
A_IFile* A_IFile::CreateStreamBuffer()
{
    return STATIC_NEW CStreamBuffer();
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

void A_IFile::Close(A_IFile*& io_pFile)
{
	if(io_pFile->m_pWriteCache && io_pFile->m_nCachePosition)
	{
		io_pFile->SpecificWrite(io_pFile->m_pWriteCache,io_pFile->m_nCachePosition);
	}

	MM_DELETE io_pFile;
	io_pFile = 0;
	
/*	*((TInt32*)(Dll::Tls()))	=	0;*/

}


//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

bool A_IFile::Delete(const char* in_pFilePath, bool in_bDataPath)
{
#ifndef ASEMANIP_PARSERONLY
	const char* path;


	if(in_pFilePath[0] != '\0' && in_pFilePath[1] == ':')
    {
        // Absolute path
		path = in_pFilePath;
    }
	else
    {
        // Relative path
        if(in_bDataPath)
            path = CHighGear::GetInstance()->Gapi().MakePath(in_pFilePath);
        else
            path = CHighGear::GetInstance()->Gapi().MakePath(in_pFilePath);
    }
#else//ASEMANIP_PARSERONLY
	const char* path = in_pFilePath;
#endif //ASEMANIP_PARSERONLY

#ifdef __SYMBIAN32__ //amatei - NGI stuff

    RFs fs;
    fs.Connect();

    int nLength = strlen(path);
    TUint16 wszBuffer[128];
    for(int i = 0; i < nLength; ++i)
    {
        wszBuffer[i] = path[i];
    }
    wszBuffer[nLength] = '\0';
    TPtr16 ptrFilename(wszBuffer, nLength, nLength);
   
    return fs.Delete(ptrFilename) == 0;

#else

#ifdef __BREW__

	IFileMgr * pIFileMgr = (IFileMgr*)CHighGear::GetInstance()->GetFileMgr();
	IFILEMGR_Remove( pIFileMgr, path );
	return true;

#else
	#ifndef WINDOWS_MOBILE
		return remove(path) == 0;
	#else
		int len = strlen(path)+1;
		wchar_t *wText = new wchar_t[len];
		//if ( wText == 0 )
		//	  return;
		memset(wText,0,len);
		::MultiByteToWideChar(  CP_ACP, NULL,path, -1, wText,len );
		bool return_value = (DeleteFile(wText) == 0);
		delete []wText;	
		return return_value;
	#endif
#endif
#endif
}

bool A_IFile::DeleteDir(const char * in_pDirPath, bool in_bDataPath)
{
#ifndef ASEMANIP_PARSERONLY
	const char* path;

	if(in_pDirPath[0] != '\0' && in_pDirPath[1] == ':')
    {
        // Absolute path
		path = in_pDirPath;
    }
	else
    {
        // Relative path
        if(in_bDataPath)
            path = CHighGear::GetInstance()->Gapi().MakePath(in_pDirPath);
        else
            path = CHighGear::GetInstance()->Gapi().MakePath(in_pDirPath);
    }
#else//ASEMANIP_PARSERONLY
	const char* path = in_pDirPath;
#endif //ASEMANIP_PARSERONLY

#ifdef __SYMBIAN32__

    RFs fs;
    fs.Connect();

    int nLength = strlen(path);
    TUint16 wszBuffer[128];

	A_ASSERT(nLength< (sizeof(wszBuffer) /sizeof(wszBuffer[0])));

    for(int i = 0; i < nLength; ++i)
    {
        wszBuffer[i] = path[i];
    }
    wszBuffer[nLength] = '\0';
    TPtr16 ptrDirname(wszBuffer, nLength, nLength);
   
    return fs.RmDir(ptrDirname) == 0;

#else

	#ifdef __BREW__
		IFileMgr * pIFileMgr = (IFileMgr*)CHighGear::GetInstance()->GetFileMgr();
		IFILEMGR_RmDir( pIFileMgr, path );
		return true;
	#else
		#ifndef WINDOWS_MOBILE
			return _rmdir(path) == 0;
		#else
		int len = strlen(path)+1;
		wchar_t *wText = new wchar_t[len];
		//if ( wText == 0 )
		//	  return;
		memset(wText,0,len);
		::MultiByteToWideChar(  CP_ACP, NULL,path, -1, wText,len );
		bool return_value = (RemoveDirectory(wText) == 0);
		delete []wText;	
		return return_value;
		#endif
	#endif

#endif
}

bool A_IFile::Copy(const char* in_pFilePathSource, const char* in_pFilePathDestination)
{
	unsigned char * data=NULL;
	int len;

	// Read source
	// + Open source
	A_IFile* input = A_IFile::Open(in_pFilePathSource, A_IFile::OPEN_READ | A_IFile::OPEN_BINARY | A_IFile::OPEN_STANDALONE , false);
	if(!input)
		return false;

	// + Read data
	len = input->GetSize();
	//You shouldn't allocate memory if the file size is zero
	if (len>0)
	{
		data = STATIC_NEW unsigned char [len];
		input->Read(data,len);
	}

	// + Close
	A_IFile::Close(input);

	// Copy to destination
	// = Open destination
	A_IFile* output = A_IFile::Open(in_pFilePathDestination, A_IFile::OPEN_WRITE | A_IFile::OPEN_BINARY | A_IFile::OPEN_STANDALONE, false );
	if(!output)
		return false;

	// + Copy
	//You don't have write to the file if the len is zero
	if (len>0)
	{
		output->Write(data,len);
	}

	// + Close and release
	A_IFile::Close(output);
	
	if( data == NULL )
		return false;
	
	DELETE_ARRAY data;
	
	return true;
}


//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

void A_IFile::HandleReadError()
{
#ifdef __SYMBIAN32__
	A_ASSERT(0);
    // Do not tolerate any read error
	/// Spyder: Fix it
    //User::Leave(KErrEof);
#endif
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------


void A_IFile::WriteString(const char* str)
{
	A_ASSERT(::strlen(str)< 0xFF);

	unsigned char len = ::strlen(str);

	Write(&len,sizeof(len));
	Write(str,len);
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
const char* A_IFile::ReadString()
{
	unsigned char length;

	Read(&length,1);

	A_ASSERT(length < sizeof(m_tmpString));

	if(length){
		Read(m_tmpString, length);
	}

	m_tmpString[length] = 0;
	
	return m_tmpString;
}


//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

void A_IFile::CacheFile()
{
    if (!m_pCache)
    {
        m_nCachePosition = 0;
        m_nCacheSize = SpecificGetSize();
        if (m_nCacheSize > 0)
        {
            m_pCache = NEW unsigned char[m_nCacheSize];
            SpecificRead(m_pCache, m_nCacheSize);
	        SpecificClose();
        }
    }
}

unsigned int A_IFile::Read(void * out_pData, unsigned int in_nSize)
{
    if (m_pCache)
    {
        if(m_nCachePosition + in_nSize <= m_nCacheSize)
        {
            memcpy(out_pData, m_pCache + m_nCachePosition, in_nSize);
            m_nCachePosition += in_nSize;
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return SpecificRead(out_pData, in_nSize);
    }
}

unsigned int A_IFile::Write(const void * in_pData, unsigned int in_nSize)
{
	// Add checksum
	if(m_UseChecksum){
		const unsigned char * data=reinterpret_cast<const unsigned char*>(in_pData);
		for(unsigned int i=0;i<in_nSize;i++){
			m_Checksum+=(*data)+1;
			data++;
		}
	}


    A_ASSERT(!m_pCache);
	if(m_pWriteCache){
		// Cached write
		unsigned int ret=1;
		
		int toWrite=in_nSize;
		m_nWriteSize+=in_nSize;
		const unsigned char * pData=reinterpret_cast<const unsigned char *>(in_pData);


		
		while(toWrite+m_nCachePosition>=k_nWriteCacheSize){
			int space=k_nWriteCacheSize-m_nCachePosition;
			unsigned char * dest=m_pWriteCache+m_nCachePosition;
			const unsigned char * src=pData;
			unsigned char * end= dest + space;
			while(dest<end){
				*dest=*src;
				dest++;
				src++;
			}
			//memcpy(m_pWriteCache+m_nCachePosition,pData,space);
			if(SpecificWrite(m_pWriteCache,k_nWriteCacheSize)==0){
				ret=0;
			}
			toWrite-=space;
			pData+=space;
			m_nCachePosition=0;
		}
		
		if(toWrite){
//			memcpy(m_pWriteCache+m_nCachePosition,pData,toWrite);

			unsigned char * dest=m_pWriteCache+m_nCachePosition;
			const unsigned char * src=pData;
			unsigned char * end= dest + toWrite;
			while(dest<end){
				*dest=*src;
				dest++;
				src++;
			}
			
			m_nCachePosition+=toWrite;
		}


		return ret;

	}else{
		// Regular write
    return SpecificWrite(in_pData, in_nSize);
}
}


bool A_IFile::ValidateChecksum()
{
	A_ASSERT(m_pCache);

	int len = GetSize();
	int a=0;
	unsigned char * data = reinterpret_cast<unsigned char*>(GetBuffer());

	// Calculate file checksum
	for(unsigned int i=0;i<len-sizeof(unsigned int);i++){
		a+=(*data)+1;
		data++;
	}
	
	// Read checksum
	unsigned int checksum;
	memcpy(&checksum,data,sizeof(checksum));

	// Compare
	return checksum==a;

}

long A_IFile::GetSize()
{
    if (m_pCache)
    {
        return m_nCacheSize;
    }
    else if(m_pWriteCache)
    {
		return m_nWriteSize;
    }
    else
    {
        return SpecificGetSize();
    }
}

bool A_IFile::Seek(unsigned int nOffsetFromStart)
{
    bool bRet = true;
    if (m_pCache)
    {
         m_nCachePosition = nOffsetFromStart;
         if (m_nCachePosition < 0)
         {
             m_nCachePosition = 0;
         }
         if (m_nCachePosition > m_nCacheSize - 1)
         {
             m_nCachePosition = m_nCacheSize - 1;
         }

         return true;
    }
    else
    {
        return SpecificSeek(nOffsetFromStart);
    }
}

int A_IFile::GetCurrentOffset()
{
	if (m_pCache)
         return m_nCachePosition;

	return 0;
}

char A_IFile::GetChar()
{
    char data;
    Read(&data, 1);

    return data;
}

short A_IFile::GetShort()
{
    short data;
    Read(&data, sizeof(short));
    return data;
}

int A_IFile::GetInt()
{
    int data;
    Read(&data, sizeof(int));
    return data;
}

long  A_IFile::GetLong()
{
	long data;
	Read(&data, sizeof(long));
	return data;
}
void A_IFile::SkipLine()
{
    // Read the integer
    char data = GetChar();
    while(!(data == '\n'))
    {
        data = GetChar();
    }
}

void* A_IFile::GetBuffer()
{
    return m_pCache;
}


//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

#if defined _WINDOWS && !defined(__BREW__)

CWinFile::CWinFile(bool noFail) 
	:A_IFile(noFail),
    m_pFile(0)
{
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

bool CWinFile::Create(const char* in_pFilePath, int in_nFlags)
{
    std::string flags;
    if ((in_nFlags & OPEN_READ_WRITE) == OPEN_READ_WRITE
        || in_nFlags & OPEN_APPEND)
    {
        flags += "a+";
    }
    else if (in_nFlags & OPEN_READ)
    {
        flags += 'r';
    }
    else
    {
        flags += 'w';
    }
    flags += ((in_nFlags & OPEN_BINARY_TEXT) == OPEN_BINARY) ? 'b' : 't';
    m_pFile = fopen(in_pFilePath, flags.c_str());

		if(m_noFail && m_pFile==0)
			Throw(0);

    if ((in_nFlags & OPEN_READ_WRITE) == OPEN_READ_WRITE)
    {
        fseek(m_pFile, 0, SEEK_SET);
    }

    return (m_pFile != 0);
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

CWinFile::~CWinFile()
{
	SpecificClose();
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

unsigned int CWinFile::SpecificRead(void * out_pData, unsigned int in_nSize)
{
  int res = fread(out_pData, in_nSize, 1, m_pFile);
	if(m_noFail && res!=1)
		Throw(0);
	return res;
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

unsigned int CWinFile::SpecificWrite(const void * in_pData, unsigned int in_nSize)
{
	int res = fwrite(in_pData, in_nSize, 1, m_pFile);
	if(m_noFail && res != 1)
		Throw(0);
	return res;
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

bool CWinFile::SpecificSeek(unsigned int nOffsetFromStart)
{
	fseek(m_pFile, nOffsetFromStart, SEEK_SET);
    return true;
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

long CWinFile::SpecificGetSize()
{
    long nSavedPosition = ftell(m_pFile);
    long nSize = 0;
    fseek(m_pFile, 0, SEEK_END);
    nSize = ftell(m_pFile);
    fseek(m_pFile, nSavedPosition, SEEK_SET);
    return nSize;
}

void CWinFile::SpecificClose()
{
	if (m_pFile)
    {
		fclose(m_pFile);
        m_pFile = 0;
    }
}

void CWinFile::Flush()
{
    if (m_pFile)
    {
        fflush(m_pFile);
    }
}

#endif

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

#ifdef __SYMBIAN32__ //amatei - NGI stuff

/// CSymbianFile

CSymbianFile::CSymbianFile(bool noFail)
	:A_IFile(noFail)
{
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

bool CSymbianFile::Create(const char* in_pFilePath, int in_nFlags)
{
  m_FileServer.Connect();

  int nLength = strlen(in_pFilePath);
  TUint16 wszBuffer[128];

  A_ASSERT(nLength< (sizeof(wszBuffer) /sizeof(wszBuffer[0])));

  for(int i = 0; i < nLength; ++i)
  {
      wszBuffer[i] = in_pFilePath[i];
  }
  wszBuffer[nLength] = '\0';
  
  TPtr16 ptrFilename(wszBuffer, nLength, nLength);

  TUint nFlag = 0;

  if (in_nFlags & OPEN_APPEND)
  {
      nFlag |= EFileRead | EFileWrite;
  }
  else
  {
      if (in_nFlags & OPEN_READ)
      {
          nFlag |= EFileRead;
      }
      if (in_nFlags & OPEN_WRITE)
      {
          nFlag |= EFileWrite;
      }
  }

  if((in_nFlags & OPEN_BINARY_TEXT) == OPEN_TEXT)
  {
      nFlag |= EFileStreamText;
  }

	bool bRes;
	//int ret;

	bRes = ( m_File.Open(m_FileServer, ptrFilename, nFlag) == KErrNone);

	if(!bRes && (nFlag & EFileWrite))
	{
		//file does not exist
		bRes = (/*(ret =*/ m_File.Create(m_FileServer, ptrFilename, nFlag) == KErrNone);
	}

    if (bRes && in_nFlags & OPEN_APPEND)
    {
        TInt pos = 0;
        m_File.Seek(ESeekEnd, pos);
    }
	
	return bRes;
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

CSymbianFile::~CSymbianFile()
{
    SpecificClose();
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

void CSymbianFile::SpecificClose()
{
    m_File.Close();
    m_FileServer.Close();
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

unsigned int CSymbianFile::SpecificRead(void * out_pData, unsigned int in_nSize)
{
    TPtr8 ptrData((TUint8*) out_pData, in_nSize, in_nSize);
    TInt ret = m_File.Read(ptrData);
    return (ret == KErrNone && ptrData.Length() == in_nSize) ? 1 : 0;
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

unsigned int CSymbianFile::SpecificWrite(const void * out_pData, unsigned int in_nSize)
{
    TPtrC8 ptrData((const TUint8*) out_pData, in_nSize);
    return ((m_File.Write(ptrData) == KErrNone) ? 1 : 0);
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

bool CSymbianFile::SpecificSeek(unsigned int nOffsetFromStart)
{
    TInt nSize = nOffsetFromStart;
    TInt nErr = m_File.Seek(ESeekStart, nSize);
    return (nErr == KErrNone);
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

long CSymbianFile::SpecificGetSize()
{
    TInt nSize = 0;
    m_File.Size(nSize);
    return (long) nSize;
}


void CSymbianFile::Flush()
{
    m_File.Flush();
}

#endif




//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------



#ifdef __BREW__

CBrewFile::CBrewFile(bool noFail) 
:A_IFile(noFail)
,m_pFile(0)
{
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

bool CBrewFile::Create(const char* in_pFilePath, int in_nFlags)
{
	OpenFileMode flags;
	if ((in_nFlags & OPEN_READ_WRITE) == OPEN_READ_WRITE
		|| in_nFlags & OPEN_APPEND)
	{
		flags = _OFM_APPEND;
	}
	else if (in_nFlags & OPEN_READ)
	{
		flags = _OFM_READ;
	}
	else
	{
		flags = _OFM_CREATE;
	}

	IFileMgr * pIFileMgr = (IFileMgr*)CHighGear::GetInstance()->GetFileMgr();

	m_pFile = IFILEMGR_OpenFile( pIFileMgr, in_pFilePath, flags );
	if ( !m_pFile && flags == _OFM_CREATE )
		m_pFile = IFILEMGR_OpenFile( pIFileMgr, in_pFilePath, _OFM_READWRITE );

	if(m_noFail && m_pFile==0) {
		Throw(0);
	} else if ( !m_pFile ) {
		
		return false;
	}

	if ((in_nFlags & OPEN_READ_WRITE) == OPEN_READ_WRITE)
	{
		IFILE_Seek( m_pFile, _SEEK_START, 0 );
	}

/*
uint32	dwTotal;
IFILEMGR_GetFreeSpace(pIFileMgr, &dwTotal);
DBGPRINTF(" +++ room available on the file system: %lu", dwTotal);
AEEFileUseInfo fui;
IFILEMGR_GetFileUseInfo(pIFileMgr, &fui);
DBGPRINTF(" +++ max space: %lu used: %lu", fui.dwMaxSpace, fui.dwSpaceUsed); 
*/


	return (m_pFile != 0);
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

CBrewFile::~CBrewFile()
{
	SpecificClose();
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

unsigned int CBrewFile::SpecificRead(void * out_pData, unsigned int in_nSize)
{
	if ( !m_pFile ) return 0;

	if( IFILE_Read( m_pFile, out_pData, in_nSize ) > 0 )
		return true;
	else
		return false;
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

unsigned int CBrewFile::SpecificWrite(const void * in_pData, unsigned int in_nSize)
{
	if ( !m_pFile ) return 0;

	char*	ptr = (char*)in_pData;
	int		ret = 1;
	int		retval = 0;


	// write until return 0 (bug when not all the data is written at once)
	while (ret && in_nSize)
	{	
		ret = IFILE_Write( m_pFile, ptr, in_nSize );
/*if (ret == 0)
{
	IFileMgr * pIFileMgr = (IFileMgr*)CHighGear::GetInstance()->GetFileMgr();
	int err = IFILEMGR_GetLastError(pIFileMgr);
	DBGPRINTF("FileWrite Err : %d", err );
	uint32	total = 0;
	err = IFILEMGR_GetFreeSpace(pIFileMgr, &total) ;
	DBGPRINTF("FileWrite free : %d / %d", err, total );
}
DBGPRINTF("FileWrite %d/%d", ret, in_nSize);*/

		if (ret > 0)
		{
			retval += ret;
			in_nSize -= ret;
			ptr += ret;
		}
	}

	return retval;
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

bool CBrewFile::SpecificSeek(unsigned int nOffsetFromStart)
{
	if ( !m_pFile ) return false;

	if ( IFILE_Seek( m_pFile, _SEEK_START, nOffsetFromStart ) == SUCCESS )
		return true;
	else
		return false;
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

long CBrewFile::SpecificGetSize()
{
	if ( !m_pFile ) return 0;

	FileInfo fileInfo;
	IFILE_GetInfo( m_pFile, &fileInfo );
	return fileInfo.dwSize;
}

void CBrewFile::SpecificClose()
{

	if (m_pFile)
	{
		IFILE_Release( m_pFile );
		m_pFile = 0;
	}
}

void CBrewFile::Flush()
{
	// NOT VALID UNDER BREW
}

#endif
