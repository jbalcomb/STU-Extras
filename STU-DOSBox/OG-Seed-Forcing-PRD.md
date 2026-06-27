# PRD — OG RNG Seed Forcing (deterministic replay)

Status: Draft (v1 scope: MAGIC New-Game world-gen seed)
Owner: matchup project (OG-MoM ↔ ReMoM)
Related: CaptureGameData-PRD.md, OG-RNG-Instrumentation.md

## 1. Problem

OG-MoM (instrumented STU-DOSBox) seeds its PRNG from the wall clock via
`Randomize()` on every New Game / load, so **every run uses a different seed**.
Several matchup divergences are seed-dependent — notably the `_world_maps`
overrun-inject bug, which only manifests when OG's adjacent `UU_TBL_1` leftover
(read out-of-bounds by `Generate_Terrain_Specials`) is **non-zero** (~2 of 3
seeds). Because no two runs share a seed, we cannot:

- diagnose a non-zero/divergent case under the `[TS-PROBE]`, then
- **confirm the fix on the same case** (the next run is a different map).

This blocks rigorous "fix → prove fixed" iteration on every seed-dependent
divergence, not just the overrun one.

## 2. Goal

Pin OG's PRNG seed to a **configured value** so a run is byte-for-byte
reproducible. Workflow this unlocks:

1. Random run → `pair0` diverges → `extract-seed` reports seed `S`.
2. Force seed `= S` → replay reproduces the **exact** divergence.
3. Diagnose (probe + `.fwv` inject-check) → fix.
4. Replay `S` → **confirm** the divergence is gone (and re-run a few other
   forced seeds to guard against regressions).

## 3. Non-goals (v1)

- Forcing the WIZARDS Load-Game reseed (pair 1). The New-Game **save** (slot 8)
  is already deterministic once MAGIC's seed is pinned, so the regression target
  and the post-load `_CITIES` capture are covered without it. WZD forcing is a
  documented follow-on (§8).
- Changing ReMoM. `extract-seed` already feeds OG's seed to ReMoM, so once OG is
  pinned both sides are deterministic automatically.
- Replacing the time source / faking the BIOS clock (heavier, unnecessary —
  we overwrite the *result*, not the input).

## 4. Background — the seed mechanism (verified)

| | seed addr (DGROUP) | `Randomize` store IP | `Set_Random_Seed` store IP |
|---|---|---|---|
| MAGIC.EXE | `DS:0x431E` (4 bytes) | `0x00CD` | `0x0080` |
| WIZARDS.EXE | `DS:0x7846` (4 bytes) | `0x00D1` | `0x0084` |

Both reseed paths end with `A3 .. ..` (`mov [_random_seed+2], ax`) — the high
word; the low word is stored just before. After the store IP, `DS:seed_off`
holds the freshly-seeded 32-bit LCG state. `extract-seed` reads exactly this
value as the run's seed `S`. The matchup's "segments" are delimited by these
reseeds (seg1 = MAGIC New Game world-gen).

## 5. Requirements

### Functional
- **F1** Read a pinned seed `S` from configuration at startup (env var, e.g.
  `OG_FORCE_SEED=0x0012697E`; absent/empty ⇒ feature off, normal random behavior).
- **F2** When forcing is on, after the **MAGIC New-Game world-gen reseed** completes,
  overwrite `MGC DS:0x431E` (4 bytes, little-endian) with `S` before the first
  world-gen `Random()` draw consumes it.
- **F3** Force **only** the seg1 anchor reseed, not earlier MAGIC reseeds (menu
  RNG etc.) — forcing the wrong reseed would desync unrelated segments.
- **F4** Emit a one-line confirmation to the OG log, e.g.
  `[SEED-FORCE] MAGIC seg1 reseed overwritten: was 0x???????? now 0x????????`.
- **F5** Idempotent / one-shot per MAGIC activation (re-arm on EXE change), so a
  later legitimate reseed isn't clobbered.

