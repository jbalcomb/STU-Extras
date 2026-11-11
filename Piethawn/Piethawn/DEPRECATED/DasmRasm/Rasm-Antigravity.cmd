@echo off
setlocal EnableExtensions EnableDelayedExpansion
REM Rasm-Antigravity.cmd
REM Usage: Rasm-Antigravity.cmd <input.asm> [output.c]
REM   If output is omitted, writes to <input>.c next to the input file.
REM   Uses the rasm Antigravity CLI path.
REM   Optional environment:
REM     RASM_ANTIGRAVITY_MODEL   Override the Antigravity model name.
REM     RASM_ANTIGRAVITY_TIMEOUT_SECONDS  Hard timeout in seconds. Default: 900.
REM     RASM_ANTIGRAVITY_EXE     Override the Antigravity CLI executable path.
REM     GEMINI_SYSTEM_MD         Path to a system prompt markdown file.
REM
REM From PowerShell:
REM   .\Rasm-Antigravity.cmd out\MAGIC\seg022\PageFlip_GrowOut__WIP.asm
REM
REM Examples:
REM   .\Rasm-Antigravity.cmd out\MAGIC\seg022\PageFlip_GrowOut__WIP.asm
REM   .\Rasm-Antigravity.cmd out\MAGIC\seg022\PageFlip_GrowOut__WIP.asm out\MAGIC\seg022\PageFlip_GrowOut__WIP.c
REM   .\Rasm-Antigravity.cmd out\WIZARDS\ovr114\AI_BU_AssignAction__WIP.asm

if "%~1"=="" (
    echo Usage: %~nx0 ^<input.asm^> [output.c]
    exit /b 1
)

set "INPUT=%~1"

if "%~2"=="" (
    set "OUTPUT=%~dpn1.c"
) else (
    set "OUTPUT=%~2"
)

if not exist "%INPUT%" (
    echo Input file not found: %INPUT%
    exit /b 1
)

echo Translating: %INPUT%
echo Output:      %OUTPUT%

if "%RASM_ANTIGRAVITY_TIMEOUT_SECONDS%"=="" set "RASM_ANTIGRAVITY_TIMEOUT_SECONDS=900"
if "%RASM_ANTIGRAVITY_EXE%"=="" set "RASM_ANTIGRAVITY_EXE=C:\Users\jbalcomb\AppData\Local\agy\bin\agy.exe"

set "TMP_OUTPUT=%OUTPUT%.tmp"
set "TMP_ERROR=%OUTPUT%.err"
set "POWERSHELL_EXE=%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe"
set "AGY_EXE=%RASM_ANTIGRAVITY_EXE%"

if not exist "%POWERSHELL_EXE%" set "POWERSHELL_EXE=powershell"

if not "%RASM_ANTIGRAVITY_MODEL%"=="" echo Model:       %RASM_ANTIGRAVITY_MODEL%
echo Timeout:     %RASM_ANTIGRAVITY_TIMEOUT_SECONDS%s

if not exist "%AGY_EXE%" (
    echo Antigravity CLI not found: %AGY_EXE%
    exit /b 1
)

if exist "%TMP_OUTPUT%" del /f /q "%TMP_OUTPUT%" >nul 2>&1
if exist "%TMP_ERROR%" del /f /q "%TMP_ERROR%" >nul 2>&1

