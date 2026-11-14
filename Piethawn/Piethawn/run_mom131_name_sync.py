#!/usr/bin/env python3
"""
ReMoM -> mom131-source-map function-name sync, end to end (ReMoM is the naming authority).

The renames go into a checkout of BinAlgo's repo -- by default the ReMoM repo's binaryalgorithm-bridge branch in its
own worktree, C:\\STU\\devel\\mom131-bridge (ReMoM tools/upstream_bridge.py keeps that branch: their main + ONE
bridge commit of ours, pushing to them disabled). Nothing is committed unless --commit is given.

  1. mom131 functions  -> in/name-snapshots/mom131/mom131_functions.tsv, read from the source (MODULES.TSV segments,
                           definition order; needs in/{WIZARDS,MAGIC}.idb.ida55-functions.json)
  2. ReMoM functions   -> in/name-snapshots/remom/remom_index.tsv, remom_functions.tsv
  3. Name map          -> in/name-snapshots/mom131-remom.name-map.tsv
  4. Compare           -> out/mom131-name-sync/mom131-vs-remom.report.{json,md}, mom131_renames_from_remom.json
     HALT (exit 3) on any problem not reconciled in mom131_name_sync/reconcile.json; each is printed with its options
     and out/mom131-name-sync/problems.reconcile-template.json holds ready-to-fill entries.
  5. Apply the renames to the checkout (it must have no uncommitted changes)
  6. Verify: build (build/build.py, DOSBox-X), both EXEs must stay identical to v1.31; steps 1-4 again, 0 renames
     must remain (the name map then shows the new names)
  7. --commit: fold the change into the bridge commit (upstream_bridge.py commit)

--report-only stops after step 4. On a failed verify the renamed checkout is kept for a look;
`git -C <checkout> checkout -- .` undoes it.
"""

from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path
from typing import List, Sequence

REPO_ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(REPO_ROOT))

from mom131_name_sync import compare_mom131_remom as compare_mod  # noqa: E402
from mom131_name_sync.apply_mom131_renames import ApplyError, apply_plan, dirty  # noqa: E402
from mom131_name_sync import mom131_functions, name_map, remom_todo  # noqa: E402
from mom131_name_sync.mom131_names import DEFAULT_SOURCE  # noqa: E402
from remom_name_sync import remom_functions  # noqa: E402
from remom_name_sync.remom_index import DEFAULT_OUT as REMOM_INDEX, DEFAULT_REMOM, take_snapshot as take_remom_snapshot  # noqa: E402

DEFAULT_BC = r"C:\STU\devel\mom131-source-map\tools"
DEFAULT_DOSBOX = r"C:\DOSBox-X\dosbox-x.exe"
UPSTREAM_BRIDGE = Path(DEFAULT_REMOM) / "tools" / "upstream_bridge.py"
OUT_DIR = compare_mod.DEFAULT_OUT_DIR
HALT_EXIT_CODE = 3
BUILD_TIMEOUT_SECONDS = 1800


class Mom131SyncError(RuntimeError):
    pass


def parse_args(argv: Sequence[str] | None = None) -> argparse.Namespace:
    p = argparse.ArgumentParser(description="ReMoM -> mom131-source-map function-name sync (ReMoM wins).")
    p.add_argument("--source", type=Path, default=Path(DEFAULT_SOURCE), help=f"mom131-source-map checkout. Default: {DEFAULT_SOURCE}")
    p.add_argument("--remom", default=DEFAULT_REMOM, help=f"ReMoM checkout. Default: {DEFAULT_REMOM}")
    p.add_argument("--reconcile", type=Path, default=compare_mod.DEFAULT_RECONCILE)
    p.add_argument("--bc", default=DEFAULT_BC, help=f"Folder holding BORLANDC (install_bc31.py). Default: {DEFAULT_BC}")
    p.add_argument("--dosbox", default=DEFAULT_DOSBOX, help=f"dosbox-x executable. Default: {DEFAULT_DOSBOX}")
    p.add_argument("--report-only", action="store_true", help="Stop after the comparison (step 4).")
    p.add_argument("--no-verify", action="store_true", help="Skip step 6.")
    p.add_argument("--commit", action="store_true", help="Step 7: fold the renames into the bridge commit.")
    p.add_argument("--message", default="Function names from ReMoM (Piethawn run_mom131_name_sync.py)")
    p.add_argument("--python", default=sys.executable)
    return p.parse_args(argv)


def build_command(args: argparse.Namespace) -> List[str]:
    """build/build.py of the checkout. Powered by Claude."""
    return [args.python, str(args.source / "build" / "build.py"), "--bc", args.bc, "--dosbox", args.dosbox]


def build(args: argparse.Namespace) -> None:
    """Build the checkout; require both EXEs identical to v1.31 (build.py exits 0 only then). Powered by Claude."""
    print(f"Building {args.source} (about 4 minutes) ...", flush=True)
    result = subprocess.run(build_command(args), cwd=args.source, capture_output=True, text=True, timeout=BUILD_TIMEOUT_SECONDS)
    tail = [line for line in result.stdout.splitlines() if "EXE" in line or "compiled" in line]
    for line in tail:
        print("  " + line)
    if result.returncode != 0:
        print(result.stdout[-3000:] + result.stderr[-2000:], file=sys.stderr)
        raise Mom131SyncError(f"build/build.py failed or an EXE is not identical (exit {result.returncode}).")


