# Implementation Plan: Runtime Game-Data Capture

**Companion to:** [CaptureGameData-PRD.md](CaptureGameData-PRD.md)
**Status:** `_players` target **DONE through the comparator** (Phases 0–4):
OG fires `[GD]` at every populating function; ReMoM emits matching `[GD]`
(per-player `STU_Log_Flush_All()` to beat the async-ring drop); and
[compare-gd-stage0.py](../../../STU_DBWD/dosbox-capture/prng-port/baselines/compare-gd-stage0.py)
diffs them field-by-field at seed `0x00173471` — `Wizard_Profile` matches
640/640.
**Pivot (upstream-first):** the comparator showed Stage-0 **world-gen
divergence** — `summon_w{x,y}`/`capital_race` differ because the fortress
cities are placed at **different map locations** (`Generate_Home_Cities`),
itself driven by `Generate_Lairs`. So we paddle upstream: next targets are
**`_LAIRS` → `_CITIES` → back to `_players`** (§1b). Plus a new capability —
**local/stack-value capture** (the `Random()` argument at `Create_Lair`) to
diff the RNG-draw inputs (PRD §5.11; discussion pending).
**Proven target:** `_players[]` (`struct s_WIZARD[NUM_PLAYERS]`)

---

## 0. Capture policy (refines PRD §5.4)

**Capture each target immediately AFTER every function that populates it
("FN-EXIT capture").** One snapshot per populating function means each
function's contribution is visible in isolation, and the *first* OG↔ReMoM
divergence pins to a specific function rather than "somewhere in new-game
setup." A final snapshot at the save landmark gives the complete record.

This is strictly richer than the PRD's original three landmarks and replaces
them.

## 1. v1 capture set for `_players[]`

In new-game execution order. Resolved in **Phase 0** from
[MGC-overlay-table.fwv](MGC-overlay-table.fwv) (entry_off = `func_ea −
seg_ea`). All populating functions turned out to be **near intra-overlay
calls** — return address is `[SS:SP]`, CS unchanged — so every one uses the
near-call FN-EXIT trap; only the save point is a resident landmark.

| # | Capture after (FN-EXIT) | Location (overlay : entry_off) | Mechanism | Populates (`_players`) |
|---|---|---|---|---|
| 1 | `Newgame_Screen_7__WIP` | ovr050 : 0x1DA8 | near FN-EXIT trap | human `_players[0]` (final picks) |
| 2 | `Init_Computer_Players_Wizard_Profile` | ovr056 : 0x0D3E | near FN-EXIT trap | AI spellranks, retorts, banner_id |
| 3 | `Init_Computer_Players_Spell_Library` | ovr056 : 0x1726 | near FN-EXIT trap | AI `spells_list[]` |
| 4 | `Init_Heroes` | ovr056 : 0x246D | near FN-EXIT trap | `Heroes[]` |
| 5 | `Init_Players` | ovr056 : 0x0A07 | near FN-EXIT trap | runtime/per-player fields |
| 6 | `Init_CP_Strategy` (OG ex-`AI_WIZ_StrategyReset__WIP`) | ovr056 : 0x096D | near FN-EXIT trap | strategy fields |
| 7 | `Init_Magic_Personalities_Objectives` | ovr056 : 0x04B2 | near FN-EXIT trap | `Personality`, `Objective` |
| 8 | `Init_Summoning_Circle_And_Spell_Of_Mastery` | ovr056 : 0x03C6 | near FN-EXIT trap | `summon_w{x,y,p}`, `som_research_cost` |
| 9 | `Init_Diplomatic_Relations` (+ `capital_race` loop) | ovr056 : 0x016A | near FN-EXIT trap | `Dipl`, `Hostility[]`, `capital_race` |
| 10 | `Save_SAVE_GAM` | ovr048 : 0x0000 | resident `j_` landmark | complete record (final) |

Notes: `Init_Heroes` *has* a `j_` (cross) stub but ran intra in the Stage-0
trace (never fired as a landmark) → treat as near FN-EXIT. OG names are
maintained in the `.fwv` by the user (e.g. `AI_WIZ_StrategyReset__WIP` →
`Init_CP_Strategy`); when a ReMoM name has no `.fwv` match, ask rather than
guess.

Still open for these (deferred into Phase 2, captured live): each function's
**prologue signature bytes** (entry disambiguation) and a per-function
**fires-once** confirmation.

The set is **data-driven and extensible**: adding a capture point is one
table row, not new code. (`Newgame_Screen_5` — human book sliders /
`_players[0].spellranks` per the ideation — is a candidate addition.)

## 1b. Upstream targets (next): `_LAIRS`, then `_CITIES`

The `_players` divergences root in **world generation**, so we re-point the
(now-proven) framework upstream. Each new target repeats the same recipe —
Phase 0 (resolve capture points from [MGC-overlay-table.fwv](MGC-overlay-table.fwv)
+ struct from `MOM_DAT.h`) → Phase 1 (field-map generator, already generic)
→ Phase 3 (wire OG `[GD]` + ReMoM emit) → comparator — just aimed at a
different struct.

**`_LAIRS`** — do first (most upstream). Capture after the lair generators:
- `Generate_Lairs` (FN-EXIT) — the full lair set.
- `Create_Lair` (FN-EXIT, per call) — each lair as created; **also the site
  for the first local-value capture** (the `Random()` argument, PRD §5.11).
- `s_LAIR` field map from `MOM_DAT.h`. `_LAIRS` is a **far pointer** to a
  block → single-indirection read (§5.6 of PRD); its DGROUP slot comes from
  the wiki / ask the user.

