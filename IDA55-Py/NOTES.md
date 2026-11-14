# IDA 5.5 Scripting Notes

How IDA Pro 5.5 behaves when driven by IDC scripts from the command line. Moved here from
Piethawn's `doc/IDA55_IDC_NOTES.md` on 2026-11-11; the Piethawn file keeps the export-specific parts.

## Confirmed IDC Facts

- `Batch(1)` exists and suppresses dialogs; it returns the previous setting, so `old = Batch(1); ... Batch(old);` restores it.
- IDA 5.5's bundled `eg_idc/analysis.idc` documents command-line script execution as `idag -A -Sanalysis.idc file`.
- IDC arrays are persistent database objects and must be created with `CreateArray()` before `SetArrayLong()`/`GetArrayElement()` use. Prefer local `auto` variables in batch scripts: fewer things to fail on a first run.
- `GenerateFile(type, handle, ea1, ea2, flags)` returns the generated line count, or `-1` on error. Output types: `OFILE_MAP`, `OFILE_LST`, `OFILE_ASM`. `GENFLG_ASMTYPE | GENFLG_ASMINC` produces an INC file.
- `SetCharPrm`, `SetShortPrm` and `SetLongPrm` all exist; use the one matching the `INF_*` field's width (for example `INF_MAXREF` is `int32`, so `SetLongPrm`).
- An IDB keeps the path of the executable it was originally loaded from (`GetInputFilePath()`), even after the IDB is moved.

## Startup Prompts and the Console

- On first use, batch IDA stops at a startup support-plan prompt ending in `Thank you for using IDA Pro! -> Continue`, before any IDC runs. Accept the license once in the blue IDA window; launchers can also send an initial Enter to IDA's stdin.
- IDA 5.5 has a legacy console limitation with very large CMD screen buffers. With the default Screen Buffer Size of `9001` the probe did not run; at `300` it did. Keep the CMD Screen Buffer Size near `300` for automation.
- A run that hangs at a prompt keeps IDA alive with the database open; always use a timeout, and see "Killing IDA" below.

## GUI vs Batch IDC Scripts

An IDC script is written for one of two ways of running it, and the end of `main()` differs.

| | GUI (interactive) | Batch (command line) |
|---|---|---|
| How it is run | Open the IDB in the IDA window, File > IDC file... | `ida55py\idaw.py -- -A -L<log> -S<script> <idb>`, usually via a project's Python launcher |
| Who closes IDA | You | The script, with `Exit(code)` |
| `Exit()` | Must not call it: it closes your IDA window | Must call it on every path, or IDA stays open until the launcher's timeout kills it |
| Exit code | Not used | `0` = success, non-zero = failure; the launcher checks the process return code |
| Saving changes | `SaveBase("", 0)`, or choose Save when closing IDA | `SaveBase("", 0)` before `Exit()` if the script changes the database |
| Dialogs | `Batch(1)` while working is still useful | `Batch(1)` is required; any dialog stalls the run |
| Results | `Message(...)` to the Output window | `Message(...)` goes to the `-L` log; files written by the script |

Because `Exit()` behaves differently in the two modes, one IDC cannot serve both. A script needed both ways
should be generated in two variants, for example a generator `--batch` option that appends `SaveBase` (when
needed) and `Exit()`.

`idc/ida55_probe.idc` is the minimal batch example: `Batch(1)`, do the work, restore `Batch`, `Exit(0)`, and
`Exit(2)` on failure.

## Killing IDA

Killing IDA while a database is open, such as when a timeout fires on a script that never calls `Exit()`, can
leave a 0-byte `.idb` or unpacked `.id0/.id1/.nam/.til` parts. Two IDA sessions on the same database folder can
do the same. `idaw.py` refuses to start in either situation, backs up the packed IDB before each run, and checks
for both afterwards. Read-only export scripts lose nothing on a kill; a script that renames or recolors loses its
unsaved changes.

Prevention: run read-only scripts with `idaw.py --work-dir DIR`. IDA then works on a copy of the IDB, and a
kill can only damage the copy; delete `DIR` and re-run. On 2026-11-11 a timeout kill during IDA's exit
(mid re-pack) left a real `MAGIC.idb` at 0 bytes; with a work copy that would have cost nothing.

IDA 5.5's text-mode `idaw.exe` needs a real console window. Launched from a process without one (for example an
automation agent's shell), it can finish the IDC and then hang on exit. Run live IDA jobs from a normal cmd or
PowerShell window.

Recovery when the real IDB was used: see the steps `idaw.py` prints, and `ida55py\restore_ida_backup.py`.

## Registry vs IDB Settings

Clean ASM output formatting is controlled by the IDB/session `INF_*` settings (set with `Set*Prm` in the IDC),
not by the registry. `HKCU\Software\Hex-Rays\IDA` holds UI state, history and directories
(`snapshots/` has examples). Registry snapshots are still useful for recovering a UI configuration.

## First-Run Triage

Run the probe before anything that changes settings or generates files:

```powershell
python -B ida55py\idaw.py --backup-dir <dir> -- -A -L<log> -S<full path>\idc\ida55_probe.idc <file.idb>
```

Success: `<file.idb>.ida55-probe.txt` exists and contains `probe=ok`, and the process returns `0`.

If the probe fails, investigate command-line IDC loading, `-S` path handling, or `-L` compatibility. If IDA still
waits at the support-plan prompt, isolate whether the prompt reads from stdin or from an IDA-owned console window
(try with and without an initial Enter on stdin, and with and without `-L`).

## Dry-Run Meaning

A launcher's dry run proves:

- the Python script starts and parses arguments
- the IDA executable path and IDC script path resolve
- the requested IDB paths exist
- the exact IDA command can be constructed

It does not prove:

- IDA accepts the IDC script, or its syntax is valid inside IDA 5.5
- settings are restored
- output files are generated, non-empty or parseable
