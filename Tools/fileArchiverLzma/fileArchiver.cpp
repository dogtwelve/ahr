#include <deque>
#include <io.h>
#include <iostream>
#include <fstream>
#include <SYS\STAT.H>
#include <windows.h>

#include <stdio.h>
#include <conio.h>
#include <process.h>

#include "fileArchiver.h"
#include "zlib.h"

#define OffsetFileName  "6RBC.off"
#define DataFileName    "6RBC.dat"


#define USE_LZMA_COMPRESSION	0

using namespace std;

//---------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------


void main(int iArgCount, char** cArgList)
{
    if(iArgCount < 2)
    {
        PrintUsage();
    }
   
    bool bClearDataBeforeArchiving = false;
    std::deque<ArchiveFileStruct*> pFileDeque;
    

    for(int iCurrentArg = 1; iCurrentArg < iArgCount; iCurrentArg++)
    {
        char* cArg = cArgList[iCurrentArg];

        if(!strcmp(cArg, "-clear"))
        {
            bClearDataBeforeArchiving = true;
        }
        else if(!strcmp(cArg, "-add"))
        {
            if(iCurrentArg + 1 >= iArgCount)
            {
                PrintUsage();
            }

            cArg = cArgList[++iCurrentArg];

            if(strchr(cArg, '*') != NULL)
            {
                cout << "patern not implemanted yet!";
                exit(0);
            }
            else
            {
                ArchiveFileStruct* pArchive = new ArchiveFileStruct;

                pArchive->cFileName = cArg;

                if(iCurrentArg + 1 < iArgCount && !strcmp(cArgList[iCurrentArg + 1], "-c"))
                {
                    pArchive->bMustBeCompressed = true;
                    iCurrentArg++;
                }
                else
                {
                    pArchive->bMustBeCompressed = false;
                }

                pFileDeque.push_back(pArchive);
            }
        }
        else if(!strcmp(cArg, "-addall"))
        {
			HANDLE	ff;
			WIN32_FIND_DATA		fd;

			char	txt[256];
			strcpy( txt, cArgList[iCurrentArg+1] );
			strcat( txt, "\\*.*" );
			ff = FindFirstFile( txt, &fd );

			if (ff == INVALID_HANDLE_VALUE)
			{
				printf("No file FOUND\n");
				return;
			}

loop1:
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				char*	namein = (char*)malloc(128);

				strcpy( namein, cArgList[iCurrentArg+1] );
				strcat( namein, "\\" );
				strcat( namein, fd.cFileName );

                ArchiveFileStruct* pArchive = new ArchiveFileStruct;
                pArchive->cFileName = namein;
				pArchive->bMustBeCompressed = true;
                pFileDeque.push_back(pArchive);
			}

			if (FindNextFile( ff, &fd ))
				goto loop1;

			break;
		}
        else if(!strcmp(cArg, "-addallnocomp"))
        {
			HANDLE	ff;
			WIN32_FIND_DATA		fd;

			char	txt[256];
			strcpy( txt, cArgList[iCurrentArg+1] );
			strcat( txt, "\\*.*" );
			ff = FindFirstFile( txt, &fd );

			if (ff == INVALID_HANDLE_VALUE)
			{
				printf("No file FOUND\n");
				return;
			}

loop2:
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				char*	namein = (char*)malloc(128);

				strcpy( namein, cArgList[iCurrentArg+1] );
				strcat( namein, "\\" );
				strcat( namein, fd.cFileName );

                ArchiveFileStruct* pArchive = new ArchiveFileStruct;
                pArchive->cFileName = namein;
				pArchive->bMustBeCompressed = false;
                pFileDeque.push_back(pArchive);
			}

			if (FindNextFile( ff, &fd ))
				goto loop2;

			break;
		}
        else
        {
            PrintUsage();
        }
    }

    if(bClearDataBeforeArchiving)
    {
        ClearDataFiles();
    }

    if(!pFileDeque.empty())
    {
        ArchiveFiles(&pFileDeque);
    }
}


//---------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------


void PrintUsage()
{
    cout << "Usage" << endl;
    exit(0);
}


//---------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------


void ClearDataFiles()
{
    // In case the file were getted from Source Safe and are read-only

    _chmod(OffsetFileName, _S_IREAD | _S_IWRITE);
    _chmod(DataFileName,   _S_IREAD | _S_IWRITE);


    remove(OffsetFileName);
    remove(DataFileName);
}


//---------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------


