// DnlTool.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DnlTool.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// The one and only application object

CWinApp theApp;

using namespace std;
void GenerateDownloadParam();


CString g_ConfigFile;
CString g_OutputFile;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		cerr << _T("Fatal Error: MFC initialization failed") << endl;
		nRetCode = 1;
	}
	else
	{
		if(argc == 3)
		{
			g_ConfigFile = argv[1];
			g_OutputFile = argv[2];
			GenerateDownloadParam();
		}
		else
		{
			cerr << "Usage wrong!" << endl;
		}
	}

	return nRetCode;
}
void GenerateDownloadParam()
{


	
	CFile Outputfile;
	ifstream CfgFile(g_ConfigFile);
	if(!CfgFile)
	{
		cerr << g_ConfigFile << "is not found!" << endl;
		return;
	}
	if(!Outputfile.Open(g_OutputFile, CFile::modeCreate|CFile::modeWrite))
	{
		cerr << g_OutputFile << "---When opening , failed!" << endl;
		Outputfile.Close();
	}
	
	//GGI
	int GGI = 0;
	CfgFile >> GGI;
	Outputfile.Write(&GGI, sizeof(int));

	//url
	string url;
	CfgFile >> url;
	int len = url.length();
	Outputfile.Write(&len, sizeof(int));
	Outputfile.Write(url.c_str(), len);


	string path;
	CfgFile >> path;
	
	//n level num
	int nLevelNum = 0;
	CfgFile >> nLevelNum;
	Outputfile.Write(&nLevelNum, sizeof(int));

	for(int i = 0 ; i < nLevelNum ; ++i)
	{
		string filename;
		string id;
		int size = 0;
		int status = 0;
		CfgFile >> filename >> id >> status;

		//filename
		int len = filename.length();
		Outputfile.Write(&len, sizeof(int));
		Outputfile.Write(filename.c_str(), len);

		//menu_id
		len = id.length();
		Outputfile.Write(&len, sizeof(int));
		Outputfile.Write(id.c_str(), len);

		//length
		CFileFind finderLevel;
		string pathfile = path+"\\"+filename;
		BOOL bWorking=finderLevel.FindFile(pathfile.c_str());
		if(bWorking)
		{ 
			finderLevel.FindNextFile();
			size = finderLevel.GetLength();
		}
		Outputfile.Write(&size, sizeof(int));

		//status
		Outputfile.Write(&status, sizeof(int));
	}
	
	CfgFile.close();
	Outputfile.Close();

}
