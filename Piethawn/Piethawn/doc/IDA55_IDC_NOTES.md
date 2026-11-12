# IDA55 IDC Notes

Last updated: 2026-11-11.

Generic IDA 5.5 scripting notes (batch vs GUI IDCs, `Exit()`/`SaveBase`, startup prompts, the CMD screen-buffer
limit, killing IDA, first-run probe, dry-run meaning) moved to
[IDA55-Py/NOTES.md](../../../IDA55-Py/NOTES.md). The launcher (`idaw.py`), the backup restore, the probe and
settings IDCs, `eg_idc/` and the registry snapshots moved to [IDA55-Py](../../../IDA55-Py/README.md). This file
keeps what is specific to the MAGIC/WIZARDS exports.

## Source Files Consulted

- `IDA55-Py\eg_idc\stu_config.idc`
- `IDA55-Py\eg_idc\stu_produce.idc`
- `IDA55-Py\eg_idc\analysis.idc`
- `IDA55-Py\eg_idc\idc.idc`
- `IDA55-Py\idc\ida55_dump_settings.idc`
- `ida_proc_data_export\ida55_dump_functions.idc`
- `run_post_ida55_dump_pipeline.py`

## Confirmed For These Exports

- The clean export IDC uses `GenerateFile` with `OFILE_MAP`/`OFILE_LST`/`OFILE_ASM`, and `GENFLG_ASMTYPE | GENFLG_ASMINC` for the INC (see [IDA55-Py/NOTES.md](../../../IDA55-Py/NOTES.md)).
- `INF_MAXREF` is `int32`, so the clean export script uses `SetLongPrm`.
- The production clean export IDC no longer depends on IDC arrays; original settings are kept in local `auto` variables to reduce first-run failure surface.
- After accepting IDA's license prompt and setting the CMD Screen Buffer Size to `300`, the probe IDC executed and wrote `C:\STU_DASM\IDA55_MOM131\MAGIC.idb.ida55-probe.txt`.
- The successful probe reported `input_file_path=C:\moo\MAGICDV\MAGIC.EXE`, so the IDB still refers to its original loaded executable path while the database itself lives under `C:\STU_DASM\IDA55_MOM131`.
- The full clean export IDC has executed successfully for both `MAGIC` and `WIZARDS`, with process return code `0` for each game.
- The live clean export generated non-empty `MAP`, `ASM`, `INC`, and `LST` files for both games under `C:\STU_DASM\IDA55_MOM131`.
- The `before` and `after` settings snapshots matched for both games, so the clean export IDC restored the tracked `INF_*` settings after generation.
- `ida_dasm_export\ida55_check_clean_asm.py` validates the clean-export formatting contract before split/post-processing: no tabs, no IDA address line prefixes, no opcode-byte prefix columns, no XREF comments, no data/code border separator lines, nonzero procedures, and matching `proc`/`endp` counts.

## Current Inferences To Validate In IDA

- Both exporters launch IDA through `IDA55-Py\ida55py\idaw.py`, passing `--ida-exe C:\STU_DASM\IDA55\idaw.exe` and `--backup-dir C:\STU_DASM\IDA55_MOM131` (the `--idb-dir`). `idaw.py` makes the packed-IDB backup and checks for hard-stop database-state warnings.
- `INF_ASMTYPE = 1` should produce Borland TASM Ideal mode, matching the old notes, but parser stability should be compared with `INF_ASMTYPE = 0`.
- Suppressing comments with `SW_NOCMT` should reduce noisy ASM, but it may hide useful named comments for some reverse-engineering tasks.

## Current Clean Export Profile

