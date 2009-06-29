
set %1=

pushd %ROOT%

	if exist %TOOLDIR%\Version\labels.txt del %TOOLDIR%\Version\labels.txt
	ss History %PROJECT_SS_PATH% -L -O@%TOOLDIR%\Version\labels.txt

	pushd %TOOLDIR%\Version
		for /F "delims=" %%i in (labels.txt) do (
			set LINE=%%i
			call Labels_ParseLine %1
		)

	popd

popd
