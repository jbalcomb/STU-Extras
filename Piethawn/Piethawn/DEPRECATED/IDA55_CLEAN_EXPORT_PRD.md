# IDA55 Clean Export Automation PRD

## Summary

Build a safer export workflow for IDA Pro 5.5 that captures the current database and registry state, temporarily applies a known clean export profile, produces `MAP`, `ASM`, `INC`, and `LST` files for `MAGIC` and `WIZARDS`, then restores the original settings.

The long-term goal is to replace fragile manual export steps with a repeatable pipeline that produces clean inputs for Piethawn's existing post-IDA processing, matching, diff, and IDC update-script generation.

## Problem

The existing `eg_idc/stu_config.idc` is old exploratory code. It contains useful discoveries about `INF_*` settings, but it mixes:

- dumping settings
- changing settings
- comments from experiments
- partial assumptions about which values affect exported files

Some settings work because they are IDB/session `INF_*` values. Other behavior may appear to require registry changes because IDA also stores UI/session state under `HKCU\Software\Hex-Rays\IDA`, such as `InputDirectory` and recent database history.

We need a workflow that separates those responsibilities and guarantees that IDA settings are restored after export.

## Goals

- Capture current IDA Pro 5.5 export-relevant settings before changing them.
- Capture registry state as a safety snapshot.
- Apply a clean export profile only for the duration of the export.
- Generate consistent `MAP`, `ASM`, `INC`, and `LST` files for both games.
- Restore original IDA settings even if a generation step fails when feasible.
- Produce before/after logs that make failures diagnosable.
- Feed the existing Piethawn post-processing workflow.

## Non-Goals

- Replace all manual IDA database editing.
- Automatically repair bad IDB analysis.
- Blindly import registry backups after every run.
- Rewrite the existing post-IDA matching pipeline in phase 1.

## Current Related Assets

- `eg_idc/stu_config.idc`
  - Old exploratory settings script.
- `eg_idc/stu_produce.idc`
  - Old exploratory file-generation script.
- `ida55_dump_settings.idc`
  - Current settings/segment dump script.
- `ida_proc_data_export\ida55_dump_functions.idc`
  - Function dump script used by the matcher and status sync. Includes function colors.
- `ida55_registry_dump.py`
  - Registry snapshot/report utility.
- `ida_dasm_export\ida55_copy_outputs.py`
  - Copies standard IDA output files from `C:\STU_DASM\IDA55_MOM131` into `in/`.
- `run_post_ida55_dump_pipeline.py`
  - Existing post-export pipeline.

## Proposed Architecture

### 1. Python Orchestrator

Create a Python script that coordinates the full export workflow from outside IDA.

Proposed name:

```text
ida_dasm_export\ida55_clean_export.py
```

Responsibilities:

- Locate `MAGIC.idb` and `WIZARDS.idb`.
- Snapshot relevant registry keys before running IDA.
- Generate or select the clean-export IDC script.
- Launch IDA 5.5 through `ida_automation\idaw.py`, not directly through `idaw.exe`.
- Back up each packed `.idb` before launching IDA.
- Capture process exit codes and logs.
- Snapshot registry keys after export.
- Verify expected output files exist.
- Optionally copy output files into `in/`.
- Optionally run post-processing.

IDA 5.5 supports batch IDC execution with a command shaped like:

```text
idaw.exe -A -Sscript.idc file.idb
```

In this project, production automation must not call `idaw.exe` directly. It must route through `ida_automation\idaw.py`, which backs up the packed `.idb`, refuses 0-byte IDBs, and reports hard-stop IDA database-state warnings. The orchestrator command shape is:

```text
python -B ida_automation\idaw.py -- -A -L<log> -S<script.idc> <file.idb>
```

### 2. Production IDC Export Script

Create a clean production IDC script instead of modifying `stu_config.idc`.

Proposed name:

```text
ida_dasm_export\ida55_clean_export.idc
```

Responsibilities:

- Enter batch mode.
- Wait for auto-analysis to settle.
- Dump current export-relevant `INF_*` settings.
- Emit a restore IDC or restore block before changing anything.
- Apply the clean export profile.
- Generate:
  - `<IDB base>.map`
  - `<IDB base>.asm`
  - `<IDB base>.inc`
  - `<IDB base>.lst`
- Leave function/color dumps to the separate `ida_proc_data_export\ida55_dump_functions.idc` workflow unless that integration is explicitly added later.
- Restore original settings.
- Dump after-restore settings.
- Exit with an explicit status code.

### 3. Registry Snapshot

Registry should be treated as a guardrail, not the main settings control surface.

The likely useful key is:

```text
HKCU\Software\Hex-Rays\IDA
```

The orchestrator should export snapshots before and after:

```powershell
reg export HKCU\Software\Hex-Rays\IDA ida55-reg-before.reg /y
reg export HKCU\Software\Hex-Rays\IDA ida55-reg-after.reg /y
```

