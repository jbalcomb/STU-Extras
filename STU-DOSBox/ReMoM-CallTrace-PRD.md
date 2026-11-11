# PRD: ReMoM Function-Call + Program-Path Capture

## Consumer

This PRD's output -- a clean `[FN-ENTER]` / `[FN-EXIT]` stream --
feeds the OG-vs-ReMoM call-path comparison.  Two scripts consume
it, at two granularities.

**1. The direct consumer:**
[`baselines/compare-call-paths.py`](/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/compare-call-paths.py)
-- the per-event function-call / program-path comparator.  It
reads ReMoM's `[FN-ENTER]` events (with the legacy `BEGIN:` lines
as fallback) from the freshest rotating STU_LOG, reads the OG call
log, walks both streams, and prints a side-by-side path table
tagging each row `match` / `OG-only` / `ReMoM-only`, plus the
earliest mismatch.  This is the script the `name=` field exists
for.  It is run standalone (not part of any orchestrator).

**2. The bridge-point reporter chain:**
[`baselines/merge-remom-trace.py`](/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/merge-remom-trace.py)
->
[`baselines/compare-anchors.py`](/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/compare-anchors.py).
`merge-remom-trace.py` interleaves the STU_LOG
`[FN-ENTER]`/`[FN-EXIT]`/`BEGIN` events with the stderr
`[RNG-CALL]` stream (keyed on the shared `g_random_call_count`)
into one chronological table at `/tmp/remom-trace.fwv`.
`compare-anchors.py` then reads that merged trace and reports the
rng-counter delta at each anchor; a non-zero delta means an
assumed re-sync point did not actually line up.

The anchor table
[`baselines/anchors.py`](/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/anchors.py)
is no longer "the checkpoints both sides cross."  Anchors are now
**re-sync barriers**: verified points where, after a divergence
the per-event walk in `compare-call-paths.py` cannot resolve, both
sides are known to be in the same state, so the walk can reset and
resume.  `compare-anchors.py` is the reporter that confirms each
barrier still holds.

These consumers are driven two ways:

- The per-stage orchestrators
  [`compare-stage0.py`](/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/compare-stage0.py),
  `compare-stage1.sh`, `compare-stage2.sh`, ... run, in order:
  (a) `merge-remom-trace.py` then `compare-anchors.py`
  (-> `anchor-comparison.txt`), (b) `diff-bytes.py` (raw per-byte
  save diff), (c) `diff-structured-stage{N}.py` (region/field
  diff).
- `compare-call-paths.py` is run standalone for the full per-event
  path walk; it is NOT currently part of the orchestrator.

This PRD covers the ReMoM-side capture only.  All of the consumers
above, and the OG-side probe, are out of scope.

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
- Building the OG↔ReMoM comparator -- the per-event
  `compare-call-paths.py`, the `merge-remom-trace.py` ->
  `compare-anchors.py` bridge-point reporter, and the per-stage
  `compare-stage{N}` orchestrators that drive the reporter chain
  already exist; this PRD is their upstream supplier of clean
  ReMoM-side data.
- Instrumenting *every* function in ReMoM.  Initial scope is the
  worklist in F4 (the `Init_New_Game` call chain).  Note that the
  anchor table
  [`baselines/anchors.py`](/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/anchors.py)
  is now a small set of *re-sync barriers*, NOT the full
  instrumentation list -- instrument the F4 functions whether or
  not they currently appear as anchors.
- Replacing or modifying the existing `STU_LOG` infrastructure.
- Reformatting / removing the existing ad-hoc `BEGIN:` / `END:`
  `LOG_INFO` lines.  Those can stay; the comparator can fall back
  to them for any function not yet TRACE_ENTER-instrumented.

## Requirements

### Functional

**F1.** Add a new `LOG_CAT_CALL_TRACE` category in `STU/src/STU_LOG.h`.

