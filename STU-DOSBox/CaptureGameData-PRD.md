# PRD: Runtime Game-Data Capture (STU-DOSBox)

**Status:** Draft (r1)
**Owner:** prng-port byte-match audit / mom-matchup
**Scope:** A generic, table-driven mechanism in STU-DOSBox that captures
arbitrary OG game-data (global structs, arrays, and scalars) out of guest
memory at chosen execution points, decodes it to human-readable values via
field maps generated from the ReMoM C definitions, emits a matching record
on the ReMoM side, and diffs the two field-by-field.
**First target (v1):** `_players[]` (`struct s_WIZARD _players[NUM_PLAYERS]`).
**Predecessor:** [Ideation-CaptureGameData.md](Ideation-CaptureGameData.md).

---

## 1. Context

The matchup compares OG MoM (running in STU-DOSBox) against ReMoM
byte-for-byte. We already capture point values out of OG memory: the
`[OG_SEED]`/`[WATCH]` seed probes, and now `[OG_SPELLRANKS]`, which dumps
`_players[].spellranks[]` at a code landmark and was cross-validated against
ReMoM's own log (identical for all 5 players at seed `0x000AECB0`).

That probe is hard-wired to one field (10 bytes at `+0x5A`). We want to
capture **all** of `_players[]`, and — more generally — **any** runtime
global, at **any** execution point, in a form a human can read and a tool
can diff. `_players[]` is simply the first instance of a capability that
should apply to `_CITIES[]`, `_units[]`, world-gen tables, RNG state, etc.

The supporting infrastructure already exists and is reused, not rebuilt:

- Per-instruction hook `og_sort_trace_check()` from
  `src/cpu/core_normal.cpp:143`.
- Guest reads via `real_readb/w/d(seg, off)` (`include/mem.h`).
- Live-EXE detection + runtime→IDA resolver
  (`og_detect_active_exe`, `og_runtime_to_name`, per-EXE `base_offset`)
  documented in [og_overlay_probe.PRD.md](../../developp/dosbox-0.74-3/src/misc/og_overlay_probe.PRD.md).
- A periodic, config-driven dumper (`src/misc/capture_state.cpp`, the
  `[capture]` section) for time/frame-sampled scalars.

## 2. Problem

Capturing each new field or struct today means hand-writing C: compute the
offset, pick the read width, format the value, pick a landmark. That
re-encodes struct knowledge that already lives in the ReMoM headers, rots
when those headers change, and does not scale past a handful of fields.
`s_WIZARD` alone is ~50 scalars plus large arrays
(`spells_list[NUM_SPELLS]`, `Historian[288]`, …) plus three nested structs
(`s_OWNED_HERO Heroes[]`, `s_WIZ_DIPL Dipl`, `s_Astr_Data Astr`),
`0x4C8` (1224) bytes per record × 6 records.

We need capture to be **declarative**: name a target and a capture point;
the layout comes from a generated field map; the engine does the rest.

## 3. Goals

- **G1 — Generic targets.** Capture any named guest global (struct, array,
  or scalar) described by a target table entry, not just `_players`.
- **G2 — Human-readable decode.** Emit decoded values: `char[]` as strings,
  integers as decimal (sign/width aware), arrays as element lists. Not raw
  hex.
- **G3 — Generated field maps (single source of truth).** Field layout is
  produced by preprocessing the ReMoM C struct definitions
  (`MOX/src/MOM_DAT.h`), not hand-authored. Hard-coded into the build to
  start; loadable from a user-supplied file later (G3b).
- **G4 — Event-driven capture at chosen landmarks.** Fire at one or more
  code points, supporting **multiple capture points per run**, each tagged
  so a value can be tracked across the run.
- **G5 — Correct OG runtime handling.** Resolve per-EXE DGROUP, live-EXE
  identity, and overlay vs resident code by reusing the existing resolver.
- **G6 — ReMoM parity emit.** ReMoM emits a matching per-field record at the
  same logical points, enabling a memory-vs-memory diff.
- **G7 — Field-level comparator.** A tool diffs the two sides per field,
  per capture point, marks mismatches, and masks known-uninitialized fields.
- **G8 — Minimally intrusive.** Cheap per-instruction reject; zero change to
  guest behavior; capture is read-only.
