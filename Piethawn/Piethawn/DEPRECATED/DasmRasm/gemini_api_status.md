
# Gemini API Status Notes

## What Kicked This Off

The immediate problem was not just that Gemini CLI was failing sometimes. The bigger problem was that when it failed, the terminal output showed a huge block of apparent garbage in the submitted prompt.

The concrete example was a failed run of `rasm_named_gemini.py` against `City_Screen__WIP`. The stderr output included a serialized request body that contained:

- a `<session_context>` block,
- workspace metadata,
- a rendered directory tree of the repo,
- and mojibake-looking tree characters such as `╬ô├╢...`.

That made it look like the Piethawn wrapper scripts were submitting junk to Gemini.

## What We Needed To Determine

There were two separate questions to answer.

1. Where was the garbage actually coming from?
2. Once we understood that, what was the smallest reliable fix?

It was not enough to inspect the wrapper and guess. The failure output already showed the actual request payload, so the goal became tracing that payload back to the correct layer.

## How We Worked Through It

We started from the local wrapper chain because that is the visible entry point.

- `rasm_named_gemini.py` resolves a named ASM function and calls `Rasm-Gemini.cmd`.
- `Rasm-Gemini.cmd` invokes `gemini -p ...` and pipes the ASM file on stdin.
- The wrapper also sets `GEMINI_SYSTEM_MD` when `RasmAiAgentPrompt.md` exists.

That established an important limit: the wrapper itself only contributes a short prompt string, the ASM payload, and an optional system prompt markdown file. It does not generate a directory tree or a `<session_context>` block.

From there, the debugging path shifted from repo code to Gemini CLI internals and local user configuration.

We inspected the local Gemini config under `C:\Users\jbalcomb\.gemini` and confirmed:

- the auth mode was `oauth-personal`,
- the project was trusted,
- and user `ui.errorVerbosity` was set to `full`.

That mattered because `full` error verbosity causes Gemini CLI to dump much more request detail on failure.

We then inspected the installed Gemini CLI sources and confirmed two critical behaviors.

### 1. Gemini CLI injects session context before the real user payload

The CLI builds initial chat history with an environment context block. That block includes:

- date,
- OS,
- temp directory,
- and optionally the workspace directory tree.

So the garbage was not coming from the Piethawn wrapper. It was coming from Gemini CLI's own request bootstrapping.

### 2. Directory tree injection is configurable

The installed CLI code showed that directory tree inclusion is controlled by:

`context.includeDirectoryTree`

and defaults to `true`.

That was the actual fix point for the worst part of the prompt bloat.

## What We Changed

We added project-local Gemini CLI settings in `.gemini/settings.json`:

```json
{
	"context": {
		"includeDirectoryTree": false
	},
	"ui": {
		"errorVerbosity": "low"
	}
}
```

### Why disable `includeDirectoryTree`

Because the noisy workspace tree was the bulk of the junk being injected into the prompt. Turning this off removes the biggest irrelevant payload from Gemini CLI's initial request while keeping the rest of the project setup intact.

### Why lower `errorVerbosity`

Because even if the CLI still includes some built-in session context, there was no reason to dump the entire serialized request body into the terminal on routine API failures. Lowering the verbosity prevents the terminal from spewing the full prompt payload when the server returns an error.

## Why We Did It This Way

We chose the smallest fix that addressed the real problem at the layer causing it.

Reasons:

- It fixed Gemini CLI behavior without rewriting the Piethawn wrapper stack.
- It kept the existing `rasm_*_gemini.py` workflow intact.
- It avoided replacing Gemini CLI with a direct API client before that was necessary.
- It used a project-local setting, so the change stays with this repo instead of forcing a user-global behavior change.

In other words, this was the lowest-risk correction that directly removed the directory-tree garbage from the request path.

## How We Verified The Fix

We did not stop at code inspection.

We reran the original command:

```powershell
c:/python314/python.exe .\rasm_named_gemini.py City_Screen__WIP
```

That completed cleanly and no longer showed the huge prompt dump.

Then we forced the failure path intentionally with an invalid model so stderr would still execute:

```powershell
c:/python314/python.exe .\rasm_named_gemini.py City_Screen__WIP --force --model definitely-not-a-real-gemini-model
```

That produced a normal concise Gemini API error without the giant serialized request body and without the mojibake directory-tree block.

That mattered because the original complaint was about failure output and hidden prompt composition, not just successful runs.

## Why The API Status Script Was Added

Once the prompt-garbage issue was understood, the next practical need was to separate:

- local wrapper bugs,
- Gemini CLI behavior,
- and actual backend availability.

That is why `gemini_api_status.py` was added.

Its job is to answer a narrower, operational question:

"Is the API host that Gemini CLI is currently using reachable from this machine?"

## What `gemini_api_status.py` Does

The script:

- reads the local Gemini CLI auth selection,
- infers the backend transport in use,
- maps that auth mode to the expected API host,
- and probes the host at four layers:
	- DNS
	- TCP
	- TLS
	- HTTP

For the current setup, it detected:

- auth type: `oauth-personal`
- transport: Google Code Assist
- host: `cloudcode-pa.googleapis.com:443`

The live run showed:

- DNS OK
- TCP OK
- TLS OK
- HTTP `401 Unauthorized`

That `401` is expected for an unauthenticated direct probe. It is still a success for status purposes because it proves the backend is reachable and responding.

## Why The Script Was Written The Way It Was

Several design choices were deliberate.

### Stdlib-only implementation

The script uses only the Python standard library.

Why:

- no extra dependency installation,
- easier to run in this repo immediately,
- consistent with the lightweight utility scripts already present here.

### It probes the backend Gemini CLI actually uses

The script does not just ping a generic Google URL. It maps the current auth mode to the real backend family that Gemini CLI is using.

That matters because Gemini CLI can use different backends depending on auth mode.

### It checks multiple layers, not just HTTP

A single failed HTTP request does not tell you enough. Splitting the probe into DNS, TCP, TLS, and HTTP narrows failures much faster.

Examples:

- DNS failure suggests name resolution or network policy issues.
- TCP failure suggests routing, firewall, or reachability problems.
- TLS failure suggests certificate or interception issues.
- HTTP 401/404/429/5xx helps distinguish auth, path, quota, and service problems.

### It supports both human and automation use

The script includes plain text output for terminal use and `--json` for automation or later wrapper integration.

## Outcome

By the end of this work, we had:

- identified that the prompt garbage was Gemini CLI auto-context, not Piethawn wrapper junk,
- disabled directory-tree injection at the project level,
- suppressed full serialized request dumps on routine failures,
- verified both success and failure paths after the fix,
- and added a dedicated API status script to check backend reachability independently of the wrapper workflow.

## Current Commands

Check API/backend status:

```powershell
c:/python314/python.exe .\gemini_api_status.py
```

Machine-readable output:

```powershell
c:/python314/python.exe .\gemini_api_status.py --json
```

Force a specific auth-mode mapping for testing:

```powershell
c:/python314/python.exe .\gemini_api_status.py --auth-type gemini-api-key
```

## Remaining Limit

Gemini CLI still adds a small built-in session context block of its own. The project-level fix removed the high-noise directory tree and the verbose terminal dump, which were the real source of the original complaint. Fully eliminating all CLI-added context would require replacing Gemini CLI with a direct API client or a different transport path.
