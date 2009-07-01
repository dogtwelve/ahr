#include "config.h"
#include "DevUtil.h"
#include "ArchivedFileManager.h"
#include "System/MemoryAllocation.h"
#include "HG/HighGear.h"
#include "ZLib/zlib.h"
#include "throw.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef IPHONE
//...
extern "C" void LOGDEBUG(const char *error, ...);
#else
#include <direct.h>
#endif



//need to construct the file absolute path
char g_FileAbsolutePath[1024];
extern char g_AppPath[];

////////////////////////////////////////////////////////////////////////////////////////
//@Class CArchivedFile
////////////////////////////////////////////////////////////////////////////////////////

#define COMPRESSED_HEADER_SIZE	4

#include "ArchivedFileManager.h"

#ifdef ASEMANIP_READERONLY

class CArchivedFile
    : public A_IFile
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

#endif //ASEMANIP_READERONLY

////////////////////////////////////////////////////////////////////////////////////////////////////
//@Implementation A_IFile 
////////////////////////////////////////////////////////////////////////////////////////////////////


#ifdef CHECK_MEMORY_LEAKS
	#define FILE_NEW(filename) ::new(filename, __LINE__, 0, (USE_MEMORYMANAGER_NEW*)0)
#else
	#define FILE_NEW(filename) STATIC_NEW
#endif

//Helper functions
inline bool IsBlank(char in_Char)
{
    return (in_Char == ' ' || in_Char == '\n' || in_Char == '\b' || in_Char == '\t' || in_Char == '\r');
}

//Class members


A_IFile::~A_IFile()
{
	if (m_pCache)
		MM_DELETE m_pCache;
    m_pCache = 0;
	if (m_pWriteCache)
		DELETE_ARRAY m_pWriteCache;
	m_pWriteCache=0;
}

A_IFile* A_IFile::Open(const char* in_pFilePath, int in_nFlags, bool nofail, bool in_bDataPath)
{
#ifdef IPHONE
	LOGDEBUG("TRY TO OPEN [%s]\n", in_pFilePath);
#endif

#ifdef ASEMANIP_READERONLY
    if( !(in_nFlags & OPEN_STANDALONE) )
    {
        // First try without using archive
    	//if( in_pFilePath[0] == 'u' ) // for user.dat 
		if(strstr(in_pFilePath, "user") != NULL
#ifdef JK_TEMPORARY
			|| strstr(in_pFilePath, ".bsprite") != NULL
#endif
			)
   		{
	        A_IFile* pStandAloneFile = A_IFile::Open(in_pFilePath, in_nFlags | OPEN_STANDALONE, false, in_bDataPath);
	        if (pStandAloneFile)
			{
	            return pStandAloneFile;
			}
   		}

		A_ASSERT( in_bDataPath && !(in_nFlags & OPEN_WRITE) && !(in_nFlags & OPEN_APPEND) );
        A_ASSERT( !(in_pFilePath[0] != '\0' && in_pFilePath[1] == ':') );

        /// Create new archive 
		CArchivedFile* pFile = FILE_NEW(in_pFilePath) CArchivedFile(in_pFilePath, in_nFlags, nofail);

        if (!pFile->Exist())
	    {
		    MM_DELETE pFile;
		    pFile = NULL;
	    }

        if (pFile && (in_nFlags & OPEN_CACHE) && !(pFile->IsCompressed()))
        {
            pFile->CacheFile();
        }
	    return pFile;
    }
#endif

    CWinFile* pFile = FILE_NEW(in_pFilePath) CWinFile(nofail);

	const char* path = in_pFilePath;
	
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
}


//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

bool A_IFile::Delete(const char* in_pFilePath, bool in_bDataPath)
{
	const char* path = in_pFilePath;
    return remove(path) == 0;   
}


//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

