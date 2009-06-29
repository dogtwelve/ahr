
call Version_SetVar
call Labels_GetLatest LAST_LABELED_VERSION

set FAILURE_REASON=OK
set TRACEFILE=%ROOT%\Tools\Version\Trace.txt

echo ---------------------------------------------------- > %TRACEFILE%

	echo Get >> %TRACEFILE%
	call SS_DoGet.bat

	echo Checkout >> %TRACEFILE%
	call SS_DoCheckouts.bat

	echo Increment for release >> %TRACEFILE%
	call Version_Increment
	call Version_Update_VersionH
	
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

	echo Labeling the version - needed for history in release note >> %TRACEFILE%
	pushd %ROOT%
	ss Label -I- %PROJECT_SS_PATH% -L"%VERSION%"
	popd

	echo Update Release note >> %TRACEFILE%
	call Version_Update_ReleaseNote

	echo Zip >> %TRACEFILE%
	if not "%FAILURE_REASON:OK=%" == "" (
		set FAILURE_REASON={Failed%FAILURE_REASON:OK=%}
	) else (
		set FAILURE_REASON=%FAILURE_REASON:OK=%
	)
	call Version_Zip %FAILURE_REASON%

	echo Increment for debug >> %TRACEFILE%
	call Version_Increment
	call Version_Update_VersionH

	echo Checkins >> %TRACEFILE%
	call SS_DoCheckins.bat

echo ---------------------------------------------------- >> %TRACEFILE%