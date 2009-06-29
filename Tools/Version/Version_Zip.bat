call Version_SetVar

set DAY=%DATE:~4,2%
set MONTH=%DATE:~7,2%
set YEAR=%DATE:~10,4%
set ZIP_DATE=%YEAR%%MONTH%%DAY%
set ZIP_DATE=%ZIP_DATE:/=%
set ZIP_DATE=%ZIP_DATE: =%

set ZIP_TIME=%TIME:~0,8%
set ZIP_TIME=%ZIP_TIME::=%
set ZIP_TIME=%ZIP_TIME: =%

set RELEASE_DIR=%ROOT%
set ZIP_FILE_NAME=%PROJECT_NAME%-%PROJECT_PHASE%-v%VERSION%-%ZIP_DATE%-%ZIP_TIME%
if not "%1" == "" (
	set ZIP_FILE_NAME=%ZIP_FILE_NAME%-%1
)

set OUTPUTPATH=%CD%\Output

pushd %ROOT%
%TOOLDIR%\PKZipC.exe -Add -Recurse -Dir=Current %OUTPUTPATH%\%ZIP_FILE_NAME%.zip %ROOT%\Release\*.*
%TOOLDIR%\PKZipC.exe -Add -Dir=Current %OUTPUTPATH%\%ZIP_FILE_NAME%.zip %ROOT%\Data\bin\6RBC.*
%TOOLDIR%\PKZipC.exe -Add -Dir=Current %OUTPUTPATH%\%ZIP_FILE_NAME%.zip %ROOT%\Data\bin\Streams\*.*
%TOOLDIR%\PKZipC.exe -Del %OUTPUTPATH%\%ZIP_FILE_NAME%.zip *.scc
popd

pushd %ROOT%\Release\NGage
%TOOLDIR%\PKZipC.exe -Add -Recurse -Path=Relative %OUTPUTPATH%\%ZIP_FILE_NAME%-NGage.zip *.*
%TOOLDIR%\PKZipC.exe -Del %OUTPUTPATH%\%ZIP_FILE_NAME%-NGage.zip *.scc
popd

pushd %ROOT%\Release\NGage_NC
%TOOLDIR%\PKZipC.exe -Add -Recurse -Path=Relative %OUTPUTPATH%\%ZIP_FILE_NAME%-NGage_NC.zip *.*
%TOOLDIR%\PKZipC.exe -Del %OUTPUTPATH%\%ZIP_FILE_NAME%-NGage_NC.zip *.scc
popd