void A_IFile::HandleReadError()
{
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
		int result = SpecificRead(out_pData, in_nSize);
		return result;
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


////////////////////////////////////////////////////////////////////////////////////////////////////
//@Implementation CachedFile 
////////////////////////////////////////////////////////////////////////////////////////////////////

CCachedFile::CCachedFile(const char* sourceFile)
{
	//S_Print("Cache archive %s", sourceFile);
	FILE * pFile;
	long size;

#ifdef IPHONE
	//construct the absolute path
	sprintf(g_FileAbsolutePath, "%s%s", g_AppPath, sourceFile);
	pFile = fopen (g_FileAbsolutePath,"rb");
#else /* IPHONE */
	pFile = fopen (sourceFile,"rb");
#endif /* IPHONE */
	
	if (pFile==NULL)
	{
		return;
	}
	else
	{
		fseek(pFile, 0, SEEK_END);
		size = ftell(pFile);
	}

	fseek(pFile, 0, SEEK_SET);
//	rewind(pFile);

	content = new char[size];

	long actualRead = fread((void*)content, 1, size, pFile);
	//FS_Print("size = %ld  actualRead = %ld", size, actualRead);

	//int cpos = 0;

	//int chunks = size / CHUNK_SIZE;
	//int remainder = size % CHUNK_SIZE;

	//for(int i = 0 ; i < chunks ; i++)
	//{
	//	fread((void*)((char*)content + cpos), 1, CHUNK_SIZE, pFile);
	//	cpos += CHUNK_SIZE;
	//}

	//if(remainder != 0)
	//{
	//	fread((void*)((char*)content + cpos), 1, remainder, pFile);
	//}

	fclose(pFile);
	crtPos = 0;
	fileSize = size;
}

int CCachedFile::Read(void * out_pData, unsigned int in_nSize)
{
	if(crtPos + in_nSize > fileSize)
		{
			return 0;
		}
	memcpy(out_pData, content + crtPos, in_nSize);
	crtPos += in_nSize;
	//return in_nSize;
	return 1;
}

int CCachedFile::GetInt()
{
    int data;
    Read(&data, sizeof(int));
    return data;
}

CCachedFile::~CCachedFile()
{
	delete content;
	content = NULL;
}

bool CCachedFile::Seek(unsigned int nOffsetFromStart)
{
	crtPos = nOffsetFromStart;
	if(crtPos > fileSize)
	{
		crtPos = fileSize - 1;
	}
	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//@Implementation CArchivedFile 
////////////////////////////////////////////////////////////////////////////////////////////////////


#ifdef ASEMANIP_READERONLY

CArchivedFile::CArchivedFile(const char* pFileName, int iFlags, bool noFail)
	:A_IFile(noFail)
{
	//FS_Print("Extract from archive %s", pFileName);
	pArchive = NULL;

	CArchivedFileManager*	arcMan = CArchivedFileManager::GetInstance();

    iFileOffset		= arcMan->GetFileIndex(pFileName);
	iSeek			= iFileOffset;
    iFileSize		= arcMan->GetFileSize(pFileName);
	//FS_Print("fileSize = %d", iFileSize);
	bFileCompressed = arcMan->GetFileCompression(pFileName);

	if(bFileCompressed)
	{
		//FS_Print("File is compressed");
	}
	else
	{
		//FS_Print("File is NOT compressed");
	}

	pFileBuffer = NULL;

	if (arcMan->CurrentArchive == NULL)
	{
		if (noFail)
		{
			A_ASSERT( arcMan->CurrentArchive );
		}
		else
			return;
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
			return;
	}

    // If the file wasn't found!
    
    if(noFail && (iFileOffset == -1 || iFileSize == -1))
    {
        A_ASSERT(false);
        HandleReadError();
    }

	if (iFileOffset == -1 || iFileSize == -1)
    {
        return;
    }

	//uncompress the file and put the data in a buffer
	if (bFileCompressed)
	{
		unsigned int uiRealFileSize;
		unsigned long ulUncompressSize;
		unsigned char ucSize[COMPRESSED_HEADER_SIZE];
		unsigned char* pBuffer = NEW unsigned char[iFileSize];



		//read the uncompressed size of the file
		if (!pArchive->pDataFile->Seek(iSeek + pArchive->iDataFileOffset))
        {
            HandleReadError();
        }

		if (!pArchive->pDataFile->Read(ucSize, COMPRESSED_HEADER_SIZE))
        {
            HandleReadError();
        }

		//retreive the file size
		uiRealFileSize  = ((unsigned char)(*(ucSize )))      << 24;
		uiRealFileSize |= ((unsigned char)(*(ucSize  + 1)))  << 16;
		uiRealFileSize |= ((unsigned char)(*(ucSize  + 2)))  <<  8;
		uiRealFileSize |= ((unsigned char)(*(ucSize  + 3)));

		//FS_Print("**** Real file size = %ld", uiRealFileSize);

		//buffer fot the uncompressed file
		pFileBuffer = NEW unsigned char[uiRealFileSize];

		//read the compressed data
		if (!pArchive->pDataFile->Seek(iSeek+COMPRESSED_HEADER_SIZE + pArchive->iDataFileOffset))
        {
            HandleReadError();
        }

		if (!pArchive->pDataFile->Read(pBuffer, iFileSize))
        {
            HandleReadError();
        }
	
		//uncompress the data
		ulUncompressSize = uiRealFileSize;

		BREW_HEAP		pheap;
		pheap.ptr = arcMan->m_heap1;
		pheap.pos = 0;

		int err = uncompress( &pheap, pFileBuffer, &ulUncompressSize, pBuffer, iFileSize );

		DELETE_ARRAY pBuffer;

		iFileOffset     = 0;
		iSeek			= iFileOffset;
		iFileSize       = uiRealFileSize;
	}
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


////////////////////////////////////////////////////////////////////////////////////////////////////
//@Implementation CStreamBuffer
////////////////////////////////////////////////////////////////////////////////////////////////////



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

////////////////////////////////////////////////////////////////////////////////////////////////////
//@Implementation CWinFile
////////////////////////////////////////////////////////////////////////////////////////////////////


CWinFile::CWinFile(bool noFail) 
	:A_IFile(noFail),
	m_pFile(0)
{
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

extern void GetSaveFilePath(char* filePath, const char* filename);

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


#ifdef IPHONE
	
	if ((in_nFlags & OPEN_USER_DATA) != 0)
		GetSaveFilePath(g_FileAbsolutePath, in_pFilePath);
	else
		sprintf(g_FileAbsolutePath, "%s%s", g_AppPath, in_pFilePath);
	
	m_pFile = fopen(g_FileAbsolutePath, flags.c_str());
#else /* IPHONE */	
	m_pFile = fopen(in_pFilePath, flags.c_str());
#endif /* IPHONE */

	if(m_noFail && m_pFile==0)
	{
		Throw(0);
		return 0;
	}

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


////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
	extern "C" 
	{
#endif /* __cplusplus */

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
#endif /* __cplusplus */