"%POWERSHELL_EXE%" -NoProfile -ExecutionPolicy Bypass -Command ^
    "$ErrorActionPreference = 'Stop';" ^
        "$inputPath = [System.IO.Path]::GetFullPath('%INPUT%');" ^
        "$outputPath = [System.IO.Path]::GetFullPath('%TMP_OUTPUT%');" ^
        "$errorPath = [System.IO.Path]::GetFullPath('%TMP_ERROR%');" ^
        "$timeoutMs = %RASM_ANTIGRAVITY_TIMEOUT_SECONDS% * 1000;" ^
        "$stdoutPath = [System.IO.Path]::GetTempFileName();" ^
        "$stderrPath = [System.IO.Path]::GetTempFileName();" ^
        "try {" ^
        "  $prompt = [System.IO.File]::ReadAllText($inputPath);" ^
        "  $prompt = 'Translate this IDA Pro assembly to C89 using the rasm skill rules. Output only the C code.' + [Environment]::NewLine + [Environment]::NewLine + $prompt;" ^
        "  $arguments = @('--dangerously-skip-permissions','--print',$prompt);" ^
        "  if ('%RASM_ANTIGRAVITY_MODEL%' -ne '') { $arguments = @('--dangerously-skip-permissions','--model','%RASM_ANTIGRAVITY_MODEL%','--print',$prompt) }" ^
        "  $proc = Start-Process -FilePath '%AGY_EXE%' -ArgumentList $arguments -PassThru -NoNewWindow -RedirectStandardOutput $stdoutPath -RedirectStandardError $stderrPath;" ^
        "  if ($null -eq $proc) { throw 'Failed to start agy.' }" ^
        "  if (-not $proc.WaitForExit($timeoutMs)) { try { $proc.Kill() } catch {} ; [System.IO.File]::WriteAllText($errorPath, 'Timed out after %RASM_ANTIGRAVITY_TIMEOUT_SECONDS% seconds.'); exit 124 }" ^
        "  $stdoutText = [System.IO.File]::ReadAllText($stdoutPath);" ^
        "  [System.IO.File]::WriteAllText($outputPath, $stdoutText);" ^
        "  $stderrText = [System.IO.File]::ReadAllText($stderrPath);" ^
        "  if ($stderrText) { [System.IO.File]::WriteAllText($errorPath, $stderrText) } elseif (Test-Path $errorPath) { Remove-Item $errorPath -Force }" ^
        "  exit $proc.ExitCode;" ^
        "} catch {" ^
        "  [System.IO.File]::WriteAllText($errorPath, $_.Exception.ToString());" ^
        "  exit 1;" ^
        "} finally {" ^
        "  if (Test-Path $stdoutPath) { Remove-Item $stdoutPath -Force }" ^
        "  if (Test-Path $stderrPath) { Remove-Item $stderrPath -Force }" ^
        "}"

set "EXIT_CODE=%ERRORLEVEL%"

if "%EXIT_CODE%"=="124" (
    echo Antigravity translation timed out after %RASM_ANTIGRAVITY_TIMEOUT_SECONDS% seconds.
    if exist "%TMP_ERROR%" type "%TMP_ERROR%"
    if exist "%TMP_OUTPUT%" del /f /q "%TMP_OUTPUT%" >nul 2>&1
    if exist "%TMP_ERROR%" del /f /q "%TMP_ERROR%" >nul 2>&1
    exit /b 124
)

if not "%EXIT_CODE%"=="0" (
    echo Antigravity translation failed with exit code %EXIT_CODE%.
    if exist "%TMP_ERROR%" type "%TMP_ERROR%"
    if exist "%TMP_OUTPUT%" del /f /q "%TMP_OUTPUT%" >nul 2>&1
    if exist "%TMP_ERROR%" del /f /q "%TMP_ERROR%" >nul 2>&1
    exit /b %EXIT_CODE%
)

move /y "%TMP_OUTPUT%" "%OUTPUT%" >nul
if errorlevel 1 (
    echo Failed to move temporary output into place.
    if exist "%TMP_OUTPUT%" del /f /q "%TMP_OUTPUT%" >nul 2>&1
    if exist "%TMP_ERROR%" del /f /q "%TMP_ERROR%" >nul 2>&1
    exit /b 1
)

if exist "%TMP_ERROR%" del /f /q "%TMP_ERROR%" >nul 2>&1

echo Done: %OUTPUT%

exit /b 0