| Setting | Value | Setter | Expected effect |
|---|---:|---|---|
| `INF_ENTAB` | `0` | `SetCharPrm` | Disable tabs |
| `INF_INDENT` | `0` | `SetCharPrm` | Remove instruction indentation |
| `INF_COMMENT` | `0` | `SetCharPrm` | Remove comment indentation |
| `INF_XREFNUM` | `0` | `SetCharPrm` | Suppress generated xrefs |
| `INF_SHOWPREF` | `0` | `SetCharPrm` | Hide line prefixes |
| `INF_BORDER` | `0` | `SetCharPrm` | Hide data/code border separator lines such as `; ---------------------------------------------------------------------------` |
| `INF_PREFFLAG` | `0` | `SetCharPrm` | Hide prefix subfields |
| `INF_CMTFLAG` | `SW_NOCMT` | `SetCharPrm` | Suppress comments |
| `INF_ORG` | `1` | `SetCharPrm` | Keep `org` directives |
| `INF_ASSUME` | `1` | `SetCharPrm` | Keep `assume` directives |
| `INF_BINPREF` | `0` | `SetShortPrm` | Suppress opcode byte prefixes |
| `INF_ASMTYPE` | `1` | `SetCharPrm` | Select TASM Ideal mode |
| `INF_MAXREF` | `0` | `SetLongPrm` | Suppress xref tail depth |

## What Does Not Work Yet

- The minimal probe IDC has executed inside IDA 5.5 in this environment.
- The full clean export IDC has executed inside IDA 5.5 for both `MAGIC` and `WIZARDS`.
- The Python orchestrator dry-run can verify command shape, but cannot prove IDA accepts every IDC statement.
- Restore behavior after a hard IDA process crash cannot be guaranteed; the generated restore IDC is the fallback.
- `C:\STU_DASM\IDA55_MOM131` now has fresh `MAP`/`ASM`/`INC`/`LST` outputs for both `MAGIC` and `WIZARDS`.
- IDA exit code `6` from the first live `MAGIC` run did not create `MAGIC.ida55-clean-export-before.txt`, so the working inference is that IDA failed before or very early in the IDC `main()` path.
- A later live probe run hung until Ctrl-C because IDA stayed alive at the startup `Continue` prompt; the Python wrapper now sends an initial Enter and times out each IDA run.
- The startup/license prompts appear to be environment state rather than IDC syntax failures; re-test after the one-time license acceptance before changing export settings.

## Test Results So Far

- `python3 -m py_compile ida_dasm_export\ida55_clean_export.py ida_dasm_export\ida55_copy_outputs.py` passes.
- `python3 -m unittest tests.test_ida55_clean_export` passes with deterministic tests covering path normalization, command construction, file verification, summary output, and log-tail helpers.
- `python3 ida_dasm_export\ida55_clean_export.py --dry-run --skip-registry-snapshot` resolves local default IDA and IDB paths, prints the two planned IDA batch commands, and now reports what worked, what was skipped, and what remains unproven.
- `python3 ida_dasm_export\ida55_clean_export.py --dry-run --skip-registry-snapshot --copy --post-process` prints IDA, copy, and post-processing commands without requiring generated exports to exist.
- `python3 ida_dasm_export\ida55_check_clean_asm.py in/MAGIC.asm in/WIZARDS.asm` passes against the copied clean exports.
- `python3 rasm_sync_translations.py` audits translated `.c` files against current split `.asm` files and the Gemini checklist in `HOWTO-DasmRasmPipeline.md`. It does not update `stub_wip_todo.md`; that file tracks IDA completion flags for MGC/WZD, not Gemini translation completion.
- `run_post_ida55_dump_pipeline.py` now removes existing split `.asm` files under `out/*/*/` before re-splitting, preserving translated `.c` files and other reports.

## Live Failure Triage

Run the minimal IDC probe first:

```powershell
c:/python314/python.exe .\ida_dasm_export\ida55_clean_export.py --game MAGIC --skip-registry-snapshot --script C:\STU\devel\STU-Extras\IDA55-Py\idc\ida55_probe.idc --skip-verify --timeout-seconds 30
```

Expected success signal:

- `C:\STU_DASM\IDA55_MOM131\MAGIC.idb.ida55-probe.txt` exists and contains `probe=ok`.
- `out\ida55-clean-export-logs\MAGIC.process.log` records the IDA process return code.
- `out\ida55-clean-export-logs\MAGIC.ida.log` records IDA's own log if IDA 5.5 accepts `-L`.

If the probe fails, see "First-Run Triage" in [IDA55-Py/NOTES.md](../../../IDA55-Py/NOTES.md) before changing export settings. To isolate the support-plan prompt, run the same command once without `--no-startup-enter` and once with `--no-ida-log`.

If the probe passes but clean export fails, inspect:

