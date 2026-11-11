@echo off
REM Rasm-Agy.cmd - translate one IDA ASM listing to C89 via the agy CLI.
REM Usage: Rasm-Agy.cmd <input.asm> [output.c]
REM   .\Rasm-Agy.cmd out\WIZARDS\ovr093\GAME_IsWon__STUB.asm
REM   .\Rasm-Agy.cmd out\MAGIC\seg022\PageFlip_GrowOut__WIP.asm out\MAGIC\seg022\PageFlip_GrowOut.c
REM Thin shim over Rasm-Agy.ps1 (all the real logic lives there).
setlocal
REM Prefer PowerShell 7+ (pwsh); fall back to Windows PowerShell 5.1.
where pwsh.exe >nul 2>&1
if %ERRORLEVEL%==0 (
    set "PS=pwsh.exe"
) else (
    set "PS=%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe"
)
"%PS%" -NoProfile -ExecutionPolicy Bypass -File "%~dp0Rasm-Agy.ps1" %*
exit /b %ERRORLEVEL%
