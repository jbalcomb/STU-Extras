#!/usr/bin/env bash
#
# og-overlay-trace.sh -- non-interactive Borland overlay trace.
#
# What the user has been doing by hand:
#   timeout 10 dosbox -conf stu-dosbox.conf 2> /tmp/og_overlay_trace.log
#   cat /tmp/og_overlay_trace.log
#
# Usage:  ./og-overlay-trace.sh [timeout-seconds]   (default 10)

set -uo pipefail
TIMEOUT=${1:-10}
# Heavy-debug DOSBox builds initialize curses for the debugger TUI;
# fallback if invoked from an environment where TERM is missing/dumb.
export TERM="${TERM:-xterm}"
[[ "$TERM" == "dumb" ]] && export TERM=xterm
DOSBOX=/home/jbalcomb/STU/developp/dosbox-0.74-3/src/dosbox
CONF=/home/jbalcomb/STU/devel/STU-Extras/STU-DOSBox/stu-dosbox.conf
LOG=/tmp/og_overlay_trace.log

timeout "${TIMEOUT}s" "$DOSBOX" -conf "$CONF" 2> "$LOG" || true
cat "$LOG"
