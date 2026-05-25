# IDA55 IDC Notes

Last updated: 2026-04-27.

## Source Files Consulted

- `eg_idc/stu_config.idc`
- `eg_idc/stu_produce.idc`
- `eg_idc/analysis.idc`
- `eg_idc/idc.idc`
- `ida55_dump_settings.idc`
- `ida_proc_data_export\ida55_dump_functions.idc`
- `run_post_ida55_dump_pipeline.py`

## Confirmed From Local IDC Files

- `GenerateFile(type, handle, ea1, ea2, flags)` exists and returns generated line count or `-1` on error.
- Output file constants are available: `OFILE_MAP`, `OFILE_LST`, and `OFILE_ASM`.
- `GENFLG_ASMTYPE | GENFLG_ASMINC` is the known local pattern for INC generation.
- `SetCharPrm`, `SetShortPrm`, and `SetLongPrm` all exist.
- `INF_MAXREF` is documented as `int32`, so the clean export script uses `SetLongPrm`.
- `Batch(1)` exists and should suppress dialogs in automation.
- IDA 5.5's bundled `analysis.idc` documents command-line script execution as `idag -A -Sanalysis.idc file`.
- IDC arrays are persistent database objects and must be created with `CreateArray()` before `SetArrayLong()`/`GetArrayElement()` use.
- The production clean export IDC no longer depends on IDC arrays; original settings are kept in local `auto` variables to reduce first-run failure surface.
- `ida55_probe.idc` is a minimal command-line IDC probe that writes `*.idb.ida55-probe.txt` without changing export settings or generating MAP/ASM/INC/LST files.
- The live `MAGIC.ida.log` showed IDA stopping at the startup support-plan prompt ending in `Thank you for using IDA Pro! -> Continue`, before the probe IDC could prove script execution.
- After decreasing a CMD terminal's Screen Buffer Size from the default `9001` to `300` and accepting IDA's license prompt in the blue IDA window, the probe IDC did execute and wrote `C:\STU_DASM\IDA55_MOM131\MAGIC.idb.ida55-probe.txt`.
- IDA 5.5 has a known legacy console limitation with very large CMD screen buffers; keep the CMD Screen Buffer Size near `300` for these automation runs.
- The successful probe reported `input_file_path=C:\moo\MAGICDV\MAGIC.EXE`, so the IDB can still refer to its original loaded executable path while the database itself lives under `C:\STU_DASM\IDA55_MOM131`.
- The full clean export IDC has executed successfully for both `MAGIC` and `WIZARDS`, with process return code `0` for each game.
- The live clean export generated non-empty `MAP`, `ASM`, `INC`, and `LST` files for both games under `C:\STU_DASM\IDA55_MOM131`.
- The `before` and `after` settings snapshots matched for both games, so the clean export IDC restored the tracked `INF_*` settings after generation.
- `ida_dasm_export\ida55_check_clean_asm.py` validates the clean-export formatting contract before split/post-processing: no tabs, no IDA address line prefixes, no opcode-byte prefix columns, no XREF comments, no data/code border separator lines, nonzero procedures, and matching `proc`/`endp` counts.

## Current Inferences To Validate In IDA

- `ida_automation\idaw.py` is the shared launcher for automation; it invokes `idaw.exe` after making the required packed-IDB backup and checking for hard-stop database-state warnings.
- `INF_ASMTYPE = 1` should produce Borland TASM Ideal mode, matching the old notes, but parser stability should be compared with `INF_ASMTYPE = 0`.
- Suppressing comments with `SW_NOCMT` should reduce noisy ASM, but it may hide useful named comments for some reverse-engineering tasks.
- Registry snapshots are useful for recovery, but clean ASM formatting appears to be controlled by IDB/session `INF_*` settings.

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
c:/python314/python.exe .\ida_dasm_export\ida55_clean_export.py --game MAGIC --skip-registry-snapshot --script .\ida55_probe.idc --skip-verify --timeout-seconds 30
```

Expected success signal:

- `C:\STU_DASM\IDA55_MOM131\MAGIC.idb.ida55-probe.txt` exists and contains `probe=ok`.
- `out\ida55-clean-export-logs\MAGIC.process.log` records the IDA process return code.
- `out\ida55-clean-export-logs\MAGIC.ida.log` records IDA's own log if IDA 5.5 accepts `-L`.

If the probe fails, investigate command-line IDC loading, `-S` path handling, or `-L` compatibility before changing export settings.

If IDA still waits at the support-plan prompt, run the same command once without `--no-startup-enter` and once with `--no-ida-log`; the next distinction to isolate is whether the prompt reads from stdin or from an IDA-owned console/window.

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

If `out\ida55-sync\magic_rename_to_wizards.json` reports a non-zero `rename_count`, run the generated rename IDC against `MAGIC.idb` from the command line:

```powershell
c:/python314/python.exe .\ida_dasm_export\ida55_clean_export.py --game MAGIC --skip-registry-snapshot --script .\out\ida55-sync\magic_rename_to_wizards.idc --skip-verify --timeout-seconds 120
```

This modifies `MAGIC.idb`; it does not update already-copied `in\*.asm` or already-split `out\*\*\*.asm` files. After the rename IDC applies changes, re-run the clean export, copy, and post-IDA pipeline steps so the repository exports reflect the new procedure names.

After splitting/post-processing, audit translated C files against current procedure names:

```powershell
c:/python314/python.exe .\rasm_sync_translations.py
```

## Dry-Run Meaning

Dry-run proves:

- the Python script starts and parses arguments
- the IDA executable path resolves
- the IDC script exists
- the requested IDB paths exist when present
- the exact IDA commands can be constructed

Dry-run does not prove:

- IDA accepts the IDC script
- the IDC script syntax is valid inside IDA 5.5
- settings are restored
- `MAP`/`ASM`/`INC`/`LST` files are generated
- generated files are non-empty or parseable