- **G9 — v1 milestone (DONE).** `_players[]` fully captured at every
  populating function, ReMoM emits matching `[GD]`, and the comparator
  ([compare-gd-stage0.py](../../../STU_DBWD/dosbox-capture/prng-port/baselines/compare-gd-stage0.py))
  diffs them field-by-field. `Wizard_Profile` matches 640/640.
- **G10 — Multi-target, upstream-first.** The framework captures any global;
  target *priority* follows the divergence chain. The `_players` comparator
  showed Stage-0 world-gen divergence (`summon_w{x,y}`, `capital_race`)
  rooting in `Generate_Home_Cities` ← `Generate_Lairs`. So we paddle
  upstream: capture order becomes **`_LAIRS` → `_CITIES` → back to
  `_players`**, fixing the root cause first.
- **G11 — Local/stack-value capture (extension, design TBD).** Beyond
  globals, capture a function's *arguments/locals* at a chosen call site —
  e.g. the value passed to `Random()` at `Create_Lair` (MAPGEN.c:2888) — by
  reading the stack (`[SS:SP±n]` / `[SS:BP±n]`) at that code point. This lets
  us compare the RNG-draw **inputs** OG↔ReMoM, not just the resulting
  globals. The existing NUNA probe already reads BP-relative locals, so the
  mechanism exists; the design is §5.11 (to be detailed in discussion).

## 4. Non-goals

- **N1.** Not a general memory editor — capture is read-only; no poke/write.
- **N2.** Per-instruction hook lives in `core_normal` only. The dynamic/full
  CPU cores are out of scope for v1 (note as a limitation).
- **N3.** No change to guest execution, timing semantics, or save output.
- **N4.** Not resolving **arbitrary** overlay-buffer code addresses beyond
  the existing resolver. Function **entry** detection in overlays *is* in
  scope (exact func-offset + prologue signature, the FN-EXIT trap, §5.4);
  arbitrary mid-overlay address resolution is not.
- **N5.** No arbitrary pointer-graph chasing. At most a single far-pointer
  indirection for dynamically allocated blocks (§5.6).

## 5. Design

### 5.1 Vocabulary

- **Capture target** — a named global to read: its per-EXE DGROUP offset,
  element stride, element count, and the field map describing one element.
- **Field map** — ordered rows `(offset, name, kind, count)` describing one
  element's layout; generated from the C struct.
- **Capture point** — a code landmark (IDA-linear address of a resident
  `j_` stub) at which a target is snapshotted; tagged with a label.
- **Capture mode** — *event* (at a code point; primary) or *periodic*
  (frame/PIC-sampled; the existing `[capture]` path, secondary).
- **Record** — one decoded element instance at one capture point.

### 5.2 Capture targets (generic, multi-target — upstream-first)

A target table generalizes the per-target constants. Target *priority*
follows the divergence chain (G10):

| pri | target | kind | populating fns | per-EXE addr / stride / count | fieldmap |
|---|---|---|---|---|---|
| 1 (next) | `_LAIRS` | far ptr → array | `Generate_Lairs` / `Create_Lair` | TBD (Phase 0 / wiki) | `s_LAIR` |
| 2 | `_CITIES` | far ptr → array | `Generate_Home_Cities` | MGC `DS:0x89D2` (ptr), stride `0x72` | `s_CITY` |
| 3 (done) | `_players` | inline array | new-game init | MGC `0x6900` / WZD `0x9ECA`, `0x4C8` ×6 | `s_WIZARD` |
| — | `_num_players` | scalar i16 | — | MGC `0x87D2` / WZD `0xBD9C` | (i16) |

Per-EXE offsets come from the ReMoM wiki "Data-Segment Memory Layout"
(MGC/WZD columns); resolve each target's exact addr/stride/count + field map
in *that target's* Phase 0, exactly as `_players` was. `_players` is inline
in DGROUP (`runtime_ds : off + i*stride + field.off`); **`_LAIRS`/`_CITIES`
are far pointers to dynamically-allocated blocks**, so they use the
single-indirection read (§5.6).

### 5.3 Field maps and the generator

A generator script parses `struct s_WIZARD` (and any target struct) from
`MOX/src/MOM_DAT.h`:

- For each member: read the `/* 0xNN */` offset comment, the C type, the
  name, and any `[count]`. Resolve count constants
  (`LEN_WIZARD_NAME`, `NUM_HEROES`, `NUM_SPELLS`, `NUM_VAULT_ITEM_SLOTS`,
  `NUM_OVERLAND_ENCHANTMENTS`, `NUM_PLAYERS`, …) from their `#define`s.