> **Status (verified 2026-06-13): ✅ DONE.**  `LOG_CAT_CALL_TRACE = 46`
> is defined in `STU/src/STU_LOG.h:79`.

**F2.** Add two macros (in a new `STU/src/STU_TRACE.h` or appended to `STU_LOG.h`):

```c
TRACE_ENTER(name_str, fmt, ...)   /* emits "[FN-ENTER] name=NAME ..." */
TRACE_EXIT(name_str, fmt, ...)    /* emits "[FN-EXIT]  name=NAME ..." */
```

Both emit at `LOG_TRACE` severity under `LOG_CAT_CALL_TRACE`.
When the category is disabled (per `ReMoM.ini` `[Logging]`),
 the macros compile to zero overhead (sizeof-noop, same as existing `LOG_*` gates).

> **Status (verified 2026-06-13): ❌ NOT DONE (deviation).**  No
> `STU/src/STU_TRACE.h` exists and no `TRACE_ENTER` / `TRACE_EXIT`
> macros are defined anywhere.  Instrumentation instead uses raw
> inline calls at 93 sites (across `MoM/src`, `MoX/src`, and `src/`), e.g.
> `LOG_TRACE(LOG_CAT_CALL_TRACE, "[FN-ENTER] name=%s rng_call=%llu", __func__, (unsigned long long)g_random_call_count)`.
> The F3 format and the gating / zero-overhead behaviour ARE met
> (they ride the existing `LOG_TRACE` category gate), but the macro
> indirection is not: the format string and the `g_random_call_count`
> read are duplicated at every site, so a format change would touch
> all 93.  **Decision needed:** introduce the macros and migrate the
> call sites, or close F2 as "won't do -- raw `LOG_TRACE` is the
> accepted convention" (in which case F2 should be rewritten to spec
> the raw-call pattern instead of macros).

**F3.** The log line format MUST be:

```
[FN-ENTER] name=<FUNC>  rng_call=<N>  [arg=<val> ...]
[FN-EXIT]  name=<FUNC>  rng_call=<N>  [retval=<val>]
```

- `rng_call` = the global counter `g_random_call_count` -- the count
  of `Random()` calls **completed** at the moment the event is
  logged.  This is NOT the same number as the adjacent
  `[RNG-CALL] call=N` field: `[RNG-CALL] call=N` is the ordinal of
  the call *about to fire*, so the number of calls completed strictly
  before it is `N - 1`.  A consumer aligning the two streams MUST
  account for this off-by-one (calls completed before an event =
  `call - 1`); `merge-remom-trace.py` already does, using
  `g_random_call_count` as the shared merge key.  Aligning FN events
  against RNG calls this way avoids timestamp matching.
- `arg=` and `retval=` are optional and printf-formatted; emit only
  what's useful per function (plane index, player index, retort
  count, etc).
- One event per line.  No multi-line entries.  No leading
  timestamps in the event payload (STU_LOG already prefixes each
  line with its own timestamp / severity / category / file:line).

> **Status (verified 2026-06-13): ✅ DONE (format).**  Every
> instrumented site emits exactly
> `[FN-ENTER] name=<fn> rng_call=<N>` /
> `[FN-EXIT]  name=<fn> rng_call=<N>`, one line each.  `name` is
> `__func__` at most sites; a few hand-written ones hardcode `name=`
> or add a payload note (e.g. `Fonts.c` `name=Load_Palette`,
> `EMS.c` `(NULL early)`) -- within F3's optional-payload allowance.
> No `arg=` / `retval=` payloads are emitted yet.

**F4.** The instrumentation worklist is the `Init_New_Game` call
chain enumerated under "Starting points" below.  Every function on
it should have `TRACE_ENTER` / `TRACE_EXIT` so
`compare-call-paths.py` can match it by name without falling back
to the existing ad-hoc `BEGIN:` lines.

