@echo off
SET JDK_DIR=C:\j2sdk1.4.2_04

SET CURR_DIR=%CD%
SET SRC_DIR=src
SET BIN_DIR=bin
SET CLASSES_DIR=classes

SET CLASS_NAME=CMain
SET JAR_NAME=BuildBitmapFont

echo *** Make Manifest file
echo Main-Class: %CLASS_NAME%> MANIFEST.MF

echo *** Compile source code
IF NOT EXIST %CLASSES_DIR% ( MD %CLASSES_DIR% )
CD %SRC_DIR%
%JDK_DIR%\bin\javac *.java -d ..\%CLASSES_DIR%
CD..

echo *** Package JAR file
IF NOT EXIST %BIN_DIR% ( MD %BIN_DIR% )
CD %CLASSES_DIR%
%JDK_DIR%\bin\jar -cvfm ..\%BIN_DIR%\%JAR_NAME%.jar ..\MANIFEST.MF *.class > NUL
CD..

DEL /q MANIFEST.MF > NUL
RD /q /s %CLASSES_DIR% > NUL

copy %BIN_DIR%\%JAR_NAME%.jar ..\%JAR_NAME%.jar

echo *** WOW! Completed!

PAUSE