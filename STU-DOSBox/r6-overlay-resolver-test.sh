#!/usr/bin/env bash
#
# r6-overlay-resolver-test.sh
#
# Two-part self-contained test for the R6 overlay-buffer byte-pattern
# resolver in og_sort_trace.h.  Designed to be run unattended (timeout-
# capped); no user input or game scenario required.
#
# PART A -- algorithm test (Python, no DOSBox):
#   Loads WIZARDS.EXE + WZD-overlay-table.fwv directly.  For each of
#   the four user-confirmed Stage 2 _unit writer reference cases
#   ((seg, ip) -> func), reads the 32-byte window at file_offset+ip
#   and searches every ovrXXX segment for it; asserts the match is
#   unique and lands on the expected segment.  This validates the
#   resolver's MATCHING ALGORITHM.
#
# PART B -- live DOSBox smoke (timeout-capped):
#   Boots WIZARDS.EXE directly under DOSBox with a hard timeout.
#   Asserts: build healthy, layer A (DGROUP) detects, layer B
#   (__EXENAME__) identifies WIZARDS, no "not yet identified" misses,
#   no spurious EXE-CHANGED rebinds.
#
# What this test does NOT cover:
#   The live byte-pattern path (when resident-image resolution lands
#   in an ovrXXX and og_overlay_seg_for_cs is invoked) only fires when
#   trace probes capture CS:IP inside an overlay buffer.  In WIZARDS'
#   no-input startup window all captured fires are resident-image
#   stubs.  To exercise the live R6 path you need the Stage 2
#   interactive flow (launch-stage2.sh) where AI code in overlay
#   buffers writes _unit.
#
# Usage:  r6-overlay-resolver-test.sh [timeout-seconds]   (default 15)

set -uo pipefail

DOSBOX=/home/jbalcomb/STU/developp/dosbox-0.74-3/src/dosbox
HERE=$(dirname "$(readlink -f "$0")")
CONF="$HERE/r6-test.conf"
FWV="$HERE/WZD-overlay-table.fwv"
WZD=/home/jbalcomb/STU_DBWD/MoM131/WIZARDS.EXE
LOG=/tmp/r6-overlay-resolver-test.log
TIMEOUT=${1:-20}    # 20s reliably reaches Init_Overland and the home-city prompt

[[ -x "$DOSBOX" ]] || { echo "ERROR: missing DOSBox at $DOSBOX" >&2; exit 2; }
[[ -f "$CONF"   ]] || { echo "ERROR: missing conf at $CONF" >&2; exit 2; }
[[ -f "$FWV"    ]] || { echo "ERROR: missing .fwv at $FWV"   >&2; exit 2; }
[[ -f "$WZD"    ]] || { echo "ERROR: missing WIZARDS.EXE at $WZD" >&2; exit 2; }

export TERM="${TERM:-xterm}"
[[ "$TERM" == "dumb" ]] && export TERM=xterm

fail=0
PASS() { echo "  PASS  $*"; }
FAIL() { echo "  FAIL  $*"; fail=$((fail+1)); }
INFO() { echo "  ----  $*"; }

# ============================================================
echo "=========================================================="
echo " PART A -- R6 algorithm test (static EXE byte-pattern)"
echo "=========================================================="
python3 - "$WZD" "$FWV" <<'PY'
import re, sys, os

WZD = sys.argv[1]
FWV = sys.argv[2]

with open(WZD, 'rb') as f:
    data = f.read()
hdr = (data[0x08] | (data[0x09] << 8)) * 16

segs = {}
with open(FWV) as f:
    for ln in f:
        m = re.match(r'^[0-9A-F]{4} ([0-9A-F]{8}) (\S+)', ln)
        if not m: continue
        ea, nm = int(m.group(1),16), m.group(2)
        segs.setdefault(nm, ea)

sorted_by_ea = sorted(segs.items(), key=lambda x: x[1])
file_off, seg_size = {}, {}
for i, (nm, ea) in enumerate(sorted_by_ea):
    file_off[nm] = ea - 0x10000 + hdr
    if i+1 < len(sorted_by_ea):
        seg_size[nm] = sorted_by_ea[i+1][1] - ea
    else:
        seg_size[nm] = len(data) - file_off[nm]

cases = [
    ("ovr051", 0x010D, "Init_Overland"),
    ("ovr062", 0x0266, "Sort_Unit_Stack"),
    ("ovr061", 0x0B1A, "Next_Unit_Nearest_Available"),
    ("ovr062", 0x0266, "Sort_Unit_Stack"),
]
WIN = 32
failures = 0
for expected_seg, ip_val, expected_func in cases:
    fo = file_off[expected_seg] + ip_val
    win = data[fo:fo+WIN]
    matches = []
    for nm in segs:
        if not nm.startswith('ovr'): continue
        if ip_val + WIN > seg_size[nm]: continue
        sfo = file_off[nm] + ip_val
        if sfo + WIN > len(data): continue
        if data[sfo:sfo+WIN] == win:
            matches.append(nm)
    ok = (len(matches) == 1 and matches[0] == expected_seg)
    print(f"  {'PASS' if ok else 'FAIL'}  seg={expected_seg}  ip={ip_val:#06x}  "
          f"matched={matches}")
    if not ok: failures += 1