def snapshot_and_compare(args: argparse.Namespace) -> dict:
    """Steps 1-4. Powered by Claude."""
    m_path, r_path = REPO_ROOT / mom131_functions.DEFAULT_OUT, REPO_ROOT / remom_functions.DEFAULT_OUT
    rows, manifest = mom131_functions.take_snapshot(args.source, m_path, REPO_ROOT)
    for exe, info in manifest["programs"].items():
        c = info["counts"]
        check = ("not checked: the .MAP is older than the source" if info.get("map_stale")
                 else f".MAP check ok {c['map_ok']}, differs {c['map_differs']}")
        print(f"mom131 {exe}: {c['functions']} functions, {c['functions'] - c['not_placed']} placed; {check}")
        if c["map_differs"]:
            raise Mom131SyncError(f"{exe}: {c['map_differs']} function(s) where the .MAP disagrees with the source order; see {m_path}")
    remom_rows = take_remom_snapshot(args.remom, REPO_ROOT / REMOM_INDEX)
    remom_functions.take_snapshot(args.remom, r_path, REPO_ROOT)
    print(f"ReMoM: {REMOM_INDEX} ({len(remom_rows)} rows)")
    for bad in json.loads(Path(str(REPO_ROOT / REMOM_INDEX) + ".manifest.json").read_text(encoding="utf-8")).get("malformed_ids", []):
        print(f"  mistyped id comment in ReMoM: {bad['file']}:{bad['line']}  {bad['text']}")
    for tag in json.loads(Path(str(REPO_ROOT / REMOM_INDEX) + ".manifest.json").read_text(encoding="utf-8")).get("misplaced_tags", []):
        print(f"  tag not above its function in ReMoM: {tag['file']}:{tag['line']}  {tag['id']} (bound to {tag['function']} elsewhere)")
    mapped = name_map.join(name_map.read_rows(m_path), name_map.read_rows(r_path))
    name_map.write_rows(mapped, REPO_ROOT / name_map.DEFAULT_OUT, name_map.COLUMNS)
    print(f"Name map: {name_map.DEFAULT_OUT} ({len(mapped)} rows)")
    result = compare_mod.compare(m_path, REPO_ROOT / REMOM_INDEX, REPO_ROOT / args.reconcile,
                                 REPO_ROOT / OUT_DIR, args.source, Path(args.bc) / "BORLANDC" / "INCLUDE")
    compare_mod.print_summary(result)
    print(f"Report: {OUT_DIR / 'mom131-vs-remom.report.md'}")
    sizes = remom_todo.write_todo(REPO_ROOT / name_map.DEFAULT_OUT, REPO_ROOT / args.reconcile, REPO_ROOT / remom_todo.DEFAULT_OUT)
    print(f"ReMoM todo: {remom_todo.DEFAULT_OUT} (part 1 {sizes['part1']}, part 2 {sizes['part2']}, part 3 {sizes['part3']})")
    return result


def main(argv: Sequence[str] | None = None) -> int:
    args = parse_args(argv)
    if not (args.source / "build" / "build.py").is_file():
        raise Mom131SyncError(f"{args.source} is not a mom131-source-map checkout (no build/build.py).")
    compare_mod.load_reconcile(REPO_ROOT / args.reconcile)  # fail fast on a malformed reconcile file
    result = snapshot_and_compare(args)
    if result["problems"]:
        return HALT_EXIT_CODE
    if args.report_only:
        return 0
    if not result["plan"]:
        print("Nothing to rename: the checkout already has ReMoM's names.")
    else:
        changes = dirty(args.source)
        if changes:
            raise Mom131SyncError(f"{args.source} has uncommitted changes ({len(changes)} file(s)); commit or stash them first.")
        plan = json.loads((REPO_ROOT / OUT_DIR / compare_mod.PLAN_NAME).read_text(encoding="utf-8"))
        applied = apply_plan(args.source, plan)
        print(f"Renamed {sum(applied['files'].values())} occurrence(s) of {applied['renames']} function(s) in {len(applied['files'])} file(s).")
        for old, where in applied["left_in_shared"]:
            print(f"  left {old} in {where} (WZD and MGC rename it differently)")
        for old, where in applied["left_in_docs"]:
            print(f"  left {old} at {where} (WZD and MGC name it differently; the mention does not say which)")
        if not args.no_verify:
            build(args)
            again = snapshot_and_compare(args)
            if again["problems"] or again["plan"]:
                raise Mom131SyncError(f"Verify failed: {len(again['plan'])} rename(s) and {len(again['problems'])} problem(s) remain. "
                                      f"Undo with: git -C {args.source} checkout -- .")
            print("Verified: both EXEs identical, and the checkout has ReMoM's names (0 renames remain).")
    if args.commit:
        result = subprocess.run([args.python, str(UPSTREAM_BRIDGE), "commit", "-m", args.message], cwd=args.source)
        if result.returncode != 0:
            raise Mom131SyncError(f"upstream_bridge.py commit failed (exit {result.returncode}).")
    print("ReMoM -> mom131-source-map name sync complete." + ("" if args.commit else " Not committed (--commit)."))
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except subprocess.TimeoutExpired as error:
        print(f"ERROR: timed out: {error}", file=sys.stderr)
        raise SystemExit(2)
    except (Mom131SyncError, ApplyError, ValueError, FileNotFoundError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        raise SystemExit(1)
