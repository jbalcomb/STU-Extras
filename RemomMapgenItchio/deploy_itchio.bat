@echo off
REM Deploy the MoM Scenario Editor WASM build to itch.io using Butler.
REM Powered by Claude.
REM
REM Prerequisites:
REM   - Butler CLI installed and on PATH
REM   - Butler authenticated (butler login)
REM   - WASM build completed (build_wasm.bat)
REM
REM Usage:
REM   deploy_itchio.bat           - deploy existing WASM build
REM   deploy_itchio.bat --build   - build WASM first, then deploy

setlocal enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "SCRIPT_DIR=%SCRIPT_DIR:~0,-1%"

REM --- Defaults ---
set "ITCHIO_USER="
set "ITCHIO_GAME="
set "ITCHIO_CHANNEL=html5"
set "BUILD_DIR=build_wasm"

REM --- Parse arguments ---
set "DO_BUILD=0"
if "%1"=="--build" set "DO_BUILD=1"

REM --- Load configuration ---
set "CONF_FILE=%SCRIPT_DIR%\deploy.conf"
if not exist "%CONF_FILE%" (
    echo Warning: deploy.conf not found.
    echo   Copy the example and fill in your itch.io details:
    echo     copy deploy.conf.example deploy.conf
    echo.
    echo   Required settings:
    echo     ITCHIO_USER=your-username
    echo     ITCHIO_GAME=mom-scenario-editor
    echo.
    exit /b 1
)

REM Parse key=value pairs from deploy.conf.
REM Powered by Claude.
for /f "usebackq tokens=1,* delims==" %%A in ("%CONF_FILE%") do (
    set "LINE=%%A"
    if not "!LINE:~0,1!"=="#" (
        if not "%%A"=="" if not "%%B"=="" (
            set "%%A=%%B"
        )
    )
)

REM --- Validate configuration ---
if "%ITCHIO_USER%"=="" (
    echo Error: ITCHIO_USER is not set in deploy.conf.
    echo   Edit deploy.conf and set your itch.io username.
    exit /b 1
)
if "%ITCHIO_USER%"=="your-username" (
    echo Error: ITCHIO_USER is not set in deploy.conf.
    echo   Edit deploy.conf and set your itch.io username.
    exit /b 1
)
if "%ITCHIO_GAME%"=="" (
    echo Error: ITCHIO_GAME is not set in deploy.conf.
    echo   Edit deploy.conf and set your itch.io project slug.
    exit /b 1
)
if "%ITCHIO_CHANNEL%"=="" set "ITCHIO_CHANNEL=html5"

REM --- Check Butler is installed ---
where butler >nul 2>&1
if %errorlevel% neq 0 (
    echo Error: Butler is not installed or not on PATH.
    echo.
    echo   Install Butler from: https://itch.io/docs/butler/installing.html
    echo.
    echo   After installing, make sure 'butler' is on your PATH.
    echo   Then authenticate with: butler login
    exit /b 1
)

REM --- Build if requested ---
if "%DO_BUILD%"=="1" (
    echo === Building WASM target ===
    call "%SCRIPT_DIR%\build_wasm.bat"
    if %errorlevel% neq 0 (
        echo.
        echo Error: WASM build failed. Deployment aborted.
        echo   Fix the build errors above and try again.
        exit /b 1
    )
    echo.
)

REM --- Validate build output ---
set "BUILD_PATH=%SCRIPT_DIR%\%BUILD_DIR%"

if not exist "%BUILD_PATH%" (
    echo Error: Build output directory not found: %BUILD_DIR%\
    echo   Run the WASM build first: build_wasm.bat
    echo   Or use: %~nx0 --build
    exit /b 1
)

set "MISSING=0"
if not exist "%BUILD_PATH%\RemomMapgenItchio.html" (
    echo   Missing: RemomMapgenItchio.html
    set "MISSING=1"
)
if not exist "%BUILD_PATH%\RemomMapgenItchio.js" (
    echo   Missing: RemomMapgenItchio.js
    set "MISSING=1"
)
if not exist "%BUILD_PATH%\RemomMapgenItchio.wasm" (
    echo   Missing: RemomMapgenItchio.wasm
    set "MISSING=1"
)
if "%MISSING%"=="1" (
    echo.
    echo Error: Required build output files are missing from %BUILD_DIR%\
    echo   Run the WASM build first: build_wasm.bat
    echo   Or use: %~nx0 --build
    exit /b 1
)

REM --- Install deployment files ---
REM Use CMake install target to copy only deployment files to a clean dist
REM directory. This also renames RemomMapgenItchio.html to index.html as
REM required by itch.io. Powered by Claude.
set "DIST_DIR=%BUILD_PATH%\dist"

echo === Installing deployment files ===
cmake --install "%BUILD_PATH%" --prefix "%DIST_DIR%"
if %errorlevel% neq 0 (
    echo.
    echo Error: cmake --install failed.
    echo   Try rebuilding: build_wasm.bat
    exit /b 1
)
echo.

if not exist "%DIST_DIR%\index.html" (
    echo Error: cmake --install did not produce index.html in %BUILD_DIR%\dist\
    echo   Try rebuilding: build_wasm.bat
    exit /b 1
)

REM --- Deploy with Butler ---
set "BUTLER_TARGET=%ITCHIO_USER%/%ITCHIO_GAME%:%ITCHIO_CHANNEL%"
set "PROJECT_URL=https://%ITCHIO_USER%.itch.io/%ITCHIO_GAME%"

echo === Deploying to itch.io ===
echo   Target: %BUTLER_TARGET%
echo   Source: %BUILD_DIR%\dist\
echo.

butler push "%DIST_DIR%" "%BUTLER_TARGET%"
if %errorlevel% neq 0 (
    echo.
    echo Error: Butler push failed.
    echo.
    echo   Troubleshooting:
    echo     - Authentication expired? Run: butler login
    echo     - Project not found? Create it at: https://itch.io/game/new
    echo     - Network error? Check your internet connection.
    exit /b 1
)

echo.
echo === Deployment successful ===
echo   View your game at: %PROJECT_URL%
echo.

endlocal
