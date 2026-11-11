# PRD: OG Local-Variable Watches (STU-DOSBox)

**Status:** Draft (r1)
**Owner:** prng-port byte-match audit / mom-matchup
**Scope:** A repeatable pattern for watching a *local variable* inside an
OG-MoM function as it executes in STU-DOSBox — emitting the value at **every
write site** so it can be diffed against the ReMoM C equivalent. Covers locals
that live in **CPU registers** and locals that live in **BP-relative stack
slots**, and functions that are **near**, **far**, or **in an overlay**.
**Reference implementation:** `City_Gold_Production` (WZD `o145`), watching
`gold_units` (register `DI`) and `gold_modifier` (stack `[SS:BP-0x0E]`).
**Predecessor concepts:** the `[WATCH]` memory-write watcher (`WATCH_TARGETS`
in `og_sort_trace.h`) and `gd_wzd_landmark_probe`.

---

## 1. Context

The matchup compares OG MoM against ReMoM byte-for-byte. When a *saved* field
diverges (e.g. `_CITIES[].gold_units`), the global write-watch (`WATCH_TARGETS`)
tells us *when the stored byte changes* but not *why* — the value is computed
in a function from many intermediate locals. To localise the bug we need OG's
**intermediate** values (the locals inside the producing function) so we can
line them up against ReMoM's and find the first step that differs.

The global write-watch cannot do this, because a local is **not a fixed memory
address**:

- It may live entirely in a **register** (the OG `.asm` `proc` header maps it,
  e.g. `gold_units = di`). A register has no address to watch.
- It may live in a **BP-relative stack slot** (`gold_modifier = word ptr -0Eh`
  → `[SS:BP-0x0E]`). The absolute address changes every call (BP = SP at entry),
  so you cannot hard-code it in the `WATCH_TARGETS` table.

So local watches use a different mechanism: a **per-instruction shadow** that is
**gated to "we are currently executing inside the target function"**, reads the
local each instruction, and logs whenever it **changes**. Change-detection means
you do **not** need each individual write instruction's address — every real
write shows up as a change, stamped with the runtime `CS:IP` (and `og_describe`
offset) so you can map it back to the disassembly.

---

## 2. Inputs you must have first

Do **not** infer any of these — get them from the disassembly artifact (ask the
user for the function's `.asm` if you don't have it; see
[[feedback_ask_for_re_artifacts]]).

1. **The function's `.asm` with its `proc` header.** The header is the storage
   map. Example (`WZD-o145p04-City_Gold_Production.asm`):
   ```
   proc City_Gold_Production far
   gold_modifier = word ptr -0Eh     ; STACK local at [SS:BP-0x0E]
   city_idx      = si                ; REGISTER local (SI)
   gold_units    = di                ; REGISTER local (DI)
   _city_idx     = word ptr  6       ; incoming arg at [BP+6]
   ```
   For each target local, classify it:
   - `name = <reg>`            → **register local** (read the register).
   - `name = word ptr -XXh`    → **stack local** at `[SS:BP-XX]` (sign: the
     IDA `-XXh` is already the BP-negative offset; read `BP - 0xXX`).
   - `name = word ptr +XXh`    → an **incoming arg / saved frame**, `[BP+XX]`.

2. **The function's linkage + location.** From the `proc ... near|far` keyword
   and the overlay table:
   - **near / far, resident segment** (`seg0NN`, `stub0NN`): the runtime→IDA
     mapping via `base_offset` is reliable. Detect by IDA-linear range.
   - **overlay** (`ovr0NN`): `base_offset` is **not** reliable for the overlay
     buffer. Detect by **name** via `og_describe` (it resolves overlays through
     the back-patched thunk map). Get the `ovr0NN` id + entry from
     `WZD-overlay-table.fwv` (see [[reference_mgc_overlay_table]]).

3. **Which EXE.** MAGIC (`OG_SR_MGC_EXE_IDX`) or WIZARDS (`OG_SR_WZD_EXE_IDX`).
   Gameplay functions are WIZARDS (see [[project_mom_executables]]).

---

## 3. Mechanism: per-instruction shadow, gated to the function

`og_sort_trace_check()` (in `og_sort_trace.h`, called once per instruction from
`core_normal.cpp`) gives you `cs_val`, `ip_val`, `ds_val`. Your probe, called
from there, does:

