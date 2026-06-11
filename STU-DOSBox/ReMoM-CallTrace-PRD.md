# PRD: ReMoM Function-Call + Program-Path Capture

## Consumer

The comparison is run **per stage** via the orchestrator scripts
[`baselines/compare-stage0.py`](/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/compare-stage0.py),
[`baselines/compare-stage1.sh`](/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/compare-stage1.sh),
[`baselines/compare-stage2.sh`](/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/compare-stage2.sh),
... (one `compare-stage{N}` per matchup stage).  Each orchestrator
runs three analyses in order of decreasing abstraction:

1. [`baselines/compare-anchors.py`](/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/compare-anchors.py)
   -- the function-call / program-path comparator,
2. `baselines/diff-bytes.py` -- raw per-byte save diff,
3. `baselines/diff-structured-stage{N}.py` -- region/field-grouped diff.

`compare-anchors.py` is the **direct consumer** of this PRD's
output.  It pairs OG function calls (from
`/tmp/magic_exe-calls.log`) with ReMoM function entries (from
ReMoM's `STU_LOG` output) at named anchors, and reports where
each side was when reaching each anchor.  The anchor table lives
at [`baselines/anchors.py`](/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/anchors.py).

So in process terms: you run `compare-stage{N}`, it invokes
`compare-anchors.py`, and `compare-anchors.py` reads the
`[FN-ENTER]` / `[FN-EXIT]` stream this PRD specifies.

This PRD covers the ReMoM-side capture only.  The comparators
(the per-stage `compare-stage{N}` orchestrators and the
`compare-anchors.py` they drive) and the OG-side probe are out
of scope.

## Problem

We need to know which ReMoM functions ran, in what order.
Today the information exists but in two awkward forms:

- `[RNG-CALL] at=FILE:LINE` entries (from the `Random_at()`
  wrapper) tell us the call site of each `Random()` call.
  `compare-rng-streams.py` textually parses the `.c` file to
  guess the enclosing function.  Misses every function that
  doesn't call `Random()`; fragile regex; breaks down anywhere
  `Random()` bypasses the wrapper.
- Ad-hoc `LOG_INFO/DEBUG/TRACE` lines already emit `BEGIN:` and
  `END:` markers in many functions, written through `STU_LOG` to
  `assets/remom_log_current.txt`.  These ARE function-entry
  events the comparator can read directly -- no `.c`-file regex
  needed.  But they share categories with everything else
  (`LOG_CAT_GENERAL`, `LOG_CAT_NEWGAME`, ...), formatting is
  inconsistent across functions (`BEGIN: foo()` vs
  `DEBUG: ...: BEGIN: foo()` vs no marker at all), and there is
  no `rng_call` ordinal so a consumer can't correlate them with
  `[RNG-CALL]` events on the same time axis.

We want a single, consistent function-entry/exit stream emitted
by ReMoM under a dedicated category, so the comparator can match
by function name without textual regex over source files.

## Goal

As part of reverse-engineering 'Master of Magic' (1994) as 'ReMoM',
we need to match ReMoM's program-path/code-flow to OG-MoM's.
This sub-project is committed to matching up which functions are
called in what order.



Produce a log of ReMoM's function execution path that:

1. Names each instrumented function explicitly at entry (and exit).
2. Is written to ReMoM's existing `STU_LOG` ring buffer
   (`assets/remom_log_current.txt` / `remom_log_new.txt`), so it
   composes with the logs ReMoM already produces.
3. Is filterable by category (default-off for production, on for
   matchup runs).
4. Is parseable by a downstream comparator (one entry per line,
   stable format).
5. Survives crashes (visible on disk even after SIGSEGV).

## Non-goals

- OG-side capture (handled separately by the DOSBox probe).
- Building the OG↔ReMoM comparator -- the per-stage
  `compare-stage{N}` orchestrators and the `compare-anchors.py`
  they drive already exist; this PRD is their upstream supplier
  of clean ReMoM-side data.
- Instrumenting *every* function in ReMoM.  Initial scope is the
  set of functions named in
  [`baselines/anchors.py`](/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/anchors.py);
  the anchor table is the working list of "what to instrument".
- Replacing or modifying the existing `STU_LOG` infrastructure.
- Reformatting / removing the existing ad-hoc `BEGIN:` / `END:`
  `LOG_INFO` lines.  Those can stay; the comparator can fall back
  to them for any function not yet TRACE_ENTER-instrumented.

## Requirements

### Functional

**F1.** Add a new `LOG_CAT_CALL_TRACE` category in `STU/src/STU_LOG.h`.

**F2.** Add two macros (in a new `STU/src/STU_TRACE.h` or appended to `STU_LOG.h`):

```c
TRACE_ENTER(name_str, fmt, ...)   /* emits "[FN-ENTER] name=NAME ..." */
TRACE_EXIT(name_str, fmt, ...)    /* emits "[FN-EXIT]  name=NAME ..." */
```

Both emit at `LOG_TRACE` severity under `LOG_CAT_CALL_TRACE`.
When the category is disabled (per `ReMoM.ini` `[Logging]`),
 the macros compile to zero overhead (sizeof-noop, same as existing `LOG_*` gates).

**F3.** The log line format MUST be:

```
[FN-ENTER] name=<FUNC>  rng_call=<N>  [arg=<val> ...]
[FN-EXIT]  name=<FUNC>  rng_call=<N>  [retval=<val>]
```

- `rng_call` = current value of the global `Random()` call counter
  (the same counter as `[RNG-CALL] call=N`).  Lets a comparator
  align FN events against RNG calls without timestamp matching.
- `arg=` and `retval=` are optional and printf-formatted; emit only
  what's useful per function (plane index, player index, retort
  count, etc).
