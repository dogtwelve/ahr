
#ifndef _File_Iphone_H_
#define _File_Iphone_H_

#include <stdio.h>
////////////////////////////////////////////////////////////////////////////////////////
//@Class A_IFile
////////////////////////////////////////////////////////////////////////////////////////

class A_IFile
{
public:
    enum EOpenFlags
    {
        OPEN_BINARY_TEXT    = 0x01,
        OPEN_BINARY         = OPEN_BINARY_TEXT,
        OPEN_TEXT           = 0,

        OPEN_READ           = 0x02,
        OPEN_WRITE          = 0x04,
        OPEN_READ_WRITE     = OPEN_READ | OPEN_WRITE,
        OPEN_APPEND         = 0x08,
        OPEN_STANDALONE     = 0x10,

        OPEN_CACHE          = 0x20,
        OPEN_WRITECACHE     = 0x40,
        OPEN_CHECKSUM       = 0x80,
		
		OPEN_USER_DATA		= 0x100,
    };

	enum {k_nWriteCacheSize=4096};


//	static A_IFile* OpenECP(ECPFileId);

	static A_IFile* Open(const char* in_pFilePath, int in_nFlags, bool nofail = true, bool in_bDataPath = true);
    static A_IFile* CreateStreamBuffer();
	static void Close(A_IFile*& io_pFile);
    static bool Delete(const char* in_pFilePath, bool in_bDataPath);
	static bool ExistInPath(const char * in_pFilePath, bool in_bIsDataPath = false);
    virtual void HandleReadError();

    A_IFile(bool noFail):m_noFail(noFail),m_pCache(0),m_nCacheSize(0),m_nCachePosition(0),m_pWriteCache(0),m_nWriteSize(0),m_Checksum(0),m_UseChecksum(false){};
    virtual ~A_IFile();


    const char*	ReadString();
    void	WriteString(const char*);

    unsigned int Read(void * out_pData, unsigned int in_nSize);
	inline	int					 ReadInt()	{int val; Read(&val,sizeof(int));return val;}
    unsigned int Write(const void * in_pData, unsigned int in_nSize);
    long GetSize();
    bool Seek(unsigned int nOffsetFromStart);

    char GetChar();
	short GetShort();
	int GetInt();
	long GetLong();
    void SkipLine();

	int GetCurrentOffset();

    virtual void Flush() = 0;

    // If the file is all allocated in a buffer
    // it returns directly this buffer
    virtual void* GetBuffer();

protected:
    void CacheFile();

protected:
    virtual unsigned int SpecificRead(void * out_pData, unsigned int in_nSize)=0;
    virtual unsigned int SpecificWrite(const void * in_pData, unsigned int in_nSize)=0;
    virtual long SpecificGetSize()=0;
    virtual bool SpecificSeek(unsigned int nOffsetFromStart)=0;
    virtual void SpecificClose()=0;

protected:

	const bool	m_noFail;
	bool m_UseChecksum;

    unsigned char * m_pCache;
    unsigned int	m_nCacheSize;
    unsigned char * m_pWriteCache;
    unsigned int	m_nWriteSize;
    unsigned int	m_nCachePosition;
	unsigned int	m_Checksum;
		char					m_tmpString[128];


};

////////////////////////////////////////////////////////////////////////////////////////
//@Class CCachedFile
////////////////////////////////////////////////////////////////////////////////////////

//#define CHUNK_SIZE	(65535)
class CCachedFile
{
	public:
		char* content;
		long crtPos, fileSize;
		CCachedFile(const char* sourceFile);
		int Read(void * out_pData, unsigned int in_nSize);
		int GetInt();
		bool Seek(unsigned int nOffsetFromStart);
		~CCachedFile();
};

////////////////////////////////////////////////////////////////////////////////////////
//@Class CStreamBuffer
////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////
//@Class File
////////////////////////////////////////////////////////////////////////////////////////

class File
{
public:
	File(const char* fileName)
	{
		m_file = A_IFile::Open( fileName, A_IFile::OPEN_READ | A_IFile::OPEN_BINARY | A_IFile::OPEN_CACHE);
	}

	~File()
	{
		if(m_file)
			A_IFile::Close(m_file);
	}

	long ReadInt()
	{
		long i;
		m_file->Read(&i,sizeof(i));
		return i;
	}

	char ReadChar()
	{
		char i;
		m_file->Read(&i,sizeof(i));
		return i;
	}


	void Close()
	{
		if(m_file)
			A_IFile::Close(m_file);
		m_file = 0; //NULL;
	}


private:
	A_IFile* m_file;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//@Class CWinFile
////////////////////////////////////////////////////////////////////////////////////////////////////

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



#endif /* _File_Iphone_H_ */

