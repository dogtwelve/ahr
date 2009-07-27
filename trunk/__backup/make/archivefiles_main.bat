@echo off

echo Archiving...

call setvar.bat

rem ------------------------------
rem  Copy the tool in the bin dir
rem ------------------------------

echo on

if "%USE_LZMA%"=="1" (
	copy %TOOLDIR%\fileArchiverLZMA.exe %PROJECT_PATH%\build\fileArchiver.exe >> NUL
	copy %TOOLDIR%\lzma.exe %PROJECT_PATH%\build\lzma.exe >> NUL
) else (
	copy %TOOLDIR%\fileArchiver.exe %PROJECT_PATH%\build\fileArchiver.exe >> NUL
)

copy %TOOLDIR%\zlib1.dll %PROJECT_PATH%\build\zlib1.dll >> NUL


rem ------------------------------
rem      clear archive files
rem ------------------------------

cd %PROJECT_PATH%\build

fileArchiver -clear


rem ------------------------------
rem       archive all file
rem ------------------------------

if exist ..\archive_main.txt del ..\archive_main.txt /F

rem call filearchiver -addall Game >>..\archive_main.txt				2>NUL
call filearchiver -addall Textures >>..\archive_main.txt			2>NUL
rem call filearchiver -addall Textures\interf >>..\archive_main.txt			2>NUL
rem call filearchiver -addall Textures\interf\RaceTxt >>..\archive_main.txt		2>NUL
rem call filearchiver -addall Textures\interf\hud >>..\archive_main.txt		2>NUL

rem copy game\game.cfg game\end.end
rem echo empty >game\end.end

rem call filearchiver -add game\end.end -c >>..\archive_main.txt			2>NUL
rem del game\end.end

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


rem --------------------------
rem     ECP
rem --------------------------
rem call make_ecp.bat

del %PROJECT_PATH%\build\main.bar
ren %PROJECT_PATH%\build\main.arc main.bar

if not "%BUILD_DIR%"=="" (
	copy %PROJECT_PATH%\build\main.bar	%BUILD_DIR%
)


