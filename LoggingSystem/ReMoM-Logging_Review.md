# ReMoM Logging — System Review and Handoff

**Audience:** AI agent or engineer taking over diagnostic/observability work on the ReMoM codebase.
**Scope:** Every logging mechanism currently in the tree, what each writes, where it goes, and the known rough edges.
**Generated against:** the state of `main` at the time of this writing — verify with `git log` and `grep` before acting on specifics.

---

## 1. TL;DR

There are **six distinct logging mechanisms** in the tree, written at different layers and for different audiences:

| # | Mechanism | Output | Toggle | Format |
|---|---|---|---|---|
| 1 | `dbg_prn()` / `DLOG()` | `DEBUG.LOG` | `#ifdef STU_DEBUG` (Debug builds) | Free-form printf |
| 2 | `trc_prn()` | `TRACE.LOG` | `#ifdef STU_DEBUG` | Free-form printf |
| 3 | `fprintf(stderr, ...)` | terminal/console | always on | Free-form |
| 4 | `MOUSE_LOG()` | `mouse_debug.log` | `#ifdef MOUSE_DEBUG` | Free-form |
| 5 | `REC_LOG` / `PLAY_LOG` (Replay) | record/replay log file or stderr | runtime, when `--record`/`--replay` is active | Free-form |
| 6 | `AI_Metrics_Emit_*()` | 5 separate `.CSV` files | `#ifdef STU_DEBUG` + `--ai-metrics` CLI | Structured CSV |

The first five all write unstructured human-readable text. The sixth is the only structured/parseable output and was built on top of the existing pattern for automated regression testing.

---

## 2. The Two Big Log Files: `DEBUG.LOG` and `TRACE.LOG`

### 2.1 Where it lives
- Public header: [STU/src/STU_DBG.h](../STU/src/STU_DBG.h)
- Implementation: [STU/src/STU_DBG.c](../STU/src/STU_DBG.c)
- Lifecycle hooks: [src/ReMoM.c:220,400](../src/ReMoM.c) and [src/HeMoM.c:518,811](../src/HeMoM.c)

### 2.2 API
```c
void Debug_Log_Startup(void);   // opens DEBUG.LOG "w" (overwrite)
void Debug_Log_Shutdown(void);  // flushes + closes
void Trace_Log_Startup(void);   // opens TRACE.LOG "w"
void Trace_Log_Shutdown(void);

void dbg_prn(const char * fmt, ...);  // vfprintf to DEBUG.LOG + fflush
void trc_prn(const char * fmt, ...);  // vfprintf to TRACE.LOG + fflush

#define DLOG(_str_)  // fprintf with __FILE__/__LINE__ prefix to DEBUG.LOG
```

### 2.3 Behavior
- **Always overwrites** the file on startup (`"w"` mode, not `"a"`). A second run nukes the first run's log.
- **`fflush()` after every write** for crash safety. This is expensive — see §7.
- Both files have an ISO-8601 timestamp at the BEGIN/END markers only; **individual log lines are not timestamped**. Callers who care about time prepend `Platform_Get_Millies()` themselves (see `[HeMoM Player] t=NNN ms ...` lines).
- In Release builds, `DLOG` macro expands to `((void)sizeof((_str_), 0))` — a no-op that still type-checks the argument. `dbg_prn`/`trc_prn` calls require explicit `#ifdef STU_DEBUG` guards at every call site.

### 2.4 Volume
~52 source files have logging calls. Highest concentrations: [HeMoM.c](../src/HeMoM.c) (102), [Settings.c](../MoM/src/Settings.c) (59), [Artificial_Human_Player.c](../src/Artificial_Human_Player.c) (52), [STU_SND.c](../STU/src/STU_SND.c) (45), [Combat.c](../MoM/src/Combat.c) (36), [AIMOVE.c](../MoM/src/AIMOVE.c) (36).

A 5-turn headless run produces ~1400 lines in DEBUG.LOG and ~1240 in TRACE.LOG.

