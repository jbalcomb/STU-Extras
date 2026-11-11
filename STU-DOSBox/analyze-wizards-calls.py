#!/usr/bin/env python3
"""
analyze-wizards-calls.py

Summarize and query the far-CALL reference log produced by the
og_far_call_probe in og_sort_trace.h.

Log line format (no [OG_FCALL] prefix in the file -- this is the
dedicated reference log):

    CCCC:IIII (src_seg:src_func+X)  ->  CCCC:OOOO (dst_seg:dst_func+Y)

Usage:
    analyze-wizards-calls.py                       -- summary (top callers, top callees, etc.)
    analyze-wizards-calls.py callers FUNC          -- everything that calls FUNC
    analyze-wizards-calls.py callees FUNC          -- everything FUNC calls
    analyze-wizards-calls.py sequence              -- chronological function-call sequence
    analyze-wizards-calls.py sequence FUNC         -- chronological calls to or from FUNC
    analyze-wizards-calls.py overlays              -- which overlays were observed executing
    analyze-wizards-calls.py file PATH             -- analyze a non-default log file

Default log path: /tmp/wizards_exe-calls.log
"""

import re
import sys
from collections import Counter, defaultdict
from pathlib import Path

DEFAULT_LOG = Path("/tmp/wizards_exe-calls.log")

# Each line:
#   KIND  CCCC:IIII (src_desc)  ->  CCCC:OOOO (dst_desc)
# where KIND is CALLF (far call 9A), CALLN (near call E8), or INT.
# *_desc is "seg:func+offset" or "seg:OOOO" or "?" or "INT NNh handler".
LINE_RE = re.compile(
    r'^(\S+)\s+'
    r'([0-9A-F]{4}):([0-9A-F]{4})\s+\(([^)]*)\)\s+->\s+'
    r'([0-9A-F]{4}):([0-9A-F]{4})\s+\(([^)]*)\)\s*$'
)


def parse_log(path: Path):
    """Yield (lineno, kind, src_cs, src_ip, src_desc, dst_cs, dst_ip, dst_desc)."""
    with path.open() as f:
        for ln, line in enumerate(f, 1):
            m = LINE_RE.match(line)
            if not m:
                continue
            yield (
                ln,
                m.group(1),
                int(m.group(2), 16), int(m.group(3), 16), m.group(4).strip(),
                int(m.group(5), 16), int(m.group(6), 16), m.group(7).strip(),
            )


def func_part(desc: str) -> str:
    """Reduce 'seg:func+offset' to 'seg:func'.  Returns desc unchanged
    if no '+' present (no labeled function)."""
    if "+" in desc:
        return desc.rsplit("+", 1)[0]
    return desc


def cmd_summary(records):
    total = len(records)
    sites = Counter()
    edges = Counter()
    callers = Counter()
    callees = Counter()
    src_overlays = Counter()
    dst_overlays = Counter()
    by_kind = Counter()
    int_vectors = Counter()
    for _, kind, scs, sip, sd, dcs, dip, dd in records:
        by_kind[kind] += 1
        sites[(scs, sip)] += 1
        edges[(func_part(sd), func_part(dd))] += 1
        callers[func_part(sd)] += 1
        callees[func_part(dd)] += 1
        if sd.startswith("ovr"):
            src_overlays[sd.split(":", 1)[0]] += 1
        if dd.startswith("ovr"):
            dst_overlays[dd.split(":", 1)[0]] += 1
        if kind == "INT":
            int_vectors[dd] += 1

    print(f"Total control-transfer executions:   {total}")
    for kind, n in sorted(by_kind.items()):
        print(f"  {kind:<6s}  {n}")
    print(f"Unique (src_cs, src_ip) call sites:  {len(sites)}")
    print(f"Unique (caller, callee) edges:       {len(edges)}")
    print()
    if int_vectors:
        print(f"INT vectors observed (by # executions):")
        for v, n in int_vectors.most_common(10):
            print(f"  {n:>7d}  {v}")
        print()
    print(f"Top 15 callers (by # outgoing executions):")
    for fn, n in callers.most_common(15):
        print(f"  {n:>7d}  {fn}")
    print()
    print(f"Top 15 callees (by # incoming far-call executions):")
    for fn, n in callees.most_common(15):
        print(f"  {n:>7d}  {fn}")
    print()
    print(f"Overlay segments observed as call SOURCE (i.e., executing):")
    for seg, n in sorted(src_overlays.items()):
        print(f"  {seg:<10s}  {n:>7d}")
    print()
    print(f"Overlay segments observed as call DESTINATION (loaded & called into):")
    for seg, n in sorted(dst_overlays.items()):
        print(f"  {seg:<10s}  {n:>7d}")


