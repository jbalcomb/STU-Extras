
# STU-DOSBox

# /home/jbalcomb/STU/devel/STU-Extras/STU-DOSBox/launch-dosbox.sh
# /home/jbalcomb/STU/devel/STU-Extras/STU-DOSBox/stu-dosbox.conf



BASE=/home/jbalcomb/STU/devel/STU-Extras/STU-DOSBox
DOSBOX=/home/jbalcomb/STU/developp/dosbox-0.74-3/src/dosbox
CONF="$BASE/stu-dosbox.conf"
MOM=/home/jbalcomb/STU_DBWD/MoM131
TRACE_LOG=/tmp/og_stage2_trace.log



cd "$BASE"

if [[ -n "$TIMEOUT" ]]; then
    timeout "${TIMEOUT}s" "$DOSBOX" -conf "$CONF" 2> "$TRACE_LOG" || true
else
    "$DOSBOX" -conf "$CONF" 2> "$TRACE_LOG"
fi
