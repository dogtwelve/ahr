@echo off
rem **************************************************************
rem 					set project path
rem **************************************************************
rem %~dp0 returns the directory where the bat file is placed 
set PROJECT_PATH=%~dp0..\..\
echo PROJECT_PATH=%PROJECT_PATH%

rem **************************************************************
rem 				Java
rem **************************************************************
rem We need this to build the alpha pngs
set JAVA_PATH="c:\Program Files\java\jdk1.6.0_07"
set JAVA_EXE=%JAVA_PATH%\bin\java.exe
echo JAVA_EXE=%JAVA_EXE%

