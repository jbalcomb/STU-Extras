# OG `Random()` Argument Capture — Instrumentation Notes

**Companion to:** [CaptureGameData-PRD.md](CaptureGameData-PRD.md) §5.11 (local/stack-value
capture) and [CaptureGameData-PLAN.md](CaptureGameData-PLAN.md) §1b.
**Probe file:** `dosbox-0.74-3/src/misc/og_sort_trace.h`
**Scope:** MAGIC.EXE (`OG_SR_MGC_EXE_IDX`) new-game, the `[OG_RNG]` stream.

> **OUTCOME (2026-06-21 re-run): H0 confirmed.** After the SS-read fix all
> 58,389 Random calls run on `SS=2373`, every call is `kind=farD`, and
> `n@+4 == raw4` everywhere (0 disagreements). The SS-change ring shows one SS
> setup (`2E18->2373` at Borland `start+BD`, `es=0192`). The old phantom
> `ss=0192` was the real **ES** bleeding through the out-of-bounds
> `Segs.val[uninit]` read; `n=1762` was garbage. **No real RNG divergence
> existed at "call 67."** The stream is now trustworthy.

---

## 1. The problem we are chasing

`og_rng_probe` hooks the resident `Random()` entry (MGC `seg022:Random`, IDA
`0x1CC84`) and logs the call argument `n`. Every call executes at the **same**
`cs:ip = 0E5D:00D4` (Random's prologue) — so they are all genuine `Random`
calls, not `base_offset` false positives.

For the overwhelming majority of calls the Borland large-model far-call layout
holds: at entry the stack is `[SS:SP]=ret_IP`, `[SS:SP+2]=ret_CS`,
`[SS:SP+4]=n`. Reading `n` at `+4` matches ReMoM exactly (e.g. `n=3`, `n=5`).

**The anomaly:** five back-to-back calls (≈ call 67–71) arrive on stack
`SS=0192`, from caller `ret=9A08:B1D2`, and `[SS:SP+4]` reads **1762**, where
the equivalent ReMoM call passes **14**. The value `14` is *nowhere* on that
stack (`+0..+16` were dumped). The dominant stack across the run is actually
`SS=6F9A` (~4743 / 5000), with `2373`, `6D21`, `0192` appearing in smaller
contexts — i.e. MoM runs `Random` from **several different stacks**.

The user's constraint is correct and is our north star: **the argument must be
on the stack or in a register.** A single function compiled by one Borland C++
toolchain does not have two calling conventions. So `+4 == 1762` means one of:

- **(H0) Probe bug — the SS read itself was garbage.** *(Found during this
  work; leading suspect.)* The original `og_rng_probe` read SS as
  `Bit16u ss = SegValue(ss);`. In C++ the just-declared local `ss` shadows the
  `SegNames` enumerator `ss` *inside its own initializer*, so this passes an
  **uninitialized local** to `SegValue`, which is `return Segs.val[index];`
  over a `Bit16u val[8]` array — an out-of-bounds / indeterminate read (the
  exact trap the file header warns about; the file convention is `*_val`
  locals). Built `-O2`. Every downstream value (the reported SS, the stack
  window, `ret_cs:ret_ip`, and `n`) was read *through that SS*, so a bad SS
  poisons all of them. This matches the symptoms: SS scattering across
  `6F9A/2373/0192/6D21` (out-of-bounds reads return scattered values), most
  calls still looking right (the stale slot usually held a sane value), and the
  occasional `n=1762`. **Fixed** (`ss_val`); the re-run is the proof — if the
  anomaly vanishes, H0 was it.
- **(H1) Wrong arg displacement** — this caller reaches `Random` by a *near*
  call (or via a thunk), so the arg is at `+2`, not `+4`.
- **(H2) Extra frame words** — a dispatcher/thunk pushed state between the
  arg-push and the `call`, so the arg is deeper than `+4`.
- **(H3) Stack switch** — the args were pushed on stack A, then `SS` was
  switched (to `0192`) before control reached `Random`, so `[0192:SP+4]` reads
  an unrelated stack.
- **(H4) DOSBox-internal timing** — the registers the hook samples don't
  correspond to the instant the args were pushed.

The three instrumentation additions below were built to *discriminate* between
these, not to assume any one of them.

---

## 2. What was added (`og_sort_trace.h`)

### 2a. Byte-window view (`og_fmt_bytes`, `og_fmt_window`)

General helpers (shared-helpers section, available to every probe). A "window"
formats bytes **before and after** a point with a `|` marker at the center, so a
matched pattern shows its context on both sides.

The decisive use: the bytes **immediately preceding a return address are the
`CALL` instruction that produced it.** Dumping `og_fmt_window(ret_cs, ret_ip,
before=8, after=2)` reveals the actual call opcode at each site:

| opcode (ends at `ret_ip`) | form | arg displacement |
|---|---|---|
| `9A lo lo seg seg` | `CALLF` direct (far) | `+4` |
| `FF /3` (modrm reg=3) | `CALLF` indirect (far) | `+4` |
| `E8 rel rel` | `CALL` direct (near) | `+2` |
| `FF /2` (modrm reg=2) | `CALL` indirect (near) | `+2` |

`og_call_arg_disp(ret_cs, ret_ip, kind)` implements this classification and
returns the correct displacement (or `-1` if unrecognized). The RNG probe then
reads the arg at the **classified** displacement, and also reports the raw `+4`
read for comparison. This directly tests **H1**.

### 2b. Segment-register state + change tracking (`og_segreg_track`)

Runs every instruction (MAGIC only). Keeps previous `SS/DS/ES`; on an **`SS`
change** it pushes a record into a 64-entry ring capturing
`(seq, cs:ip, SS old→new, DS, ES, SP, BP)`. `DS`/`ES` values ride along in each
record; setting `OG_SEGCHG_LIVE 1` additionally streams every `SS`/`DS`/`ES`
change to stderr as `[SEGCHG]`.

The ring focuses on `SS` (the mystery) so high-churn `ES` string-op changes
don't evict the signal. When the RNG probe fires on an interesting call it dumps
the recent ring as `[OG_RNG_SEG]` — so we can see **whether `SS` was switched
just before `Random`, and by what code.** This tests **H3** (and exposes **H4**
if `SS` changed at an implausible `cs:ip`).

### 2c. Flag / ignore table (`og_rng_flags`)

A small table of known instances, matched by resolved caller substring, `SS`,
and/or raw `+4` value, each carrying a human note and an optional known-good
`expect_n`. When a call matches, its `[OG_RNG]` line is tagged `[KNOWN-GOOD]`
with the note, so a downstream stream-diff can ignore it.

Seeded with the call-67 instance, per the user's instruction to **specifically
note that `+4` should be `14`, not `1762`**:

```
{ caller=NULL, ss=0x0192, raw4=1762, expect=14,
  "KNOWN-GOOD: +4 reads 1762 but true n should be 14 — arg not at far +4 here" }
```

Once the byte-window + caller resolution name this caller, re-key the entry on
the resolved function (`caller_substr`) so it survives a different overlay load
/ seed instead of relying on the raw `SS`/value.

---

## 3. Reading the output

```
[OG_RNG] call=N cs=0E5D:00D4 ss=SSSS:SPSP kind=KIND n=ARG raw4=RAW ret=CCCC:IIII=CALLER [KNOWN-GOOD: ...]
[OG_RNG_BYTES] call=N callsite ret=CCCC:IIII  <8 bytes | 2 bytes>     ← the CALL instruction
[OG_RNG_BYTES] call=N random  0E5D:00D4       <prologue window>       ← confirms it's Random
[OG_RNG_STK]   call=N ss:sp=SSSS:SPSP +0..+16 = ...
[OG_RNG_SEG]   call=N recent SS changes: seq=.. cs:ip=.. ss OLD->NEW ds=.. es=.. sp=.. bp=..
```

- `n` = arg read at the **classified** displacement (best estimate).
- `raw4` = the old unconditional `+4` read (for back-compat / comparison).
- `kind` = `farD`/`farI`/`nearD`/`nearI`/`?`.
- `CALLER` = `og_describe(ret_cs, ret_ip)` — resident or overlay (R6) name.

The detail lines (`_BYTES`/`_STK`/`_SEG`) print for the first 120 calls and for
any flagged call, to bound output.

### Decision procedure for the `0192` anomaly

0. **First, just re-run the fixed probe.** If `SS` no longer scatters and the
   `0192`/`1762` instance is gone, it was **H0** (the SS-read UB) — done.
1. `kind` for the `0192` caller — if `nearD`/`nearI`, **H1** confirmed and `n`
   now reads the real arg at `+2`. (Note: `+2` on that stack was `9A08`, not
   `14`, so a plain near call alone does not explain `14`.)
2. `[OG_RNG_SEG]` — if `SS` flipped to `0192` at a `cs:ip` *between* the
   caller and `Random`, **H3**: the args are on the pre-switch stack; widen the
   stack scan to the old `SS` recorded in the ring.
3. If the `callsite` bytes are a normal far `9A …` and no `SS` switch occurred,
   then `14` genuinely is not in this frame → re-open the "different logical
   call / stream misalignment" reading and compare **by resolved caller**, not
   by call index.

---

## 4. Build / run

Same as the existing probe: rebuild DOSBox (`OG_SORT_TRACE 1` is set in the
header), run a new game in MAGIC.EXE, capture stderr. Toggles:

- `OG_SEGCHG_LIVE` (default `0`) — stream every `SS`/`DS`/`ES` change.
- The `_BYTES`/`_STK`/`_SEG` detail cap (120) is in `og_rng_probe`.

Output is stderr `[OG_RNG*]` / `[SEGCHG]`; grep by prefix.
