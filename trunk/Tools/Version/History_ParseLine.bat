
rem Parse one line

if not defined LINE ( goto end )
set LINE=%LINE:"='%
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


if defined IS_IN_COMMENT (
	if not "%LINE:******=%" == "%LINE%" (
		set IS_IN_COMMENT=
	)
) else (
	if not "%LINE:Comment: =%" == "%LINE%" (
		set IS_IN_COMMENT=1
	)
)

if defined IS_IN_COMMENT (
	if "%LINE:*~=%" == "%LINE%" (
		if not "%LINE:Comment: =%" == "" (
			if not "%LINE:**=%" == "%LINE%" (
				echo %LINE:Comment: =%>>%2
			) else (
				echo %LINE:Comment: =%>>%1
			)
		)
	)
)

:end
