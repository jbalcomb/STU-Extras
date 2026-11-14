# Notes — Borland Overlay Trap, Empirical Findings

Working document: things we've **verified** vs things we **think** vs
**open questions**.  Updated as the trace probe surfaces new data.

Companion to [og_overlay_probe.PRD.md](og_overlay_probe.PRD.md).  Edit
freely as understanding evolves.

---

## Verified

These claims are backed by either trace output or direct IDA inspection.

| # | Claim | Source |
|---|-------|--------|
| V1 | DOS's Set Interrupt Vector (INT 21h AH=25h) is called once during CRT startup with `AL=0x3F`. Logged handler address: `2259:04F6` (one specific session). | `[OG_OVR] INT 21/25 install` log line from dos.cpp probe. |
| V2 | The byte sequence `CD 3F` exists at only **one** symbol in the disassembly: `__OVRTRAP__` in the `_STUB_` segment, at IDA address `3156:0000`. | IDA grep (jbalcomb). |
| V3 | The runtime `__OVRTRAP__` is not a single instruction but **a table of 5-byte records**, each starting with `CD 3F`, repeating at stride 5. Fire #1's `+16` window shows `>CD 3F< 00 00 00 CD 3F 9F 03 00 CD 3F 0F 04 00 CD 3F D7`. | `[OG_OVR] code at ...` window from per-fire dump. |
| V4 | Each record's 3 bytes of metadata after `CD 3F` differ per record. Pattern: `CD 3F <byte> <byte> <byte>`. | Same dump as V3. |
| V5 | The 16 bytes immediately preceding each `CD 3F` we've caught share a common pattern: `C8 04 00 00 00 00 00 00 00 00 00 00 00 00 CD 3F`. The `C8 04` looks like a length / sizeof field (1224 dec = sizeof s_WIZARD, coincidence?), and is the same in both fires. | Per-fire dumps fire #0 + fire #1. |
| V6 | The word at offset −14 from each `CD 3F` differs between fires. Fire #0: `66 23` (=`0x2366`). Fire #1: `6A 23` (=`0x236A`). | Per-fire dumps. |
| V7 | Fire #0 happens while `DS=SS=0x0192` and `DS:SI` points at `CD 20 FF 9F ...`. `CD 20` is `INT 20h` (DOS Program Terminate) — i.e. PSP[0]. So Fire #0 is **before the CRT has switched DS away from the PSP**. | DS:SI dump in fire #0. |
| V8 | Fire #1 has `DS:SI` pointing at `....Borland C++ ` ASCII text. This is the Borland CRT identifier stamp in the program's data segment. So Fire #1 is **after CRT init completed**. | DS:SI dump in fire #1. |
| V9 | Fire #1's stack contains the ASCII bytes `4153 4556 2E38 4147` = "SAVE8.GA" (low-byte-first per word). | Stack dump in fire #1. |
| V10 | Fire #1 stack content "SAVE8.GA" does **not** exist as a literal in the disassembly — it's runtime-constructed. | jbalcomb grep of disassembly. |
| V11 | Fire #0's AX = `0x17D4` (= 6100). main() in IDA contains exactly one occurrence of bytes `B8 D4 17` (= `MOV AX, 17D4h`), which is followed by a `CALL FAR j_Allocate_Data_Space`. | jbalcomb in IDA. |
| V12 | The `__OVRTRAP__` region is a **two-level structure**, not a flat 5-byte-stride table.  The 128-byte window around each fire shows recurring "groups" at 32-byte stride.  Each group has shape:  `[16-byte header] [CD 3F + metadata + padding] ...`  where the 16-byte header has the pattern `00 00 <SS HH> 00 00 00 00 C8 04 00 00 00 00 00 00`  (HHSS is a little-endian word, varies per group). | Wider code window in fires #0 and #1. |
| V13 | Each header's `C8 04` word reads as **0x04C8 = 1224 decimal** -- same value in every group.  Plausible candidates: per-overlay block size, or a Borland-specific magic. | Same dumps. |
| V14 | Inside each group there are **two distinct stub flavours**:  **Type A** = `CD 3F + 3 bytes` (5-byte stride, packed in dense runs)  **Type B** = `CD 3F + 14 bytes` (16-byte stride, header-preceded, more metadata). | Fire #1 dump: Type A run at offsets 0..29, Type B entry at +32. |
| V15 | The header-word values seen so far -- `0x2366`, `0x236A`, `0x236D` -- are small distances apart (3-7 segments) and close to the runtime CS of the install handler (`0x2259`).  They look like neighbouring runtime segment numbers, not random words. | Headers from Fire #0 / Fire #1 wider windows. |
| V16 | The header byte signature `00 00 ?? ?? 00 00 00 00 C8 04 00 00 00 00 00 00` (16 bytes, ?? = any) does NOT appear in either MAGIC.EXE or WIZARDS.EXE statically.  Zero matches in both files. | `/tmp/find_ovrtrap_pattern.py`. |
| V17 | None of the header-word values `0x2366`, `0x236A`, `0x236D` correspond to any overlay's `SEG_EA` in the overlay function table. | overlay-table.fwv grep. |
| V18 | Combined with V2 ("CD 3F appears once in the disassembly, as `__OVRTRAP__`") and V16: `__OVRTRAP__` is a **2-byte data template constant** (`0x3FCD`), not a function or single instruction.  The runtime stub table is **constructed by the Borland CRT during program startup**, by stamping copies of this template into a runtime-allocated memory region, then filling in per-stub metadata around it.  Nothing about the table layout itself is present in the EXE file -- only the template constant and the construction code are. | Synthesis of V2, V3, V12, V16, V17. |

