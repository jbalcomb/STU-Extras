@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >/dev/null 2>&1
cd /D C:\STU\devel\STU-Extras\RemomMapgenItchio\build
cmake --build . > build_output.txt 2>&1
echo EXIT_CODE=%ERRORLEVEL% >> build_output.txt
