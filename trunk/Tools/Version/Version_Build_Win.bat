call Version_SetVar

set COMPILATION_SUCCESS=

REM set WINDOWS_PROJECT=%ROOT%\Src\Windows_Project

pushd %ROOT%
call "C:\Program Files\Microsoft Visual Studio .NET 2003\Common7\IDE\devenv.exe" %PROJECT_NAME%.sln /rebuild Release
if exist .\Release\Windows\%PROJECT_NAME%.exe (
	set COMPILATION_SUCCESS=1
	)

popd
