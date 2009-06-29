#include ".\inicontainer.h"
#include "stdio.h"
#include "memory.h"



#define LINE_BUFFER_SIZE	10000
#define READ_BUFFER_SIZE	100000

int CIniContainer::CountLines()
{
	FILE*	fin = fopen( m_fileName, "rt" );
	if (fin == NULL)
	{
		throw "File not found!";
	}

	__try
	{
		char buffer[READ_BUFFER_SIZE];
		//Counts the lines in the file
		int bytesRead=0;
		int linesCount = 0;
		do {
			bytesRead = fread(buffer,1,READ_BUFFER_SIZE,fin);

			for (int i = 0; i < bytesRead;i ++)
			{
				if (buffer[i] == (char) 0x0A) linesCount++;
			}
		} while (bytesRead == READ_BUFFER_SIZE);
		return linesCount;
	} 
	__finally {
		fclose(fin);
	}
}
void CIniContainer::LoadLines()
{
	printf("Loading file %s ...",m_fileName);
	int linesCount = CountLines();	
	m_lines = new char * [linesCount];	
	//Now load the lines
	FILE*	fin = fopen( m_fileName, "rt" );
	if (fin == NULL)
	{
		throw "File not found!";
	}

	__try
	{
		char lineBuffer[LINE_BUFFER_SIZE];
		int  currentPositionInLineBuffer = -1;
		int  currentLineIndex = -1;
		bool  newLineSequence = false;
		char buffer[READ_BUFFER_SIZE];
		//Counts the lines in the file
		int bytesRead=0;
		do {
			bytesRead = fread(buffer,1,READ_BUFFER_SIZE,fin);
			
			for (int i = 0; i < bytesRead;i ++)
			{				
				if (buffer[i] == (char) 0x0D) {
					continue;
				}
				if (buffer[i] == (char) 0x0A ){
					currentLineIndex++;
					if (currentLineIndex>=linesCount) throw "Line count greater than expected.\n";
					
					if ((currentPositionInLineBuffer != -1) )
					{
						//We have characters for the line
						m_lines[currentLineIndex]= new char[currentPositionInLineBuffer+2];						
						memcpy(m_lines[currentLineIndex],lineBuffer,currentPositionInLineBuffer+1);
					} else
					{
						//Empty line
						m_lines[currentLineIndex]=NULL;
					}
					continue;
				}

				currentPositionInLineBuffer++;
				if (currentPositionInLineBuffer>=LINE_BUFFER_SIZE) 
					throw "Maximum line size exceeded";
				lineBuffer[currentPositionInLineBuffer] = buffer[i];
			}
		} while (bytesRead == READ_BUFFER_SIZE);
	} 
	__finally {
		fclose(fin);
	}


}
CIniContainer::CIniContainer(const char * fileName):
m_fileName(fileName)
{
	LoadLines();
}

CIniContainer::~CIniContainer(void)
{
}