### Non-functional
- **N1** Zero effect when off (env unset) — no behavior or perf change.
- **N2** No new build flags; lives in the existing `og_sort_trace.h` per-instruction
  hook alongside the RNG instrumentation it reuses.
- **N3** Deterministic: same `S` ⇒ identical OG output (validated, §7).

## 6. Design

### 6.1 Interception point
Reuse the existing reseed-store detection. The per-instruction probe already
recognizes the `A3 ..` store at the MAGIC `Randomize` store IP (`0x00CD`). The
store writes *before* it executes, so overwrite on the **instruction immediately
after** the store completes (or on the next probe tick where `ip` has advanced
past the store), writing `S` to `DS:0x431E`.

### 6.2 Which reseed is "seg1"
The RNG instrumentation already counts reseed segments (Randomize/Set_Random_Seed
anchors). seg1 = the MAGIC New-Game world-gen reseed. v1 forces the reseed that
opens seg1 — identified by the existing segment counter (the Nth MAGIC reseed),
not by wall position. Decision to lock in the plan (§7, P1): confirm the seg1
anchor index empirically from one instrumented run before wiring the force.

### 6.3 Config
- `OG_FORCE_SEED` env var, parsed as hex/dec 32-bit. Read once at probe init.
- Harness sets it per replay (the baselines runner exports it before launching
  dosbox). Unset ⇒ off.

### 6.4 Overwrite
```
real_writew(MGC_DS, 0x431E, S & 0xFFFF);
real_writew(MGC_DS, 0x4320, (S >> 16) & 0xFFFF);
```
guarded by: forcing-on, MAGIC active, seg1 anchor reached, one-shot flag.

### 6.5 Pipeline integration
`extract-seed` is unchanged — it reads `DS:0x431E` post-reseed, which is now `S`,
so ReMoM is fed `S` automatically. The runner gains an optional `--force-seed S`
that exports `OG_FORCE_SEED` and tags the run dir.

## 7. Plan (phased)

- **P0 — Confirm reproducibility assumption.** Force a fixed `S` (hardcoded),
  run twice, diff the OG `[GD]`/`[OG_RNG]` output. If identical ⇒ memory state is
  deterministic given the seed and we're good. If not ⇒ surface the other
  non-determinism source before proceeding (gate the whole feature on this).
- **P1 — Identify the seg1 anchor.** One instrumented run; read the reseed
  segment counter to pin which MAGIC reseed opens world-gen. Lock the index.
- **P2 — Implement F1–F5.** Env parse + one-shot overwrite at the seg1 anchor +
  `[SEED-FORCE]` log line. Off when env unset (N1).
- **P3 — Runner flag.** `--force-seed` exports `OG_FORCE_SEED`, records it in the
  run dir for provenance.
- **P4 — Validate determinism (N3).** Same `S` twice ⇒ identical `compare-rng`
  and `compare-gd` outputs.
- **P5 — Reproduce the overrun divergence.** Random runs until `pair0` diverges;
  capture `S`; force `S`; confirm the divergence reproduces deterministically.
  Hand off to the overrun-inject fix work (diagnose → fix → replay `S` to confirm).

## 8. Follow-ons (out of v1)
- Force the WIZARDS Load-Game reseed (`WZD DS:0x7846`, store IP `0x00D1`) for full
  gameplay-phase determinism (a second entry in the same force table).
- Force a **table** of named reseeds (per-segment) if multi-segment determinism
  is later needed.

## 9. Risks / open questions
- **R1 (gating)** Memory state may carry run-to-run non-determinism beyond the
  seed (uninitialized reads, DOS boot state). P0 explicitly tests this; if it
  fails, seed-forcing alone won't give reproducibility and we rethink.
- **R2** Forcing the wrong reseed desyncs unrelated segments (F3 mitigates;
  P1 confirms the anchor).
- **R3** The overrun content (`UU_TBL_1` leftover) is assumed deterministic given
  the seed; P5 is the real test of that assumption (it's the whole point).
