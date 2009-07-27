@echo off 
rem ///////////////////////////////////
rem 	SET PROJECT CONFIGURATION
rem //////////////////////////////////

call setenv.bat

rem ---------------------------------------------
rem               LANGUAGE 
rem ---------------------------------------------

rem  VERSION_LANGUAGE = EN, DE, FR, IT, SP, US, ML( multi language )
rem NOTE: case sensitive
set ALL_LANGUAGES=EN;DE;FR;IT;SP;JP
set VERSION_LANGUAGE=ML


rem ---------------------------------------------
rem              BUILD VERSION
rem ---------------------------------------------

rem Digits number: 1
set VERSION_MAJOR=0
set VERSION_MINOR=0
set VERSION_BUILD=1
set BUILD_VERSION=%VERSION_MAJOR%%VERSION_MINOR%%VERSION_BUILD%

rem ---------------------------------------------
rem              PHONE
rem ---------------------------------------------

set PLATFORM=iphone
set PHONE=iphone
set SCREEN=320x480
set PACKAGE=high

rem set PLATFORM=_%PLATFORM%

rem ---------------------------------------------
rem              VERSION
rem ---------------------------------------------

call GenerateVersionH.bat

rem ---------------------------------------------
rem              RELEASE FOLDER
rem ---------------------------------------------

rem set BUILD_DIR=%PROJECT_PATH%build\%PLATFORM%\Win32
set BUILD_DIR=%PROJECT_PATH%\build\Win32
set RELEASE_DIR=%PROJECT_PATH%build\%PLATFORM%\Release


:END

set LINE_BREAK=echo *****************************************************************************

%LINE_BREAK%
echo Version set to %VERSION_MAJOR%.%VERSION_MINOR%.%VERSION_BUILD%
%LINE_BREAK%
