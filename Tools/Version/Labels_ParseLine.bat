
rem Parse one line

if not defined LINE ( goto end )
set LINE=%LINE:"=%
if "%LINE%" == "" ( goto end )
set LINE=%LINE:(=[%
if "%LINE%" == "" ( goto end )
set LINE=%LINE:)=]%
if "%LINE%" == "" ( goto end )
set LINE=%LINE:<= %
if "%LINE%" == "" ( goto end )
set LINE=%LINE:>= %
if "%LINE%" == "" ( goto end )
set LINE=%LINE:|= %
if "%LINE%" == "" ( goto end )

if not defined %1 (
	if not "%LINE:Label: =%" == "%LINE%" (
		set %1=%LINE:Label: =%
	)
)

:end
