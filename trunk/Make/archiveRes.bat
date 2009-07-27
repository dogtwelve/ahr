@echo off

echo Archiving...

call setenv.bat

rem ------------------------------
rem  Copy the tool in the bin dir
rem ------------------------------

echo on

if "%USE_LZMA%"=="1" (
	copy %TOOLDIR%\fileArchiverLZMA.exe %PRJ_BUILD_OUTPUT_DIR%\fileArchiver.exe >> NUL
	copy %TOOLDIR%\lzma.exe %PRJ_BUILD_OUTPUT_DIR%\lzma.exe >> NUL
) else (
	copy %TOOLDIR%\fileArchiver.exe %PRJ_BUILD_OUTPUT_DIR%\fileArchiver.exe >> NUL
)

copy %TOOLDIR%\zlib1.dll %PRJ_BUILD_OUTPUT_DIR%\zlib1.dll >> NUL


rem ------------------------------
rem      clear archive files
rem ------------------------------

cd %PRJ_BUILD_OUTPUT_DIR%
fileArchiver -clear


rem ------------------------------
rem       archive all file
rem ------------------------------

if exist ..\archive_main.txt del ..\archive_main.txt /F
call filearchiver -addall sprite >>..\archive_main.txt			2>NUL
rem call filearchiver -addall Textures\interf >>..\archive_main.txt			2>NUL


rem ------------------------------
rem       Delete the tool
rem ------------------------------

del /f /q fileArchiver.exe
del /f /q zlib1.dll

%TOOLDIR%\archivemerge 6rbc main

del /F /q 6rbc.dat
del /F /q 6rbc.off

rem --------------------------
rem     DELETE DATA
rem --------------------------

cd ..


del %PRJ_BUILD_OUTPUT_DIR%\main.bar
ren %PRJ_BUILD_OUTPUT_DIR%\main.arc main.bar


copy %PRJ_BUILD_OUTPUT_DIR%\main.bar	%VC_DEBUG_DIR%
copy %PRJ_BUILD_OUTPUT_DIR%\main.bar	%WIN32_EXE_RELEASE_DIR%
copy %PRJ_BUILD_OUTPUT_DIR%\main.bar	%IPHONE_PRJ_DIR%

