#ifndef __ARCHIVEDFILEMANAGER__
#define __ARCHIVEDFILEMANAGER__

#include "Config.h"
#include "Singleton.h"
#include "File.h"
#include <map>

#define ARCHIVE_MAIN	0
#define ARCHIVE_CARSA	1
#define ARCHIVE_CARSB	2
#define ARCHIVE_TRACKA	3
#define ARCHIVE_TRACKB	4
#define ARCHIVE_TRACKC	5
#define ARCHIVE_TRACKD	6
#define ARCHIVE_TRACKE	7
#define ARCHIVE_TRACKF	8
#define ARCHIVE_TRACKG	9
#define ARCHIVE_TRACKI	10
#define ARCHIVE_TRACKJ	11
#define ARCHIVE_TRACKK	12
#define ARCHIVE_TRACKL	13
#define ARCHIVE_TRACKM	14
#define ARCHIVE_TRACKN	15


//#define ARCHIVE_TRACKH	10

#ifndef	DISABLE_SOUND_OPTIONS
	#define ARCHIVE_SOUNDS	10
	
	#ifdef USE_12_TRACKS
//TEST 0518
		#define ARCHIVE_NB		1 //17
	#else
		#define ARCHIVE_NB		11
	#endif
#else
	#ifdef USE_12_TRACKS
		#define ARCHIVE_NB		16
	#else
		#define ARCHIVE_NB		10
	#endif
#endif


#define OffsetByteNumber    4

typedef struct 
{
	unsigned char*	ptr;
	int				pos;

} BREW_HEAP;

#define HEAP1_SIZE	(1024 * 500) 

//#define USE_FILE_HEAP		// to try get better loading times, and less memory frag

#ifdef USE_FILE_HEAP

	#define HEAP2_SIZE	65536
	#define HEAP3_SIZE	(65536*3)

#endif

class CArchivedFileManager : public CSingleton<k_nSingletonArchivedFilesManager, CArchivedFileManager>
{
public:

    struct ltstr
    {
      bool operator()(const char* s1, const char* s2) const
      {
        return strcmp(s1, s2) < 0;
      }
    };

    struct SFileProperties
    {
        unsigned long ulSize;
        unsigned long ulOffset;
		bool          bIsCompressed;
    };

	typedef struct 
	{
		A_IFile*                        pDataFile;
		int								iDataFileSize;
		int								iDataFileOffset;
		int								iMapFileSize;
		char*							cMappingTable;
	    std::map<char*, SFileProperties, ltstr>      pFileNameMappingTable;

	} ArchiveData;


	ArchiveData						Archives[ARCHIVE_NB];
	ArchiveData*					CurrentArchive;
	ArchiveData*					OpenCurrentArchive();
	void							CloseArchive(ArchiveData*);
	void 							CloseAllArchives();

    CArchivedFileManager();
    ~CArchivedFileManager();

	void							CreateAllFileMappingTable();

    unsigned long                   GetFileIndex(const char* cFileName);
    unsigned long                   GetFileSize(const char* cFileName);
	bool                            GetFileCompression(const char* cFileName);


	// heap for file decompression
	unsigned char*					m_heap1;	// to store trees
#ifdef USE_FILE_HEAP
	unsigned char*					m_heap2;	// to store compressed file
	unsigned char*					m_heap3;	// to store uncompressed file
#endif

	//bool			IsFilePresent(FileRef i) const{return i!=pFileNameMappingTable.end();}
	//A_IFile*			OpenECPFile(ECPFileId);

    void                            CreateFileMappingTable( int archiveNb );


private:
    

	//ECPFileData						m_ecpFileData[ECPFILE_NBRFILES];


    unsigned int                    RetrieveFileOffset(char* cBuffer, int iOffset, bool* dataCompressed);
    void                            StringToLower(char* cString);
};

#endif