void ArchiveFiles(std::deque<ArchiveFileStruct*>* pFileDeque)
{
    // In case the file were getted from Source Safe and are read-only

    _chmod(OffsetFileName, _S_IREAD | _S_IWRITE);
    _chmod(DataFileName,   _S_IREAD | _S_IWRITE);




    while(!pFileDeque->empty())
    {
    fstream pOffsetFile(OffsetFileName, ios::binary | ios::out | ios::app);
    fstream pDataFile  (DataFileName,   ios::binary | ios::out | ios::app);
   
    ifstream pCurrentArchiveFile;

        ArchiveFileStruct* pCurrentArchiveStruct = pFileDeque->front();
        pFileDeque->pop_front();
        
        pCurrentArchiveFile.open(pCurrentArchiveStruct->cFileName, ios::binary | ios::in/* | ios::nocreate*/);


        // If the file is not found, process the next one

        if(pCurrentArchiveFile.fail())
        {
            pCurrentArchiveFile.clear();

            cout << endl << "Error processing file: " << pCurrentArchiveStruct->cFileName << endl;
            cout << "Skipping..." << endl << endl;

            delete pCurrentArchiveStruct;
            continue;
        }

        pDataFile.seekp(0, ios::end);
        unsigned int uiDataFileOffset = pDataFile.tellp();

		cout << "archiving: " << pCurrentArchiveStruct->cFileName << " at: " << uiDataFileOffset << endl;




        // Get the length of the input file

        pCurrentArchiveFile.seekg(0, ios::end);
        int iInputFileLength = pCurrentArchiveFile.tellg();
        pCurrentArchiveFile.seekg(0, ios::beg);


        // Get the input file data and write it to the data file

        char* cInputFileData = new char[iInputFileLength];
		char* cOuputFileData = new char[iInputFileLength*2];
		unsigned long comprLen = iInputFileLength*2;

        pCurrentArchiveFile.read(cInputFileData, iInputFileLength);

#ifdef USE_LZMA_COMPRESSION
		pCurrentArchiveFile.close();
#endif



		
		if(pCurrentArchiveStruct->bMustBeCompressed == true)
		{
			cout << "compressing: " << pCurrentArchiveStruct->cFileName;


#ifndef USE_LZMA_COMPRESSION
			int err = compress(cOuputFileData, &comprLen, cInputFileData, iInputFileLength);
#else
			char temp [256];
			char resultfile[256];

			strcpy(temp,"e ");
			strcat(temp, pCurrentArchiveStruct->cFileName);
			strcat(temp, " " );

			strcpy(resultfile, pCurrentArchiveStruct->cFileName);
			strcat(resultfile, ".lzma");
			
			strcat(temp, resultfile);
			
			//ShellExecute(NULL,"open","lzma.exe", temp, NULL, SW_NORMAL);
			_spawnl( _P_WAIT, "lzma.exe", "lzma.exe", temp, NULL );

			pCurrentArchiveFile.open(resultfile, ios::binary | ios::in/* | ios::nocreate*/);

			if( !pCurrentArchiveFile.is_open() )
			{
				MessageBox( 0, temp, "error", MB_OK );
			}
			//while(!pCurrentArchiveFile.is_open())
			//{
				//printf("no file");
			//	pCurrentArchiveFile.open(resultfile, ios::binary | ios::in | ios::nocreate);
			//}
			// Get the length of the input file
			pCurrentArchiveFile.seekg(0, ios::end);
			comprLen = pCurrentArchiveFile.tellg();
			pCurrentArchiveFile.seekg(0, ios::beg);

			pCurrentArchiveFile.read(cOuputFileData, comprLen);
			
			pCurrentArchiveFile.close();

			remove(resultfile);
#endif

#ifndef USE_LZMA_COMPRESSION
		
			if(err != 0)
			{
				cout << "****************COMPRESSION ERROR***************" << endl;
				cout << zError( err) << endl;

			}
#endif


			cout << "in:" << iInputFileLength << " / out:" << comprLen << " = " << ((100.0f * comprLen) / iInputFileLength) << "%";

			if (comprLen >= (7*iInputFileLength/8))
			{
				// not compressed enough, just store
				pDataFile.write         (cInputFileData, iInputFileLength);
				cout << " *** stored ***" << endl;
				pCurrentArchiveStruct->bMustBeCompressed = false;
			}
			else
			{
				pDataFile.put((unsigned char)((iInputFileLength >> 24) & 0xff));
				pDataFile.put((unsigned char)((iInputFileLength >> 16) & 0xff));
				pDataFile.put((unsigned char)((iInputFileLength >>  8) & 0xff));
				pDataFile.put((unsigned char)((iInputFileLength >>  0) & 0xff));

				pDataFile.write         (cOuputFileData, comprLen);
				cout << endl;
			}
		}
		else
		{
			pDataFile.write         (cInputFileData, iInputFileLength);
		}


        // Print the file name and offset in the Offset file

        pOffsetFile << pCurrentArchiveStruct->cFileName;
        pOffsetFile.put('\0');
        
		if(pCurrentArchiveStruct->bMustBeCompressed == true)
		{
			pOffsetFile.put((unsigned char)((uiDataFileOffset >> 24) & 0xff | 0x80));
			pOffsetFile.put((unsigned char)((uiDataFileOffset >> 16) & 0xff));
			pOffsetFile.put((unsigned char)((uiDataFileOffset >>  8) & 0xff));
			pOffsetFile.put((unsigned char)((uiDataFileOffset >>  0) & 0xff));
		}
		else
		{
			pOffsetFile.put((unsigned char)((uiDataFileOffset >> 24) & 0xff));
			pOffsetFile.put((unsigned char)((uiDataFileOffset >> 16) & 0xff));
			pOffsetFile.put((unsigned char)((uiDataFileOffset >>  8) & 0xff));
			pOffsetFile.put((unsigned char)((uiDataFileOffset >>  0) & 0xff));
		}

#ifndef USE_LZMA_COMPRESSION
        pCurrentArchiveFile.close();
#endif

        delete pCurrentArchiveStruct;

	pOffsetFile.close();
    pDataFile.close();
    }

}


//---------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------
