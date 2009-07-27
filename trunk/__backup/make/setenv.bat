@echo off
rem **************************************************************
rem 					set project path
rem **************************************************************
rem %~dp0 returns the directory where the bat file is placed 
set PROJECT_PATH=%CD%\..
echo PROJECT_PATH=%PROJECT_PATH%

rem **************************************************************
rem 				Java
rem **************************************************************
rem We need this to build the alpha pngs
rem 아래 세팅...
rem set JAVA_PATH="c:\Program Files\java\jdk1.6.0_07"
rem set JAVA_EXE=%JAVA_PATH%\bin\java.exe
rem echo JAVA_EXE=%JAVA_EXE%

