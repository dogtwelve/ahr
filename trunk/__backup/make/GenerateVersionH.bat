@echo off

set OUTPUT_FILE=%PROJECT_PATH%\src\%PLATFORM%\Version.h
rem VERSION_MINOR should be 2 digits
set VERSION=%VERSION_MAJOR%.%VERSION_MINOR%.%VERSION_BUILD%



echo //>																				%OUTPUT_FILE%
echo // This file was generated using GenerateVersionH.bat>>							%OUTPUT_FILE%
echo //>>																				%OUTPUT_FILE%
echo #ifndef __GL_VERSION_H_INCLUDED__>>												%OUTPUT_FILE%
echo #define __GL_VERSION_H_INCLUDED__>>												%OUTPUT_FILE%
echo //>>																				%OUTPUT_FILE%
echo #define VERSION_TEXT "%VERSION%">>													%OUTPUT_FILE%
echo #define VERSION_COMPLETE_TEXT "Asphalt4 Version " VERSION_TEXT>>				%OUTPUT_FILE%
echo #define VERSION_MAJOR (%VERSION_MAJOR%) >>					                        %OUTPUT_FILE%
echo #define VERSION_MINOR (%VERSION_MINOR%) >>	                                        %OUTPUT_FILE%
echo #define VERSION_BUILD (%VERSION_BUILD%) >>	                                        %OUTPUT_FILE%
echo #define VERSION_ID  ((VERSION_MAJOR^<^<24)+(VERSION_MINOR^<^<16)+VERSION_BUILD) >>	%OUTPUT_FILE%
echo //>>																				%OUTPUT_FILE%
echo #endif //__GL_VERSION_H_INCLUDED__>>												%OUTPUT_FILE%
echo //>>																				%OUTPUT_FILE%
