
call cygwin_config.bat

%CYGWIN_DRIVE%
chdir %CYGWIN_PATH%\bin

ECHO %HOME_DIR%

bash --login -i


pause