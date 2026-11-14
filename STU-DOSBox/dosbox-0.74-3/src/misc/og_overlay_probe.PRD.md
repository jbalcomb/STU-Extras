# PRD: Borland Overlay Manager Hook for OG MoM Trace

**Status:** Draft (revision 2)
**Owner:** prng-port byte-match audit
**Scope:** A new probe in [og_sort_trace.h](og_sort_trace.h) and an
auto-annotation layer for the existing trace output.

---

## Revision history

- **r1** -- initial draft.  Strategy: capture `AX = SEG_IDX` at INT 3F
  fire time, read `JMP FAR` target seg from stub bytes, build
  `runtime_cs -> seg_idx` map; resolve to overlay name only
  (function-name resolution explicitly out of scope as N1).
- **r2** -- earlier revision.  Three requirement / strategy changes:
  - Goal added: function-level resolution
    (`j_Draw_Logos+0`), not just overlay name.  N1 in r1 removed.
  - Strategy replaced: r1's `AX`/`JMP FAR` capture did not survive
    contact with the runtime stub-table layout (header words are
    DS-near pointers to string tables, not seg_idx; runtime stub
    groups are CRT-constructed, not in EXE).  Replaced with
    arithmetic runtime->IDA mapping anchored on a two-stage
    data-segment probe: (1) `OG_DGROUP_SIG` -- the canonical Borland
    C++ data-segment signature placed at the start of every program's
    data segment -- locates DGROUP at runtime; (2) `__EXENAME__`
    literal scan in DS distinguishes MGC from WZD.
  - New requirement added: back-fill of any INT 3F fires that occur
    before EXE identity is established (see G4 below).

- **r3** -- current.  R6's byte-pattern resolver for overlay-buffer
  code (rev 2 of R6) was demonstrated to give wrong function names
  for `_unit` writers W2/W3/W4 (it can land at a coincidental match
  in another overlay or another function in the same overlay).
  Trace output that references runtime CS values is also unstable
  across sessions and should not appear in user-facing output.

  New direction (this revision):
  - Stop relying on byte-pattern matching to identify the loaded
    overlay for a given executing code location.
  - Stop emitting runtime CS values to the user.  Trace output uses
    only stable identifiers: `(overlay, function, offset-within-function)`
    in IDA's terms.
  - Goal G6 added: **observe what the BC++ 3.0 overlay manager
    actually does on dispatch** rather than speculate.  Tool for this
    is a post-INT-3F instruction-follow probe (see new G6 and
    §5.7 below).  The observations from this probe inform the next
    resolver design (e.g., back-patched-thunk reading, or some
    other authoritative mechanism we have not yet validated).
  - All earlier claims sourced from the TP6 article (Bucknall, .EXE
    Magazine 1992) are demoted from "facts" to "hypotheses".  TP6 is
    a related but distinct system.  Anything we want to assert about
    BC++ 3.0's manager must come from observation of the running
    program, not extrapolation.

---

## 1. Context

WIZARDS.EXE and MAGIC.EXE are built with Borland Turbo C++'s overlay
system: code is split into a resident base segment plus N "overlays"
loaded into shared buffer slots on demand.  Each overlay function call
routes through a stub that fires `INT 3F` -- the Borland overlay
manager -- which loads the requested overlay if it isn't already
resident, then transfers control.

The byte-match audit work (Stage 1, Stage 2, ...) instruments OG via
[og_sort_trace.h](og_sort_trace.h), a hook called once per executed
instruction from `CPU_Core_Normal_Run`.  Existing probes emit lines
like:

```
[OG_UNIT] writer cs=4681 ip=0B1A  _unit  5 -> 0
[OG_SORT] cs=4525 CMP    idx2=6 spl2=54 idx1=5 cost1=80 cost2=75  -> SHIFT
```

The `cs=XXXX` values are *runtime* CS values: a different segment each
session, depending on what DOS handed the program at load time and how
the overlay manager has paged things in.  We have static
overlay-function tables exported from IDA for both EXEs:

