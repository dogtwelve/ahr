# Microsoft Developer Studio Project File - Name="MergeCar" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=MergeCar - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MergeCar.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MergeCar.mak" CFG="MergeCar - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MergeCar - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "MergeCar - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Games/Asphalt2/Tools/MergeCar", EVITAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MergeCar - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\Unify" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "MergeCar - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\Unify" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "MergeCar - Win32 Release"
# Name "MergeCar - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\CarExporter.cpp
# End Source File
# Begin Source File

SOURCE=.\CarExporter.h
# End Source File
# Begin Source File

SOURCE=.\ImageProcess.cpp
# End Source File
# Begin Source File

SOURCE=.\ImageProcess.h
# End Source File
# Begin Source File

SOURCE=.\MergeCar.cpp
# End Source File
# Begin Source File

SOURCE=.\Surface.cpp
# End Source File
# Begin Source File

SOURCE=.\Surface.h
# End Source File
# Begin Source File

SOURCE=.\TextureWriter.cpp
# End Source File
# Begin Source File

SOURCE=.\TextureWriter.h
# End Source File
# Begin Source File

SOURCE=.\TgaReader.cpp
# End Source File
# Begin Source File

SOURCE=.\TgaReader.h
# End Source File
# Begin Source File

SOURCE=.\TgaWriter.cpp
# End Source File
# Begin Source File

SOURCE=.\TgaWriter.h
# End Source File
# Begin Source File

SOURCE=..\Unify\util.h
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Unify"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Unify\Unify.cpp
# End Source File
# Begin Source File

SOURCE=..\Unify\Unify.h
# End Source File
# Begin Source File

SOURCE=..\Unify\UnifyReader.cpp
# End Source File
# Begin Source File

SOURCE=..\Unify\UnifyReader.h
# End Source File
# Begin Source File

SOURCE=..\Unify\UnifyWriter.cpp
# End Source File
# Begin Source File

SOURCE=..\Unify\UnifyWriter.h
# End Source File
# End Group
# Begin Group "LibASE"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\LibASE\ASEException.cpp
# End Source File
# Begin Source File

SOURCE=..\LibASE\ASEException.h
# End Source File
# Begin Source File

SOURCE=..\LibASE\ASEMap.cpp
# End Source File
# Begin Source File

SOURCE=..\LibASE\ASEMap.h
# End Source File
# Begin Source File

SOURCE=..\LibASE\ASEMaterial.cpp
# End Source File
# Begin Source File

SOURCE=..\LibASE\ASEMaterial.h
# End Source File
# Begin Source File

SOURCE=..\LibASE\ASEMesh.cpp
# End Source File
# Begin Source File

SOURCE=..\LibASE\ASEMesh.h
# End Source File
# Begin Source File

SOURCE=..\LibASE\ASEParser.cpp
# End Source File
# Begin Source File

SOURCE=..\LibASE\ASEParser.h
# End Source File
# Begin Source File

SOURCE=..\LibASE\ASEReader.cpp
# End Source File
# Begin Source File

SOURCE=..\LibASE\ASEReader.h
# End Source File
# Begin Source File

SOURCE=..\LibASE\DisableStlWarnings.h
# End Source File
# End Group
# End Target
# End Project
