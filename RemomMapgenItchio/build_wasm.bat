@echo off
REM Build the MoM Scenario Editor for WebAssembly using Emscripten.
REM Powered by Claude.
REM
REM Prerequisites:
REM   - Emscripten SDK installed and emsdk_env.bat run
REM   - CMake 4.2+ and Ninja build system
REM
REM Usage:
REM   build_wasm.bat        - build only
REM   build_wasm.bat serve  - build + serve locally for testing

where emcmake >nul 2>&1
if %errorlevel% neq 0 (
    echo Error: Emscripten SDK not found. Please run emsdk_env.bat first.
    exit /b 1
)

set BUILD_DIR=build_wasm

echo === Configuring WASM build ===
call emcmake cmake -B %BUILD_DIR% -G Ninja -DCMAKE_BUILD_TYPE=Release
if %errorlevel% neq 0 exit /b 1

echo === Building ===
cmake --build %BUILD_DIR%
if %errorlevel% neq 0 exit /b 1

echo.
echo === Build complete ===
dir /b %BUILD_DIR%\RemomMapgenItchio.html %BUILD_DIR%\RemomMapgenItchio.js %BUILD_DIR%\RemomMapgenItchio.wasm 2>nul

if "%1"=="serve" (
    echo.
    echo === Serving at http://localhost:8080 ===
    echo Open http://localhost:8080/RemomMapgenItchio.html in your browser
    cd %BUILD_DIR%
    python -m http.server 8080
)