```
on each instruction (gated to the target EXE):
    in_fn = (CS:IP is inside the target function?)      // §4
    if  in_fn and not was_in_fn:    ENTER -> baseline the locals, reset prev[]
    if  in_fn:                      read each local (§5); if changed -> emit (§6)
    if  was_in_fn and not in_fn:    EXIT  -> emit final value, clear state
    was_in_fn = in_fn
```

Why gated to the function: a register local (DI here) is **callee-saved** in
Borland's convention, so it survives the nested calls the function makes — but
**during** those nested calls `CS:IP` is in another function that may use DI as
scratch and reuses BP for its own frame. If you read ungated you log garbage.
Reading **only while CS:IP is in the target function** is what makes both the
register read and the `[BP-off]` read correct.

---

## 4. Detecting "we are inside the function right now"

### 4a. Overlay function (use this for `ovr0NN`) — name match

```c
char desc[160];
og_describe(cs_val, ip_val, desc, sizeof desc);     // "ovr145:City_Gold_Production+1A3"
int in_fn = (strstr(desc, "City_Gold_Production") != NULL);
```

`og_describe` handles overlays via the runtime overlay map. **Caveat:** it can
only resolve an overlay once that overlay's runtime CS has been observed (the
overlay manager's `INT 3F` load / a cross-overlay `j_` thunk records it). A
function reached *only* by intra-overlay near calls may be unresolved until the
overlay is loaded through a watched path — if `[WATCH]` lines never appear,
that's the cause; trigger off a known cross-overlay landmark first (see
[[project_og_overlay_trigger]]). The `+NNN` in `desc` is the offset from the
function entry → maps directly to the `.asm` (e.g. `+1D8` ≈ `loc_C81D8`).

### 4b. Resident near/far function — IDA-linear range

```c
struct og_exe_info* e = &og_exes[OG_SR_WZD_EXE_IDX];
Bit32s ida = (Bit32s)(((Bit32u)cs_val << 4) + ip_val) - e->base_offset;
int in_fn = (ida >= (Bit32s)FUNC_IDA_ENTRY && ida < (Bit32s)(FUNC_IDA_ENTRY + FUNC_LEN));
```

`FUNC_IDA_ENTRY` / `FUNC_LEN` come from the overlay table row + the `.asm`
extent. This is cheaper than `og_describe`; prefer it for resident code.

> Performance: `og_describe` per instruction over a whole EXE is not free. For a
> short debugging run it's fine. To speed it up, cheap-pre-gate first — for an
> overlay, compare `cs_val` against the cached runtime CS of the target overlay
> (from `og_ovr_map`) before calling `og_describe`.

---

## 5. Reading a local by storage class

```c
/* register local (DI / SI / AX / BP ...) */
Bit16u gold_units = (Bit16u)(reg_edi & 0xFFFFu);
Bit16u city_idx   = (Bit16u)(reg_esi & 0xFFFFu);   /* context for the log line */

/* stack local [SS:BP-0x0E] */
Bit16u ss_val = SegValue(ss);
Bit16u bp_val = (Bit16u)(reg_ebp & 0xFFFFu);
Bit16u gold_modifier = real_readw(ss_val, (Bit16u)(bp_val - 0x0E));
```

Notes:
- Locals are usually `int16_t`. Read as `Bit16u`; cast to `(Bit16s)` when
  printing if a value can be negative.
- A stack local is **garbage before its first write** and before the prologue
  (`push bp; mov bp,sp; sub sp,N`) sets up the frame. Change-detection absorbs
  this: the entry baseline may be garbage, and the **first real write logs as
  the first change**. Don't treat the pre-write value as meaningful — the
  `og_describe` offset on each line tells you which write it was.
- Width: use `real_readb` for a byte local, `real_readw` for word, `real_readd`
  for dword. Match the `.asm` (`byte/word ptr`).

---

## 6. Emit format

All probe output goes to **stderr** via `fprintf` (consistent with `[WATCH]`,
`[GD]`, `[OG_SEED]`). Make it greppable per-variable and stamp the site:

```c
fprintf(stderr,
    "[WATCH-LOCAL] %s  city=%d  %s=%d  at=%s (%04X:%04X)\n",
    "City_Gold_Production", (Bit16s)city_idx,
    "gold_modifier", (Bit16s)gold_modifier,
    desc, cs_val, ip_val);
```

- One line per change, per variable. Grep `[WATCH-LOCAL] ... gold_modifier=` or
  `gold_units=` to isolate one local's evolution.