def cmd_callers(records, target):
    """All distinct (caller, line-of-first-occurrence) where dst func name contains target."""
    hits = []
    seen = set()
    for ln, kind, scs, sip, sd, dcs, dip, dd in records:
        if target.lower() not in dd.lower():
            continue
        key = (sd, dd)
        if key in seen:
            continue
        seen.add(key)
        hits.append((ln, kind, scs, sip, sd, dcs, dip, dd))
    print(f"Distinct callers of '{target}':  {len(hits)} edges")
    for ln, kind, scs, sip, sd, dcs, dip, dd in hits:
        print(f"  line {ln:>6d}:  {kind:<5s}  {sd}  ->  {dd}  (runtime {scs:04X}:{sip:04X} -> {dcs:04X}:{dip:04X})")


def cmd_callees(records, target):
    hits = []
    seen = set()
    for ln, kind, scs, sip, sd, dcs, dip, dd in records:
        if target.lower() not in sd.lower():
            continue
        key = (sd, dd)
        if key in seen:
            continue
        seen.add(key)
        hits.append((ln, kind, scs, sip, sd, dcs, dip, dd))
    print(f"Distinct callees of '{target}':  {len(hits)} edges")
    for ln, kind, scs, sip, sd, dcs, dip, dd in hits:
        print(f"  line {ln:>6d}:  {kind:<5s}  {sd}  ->  {dd}  (runtime {scs:04X}:{sip:04X} -> {dcs:04X}:{dip:04X})")


def cmd_sequence(records, target=None):
    """Chronological call sequence (every execution)."""
    for ln, kind, scs, sip, sd, dcs, dip, dd in records:
        if target is not None:
            if target.lower() not in sd.lower() and target.lower() not in dd.lower():
                continue
        print(f"  {ln:>6d}:  {kind:<5s}  {sd}  ->  {dd}")


def cmd_overlays(records):
    """Show which overlays appeared, in order of first appearance,
    distinguishing 'executing' (appeared as src) from 'called into'
    (appeared as dst only)."""
    first_src = {}
    first_dst = {}
    for ln, _kind, _, _, sd, _, _, dd in records:
        if sd.startswith("ovr"):
            seg = sd.split(":", 1)[0]
            if seg not in first_src:
                first_src[seg] = ln
        if dd.startswith("ovr"):
            seg = dd.split(":", 1)[0]
            if seg not in first_dst:
                first_dst[seg] = ln
    print(f"Overlays observed as call SOURCE (executing, line of first appearance):")
    for seg, ln in sorted(first_src.items(), key=lambda kv: kv[1]):
        print(f"  line {ln:>6d}:  {seg}")
    print()
    print(f"Overlays observed as call DESTINATION ONLY (loaded but not seen as caller yet):")
    only_dst = sorted(set(first_dst) - set(first_src),
                      key=lambda s: first_dst[s])
    for seg in only_dst:
        print(f"  line {first_dst[seg]:>6d}:  {seg}")


def main(argv):
    log_path = DEFAULT_LOG
    args = list(argv[1:])
    if args and args[0] == "file":
        if len(args) < 3:
            print("usage: ... file PATH SUBCOMMAND ...", file=sys.stderr)
            return 2
        log_path = Path(args[1])
        args = args[2:]

    if not log_path.exists():
        print(f"ERROR: log file not found: {log_path}", file=sys.stderr)
        return 2

    records = list(parse_log(log_path))
    if not records:
        print(f"ERROR: no parseable lines in {log_path}", file=sys.stderr)
        return 2

    if not args:
        cmd_summary(records)
        return 0
    cmd = args[0]
    rest = args[1:]
    if cmd == "summary":
        cmd_summary(records)
    elif cmd == "callers":
        if not rest:
            print("usage: ... callers FUNCNAME", file=sys.stderr); return 2
        cmd_callers(records, rest[0])
    elif cmd == "callees":
        if not rest:
            print("usage: ... callees FUNCNAME", file=sys.stderr); return 2
        cmd_callees(records, rest[0])
    elif cmd == "sequence":
        cmd_sequence(records, rest[0] if rest else None)
    elif cmd == "overlays":
        cmd_overlays(records)
    else:
        print(__doc__, file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
