<#
    Launch-IDA55.ps1
    Replaces the "STU IDA55" desktop shortcut.
    - Self-elevates (UAC) if not already admin, like the old RunAsAdmin shortcut.
    - Backs up the live .idb files in $IdbDir before launch and after IDA exits,
      as NAME-YYYYMMDDHHMM.idb, skipping any that are identical to the newest backup.
    - Logs to $IdbDir\ida55-launcher.log (the window runs hidden).
#>
param(
    [string]$IdaExe = 'C:\STU_DASM\IDA55\idag.exe',
    [string]$IdbDir = 'C:\STU_DASM\IDA55_MOM131',
    # Database to open (skips IDA's startup dialog). Pass -Idb '' to get the dialog back.
    [string]$Idb    = 'C:\STU_DASM\IDA55_MOM131\WIZARDS.idb'
)

$ErrorActionPreference = 'Stop'
$LogFile = Join-Path $IdbDir 'ida55-launcher.log'

<#
    Appends a timestamped line to the launcher log.
    Powered by Claude.
#>
function Write-Log([string]$Message) {
    $line = '{0:yyyy-MM-dd HH:mm:ss}  {1}' -f (Get-Date), $Message
    Add-Content -Path $LogFile -Value $line
}

<#
    Returns $true if the current process is running elevated.
    Powered by Claude.
#>
function Test-Admin {
    $id = [Security.Principal.WindowsIdentity]::GetCurrent()
    (New-Object Security.Principal.WindowsPrincipal $id).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

<#
    Copies each live NAME.idb (no '-' / ' ' in the base name) to NAME-YYYYMMDDHHMM.idb,
    unless its content matches the newest existing NAME-*.idb backup.
    Powered by Claude.
#>
function Backup-Idbs([string]$Phase) {
    $stamp = Get-Date -Format 'yyyyMMddHHmm'
    $live = Get-ChildItem -Path $IdbDir -Filter '*.idb' -File | Where-Object { $_.BaseName -notmatch '[- ]' }
    foreach ($idb in $live) {
        try {
            if ($idb.Length -eq 0) {
                Write-Log "[$Phase] WARNING $($idb.Name) is 0 bytes, NOT backed up"
                continue
            }
            $newest = Get-ChildItem -Path $IdbDir -Filter "$($idb.BaseName)-*.idb" -File |
                Where-Object { $_.BaseName -match "^$([regex]::Escape($idb.BaseName))-\d{12}$" } |
                Sort-Object Name -Descending | Select-Object -First 1
            if ($newest -and $newest.Length -eq $idb.Length -and
                (Get-FileHash $newest.FullName).Hash -eq (Get-FileHash $idb.FullName).Hash) {
                Write-Log "[$Phase] $($idb.Name) unchanged (matches $($newest.Name)), skipped"
                continue
            }
            $dest = Join-Path $IdbDir "$($idb.BaseName)-$stamp.idb"
            if (Test-Path $dest) { $dest = Join-Path $IdbDir "$($idb.BaseName)-$stamp-$Phase.idb" }
            Copy-Item -Path $idb.FullName -Destination $dest
            Write-Log "[$Phase] $($idb.Name) -> $(Split-Path $dest -Leaf)"
        } catch {
            Write-Log "[$Phase] ERROR backing up $($idb.Name): $_"
        }
    }
}

# Re-launch elevated if needed (UAC prompt), hidden, then exit this instance.
if (-not (Test-Admin)) {
    $argList = "-NoProfile -ExecutionPolicy Bypass -WindowStyle Hidden -File `"$PSCommandPath`" -IdaExe `"$IdaExe`" -IdbDir `"$IdbDir`" -Idb `"$Idb`""
    Start-Process -FilePath 'powershell.exe' -ArgumentList $argList -Verb RunAs -WindowStyle Hidden
    exit
}

try {
    Write-Log "=== launcher start ==="
    if ($Idb) {
        if (-not (Test-Path $Idb)) { throw "IDB not found: $Idb" }
        if ((Get-Item $Idb).Length -eq 0) {
            throw "IDB is 0 bytes (corrupt/truncated): $Idb`nIDA was NOT launched. Restore it from a NAME-YYYYMMDDHHMM.idb backup."
        }
    }
    Backup-Idbs 'pre'
    $sp = @{ FilePath = $IdaExe; WorkingDirectory = (Split-Path $IdaExe); PassThru = $true }
    if ($Idb) { $sp.ArgumentList = "`"$Idb`"" }
    $p = Start-Process @sp
    Write-Log "started $IdaExe $Idb (pid $($p.Id))"
    $p.WaitForExit()
    Write-Log "IDA exited (code $($p.ExitCode))"
    Backup-Idbs 'post'
    if ($Idb -and (Test-Path $Idb) -and (Get-Item $Idb).Length -eq 0) {
        throw "IDA exited and left $Idb at 0 bytes!`nIt was NOT backed up. Restore from the newest NAME-YYYYMMDDHHMM.idb backup."
    }
    Write-Log "=== launcher done ==="
} catch {
    Write-Log "FATAL: $_"
    Add-Type -AssemblyName PresentationFramework
    [System.Windows.MessageBox]::Show("IDA55 launcher error:`n$_`n`nSee $LogFile", 'STU IDA55') | Out-Null
}
