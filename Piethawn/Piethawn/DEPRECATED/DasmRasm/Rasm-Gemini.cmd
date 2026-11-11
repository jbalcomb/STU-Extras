@echo off
setlocal EnableExtensions EnableDelayedExpansion
REM Rasm-Gemini.cmd
REM Usage: Rasm-Gemini.cmd <input.asm> [output.c]
REM   If output is omitted, writes to <input>.c next to the input file.
REM   Uses the rasm Gemini CLI skill.
REM   Optional environment:
REM     RASM_GEMINI_MODEL   Override the Gemini model name.
REM     RASM_GEMINI_TIMEOUT_SECONDS  Hard timeout in seconds. Default: 900.
REM     RASM_GEMINI_API_STATUS_TIMEOUT_SECONDS  Preflight API status timeout in seconds. Default: 10.
REM     RASM_GEMINI_SKIP_API_STATUS  Set to 1 to skip the API status preflight.
REM     RASM_GEMINI_STATUS_PYTHON  Override the Python launcher used for the API status preflight.
REM     GEMINI_SYSTEM_MD    Path to a system prompt markdown file.
REM
REM From PowerShell:
REM   .\Rasm-Gemini.cmd out\MAGIC\seg022\PageFlip_GrowOut__WIP.asm
REM
REM Examples:
REM   .\Rasm-Gemini.cmd out\MAGIC\seg022\PageFlip_GrowOut__WIP.asm
REM   .\Rasm-Gemini.cmd out\MAGIC\seg022\PageFlip_GrowOut__WIP.asm out\MAGIC\seg022\PageFlip_GrowOut__WIP.c
REM   .\Rasm-Gemini.cmd out\WIZARDS\ovr114\AI_BU_AssignAction__WIP.asm

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

if "%RASM_GEMINI_TIMEOUT_SECONDS%"=="" set "RASM_GEMINI_TIMEOUT_SECONDS=900"
if "%RASM_GEMINI_API_STATUS_TIMEOUT_SECONDS%"=="" set "RASM_GEMINI_API_STATUS_TIMEOUT_SECONDS=10"

set "MODEL_ARG="
if not "%RASM_GEMINI_MODEL%"=="" set "MODEL_ARG=--model %RASM_GEMINI_MODEL%"
set "PROMPT=Translate this IDA Pro assembly to C89 using the rasm skill rules. Output only the C code."
set "PS_ARGS=-p ""%PROMPT%"""
if not "%RASM_GEMINI_MODEL%"=="" set "PS_ARGS=--model ""%RASM_GEMINI_MODEL%"" %PS_ARGS%"
set "TMP_OUTPUT=%OUTPUT%.tmp"
set "TMP_ERROR=%OUTPUT%.err"
set "POWERSHELL_EXE=%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe"
set "STATUS_SCRIPT=%~dp0gemini_api_status.py"
set "STATUS_REPO_PYTHON=%~dp0.venv\Scripts\python.exe"
set "STATUS_FALLBACK_PYTHON=c:\python314\python.exe"
set "STATUS_PYTHON_EXE="
set "STATUS_PYTHON_ARGS="

if not exist "%POWERSHELL_EXE%" set "POWERSHELL_EXE=powershell"

if not "%RASM_GEMINI_MODEL%"=="" echo Model:       %RASM_GEMINI_MODEL%
echo Timeout:     %RASM_GEMINI_TIMEOUT_SECONDS%s

if not "%RASM_GEMINI_SKIP_API_STATUS%"=="1" (
    if not exist "%STATUS_SCRIPT%" (
        echo Gemini API status script not found: %STATUS_SCRIPT%
        exit /b 1
    )

    if not "%RASM_GEMINI_STATUS_PYTHON%"=="" (
        call :set_status_python "%RASM_GEMINI_STATUS_PYTHON%"
    ) else (
        call :set_status_python "%STATUS_FALLBACK_PYTHON%"
        if "!STATUS_PYTHON_EXE!"=="" call :set_status_python py -3
        if "!STATUS_PYTHON_EXE!"=="" call :set_status_python python
        if "!STATUS_PYTHON_EXE!"=="" call :set_status_python "%STATUS_REPO_PYTHON%"
    )

    if "!STATUS_PYTHON_EXE!"=="" (
        echo Failed to find a Python launcher for Gemini API status preflight.
        echo Set RASM_GEMINI_STATUS_PYTHON or RASM_GEMINI_SKIP_API_STATUS=1 to continue without it.
        exit /b 1
    )

    echo Preflight:   Gemini API status
    call "!STATUS_PYTHON_EXE!" !STATUS_PYTHON_ARGS! "%STATUS_SCRIPT%" --timeout %RASM_GEMINI_API_STATUS_TIMEOUT_SECONDS%
    if errorlevel 1 (
        echo Gemini API status preflight failed.
        exit /b 1
    )
)