### 2.5 Conventions (observed, not enforced)
- **Function entry/exit:** `dbg_prn("DEBUG: [%s, %d]: BEGIN: Foo()\n", __FILE__, __LINE__);`
- **Subsystem tag prefixes** are common but inconsistent. Examples: `AI_TURN:`, `AI_NPC:`, `AI_ORDERS:`, `AI_RESEARCH:`, `[HeMoM]`, `[HeMoM Player]`, `[NEXTTURN]`, `[GOLD]`, `[IKI]`, `[headless]`.
- A handful of tags use brackets, a handful use trailing colons, a handful use both. No central registry.

### 2.6 Why DEBUG vs TRACE?
Looking at usage: there's no consistent semantic distinction. Most callers `dbg_prn` and `trc_prn` the same message (search for paired calls in [HeMoM.c](../src/HeMoM.c)). Effectively they are two copies of the same log. Treat this as historical accident, not a useful filter.

---

## 3. `fprintf(stderr, ...)` — the always-on channel

Used for messages that should reach the operator even in Release builds. Common patterns:

- HeMoM CLI parsing echoes every arg to stderr regardless of build type.
- Synthetic player ([HeMoM_Player.c](../src/HeMoM_Player.c)) logs every injected key/click to stderr with millisecond timestamps. This is the easiest log to read when debugging a scenario.
- Many subsystems wrap an `fprintf(stderr, ...)` and an `#ifdef STU_DEBUG dbg_prn`/`trc_prn` triple — see HeMoM.c CLI handling for the template.

**No file. No flushing guarantees beyond stdio's defaults.** When run under CTest, stderr is captured per-test and written to `build/Testing/Temporary/LastTest.log`.

---

## 4. `MOUSE_LOG()` — opt-in input tracing

