@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
cd /D C:\STU\devel\STU-Extras\RemomMapgenItchio
echo STARTING_BUILD > build_log.txt
cmake -S . -B build --fresh -G Ninja -DCMAKE_BUILD_TYPE=Debug >> build_log.txt 2>&1
cmake --build build >> build_log.txt 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo BUILD_FAILED >> build_log.txt
    exit /B 1
)
echo BUILD_SUCCEEDED >> build_log.txt
cd build
ctest --output-on-failure >> ..\build_log.txt 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo TESTS_FAILED >> ..\build_log.txt
    exit /B 1
)
echo TESTS_PASSED >> ..\build_log.txt
