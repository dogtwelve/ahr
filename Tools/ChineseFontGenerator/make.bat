@echo off

pushd %cd%

cd ..\..\src\_iphone
call setenv.bat

popd

SET CLASS_NAME=CMain
SET JAR_NAME=ChineseFontGenerator

SET CURR_DIR=%CD%
SET TOOL_DIR=%PROJECT_PATH%\Tools\%JAR_NAME%
SET SRC_DIR=%TOOL_DIR%\src
SET BIN_DIR=%TOOL_DIR%\bin
SET CLASSES_DIR=%TOOL_DIR%\classes


echo *** Make Manifest file
echo Main-Class: %CLASS_NAME%> %TOOL_DIR%\MANIFEST.MF

echo *** Compile source code
IF NOT EXIST %CLASSES_DIR% ( MD %CLASSES_DIR% )
%JAVA_PATH%\bin\javac %SRC_DIR%\*.java -d %CLASSES_DIR%

echo *** Package JAR file
if not exist %BIN_DIR% ( MD %BIN_DIR% )

rem Make jar file
cd %CLASSES_DIR%
%JAVA_PATH%\bin\jar -cvfm %BIN_DIR%\%JAR_NAME%.jar %TOOL_DIR%\MANIFEST.MF *.class > NUL
cd %CURR_DIR%

del /q %TOOL_DIR%\MANIFEST.MF > NUL
rd /q /s %CLASSES_DIR% > NUL

rem Copy jar file to proper place
copy /Y %BIN_DIR%\%JAR_NAME%.jar %PROJECT_PATH%\Data\Language\.