## Hypothesized

Plausible reasoning from the evidence, but not yet confirmed.

| # | Hypothesis | Why we think so | What would confirm |
|---|------------|----------------|--------------------|
| H1 | The `__OVRTRAP__` table is the **stub thunk table**: every `j_FooFunction` in the binary is one record in this table, indexed by offset from `__OVRTRAP__`. | V3 / V4 / V5: many `CD 3F` slots, regular stride, structurally homogeneous header. | IDA: look at `__OVRTRAP__` as a label and check whether `j_*` symbols sit at +0, +5, +10, +15, ... inside it. |
| H2 | The 3-byte metadata after each `CD 3F` encodes which overlay function to dispatch (overlay index + offset, or a single packed value). | V4. The trap handler must know which target to load; AX is being used by the caller for function args (V11), so it isn't carrying the index. | IDA: inspect the trap handler at `2259:04F6` (whatever IDA addr maps to it) and see how it reads `[SS:SP+0]` to find the post-`CD 3F` bytes. |
| H3 | ~~The word at offset −14 (`0x2366` / `0x236A`) is the overlay function's **patched target segment** — a JMP-FAR-style destination that the manager rewrites once the overlay is loaded.~~ **FALSIFIED by V17.** Those words don't match any overlay's `SEG_EA`. They're some other kind of runtime pointer. | V6: differs per fire site, matches the magnitude of a segment value, sits a fixed distance from the trap. | IDA: see whether the stub layout shows `JMP FAR <off>:<seg>` with that exact seg word at offset −14. |
| H4 | Fire #0 is **the CRT-time `j_Allocate_Data_Space(6100)` call** out of `main()` that jbalcomb identified. | V11 (AX matches) + V7 (early-CRT context — `main()` is called early). | If we add a code-bytes snapshot to the trace for the call site that issued the `CALL FAR`, the bytes should match the surroundings of `B8 D4 17` in `main()`. |
| H5 | Fire #1 is a file-open of `SAVE8.GAM`. AX=0 fits "mode = read-only" if a Borland `open()` thunk is being dispatched. | V8 + V9: well-formed runtime program state, filename on stack, AX=0. | IDA: find the `j_open` (or equivalent) stub; verify call sites that pass a filename pointer plus AX=0. |

## Open questions

