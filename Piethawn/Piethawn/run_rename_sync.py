#!/usr/bin/env python3
"""
WZD -> MGC procedure-name sync, end to end: build the rename IDC from the dumps in in/, apply it to MAGIC.idb,
and verify.

  1. ida_proc_data_export/ida55_functions_to_inventory.py   -> out/ida55-procedure-snapshots/{MGC,WZD}.procedure-inventory.json
  2. ida55_asm_body_evidence.py                              -> {MGC,WZD}.asm-body-evidence.json
  3. ida55_join_procedure_evidence.py                        -> {MGC,WZD}.procedure-evidence.json
  4. ida_proc_name_sync_prereq/compare_procedure_evidence.py -> ida_proc_name_sync_prereq/MGC-vs-WZD.procedure-evidence-comparison.json
  5. ida_proc_name_sync_prereq/build_magic_name_sync_handoff.py -> ida_proc_name_sync/magic_sync_names_from_wzd.json
  6. ida_proc_name_sync/generate_magic_name_sync.py          -> ida_proc_name_sync/magic_sync_names_from_wzd.idc (GUI)
                                                    --batch -> ida_proc_name_sync/magic_sync_names_from_wzd.batch.idc
  7. Apply (skipped when the handoff has 0 renames): IDA55-Py/ida55py/idaw.py --work-dir --write-back runs the batch IDC
     on a copy of MAGIC.idb. The real MAGIC.idb is replaced (after a backup) only when every rename succeeded, the
     copy is healthy and the real IDB did not change meanwhile. Otherwise it is left alone and the work folder is kept.
     The IDA log is checked for collision / name-mismatch / not-found / rename-failed.
  8. Verify: re-export MAGIC (function .json and clean .asm, both on work copies) into in/, re-run steps 1-6,
     and require 0 renames.

Refresh in/ first with run_ida55_dump_and_split.py. Close every IDA window before steps 7-8.
--no-apply stops after step 6; --no-verify skips step 8.
"""

from __future__ import annotations

import argparse
import json
import os
import shutil
import subprocess
import sys
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Sequence

REPO_ROOT = Path(__file__).resolve().parent
SEGMENT_MAP = Path("ida_proc_name_sync_prereq") / "MGC-vs-WZD.segment-map.json"
COMPARISON = Path("ida_proc_name_sync_prereq") / "MGC-vs-WZD.procedure-evidence-comparison.json"
HANDOFF = Path("ida_proc_name_sync") / "magic_sync_names_from_wzd.json"
RENAME_IDC = Path("ida_proc_name_sync") / "magic_sync_names_from_wzd.idc"
BATCH_RENAME_IDC = Path("ida_proc_name_sync") / "magic_sync_names_from_wzd.batch.idc"
SNAPSHOTS = Path("out") / "ida55-procedure-snapshots"
DATABASES = (("MGC", "MAGIC"), ("WZD", "WIZARDS"))

IDAW_WRAPPER = Path("..") / ".." / "IDA55-Py" / "ida55py" / "idaw.py"
DEFAULT_IDB_DIR = r"C:\STU_DASM\IDA55_MOM131"
DEFAULT_IDA_EXE = r"C:\STU_DASM\IDA55\idaw.exe"
WORK_ROOT = Path("out") / "ida55-work"
LOG_DIR = Path("out") / "ida55-rename-sync-logs"
DEFAULT_TIMEOUT_SECONDS = 900
RENAME_RESULTS = ("renamed", "already-synced", "not-found", "name-mismatch", "collision", "rename-failed")
RENAME_PROBLEMS = ("not-found", "name-mismatch", "collision", "rename-failed")


class RenameSyncError(RuntimeError):
    pass