- Map C type → `kind`: `U8/I8/U16/I16/U32/I32`, `STR` for `char[]`, and
  **recurse into nested structs** (`s_OWNED_HERO`, `s_WIZ_DIPL`,
  `s_Astr_Data`) so they flatten to `Heroes[0].xyz` rows.
- **Self-check:** independently compute each offset by summing type sizes
  under `#pragma pack(2)` and assert it equals the `/* 0xNN */` comment.
  Mismatch is a hard error (catches a stale comment before it corrupts a
  capture). Stronger variant (later): emit a tiny C program using
  `offsetof`/`sizeof` for compiler-exact offsets.
- **Outputs, one source two forms:** (a) a C header with a
  `static const gd_field_t s_wizard_fields[]` table compiled into the probe
  (G3); (b) a data file with the same schema, loadable at probe init like
  the `.fwv` overlay tables already are (G3b).

### 5.4 Capture points and overlay handling

**Policy:** capture a target at the **FN-EXIT of every function that
populates it**, plus a final snapshot at the save landmark. One snapshot per
populating function isolates each function's effect and pins the *first*
OG↔ReMoM divergence to a specific function. The full v1 capture set for
`_players` (`Newgame_Screen_7__WIP`, `Init_Computer_Players_Wizard_Profile`,
`Init_Computer_Players_Spell_Library`, the `Init_Runtime` children, …) and
the per-function detection spec live in
[CaptureGameData-PLAN.md](CaptureGameData-PLAN.md) §1–§2.

Two firing mechanisms:

- **Resident landmark.** Runtime CS:IP resolves (via `base_offset`) to a
  resident `j_` stub / function address. Used for cross-overlay calls and
  the save point. Exact.
- **FN-EXIT trap** (for intra-overlay functions). Functions called
  intra-overlay (`NX_j_*`) never pass through their resident INT-3F stub, so
  a stub trigger never fires. Instead: detect the function's **entry**
  (resident address, or — for overlay code — resolver-says-`ovrNNN` +
  `IP == func offset` + prologue signature, the proven NUNA technique),
  snapshot its **return address** off the stack, and fire when execution
  reaches that address. See PLAN §2.

Captures are tagged by function name so a field is tracked across the
new-game sequence and the first divergence pinned (the ideation's "walk the
call streams side-by-side" use case).

### 5.5 Capture modes

- **Event mode (primary).** Fire when the live EXE matches the target's EXE
  and runtime CS:IP resolves to a configured landmark. One-shot per landmark
  per EXE activation (re-arm on EXE change), as `[OG_SPELLRANKS]` already
  does.
- **Periodic mode (secondary, existing).** The `[capture]` PIC-sampled CSV
  path remains for time-series scalars. The field-map decode is shared so a
  target can be rendered either way.

### 5.6 Reading guest memory

- Static globals: `real_readN(runtime_ds, off + i*stride + field.offset)`,
  using the live EXE's confirmed DGROUP segment (`og_exes[idx].runtime_ds`).
- Dynamically allocated blocks: one far-pointer indirection — read a
  `seg:off` pointer at a known DGROUP slot, then read fields relative to it
  (the `[capture]` `farptr` form already does this). Deeper graphs are N5.
- x86 little-endian throughout; the generator records signedness/width.

### 5.7 Output / emit format

Human-readable, greppable, one decoded record per capture, e.g.:

```
[GD] target=_players point=Initialize_Events exe=MAGIC.EXE i=1 n=5
[GD] _players[1].wizard_id = 3
[GD] _players[1].name = "Rjak"
[GD] _players[1].spellranks = [N=0 S=0 C=5 L=0 D=5]
...
```

(`[OG_SPELLRANKS]` is retained as a convenience summary line.) Field lines
carry target/index/point so the comparator can align them unambiguously.

### 5.8 ReMoM parity emit (G6)

ReMoM emits the same per-field record at the same logical points (it has the
typed struct, and already logs spellranks via `LOG_DEBUG` at
`INITGAME.c:92`). Same `name = value` formatting → trivial alignment.
ReMoM's real log is `assets/remom_log_new.txt`, not the stderr run log.

### 5.9 Comparator (G7)

A tool reads both sides' `[GD]` records (driven by the same field map),
prints side-by-side per field per capture point with `<<` mismatch markers
(the `analyze-players.txt` style), and **masks** `Unknown_*`/padding rows
that are expected to be uninitialized and would diff spuriously.

### 5.10 Ideation open questions — resolved

- *Per-CPU / core_normal?* Yes; the hook is in `core_normal` (N2 limits to
  that core for v1).
- *Recognizing calls in overlay code?* Live-EXE detection + the runtime→IDA
  resolver; trigger on resident `j_` landmarks, not `NX_j_` intra-overlay
  sites (§5.4).
- *Data-segment addresses?* Per-EXE DGROUP offset read relative to the live
  `runtime_ds` (§5.2/5.6).
- *Code-segment addresses?* IDA-linear via `base_offset` for resident code;
  overlay-buffer code is avoided as a trigger (N4).
- *Dynamically allocated memory?* Single far-pointer indirection (§5.6); N5
  beyond that.
- *Data structures?* Generated field maps (§5.3).

### 5.11 Local/stack-value capture (G11, design TBD — discussion pending)

The struct/global capture above answers "what does memory hold *after* a
function." World-gen divergence (e.g. `Generate_Lairs` driving different
home-city placement) is driven by the **RNG-draw stream**, so we also want
to capture *inputs*: the argument passed to `Random()` at a specific call
site (e.g. `Create_Lair`, MAPGEN.c:2888), matching ReMoM's `[RNG-CALL]`
lines which already log `n` + before/after seed.

