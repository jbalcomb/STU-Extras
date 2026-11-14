#!/usr/bin/env python3
"""
ReMoM -> WIZARDS.idb procedure-name sync, end to end (ReMoM is the naming authority).

  1. Export WIZARDS function data -> in/WIZARDS.idb.ida55-functions.json  (IDA on a work copy; --use-existing-export skips)
  2. Snapshot ReMoM's names       -> in/name-snapshots/remom/remom_index.tsv (+ .manifest.json with the ReMoM commit)
  3. Compare and validate         -> out/remom-name-sync/remom-vs-wzd.report.{json,md}, wzd_renames_from_remom.json
     HALT (exit 3) on any problem not reconciled in remom_name_sync/reconcile.json: ambiguous, multiple-functions,
     no-ida-proc, wrong-letter, invalid-name, duplicate-target, collision, temp-name-taken. Each problem is printed
     with its options, and out/remom-name-sync/problems.reconcile-template.json holds ready-to-fill entries.
  4. Write the rename IDCs        -> ida_proc_name_sync/wzd_sync_names_from_remom.idc (GUI) and .batch.idc
  5. Apply (skipped when 0 renames): idaw.py --work-dir --write-back runs the batch IDC on a copy of WIZARDS.idb;
     the real WIZARDS.idb is backed up and replaced only when every rename succeeded and the copy is healthy.
  6. Verify: export WIZARDS again, compare again, require 0 renames.
  7. MGC follows WZD: run_rename_sync.py carries the new WZD names to MAGIC.idb (--no-mgc skips).

Close every IDA window first. --report-only stops after step 3.
"""

from __future__ import annotations

import argparse
import json
import shutil
import subprocess
import sys
from datetime import datetime
from pathlib import Path
from typing import List, Sequence

REPO_ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(REPO_ROOT))

from ida_proc_name_sync.generate_magic_name_sync import load_renames, render_idc  # noqa: E402
from remom_name_sync import compare_remom_wzd as compare_mod  # noqa: E402
from remom_name_sync.remom_index import DEFAULT_OUT as INDEX_PATH, DEFAULT_REMOM, take_snapshot  # noqa: E402
from run_rename_sync import parse_rename_log, run_step  # noqa: E402

FUNCTIONS_JSON = Path("in") / "WIZARDS.idb.ida55-functions.json"
RECONCILE = compare_mod.DEFAULT_RECONCILE
OUT_DIR = compare_mod.DEFAULT_OUT_DIR
PLAN = OUT_DIR / compare_mod.PLAN_NAME
GUI_IDC = Path("ida_proc_name_sync") / "wzd_sync_names_from_remom.idc"
BATCH_IDC = Path("ida_proc_name_sync") / "wzd_sync_names_from_remom.batch.idc"
LABEL = "wzd-name-sync"
IDAW_WRAPPER = Path("..") / ".." / "IDA55-Py" / "ida55py" / "idaw.py"
DEFAULT_IDB_DIR = r"C:\STU_DASM\IDA55_MOM131"
DEFAULT_IDA_EXE = r"C:\STU_DASM\IDA55\idaw.exe"
WORK_ROOT = Path("out") / "ida55-work"
LOG_DIR = Path("out") / "remom-name-sync" / "logs"
DEFAULT_TIMEOUT_SECONDS = 900
HALT_EXIT_CODE = 3


class RemomSyncError(RuntimeError):
    pass


