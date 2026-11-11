<#
.SYNOPSIS
    Translate one IDA Pro ASM listing to C89 using the Antigravity (agy) CLI.

.DESCRIPTION
    Give it a path to a .asm file; it writes the C89 translation next to it as .c.

    ONE code path for every input, any size. The wrapper assembles a single,
    fully self-contained prompt file (rules + task + the ASM text) in an isolated
    temp directory, then tells agy to read ONLY that one file and nothing else.

    Why this shape: agy's prompt can only ride the --print argument (Windows caps
    that at ~32 KB, and stdin is ignored), and pointing agy at the ASM by path
    turns it into a wandering agent -- it greps, lists dirs, shells out to search,
    and reads unrelated files. A self-contained temp file removes both problems:
    there is no size limit (it is a file, not a command-line arg) and nothing for
    agy to hunt for, so it just reads one file and emits C.

    No setup required. agy runs with --dangerously-skip-permissions, which is safe
    here only because its workspace is locked to the temp dir (both --add-dir and
    the process cwd) and that dir holds one self-contained file -- agy can approve
    tools but has nothing else to read, run, or wander into. The repo and sibling
    projects are never in scope.

.EXAMPLE
    .\Rasm-Agy.ps1 out\WIZARDS\ovr093\GAME_IsWon__STUB.asm

.EXAMPLE
    .\Rasm-Agy.ps1 out\MAGIC\seg022\PageFlip_GrowOut__WIP.asm -Force

.EXAMPLE
    .\Rasm-Agy.ps1 out\WIZARDS\ovr093\GAME_IsWon__STUB.asm out\WIZARDS\ovr093\GAME_IsWon.c
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string]$Asm,

    [Parameter(Position = 1)]
    [string]$Out,

    [string]$Rules,

    [string]$Model,

    [int]$TimeoutSec = 900,

    [string]$AgyExe = "$env:LOCALAPPDATA\agy\bin\agy.exe",

    [switch]$Force,

    [switch]$DryRun
)

$ErrorActionPreference = 'Stop'

# Resolve this script's own directory robustly (param defaults can't see $PSScriptRoot).
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
if (-not $Rules) { $Rules = Join-Path $ScriptDir 'RasmAiAgentPrompt.md' }

function Resolve-InputPath([string]$p) {
    if ([System.IO.Path]::IsPathRooted($p)) { return $p }
    return (Join-Path (Get-Location) $p)
}

function Fail([string]$msg) {
    Write-Host "ERROR: $msg" -ForegroundColor Red
}

$asmPath = Resolve-InputPath $Asm
if (-not (Test-Path -LiteralPath $asmPath)) {
    Fail "Input ASM not found: $asmPath"
    exit 1
}
if (-not (Test-Path -LiteralPath $Rules)) {
    Fail "Rules prompt not found: $Rules"
    exit 1
}
if (-not (Test-Path -LiteralPath $AgyExe)) {
    Fail "agy CLI not found: $AgyExe"
    exit 1
}

if (-not $Out) {
    $Out = [System.IO.Path]::ChangeExtension($asmPath, '.c')
}
$outPath = Resolve-InputPath $Out

if ((Test-Path -LiteralPath $outPath) -and -not $Force) {
    Write-Host "Skipping existing output (use -Force to overwrite): $outPath"
    exit 0
}

# Build the self-contained prompt file: rules + task + ASM, all inlined. This is
# the only thing agy will read, so its size is unbounded and it needs to hunt
# for nothing.
$rulesText = Get-Content -Raw -LiteralPath $Rules
$asmText   = Get-Content -Raw -LiteralPath $asmPath
$promptDoc = $rulesText + @"


=== TASK ===
Translate the IDA Pro assembly below to C89 per the rules above.
Output ONLY the C source code. No prose before or after. No markdown code fences.