The anchor table
[`baselines/anchors.py`](/home/jbalcomb/STU_DBWD/dosbox-capture/prng-port/baselines/anchors.py)
is a *subset* of this worklist -- the functions promoted to
re-sync barriers because a divergence was resolved at them.  Every
`remom_fn` in `anchors.py` MUST be instrumented (the bridge
reporter depends on its `[FN-ENTER]` row), but the worklist is
broader than the anchor table and grows as `compare-call-paths.py`
flags OG-only functions that ReMoM actually runs.

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

> **Status (verified 2026-06-13): ⚠️ PARTIAL.**
>
> Instrumented with `[FN-ENTER]` / `[FN-EXIT]`:
> - `MoM/src/MAPGEN.c` -- the full world-gen chain (36 functions:
>   `Init_New_Game`, `Init_Landmasses`, `Generate_Landmasses`,
>   `Translate_Heightmap_To_Base_Terrain_Types`,
>   `Generate_Climate_Terrain_Types`, `Generate_Nodes`, `Make_Aura`,
>   `Set_Node_Type`, `Rebalance_Node_Types`, `Generate_Towers`,
>   `Extend_Islands`, `Generate_Lairs`, `Create_Lair`,
>   `Generate_Home_Cities`, `Generate_Neutral_Cities`,
>   `Generate_Terrain_Specials`, `Generate_Roads`, `Simex_Autotiling`,
>   `River_Path`, and more).
> - `MoM/src/INITGAME.c` -- computer-player setup (13 sites incl.
>   `Init_Computer_Players`, `Init_Computer_Players_Wizard_Profile`,
>   `Init_Computer_Players_Spell_Library`, `Init_Players`,
>   `Init_Heroes`, `Init_Diplomatic_Relations`).
> - `MoM/src/CITYCALC.c` -- `Do_City_Calculations` + 9 city-economy
>   callees.
> - `MoM/src/LOADER.c` -- `Load_TERRSTAT`, `Load_SPELLDAT`.
> - `MoM/src/LoadScr.c` -- `PreInit_Overland`, `Init_Overland`,
>   `Loaded_Game_Update`.
> - `MoX/src/LOADSAVE.c` -- `Save_SAVE_GAM` (writer),
>   `Load_SAVE_GAM` (reader).
> - `MoM/src/NewGame.c` -- `Randomize_Book_Heights`, `Newgame_Screen_0`.
> - `src/ReMoM.c` -- `MOM_main` (the ported MoM entry point).
>
> NOT yet on `[FN-ENTER]` (legacy `BEGIN:` only -- so they appear in
> the merged trace with an unknown rng_call):
> - `main` (`src/HeMoM.c:835`) -- the HeMoM **host harness** entry,
>   `BEGIN:`-only by design (the Linux/SDL wrapper, outside the MoM
>   call graph).  The MoM entry `MOM_main` -- the actual anchor
>   target -- IS instrumented (see the main/MOM_main note below).
>
> Anchor impact: all 5 anchors in `anchors.py` now resolve on the
> ReMoM side via `[FN-ENTER]` -- `Load_TERRSTAT`,
> `Randomize_Book_Heights`, `Newgame_Screen_0`, `Init_New_Game`, and
> (after the retarget below) the `main` anchor via `MOM_main`.
>
> **main vs MOM_main (resolved 2026-06-13).**  OG's `_main` is
> MAGIC.EXE's entry, which corresponds to ReMoM's **`MOM_main`**
> (`src/ReMoM.c:445`, `[FN-ENTER]`-instrumented), NOT the HeMoM
> host-harness `main` (`src/HeMoM.c:835`, the Linux/SDL wrapper that
> runs before any MoM code).  Two fixes landed:
> 1. The `main` anchor in `anchors.py` sets `remom_fn = "MOM_main"`
>    (file `ReMoM.c`, line 445), so `compare-anchors.py` resolves the
>    anchor and its rng delta.
> 2. `anchors.py` gained a `RENAMES` map (`{"main": "MOM_main"}`) that
>    `compare-call-paths.py`'s OG-side `canon_og()` applies, so the
>    per-event side-by-side row pairs as `match` (`_main` @rng0 <->
>    `MOM_main` @rng0, Δ+0).  Renames apply to the OG side ONLY, so
>    ReMoM's HeMoM host `main` stays distinct from `MOM_main`.
>
> drake178 `__WIP` city renames need NO `RENAMES` entry: the OG call
> log already emits the resolved names (`Generate_Home_Cities`,
> `Generate_Neutral_Cities`, `Generate_Towers`); the `..._WIP` symbols
> in `MGC-overlay-table.fwv` are stale relative to the call log.

