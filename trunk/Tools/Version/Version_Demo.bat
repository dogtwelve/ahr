
call Version_SetVar
call Labels_GetLatest LAST_LABELED_VERSION

set FAILURE_REASON=OK
set TRACEFILE=%ROOT%\Tools\Version\Trace.txt

echo ---------------------------------------------------- > %TRACEFILE%

	echo Get >> %TRACEFILE%
	call SS_DoGet.bat

	echo Build Data >> %TRACEFILE%
	rename ..\..\Data Data_Full
	rename ..\..\Data_Demo Data
	call Version_Build_Data
	rename ..\..\Data Data_Demo
	rename ..\..\Data_Full Data

	echo #define DEMO_VERSION 1 > ..\..\Src\Config\Config.h

		echo Build NGage >> %TRACEFILE%
		call Version_Build_NGage
		if not defined COMPILATION_SUCCESS set FAILURE_REASON=%FAILURE_REASON%-NGage

		echo Build NGage_NC >> %TRACEFILE%
		call Version_Build_NGage_NC
		if not defined COMPILATION_SUCCESS set FAILURE_REASON=%FAILURE_REASON%-NGageNC

		echo Build Win >> %TRACEFILE%
		call Version_Build_Win
		if not defined COMPILATION_SUCCESS set FAILURE_REASON=%FAILURE_REASON%-Win

	echo // > ..\..\Src\Config\Config.h

	echo Zip >> %TRACEFILE%
	if not "%FAILURE_REASON:OK=%" == "" (
		set FAILURE_REASON={Failed%FAILURE_REASON:OK=%}
	) else (
		set FAILURE_REASON=%FAILURE_REASON:OK=%
	)
	call Version_Zip %FAILURE_REASON%

echo ---------------------------------------------------- >> %TRACEFILE%