Mechanism sketch (the existing NUNA probe already reads BP-relative locals,
`real_readw(ss, bp-0x10)`, so the primitive exists):

- **Fire at the call site or callee entry.** At a `CALL Random` site the
  argument is on the stack just-pushed; at `Random`'s entry it sits past the
  return address (`[SS:SP+2]` for a near call). Identify the site by the same
  overlay-offset + signature technique as the FN-EXIT traps.
- **Read the argument** off `SS:SP`/`SS:BP` per the calling convention, and
  emit `[GD_ARG] <site> Random(n=...)` (and optionally the result).

Open questions for discussion: pinning the exact `Random` call site inside
overlay code; cdecl arg location vs. register; whether to capture per-site or
hook `Random`'s entry and attribute by caller; and how this aligns with
ReMoM's `[RNG-CALL]` for a stream-level diff.

## 6. Implementation plan

1. **Field-map generator + self-check.** Parse `s_WIZARD` from
   `MOM_DAT.h`, resolve constants, recurse nested structs, validate offsets
   against the comments, emit the hard-coded C table. Show the table for
   review before wiring it in.
2. **Generic capture engine.** Add a target table + landmark table to
   `og_sort_trace.h`; replace `og_spellranks_probe` with a table-driven
   `gd_capture` that decodes/emits per §5.7. Verify it reproduces the known
   spellranks exactly.
3. **Multiple capture points** (§5.4) with per-point tags.
4. **ReMoM matching emit** (§5.8) + **comparator** (§5.9); run OG + ReMoM at
   one seed and confirm a clean Stage-0 field-level match.
5. **Externalize** target/landmark/field-map tables to files (G3b) so new
   targets need no rebuild.

## 7. Risks & open questions

- **R1. Uninitialized `Unknown_*`/padding** diff spuriously → comparator
  masks them; generator marks them.
- **R2. Capture-point completeness** — fields populated by later functions
  aren't set at an early landmark. Mitigated by multiple points and the
  save landmark for the complete record.
- **R3. Field-map drift** if struct comments go stale → self-check now;
  `offsetof`-based generation later.
- **R4. Log volume** — full `s_WIZARD` × 6 × N points is large. Acceptable
  for a dev probe; consider a compact mode.
- **R5. Pointer/segment fields** in *other* targets (not `s_WIZARD`, which
  is pointer-free) are environment-dependent → flag/skip in the field map.
- **R6. core_normal only** (N2) — captures are invalid under dynamic/full
  cores; the harness must pin `core=normal` (Stage configs already do).

## 8. Success criteria

- **C1.** `_players[]` captured human-readably at ≥2 landmarks; spellranks
  agree with `[OG_SPELLRANKS]` and the earlier validation.
- **C2.** Field map generated from `MOM_DAT.h`; self-check (computed offsets
  == comment offsets, total == `0x4C8`) passes.
- **C3.** A second target captured by adding only a target-table row (no new
  decode code) — proving genericity.
- **C4.** Comparator shows a full-field OG↔ReMoM match for Stage 0
  `_players` (expected to match).
- **C5.** No measurable guest slowdown and no change to `SAVE1.GAM`.
