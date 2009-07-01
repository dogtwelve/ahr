

#ifdef USE_IPHONE_FILE_IMPLEMENTATION

#include "iPhone/File_iPhone.h"

#else /* USE_IPHONE_FILE_IMPLEMENTATION */


// A_IFile.h: interface for the A_IFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILE_H__5C1DAC8A_E8C0_4BF5_8FD0_2C3E02315DC1__INCLUDED_)
#define AFX_FILE_H__5C1DAC8A_E8C0_4BF5_8FD0_2C3E02315DC1__INCLUDED_

//#define MAX_FILE_NAME 64

/*enum ECPFileId
{
	ECPFILE_CHAMPION_CFG,
	ECPFILE_GAME_CFG,
	ECPFILE_IA_CFG,
	ECPFILE_TUNING_CFG,

	ECPFILE_NBRFILES
};*/


#define DBG_BT debug_out


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
    static bool DeleteDir(const char * in_pDirPath, bool in_bDataPath);
	//static bool ExistInSavePath(const char* in_pFilePath);
	static bool ExistInPath(const char * in_pFilePath, bool in_bIsDataPath = false);
	static bool Copy(const char* in_pFilePathSource, const char* in_pFilePathDestination);
    virtual void HandleReadError();

	//Closes the last buffer opened by A_IFile::Open
	static void CloseLastBuffer();
    
    A_IFile(bool noFail):
		m_noFail(noFail),
		m_UseChecksum(false),
		m_pCache(0),
		m_nCacheSize(0),
		m_pWriteCache(0),
		m_nWriteSize(0),
		m_nCachePosition(0),
		m_Checksum(0)
		{};
    virtual ~A_IFile();
    
    
    const char*	ReadString();
    void	WriteString(const char*);
    
    unsigned int Read(void * out_pData, unsigned int in_nSize);
	inline	int	 ReadInt()	{int val; Read(&val,sizeof(int));return val;}
    unsigned int Write(const void * in_pData, unsigned int in_nSize);
    long GetSize();
    bool Seek(unsigned int nOffsetFromStart);

    char GetChar();
	short GetShort();
	int GetInt();
	long GetLong();
    void SkipLine();
	int WriteChecksum(){m_UseChecksum=false; return Write(&m_Checksum,sizeof(m_Checksum));}
	bool ValidateChecksum();

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

A_IFile& operator>>(A_IFile& in_File, char * out_pReadValue);
A_IFile& operator>>(A_IFile& in_File, int& out_nReadValue);
void SkipLine(A_IFile& in_File);

//////////////////////////////////////////////////////////////////////////

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

#endif // !defined(AFX_IFILE_H__5C1DAC8A_E8C0_4BF5_8FD0_2C3E02315DC1__INCLUDED_)

#endif /* USE_IPHONE_FILE_IMPLEMENTATION */