- Include a **context key** (here `city_idx` from SI) so multiple calls (one per
  city) are distinguishable.
- Include `desc` (the `og_describe` string) **and** raw `CS:IP` — `desc` maps to
  the `.asm` write site; raw `CS:IP` is the fallback if `desc` can't resolve.

---

## 7. Probe template (copy, then specialise)

```c
/* ---- [WATCH-LOCAL] <FUNC> locals: <localA> (reg), <localB> ([BP-off]) ----
 * Per-instruction shadow gated to <FUNC>.  Logs each local on change.
 * Temporary debug instrumentation -- remove when the divergence is closed. */
static void wl_<func>_probe(Bit16u cs_val, Bit16u ip_val) {
    if (og_active_exe_idx != OG_SR_WZD_EXE_IDX) return;   /* §2.3 EXE gate */

    static int      last_exe   = -2;
    static int      was_in_fn  = 0;
    static Bit16u   prev_a = 0, prev_b = 0;
    if (og_active_exe_idx != last_exe) {                  /* reset on EXE change */
        last_exe = og_active_exe_idx; was_in_fn = 0; prev_a = prev_b = 0;
    }

    char desc[160];
    og_describe(cs_val, ip_val, desc, sizeof desc);
    int in_fn = (strstr(desc, "<FUNC>") != NULL);         /* §4a overlay name-match */

    if (in_fn) {
        Bit16u ctx = (Bit16u)(reg_esi & 0xFFFFu);         /* context key (city_idx=SI) */
        Bit16u a   = (Bit16u)(reg_edi & 0xFFFFu);         /* §5 register local */
        Bit16u ss  = SegValue(ss);
        Bit16u bp  = (Bit16u)(reg_ebp & 0xFFFFu);
        Bit16u b   = real_readw(ss, (Bit16u)(bp - 0x0E)); /* §5 stack local */

        if (!was_in_fn) {                                  /* ENTER: baseline */
            prev_a = a; prev_b = b;
            fprintf(stderr, "[WATCH-LOCAL] <FUNC> ENTER ctx=%d at=%s\n",
                    (Bit16s)ctx, desc);
        } else {
            if (a != prev_a) {
                fprintf(stderr, "[WATCH-LOCAL] <FUNC> ctx=%d <localA>=%d at=%s (%04X:%04X)\n",
                        (Bit16s)ctx, (Bit16s)a, desc, cs_val, ip_val);
                prev_a = a;
            }
            if (b != prev_b) {
                fprintf(stderr, "[WATCH-LOCAL] <FUNC> ctx=%d <localB>=%d at=%s (%04X:%04X)\n",
                        (Bit16s)ctx, (Bit16s)b, desc, cs_val, ip_val);
                prev_b = b;
            }
        }
        was_in_fn = 1;
    } else if (was_in_fn) {                                /* EXIT */
        fprintf(stderr, "[WATCH-LOCAL] <FUNC> EXIT  <localA>=%d <localB>=%d\n",
                (Bit16s)prev_a, (Bit16s)prev_b);
        was_in_fn = 0;
    }
}
```

Register the call inside `og_sort_trace_check()` next to the other WZD probes
(near `gd_wzd_landmark_probe(cs_val, ip_val);`):

```c
wl_<func>_probe(cs_val, ip_val);
```

---

## 8. Caveats / gotchas

- **No-op writes are invisible.** Change-detection skips a write that stores the
  same value. If you must see *every* write including no-ops, probe the exact
  write-instruction addresses instead (needs each write's IDA address from the
  `.asm`) — heavier, rarely worth it.
- **Register locals only valid in-range.** Gate to the function (§3). Don't read
  the register during the function's nested calls.
- **Stack local needs the frame.** `[BP-off]` is only meaningful after the
  prologue. The ENTER baseline may be pre-prologue garbage; that's fine — the
  first true write is the first logged change.
- **Overlay resolution depends on the overlay being loaded** (§4a). No `[WATCH]`
  lines ⇒ the overlay's runtime CS was never observed; trigger off a landmark.
- **Watch the disassembly, not the C source.** OG can do things the ReMoM C
  doesn't (the reference example: OG `add gold_units, gold_units` doubles gold
  for Dwarf cities — no ReMoM equivalent). The whole point is to see OG's actual
  behaviour, so map every logged change to the `.asm`, not to the `.c`.
- **It's temporary.** A per-instruction probe over a whole EXE is overhead.
  Remove the probe + its call once the divergence is closed.