- Macro: [platform/include/Platform.h:464](../platform/include/Platform.h#L464) — `fprintf(Mouse_Dbg_Log(), __VA_ARGS__)`
- File: `mouse_debug.log` (lazy-opened on first use, unbuffered `setvbuf(_IONBF)`)
- Gated by `#ifdef MOUSE_DEBUG` — **not** by `STU_DEBUG`. Separate define, not on by default.
- Heavily used in [sdl2_PFL.c](../platform/sdl2/sdl2_PFL.c) and [MOM_SCR.c](../MoM/src/MOM_SCR.c) (screen transitions are also logged here, slightly off-label).

Useful when chasing input-routing bugs; otherwise irrelevant noise.

---

## 5. Replay subsystem logs

- File: [platform/replay/Replay.c](../platform/replay/Replay.c)
- Macros: `REC_LOG`, `PLAY_LOG` — wrap stderr or a per-session log file
- Only active when `--record`/`--replay` CLI flags are passed
- Records field-hit events from the synthetic player so deterministic replays can be diffed against recordings

Out of scope for most diagnostic work unless you're debugging the replay framework itself.

---

## 6. `AI_METRICS` — the only structured output

### 6.1 Where it lives
- Header: [STU/src/AI_METRICS.h](../STU/src/AI_METRICS.h)
- Implementation: [STU/src/AI_METRICS.c](../STU/src/AI_METRICS.c)
- Toggle: `#ifdef STU_DEBUG` (compile-time) + `int16_t AI_Metrics_Enabled = 0` (runtime, set by `--ai-metrics` CLI on HeMoM and ReMoM)
- Lifecycle: same `_Startup`/`_Shutdown` pattern as `STU_DBG`, but startup happens **after CLI parsing** so the runtime flag has been set

### 6.2 Files produced
All written to the working directory at startup (`stu_fopen("FILENAME.CSV", "w")`):

| File | Cardinality | Purpose |
|---|---|---|
| `AI_TURN_SUMMARY.CSV` | NUM_PLAYERS rows per turn (typically 6) | Scoreboard: gold, mana, fame, units, cities, accumulators |
| `AI_NPC_EVENTS.CSV` | sparse — one per event | Raider/monster spawns, NPC targeting, garrison culls |
| `AI_BUILD_DETAIL.CSV` | one per city per turn (AI wizards only) | City production decisions with weights + top-3 alternatives |
| `AI_UNIT_OUTCOMES.CSV` | one per non-idle unit per turn | Movement results: src/dst/end positions, move_failed flag |
| `AI_DECISIONS.CSV` | one per decision point | Generic decision log: RESEARCH, HOSTILITY, etc. |

Every row starts with `turn,player_idx,...` (or `turn,...` for non-player-scoped tables), making joins trivial.

### 6.3 API
```c
extern int16_t AI_Metrics_Enabled;

void AI_Metrics_Startup(void);
void AI_Metrics_Shutdown(void);

void AI_Metrics_Emit_Turn_Summary(int16_t turn, int16_t difficulty, int16_t num_players);
void AI_Metrics_Emit_NPC_Event(int16_t turn, const char *event_type, int16_t source_idx,
    int16_t source_wx, int16_t source_wy, int16_t source_wp,
    int16_t count, int16_t budget,
    int16_t target_wx, int16_t target_wy,
    int16_t accumulator, int16_t threshold);
void AI_Metrics_Emit_Build(int16_t turn, int16_t player_idx, int16_t city_idx,
    int16_t city_pop, int16_t city_units,
    int16_t product_count, int16_t chosen_weight, int16_t chosen_product,
    int16_t * product_array, int16_t * weights);
void AI_Metrics_Emit_Unit_Outcome(int16_t turn, int16_t player_idx, int16_t unit_idx,
    int16_t unit_type, int16_t status,
    int16_t src_wx, int16_t src_wy, int16_t dst_wx, int16_t dst_wy,
    int16_t end_wx, int16_t end_wy, int16_t move_failed);
void AI_Metrics_Emit_Decision(int16_t turn, int16_t player_idx,
    const char *category, int16_t subject_idx, int16_t decision,
    const char *decision_name, int16_t score, int16_t alt_count);
```

Each emit function checks `AI_Metrics_Enabled` at the top and early-returns. Zero overhead when disabled.

### 6.4 Call sites
- Turn summary: [AIDUDES.c](../MoM/src/AIDUDES.c) end of `AI_Next_Turn()`
- NPC events: 8 sites in [AIDATA.c](../MoM/src/AIDATA.c) — `NPC_Excess_Garrison`, `Make_Monsters`, `Make_Raiders`, `NPC_Destinations`
- Build detail: [AIBUILD.c](../MoM/src/AIBUILD.c) inside `Player_Colony_Autobuild_CP` after `Get_Weighted_Choice`
- Unit outcomes: [SETTLE.c](../MoM/src/SETTLE.c) inside `AI_MoveUnits` with pre/post position snapshots
- Decisions: [AISPELL.c](../MoM/src/AISPELL.c) (RESEARCH) and [AIDATA.c](../MoM/src/AIDATA.c) (HOSTILITY)

### 6.5 Evaluation tool
[tests/check_ai_metrics.c](../tests/check_ai_metrics.c) is a standalone C program that reads the CSVs and evaluates assertions from a `.txt` rules file. Five assertion types:

- `LIVENESS  <csv> <filter> <column> MIN|MAX <value>`
- `BOUNDS    <csv> <filter> <column> RANGE <lo> <hi>`
- `TREND     <csv> <filter> <column> INCREASES|DECREASES|NONZERO`
- `RATE      <csv> <filter> PER <turns> MIN|MAX <count>`
- `BASELINE  <csv> <filter> <column> <baseline_path> TOLERANCE <pct>`

Used by the CTest fixture suite at [tests/CMakeLists.txt](../tests/CMakeLists.txt) (`HeMoM_AIBehavior_*`). The current rules file is [tests/assert_ai_5turns.txt](../tests/assert_ai_5turns.txt) with 25 assertions. Baseline CSV is committed at `tests/baseline_ai_5turns_TURN_SUMMARY.csv`.

Run with `ctest -L ai` from the build directory (Debug builds only — see §7.2).

---

## 7. Known Pain Points

### 7.1 `fflush()` after every write
Every `dbg_prn`/`trc_prn`/`AI_Metrics_Emit_*` call flushes to disk. For crash safety this is correct, but it dominates AI_Next_Turn's wall time in instrumented builds — observed at ~11.5 seconds per turn with metrics enabled vs sub-second without. Long scenarios become CTest timeout risks.

**Mitigation candidates:** batch flush (per turn, not per emit), or `setvbuf(_IOFBF)` with explicit flushes at turn boundaries only.

### 7.2 `--ai-metrics` is Debug-only
[HeMoM.c](../src/HeMoM.c) prints `--ai-metrics ignored (not a debug build)` under Release, because everything `AI_METRICS` is under `#ifdef STU_DEBUG`. The CTest fixtures are accordingly gated on `if(CMAKE_BUILD_TYPE STREQUAL "Debug")` in [tests/CMakeLists.txt](../tests/CMakeLists.txt). To enable AI metrics in Release: either move the module out of the `STU_DEBUG` guard (preferred) or define `STU_DEBUG` in Release too (heavy-handed).

### 7.3 The synthetic-player save-and-quit tail is flaky under Release
[assets/scenario_tail_save_and_quit.hms](../assets/scenario_tail_save_and_quit.hms) clicks (62,177) on the Save/Load Quit button to navigate back to Main Menu before pressing Q to quit-to-DOS. Under MSVC-release headless, this click misses ~1-of-3 runs and the game never exits, hitting CTest's timeout. Root cause is in headless input dispatch / field-press handling, not the metrics system. Comment in [tests/CMakeLists.txt](../tests/CMakeLists.txt) describes this.

### 7.4 DEBUG.LOG and TRACE.LOG are mostly duplicates
Many call sites have a `#ifdef STU_DEBUG dbg_prn(...); trc_prn(...);` block. Pick one or unify. (See §2.6.)

### 7.5 Free-form text is hard to filter
Subsystem tags (`AI_TURN:`, `[HeMoM]`, `[GOLD]`) are useful for `grep` but undocumented and inconsistent. There is no log-level concept (INFO/WARN/ERROR). Errors and informational messages all use the same `dbg_prn`.

### 7.6 No log rotation / size cap
DEBUG.LOG gets overwritten on every run but is unbounded within a run. A 1000-turn scenario would produce a multi-GB log.

### 7.7 Inconsistent timestamps
Only BEGIN/END markers have ISO-8601 timestamps. Real-time correlation between logs requires manual `Platform_Get_Millies()` prefixes per call site.

### 7.8 `MOUSE_LOG` opens `mouse_debug.log` lazily and never closes it
[platform/include/Platform.h:454](../platform/include/Platform.h#L454) — the file handle leaks at process exit. Probably fine in practice but worth knowing.

---

## 8. Practical Cheatsheet

### Running with metrics
```bash
cd build/bin/Debug
./HeMoM --continue --scenario ../../../assets/test_ai_5turns.hms --ai-metrics --dump-save SAVE1.GAM
```
Or via the test suite (which manages cwd and paths):
```bash
ctest --test-dir build -L ai -V
```

### Inspecting outputs after a run
```bash
# All artifacts land in the binary's working directory:
ls build/bin/Debug/AI_*.CSV build/bin/Debug/*.LOG

# Quick checks:
head -5 build/bin/Debug/AI_TURN_SUMMARY.CSV
grep RAIDER build/bin/Debug/AI_NPC_EVENTS.CSV
grep WARNING build/bin/Debug/DEBUG.LOG | head
awk -F, 'NR>1 && $2==5 {print $1, $11, $12}' build/bin/Debug/AI_TURN_SUMMARY.CSV
```

### Regenerating the baseline
```bash
ctest --test-dir build -L ai
cp build/bin/Debug/AI_TURN_SUMMARY.CSV tests/baseline_ai_5turns_TURN_SUMMARY.csv
# verify tests still pass against the new baseline:
ctest --test-dir build -L ai
```

### Adding a new structured metric
1. Add prototype + macro stub to [STU/src/AI_METRICS.h](../STU/src/AI_METRICS.h).
2. Add file handle + open in `AI_Metrics_Startup` + close in `AI_Metrics_Shutdown` + emit function in [STU/src/AI_METRICS.c](../STU/src/AI_METRICS.c).
3. Add `#include "../../STU/src/AI_METRICS.h"` to the file you're instrumenting (it isn't pulled in transitively).
4. Add the `AI_Metrics_Emit_*()` call at the decision site.
5. Add rules to [tests/assert_ai_5turns.txt](../tests/assert_ai_5turns.txt) and rebuild.

### Adding a new assertion type to check_ai_metrics
1. Add a `Eval_<Name>` function in [tests/check_ai_metrics.c](../tests/check_ai_metrics.c) following the existing pattern.
2. Add a parser branch in `main()` after the existing `BASELINE` branch.
3. Document the syntax in the header comment.

---

## 9. Real Findings the System Has Surfaced

These are visible right now in CSVs from any recent test run — useful both as a sanity check that the system works and as an actual TODO list:

1. **All 4 AI wizards' starting units are stuck.** `AI_UNIT_OUTCOMES.CSV` shows `dst_wx=0, dst_wy=0, move_failed=1` for every wizard unit. `DEBUG.LOG` grep for `AI_ORDERS: WARNING` shows `caller=1` in every case — the garrison pushout path at [AIMOVE.c:515](../MoM/src/AIMOVE.c) is reading `AI_Continent_X_Ptr[landmass_idx]` which is uninitialized.

2. **`raider_acc` never increments.** `AI_TURN_SUMMARY.CSV` shows player 5's `raider_acc=0` for every turn. Trace into `Make_Raiders` ([AIDATA.c:352](../MoM/src/AIDATA.c)) — the `have_neutral_city == ST_FALSE` early-return fires because the test worldgen has no neutral cities. Either fix the worldgen or fix the early-return.

3. **`monster_acc` does progress** (3→4→5→8→9→12 over 5 turns) but never reaches threshold (50-difficulty*5=40) before the scenario ends.

---

## 10. File Index

| Path | Purpose |
|---|---|
| [STU/src/STU_DBG.h](../STU/src/STU_DBG.h) / [.c](../STU/src/STU_DBG.c) | `dbg_prn`/`trc_prn`/`DLOG` and `DEBUG.LOG`/`TRACE.LOG` lifecycle |
| [STU/src/AI_METRICS.h](../STU/src/AI_METRICS.h) / [.c](../STU/src/AI_METRICS.c) | CSV-based AI metrics |
| [platform/include/Platform.h](../platform/include/Platform.h) | `MOUSE_LOG` macro and lazy file open |
| [platform/replay/Replay.c](../platform/replay/Replay.c) | `REC_LOG`/`PLAY_LOG` for record/replay |
| [tests/check_ai_metrics.c](../tests/check_ai_metrics.c) | CSV assertion evaluator |
| [tests/CMakeLists.txt](../tests/CMakeLists.txt) | CTest fixture wiring for `HeMoM_AIBehavior_*` |
| [tests/assert_ai_5turns.txt](../tests/assert_ai_5turns.txt) | 25 behavioral assertions |
| [tests/baseline_ai_5turns_TURN_SUMMARY.csv](../tests/baseline_ai_5turns_TURN_SUMMARY.csv) | Captured baseline for regression checks |
| [src/ReMoM.c](../src/ReMoM.c) / [src/HeMoM.c](../src/HeMoM.c) | Lifecycle hooks for all logging subsystems |
