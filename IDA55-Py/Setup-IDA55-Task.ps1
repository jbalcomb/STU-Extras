<#
    Setup-IDA55-Task.ps1
    One-time setup (asks for UAC once) so IDA55 launches as admin with NO UAC prompt afterwards:
      1. Registers scheduled task "STU IDA55" -> runs Launch-IDA55.ps1 with highest privileges,
         interactively as the current user (so the IDA window shows on your desktop).
      2. Points the Desktop "STU IDA55" shortcut at: schtasks /run /tn "STU IDA55"
    Re-run any time to update the task/shortcut. Use -Remove to delete the task.
#>
param(
    [string]$TaskName = 'STU IDA55',
    [string]$Launcher = (Join-Path $PSScriptRoot 'Launch-IDA55.ps1'),
    [string]$Shortcut = (Join-Path ([Environment]::GetFolderPath('Desktop')) 'STU IDA55.lnk'),
    [string]$IdaExe   = 'C:\STU_DASM\IDA55\idag.exe',
    [string]$RunAsUser = "$env:USERDOMAIN\$env:USERNAME",
    # Optional: database for this task/shortcut. Empty = launcher's default (WIZARDS.idb).
    [string]$Idb = '',
    [switch]$Remove
)

$ErrorActionPreference = 'Stop'
$ResultFile = Join-Path $env:TEMP 'Setup-IDA55-Task.result.txt'

<#
    Returns $true if the current process is running elevated.
    Powered by Claude.
#>
function Test-Admin {
    $id = [Security.Principal.WindowsIdentity]::GetCurrent()
    (New-Object Security.Principal.WindowsPrincipal $id).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

<#
    Registers (or replaces) the elevated, interactive scheduled task that runs the launcher.
    Powered by Claude.
#>
function Register-IdaTask {
    $psArgs = "-NoProfile -ExecutionPolicy Bypass -WindowStyle Hidden -File `"$Launcher`""
    if ($Idb) { $psArgs += " -Idb `"$Idb`"" }
    $action = New-ScheduledTaskAction -Execute "$env:SystemRoot\System32\WindowsPowerShell\v1.0\powershell.exe" `
                                      -Argument $psArgs -WorkingDirectory (Split-Path $IdaExe)
    # Interactive = only runs in your logged-on session, so the GUI is visible (no stored password needed).
    $principal = New-ScheduledTaskPrincipal -UserId $RunAsUser -LogonType Interactive -RunLevel Highest
    $settings = New-ScheduledTaskSettingsSet -AllowStartIfOnBatteries -DontStopIfGoingOnBatteries `
                                             -ExecutionTimeLimit ([TimeSpan]::Zero) -MultipleInstances Parallel `
                                             -StartWhenAvailable:$false
    Register-ScheduledTask -TaskName $TaskName -Action $action -Principal $principal -Settings $settings `
                           -Description 'Launch IDA 5.5 elevated (no UAC) and back up MOM131 .idb files.' -Force | Out-Null
}

<#
    Rewrites the desktop shortcut to trigger the scheduled task.
    Powered by Claude.
#>
function Set-IdaShortcut {
    $sh = New-Object -ComObject WScript.Shell
    $s = $sh.CreateShortcut($Shortcut)
    $s.TargetPath = "$env:SystemRoot\System32\schtasks.exe"
    $s.Arguments = "/run /tn `"$TaskName`""
    $s.WorkingDirectory = Split-Path $IdaExe
    $s.IconLocation = "$IdaExe,0"
    $s.WindowStyle = 7   # minimized: schtasks' console doesn't pop up
    $s.Save()
    # Make sure the shortcut itself is NOT flagged "Run as administrator" (that would prompt UAC again).
    $b = [IO.File]::ReadAllBytes($Shortcut)
    if ($b[0x15] -band 0x20) { $b[0x15] = $b[0x15] -band (-bnot 0x20); [IO.File]::WriteAllBytes($Shortcut, $b) }
}

# Registering a highest-privilege task needs admin once: re-launch elevated, passing the real user along.
if (-not (Test-Admin)) {
    $argList = "-NoProfile -ExecutionPolicy Bypass -File `"$PSCommandPath`" -TaskName `"$TaskName`" -Launcher `"$Launcher`" " +
               "-Shortcut `"$Shortcut`" -IdaExe `"$IdaExe`" -RunAsUser `"$RunAsUser`""
    if ($Idb)    { $argList += " -Idb `"$Idb`"" }
    if ($Remove) { $argList += ' -Remove' }
    Start-Process -FilePath 'powershell.exe' -ArgumentList $argList -Verb RunAs -Wait
    if (Test-Path $ResultFile) { Get-Content $ResultFile }
    exit
}

try {
    if ($Remove) {
        Unregister-ScheduledTask -TaskName $TaskName -Confirm:$false
        "OK: removed task '$TaskName'" | Set-Content $ResultFile
    } else {
        if (-not (Test-Path $Launcher)) { throw "Launcher not found: $Launcher" }
        Register-IdaTask
        Set-IdaShortcut
        "OK: task '$TaskName' registered for $RunAsUser (highest privileges); shortcut '$Shortcut' -> schtasks /run" |
            Set-Content $ResultFile
    }
} catch {
    "FAILED: $_" | Set-Content $ResultFile
}
Get-Content $ResultFile
