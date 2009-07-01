
#pragma warning(disable:4786)
#include "A4_3d_platform_def.h"

#include "GenDef.h"
#include <ctype.h>

#include "ArchivedFileManager.h"
#include "File.h"

const char* const ArchiveFileNames[ARCHIVE_NB] =
{
	"main.bar"//,
	//TEST 0518
//	"carsa.bar",
//	"carsb.bar",
//	"tracksa.bar",
//	"tracksb.bar",
//#if defined USE_DRM && defined NGI
//	"tracksc.bar.ngdat",
//	"tracksd.bar.ngdat",
//	"trackse.bar.ngdat",
//	"tracksf.bar.ngdat",
//	"tracksg.bar.ngdat",
//#else	
//	"tracksc.bar",
//	"tracksd.bar",
//	"trackse.bar",
//	"tracksf.bar",
//	"tracksg.bar",
//
//#ifdef USE_12_TRACKS
//	"tracksi.bar",
//	"tracksj.bar",
//	"tracksk.bar",
//	"tracksl.bar",
//	"tracksm.bar",
//	"tracksn.bar",
//#endif
//#endif // USE_DRM
//
//#ifndef	DISABLE_SOUND_OPTIONS	
//	"sounds.bar"	
//#endif	
};

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------


CArchivedFileManager::CArchivedFileManager()
{
	CurrentArchive 	= NULL;

	for (int i = 0; i < ARCHIVE_NB; i++)
	{
		Archives[i].pDataFile = NULL;
		Archives[i].cMappingTable = NULL;
		// rax - speed up loading, create mapping table only for main.bar
		//CreateFileMappingTable( i );
	}
	CreateFileMappingTable( 0 );

	// alloc HEAP
	m_heap1 = (unsigned char*)MM_MALLOC( HEAP1_SIZE );

#ifdef USE_FILE_HEAP
	m_heap2 = (unsigned char*)MM_MALLOC( HEAP2_SIZE );
	m_heap3 = (unsigned char*)MM_MALLOC( HEAP3_SIZE );
#endif
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

void CArchivedFileManager::CreateAllFileMappingTable()
{
	for (int i = 0; i < ARCHIVE_NB; i++)
	{
		if (Archives[i].cMappingTable == NULL)
			CreateFileMappingTable( i );
	}
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

CArchivedFileManager::ArchiveData*	CArchivedFileManager::OpenCurrentArchive()
{
	if (CurrentArchive == NULL)
		return NULL;

	if( CurrentArchive->pDataFile == NULL )
	{		
		//debug_out( "	Archive name: %s\n", ArchiveFileNames[CurrentArchive-Archives] );
		CurrentArchive->pDataFile = A_IFile::Open(ArchiveFileNames[CurrentArchive-Archives], A_IFile::OPEN_READ | A_IFile::OPEN_BINARY | A_IFile::OPEN_STANDALONE, false);			
	}
	else
	{
		CurrentArchive->pDataFile->Seek( 0 );
	}
	
	if (CurrentArchive->pDataFile)
		return CurrentArchive;
	else
		return NULL;

//	// jump header + mapping file, set to start of data
//	pDataFile->Seek( iDataFileOffset );
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

void	CArchivedFileManager::CloseArchive(ArchiveData* pArc)
{
	if ( pArc==NULL || pArc->pDataFile == NULL )
		return;

	A_IFile::Close(pArc->pDataFile);  

	pArc->pDataFile = NULL;
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

void 	CArchivedFileManager::CloseAllArchives()
{
	for(int i=0; i<ARCHIVE_NB; i++)
	{
		CloseArchive( &Archives[i] );
	}
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------

CArchivedFileManager::~CArchivedFileManager()
{
	CloseAllArchives();

	// free heap
#ifdef USE_FILE_HEAP
	MM_FREE( m_heap3 );
	MM_FREE( m_heap2 );
#endif
	MM_FREE( m_heap1 );

	// delete the mapping table we had to keep for the strings (so that we could avoid all the small NEWs)
//#ifdef __BREW__
	int	i;
	for (i = 0; i < ARCHIVE_NB; i++)
	{
		if (Archives[i].cMappingTable)
			DELETE_ARRAY Archives[i].cMappingTable;
	}
}


//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------


void CArchivedFileManager::CreateFileMappingTable( int archiveNb )
{
	ArchiveData*	archive = Archives + archiveNb;

	memset( archive, sizeof(ArchiveData), 0 );

	// Open the archive file containing the mapping table and the data file

	debug_out("CreateFileMappingTable for %s\n", ArchiveFileNames[archiveNb]);

    A_IFile* pMappingTable = A_IFile::Open(ArchiveFileNames[archiveNb], A_IFile::OPEN_READ | A_IFile::OPEN_BINARY | A_IFile::OPEN_STANDALONE, false);

	if (!pMappingTable)
	{
		debug_out("Error, file not loaded\n");
        return;
	}

	// Security for Aborded downloads ! (check too short size)
	if (pMappingTable->GetSize() < 1024)
	{
		A_IFile::Close(pMappingTable);
		pMappingTable = NULL;
		return;
	}

	// get mapping data size
	int iDataSize = archive->iMapFileSize = pMappingTable->GetInt();

	// compute position of datafile inside the archive
	archive->iDataFileOffset = archive->iMapFileSize + 2*sizeof(int);


	// get data file size
	archive->iDataFileSize = pMappingTable->GetInt();


    // Get all the data
	archive->cMappingTable = NEW char[iDataSize];
    pMappingTable->Read(archive->cMappingTable, iDataSize);


    // Close the file
    A_IFile::Close(pMappingTable);

    // Parse the table and fill the map

    char* cCurrentString;
    char* cMappingTableOffset   = archive->cMappingTable;


    unsigned long ulCurrentOffset;
    unsigned long ulCurrentSize;
	bool          bDataCompressed;
	bool          bDummy;


    while(cMappingTableOffset < archive->cMappingTable + iDataSize)
    {
	
		// Get the current file name

		// BREW set the string to point to the mappingtabledata (we keep the data in memory!)
//#ifdef __BREW__
        cCurrentString = cMappingTableOffset;//NEW char[strlen(cMappingTableOffset) + 1];
//#else
//        cCurrentString = NEW char[strlen(cMappingTableOffset) + 1];
//#endif
		strcpy(cCurrentString, cMappingTableOffset);
        StringToLower(cCurrentString);
		
		//debug_out("file in archive: %s", cCurrentString);


        // Get the current offset

        ulCurrentOffset = RetrieveFileOffset(cMappingTableOffset, strlen(cMappingTableOffset) + 1, &bDataCompressed);

        if(cMappingTableOffset + strlen(cMappingTableOffset) + 1 + OffsetByteNumber < archive->cMappingTable + iDataSize)
        {
            // If the file is not the last one, his size is: NextFileOffset - FileOffset

            char*   cNextFileName   = cMappingTableOffset + strlen(cMappingTableOffset) + 1 + OffsetByteNumber;
            ulCurrentSize           = RetrieveFileOffset(cNextFileName, strlen(cNextFileName) + 1, &bDummy) - ulCurrentOffset;
        }
        else
        {
            // If the file is the last one, his size is: TotalSizeOfTheArchive - FileOffset

            //ulCurrentSize = iDataSize - ulCurrentOffset; NO !!! we don't want the size of the offset file, but the size of the data file !!!
			ulCurrentSize = archive->iDataFileSize - ulCurrentOffset;
        }

        SFileProperties pFileProperties;
        pFileProperties.ulOffset      = ulCurrentOffset;
        pFileProperties.ulSize        = ulCurrentSize;
		pFileProperties.bIsCompressed = bDataCompressed;

        archive->pFileNameMappingTable[cCurrentString] = pFileProperties;


        // Go to the next file name (skip 4 byte for the offset + 1 byte for '\0'

        cMappingTableOffset += strlen(cMappingTableOffset) + 1 + OffsetByteNumber;
    }

	// BREW DO not delete the mapping table, we use all the strings that are inside (will be deleted in the destructor)

#ifdef __BREW__
//	DELETE_ARRAY Archives[archiveNb].cMappingTable;
//	Archives[archiveNb].cMappingTable = NULL;
#else
//    DELETE_ARRAY cMappingTable;
#endif
}


//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------


unsigned int CArchivedFileManager::RetrieveFileOffset(char* cBuffer, int iOffset, bool* dataCompressed)
{	
    unsigned int uiFileOffset;

    uiFileOffset  = ((unsigned char)(*(cBuffer + iOffset)))      << 24;
    uiFileOffset |= ((unsigned char)(*(cBuffer + iOffset + 1)))  << 16;
    uiFileOffset |= ((unsigned char)(*(cBuffer + iOffset + 2)))  <<  8;
    uiFileOffset |= ((unsigned char)(*(cBuffer + iOffset + 3)));

	// check for compression flag
	if((uiFileOffset & 0x80000000) == 0x80000000)
	{
		*dataCompressed = true;

		//remove the compression flag to retrieve the real offset
		uiFileOffset = uiFileOffset & 0x7fffffff;
	}
	else
	{
		*dataCompressed = false;
	}

    return uiFileOffset;

}


//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------


unsigned long CArchivedFileManager::GetFileIndex(const char* cFileName)
{
	char* cString;
    cString = NEW char[strlen(cFileName) + 1];
    strcpy(cString, cFileName);

    StringToLower(cString);

	int	i = ARCHIVE_NB;
	CurrentArchive = Archives;

	while (i--)
	{
		std::map<char*, SFileProperties, ltstr>::iterator it = CurrentArchive->pFileNameMappingTable.find(cString);

		if (it != CurrentArchive->pFileNameMappingTable.end())
		{
			// File found !
			DELETE_ARRAY cString;
			return it->second.ulOffset;
		}

		CurrentArchive++;
	}

//if (debug)
//DBGPRINTF("NOTfound arch" );

	CurrentArchive = NULL;

	DELETE_ARRAY cString;

	// file not found
    return -1;
}


//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------


unsigned long CArchivedFileManager::GetFileSize(const char* cFileName)
{
	char* cString;
    cString = NEW char[strlen(cFileName) + 1];
    strcpy(cString, cFileName);


    StringToLower(cString);

	int	i = ARCHIVE_NB;
	CurrentArchive = Archives;

	while (i--)
	{
		{
			std::map<char*, SFileProperties, ltstr>::iterator it = CurrentArchive->pFileNameMappingTable.find(cString);

			if (it != CurrentArchive->pFileNameMappingTable.end())
			{
				// File found !
				DELETE_ARRAY cString;
				return it->second.ulSize;
			}
		}

		CurrentArchive++;
	}

	CurrentArchive = NULL;

	DELETE_ARRAY cString;

	// file not found
    return -1;
}

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------


bool CArchivedFileManager::GetFileCompression(const char* cFileName)
{
	char* cString;
    cString = NEW char[strlen(cFileName) + 1];
    strcpy(cString, cFileName);


    StringToLower(cString);

	int	i = ARCHIVE_NB;
	CurrentArchive = Archives;

	while (i--)
	{
		{
			std::map<char*, SFileProperties, ltstr>::iterator it = CurrentArchive->pFileNameMappingTable.find(cString);

			if (it != CurrentArchive->pFileNameMappingTable.end())
			{
				// File found !
				DELETE_ARRAY cString;
				return it->second.bIsCompressed;
			}
		}

		CurrentArchive++;
	}

	CurrentArchive = NULL;
	DELETE_ARRAY cString;

	// file not found
    return false;
}


//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------


void CArchivedFileManager::StringToLower(char* cString)
{
    for(unsigned int i = 0; i < strlen(cString); i++)
    {
        cString[i] = tolower(cString[i]);
    }
}


//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------


/*A_IFile* CArchivedFileManager::OpenECPFile(ECPFileId fileId)
{
	TRACE("OpenECPFile ");
	TRACE(kECPFilesNames[fileId]);
	TRACE("\n");

	return STATIC_NEW  ECPFile(m_ecpFileData[fileId]);
}*/


