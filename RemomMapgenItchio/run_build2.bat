@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
cd /D C:\STU\devel\STU-Extras\RemomMapgenItchio\build
ninja -d explain > C:\STU\devel\STU-Extras\RemomMapgenItchio\build_log.txt 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo BUILD_FAILED >> C:\STU\devel\STU-Extras\RemomMapgenItchio\build_log.txt
    exit /B 1
)
echo BUILD_SUCCEEDED >> C:\STU\devel\STU-Extras\RemomMapgenItchio\build_log.txt
ctest --output-on-failure >> C:\STU\devel\STU-Extras\RemomMapgenItchio\build_log.txt 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo TESTS_FAILED >> C:\STU\devel\STU-Extras\RemomMapgenItchio\build_log.txt
    exit /B 1
)
echo TESTS_PASSED >> C:\STU\devel\STU-Extras\RemomMapgenItchio\build_log.txt