Importing a registry backup should be a manual recovery step unless we prove a specific value must be restored automatically.

## Clean Export Profile

The clean profile should be applied with `SetCharPrm`, `SetShortPrm`, and `SetLongPrm` as appropriate.

Recommended initial profile:

| Setting | Value | Purpose |
|---|---:|---|
| `INF_ENTAB` | `0` | Disable tabs in output |
| `INF_INDENT` | `0` | Remove instruction indentation |
| `INF_COMMENT` | `0` | Remove comment indentation |
| `INF_XREFNUM` | `0` | Suppress generated xref comments |
| `INF_SHOWPREF` | `0` | Suppress line prefixes |
| `INF_BORDER` | `0` | Suppress data/code border separator lines such as `;---------------------------------------------------------------------------` |
| `INF_PREFFLAG` | `0` | Suppress stack/segment/function prefix fields |
| `INF_CMTFLAG` | `SW_NOCMT` | Suppress comments |
| `INF_ORG` | `1` | Keep `org` directives |
| `INF_ASSUME` | `1` | Keep `assume` directives |
| `INF_BINPREF` | `0` | Suppress opcode byte prefixes |
| `INF_ASMTYPE` | `1` | Use Borland TASM Ideal mode |
| `INF_MAXREF` | `0` | Max xref depth; use `SetLongPrm` |

Open validation item:

- Confirm whether `INF_ASMTYPE = 1` is always the best target assembler for the current split/diff tools, or whether generic Intel output gives more stable parsing.

## Generated Files

For each database, the clean export stage should produce:

```text
C:\STU_DASM\IDA55_MOM131\MAGIC.map
C:\STU_DASM\IDA55_MOM131\MAGIC.asm
C:\STU_DASM\IDA55_MOM131\MAGIC.inc
C:\STU_DASM\IDA55_MOM131\MAGIC.lst
C:\STU_DASM\IDA55_MOM131\WIZARDS.map
C:\STU_DASM\IDA55_MOM131\WIZARDS.asm
C:\STU_DASM\IDA55_MOM131\WIZARDS.inc
C:\STU_DASM\IDA55_MOM131\WIZARDS.lst
```

Function/color dumps are separate inputs generated by `ida_proc_data_export\ida55_dump_functions.idc`; run them through `ida_proc_data_export\ida55_export_function_data.py`, which uses the shared `ida_automation\idaw.py` launcher.

