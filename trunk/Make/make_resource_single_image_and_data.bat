call setenv.bat

::WE ARE @ PRJ_BUILD_DIR


::Make temporary dir
RD /Q /S bld_tmp
MD bld_tmp

pushd bld_tmp

::Resource Compile
copy %PRJ_DATA_GFX_DIR%\*.png .
copy %PRJ_DATA_GFX_DIR%\*.bmp .
copy %PRJ_DATA_GFX_DIR%\*.sprite .

%AURORA% %PRJ_SPEC_DIR%\export_single_image_and_data.sprcmd

popd

pushd %PRJ_BUILD_OUTPUT_DIR%\sprite
for %%i in (*.data) do (
	copy /b /y %%~ni.data+%%~ni.image	%%~ni.bsprite 
	
	del %%~ni.data
	del %%~ni.image
)
popd

::Remove temporary dir
RD /Q /S bld_tmp

echo ================================
echo ==== build done ================
echo ================================

pause
