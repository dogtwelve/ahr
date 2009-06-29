@echo off

call Version_SetVar

echo ----------------------------------------------------

echo Get...
call SS_DoGet.bat

echo Checkouts...
call SS_DoCheckouts.bat

echo Building Alice...
call Version_Build_Data_Tools

echo Building win version...
call Version_Build_Win

echo Checkins...
call SS_DoCheckins.bat

echo ----------------------------------------------------