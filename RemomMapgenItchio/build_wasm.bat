@echo off
REM Build the MoM Scenario Editor for WebAssembly using Emscripten.
REM Powered by Claude.
REM
REM Prerequisites:
REM   - Emscripten SDK installed and emsdk_env.bat run
REM   - CMake 4.2+ and Ninja build system
REM
REM Usage:
REM   build_wasm.bat               - Release build
REM   build_wasm.bat --debug       - Debug build (DWARF info, source maps, assertions)
REM   build_wasm.bat --serve         - Release build + serve locally
REM   build_wasm.bat --debug --serve - Debug build + serve locally

setlocal enabledelayedexpansion

where emcmake >nul 2>&1
if %errorlevel% neq 0 (
    echo Error: Emscripten SDK not found. Please run emsdk_env.bat first.
    exit /b 1
)

REM Parse arguments. Powered by Claude.
set "BUILD_TYPE=Release"
set "DO_SERVE=0"
for %%A in (%*) do (
    if "%%A"=="--debug" set "BUILD_TYPE=Debug"
    if "%%A"=="--serve" set "DO_SERVE=1"
)

if "%BUILD_TYPE%"=="Debug" (
    set "BUILD_DIR=build_wasm_debug"
) else (
    set "BUILD_DIR=build_wasm"
)

echo === Configuring WASM %BUILD_TYPE% build ===
call emcmake cmake -B %BUILD_DIR% -G Ninja -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
if %errorlevel% neq 0 exit /b 1

echo === Building ===
cmake --build %BUILD_DIR%
if %errorlevel% neq 0 exit /b 1

echo.
echo === Build complete (%BUILD_TYPE%) ===
dir /b %BUILD_DIR%\RemomMapgenItchio.html %BUILD_DIR%\RemomMapgenItchio.js %BUILD_DIR%\RemomMapgenItchio.wasm 2>nul

REM Stage to dist/ with index.html so local serve matches itch.io. Powered by Claude.
set "DIST_DIR=%BUILD_DIR%\dist"
if exist "%DIST_DIR%" rd /s /q "%DIST_DIR%"
mkdir "%DIST_DIR%"
copy "%BUILD_DIR%\RemomMapgenItchio.html" "%DIST_DIR%\index.html" >nul
copy "%BUILD_DIR%\RemomMapgenItchio.js" "%DIST_DIR%\" >nul
copy "%BUILD_DIR%\RemomMapgenItchio.wasm" "%DIST_DIR%\" >nul
if exist "%BUILD_DIR%\RemomMapgenItchio.data" copy "%BUILD_DIR%\RemomMapgenItchio.data" "%DIST_DIR%\" >nul
if exist "%BUILD_DIR%\RemomMapgenItchio.worker.js" copy "%BUILD_DIR%\RemomMapgenItchio.worker.js" "%DIST_DIR%\" >nul
echo   Staged to %DIST_DIR%\

if "%DO_SERVE%"=="1" (
    echo.
    echo === Serving at http://localhost:8080 ===
    echo Open http://localhost:8080 in your browser
    cd %DIST_DIR%
    python -m http.server 8080
)

endlocal
