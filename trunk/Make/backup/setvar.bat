@echo off

if not "%SETVAR%"=="1" (
 	set SETVAR=1
) else (
rem  	goto end
)

set CURRENT_DIR=%cd%

if "%PROJECT_PATH%"=="" (
	echo **************************************************************
	echo ** Setting iPhone the default platform
	echo **************************************************************

	rem cd ..\src\_iphone
	call config.bat
)

rem cd %CURRENT_DIR%

rem ---------------------------------------------
rem              DATA CONFIG
rem ---------------------------------------------


set USE_PVRTC_TEXTURE_COMPRESS=0
set  USE_SINGLE_IMAGE_SPRITES=0
set                  USE_LZMA=0

set                   USE_MP3=0
set                  USE_MIDI=0
set                   USE_WAV=0
set                   USE_ZND=0
set 		USE_WAV_MUSIC=0
set               USE_ZND_NGI=0
set		     CAR_PACK=5Mb
set 	       DELETE_CHINESE=0
set		 STRINGS_FILE=A4_strings_iPhone.xml
set 	USE_IPHONE_TRACKS=0
set 	USE_BONUS_CARS=0
set 	USE_PVR_TEXTURES=0

if "%PLATFORM%"=="_symbian" (
	set USE_MIDI=1
	set USE_ZND=1
	set USE_LZMA=1

if "%PHONE%"=="SAMSUNG" (
	set USE_WAV_MUSIC=1
	set USE_MIDI=0
)
if "%PHONE%"=="Z8" (
	set USE_WAV_MUSIC=1
	set USE_MIDI=0
)
if "%PHONE%"=="6630_N70" (
	set USE_WAV_MUSIC=1
	set USE_MIDI=0
)
	set DELETE_CHINESE=1

	set STRINGS_FILE=A4_strings_Symbian_WM.xml
)

if "%PLATFORM%"=="_ngage" (
	set USE_MP3=1
	set USE_ZND_NGI=1
	set CAR_PACK=32Mb
	
	set STRINGS_FILE=A4_strings_nGage.xml
)

if "%PLATFORM%"=="_windowsMobile" (
	set USE_WAV=1
	set USE_LZMA=1
	set USE_WAV_MUSIC=1
	set DELETE_CHINESE=1
	
	set STRINGS_FILE=A4_strings_Symbian_WM.xml
)

if "%PLATFORM%"=="_iphone" (
	set USE_PVRTC_TEXTURE_COMPRESS=0
	set USE_SINGLE_IMAGE_SPRITES=1
	set USE_WAV=1
	set USE_M4A=1
	set CAR_PACK=32Mb
	set USE_IPHONE_TRACKS=1
	
	rem remove bonus cars for now...
	rem set USE_BONUS_CARS=1
	
	set USE_PVR_TEXTURES=1
	
	set STRINGS_FILE=A4_strings_iPhone.xml
)

rem ---------------------------------------------
rem              PATHS
rem ---------------------------------------------


SET   DATADIR=%PROJECT_PATH%\Data
SET   TOOLDIR=%PROJECT_PATH%\Tools
SET    CARDIR=%DATADIR%\Cars
SET AURORA_GT=%TOOLDIR%\AuroraGT\AuroraGT.exe



:end