- `out\ida55-clean-export-logs\MAGIC.ida.log`
- `out\ida55-clean-export-logs\MAGIC.process.log`
- `C:\STU_DASM\IDA55_MOM131\MAGIC.ida55-clean-export-before.txt`
- `C:\STU_DASM\IDA55_MOM131\MAGIC.ida55-clean-export-active.txt`
- `C:\STU_DASM\IDA55_MOM131\MAGIC.ida55-clean-export-after.txt`

Once the probe passes, run the clean export:

```powershell
c:/python314/python.exe .\ida_dasm_export\ida55_clean_export.py --game MAGIC --copy --timeout-seconds 120
c:/python314/python.exe .\ida_dasm_export\ida55_clean_export.py --game WIZARDS --copy --timeout-seconds 120
```
c:/python314/python.exe -B .\ida_dasm_export\ida55_clean_export.py --timeout-seconds 120 --copy --post-process

After both clean exports pass, copy the outputs into the repository:

```powershell
c:/python314/python.exe .\ida_dasm_export\ida55_copy_outputs.py
```

After copying, run the post-IDA pipeline. This clears old split `.asm` files under `out/*/*/`, preserves translated `.c` files, checks clean ASM formatting, and re-splits the fresh IDA exports:

```powershell
c:/python314/python.exe .\run_post_ida55_dump_pipeline.py
```

If `ida_proc_name_sync\magic_sync_names_from_wzd.json` lists any renames, apply the generated rename IDC to `MAGIC.idb` in the IDA GUI: open `MAGIC.idb`, then File > IDC file... > `ida_proc_name_sync\magic_sync_names_from_wzd.idc`. It is generated by `ida_proc_name_sync\generate_magic_name_sync.py` as the last step of `run_post_ida55_dump_pipeline.py`. See [NAME_SYNC_PROCESS.md](../NAME_SYNC_PROCESS.md).

This IDC is a GUI-mode script: it calls `SaveBase` but not `Exit()`. Do not run it through `ida55_clean_export.py --script` or `idaw.py -A -S...` as it stands; see "GUI vs Batch IDC Scripts" in [IDA55-Py/NOTES.md](../../../IDA55-Py/NOTES.md).

The rename modifies `MAGIC.idb`; it does not update already-copied `in\*.asm` or already-split `out\*\*\*.asm` files. After the rename IDC applies changes, re-run the clean export, function data export, and post-IDA pipeline steps so the repository exports reflect the new procedure names. The handoff should then list 0 renames.

Superseded: earlier versions of these notes ran `out\ida55-sync\magic_rename_to_wizards.idc` from the command line. That IDC came from `ida55_generate_magic_rename_idc.py` (driven by `ida55_match_functions.py`), and both scripts have been removed. Its template called neither `SaveBase` nor `Exit()`, so a command-line run stayed alive until the timeout killed IDA, without saving the renames.

After splitting/post-processing, audit translated C files against current procedure names:

```powershell
c:/python314/python.exe .\rasm_sync_translations.py
```

## IDCs By Mode

What each mode requires is in "GUI vs Batch IDC Scripts" in [IDA55-Py/NOTES.md](../../../IDA55-Py/NOTES.md).

| IDC | Mode | Ends with |
|---|---|---|
| `IDA55-Py\idc\ida55_probe.idc` | Batch | `Exit(0)` / `Exit(2)` |
| `ida_dasm_export\ida55_clean_export.idc` | Batch | `Exit(0)` / `Exit(1)` (read-only export, no `SaveBase`) |
| `ida_proc_data_export\ida55_dump_functions.idc` | Batch | `Exit(0)` / `Exit(1)` (read-only export, no `SaveBase`) |
| `ida_proc_name_sync\magic_sync_names_from_wzd.idc` | GUI | `SaveBase`, `Batch(0)`, no `Exit()` |
| `ida_proc_status_sync\mgc_proc_status_sync.idc`, `wzd_proc_status_sync.idc` | GUI | neither `SaveBase` nor `Exit()`; choose Save when closing IDA ([COLOR_SYNC_PROCESS.md](../COLOR_SYNC_PROCESS.md)) |

The `ida55_dseg_dump.idc`, `ida55_ovr_func_map.idc` and `ida55_seg_attr_dump.idc` dumps are under `DEPRECATED\ida_proc_data_export\`. `ida55_dseg_dump.idc` had its `Exit()` calls removed for GUI use.