def parse_args(argv: Sequence[str] | None = None) -> argparse.Namespace:
    p = argparse.ArgumentParser(description="ReMoM -> WIZARDS.idb procedure-name sync (ReMoM wins).")
    p.add_argument("--remom", default=DEFAULT_REMOM, help=f"ReMoM checkout. Default: {DEFAULT_REMOM}")
    p.add_argument("--reconcile", type=Path, default=RECONCILE, help=f"Reconciliation file. Default: {RECONCILE}")
    p.add_argument("--report-only", action="store_true", help="Stop after the comparison report (step 3).")
    p.add_argument("--use-existing-export", action="store_true", help=f"Skip step 1 and use the existing {FUNCTIONS_JSON}.")
    p.add_argument("--no-verify", action="store_true", help="Skip step 6.")
    p.add_argument("--no-mgc", action="store_true", help="Skip step 7 (carrying the WZD names to MAGIC.idb).")
    p.add_argument("--keep-work", action="store_true", help="Keep the apply step's work folder after a successful write-back.")
    p.add_argument("--dry-run", action="store_true", help="Print the IDA steps instead of running them (steps 2-4 still run).")
    p.add_argument("--idb-dir", default=DEFAULT_IDB_DIR, help=f"Folder with WIZARDS.idb. Default: {DEFAULT_IDB_DIR}")
    p.add_argument("--ida-exe", default=DEFAULT_IDA_EXE, help=f"IDA executable passed to idaw.py. Default: {DEFAULT_IDA_EXE}")
    p.add_argument("--timeout-seconds", type=int, default=DEFAULT_TIMEOUT_SECONDS)
    p.add_argument("--python", default=sys.executable)
    return p.parse_args(argv)


def export_command(python: str, timeout_seconds: int) -> List[str]:
    """Export WIZARDS function data into in/ (IDA on a work copy). Powered by Claude."""
    return [python, "-B", str(Path("ida_proc_data_export") / "ida55_export_function_data.py"),
            "--game", "WIZARDS", "--timeout-seconds", str(timeout_seconds), "--python", python]


def apply_command(python: str, ida_exe: str, idb_path: Path, work_dir: Path, backup_dir: Path, log_path: Path) -> List[str]:
    """idaw.py runs the batch IDC on a work copy of WIZARDS.idb and writes it back only if healthy. Powered by Claude."""
    return [
        python, "-B", str((REPO_ROOT / IDAW_WRAPPER).resolve()),
        "--ida-exe", ida_exe, "--work-dir", str(work_dir), "--write-back", "--backup-dir", str(backup_dir),
        "--", "-A", f"-L{log_path}", f"-S{(REPO_ROOT / BATCH_IDC).resolve()}", str(idb_path),
    ]


def mgc_command(python: str) -> List[str]:
    """Carry the WZD names to MAGIC.idb with the existing WZD -> MGC rename sync. Powered by Claude."""
    return [python, "-B", "run_rename_sync.py", "--python", python]


def export_wizards(args: argparse.Namespace) -> None:
    """Steps 1 and 6a. Powered by Claude."""
    result = run_step(export_command(args.python, args.timeout_seconds), args.dry_run, args.timeout_seconds + 120)
    if result != 0:
        raise RemomSyncError(f"WIZARDS function export failed with exit code {result}.")


def run_compare(args: argparse.Namespace) -> dict:
    """Step 3. Powered by Claude."""
    result = compare_mod.compare(REPO_ROOT / INDEX_PATH, REPO_ROOT / FUNCTIONS_JSON, REPO_ROOT / args.reconcile, REPO_ROOT / OUT_DIR)
    compare_mod.print_summary(result)
    print(f"Report: {OUT_DIR / 'remom-vs-wzd.report.md'}")
    return result


def write_idcs() -> int:
    """Step 4: GUI and batch rename IDCs from the plan; returns the number of rename steps. Powered by Claude."""
    renames = load_renames(REPO_ROOT / PLAN)
    (REPO_ROOT / GUI_IDC).write_text(render_idc(renames, batch=False, label=LABEL), encoding="utf-8")
    (REPO_ROOT / BATCH_IDC).write_text(render_idc(renames, batch=True, label=LABEL), encoding="utf-8")
    print(f"Rename IDC: {GUI_IDC} (GUI), {BATCH_IDC} (batch); {len(renames)} step(s)")
    return len(renames)