---

## 9. How to use this (instructions to self)

When asked to "watch local `X` (and `Y`) in OG function `F`":

1. **Get the `.asm`** for `F` (ask if absent). Read its `proc` header.
2. **Classify each target local** from the header: register vs `word ptr -XXh`
   stack slot vs arg (§2.1). Note the exact register / offset / width.
3. **Classify `F`**: near/far resident vs overlay; which EXE (§2.2–2.3). Get the
   `ovr0NN` id (overlay) or IDA entry+len (resident) from the overlay table.
4. **Pick detection** (§4): overlay → `og_describe` name-match; resident →
   IDA-linear range.
5. **Copy the template** (§7); substitute `<FUNC>`, the EXE idx, each local's
   read (register vs `real_readw(ss, bp-off)`), the context key, and the emit
   lines. One emit per local, greppable, stamped with `desc` + `CS:IP`.
6. **Register the call** in `og_sort_trace_check()` by the other WZD probes.
7. **Build** STU-DOSBox; **run** the pipeline that exercises `F`; **grep**
   stderr for `[WATCH-LOCAL] ... <local>=`.
8. **Map** each logged change's `at=ovrNNN:F+OFF` back to the `.asm` write site
   to read off which step produced which value; diff against ReMoM.
9. **Remove** the probe + its call when done.

---

## 10. Reference implementation: `City_Gold_Production` (WZD o145)

Storage map (from `WZD-o145p04-City_Gold_Production.asm` `proc` header):
`gold_units = di` (register), `gold_modifier = word ptr -0Eh` (`[SS:BP-0x0E]`),
`city_idx = si` (context key). Overlay `o145`, far. EXE = WIZARDS.

Expected change sequence per non-empty city (from the `.asm`, in order):

| local           | site (og_describe maps here)                    |
|-----------------|-------------------------------------------------|
| `gold_modifier` | base = `Map_Square_Gold_Bonus + City_Road_Trade_Bonus` |
| `gold_modifier` | `+50` Nomad race                                |
| `gold_modifier` | cap to `population*3`                            |
| `gold_modifier` | `+100` Merchants Guild                           |
| `gold_modifier` | `+50` Bank                                       |
| `gold_modifier` | `+50` Marketplace                                |
| `gold_modifier` | `+100` Prosperity                                |
| `gold_units`    | neutral `pop−rebels` / else `(pop−rebels)*tax/2` |
| `gold_units`    | **`*2` Dwarf race** (no ReMoM equivalent)        |
| `gold_units`    | `+= Square_Gold_Income` (once per useable square)|
| `gold_units`    | `*(gold_modifier+100)/100`                       |
| `gold_units`    | `+= production_units/2` (TradeGoods)             |
| `gold_units`    | cap at 255                                        |

The final `gold_units` at EXIT is what's stored to `_CITIES[city_idx].gold_units`
(via `City_Gold_Production`'s caller). Diff the OG `[WATCH-LOCAL]` sequence
against ReMoM's `City_Gold_Production` to find the first diverging step.

---

## 11. Primitive reference (grep `og_sort_trace.h` — names, not line numbers)

| need                          | primitive                                         |
|-------------------------------|---------------------------------------------------|
| per-instruction entry         | `og_sort_trace_check()` (called from `core_normal.cpp`) |
| current CS / IP / DS          | `SegValue(cs)`, `reg_eip & 0xFFFF`, `SegValue(ds)`|
| active EXE gate               | `og_active_exe_idx`, `OG_SR_WZD_EXE_IDX` / `OG_SR_MGC_EXE_IDX` |
| overlay name+offset of CS:IP  | `og_describe(cs, ip, buf, len)`                   |
| runtime→IDA (resident)        | `((cs<<4)+ip) - og_exes[idx].base_offset`         |
| overlay runtime-CS map        | `og_ovr_map`, `og_seg_table`, `og_func_table`     |
| read register                 | `(Bit16u)(reg_eXX & 0xFFFFu)`                      |
| read stack/seg word           | `real_readw(SegValue(ss), bp - off)` (`real_readb/d` for 1/4) |
| one-shot / per-call state     | `static` vars + reset on `og_active_exe_idx` change (see `gd_wzd_landmark_probe`) |
| existing change-watcher       | `watch_targets_check`, `struct watch_target`, `WATCH_TARGETS` |
| existing landmark probe       | `gd_wzd_landmark_probe`, `struct gd_landmark_t`   |
