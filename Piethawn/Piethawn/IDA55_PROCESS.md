# IDA55 Process

This is the current Piethawn workflow after the manual IDA Pro 5.5 dumps exist.

## Scope

The automated process starts after these manual files have already been produced:

- `in/MAGIC.asm`
- `in/WIZARDS.asm`
- `in/MAGIC.idb.ida55-functions.txt`
- `in/WIZARDS.idb.ida55-functions.txt`

The main analysis pipeline stops when the rename IDC script for `MAGIC` has been generated. Completion-color synchronization and freshness checks are separate follow-on steps.

## Manual Steps In IDA 5.5

For each game:

1. Open the IDB in IDA Pro 5.5.
2. Export the full assembly listing to `in/MAGIC.asm` or `in/WIZARDS.asm`.
3. Run `ida55_dump_functions.idc`.
4. Save the resulting text dump as:
   - `in/MAGIC.idb.ida55-functions.txt`
   - `in/WIZARDS.idb.ida55-functions.txt`
5. If you want completion-state sync, also run `ida55_dump_function_colors.idc`.
6. Save the resulting color dumps as:
   - `in/MAGIC.idb.ida55-function-colors.txt`
   - `in/WIZARDS.idb.ida55-function-colors.txt`

The original executables are also expected at:

- `in/MAGIC.EXE`
- `in/WIZARDS.EXE`

## Automated Pipeline

Run:

```powershell
python -B .\run_post_ida55_dump_pipeline.py
```

The runner now prefers inputs under `in/` by default. If a file has not been moved yet, it also falls back to the repo root for compatibility.

Default stages:

1. Extract DOS EXE header JSON:
   - `exe_header_dump.py`
2. Extract Borland overlay metadata JSON:
   - `borland_ovrinfo_dump.py`
3. Split each full IDA ASM dump into per-segment and per-procedure files:
   - `ida55_split_asm.py`
4. Convert the function text dumps into JSON with byte hashes:
   - `ida55_function_dump.py`
5. Build the split-ASM duplicate index and normalized diff report:
   - `asm_dedupe_index.py`
   - `asm_diff_report.py`
6. Match functions using machine-code hashes plus positional segment overrides:
   - `ida55_match_functions.py`
7. Generate the `MAGIC` rename IDC script using `WIZARDS` as the canonical name source:
   - `ida55_generate_magic_rename_idc.py`
8. Generate pair-diff reports for:
   - `Save_SAVE_GAM`
   - `Load_SAVE_GAM`

This stage does not use function colors. It produces the base function-pairing data used by later color propagation.

## Important Matching Rule

The primary machine-code matcher is augmented by [ida55_exact_match_overrides.json](/c:/Users/jbalcomb/OneDrive%20-%20ID%20Networks,%20Inc/devel/Piethawn/ida55_exact_match_overrides.json), which pairs selected `MAGIC` and `WIZARDS` segments by function position, not just by function name.

That means:

- the first function in a mapped `MAGIC` segment is paired with the first function in the mapped `WIZARDS` segment
- the second function is paired with the second function
- and so on

This is how renamed-but-equivalent procedures such as `RP_CLK_ExtWait` and `UU_CLK_ExtWait` are treated as exact matches for synchronization purposes.

## Manual IDC Step

After the pipeline completes, run this inside `MAGIC.idb`:

- [out/ida55-sync/magic_rename_to_wizards.idc](/c:/Users/jbalcomb/OneDrive%20-%20ID%20Networks,%20Inc/devel/Piethawn/out/ida55-sync/magic_rename_to_wizards.idc)

This renames matched `MAGIC` functions to the corresponding `WIZARDS` names.

## Completion Color Workflow

When a function transcription is finished, mark the function in IDA with your completion color. The current merged tooling treats `00FFFF80` as the completion color.

Run these steps after you have new color dumps:

```powershell
python -B .\ida55_function_color_dump.py .\in\MAGIC.idb.ida55-function-colors.txt --output .\in\MAGIC.idb.ida55-function-colors.json
python -B .\ida55_function_color_dump.py .\in\WIZARDS.idb.ida55-function-colors.txt --output .\in\WIZARDS.idb.ida55-function-colors.json
python -B .\ida55_merge_completion_status.py
```

That produces:

- `out/ida55-sync/completion-merge.json`
- `out/ida55-sync/magic_sync_completion.idc`
- `out/ida55-sync/wizards_sync_completion.idc`

The merge uses the duplicate/equivalence graph from `out/ida55-function-match.json`, so completion state propagates across exact byte matches and positional override matches.

Run the generated IDC inside either IDB to copy completion status from the merged result back into IDA.

### Completion Sync Logging

The generated completion IDC scripts now log their work to:

- `MAGIC.idb.completion-sync.log`
- `WIZARDS.idb.completion-sync.log`

Log rows include:

- `missing,<name>`
- `already-set,<name>,<current_color>`
- `setcolor-failed,<name>,<old_color>,<new_color>`
- `updated,<name>,<old_color>,<new_color>`

## Freshness Tracking

Use the timestamp snapshot to record when the underlying IDA databases were last modified:

```powershell
python -B .\ida55_db_timestamp_config.py --ida-dir "C:\STU_DASM\IDA55_MOM131" --output .\project_config.json
```

This writes `ida55_database_timestamps` into `project_config.json`. The key field for pipeline freshness is:

- `ida55_database_timestamps.databases.<GAME>.latest_modified_utc`

If a database timestamp is newer than the last pipeline outputs derived from it, the dumps and downstream pipeline should be refreshed.

## Daily Refresh Snapshot Task

To register a daily Windows Scheduled Task for the database timestamp snapshot:

```powershell
python .\configure_daily_timestamp_task.py
```

Useful variants:

```powershell
python .\configure_daily_timestamp_task.py --dry-run
python .\configure_daily_timestamp_task.py --time 03:30
```

By default this creates the task:

- `Piethawn IDA55 Timestamp Snapshot`

which runs `ida55_db_timestamp_config.py` daily and updates `project_config.json`.

## Cleanup

To remove generated artifacts while keeping the final rename IDC:

```powershell
python -B .\clean_post_ida55_pipeline.py
```

That is a dry run. To actually delete the files:

```powershell
python -B .\clean_post_ida55_pipeline.py --apply
```

To clean everything generated after the manual dumps, including the final rename IDC:

```powershell
python -B .\clean_post_ida55_pipeline.py --stage post-dump --apply
```
