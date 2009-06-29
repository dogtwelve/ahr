
set IS_IN_COMMENT=

for /F "delims=" %%i in (%1) do (
	set LINE=%%i
	call History_ParseLine %2 %3
)
