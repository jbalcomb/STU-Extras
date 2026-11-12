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
python -B ida55py\idaw.py [--ida-exe EXE] [--backup-dir DIR] [--dry-run] -- -A -L<log> -S<script.idc> <file.idb>
```

- Options for `idaw.py` go before `--`; everything after it is passed to IDA. Exactly one `.idb` is required.
- IDA executable: `--ida-exe`, else `%IDA55_EXE%`, else `C:\STU_DASM\IDA55\idaw.exe`.
- Backup folder: `--backup-dir`, else `%IDA55_BACKUP_DIR%`, else the IDB's own folder. Backups are named
  `<stem>-YYYYMMDDHHMM.idb`.
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