=== ASSEMBLY ===
"@ + "`n" + $asmText

# Isolated temp dir holding exactly one file, so --add-dir exposes nothing else.
$workDir  = Join-Path ([System.IO.Path]::GetTempPath()) ("rasm-agy-" + [System.Guid]::NewGuid().ToString("N"))
$null     = New-Item -ItemType Directory -Path $workDir -Force
$docPath  = Join-Path $workDir 'translate.txt'
Set-Content -LiteralPath $docPath -Value $promptDoc -NoNewline -Encoding UTF8

# Strict instruction: read that one file, do not explore.
$prompt = "Read the file `"$docPath`" and carry out the translation instructions " +
          "inside it exactly. Do NOT search, list directories, run shell commands, " +
          "or read any other file. Output ONLY the C source code to stdout. " +
          "No prose. No markdown code fences."

# --dangerously-skip-permissions is safe HERE because the workspace is locked to
# $workDir (both --add-dir and the process cwd below), and that dir holds exactly
# one self-contained file. agy can auto-approve tools but has nothing else to
# read, run, or wander into. (To run permission-gated instead, drop this flag and
# add  "permissions":{"allow":["read_file"]}  to agy's settings.json.)
$agyArgs = @()
if ($Model) { $agyArgs += @('--model', $Model) }
$agyArgs += @('--print-timeout', ("{0}s" -f $TimeoutSec))
$agyArgs += @('--add-dir', $workDir)
$agyArgs += '--dangerously-skip-permissions'
$agyArgs += @('--print', $prompt)

Write-Host "Translating: $asmPath"
Write-Host "Output:      $outPath"
Write-Host ("Prompt doc:  {0} chars -> {1}" -f $promptDoc.Length, $docPath)
if ($Model) { Write-Host "Model:       $Model" }

if ($DryRun) {
    Write-Host "DRY RUN. Would invoke:"
    Write-Host ("  {0} {1}" -f $AgyExe, (($agyArgs | ForEach-Object { if ($_ -eq $prompt) { '<prompt>' } else { $_ } }) -join ' '))
    Remove-Item -LiteralPath $workDir -Recurse -Force -ErrorAction SilentlyContinue
    exit 0
}

# Run agy via the call operator (clean native-arg passing in pwsh 7+) with
# stdout/stderr redirected to temp files. agy's own --print-timeout is the
# backstop against hangs.
$stdoutFile = [System.IO.Path]::GetTempFileName()
$stderrFile = [System.IO.Path]::GetTempFileName()
# Launch agy with its cwd set to $workDir so its workspace/context scan sees only
# the temp dir -- never the repo or sibling projects.
Push-Location -LiteralPath $workDir
try {
    & $AgyExe @agyArgs 1> $stdoutFile 2> $stderrFile
    $exit = $LASTEXITCODE
}
finally {
    Pop-Location
    $result  = if (Test-Path $stdoutFile) { Get-Content -Raw -LiteralPath $stdoutFile } else { '' }
    $errText = if (Test-Path $stderrFile) { Get-Content -Raw -LiteralPath $stderrFile } else { '' }
    Remove-Item -LiteralPath $stdoutFile, $stderrFile -Force -ErrorAction SilentlyContinue
    Remove-Item -LiteralPath $workDir -Recurse -Force -ErrorAction SilentlyContinue
}

if ($exit -ne 0) {
    Fail "agy failed with exit code $exit."
    if ($errText) { Write-Host $errText }
    exit $exit
}
if ([string]::IsNullOrWhiteSpace($result)) {
    Fail "agy produced no output."
    if ($errText) { Write-Host "agy said: $errText" }
    exit 1
}

# Defensive cleanup: strip a wrapping ```c ... ``` fence if the model added one.
$result = $result -replace "^\s*```[a-zA-Z0-9]*\r?\n", '' -replace "\r?\n```\s*$", ''

# Write atomically.
$tmpOut = "$outPath.tmp"
Set-Content -LiteralPath $tmpOut -Value $result -NoNewline -Encoding UTF8
Move-Item -LiteralPath $tmpOut -Destination $outPath -Force
Write-Host "Done: $outPath"
exit 0