sys.exit(1 if failures else 0)
PY
if [[ $? -eq 0 ]]; then
    PASS "Part A: all 4 reference (seg, ip) windows uniquely match"
else
    FAIL "Part A: algorithm test failed (see above)"
fi

echo
# ============================================================
echo "=========================================================="
echo " PART B -- live DOSBox smoke test (timeout=${TIMEOUT}s)"
echo "=========================================================="
echo " DOSBox: $DOSBOX"
echo " conf:   $CONF"
echo " trace:  $LOG"
echo

timeout "${TIMEOUT}s" "$DOSBOX" -conf "$CONF" 2> "$LOG" > /dev/null
rc=$?
case $rc in
    124) INFO "DOSBox: timeout-killed (expected)";;
    0)   INFO "DOSBox: self-exit clean";;
    *)   FAIL "DOSBox: unexpected exit code $rc";;
esac

if [[ -s "$LOG" ]]; then
    PASS "Part B: trace log captured ($(wc -l < "$LOG") lines)"
else
    FAIL "Part B: trace log empty"
fi

dgroup_count=$(grep -c "Borland DGROUP detected" "$LOG" || true)
if [[ $dgroup_count -ge 1 ]]; then
    PASS "Part B: layer A detected ($dgroup_count Borland DGROUP event(s))"
else
    FAIL "Part B: layer A never detected DGROUP"
fi

if grep -q "active EXE = WIZARDS.EXE" "$LOG"; then
    PASS "Part B: layer B identified WIZARDS.EXE"
else
    FAIL "Part B: layer B did not identify WIZARDS.EXE"
fi

not_yet=$(grep -c "not yet identified" "$LOG" || true)
if [[ $not_yet -eq 0 ]]; then
    PASS "Part B: no fires resolved before EXE was identified"
else
    FAIL "Part B: $not_yet fire(s) hit 'not yet identified'"
fi

changed=$(grep -c "active EXE CHANGED" "$LOG" || true)
if [[ $changed -eq 0 ]]; then
    PASS "Part B: no spurious EXE-CHANGED rebinds"
else
    FAIL "Part B: $changed CHANGED event(s) -- detector flapping"
fi

# Informational: how many INT 3F resolutions of each kind?
total_resolves=$(grep -cE "resolved: .*  IDA " "$LOG" || true)
stub_resolves=$(grep -cE "resolved: .*  IDA stub[0-9]+:" "$LOG" || true)
seg_resolves=$(grep -cE "resolved: .*  IDA seg[0-9]+:"  "$LOG" || true)
ovr_resolves=$(grep -cE "resolved: .*  IDA ovr[0-9]+:"  "$LOG" || true)
INFO "Part B: $total_resolves INT 3F resolutions  ($stub_resolves stub, $seg_resolves seg, $ovr_resolves ovr)"

# R6 LIVE assertion: WIZARDS auto-loads SAVE9.GAM during startup, which
# executes Init_Overland in the loaded-game-update init path.  That
# fires a single _unit write at cs=3FD1 ip=010D, captured by
# og_stage2_unit_write_probe and routed through og_describe -- which
# is the function that invokes the R6 byte-pattern resolver.
#
# This is the only Stage 2 _unit-writer reference case that fires
# without user input; the other three (W2/W3/W4, Sort_Unit_Stack and
# Next_Unit_Nearest_Available) require an AI turn.  If THIS one
# regresses, the live R6 path is broken.
unit_test_w1_pass=$(grep -cE "OG_UNIT_TEST.*cs=3FD1 ip=010D.*PASS$" "$LOG" || true)
unit_test_w1_fail=$(grep -cE "OG_UNIT_TEST.*cs=3FD1 ip=010D.*FAIL$" "$LOG" || true)
if [[ $unit_test_w1_pass -ge 1 ]]; then
    PASS "R6 live: W1 reference (Init_Overland) resolved correctly"
    grep -E "OG_UNIT_TEST .*cs=3FD1 ip=010D" "$LOG" | head -1 | sed 's/^/         /'
elif [[ $unit_test_w1_fail -ge 1 ]]; then
    FAIL "R6 live: W1 reference (Init_Overland) FAILED -- byte-pattern resolver broken"
    grep -E "OG_UNIT_TEST .*cs=3FD1 ip=010D" "$LOG" | head -1 | sed 's/^/         /'
else
    FAIL "R6 live: no W1 reference assertion fired -- _unit write never observed"
    INFO "         (expected one [OG_UNIT] writer event during SAVE9.GAM load."
    INFO "          Either the probe is broken, detection lagged, or the"
    INFO "          startup didn't auto-load SAVE9.)"
fi

echo
if [[ $fail -eq 0 ]]; then
    echo "RESULT: ALL PASS"
    exit 0
else
    echo "RESULT: $fail FAILURE(S)"
    exit 1
fi