Before each IDA launch, `ida_automation\idaw.py` should create a timestamped packed-IDB backup in `C:\STU_DASM\IDA55_MOM131\`, for example:

```text
MAGIC-202605011748.idb
WIZARDS-202605011748.idb
```

## Workflow Phases

### Phase 1: Capture And Restore Settings

- Build `ida_dasm_export\ida55_clean_export.idc`.
- Snapshot export-relevant `INF_*` settings before changes.
- Generate a restore block before changing anything.
- Apply the clean profile.
- Restore original settings.
- Dump settings after restore.
- Verify before/after values match.

Success criteria:

- Running the IDC on one IDB leaves the original settings restored.
- Logs show every changed value and its restored value.

### Phase 2: Produce Clean Output Files

- Generate `MAP`, `ASM`, `INC`, and `LST`.
- Verify output files exist and are non-empty.
- Add clear failure messages when `GenerateFile` fails.
- Preserve output logs next to the IDB.

Success criteria:

- `MAGIC` and `WIZARDS` exports can be regenerated consistently from IDA batch mode.

### Phase 3: Python Orchestration

- Add `ida_dasm_export\ida55_clean_export.py`.
- Locate `ida_automation\idaw.py` and IDB files from config/defaults.
- Run `MAGIC.idb` and `WIZARDS.idb` export jobs through `ida_automation\idaw.py`.
- Export registry snapshots before and after.
- Verify outputs.
- Return non-zero on missing outputs, IDA failure, 0-byte IDB detection, or current-run unpacked-database warning.

Success criteria:

- One command exports both games without manual IDA menu work.

### Phase 4: Copy Outputs Into Repo

Implemented: `ida_dasm_export\ida55_clean_export.py --copy` calls `ida_dasm_export\ida55_copy_outputs.py` after successful export and verification.

Target copy:

```text
from: C:\STU_DASM\IDA55_MOM131\
to:   C:\STU\devel\STU-Extras\Piethawn\Piethawn\in\
```

Files:

```text
MAGIC.MAP
MAGIC.ASM
MAGIC.INC
MAGIC.LST
WIZARDS.MAP
WIZARDS.ASM
WIZARDS.INC
WIZARDS.LST
```

Current behavior:

- Copy overwrites the standard files under `in/`.
- Copied filenames use the existing repository style, such as `in/MAGIC.asm`.
- Function/color dumps remain separate and are not part of this clean-export copy step.

### Phase 5: Post-Process Into Segments And Procedures

Implemented as an explicit option: `ida_dasm_export\ida55_clean_export.py --post-process` calls `run_post_ida55_dump_pipeline.py` after fresh exports land in `in/`.

Existing command:

```powershell
python -B .\run_post_ida55_dump_pipeline.py
```

Relevant current stages:

- Split full ASM dumps into segment/procedure trees:
  - `ida55_split_asm.py`
- Convert function text dumps to JSON:
  - `ida_proc_data_export\ida55_function_dump.py`
- Generate `out/MAGIC/...`
- Generate `out/WIZARDS/...`

Current behavior:

- Post-processing remains explicit via `--post-process`.
- The post-processing runner owns validation of required inputs, including function dumps.

### Phase 6: TBD Generate Diffs And MGC/WZD Update Scripts

TBD: run the matching, diff, and sync/update-script generation steps after split/procedure outputs are refreshed.

Existing stages:

- Generate split-ASM duplicate index:
  - `asm_dedupe_index.py`
- Generate normalized ASM diff reports:
  - `asm_diff_report.py`
- Match functions by machine-code hashes and segment-position overrides:
  - `ida55_match_functions.py`
- Generate MGC/MAGIC rename/update IDC from WZD/WIZARDS names:
  - `ida55_generate_magic_rename_idc.py`
- Generate sample pair diffs:
  - `ida55_function_pair_diff.py`
- Merge completion status and generate completion sync scripts:
  - `ida_proc_status_sync/ida_proc_status_sync.py`

Expected outputs:

```text
out/asm-dedupe-index.json
out/asm-diff-report/...
ida_proc_status_sync/ida_proc_status_sync.plan.json
ida_proc_status_sync/mgc_proc_status_sync.idc
ida_proc_status_sync/wzd_proc_status_sync.idc
```

Name-sync outputs now live under the newer name-sync/prereq directories rather than the old `out/ida55-sync/magic_rename_to_wizards.*` path.

Open decisions:

- Should the naming use `MGC/WZD` or `MAGIC/WIZARDS` in new script names and logs?
- Should rename IDC generation remain one-way from WZD to MGC, or should the framework allow either direction?
- Should completion-color sync be part of the default run or a separate flag?

## Error Handling

- If IDA export fails before settings are restored, the log must say so plainly and point to the generated restore IDC.
- If registry snapshots fail, the orchestrator should warn but still allow IDA export when explicitly requested.
- If any required output file is missing or empty, the orchestrator should fail.
- If copy/post-processing is enabled and fails, later stages should not run.
- If `ida_automation\idaw.py` sees a 0-byte `.idb` before or after launch, the run must fail.
- If current-run IDA log output contains `IDA has found unpacked version of database`, the run must fail and stop before the next IDB.

## Verification

Minimum verification:

- Run settings-only mode against one test IDB.
- Confirm before/after settings dumps match.
- Run export mode against `MAGIC.idb` and confirm `MAP`, `ASM`, `INC`, and `LST` are generated.
- Run export mode against `WIZARDS.idb` and confirm `MAP`, `ASM`, `INC`, and `LST` are generated.
- Confirm `ida_automation\idaw.py` creates timestamped packed-IDB backups before launch.
- Confirm `ida_dasm_export\ida55_clean_export.py --copy` copies all 8 standard output files into `in/`.
- Run `ida_dasm_export\ida55_check_clean_asm.py in/MAGIC.asm in/WIZARDS.asm`; it must reject tabs, line prefixes, opcode byte prefixes, XREF comments, border separator lines, and proc/endp mismatches.
- Run `run_post_ida55_dump_pipeline.py` after copying outputs when `--post-process` is requested.

Regression checks:

- Existing `ida55_split_asm.py` still parses the generated `.asm`.
- Existing function dump conversion still produces JSON.
- Existing match report still produces effective exact matches.
- Existing rename IDC generation still produces a non-empty manifest when applicable.

## Risks

- IDA 5.5 IDC error handling is limited; restore behavior may need a conservative generated restore script in addition to inline restoration.
- IDA 5.5 can destroy the packed `.idb` if automation continues through an unpacked-database prompt; `ida_automation\idaw.py` is mandatory for automation.
- Some options may be persisted in the IDB immediately after `Set*Prm`, so failed runs need manual restore support.
- Registry changes may not affect clean ASM output, but can affect script lookup and recent-directory behavior.
- Target assembler choice may affect parser stability and should be validated against current split/diff tools.

## Implementation Notes

- Keep `eg_idc/stu_config.idc` as historical notes.
- Do not mutate it into production automation.
- Prefer a new IDC with a narrow, auditable purpose.
- Keep all generated logs next to the IDB or under a dedicated export-log directory.
- Make destructive actions opt-in.
- Use `ida_automation\idaw.py` for every IDA launch.
- Recommended production command after IDBs are known-good:

```powershell
c:/python314/python.exe -B .\ida_dasm_export\ida55_clean_export.py --timeout-seconds 120 --copy
```

- Add `--post-process` only when the downstream pipeline should run immediately after copy.
