@echo off

rem ------------------------
rem CREATE DIRECTORIES
rem ------------------------

call setvar.bat

set DIRS=.\bin
set DIRS=%DIRS% .\bin\Cars 
set DIRS=%DIRS% .\bin\Sounds 
set DIRS=%DIRS% .\bin\Game 
set DIRS=%DIRS% .\bin\Streams
set DIRS=%DIRS% .\bin\Textures 
set DIRS=%DIRS% .\bin\Tracks
set DIRS=%DIRS% .\bin\Textures\interf 
set DIRS=%DIRS% .\bin\Textures\interf\RaceTxt
set DIRS=%DIRS% .\bin\Textures\interf\hud
set DIRS=%DIRS% .\bin\Video


for %%i in ( %DIRS% ) do if not exist %%i mkdir %%i

rem cd..
rem echo on

pause

call %~dp0makeSprites.bat
cd %~dp0

goto ENDOFMAKEDATA

rem ------------------------
rem LANGUAGES
rem ------------------------

echo *** Make Languages

call makeLanguages.bat

rem ------------------------
rem GAME CONFIG
rem ------------------------

echo *** Make Config

call makeGameConfig.bat

rem ------------------------
rem Animated objects
rem ------------------------

call makeobjects.bat

rem ------------------------
rem TRACKS
rem ------------------------

echo *** Make Tracks

call makeTracks

rem ------------------------
rem TEXTURES
rem ------------------------

call makeTexture.bat



rem ------------------------
rem CARS
rem ------------------------

echo *** Make Cars

call makeCars.bat

rem ------------------------
rem SOUND
rem ------------------------

echo *** Make Sounds

call makeSounds.bat

rem %TOOLDIR%\musiccfg .\music\music_sequence.txt .\bin\music\music_sequence.cfg



rem ------------------------
rem CAMERAS
rem ------------------------

echo *** Make Cameras
call makeCameras.bat


rem ------------------------
rem GLOBE
rem ------------------------

rem Copy globe to the 
rem xcopy ui\menu\globe.bin .\bin\ui\menu\ /Y


:ENDOFMAKEDATA
pause