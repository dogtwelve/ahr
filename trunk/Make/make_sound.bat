call setenv.bat

::WE ARE @ PRJ_BUILD_DIR


pushd %PRJ_BUILD_OUTPUT_DIR%

del bgm /Q/A
del sfx /Q/A

copy %PRJ_DATA_SFX_DIR% sfx
copy %PRJ_DATA_BGM_DIR% bgm
pause