- One event per line.  No multi-line entries.  No leading
  timestamps in the event payload (STU_LOG already prefixes each
  line with its own timestamp / severity / category / file:line).

**F4.** The instrumentation worklist is **the anchor table** at
[`baselines/anchors.py`](/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/anchors.py).
Every function listed there as a `remom_fn` should have
`TRACE_ENTER` / `TRACE_EXIT` so the comparator can match it
without falling back to the existing ad-hoc `BEGIN:` lines or
to `[RNG-CALL]` enclosing-function inference.

Starting points (these are already anchors or will be in the
near term, all called from `Init_New_Game` in `MoM/src/MAPGEN.c`):

- `main` (HeMoM.c) / `_main` on OG side
- `Randomize_Book_Heights` (NewGame.c)
- `Init_Computer_Players` and its callees
  (`Init_Computer_Players_Wizard_Profile`,
  `Init_Computer_Players_Spell_Library`, ...)
- `Init_Landmasses`, `Generate_Landmasses`,
  `Translate_Heightmap_To_Base_Terrain_Types`,
  `Generate_Climate_Terrain_Types`
- `Generate_Nodes`, `Make_Aura`, `Set_Node_Type`,
  `Rebalance_Node_Types`
- `Generate_Towers`, `Extend_Islands`, `Generate_Lairs`,
  `Create_Lair`
- `Generate_Home_Cities`, `Generate_Neutral_Cities`,
  `Generate_Terrain_Specials`, `Generate_Roads`,
  `Simex_Autotiling`, `River_Path`
- `Init_New_Game` itself, `Save_SAVE_GAM`,
  `Loaded_Game_Update`, `Init_Overland`, `PreInit_Overland`,
  `Do_City_Calculations` (the post-load city-economy chain
  identified during matchup work)

When new anchors are added to `anchors.py`, instrument
their `remom_fn` here.  When functions are removed from the
anchor table, instrumentation can stay -- extra TRACE_ENTER
events don't hurt.

Helper functions called many times per top-level call (e.g.
`Random()`, `Test_Bit_Field()`) are NOT instrumented -- they
would swamp the log.

**F5.** ReMoM-side change: the existing `Random_at()` wrapper in
`MoX/src/random.c` already increments a global call counter.
Expose that counter (or its accessor) so `TRACE_ENTER` / `TRACE_EXIT`
can read it.

**F6.** Provide an `[Logging]`-section example in
`assets/ReMoM.ini` that enables `CALL_TRACE = true` (default
behavior is "category not listed" → enabled, so this is mainly
documentation that the category exists).

### Non-functional

**NF1.** When `CALL_TRACE` is disabled at runtime, the per-call
overhead is one branch on a category-enable flag (same as every
other `LOG_*` macro).  No allocations, no formatting.

**NF2.** When `CALL_TRACE` is enabled, the log volume is roughly
one ENTER/EXIT pair per instrumented function call.  Order of
magnitude: ~thousands of lines for a representative matchup run.
The STU_LOG ring buffer must be sized to hold it without dropping
events; if it can't, the ring size becomes a follow-up item.

**NF3.** Logs MUST survive SIGSEGV.  This means STU_LOG either
already flushes on crash, OR we add a `signal(SIGSEGV)` handler
that calls `STU_Log_Flush_All()` before re-raising.  Need to
verify which one is currently true.

