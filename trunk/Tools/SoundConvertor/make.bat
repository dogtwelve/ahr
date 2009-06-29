@echo off

SET CURR_DIR=%~dp0
SET SRC_DIR=%CURR_DIR%\src
SET BIN_DIR=%CURR_DIR%\bin
SET CLASSES_DIR=%CURR_DIR%\classes

SET CLASS_NAME=SoundConvertor
SET JAR_NAME=SoundConvertor

SET OUTPUT_JAR_PATH=%CURR_DIR%..

call .\..\..\setenv.bat


echo *** Make Manifest file
echo Main-Class: %CLASS_NAME%> MANIFEST.MF


echo *** Compile source code
IF NOT EXIST %CLASSES_DIR% ( MD %CLASSES_DIR% )
%JAVA_PATH%\bin\javac %SRC_DIR%\*.java -d %CLASSES_DIR%


rem echo *** Package JAR file
rem IF NOT EXIST %BIN_DIR% ( MD %BIN_DIR% )
cd %CLASSES_DIR%
%JAVA_PATH%\bin\jar -cvfm %BIN_DIR%\%JAR_NAME%.jar ..\MANIFEST.MF *.class > NUL
cd ..


del /q MANIFEST.MF > NUL
rd /q /s %CLASSES_DIR% > NUL

echo %OUTPUT_JAR_PATH%
copy /Y %BIN_DIR%\%JAR_NAME%.jar %OUTPUT_JAR_PATH%

pause