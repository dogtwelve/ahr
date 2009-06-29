
call Version_SetVar

set RELEASE_NOTE_FILEPATH=%ROOT%\Release\ReleaseNote.txt

pushd %ROOT%

	if exist %TOOLDIR%\Version\history.txt del %TOOLDIR%\Version\history.txt
	ss History %PROJECT_SS_PATH% -VL"%VERSION%"~L"%LAST_LABELED_VERSION%" -R -L- -O@%TOOLDIR%\Version\history.txt

	pushd %TOOLDIR%\Version

		if exist feature.txt del feature.txt
		if exist bugfix.txt del bugfix.txt
		call History_Parse history.txt feature.txt bugfix.txt
		if not exist feature.txt ( echo No new features  > feature.txt )
		if not exist bugfix.txt ( echo No bugs were fixed > bugfix.txt )

		call RemoveDup feature.txt
		call RemoveDup bugfix.txt

		echo ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~> version.header.txt
		echo Version %VERSION%>> version.header.txt
		echo ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~>> version.header.txt
		copy %RELEASE_NOTE_FILEPATH% %RELEASE_NOTE_FILEPATH%.old /Y
		copy version.header.txt+feature.header.txt+feature.txt+bugfix.header.txt+bugfix.txt+%RELEASE_NOTE_FILEPATH%.old %RELEASE_NOTE_FILEPATH% /Y
		del %RELEASE_NOTE_FILEPATH%.old

	popd

popd
