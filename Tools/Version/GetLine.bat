set RESULT_LINE
set LINE_ID=0
set RESULT_LINE_ID=%2
for /F "delims=" %%i in (%1) do (
	set CURRENT_LINE=%%i
	call GetLine_Step
)