**F5.** ReMoM-side change: the existing `Random_at()` wrapper in
`MoX/src/random.c` already increments a global call counter.
Expose that counter (or its accessor) so `TRACE_ENTER` / `TRACE_EXIT`
can read it.

> **Status (verified 2026-06-13): ✅ DONE.**  The counter is
> `g_random_call_count`, declared `extern uint64_t g_random_call_count;`
> in `MoX/src/random.h:74`, incremented inside `Random()`
> (`random.c:252`, `:341`), and read directly by every
> `[FN-ENTER]` / `[FN-EXIT]` site.  Exposed as a plain extern global
> (no accessor function), which satisfies "expose that counter."

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
`compare-call-paths.py` and `merge-remom-trace.py` use a
`pick_freshest_remom_log()` helper that scans all three and
returns the one with the highest mtime.  (`compare-anchors.py`
gets this indirectly -- it reads the `/tmp/remom-trace.fwv` that
`merge-remom-trace.py` produces.)  Hardcoding `remom_log_current.txt`
has produced false negatives ("function never logged") when the
`[FN-ENTER]` / `BEGIN` was sitting in `remom_log_new.txt` from a
more recent run.

### Consumer contract

`compare-call-paths.py` reads these events from the freshest
rotating STU_LOG and treats the `name=` field as authoritative for
function identity -- no source parsing.  Functions that don't
appear as `[FN-ENTER]` events fall back to (a) ad-hoc `BEGIN:`
lines for the same function, or (b) `[RNG-CALL]` enclosing-function
inference (the latter only in `compare-rng-streams.py`) -- both
noisier, both inferior, but available where this PRD's
instrumentation hasn't landed yet.

`compare-anchors.py` does not read the raw STU_LOG directly.  It
reads the merged `/tmp/remom-trace.fwv` that `merge-remom-trace.py`
builds (STU_LOG `[FN-ENTER]`/`[FN-EXIT]`/`BEGIN` plus stderr
`[RNG-CALL]`), and uses the first `[FN-ENTER]` (or `BEGIN`) row
matching each `anchor.remom_fn` to read that anchor's rng counter.

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

- The comparator scripts that consume these logs -- the per-event
  `compare-call-paths.py`, the `merge-remom-trace.py` ->
  `compare-anchors.py` bridge-point reporter, and the per-stage
  `compare-stage{N}` orchestrators (`compare-stage0.py`,
  `compare-stage1.sh`, `compare-stage2.sh`, ...) that drive the
  reporter chain -- already exist.  They pick up `[FN-ENTER]`
  events as functions are instrumented; until then they fall back
  to existing `BEGIN:` lines.
- The anchor table at `baselines/anchors.py` -- the re-sync
  barrier set, owned by the comparator.  It is a subset of, not
  the same as, the F4 instrumentation worklist.
- The OG-side probe in DOSBox / STU-DOSBox -- separate work.
- Performance-mode builds
  (`STU_LOG_MIN_SEVERITY > LOG_SEV_TRACE`) -- those already strip
  the macros; no special handling needed.
- The `compare-rng-streams.py` PRNG analysis -- this PRD is for
  the function-call / program-path comparator (`compare-call-paths.py`),
  not RNG-stream alignment.  The two analyses share infrastructure
  but answer different questions.
