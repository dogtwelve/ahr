
if exist nodup.txt (
	find /C "%LINE%" nodup.txt > find_result.TXT
	(	set /P UNUSED=
		set /P FIND_RESULT= ) < find_result.TXT
	del find_result.TXT
) else (
	set FIND_RESULT=0
)
set FIND_RESULT=%FIND_RESULT:nodup.txt=%
set FIND_RESULT=%FIND_RESULT:-=%
set FIND_RESULT=%FIND_RESULT: =%
set FIND_RESULT=%FIND_RESULT::=%

if "%FIND_RESULT%" == "0" (
	echo %LINE%>> nodup.txt
)
