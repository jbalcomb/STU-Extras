# IDA55 Clean Export Tasks

Last updated: 2026-04-26.

## Phase 1: Capture And Restore Settings

- [x] Create production IDC script separate from `eg_idc/stu_config.idc`.
- [x] Dump export-relevant settings before mutation.
- [x] Write a restore IDC before mutation.
- [x] Apply the clean export profile.
- [x] Restore original settings after export.
- [x] Dump settings after restore.
- [ ] Run inside IDA 5.5 against a disposable/test IDB.
- [ ] Confirm before/after settings match in IDA output files.

## Phase 2: Produce Clean Output Files

- [x] Generate `MAP`, `ASM`, `INC`, and `LST` in the IDB directory.
- [x] Log `GenerateFile` return values.
- [x] Add output verification in the Python orchestrator.
- [ ] Run export against `MAGIC.idb`.
- [ ] Run export against `WIZARDS.idb`.
- [ ] Confirm generated ASM still parses with `ida55_split_asm.py`.

## Phase 3: Python Orchestration

- [x] Add `ida_dasm_export\ida55_clean_export.py`.
- [x] Add dry-run mode.
- [x] Add IDA executable resolution.
- [x] Add IDB discovery/defaults for `MAGIC` and `WIZARDS`.
- [x] Add optional registry snapshots.
- [x] Add optional copy step.
- [x] Add optional post-processing step.
- [ ] Validate actual IDA launch command on Windows.
- [x] Route IDA launches through shared `ida_automation\idaw.py` wrapper instead of raw `idaw.exe`.

## Phase 4: Copy Outputs Into Repo

- [x] Reuse `ida_dasm_export\ida55_copy_outputs.py`.
- [ ] Decide whether function and color dumps belong in the copy step.
- [ ] Decide whether copied filename casing should be normalized.
- [ ] Confirm copy behavior after fresh IDA exports.

## Phase 5: Post-Process Into Segments And Procedures

- [x] Wire optional `--post-process` to `run_post_ida55_dump_pipeline.py`.
- [ ] Run post-processing after fresh copied exports.
- [ ] Confirm `out/MAGIC/...` and `out/WIZARDS/...` are refreshed.
- [ ] Confirm `ida_proc_data_export\ida55_function_dump.py` handles freshly exported function dumps.

## Phase 6: Diffs And MGC/WZD Update Scripts

- [x] Reuse existing post-processing runner for matching and rename IDC generation.
- [ ] Confirm `out/asm-dedupe-index.json` refreshes.
- [ ] Confirm `out/asm-diff-report/...` refreshes.
- [ ] Confirm `out/ida55-sync/magic_rename_to_wizards.idc` refreshes.
- [ ] Confirm completion sync scripts still generate when color dumps are refreshed.

## Deterministic Tests

- [x] Syntax-check new Python code.
- [x] Unit-test path normalization and command generation.
- [x] Unit-test expected output names.
- [x] Unit-test missing IDA executable counterexample.
- [x] Unit-test missing output verification counterexample.
- [ ] Run real IDA 5.5 export smoke test on Windows.
- [ ] Compare real settings before/after dumps from IDA.
