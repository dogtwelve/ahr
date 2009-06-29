@echo on

call setvar.bat

echo %SCREEN%

rem cd %~dp0sprites\%SCREEN%\
cd %DATADIR%\Sprite

if "%USE_SINGLE_IMAGE_SPRITES%"=="1" (
	cd single_image_sprites
)

call export.cmd

rem cd %~dp0