def parse_args(argv: Sequence[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Build the WZD -> MGC rename IDC from in/, apply it to MAGIC.idb and verify.")
    parser.add_argument("--dry-run", action="store_true", help="Print the steps without running them.")
    parser.add_argument("--no-apply", action="store_true", help="Stop after building the IDCs (steps 1-6).")
    parser.add_argument("--no-verify", action="store_true", help="Skip the re-export and 0-renames check after applying.")
    parser.add_argument("--keep-work", action="store_true", help="Keep the apply step's work folder after a successful write-back.")
    parser.add_argument("--idb-dir", default=DEFAULT_IDB_DIR, help=f"Folder with MAGIC.idb/WIZARDS.idb. Default: {DEFAULT_IDB_DIR}")
    parser.add_argument("--ida-exe", default=DEFAULT_IDA_EXE, help=f"IDA executable passed to idaw.py. Default: {DEFAULT_IDA_EXE}")
    parser.add_argument("--timeout-seconds", type=int, default=DEFAULT_TIMEOUT_SECONDS, help=f"Seconds to wait for each IDA run. Default: {DEFAULT_TIMEOUT_SECONDS}")
    parser.add_argument("--python", default=sys.executable, help="Python interpreter for child scripts. Default: current interpreter")
    return parser.parse_args(argv)


def required_inputs() -> List[Path]:
    paths = [SEGMENT_MAP]
    for _, game in DATABASES:
        paths += [Path("in") / f"{game}.asm", Path("in") / f"{game}.idb.ida55-functions.json"]
    return paths


def rename_chain_commands(python: str) -> List[List[str]]:
    """The six chain steps in order; every input and output path is explicit. Powered by Claude."""
    commands: List[List[str]] = []
    for database, game in DATABASES:
        inventory = SNAPSHOTS / f"{database}.procedure-inventory.json"
        body = SNAPSHOTS / f"{database}.asm-body-evidence.json"
        evidence = SNAPSHOTS / f"{database}.procedure-evidence.json"
        commands += [
            [python, "-B", str(Path("ida_proc_data_export") / "ida55_functions_to_inventory.py"),
             str(Path("in") / f"{game}.idb.ida55-functions.json"), "--database", database, "--output", str(inventory)],
            [python, "-B", "ida55_asm_body_evidence.py", str(Path("in") / f"{game}.asm"), "--database", database, "--output", str(body)],
            [python, "-B", "ida55_join_procedure_evidence.py", "--inventory", str(inventory), "--asm-body-evidence", str(body), "--output", str(evidence)],
        ]
    commands += [
        [python, "-B", str(Path("ida_proc_name_sync_prereq") / "compare_procedure_evidence.py"),
         "--left", str(SNAPSHOTS / "MGC.procedure-evidence.json"), "--right", str(SNAPSHOTS / "WZD.procedure-evidence.json"),
         "--segment-map", str(SEGMENT_MAP), "--output", str(COMPARISON)],
        [python, "-B", str(Path("ida_proc_name_sync_prereq") / "build_magic_name_sync_handoff.py"), "--comparison", str(COMPARISON), "--output", str(HANDOFF)],
        [python, "-B", str(Path("ida_proc_name_sync") / "generate_magic_name_sync.py"), "--renames", str(HANDOFF), "--output-idc", str(RENAME_IDC)],
    ]
    return commands


def batch_idc_command(python: str) -> List[str]:
    """Generate the command-line (Exit()-terminated) variant of the rename IDC. Powered by Claude."""
    return [python, "-B", str(Path("ida_proc_name_sync") / "generate_magic_name_sync.py"),
            "--renames", str(HANDOFF), "--output-idc", str(BATCH_RENAME_IDC), "--batch"]


def apply_command(python: str, ida_exe: str, idb_path: Path, work_dir: Path, backup_dir: Path, log_path: Path) -> List[str]:
    """idaw.py runs the batch IDC on a work copy of MAGIC.idb and writes it back only if healthy. Powered by Claude."""
    return [
        python, "-B", str((REPO_ROOT / IDAW_WRAPPER).resolve()),
        "--ida-exe", ida_exe, "--work-dir", str(work_dir), "--write-back", "--backup-dir", str(backup_dir),
        "--", "-A", f"-L{log_path}", f"-S{(REPO_ROOT / BATCH_RENAME_IDC).resolve()}", str(idb_path),
    ]


def verify_export_commands(python: str, timeout_seconds: int) -> List[List[str]]:
    """Re-export MAGIC's function .json and clean .asm into in/ (both on work copies). Powered by Claude."""
    shared = ["--game", "MAGIC", "--timeout-seconds", str(timeout_seconds), "--python", python]
    return [
        [python, "-B", str(Path("ida_proc_data_export") / "ida55_export_function_data.py"), *shared],
        [python, "-B", str(Path("ida_dasm_export") / "ida55_clean_export.py"), "--skip-registry-snapshot", *shared],
    ]


def rename_count(handoff_path: Path) -> int:
    """Number of renames listed in the handoff JSON. Powered by Claude."""
    return len(json.loads(handoff_path.read_text(encoding="utf-8")).get("renames", []))


def parse_rename_log(text: str, label: str = "magic-name-sync") -> Dict[str, object]:
    """Count the batch IDC's per-entry result lines and pick out its <label>-summary and <label>-saved lines. Powered by Claude."""
    counts = {result: 0 for result in RENAME_RESULTS}
    problems: List[str] = []
    summary = None
    saved = False
    for line in text.splitlines():
        line = line.strip()
        kind = line.split(",", 1)[0]
        if kind in counts:
            counts[kind] += 1
            if kind in RENAME_PROBLEMS:
                problems.append(line)
        elif line.startswith(f"{label}-summary,"):
            summary = line
        elif line == f"{label}-saved":
            saved = True
    return {"counts": counts, "problems": problems, "summary": summary, "saved": saved}


def terminate_process_tree(process: subprocess.Popen) -> None:
    """Kill a child process plus any grandchildren (idaw.py and the idaw.exe it started). Powered by Claude."""
    if os.name == "nt":
        subprocess.run(["taskkill", "/T", "/F", "/PID", str(process.pid)], capture_output=True, check=False)
    else:
        process.kill()
    try:
        process.wait(timeout=30)
    except subprocess.TimeoutExpired:
        process.kill()


def run_step(command: Sequence[str], dry_run: bool, timeout_seconds: int | None = None) -> int:
    """Print and run one step from the repo root; kill the whole process tree on timeout. Powered by Claude."""
    print("+", " ".join(map(str, command)), flush=True)
    if dry_run:
        return 0
    process = subprocess.Popen(list(map(str, command)), cwd=REPO_ROOT)
    try:
        return process.wait(timeout=timeout_seconds if timeout_seconds and timeout_seconds > 0 else None)
    except (subprocess.TimeoutExpired, KeyboardInterrupt):
        terminate_process_tree(process)
        raise


def run_chain(python: str, dry_run: bool) -> None:
    """Steps 1-6 plus the batch IDC. Powered by Claude."""
    for command in rename_chain_commands(python) + [batch_idc_command(python)]:
        result = run_step(command, dry_run)
        if result != 0:
            raise subprocess.CalledProcessError(result, command)


def apply_renames(args: argparse.Namespace, stamp: str) -> None:
    """Step 7: run the batch IDC on a work copy of MAGIC.idb with write-back, then check the IDA log. Powered by Claude."""
    idb_dir = Path(args.idb_dir)
    idb_path = idb_dir / "MAGIC.idb"
    work_dir = REPO_ROOT / WORK_ROOT / f"MAGIC-rename-{stamp}"
    log_path = REPO_ROOT / LOG_DIR / f"MAGIC-rename-{stamp}.ida.log"
    if not args.dry_run:
        log_path.parent.mkdir(parents=True, exist_ok=True)
    result = run_step(apply_command(args.python, args.ida_exe, idb_path, work_dir, idb_dir, log_path), args.dry_run, args.timeout_seconds)
    if args.dry_run:
        return

    report = parse_rename_log(log_path.read_text(encoding="utf-8", errors="replace") if log_path.exists() else "")
    print(f"IDA log: {log_path}")
    print(f"Rename results: {report['counts']}")
    if report["summary"]:
        print(report["summary"])
    for problem in report["problems"]:
        print(f"  problem: {problem}", file=sys.stderr)
    if result != 0 or report["problems"] or not report["summary"] or not report["saved"]:
        raise RenameSyncError(
            f"Rename apply failed (idaw.py exit code {result}). The real MAGIC.idb was not replaced unless "
            f"'Written back:' appears above. Work folder kept: {work_dir}. IDA log: {log_path}"
        )
    print(f"MAGIC.idb updated; backup made in {idb_dir}.")
    if args.keep_work:
        print(f"kept {work_dir}")
    else:
        shutil.rmtree(work_dir, ignore_errors=True)
        print(f"removed {work_dir}")


def verify(args: argparse.Namespace) -> None:
    """Step 8: re-export MAGIC into in/, re-run the chain, and require 0 renames. Powered by Claude."""
    for command in verify_export_commands(args.python, args.timeout_seconds):
        result = run_step(command, args.dry_run, args.timeout_seconds + 120)
        if result != 0:
            raise subprocess.CalledProcessError(result, command)
    run_chain(args.python, args.dry_run)
    if args.dry_run:
        return
    remaining = rename_count(REPO_ROOT / HANDOFF)
    if remaining:
        raise RenameSyncError(f"Verify failed: the handoff still lists {remaining} rename(s) after applying: {HANDOFF}")
    print("Verified: 0 renames remain.")


def main(argv: Sequence[str] | None = None) -> int:
    args = parse_args(argv)
    if not args.dry_run:
        missing = [str(path) for path in required_inputs() if not (REPO_ROOT / path).is_file()]
        if missing:
            raise FileNotFoundError("Missing inputs: " + ", ".join(missing))

    run_chain(args.python, args.dry_run)
    print(f"Rename IDC: {RENAME_IDC} (GUI), {BATCH_RENAME_IDC} (batch)")

    count = 0 if args.dry_run else rename_count(REPO_ROOT / HANDOFF)
    if not args.dry_run:
        print(f"Renames in handoff: {count}")
        if count == 0:
            print("Nothing to apply.")
            return 0
    if args.no_apply:
        print("--no-apply: stopping before IDA.")
        return 0

    stamp = datetime.now().strftime("%Y%m%d%H%M%S")
    apply_renames(args, stamp)
    if not args.no_verify:
        verify(args)
    print("Dry run complete." if args.dry_run else "Rename sync complete.")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except subprocess.CalledProcessError as error:
        print(f"ERROR: step failed with exit code {error.returncode}: {' '.join(map(str, error.cmd))}", file=sys.stderr)
        raise SystemExit(error.returncode or 1)
    except subprocess.TimeoutExpired as error:
        print(f"ERROR: timed out; only the work copy was at risk: {error}", file=sys.stderr)
        raise SystemExit(2)
    except (RenameSyncError, FileNotFoundError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        raise SystemExit(1)
