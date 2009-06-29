REM Do a complete build without labeling or doing any checkouts/checkins

call Version_SetVar

set FAILURE_REASON=OK
set TRACEFILE=%ROOT%\Tools\Version\Trace.txt

echo ---------------------------------------------------- > %TRACEFILE%

	echo Get >> %TRACEFILE%
	call SS_DoGet.bat

	pushd %ROOT%\Release\Windows
	ss checkout -I- -O- %PROJECT_SS_PATH%/Release/Windows/Asphalt3.exe
	popd

	echo Build Data >> %TRACEFILE%
	call Version_Build_Data

	echo Build NGage >> %TRACEFILE%
	call Version_Build_NGage
	if not defined COMPILATION_SUCCESS set FAILURE_REASON=%FAILURE_REASON%-NGage

	REM NC version not built for now
	REM echo Build NGage_NC >> %TRACEFILE%
	REM call Version_Build_NGage_NC
	REM if not defined COMPILATION_SUCCESS set FAILURE_REASON=%FAILURE_REASON%-NGageNC

	echo Build Win >> %TRACEFILE%
	call Version_Build_Win
	if not defined COMPILATION_SUCCESS set FAILURE_REASON=%FAILURE_REASON%-Win
	
	pushd %ROOT%\Release\Windows
	ss checkin -I- -O- %PROJECT_SS_PATH%/Release/Windows/Asphalt3.exe
	popd

	echo Zip >> %TRACEFILE%
	if not "%FAILURE_REASON:OK=%" == "" (
		set FAILURE_REASON={Failed%FAILURE_REASON:OK=%}
	) else (
		set FAILURE_REASON=%FAILURE_REASON:OK=%
	)
	call Version_Zip %FAILURE_REASON%
echo -------