call Version_SetVar

set NGAGE_PROJECT=%ROOT%\Src\NGage_Project

pushd %NGAGE_PROJECT%
call MAKE_BLDFILES.BAT
call MAKE_PHONE_NGAGE.BAT
popd
