if exist nodup.txt del nodup.txt
rem echo ------------------------------------>nodup.txt
for /F "delims=" %%i in (%1) do (
	set LINE=%%i
	call RemoveDup_ParseLine
)
copy nodup.txt %1 /Y
del nodup.txt