**`_CITIES`** — then. Capture after `Generate_Home_Cities` (and sibling
city-gen functions). `s_CITY` field map (struct size `0x72`); `_CITIES` is a
far pointer at MGC `DS:0x89D2` (per RE notes) → indirection read.

**`_players` recheck** — last. Re-run the comparator once lairs+cities agree;
the `summon`/`capital_race`/personality/diplomacy divergences should largely
resolve once the upstream world-gen matches.

Open per-target items (resolve in each Phase 0, asking the user for any
DGROUP offset not in the wiki): the far-pointer DGROUP slots, struct
strides/counts, and the lair/city generator overlay offsets + signatures.

## 2. FN-EXIT capture mechanism (the new capability)

PRD N4 avoided overlay-buffer code triggering. The capture-after-each-function
policy requires firing after intra-overlay functions (`Wizard_Profile`,
`Spell_Library`, …), so we add a bounded, proven form of it:

1. **Detect entry of function X.**
   - *Resident* X: runtime CS:IP resolves (via `base_offset`) to X's IDA
     entry. Exact.
   - *Overlay* X: the resolver says we're in `ovrNNN` **and** `IP ==
     (X.func_ea − seg_ea)` **and** the first bytes match X's prologue
     signature. This is the NUNA technique (`og_sort_trace.h` already does it
     for `Next_Unit_Nearest_Available` in ovr061). An exact function-entry
     offset + prologue is distinctive, so it avoids the ambiguity that makes
     general overlay-address resolution unreliable.
2. **Snapshot the return address** from the stack at entry: near call →
   `[SS:SP]` (return IP, CS unchanged); far call → `[SS:SP]=IP`,
   `[SS:SP+2]=CS`. Call type is recorded per function in the table.
3. **Trap the return:** when CS:IP next equals the snapshotted return
   address, run the capture for the target(s) bound to that point, then
   disarm.

Notes / edge cases:
- **Re-entrancy:** keep a small return-address stack keyed by entry. The v1
  functions are not recursive; deeper handling is a later concern.
- **Overlay swap mid-function:** if X calls into another overlay, CS changes
  during X but the trap matches the *caller's* CS:IP on return, so it still
  fires correctly.
- This relaxes PRD **N4 for entry detection only**; we still do not resolve
  arbitrary overlay-buffer addresses.

## 3. Phases

**Phase 0 — Resolve the capture set.**
For each function in §1: look up overlay, entry offset (`func_ea − seg_ea`),
and call type (resident `j_` vs intra-overlay `NX_j_`) in the overlay
tables; capture its prologue signature bytes; confirm it executes in a real
Stage-0 trace. *Deliverable:* the completed §1 table + a per-function
`(detect-addr, signature, call-type, ret-trap)` spec.

**Phase 1 — Field-map generator + self-check** (PRD §5.3).
Parse `s_WIZARD` from `MOM_DAT.h` → `(offset, name, kind, count)`, resolve
count constants, recurse nested structs, self-check computed offsets vs the
`/* 0xNN */` comments and total vs `0x4C8`. Emit the hard-coded C table.
*Deliverable:* generated `s_wizard_fields[]` header + a printed table for
review.

**Phase 2 — Generic capture engine + FN-EXIT trap.**
Add the target table (PRD §5.2) and the §2 entry/return-trap machinery to
`og_sort_trace.h`; replace `og_spellranks_probe` with a table-driven
`gd_capture` that decodes/emits per PRD §5.7. Wire capture point #2
(`Wizard_Profile` FN-EXIT) first. *Acceptance:* `_players` spellranks match
the existing `[OG_SPELLRANKS]` values exactly.

**Phase 3 — Wire all capture points.**
Add points #1, #3–#10; tag each record with its function name; verify each
fires once per new game (grep the trace). *Acceptance:* one tagged record
set per point; field values evolve sensibly across the sequence.

**Phase 4 — ReMoM parity emit + comparator.**
ReMoM already logs `FN-EXIT` (`LOG_TRACE`, `LOG_CAT_CALL_TRACE`) for these
functions; extend each to dump the same per-field record. Build the
comparator: per function, per field, side-by-side with `<<` markers, masking
`Unknown_*`/padding. *Acceptance:* full-field OG↔ReMoM match for Stage-0
`_players` at every capture point (expected to match); if not, the first
mismatching `(function, field)` is reported.

**Phase 5 — Externalize tables.**
Move target table, capture-point table, and field maps to files loaded at
probe init (like the `.fwv` tables), so new targets/points need no rebuild
(PRD G3b).

## 4. Validation throughout

- Phase 1: generator self-check passes (offsets == comments, total `0x4C8`).
- Phase 2: spellranks at `Wizard_Profile` FN-EXIT == prior `[OG_SPELLRANKS]`
  and == ReMoM (already verified at seed `0x000AECB0`).
- Phase 3: each point's record cross-checks against OG `SAVE1.GAM` at the
  save point.
- Phase 4: ReMoM and OG agree field-by-field at each point for Stage 0.

## 5. Risks (delta from PRD)

- **Entry-signature collisions** across overlays → use exact func offset +
  prologue; validate each function's signature in Phase 0.
- **Far vs near call** misclassification corrupts the return-address read →
  record call type per function from the disassembly in Phase 0.
- **A populating function that writes `_players` from a non-listed site**
  (e.g. a screen we didn't enumerate) → the save-point record (#10) is the
  backstop; comparator flags any field that only matches at #10.
- Inherits PRD R1 (mask Unknown/pad), R3 (field-map drift), R6 (core_normal
  only).
