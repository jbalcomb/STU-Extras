# MoM `_unit` Divergence Investigation

Task-scoped working doc.  Source of truth for **what we're trying to
prove, what we've observed, and what data we still need**.

This is **not**
[STU-DOSBOX-scratch-notes.md](STU-DOSBOX-scratch-notes.md) -- those
are user notes; do not edit them from here.

---

## 1. Problem statement

The _unit mismatch bug only applies to 'Stage 2 - Next Turn'.

The OG-vs-ReMoM byte-for-byte comparison harness
[`prng-port/baselines/`](../../../../../STU_DBWD/dosbox-capture/prng-port/baselines/README.md)
flagged `_unit` (WIZARDS DGROUP global, 16-bit) as a categorical
mismatch in **Stage 2 (Next Turn)**.  From
[`stage2-next-turn/RECIPE.md` "Expected diff" section](../../../../../STU_DBWD/dosbox-capture/prng-port/baselines/stage2-next-turn/RECIPE.md):

> `_unit` global (reset at AI-turn end in OG -- ReMoM currently doesn't reset)

The observation is: after the AI's end-of-turn pipeline runs in OG,
`_unit` is reset to a specific value (presumed 0 or -1); in ReMoM the
same end-of-turn pipeline leaves `_unit` holding whatever value the
last write set.  This shows up as a byte diff between Stage 2's
canonical output (`canonical-save5.GAM`) and ReMoM's `assets/SAVE1.GAM`.

The Stage 1 (Load + immediate Save) round-trip is **expected to
match** -- and the trace-level smoke runs done so far confirm that
both sides' `_unit` trajectory through load is identical
(0 → 5 via DOS read → 0 via Init_Overland → 0 via Next_Unit_Nearest_Available Closest_Active
→ 5 via Sort_Unit_Stack → 0 via Next_Unit_Nearest_Available Closest_Active → 5 via SUS).
Stage 1 is **not where the bug lives**.

## 2. Bug location, in source terms (working hypothesis)

OG-MoM resets `_unit` at the *end of AI turn processing*.  The reset
site is somewhere along OG's equivalent of
`NEXTTURN.c::GAME_NextAIPlayer` / `Next_Turn_Calc`.  ReMoM has the
function chain but is missing the reset assignment.

This is a working hypothesis -- not yet confirmed by trace data on
either side for Stage 2.  Stage 2 traces will either confirm it or
re-locate the bug.

## 3. Out-of-scope conclusions to avoid

- **Do not chase `_unit` deltas in Stage 0 or Stage 1.**  Both are
  expected to match.  If trace counts differ between OG and ReMoM in
  those stages it is instrumentation-completeness noise (one side has
  more assignment sites instrumented, or sites at IPs the OG site
  table doesn't yet cover), not the divergence under investigation.
  Specifically: the Stage-1 SUS write-count delta observed earlier
  (OG records 4 SUS writes, ReMoM records 8) is noise of this kind,
  **not the bug**.
- **Do not generalize from the Stage-1 load run.**  Matching `_unit`
  trajectories in Stage 1 say nothing about Stage 2.

## 4. Three-stage comparison plan

Mirrors the
[baselines/](../../../../../STU_DBWD/dosbox-capture/prng-port/baselines/README.md)
structure.  Each stage is a separate OG session and a separate
datapoint; only **Stage 2** is the bug investigation -- Stage 0 and
Stage 1 are baseline-match verification.

Stage chaining: each stage's canonical OUTPUT is the next stage's
canonical INPUT.  Stage N owns OG save slots 2N (working input, copy
of prior canonical) and 2N+1 (canonical output).  Slot 9 is also
overwritten with the prior canonical so WIZARDS-direct / Continue
runs land on the same input.

| Stage | OG trigger | ReMoM trigger | Compare artifact | `_unit` expectation |
|------|-----------|---------------|------------------|---------------------|
| 0 (New Game)     | `launch-stage0.sh`: MAGIC New Game per `BaseLine.RECIPE.md` → save slot 1 → quit | `HeMoM --newgame test_rjak.ini --seed <canonical>` | `stage0-newgame/canonical-save1.GAM` byte-cmp | **Match** (verify only) |
| 1 (Load Game)    | `launch-stage1.sh`: load slot 2 (copy of Stage 0 canonical) → save slot 3 → quit | `HeMoM --continue` + `stage1-wizards-load/scenario.hms` | `stage1-wizards-load/canonical-save3.GAM` byte-cmp | **Match** (verify only -- already confirmed at the trace level; in OG load+save is the identity, so slots 1 and 3 are byte-identical) |
| 2 (Next Turn)    | `launch-stage2.sh`: load slot 4 (copy of Stage 1 canonical) → Patrol → N → save slot 5 → quit | `HeMoM --continue` + `stage2-next-turn/scenario.hms` | `stage2-next-turn/canonical-save5.GAM` byte-cmp | **Mismatch expected -- this is the bug** |

## 5. Data we need from each stage

For each stage, on both sides, capture:

- **Every write to `_unit`**, not just changes.  Same-value writes
  (`_unit = 0` when already 0; `_unit = 5` when already 5) matter
  because they tell us which code path executed.  ReMoM does this
  naturally via the `[_unit ASSIGN]` log lines added at every
  assignment site.  OG's `og_stage2_unit_write_probe` only logs
  changes; that gap is what `og_stage2_unit_write_site_probe` (this
  task) closes by hooking on known instruction sites.
- **The value written**, source-attributed
  (segment + function + offset).
- **For Next_Unit_Nearest_Available invocations**: the two locals `Closest_Active_Unit` and
  `Closest_Waiting_Unit` at the branch decision (already done by
  the `[Next_Unit_Nearest_Available] BRANCH` probe in `og_sort_trace.h`).

## 6. Known `_unit` writer sites in OG-MoM WIZARDS.EXE

| Site                                       | CS (dynamic) | IP (static) | Notes |
|--------------------------------------------|-------------|------------|-------|
| `F000:14C1` DOS `INT 21h` `gfread` inside `Load_SAVE_GAM` | F000 (ISR)  | 14C1 | Loads `_unit` from save file |
| `ovr051:Init_Overland+10D`                 | ovr051 buf  | 0x010D | Resets `_unit = 0` at overland init |
| `ovr062:Sort_Unit_Stack+149`               | ovr062 buf  | 0x0266 | Stores selected stack head |
| `ovr061:Next_Unit_Nearest_Available+23D`   | ovr061 buf  | 0x0B1A | `_unit = Closest_Active_Unit` branch |
| `ovr061:Next_Unit_Nearest_Available+???`   | ovr061 buf  | TBD     | `_unit = Closest_Waiting_Unit` branch (not yet observed) |

CS values are runtime-dynamic and session-specific; use only the
segment-name labels above for stable references.

## 7. Plan

1. Add `og_stage2_unit_write_site_probe` to `og_sort_trace.h`.
   Site-table-driven: at every guest instruction, check if CS:IP
   matches a known `_unit`-write site (overlay-buffer match via
   patched-thunks lookup for non-ISR sites), and on the next
   instruction log the post-store `_unit` value.  This logs **every
   write, including same-value writes**, complementing the existing
   change-only probe.
2. Once you refresh the baseline, run all three stages on both sides:
   - Stage 0 and Stage 1: verify byte match and probe behavior.
     Differences in write counts here are instrumentation noise, not
     the bug -- do not chase them.
   - Stage 2: capture the full `_unit` write trajectory on both sides
     and find the first site OG writes but ReMoM doesn't (or vice
     versa).
3. The first divergent step in Stage 2 names the bug.

## 8. Cross-refs

- [BaseLine.RECIPE.md](../../../../../STU_DBWD/dosbox-capture/prng-port/baselines/BaseLine.RECIPE.md)
- [stage1-wizards-load/RECIPE.md](../../../../../STU_DBWD/dosbox-capture/prng-port/baselines/stage1-wizards-load/RECIPE.md)
- [stage2-next-turn/RECIPE.md](../../../../../STU_DBWD/dosbox-capture/prng-port/baselines/stage2-next-turn/RECIPE.md)
- [og_overlay_findings.md](../../../../developp/dosbox-0.74-3/src/misc/og_overlay_findings.md)
