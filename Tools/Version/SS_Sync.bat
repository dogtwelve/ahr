
call Version_SetVar

if defined DISABLE_SS ( goto SS_Sync_end )

pushd %ROOT%

if "%1" == "get" (
	ss Get -I- -O- %PROJECT_SS_PATH% -R
) else (

	ss %1 -I- -O- %PROJECT_SS_PATH%/%VERSION_FILENAME%

	pushd %ROOT%\Src
	ss %1 -I- -O- %PROJECT_SS_PATH%/Src/Version.h
	popd

	pushd %ROOT%\Release
	ss %1 -I- -O- %PROJECT_SS_PATH%/Release/ReleaseNote.txt
	popd

	pushd %ROOT%\Release\NGage
	ss %1 -I- -O- %PROJECT_SS_PATH%/Release/NGage/game.id
	popd

	pushd %ROOT%\Release\Windows
	ss %1 -I- -O- %PROJECT_SS_PATH%/Release/Windows/Asphalt3.exe
	popd
)

popd

:SS_Sync_end