if exist "%TMP_OUTPUT%" del /f /q "%TMP_OUTPUT%" >nul 2>&1
if exist "%TMP_ERROR%" del /f /q "%TMP_ERROR%" >nul 2>&1

"%POWERSHELL_EXE%" -NoProfile -ExecutionPolicy Bypass -Command ^
  "$ErrorActionPreference = 'Stop';" ^
    "$inputPath = [System.IO.Path]::GetFullPath('%INPUT%');" ^
    "$outputPath = [System.IO.Path]::GetFullPath('%TMP_OUTPUT%');" ^
    "$errorPath = [System.IO.Path]::GetFullPath('%TMP_ERROR%');" ^
    "$timeoutMs = %RASM_GEMINI_TIMEOUT_SECONDS% * 1000;" ^
    "try {" ^
    "  $geminiCommand = Get-Command gemini.cmd -ErrorAction SilentlyContinue;" ^
    "  if ($null -eq $geminiCommand) { $geminiCommand = Get-Command gemini -ErrorAction Stop }" ^
    "  $geminiSource = $geminiCommand.Source;" ^
    "  $psi = New-Object System.Diagnostics.ProcessStartInfo;" ^
    "  $psi.UseShellExecute = $false;" ^
    "  $psi.RedirectStandardInput = $true;" ^
    "  $psi.RedirectStandardOutput = $true;" ^
    "  $psi.RedirectStandardError = $true;" ^
    "  if ($geminiSource -match '\.(cmd|bat)$') { $psi.FileName = $env:ComSpec; $psi.Arguments = '/d /c ""' + $geminiSource + '" %PS_ARGS%"' } else { $psi.FileName = $geminiSource; $psi.Arguments = '%PS_ARGS%' }" ^
    "  $proc = [System.Diagnostics.Process]::Start($psi);" ^
    "  if ($null -eq $proc) { throw 'Failed to start gemini.' }" ^
    "  $stdoutTask = $proc.StandardOutput.ReadToEndAsync();" ^
    "  $stderrTask = $proc.StandardError.ReadToEndAsync();" ^
    "  $proc.StandardInput.Write([System.IO.File]::ReadAllText($inputPath));" ^
    "  $proc.StandardInput.Close();" ^
    "  if (-not $proc.WaitForExit($timeoutMs)) { try { $proc.Kill() } catch {} ; [System.IO.File]::WriteAllText($errorPath, 'Timed out after %RASM_GEMINI_TIMEOUT_SECONDS% seconds.'); exit 124 }" ^
    "  $proc.WaitForExit();" ^
    "  [System.IO.File]::WriteAllText($outputPath, $stdoutTask.GetAwaiter().GetResult());" ^
    "  $stderrText = $stderrTask.GetAwaiter().GetResult();" ^
    "  if ($stderrText) { [System.IO.File]::WriteAllText($errorPath, $stderrText) } elseif (Test-Path $errorPath) { Remove-Item $errorPath -Force }" ^
    "  exit $proc.ExitCode;" ^
    "} catch {" ^
    "  [System.IO.File]::WriteAllText($errorPath, $_.Exception.ToString());" ^
    "  exit 1;" ^
    "}"

set "EXIT_CODE=%ERRORLEVEL%"

if "%EXIT_CODE%"=="124" (
    echo Gemini translation timed out after %RASM_GEMINI_TIMEOUT_SECONDS% seconds.
    if exist "%TMP_ERROR%" type "%TMP_ERROR%"
    if exist "%TMP_OUTPUT%" del /f /q "%TMP_OUTPUT%" >nul 2>&1
    if exist "%TMP_ERROR%" del /f /q "%TMP_ERROR%" >nul 2>&1
    exit /b 124
)

if not "%EXIT_CODE%"=="0" (
    echo Gemini translation failed with exit code %EXIT_CODE%.
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

:set_status_python
set "STATUS_PYTHON_EXE="
set "STATUS_PYTHON_ARGS="

if "%~1"=="" exit /b 0

set "STATUS_CANDIDATE_EXE=%~1"
set "STATUS_CANDIDATE_ARGS=%~2"

call "%STATUS_CANDIDATE_EXE%" %STATUS_CANDIDATE_ARGS% --version >nul 2>&1
if errorlevel 1 exit /b 0

set "STATUS_PYTHON_EXE=%STATUS_CANDIDATE_EXE%"
set "STATUS_PYTHON_ARGS=%STATUS_CANDIDATE_ARGS%"
exit /b 0