| # | Question |
|---|----------|
| Q1 | How do we recover the stub's index within the `__OVRTRAP__` table from runtime CS:IP? (Maps "this fire" → "this `j_*` symbol".) Probably IDA-side: once we know `__OVRTRAP__`'s runtime base, the stub index is `(runtime_ip - base) / 5`. |
| Q2 | What is `0x04C8` / `0x04CC` recurring at offset −8 from `CD 3F`? Could be a count, a buffer length, a stride. Same value in both fires suggests a constant — maybe size of the per-stub control block. |
| Q3 | Are there far-call return addresses on the stack we could resolve back to the **real game-code caller** (`main()` for fire #0, whoever-builds-"SAVE8.GA" for fire #1)? |

## WIZARDS.EXE deterministic startup execution

These are events that **always happen** on every direct launch of
`WIZARDS.EXE`, with no user input required.  Documented here so
"expected vs observed in the trace" is a checklist, not a re-derived
inference each time.

### Auto-loaded save

WIZARDS.EXE opens `SAVE9.GAM` during its own startup, before any
menu is presented.  Visible in DOSBox's own logging as:

```
FILES:file open command 0 file SAVE9.GAM
```

### Save-loaded values land in memory via DOS, BEFORE any game code runs

OG-MoM's save-load uses Borland's `fread` (specifically `gfopen`,
`gfread`, and `gfwrite` in `seg011`), which ultimately invokes INT
21h to do disk I/O.  Each saved global is read directly into its
in-memory location, byte-for-byte.  Per ReMoM analog
[LOADSAVE.c:82](/home/jbalcomb/STU/devel/ReMoM/MoX/src/LOADSAVE.c#L82):
`stu_fwrite(&_unit, 1, 2, file_pointer);` -- one of many such per-global
read/writes.

**Consequence for our trace probes**: by the time WIZARDS' own C code
starts running the load-game update path, `_unit` has already been
populated from `SAVE9.GAM` (e.g., the canonical save stores `_unit = 5`,
so post-load the in-memory value is `5`).  Per-instruction memory-write
probes see this change but attribute it to `CS=F000:xxxx` -- DOSBox's
fake CS for ISRs (interrupt-service-routine callbacks; native C
handlers for BIOS/DOS interrupts).  These are REAL writes from the
ISR's perspective and must not be filtered out; the probe labels
them with `ISR` in the log.

#### Verified call chain for `_unit`'s first write (canonical SAVE9.GAM)

```
seg001:_main+2DB
  -> stub050:j_Load_SAVE_GAM+0           ; far CALL to stub thunk
     -> INT 3Fh                          ; Borland overlay manager
                                         ;   loads ovr050 into a buffer
        -> ovr050:Load_SAVE_GAM
           -> +44 strcat (filename build), +7B gfopen, +95..+1BA = 13
              prior gfreads (other globals)
           -> +1D2  (runtime IP 0x05BA)  ; <-- THE _unit gfread
                                         ;     (14th gfread overall in
                                         ;     Load_SAVE_GAM)
              -> seg011:gfread+0
                 -> seg011:gfread+2A
                    -> INT 21h           ; DOS file-read service writes
                                         ;   2 bytes from SAVE9.GAM
                                         ;   directly into
                                         ;   WZD DGROUP:0xBD8E
                       ===> _unit = 5    ; (or whatever the save holds)
```

Stable identifiers (use these when reading new traces, since runtime
CS values are session-dependent):
- `ovr050:Load_SAVE_GAM+1D2` -- the call-site offset within the function
- 14th gfread call within Load_SAVE_GAM -- the ordinal position
- `seg011:gfread+2A` -- the INT 21h site within gfread

In our trace this single DOS-write event surfaces twice:
1. In `/tmp/wizards_exe-calls.log` (the call/ISR reference log) as
   a CALLF (Load_SAVE_GAM+1D2 -> gfread) immediately followed by an
   INT (gfread+2A -> INT 21h handler).
2. In the main trace log as
   `[OG_UNIT] writer cs=F000 ip=14C1 ISR _unit 0 -> 5 (change #1)`
   -- captured by the per-instruction memory-watch probe, with
   `cs=F000:14C1` being DOSBox's fake CS for the INT 21h native
   callback.

### Load-game-update init call chain

After the save is read into memory by DOS, the program enters the
load-game-update init path.  This path executes the following (per
jbalcomb), every time, no input required:

```
Init_Overland                     (ovr051)
  -> WIZ_NextIdleStack            (ovr061)
       -> Next_Unit_Nearest_Available  (ovr061)
            -> writes _unit at WIZARDS DGROUP offset 0xBD8E
```

`_unit`'s value at WZD detection time depends on whether the save
has been read yet:
- Before save load: `0` (BSS-initialized).
- After save load (via DOS): `5` (or whatever the save file holds).

Init_Overland always writes `_unit = 0` unconditionally at offset
`+0x1CD` -- visible in the trace as `5 -> 0` because the pre-existing
`5` came from the DOS save load (above), not from any game C code.

### Expected trace signatures (in order)

Given the above, a healthy WIZARDS.EXE direct-launch trace should
contain at minimum these lines, in roughly this order:

1. `[OG_OVR] Borland DGROUP detected: signature at DS:0000`
2. `[OG_OVR] active EXE = WIZARDS.EXE  __EXENAME__("wizards.exe") at runtime SSSS:OOOO`
3. `[OG_OVR] base_offset = ... = ...`
4. INT 3F fires for the resident-image stubs that get invoked during
   startup -- `[OG_OVR] fire #N ... resolved: ... stubXXX:j_FuncName+0`
   lines.  The deterministic ones include resource loaders
   (`j_Load_MAGIC_SET`, `j_Load_SAVE_GAM`, `j_Load_WZD_Resources`,
   `j_Reset_City_Area_Bitfields`, ...) and -- crucially for our R6
   work -- the stubs in the call chain above (`j_Init_Overland`,
   `j_WIZ_NextIdleStack`, `j_Next_Unit_Nearest_Available`).
5. `[OG_UNIT] initial value=0  (at ds:SSSS:BD8E)` -- WZD-active probe arms.
6. `[OG_UNIT] writer cs=CCCC ip=010D (...)  _unit  N -> 0` --
   Init_Overland's tail writes `_unit` to 0.  The `cs` here is
   the runtime CS of the **ovr051 buffer** (dynamic per session, but
   always present in the trace).
7. `[OG_UNIT] writer cs=CCCC ip=0B1A (...)  _unit  0 -> 5` --
   Next_Unit_Nearest_Available writes `_unit` (Closest_Waiting_Unit
   variant).
8. Possibly `[OG_UNIT_TEST] cs=... ip=...  ...PASS|FAIL` if the
   resolver's resolution matches the user-confirmed reference.

If the trace is missing the `[OG_UNIT] writer ...` lines entirely,
the most likely cause is **the emulator didn't get far enough in
wall-clock time to execute the load-game-update init path**, not
"the path didn't run."  Bump the timeout, reduce per-fire log
overhead, or check the DOSBox `FILES:file open command` log for
`SAVE9.GAM` to see how far startup actually progressed.

### What this is NOT

- This describes WIZARDS.EXE launched **directly** (e.g.
  [r6-test.conf](/home/jbalcomb/STU/devel/STU-Extras/STU-DOSBox/r6-test.conf)).
  It is NOT the Stage 2 scenario, which starts with MAGIC.EXE and
  chains to WIZARDS.EXE -- that adds an `[OG_OVR] active EXE CHANGED:
  MAGIC.EXE -> WIZARDS.EXE` event before everything above.
- These are the events that always happen; an interactive run also
  produces many more writes to `_unit` once the AI turn begins, but
  those are NOT in this section.

## Trace log lines -- inventory & interpretation

Each row documents one line family emitted by [og_sort_trace.h](og_sort_trace.h)
plus the dos.cpp INT 21/25 probe.  All lines are stderr-only; the
test launchers redirect them to `/tmp/og_*_trace.log` (or
`/tmp/r6-overlay-resolver-test.log`).

### Startup / setup

| Line prefix | Format | Interpretation | Purpose |
|-------------|--------|----------------|---------|
| `[OG_OVR] loaded NAME` | `loaded EXE_NAME (N bytes), MZ header=0xN` | Static EXE bytes have been mmapped + MZ header parsed | One-shot per EXE; confirms `og_exe_load` succeeded |
| `[OG_OVR] loaded N segs, M funcs from PATH` | One-shot per `.fwv` | IDA overlay table parsed | Confirms `og_exe_load_ovrtab` succeeded |
| `[OG_OVR] INT 21/25 install INT 3F vector = SS:OO` | One per INT 21h AH=25h AL=3Fh | Borland CRT installing its overlay-manager handler | Confirms we're on a Borland-overlay program AND records the handler address (we could later disassemble the handler at `SS:OO` to read manager internals) |
| `[OG_OVR] Borland DGROUP detected: signature at DS:OOOO` | Per first-time-bind | Layer A of EXE identification fired (toolchain-level) | Generic Borland anchor; gates everything downstream |
| `[OG_OVR] active EXE = NAME  __EXENAME__("name") at runtime SS:OO` | Per first-time-bind | Layer B identified one of our project EXEs | Project-specific binding; sets `og_active_exe_idx` |
| `[OG_OVR] base_offset = RRRRRRRR - IIIIIIII = +/-N` | Per first-time-bind | Resident-image runtime->IDA delta computed | Used by `og_runtime_to_name` and `og_describe` for all resident-image CS:IP |
| `[OG_OVR] active EXE CHANGED: A -> B (chained process)` | Per cross-EXE chain (MAGIC -> WIZARDS or vice versa) | Process replaced itself via DOS EXEC; previous binding discarded | Captures the MAGIC.EXE -> WIZARDS.EXE transition during Stage 2 |

### Per INT 3F fire

The CD 3F instruction at runtime CS:IP triggered our `og_overlay_int3f_probe`.
Each fire emits 5-6 lines (in order):

| Line prefix | Format | Interpretation | Purpose |
|-------------|--------|----------------|---------|
| `[OG_OVR] INT 3F site cs=CC ip=II -- new fire site` | One-shot per unique (cs,ip) | First time we've seen INT 3F execute at this specific call site | Diagnostic / dedup marker |
| `[OG_OVR] fire #N cs=CC:II ax=AA bx=BB cx=CC dx=DD di=DD si=SS bp=BB ds=DD es=EE ss:sp=SS:PP` | One per fire | Full register snapshot at the CD 3F instruction | Raw state for reasoning about how the call was set up by the caller |
| `[OG_OVR]   resolved: WIZARDS.EXE  IDA seg:OOOO = func+X` | One per fire | Runs `og_runtime_to_name(cs, ip)`: resident-image arithmetic resolution.  For INT 3F fires `cs` is always a resident `stubXXX` segment, so this resolves correctly without R6.  The function name printed is the `j_` thunk (the dispatcher), not the actual overlay function | Identifies WHICH overlay function is about to be dispatched (by name of its thunk) |
| `[OG_OVR_AX] fire #N cs=CC:II AX=AA -> EXE seg_name (SEG_EA=...)` or `<no seg_idx match>` | One per fire | Looks up `AX` against the EXE's seg_idx column in the `.fwv`.  Empirically AX is NOT the overlay seg_idx -- only coincidental low-value matches succeed | Originally hypothesized AX=seg_idx; the data falsifies it.  Kept for now as a documented dead end |
| `[OG_OVR_STUB] fire #N CC:II (+0..+31): 32 hex bytes` | One per fire | Raw 32 bytes starting at the CD 3F itself.  When the fire landed at `stubXXX:0000`, these ARE the 32-byte stub descriptor.  When it landed at a per-function thunk (e.g. `stubXXX:0020`), these are 5-byte jump-table records | Verify-against-disassembly window |
| `[OG_OVR_STUB] fire #N CC:0000 (segment descr): 32 hex bytes` | One per fire | Raw 32 bytes at the START of the segment (cs:0000), which IS the stub descriptor unconditionally | Always shows the descriptor regardless of where in the segment the fire landed |
| `[OG_OVR_STUB] fire #N parsed (from cs=CC:0000): int_code=... fileoff=... codesize=... ...` | One per fire | 32 bytes at the start of the stub segment decoded per the jbalcomb RE-notes layout (int_code, memswap, fileoff, codesize, relsize, nentries, prevstub, workarea[16]) | Reads each fixed-meaning field; the trailing 16-byte workarea is dumped raw on the next line for separate analysis |
| `[OG_OVR_STUB] fire #N workarea: 16 hex bytes` | One per fire | The 16-byte workarea field at descriptor offset 16..31, raw.  Contents are overlay-manager-private; meaning is not yet documented for BC++ 3.0 | Pattern-spotting against TP6's loose analog (Bucknall .EXE Magazine 1992), but TP6 is a related-not-identical system, so workarea byte interpretations stay observational until we either pin them down from disassembling the BC++ INT 3F handler or get authoritative BC++ docs |

### EXE identification / R6 resolver

| Line prefix | Format | Interpretation | Purpose |
|-------------|--------|----------------|---------|
| `[OG_R6_LEARN] cs=CC -> ovrXXX header_offset=HHHH (...)` | First encounter of an overlay-buffer CS | R6 rev 2 byte-pattern resolver learned this CS→overlay mapping.  Pending replacement by a workarea-field-based mapping once we identify which workarea byte(s) hold the loaded runtime CS for BC++ 3.0 | Diagnostic for the byte-pattern resolver phase |
| `[OG_R6_LEARN]   runtime cs:ip: 32 bytes` | After LEARN | The runtime bytes the matcher saw | Verifiable against the EXE |
| `[OG_R6_LEARN]   static  ovrXXX@OOOO (file 0xN): 32 bytes` | After LEARN | The static EXE bytes the matcher chose | Mismatched positions printed in `(parens)` |
| `[OG_R6_LEARN]   runner  ovrYYY@OOOO (file 0xN): 32 bytes` | After LEARN | The runner-up candidate's static bytes | For sanity-checking ambiguity |
| `[OG_R6_NOMATCH] cs=CC ip=II best=N/32 runner=M/32 margin=K` | When no clear winner | Resolver rejected ambiguous match | Refuses to guess; caller falls through to single-base (wrong but visible) |
| `[OG_R6_NOMATCH]   runtime bytes: 32 bytes` | After NOMATCH | Raw bytes that couldn't be uniquely placed | Lets us decide if it's a relocation issue, a duplicate code block, etc. |

### `_unit` Stage 2 probe

| Line prefix | Format | Interpretation | Purpose |
|-------------|--------|----------------|---------|
| `[OG_UNIT] initial value=N (at ds:DDDD:OOOO)` | One-shot at probe start | First read of `_unit` after WZD detection bound | Establishes baseline value |
| `[OG_UNIT_DBG] arm-time memory DDDD:OOOO..+32: ...` | One-shot at probe start | 32 bytes around `_unit` so we can verify what's actually at that address vs what we expect | Sanity check on the polled address |
| `[OG_UNIT_DBG] fire #N snapshot:  _unit = N` | Per INT 3F fire while WZD active | `_unit`'s value at each INT 3F call site | Timeline of the value across known anchor points, independent of the per-instruction change detector |
| `[OG_UNIT] writer cs=CC ip=II (NAME)  _unit  X -> Y  (change #N)` | Per `_unit` change attributed to guest x86 code | The previous instruction wrote `_unit`; CS:IP attributed via DOSBox's per-instruction trace.  `NAME` is the resolved overlay:function+offset | Main Stage 2 evidence -- guest-code writers, with `change #N` indexing into the timeline |
| `[OG_UNIT] writer cs=F000 ip=II  ISR  _unit  X -> Y  (change #N)` | Per `_unit` change attributed to a DOSBox native callback (INT handler) | The change happened during a BIOS/DOS service running as a native C callback (CS=F000+ is DOSBox's fake CS for these).  Often INT 21h file I/O loading the value from disk -- see [[project_save_load_dos_io]] | Real behavior, not filtered: these are LEGITIMATE writes from the ISR's perspective and must not be hidden |
| `[OG_UNIT_TEST] cs=CC ip=II  expected=SEG:FUNC  got=...  PASS\|FAIL` | Per guest-code writer matching a reference in `OG_UNIT_WRITER_REFS[]` (ISR writers are not asserted) | The resolver's output vs the user-confirmed reference | Regression assertion |


## R6 (rev 2) overlay-buffer CS resolver -- walkthrough

The resolver lives in [og_sort_trace.h](og_sort_trace.h) as
`og_overlay_seg_for_cs`, called from `og_describe` whenever
single-base resolution lands in an `ovrXXX` segment.  The walkthrough
below traces the W1 case (`cs=3FD1 ip=010D`, the `_unit = 0` write
during `Init_Overland`) byte by byte.

### Inputs

- Runtime register state at the write moment:
  `cs=3FD1`, `ip=010D`.
- Static EXE bytes loaded once into `og_exes[WZD].data` from
  [/home/jbalcomb/STU_DBWD/MoM131/WIZARDS.EXE](file:///home/jbalcomb/STU_DBWD/MoM131/WIZARDS.EXE).
- Segment table loaded from
  [WZD-overlay-table.fwv](/home/jbalcomb/STU/devel/STU-Extras/STU-DOSBox/WZD-overlay-table.fwv).
- Per-EXE `header_size = 0x2A00` (parsed from MZ header).
- `image_base = 0x10000`.

### Step 1 -- cache lookup (miss on first call)

```
for c in og_ovr_cs_cache:
    if c.runtime_cs == 0x3FD1:  # not present yet
        verify ...
```

No entry for `cs=0x3FD1` -> cache miss, fall through to learn.

### Step 2 -- snapshot 32 runtime bytes at `cs:ip`

```c
for i in 0..31:
    win[i] = real_readb(0x3FD1, 0x010D + i)
```

For W1, those 32 bytes turn out to be:
```
C7 06 8E BD 00 00 C4 1E C8 9C 26 8A 07 98 A3 84 BD C4 1E C8 9C 26 8A 47 01 98 A3 82 BD C7 06 84
```

Disassembled, that's:
```
C7 06 8E BD 00 00       mov word ptr [BD8E], 0       ; _unit = 0   <-- THIS is the write
C4 1E C8 9C             les bx, [9CC8]               ; load far ptr from data
26 8A 07                mov al, es:[bx]
98                      cbw
A3 84 BD                mov [BD84], ax
C4 1E C8 9C             les bx, [9CC8]
26 8A 47 01             mov al, es:[bx+1]
98                      cbw
A3 82 BD                mov [BD82], ax
C7 06 84 ...            mov word ptr [BD84], ...
```

Note these 32 bytes contain no relocation slots (no `9A xx xx ss ss`
far calls, no `B8 ss ss` segment loads).  The `les bx, [9CC8]` uses a
near (DS-relative) address `0x9CC8`, NOT a segment value, so the byte
sequence is identical in static EXE and runtime memory.

### Step 3 -- score every position in every `ovrXXX` segment

For each segment whose name starts with `ovr`:

```c
seg_start = e->segs[k].file_offset      # SEG_EA - 0x10000 + 0x2A00
seg_end   = seg_start + e->segs[k].size  # next-larger seg_ea - this seg_ea
limit     = seg_end - 32

for fo in seg_start..limit:
    hits = count_matching_bytes(win, e->data + fo)   # 0..32
    track best (hits, seg_idx, offset_within_seg)
    track runner-up (hits, seg_idx, offset_within_seg)
```

For W1 the inner loop runs over 115 `ovrXXX` segments
(`ovr050..ovr164`).  Inside ovr051 (file_offset `0x38760`, size
`0x260` per the .fwv), at static file offset `0x38760 + 0x1CD =
0x3892D`, the 32 bytes are:
```
C7 06 8E BD 00 00 C4 1E C8 9C 26 8A 07 98 A3 84 BD C4 1E C8 9C 26 8A 47 01 98 A3 82 BD C7 06 84
```
-- exact match (hits = 32/32).  Best updates to
`(32, seg_table_idx=ovr051, off=0x1CD)`.

Continuing through other overlays, the closest competitor is in
ovr083 with only 12 matching bytes -- generic code patterns
(probably the `C7 06` prefix appearing in random places).

### Step 4 -- accept the win

```
best_hits = 32, second_hits = 12
margin = 32 - 12 = 20
32 >= OG_OVR_MATCH_MIN_HITS (26) AND margin >= OG_OVR_MATCH_MARGIN (4)  -> accept
```

### Step 5 -- compute header_offset and cache

```c
header_offset = best_off - ip_val = 0x01CD - 0x010D = 0x00C0
```

That `0xC0` is the size of ovr051's overlay header in the static
EXE (a 16-byte shared epilogue + a function-offset table).  At
runtime, the overlay manager stripped that header when loading the
buffer.  So runtime `cs:0` corresponds to static `ovr051+0xC0`.

The cache stores:
- `runtime_cs = 0x3FD1`
- `seg_table_idx = (ovr051's index in og_exes[WZD].segs[])`
- `header_offset = 0x00C0`
- `verify_bytes[8]` = 8 bytes read at runtime `cs:header_offset` = first 8 bytes of ovr051's static content past the header.

### Step 6 -- log line emitted to stderr

```
[OG_R6_LEARN] cs=3FD1 -> ovr051  header_offset=00C0  (cs:ip=3FD1:010D at ovr051@01CD  hits=32/32  runner=12/32 ovr083@04D3)
```

### Step 7 -- the resolver returns `(seg_table_idx=ovr051, static_offset=0x1CD)`

`og_describe` then walks the function table for ovr051's `seg_idx`
(0x174), finds the function with largest `ip_offset <= 0x1CD`:
- `Init_Overland`        `ip_offset = 0x000`  (the 1st function, starts at SEG_EA + 0)
- `PreInit_Overland`     `ip_offset = 0x1D2`  (too far, > 0x1CD)

Pick `Init_Overland` at offset 0.  Report:
- `seg_name = "ovr051"`
- `func_name = "Init_Overland"`
- offset within function = `0x1CD - 0 = 0x1CD`

### Final line in the log

```
[OG_UNIT] writer cs=3FD1 ip=010D (ovr051:Init_Overland+1CD)  _unit  5 -> 0
[OG_UNIT_TEST] cs=3FD1 ip=010D  expected=ovr051:Init_Overland  got=ovr051:Init_Overland+1CD  PASS
```

### Step 8 -- second call (cache HIT)

If the same `cs=3FD1` shows up again later (e.g. with a different
`ip`), the lookup goes through the fast path:

```
cache entry for cs=0x3FD1 found:
    verify: read 8 bytes at cs:header_offset=cs:00C0
    compare to verify_bytes[8] saved at learn
    if equal:
        static_offset = ip + 0x00C0
        return (seg_table_idx=ovr051, static_offset)
    else:  # buffer reloaded with a different overlay
        invalidate cache entry, fall back to step 2
```

No EXE search.  Just one `real_readb`-loop of 8 bytes and
arithmetic.

### When the algorithm fails: W3 case

```
[OG_R6_NOMATCH] cs=4681 ip=0B1A  best=32/32 (ovr061@185A)  runner=29/32 (ovr061@17C8)  margin=3
[OG_R6_NOMATCH]   runtime bytes: A3 8E BD A1 8E BD B1 05 D3 E0 C4 1E C2 9E 03 D8 26 8A 07 98 A3 84 BD A1 8E BD B1 05 D3 E0 C4 1E
```

The best position (ovr061@185A) and runner-up (ovr061@17C8) are
BOTH in the same overlay, both inside `STK_DoMeldWithNode`
(0x1767..0x195D per .fwv), only 0x92 bytes apart.  The 32-byte
window matches two near-identical code blocks within one function
(probably a unit-iteration loop that compiles twice).  Margin is 3,
below the threshold of 4, so the match is REJECTED rather than
guessed.  The resolver falls through, `og_describe` uses
single-base resolution and reports a wrong name.  Better than
inventing a confident wrong answer.

---

## Stage 2 `_unit` writer references (WIZARDS.EXE only)

User-confirmed truth set for the four runtime `(cs, ip)` pairs observed
in the Stage 2 canonical-save9 -> SAVE2 baseline trace.  The resolver
currently mis-resolves these because it applies the resident-image
`base_offset` to overlay-buffer CS values (PRD R6); these entries
exist so we can assert PASS/FAIL inline and notice the day the
resolver actually gets them right.

**Important caveats:**
- These addresses apply to **WIZARDS.EXE only**.  MAGIC.EXE has no
  business resolving these references.
- Runtime `cs_val` is the overlay-buffer paragraph in *this baseline
  session* (`launch-stage2.sh` against `canonical-save9.GAM`).  Borland's
  overlay manager can load the same overlay into different buffer
  paragraphs across sessions; the `ip_val` (IP-within-overlay) is the
  invariant.
- IDA `paragraph:offset` references below are user-supplied IDA view
  coordinates; they may not equal `SEG_EA + ip_off` for that overlay
  (IDA displays the same linear with different seg bases depending on
  the chosen image base).

| # | runtime cs:ip | _unit edge | overlay | function | IDA addr (per user) |
|---|---------------|------------|---------|----------|---------------------|
| W1 | `3FD1:010D`   | `5 -> 0`   | ovr051  | `Init_Overland`              | `4506:010D` |
| W2 | `384E:0266`   | `0 -> 5`   | ovr062  | `Sort_Unit_Stack`            | `54E2:0266` |
| W3 | `4681:0B1A`   | `5 -> 0`   | ovr061  | `Next_Unit_Nearest_Available` (Closest_Waiting_Unit variant) | (per WZD .fwv: FUNC_EA `0x53A2D`, offset `0xB1A` within overlay) |
| W4 | `45A8:0266`   | `0 -> 5`   | ovr062  | `Sort_Unit_Stack`            | (same function as W2; different runtime buffer) |

Companion (not in current trace, but documented for posterity):
- ovr061 contains a **second** `Next_Unit_Nearest_Available` variant at IDA `5315:0A83`
  (linear `0x53BD3`, offset `0xA83` within ovr061).  Difference:
  Closest_Active_Unit vs Closest_Waiting_Unit.

## Next steps for IDA-matching

1. **Search `__OVRTRAP__` in IDA**: confirm V3/H1 by checking whether `j_*` thunks are labeled at offsets +0, +5, +10, +15, ... inside it.
2. **Match Fire #0 site exactly**: the byte fingerprint `C8 04 00 00 00 00 00 00 00 00 00 00 00 00 CD 3F 00 00 00 00 00 00 00 00 00 00 00 00 00 00 CD 3F` should appear once in the disassembly; whatever stub that lands in is what Fire #0 dispatches to.  Cross-check H4 by confirming the disassembly there says it dispatches to `j_Allocate_Data_Space`.
3. **Match Fire #1 site**: same drill, fingerprint is the after-Fire-#0 pattern: `CD 3F 00 00 00 CD 3F 9F 03 00 CD 3F 0F 04 00 CD 3F D7`. Whatever stub that is, label it.
4. **Identify what `0x2366` / `0x236A` are**: are they IDA segment values?  If so, list those segments' names from the overlay function table — they may **be** the overlays.
5. **Widen the byte windows** in the trace (already done in next probe rev) so we can see ~16 stubs around each fire — gives us the local stub-table chunk for fingerprinting.
6. **Pull the real caller off the stack**: once we know the calling convention (near vs far), the bottom word(s) of the stack at fire time identify the real game-code caller of the stub.  For Fire #0, the caller should resolve to `main()` somewhere just past the `B8 D4 17` site.
