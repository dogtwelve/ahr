
pushd ..\..
	set ROOT=%CD%
popd
set TOOLDIR=%ROOT%\tools

rem Need to have Sourcesafe (ss.exe) in PATH
set SSEXEC=ss
set SSDATABASE=\\drop.mtl.ubisoft.org\vss\Gameloft\VSSGL-Dev
set SSDIR=%SSDATABASE%

set PROJECT_NAME=Asphalt3
set PROJECT_PHASE=Beta
set PROJECT_SS_PATH=$/Games/Asphalt1.1
set VERSION_FILENAME=%PROJECT_NAME%.ver
set VERSION_FILEPATH=%ROOT%\%VERSION_FILENAME%

set /P VERSION= < %VERSION_FILEPATH%
set VERSION=%VERSION: =%
