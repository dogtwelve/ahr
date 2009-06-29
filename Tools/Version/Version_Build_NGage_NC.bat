call Version_SetVar

set NGAGE_PROJECT=%ROOT%\Src\NGage_Project

xcopy /s /y ..\..\Release\NGage ..\..\Release\NGage_NC

pushd %NGAGE_PROJECT%
call MAKE_BLDFILES_S60.bat
call MAKE_PHONE_S60.bat
popd
