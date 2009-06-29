call Version_SetVar

pushd %TOOLDIR%\Alice
call msdev Alice.dsp /MAKE "Alice - Win32 Release" /REBUILD
popd
