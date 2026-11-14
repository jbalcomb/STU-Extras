# IDA55-Py

Scripting IDA Pro 5.5 from Python on Windows. Nothing here knows about a particular game or IDB; projects
(such as Piethawn) call it and pass their own paths.

Reference material (IDA 5.5 install, SDK, IDA-Python wrapper, config dumps, option screenshots):
`J:\STU_J\STU-IDA`.

## Contents

| Path | What |
|---|---|
| `ida55py/idaw.py` | Safe IDA launcher: refuses if IDA is already running or unpacked `.id0/.id1/.nam/.til` sit beside the IDB, backs up the packed `.idb`, runs `idaw.exe`, then checks for a 0-byte IDB and leftover unpacked parts |
| `ida55py/restore_ida_backup.py` | Restores an `.idb` from the newest nonzero `idaw.py` backup, keeping a `-damaged-` copy and checking sha256 |
| `ida55py/registry_dump.py` | Dumps IDA-related registry keys (Hex-Rays/Ilfak, uninstall, App Paths) to JSON |
| `ida_help/` | Turns `idahelp.chm` into greppable text plus page and IDC-function indexes. Copied from `ReMoM-Mod/tools/ida_help`, which is still the original for now |
| `idc/ida55_probe.idc` | Minimal batch IDC: writes `<idb>.ida55-probe.txt` and exits; proves command-line IDC execution works |
| `idc/ida55_dump_settings.idc` | Dumps the IDB's `INF_*` settings |
| `eg_idc/` | IDA 5.5's bundled sample IDCs plus `stu_*` experiments |
| `IDA55-IDC_Function_List.txt` | IDC function list from the IDA 5.5 help |
| `snapshots/` | Registry snapshots (`.reg`) and a `registry_dump.py` output |
| `NOTES.md` | How IDA 5.5 behaves when scripted: batch vs GUI IDCs, startup prompts, console limits |

## idaw.py

```powershell
python -B ida55py\idaw.py [--ida-exe EXE] [--work-dir DIR [--write-back]] [--backup-dir DIR] [--dry-run] -- -A -L<log> -S<script.idc> <file.idb>
```

- Options for `idaw.py` go before `--`; everything after it is passed to IDA. Exactly one `.idb` is required.
- IDA executable: `--ida-exe`, else `%IDA55_EXE%`, else `C:\STU_DASM\IDA55\idaw.exe`.
- **`--work-dir DIR` (use this for anything that only reads the database):** `idaw.py` copies the `.idb` into
  `DIR` (which must be new or empty) and runs IDA on the copy. The real IDB is never opened, so a hang, a timeout
  kill or a crash can only damage the copy; the fix is to delete `DIR`. IDCs that write next to `GetIdbPath()`
  write into `DIR`, so the caller collects its outputs from there. No backup is made, so `--backup-dir` is only
  accepted together with `--write-back`.
- **`--write-back` (with `--work-dir`, for scripts that change the database):** after IDA exits, the real IDB is
  backed up to `--backup-dir` and replaced with the copy, but only when IDA's exit code is 0, the copy is healthy,
  no IDA is running, and the real IDB has not changed since it was copied (size and mtime). The replace goes through
  a temporary file in the same folder and is checked by sha256. Otherwise the real IDB is left alone and `DIR` is
  kept for inspection. The script must end with `Exit(0)` only on full success.
- Without `--work-dir`, IDA runs on the real IDB after a backup to `--backup-dir`, else `%IDA55_BACKUP_DIR%`,
  else the IDB's own folder. Backups are named `<stem>-YYYYMMDDHHMM.idb`.
- Unsafe overrides: `--allow-running-ida`, `--allow-unpacked-parts`.
- On a damaged result it prints recovery steps, including the exact `restore_ida_backup.py` command, and
  changes nothing.

## restore_ida_backup.py

```powershell
python -B ida55py\restore_ida_backup.py --idb-dir DIR [--backup-dir DIR] [--idb NAME ...] [--apply]
```

- `--idb-dir` (or `%IDA55_IDB_DIR%`) is required. `--backup-dir` defaults to `%IDA55_BACKUP_DIR%`, else `--idb-dir`.
- `--idb` (alias `--game`) takes a stem or file name and repeats; default is every `.idb` in `--idb-dir`.
- Without `--apply` it only prints the plan. It refuses to overwrite a nonzero current IDB unless
  `--allow-overwrite-nonzero` is given.

## Tests

From this folder:

```powershell
python -B -m unittest tests.test_idaw tests.test_restore_ida_backup ida_help.tests.test_ida_help
```