## Output / contract

After any matchup run with `CALL_TRACE` enabled, the freshest
ReMoM STU_LOG file contains an interleaved stream of `[FN-ENTER]`
/ `[FN-EXIT]` events under category `LOG_CAT_CALL_TRACE` at
severity `LOG_TRACE`, with `rng_call=N` on each event that
matches the `call=N` field on adjacent `[RNG-CALL]` events from
the existing `Random_at()` wrapper.

### STU_LOG file rotation

ReMoM writes STU_LOG output across THREE files in `assets/`:

- `remom_log_new.txt`
- `remom_log_current.txt`
- `remom_log_previous.txt`

The names are misleading.  The file holding the most recent run
is whichever was most recently written (highest mtime), NOT
necessarily `remom_log_current.txt`.  Rotation can leave today's
data in `new.txt` while `current.txt` still holds an older run.

**Consumers MUST pick the active log by mtime, not by name.**
`compare-anchors.py` and `compare-call-paths.py` use a
`pick_freshest_remom_log()` helper that scans all three and
returns the one with the highest mtime.  Hardcoding
`remom_log_current.txt` has produced false negatives ("function
never logged") when the BEGIN was sitting in `remom_log_new.txt`
from a more recent run.

### Consumer contract

`compare-anchors.py` (invoked by the per-stage `compare-stage{N}`
orchestrator) reads these events from the freshest ReMoM STU_LOG
file and treats the `name=` field as authoritative for the
matching `anchor.remom_fn`.  No source parsing.  Anchors that
don't appear as `[FN-ENTER]` events fall back to (a) ad-hoc
`BEGIN:` lines for the same function, or (b) `[RNG-CALL]`
enclosing-function inference -- both noisier, both inferior, but
available where this PRD's instrumentation hasn't landed yet.

### Name normalisation (OG vs ReMoM)

Consumers comparing ReMoM `[FN-ENTER] name=<fn>` events against
OG-side function names from the call log MUST normalise both
sides before matching:

- Strip Borland's leading-underscore extern decoration
  (OG: `_main`  →  ReMoM: `main`).
- Strip Borland's overlay-manager jump-far thunk prefix
  (OG: `j_Init_New_Game`  →  ReMoM: `Init_New_Game`).
  Every cross-overlay CALLF in OG targets a `j_<fn>` thunk in the
  caller's stub segment, not the target function directly.  The
  thunk JMP-FARs to the real function.  For matchup purposes the
  thunk and target are the same function.
- Apply any registered drake178 `__WIP`-suffix renames from the
  OG↔ReMoM name map.  Those are RE-tagging, not real differences.

The `compare-call-paths.py` `norm()` helper does the first two
strips automatically.  Anchor renames are handled by carrying both
`og_fn` and `remom_fn` in `anchors.py`.

## Open questions

- **Args / retvals.**  Per-function: which args/retvals are worth
  logging?  Suggest a per-function decision when instrumenting; a
  minimal default is `()` (no args printed).
- **Recursion / depth.**  Should we track call depth in the event
  payload?  Useful for indented pretty-printing but not required
  for matchup.
- **Macro vs raw `LOG_INFO`.**  The wrapping macros above let us
  later change format without touching every call site.  Worth the
  ~50 lines of header glue.
- **Which file holds the macros.**  Either appended to
  `STU/src/STU_LOG.h` or a new `STU/src/STU_TRACE.h` that includes
  STU_LOG.  No strong preference; pick whichever doesn't churn
  unrelated source.

## Out of scope (explicit)

- The comparator scripts that consume these logs -- the per-stage
  `compare-stage{N}` orchestrators (`compare-stage0.py`,
  `compare-stage1.sh`, `compare-stage2.sh`, ...) and the
  `baselines/compare-anchors.py` they invoke -- already exist.
  `compare-anchors.py` will pick up TRACE_ENTER events when they
  land; until then it falls back to existing `BEGIN:` lines and
  `[RNG-CALL]` enclosing-function inference.
- The anchor table at `baselines/anchors.py` -- it's the
  worklist for F4, owned by the comparator.
- The OG-side probe in DOSBox / STU-DOSBox -- separate work.
- Performance-mode builds
  (`STU_LOG_MIN_SEVERITY > LOG_SEV_TRACE`) -- those already strip
  the macros; no special handling needed.
- The `compare-rng-streams.py` PRNG analysis -- this PRD is for
  the function-call / program-path comparator, not RNG-stream
  alignment.  The two analyses share infrastructure but answer
  different questions.
