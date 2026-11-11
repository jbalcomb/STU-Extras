# Gemini CLI Operator Runbook

## Purpose

Use this when a Gemini translation fails and you need to answer two questions quickly:

1. Is Gemini CLI adding noisy context to the request?
2. Is the backend API reachable from this machine?

## Current Fixes In This Repo

Project-local Gemini settings live in `.gemini/settings.json`.

They currently do two things:

- disable automatic directory-tree injection with `context.includeDirectoryTree=false`
- reduce failure spam with `ui.errorVerbosity=low`

`Rasm-Gemini.cmd` now runs `gemini_api_status.py` before starting Gemini unless you explicitly skip it.

## Normal Translation Flow

Run the wrapper normally:

```powershell
.\Rasm-Gemini.cmd out\MAGIC\seg022\PageFlip_GrowOut__WIP.asm
```

The wrapper now does this in order:

1. checks that the Gemini backend is reachable
2. stops early if the network or endpoint check fails
3. launches Gemini only after the preflight passes

## If The Preflight Fails

Run the status check directly:

```powershell
c:/python314/python.exe .\gemini_api_status.py
```

What to look for:

- `DNS OK` means the hostname resolves
- `TCP OK` means the host is reachable on port 443
- `TLS OK` means HTTPS negotiation works
- `HTTP 401 Unauthorized` is acceptable for this probe and still means the service is up

Treat these as real failures:

- DNS failure
- TCP failure
- TLS failure
- HTTP 5xx

## If You Need To Bypass The Preflight

Skip the API check for one run:

```powershell
$env:RASM_GEMINI_SKIP_API_STATUS = '1'
.\Rasm-Gemini.cmd out\MAGIC\seg022\PageFlip_GrowOut__WIP.asm
Remove-Item Env:RASM_GEMINI_SKIP_API_STATUS
```

Use a longer preflight timeout:

```powershell
$env:RASM_GEMINI_API_STATUS_TIMEOUT_SECONDS = '20'
.\Rasm-Gemini.cmd out\MAGIC\seg022\PageFlip_GrowOut__WIP.asm
Remove-Item Env:RASM_GEMINI_API_STATUS_TIMEOUT_SECONDS
```

Force a specific Python launcher for the preflight:

```powershell
$env:RASM_GEMINI_STATUS_PYTHON = 'c:/python314/python.exe'
.\Rasm-Gemini.cmd out\MAGIC\seg022\PageFlip_GrowOut__WIP.asm
Remove-Item Env:RASM_GEMINI_STATUS_PYTHON
```

## If Prompt Garbage Shows Up Again

Check these first:

1. Confirm `.gemini/settings.json` still has `context.includeDirectoryTree=false`.
2. Confirm `.gemini/settings.json` still has `ui.errorVerbosity=low`.
3. Re-run `gemini_api_status.py` to separate backend problems from wrapper problems.
4. If the backend is healthy, inspect Gemini CLI version and local config changes next.

## Expected Reality

Gemini CLI still adds a small built-in session context block. The repo fix removes the large directory-tree dump and keeps routine failures readable. Fully removing all CLI-added context would require replacing Gemini CLI with a direct API client.