def apply_renames(args: argparse.Namespace, stamp: str) -> None:
    """Step 5: run the batch IDC on a work copy of WIZARDS.idb with write-back, then check the IDA log. Powered by Claude."""
    idb_dir = Path(args.idb_dir)
    work_dir = REPO_ROOT / WORK_ROOT / f"WIZARDS-remom-rename-{stamp}"
    log_path = REPO_ROOT / LOG_DIR / f"WIZARDS-remom-rename-{stamp}.ida.log"
    if not args.dry_run:
        log_path.parent.mkdir(parents=True, exist_ok=True)
    result = run_step(apply_command(args.python, args.ida_exe, idb_dir / "WIZARDS.idb", work_dir, idb_dir, log_path),
                      args.dry_run, args.timeout_seconds)
    if args.dry_run:
        return
    report = parse_rename_log(log_path.read_text(encoding="utf-8", errors="replace") if log_path.exists() else "", LABEL)
    print(f"IDA log: {log_path}")
    print(f"Rename results: {report['counts']}")
    if report["summary"]:
        print(report["summary"])
    for problem in report["problems"]:
        print(f"  problem: {problem}", file=sys.stderr)
    if result != 0 or report["problems"] or not report["summary"] or not report["saved"]:
        raise RemomSyncError(
            f"Rename apply failed (idaw.py exit code {result}). The real WIZARDS.idb was not replaced unless "
            f"'Written back:' appears above. Work folder kept: {work_dir}. IDA log: {log_path}"
        )
    print(f"WIZARDS.idb updated; backup made in {idb_dir}.")
    if args.keep_work:
        print(f"kept {work_dir}")
    else:
        shutil.rmtree(work_dir, ignore_errors=True)


def main(argv: Sequence[str] | None = None) -> int:
    args = parse_args(argv)
    if not Path(args.remom).is_dir():
        raise RemomSyncError(f"ReMoM checkout not found: {args.remom}")
    compare_mod.load_reconcile(REPO_ROOT / args.reconcile)  # fail fast on a malformed reconcile file

    if args.use_existing_export:
        if not (REPO_ROOT / FUNCTIONS_JSON).is_file():
            raise RemomSyncError(f"--use-existing-export: {FUNCTIONS_JSON} does not exist.")
    else:
        export_wizards(args)

    rows = take_snapshot(args.remom, REPO_ROOT / INDEX_PATH)
    print(f"ReMoM snapshot: {INDEX_PATH} ({len(rows)} rows)")

    if args.dry_run and not (REPO_ROOT / FUNCTIONS_JSON).is_file():
        print(f"Dry run: {FUNCTIONS_JSON} does not exist yet, so the comparison is skipped.")
        return 0
    result = run_compare(args)
    if result["problems"]:
        return HALT_EXIT_CODE
    if args.report_only:
        return 0

    steps = write_idcs()
    if steps == 0:
        print("Nothing to apply: WIZARDS.idb already has ReMoM's names.")
    else:
        stamp = datetime.now().strftime("%Y%m%d%H%M%S")
        apply_renames(args, stamp)
        if not args.no_verify:
            export_wizards(args)
            if not args.dry_run:
                again = run_compare(args)
                remaining = len(json.loads((REPO_ROOT / PLAN).read_text(encoding="utf-8"))["renames"])
                if again["problems"] or remaining:
                    raise RemomSyncError(f"Verify failed: {remaining} rename step(s) and {len(again['problems'])} problem(s) remain.")
                print("Verified: WIZARDS.idb matches ReMoM (0 renames remain).")

    if not args.no_mgc:
        result = run_step(mgc_command(args.python), args.dry_run)
        if result != 0:
            raise RemomSyncError(f"run_rename_sync.py (WZD -> MGC) failed with exit code {result}.")
    print("Dry run complete." if args.dry_run else "ReMoM -> WZD name sync complete.")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except subprocess.TimeoutExpired as error:
        print(f"ERROR: timed out; only the work copy was at risk: {error}", file=sys.stderr)
        raise SystemExit(2)
    except (RemomSyncError, ValueError, FileNotFoundError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        raise SystemExit(1)