- [/home/jbalcomb/STU/devel/STU-Extras/STU-DOSBox/MGC-overlay-table.fwv](file:///home/jbalcomb/STU/devel/STU-Extras/STU-DOSBox/MGC-overlay-table.fwv)
- [/home/jbalcomb/STU/devel/STU-Extras/STU-DOSBox/WZD-overlay-table.fwv](file:///home/jbalcomb/STU/devel/STU-Extras/STU-DOSBox/WZD-overlay-table.fwv)

Format: `SEG_ SEG_EA SEG_NAME FUNC_EA FUNC_NAME`.  These give us
`IDA linear address -> (seg_name, func_name, func_offset)`.  What's
missing is the *runtime* link: `runtime CS:IP -> IDA linear address`.
Once we have that link, we can resolve any runtime CS:IP to a named
function with intra-function offset.

## 2. Problem

Trace output is not self-describing.  Every diagnosis loop pays a
cross-reference tax to translate `cs=4681` into "ovr061
(Next_Unit_Nearest_Available)" before reasoning about the data.

We want every interesting trace line to carry the named function and
offset inline, so reading the trace doesn't require keeping an IDA
window open.

## 3. Goals

- **G1.** Detect every `INT 3F` invocation during the session and emit
  a diagnostic dump of the fire site (registers, stack window, code
  window with `>CD 3F<` marker, candidate return addresses).
- **G2.** Identify which executable (MAGIC.EXE or WIZARDS.EXE) is the
  process under trace, AND locate its data segment in runtime memory.
  The identification mechanism MUST use intentional, stable Borland
  C++ anchors -- byte fingerprinting or arbitrary sentinel scanning is
  not acceptable.  See §5.1: stage 1 is `OG_DGROUP_SIG` (Borland's
  intentional data-segment signature), stage 2 is `__EXENAME__` (the
  CRT exename literal that distinguishes MGC from WZD).
- **G3.** Provide a helper, `og_runtime_to_name(cs_val, ip_val)`, that
  resolves any runtime CS:IP to `(seg_name, func_name, func_offset)`
  via arithmetic against the active EXE's static overlay table.
  Emits one `[OG_OVR]   resolved: ...` line per call.
- **G4.** Back-fill any INT 3F fires that occur before EXE identity
  is established.  Borland CRT startup performs overlay calls while
  DS still points to the PSP (before the CRT has set DS to the
  program's data segment), which means `__EXENAME__` is not yet
  reachable via the runtime DS even though the INT 3F handler is
  already installed.  Such fires MUST be queued, then resolved
  retroactively the first time `__EXENAME__` is found, so no fire is
  silently dropped.
- **G5.** Add zero per-instruction cost when the probe finds nothing
  interesting (cheap reject on the first byte: `0xCD`).
- **G6.** **Empirically observe** what the BC++ 3.0 overlay manager
  does on dispatch -- without speculating from related-but-distinct
  systems (TP6) or from byte-pattern matches that are known to
  misidentify (R6 rev 2 byte search).  Implementation: a post-
  INT-3F instruction-follow probe (§5.7) that logs the next N guest
  instructions executed after each INT 3F fire, verbatim.  The
  resulting trace shows the manager's actual sequence: handler entry,
  bookkeeping, transition to overlay code, function entry, and (if
  it occurs) the back-patching of the originating thunk.  The
  observations from G6 inform what authoritative resolver to build
  next; until then, function attribution for overlay-buffer code is
  marked as "unverified" rather than guessed.

  Output: stable identifiers only.  Runtime CS values are session-
  dynamic and must NOT appear in trace output meant for human
  interpretation, even if they're necessary internally to read
  memory.

## 4. Non-goals

- **N1.** Cross-overlay chain support.  If a third party has chained
  itself ahead of Borland's INT 3F handler, we may capture noise.  In
  practice DOS games at this era don't do that, so accept the
  limitation.
- **N2.** Production-build perf tuning.  The trace is a dev/debug
  build option; reasonable per-instruction cost is fine.
- **N3.** Persistence across sessions.  The active EXE,
  base_offset, and any queued/resolved fires rebuild from scratch
  each launch; we don't write to disk.
- **N4.** Supporting EXEs other than MAGIC.EXE and WIZARDS.EXE.

## 5. Design

### 5.1 Detecting DGROUP and identifying the running EXE

Detection has two distinct concerns that must not be conflated:

| Layer | What it answers | Mechanism | Scope |
|-------|-----------------|-----------|-------|
| A | "Is DS the data segment of some Borland C++ program?" | `OG_DGROUP_SIG` scan | Generic toolchain check; applies to any Borland C++ DOS program |
| B | "Is that program one we have an IDA overlay table for?" | `__EXENAME__` literal scan | Project-specific; only matches MGC or WZD |

Layer A is a **toolchain** check.  Layer B is a **project identity**
check.  Matching A says nothing about identity; matching B without A
would be ungated and unsafe (we'd be guessing where DGROUP lives).
Both must succeed before we bind `base_offset` and start resolving.

**Layer A -- generic Borland DGROUP signature.**  The Borland C++
linker places this fixed 47-byte marker at the head of DGROUP for
every program it builds:

```
00 00 00 00 "Borland C++ - Copyright 1991 Borland Intl." 00
```

This is a toolchain artifact, not a program-identity artifact.  Any
Borland C++ DOS program built with this linker version carries the
same signature.  Detection scans a tiny window `DS:0x0000..0x0010`
for these bytes and returns the offset, or -1 if not found.  Matching
here tells us "DS points at some Borland C++ DGROUP" -- nothing more.

**Layer B -- project-specific `__EXENAME__` literal at its known IDA
address.**  The Borland CRT stores the program's executable filename
as a NUL-terminated literal at a fixed compile-time address.  Per
the IDA dump:

- MAGIC.EXE:   `__EXENAME__` = `"magic.exe\0\0\0"`,   IDA `3158:0248`  (linear `0x317C8`)
- WIZARDS.EXE: `__EXENAME__` = `"wizards.exe\0"`,    IDA `3383:0580`  (linear `0x33DB0`)

For each candidate EXE, use the address directly -- no blind scan.
Layer A gave us `runtime_sig_linear`; layer B hypothesizes
`base_offset` per candidate and checks the exact predicted location:

```
hyp_base               = runtime_sig_linear - candidate.sig_ida_linear
exename_runtime_linear = candidate.exename_ida_linear + hyp_base
exename_seg            = exename_runtime_linear >> 4
exename_off_lo         = exename_runtime_linear & 0xF
```

Read `strlen(candidate.exename)+1` bytes at `(exename_seg,
exename_off_lo)` and compare EXACTLY (case-sensitive, NUL-terminated)
against `candidate.exename`.  If they match, this is the active EXE.

Rationale for precision-over-coverage: MAGIC.EXE's data contains both
its own lowercase `__EXENAME__` (`"magic.exe"`) AND an uppercase
sibling reference (`"WIZARDS.EXE"` -- used to launch the game).  A
tolerant scan (case-insensitive, or any-DS-offset) would match the
sibling reference against the `"wizards.exe"` pattern and
misidentify MAGIC.EXE as WIZARDS.EXE.  Checking the IDA-known
address only avoids the entire class of cross-reference false
positives.

**Outcomes:**

- Layer A fails -> DS is not (yet) a Borland C++ DGROUP.  Silent
  bail; try again on the next fire.
- Layer A succeeds, Layer B fails -> some Borland C++ program is
  running, but it's neither MGC nor WZD.  We have no overlay table
  for it; emit a "not resolving" line and bail.  This is the correct
  behavior for "we got run on the wrong EXE" -- we don't pretend.
- Both succeed -> bind `base_offset` per §5.2 and resolve from here
  forward.

**Chained processes.**  Detection runs on every executed instruction.
After the initial bind, the fast path early-returns when DS still
matches the bound EXE's `runtime_ds`.  When DS shifts to a value
that isn't the bound EXE's DGROUP (most commonly: the program briefly
addresses non-DGROUP memory; less commonly but importantly: a child
process has taken over after `EXEC`), the slow path re-runs Layer A.
If Layer A fails on the new DS, the binding is left undisturbed.  If
Layer A succeeds AND Layer B identifies a DIFFERENT project EXE, the
detector emits

```
[OG_OVR] active EXE CHANGED: NAME -> NAME (chained process)
```

flips the old EXE's `active=0` and the new one's `active=1`, and
recomputes `base_offset` for the new EXE.  This is critical for the
Stage 2 capture flow, which starts under MAGIC.EXE (the setup
launcher) and transitions to WIZARDS.EXE (the gameplay binary)
mid-session.

### 5.2 Binding base_offset

When both layers succeed, the IDA position of OG_DGROUP_SIG is known
for the identified EXE (`sig_ida_linear`).  Compute:

```
runtime_sig_linear = (ds << 4) + sig_off
base_offset        = runtime_sig_linear - sig_ida_linear
```

The signature -- not the exename literal -- is the correct anchor:
its IDA address is canonical for that EXE's DGROUP head, while the
exename literal's runtime location in DS may not correspond to its
canonical IDA position (see Layer B note above).

This `base_offset` is correct for DATA reads and for resident-image
code (verified empirically: fire #1 with CS=2366 resolves to
`stub058:j_Draw_Logos+0`, an expected resident overlay stub).  For
code in overlay buffers (loaded at runtime paragraphs separate from
the resident image), the single base_offset is NOT correct -- those
need per-overlay CS->seg_idx mapping; see R6.

### 5.3 Runtime-to-IDA arithmetic

For any runtime `cs_val:ip_val`:

```
runtime_linear = (cs_val << 4) + ip_val
ida_linear     = runtime_linear - base_offset
```

Then locate `(seg_name, func_name, offset)`:

1. Find the segment `s` in the active EXE's segs[] with the largest
   `seg_ea` <= `ida_linear`.
2. Within that segment's `seg_idx`, find the function `f` in funcs[]
   with the largest `ip_offset` (relative to segment) <= the offset
   of `ida_linear` within the segment.
3. Emit:
   `[OG_OVR]   resolved: NAME  IDA SEG_NAME:OOOO = FUNC_NAME+X`

### 5.4 Pre-detection fire back-fill

`og_detect_active_exe(ds_val)` is called from `og_sort_trace_check`
on every executed instruction, not just at INT 3F fires.  This means
detection binds as soon as the CRT first puts DGROUP into DS, which
in practice happens BEFORE the first INT 3F fire.  In current
empirical runs both fire #0 and fire #1 resolve natively without
back-fill.

The back-fill mechanism is retained as a safety net to satisfy G4
("no silent drops"): if any INT 3F probe call observes
`og_active_exe_idx < 0`, push that fire's `(cs, ip)` onto a small
pending-fires queue.  The first time detection succeeds, drain the
queue and emit one `[OG_OVR]   backfilled fire #N: ...` line per
queued entry.  Queue size is bounded (16 entries); overflow emits a
one-line warning and drops further entries.

### 5.5 Data structures

```c
struct og_seg_info {
    Bit16u  seg_idx;
    Bit32u  seg_ea;
    char    seg_name[24];
};

struct og_func_info {
    Bit16u  seg_idx;
    Bit16u  ip_offset;   /* offset within segment */
    char    func_name[64];
};

struct og_exe_info {
    const char*    path;             /* path to MAGIC.EXE / WIZARDS.EXE on host */
    const char*    ovrtab_path;      /* path to .fwv overlay table */
    const char*    label;            /* "MAGIC.EXE" / "WIZARDS.EXE" */
    const char*    exename;          /* "magic.exe" / "wizards.exe" */
    Bit32u         exename_file_offset;  /* reference: file offset of __EXENAME__ */
    Bit32u         exename_ida_linear;   /* IDA linear of __EXENAME__ */
    Bit32u         sig_ida_linear;       /* IDA linear of OG_DGROUP_SIG start */
    unsigned char* data;
    size_t         size;
    Bit16u         header_size;      /* read from MZ header */
    struct og_seg_info  segs[OG_OVRTAB_MAX_SEGS];
    struct og_func_info funcs[OG_OVRTAB_MAX_FUNCS];
    unsigned       seg_count;
    unsigned       func_count;
    int            ovrtab_loaded;
    int            active;
    Bit32s         base_offset;      /* runtime_sig_linear - sig_ida_linear */
    Bit16u         runtime_ds;       /* DS register value observed at detection */
};

static struct og_exe_info og_exes[OG_EXE_COUNT];
static int og_active_exe_idx = -1;

struct og_pending_fire {
    Bit16u cs_val;
    Bit16u ip_val;
    unsigned fire_seq;   /* original fire #N for log clarity */
};
static struct og_pending_fire og_pending_fires[16];
static unsigned og_pending_count = 0;
```

### 5.6 Post-INT-3F instruction-follow probe (new in r3)

Purpose: empirically observe what the BC++ 3.0 overlay manager does
after each INT 3F dispatch -- handler logic, transition out of the
handler, function entry into the loaded overlay, and any back-patching
of the originating thunk -- without speculating from a related-but-
different system (TP6) or from byte-pattern matches.

Mechanism: when the existing INT 3F probe fires, set a per-fire
"log next N instructions" counter (N=32 default).  In the per-
instruction hook, if the counter is non-zero, log one
`[OG_OVR_FOLLOW]` line for the current instruction (CS:IP + first
8 bytes verbatim -- 8 covers any single x86 instruction encoding)
and decrement.  When N hits zero, stop until the next INT 3F fire
resets it.

Per-fire output looks like:

```
[OG_OVR_FOLLOW] +01  cs=SSSS:OOOO  bytes: AA BB CC DD EE FF GG HH
[OG_OVR_FOLLOW] +02  cs=SSSS:OOOO  bytes: ...
...
[OG_OVR_FOLLOW] +20  cs=SSSS:OOOO  bytes: ...
```

The `+NN` counter is the instruction's position in the post-fire
sequence (1 = instruction after CD 3F, 32 = the 32nd one).

This is a TEMPORARY diagnostic probe.  Its purpose is to produce
the evidence base for a follow-up design decision.  Once we have
characterized the manager's dispatch mechanism, we delete or gate
this probe behind an explicit debug flag.

Note: CS values DO appear in this probe's output, because the whole
purpose is to study the dispatch mechanism mechanically.  This is
the one exception to the "no runtime CS in user-facing output"
rule (G6), and only because the consumer of these lines is a
short-term RE task, not ongoing trace interpretation.

### 5.7 Trace line vocabulary

```
[OG_OVR] INT 21/25 install INT 3F vector = SSSS:OOOO
[OG_OVR] INT 3F site cs=SSSS ip=OOOO  -- new fire site
[OG_OVR] fire #N  cs=SSSS:OOOO  ax=... bx=... ...
[OG_OVR] Borland DGROUP detected: signature at DS:OOOO
[OG_OVR] active EXE = NAME  __EXENAME__("name") at runtime SSSS:OOOO
[OG_OVR] base_offset = RRRRRRRR - IIIIIIII = +/-N
[OG_OVR] active EXE CHANGED: NAME -> NAME (chained process)
[OG_OVR] Borland DGROUP at DS:OOOO, but program is neither MGC nor WZD -- not resolving
[OG_OVR]   resolved: NAME  IDA SEG_NAME:OOOO = FUNC_NAME+X
[OG_OVR_STUB] fire #N  cs:0000 (segment descr): <32 hex bytes>
[OG_OVR_STUB] fire #N  parsed (from cs=CC:0000): int_code=... fileoff=... codesize=...
[OG_OVR_STUB] fire #N  workarea: <16 hex bytes>
[OG_OVR_FOLLOW] +NN  cs=SSSS:OOOO  bytes: BB BB BB BB BB BB BB BB
```

The `[OG_OVR_FOLLOW]` lines are produced by the post-INT-3F probe
described in §5.6 -- the temporary diagnostic for understanding the
manager's actual dispatch mechanism.

## 6. Implementation plan

1. **Add `og_exe_info` table + loader** -- two entries (MAGIC,
   WIZARDS), with paths/exenames/file-offsets.  `og_exe_load(e)` mmaps
   the EXE bytes, parses MZ header for header_size, and loads the
   .fwv overlay table into segs[]/funcs[].  Done.
2. **Add `og_detect_active_exe(ds)`** -- DS scan for the lowercase
   exename strings; on match, compute base_offset, set
   `og_active_exe_idx`, emit `active EXE = ...` line.  Done.
3. **Add `og_runtime_to_name(cs, ip)`** -- arithmetic mapping +
   seg/func lookup; emits one resolved-line.  Done.
4. **Add pending-fire queue + back-fill** -- queue pre-detection
   fires; on first successful detect, drain the queue and emit
   `backfilled fire #N: ...` lines.  TODO.
5. **Wire into INT 3F probe** -- call `og_detect_active_exe(ds)` then
   `og_runtime_to_name(cs, ip)` in `og_overlay_int3f_probe`.  Done
   (back-fill enqueue/drain pending in step 4).
6. **Annotate existing probes** -- `og_stage1_emit_trace_points` and
   `og_stage2_unit_write_probe` call `og_runtime_to_name(cs, ip)`
   to enrich their lines with `IDA SEG_NAME:OOOO = FUNC_NAME+X`.
   TODO.
7. **Sanity check** -- on a Stage 2 run, verify the trace shows
   `Sort_Research_List` for the Stage 1 trace points and a sensible
   resolved name for the `_unit` write site.

## 7. Risks & open questions

- **R1. Cross-EXE references in static data.**  MAGIC.EXE's data
  contains the literal `"WIZARDS.EXE"` (uppercase) as a sibling
  reference used to spawn the game.  A tolerant scan of DS for the
  exename literal (case-insensitive, or accepting any DS offset)
  would false-positive on this and misidentify MAGIC.EXE as WIZARDS.
  Mitigation: check the literal EXACTLY (case-sensitive, NUL
  terminator) at the IDA-known address of `__EXENAME__`, not at any
  occurrence found via scan.  This is layer B's "use the addresses"
  rule.
- **R2. Pre-CRT-init INT 3F fires.**  Borland CRT installs the INT 3F
  handler before it sets DS to the data segment.  This produces
  fires with `DS = PSP`, where the EXENAME scan cannot succeed.  G4
  back-fill is the mitigation.  Queue overflow is the failure mode;
  we cap at 16 entries based on observation (typical CRT startup
  fires <= 2 overlay calls before DS is set).
- **R3. Handler reentry.**  Can INT 3F fire while the handler is
  already running (e.g. during overlay-load disk I/O)?  Probably not
  in this codebase, but the probe is idempotent against it.
- **R4. Aliasing across overlay swaps.**  Overlay buffer reuse means
  the same runtime CS may map to different IDA linears at different
  times.  Our resolver always uses the most recent base_offset --
  which is fine because base_offset is a property of the EXE, not the
  overlay buffer.  CS:IP -> IDA linear is invariant for code in the
  resident base segment; for overlay code, it requires the overlay to
  currently be loaded at the CS in question.  Trace consumers must
  understand this is "what the resolver thinks NOW", not necessarily
  "what was in that buffer at fire time".  Acceptable for v1.
- **R6. Overlay code mapping.**  base_offset is anchored on the
  data-segment signature, which puts the EXE's resident image at a
  fixed runtime location.  Verified empirically: fire #1 with the
  signature anchor resolves to `stub058:j_Draw_Logos+0`, which is
  the expected resident stub-thunk.  For code in overlay buffers
  (loaded at runtime paragraphs separate from the resident image),
  the single base_offset is NOT correct -- those need a per-overlay
  resolver.  See [og_overlay_findings.md](og_overlay_findings.md)
  "R6 (rev 2) overlay-buffer CS resolver -- walkthrough" for the
  current implementation.

  **r6 rev 1** (initial fix): byte-pattern search assuming runtime
  CS:0 = static SEG_EA:0, with strict `memcmp` on a 32-byte window.
  Broken in two ways: (a) Borland's overlay manager prepends a
  variable-size header per overlay (0xC0 for ovr051, 0x30 for ovr063,
  etc.); runtime CS:0 actually maps to static SEG_EA + header_size.
  (b) Strict memcmp can't survive relocation fixups (`9A xx xx ss ss`
  far calls, `B8 ss ss` segment loads) applied at load time.

  **r6 rev 2** (current): tolerant byte-pattern match WITH a per-CS
  cache.
  - On first encounter of a runtime CS, snapshot 32 bytes at CS:IP
    and score every position in every `ovrXXX` segment (matching
    bytes, not strict equality).  Track best + runner-up across
    everything.  Accept best if `hits >= 26` AND
    `best - runner_up >= 4` (margin tolerates ~3 relocation slots in
    the window).
  - Once accepted, compute `header_offset = best_static_offset -
    ip_val` (the size of the overlay's header in the runtime
    buffer).  Cache `(cs -> seg_idx, header_offset, 8 verify
    bytes)`.
  - Subsequent calls with same CS: read 8 bytes at
    `cs:header_offset` (= overlay's static byte 0), compare to
    cached `verify_bytes`.  Match -> direct arithmetic
    `static_offset = ip + header_offset`, no search.  Mismatch ->
    overlay buffer was reloaded with a different overlay; evict
    and re-learn.
  - General-purpose: backs every code-location description in the
    trace (not `_unit`-specific).
  - Logs `[OG_R6_LEARN]` per first-encounter and `[OG_R6_NOMATCH]`
    per ambiguous case (with runner-up position and runtime byte
    dump) so patterns surface over time.

  **Known limitation**: when the SAME 32-byte code block appears
  twice within one overlay (e.g. an iteration loop the compiler
  inlined or duplicated), the resolver can't tell which instance
  executed.  Margin check rejects rather than guesses, so the
  caller falls through to the (wrong but flagged) single-base
  result.  Possible future fix: relocation-aware blanking of the
  fixup byte slots before comparison.
- **R5. EXE bytes on disk vs in DOSBox C: drive.**  We load
  MAGIC.EXE / WIZARDS.EXE from a host path
  (`/home/jbalcomb/STU_DBWD/MoM131/`) hardcoded in `og_exe_info`.  If
  the user has multiple versions, this could mismatch the actually-
  running EXE.  Detection by `__EXENAME__` only identifies which of
  MAGIC/WIZARDS is running, not which build.  Out of scope; mismatch
  would manifest as wrong resolved-names.

## 8. Success criteria

- **C1.** A Stage 2 run produces an `[OG_OVR] active EXE = MAGIC.EXE`
  line within the first ~10 INT 3F fires.
- **C2.** Every `[OG_OVR] fire #N` is followed by either a
  `resolved: ... FUNC_NAME+X` line in the same scan, OR a later
  `backfilled fire #N: ...` line with the same resolution.  No
  silent drops.
- **C3.** Every `[OG_UNIT] writer` and `[OG_SORT] CMP/REALM/ENTER`
  line carries the resolved `IDA SEG_NAME:OOOO = FUNC_NAME+X`
  annotation.
- **C4.** Manual cross-reference against the IDA disassembly for at
  least one resolved line confirms the resolver's seg/func/offset is
  correct (within +/- a few bytes -- IDA function-start vs
  call-target-thunk drift is acceptable).
- **C5.** No measurable slowdown vs current trace at OG main-menu
  idle.  Per-instruction cost is still dominated by the existing
  push-bp / read-byte probes.
