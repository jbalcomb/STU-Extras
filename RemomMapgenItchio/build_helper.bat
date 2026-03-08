@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
cd /D C:\STU\devel\STU-Extras\RemomMapgenItchio\build
cmake --build .
if %ERRORLEVEL% NEQ 0 exit /B %ERRORLEVEL%
ctest --output-on-failure
