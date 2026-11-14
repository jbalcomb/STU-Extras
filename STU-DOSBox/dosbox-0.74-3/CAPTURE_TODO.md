# DOSBox State Capture — Task List

Companion to [CAPTURE_PRD.md](CAPTURE_PRD.md) and [CAPTURE_PLAN.md](CAPTURE_PLAN.md).

## Legend

- [x] Done & verified
- [ ] Pending
- [~] In progress / partially done
- [?] Blocked or needs decision

---

## v1 — Static config-file probes (DONE)

### Research & design

- [x] Read DOSBox's existing logging system to understand the patterns (Section_prop, AddDestroyFunction, LOG_StartUp registration). See [LOGGING_REVIEW.md](LOGGING_REVIEW.md).
- [x] Decide v1 scope vs. v2 scope. v1 = static config probes, real-mode only, CSV output. v2 = embedded Lua, bidirectional read+inject.
- [x] Decide probe-list syntax. Chose multi-line `Section_line` style, matching `[autoexec]`, over numbered `probe1=…` keys.
- [x] Decide emit policy. Chose always-emit (one row per tick) over change-only, for uniform downstream time-series.

### Implementation

- [x] Write [src/misc/capture_state.cpp](src/misc/capture_state.cpp) — probe parsing, PIC scheduling, CSV output, destroy hook.
- [x] Register section in [src/dosbox.cpp](src/dosbox.cpp): one extern decl + one `CAPTURE_STATE_Init()` call before `[autoexec]`.
- [x] Add `capture_state.cpp` to [src/misc/Makefile.am](src/misc/Makefile.am).
- [x] Add `capture_state.cpp` to [src/misc/Makefile.in](src/misc/Makefile.in) (objects list, sources list, dependency include).

### Build & verification

- [x] `make capture_state.o` — single-file compile clean.
- [x] `make` — full project build clean.
- [x] Smoke test: probe BIOS tick counter at `0040:006C` for 3 s at 1000 Hz. Result: 2260 rows, 41-tick delta over 2240 ms = 54.6 ms/tick (matches BIOS 55 ms/18.2 Hz). Sample rate 1009 Hz vs. 1000 Hz configured.

### Bugs found & fixed

- [x] **Off-by-1000 sample rate** — initial code passed `1.0e6f / rate_hz` to `PIC_AddEvent` thinking microseconds. `PIC_AddEvent`'s delay is **milliseconds**, undocumented in [include/pic.h](include/pic.h#L62) and only inferable from a code comment at [src/hardware/vga_draw.cpp:1134](src/hardware/vga_draw.cpp#L1134). Fixed to `1.0e3f / rate_hz`. Memorized in project memory.

### Documentation

- [x] [CAPTURE_PRD.md](CAPTURE_PRD.md) — requirements and scope.
- [x] [CAPTURE_PLAN.md](CAPTURE_PLAN.md) — implementation plan and test plan.
- [x] [CAPTURE_TODO.md](CAPTURE_TODO.md) — this file.

---

## v1.2 — Sentinel verify (DONE)

Initial v1.1 (`ds` symbolic) had a severe noise floor: PIC ticks often land during DOS/BIOS interrupts where DS isn't the target program's segment, so most samples were garbage reads from the wrong segment. Added a guard mechanism.

- [x] `verify = SEG : OFF : ASCII_BYTES` config line. Multiple verify lines AND'ed.
- [x] Capture tick drops the row entirely if any verifier fails. `sample` counter still increments so gaps are visible.
- [x] Smoke-tested: always-failing verifier yields 0 data rows; no verifier behaves as before.
- [x] mom.conf updated with two verifiers — "Borland" at DS:0004 (Borland C runtime banner) and "Master of Magic" at DS:52B3 (game-specific).

## v1.1 — Pointer-indirect probes (DONE)

The original v1 spec only supported fixed `SEG:OFF` reads. MoM's globals are pointer variables whose value depends on the runtime data-segment layout, so v1 as-shipped couldn't reach them. Added two indirect forms.

- [x] Extend Probe struct + parser with `ptr` indirect form (16-bit near pointer).
- [x] Extend with `farptr` form (32-bit far pointer: offset + segment, LE).
- [x] Add `ds` symbolic segment (resolves to the live DS register at sample time).
- [x] Tolerate whitespace around `:` in probe specs.
- [x] Tolerate whitespace around `=` in `capturefile=` / `rate_hz=` lines.
- [x] Smoke-test `ptr` near mode via BIOS keyboard head pointer at `0040:001A` → resolves to `0x1E` (start of kbd buffer). Confirmed.
- [x] Smoke-test `farptr` far mode via INT 21h vector at `0000:0084` → far pointer `F000:14C0`; first bytes of DOSBox's INT 21h handler (`FB FE` = STI + callback prefix) read cleanly. Confirmed.
- [x] Documented in `CAPTURE_PRD.md` (F4/F4a/F4b), `CAPTURE_PLAN.md` §3.

## Next step — Master of Magic 1.31 farmer-count capture

**Game**: [/home/jbalcomb/STU_DBWD/MoM131](/home/jbalcomb/STU_DBWD/MoM131) (real-mode DOS, `MAGIC.EXE`).
**Target probe**: `cities[0].farmer_count` — farmer count of the human player's first city, single byte.

### Launch plumbing (DONE)

- [x] Workspace at [/home/jbalcomb/STU_DBWD/dosbox-capture/](file:///home/jbalcomb/STU_DBWD/dosbox-capture/).
- [x] `mom.conf` mounts MoM131 as C:, launches `MAGIC.EXE`, configures `[capture]` with the far-pointer probe `city0_farmers : farptr : ds : 9CBC : 15 : byte`.
- [x] `launch-mom.sh` wraps invocation with pre-flight checks and `cd`s into the workspace so MEMDUMP files land predictably.
- [x] Baseline play session validated: SAVE1 created with 3 farmers, SAVE2 created with 4 farmers.

### Address resolution (DONE)

- [x] Corrected: gameplay runs in **WIZARDS.EXE** (not MAGIC.EXE — MAGIC is the front-end / new-game setup).
- [x] `_CITIES` in WIZARDS.EXE: `DS:0x9CBC`, **4-byte FAR pointer** (not 2-byte near as initially assumed).
- [x] `sizeof(s_CITY) = 0x66`, `offsetof(farmer_count) = 0x15`.
- [x] Cross-checked: SAVE1 vs SAVE2 file diff lands at file offset `0x8AC1` = `cities[0]` base (0x8AAC) + 0x15. Save-file layout and in-memory struct layout match.
- [x] Hands-on debugger verified: with SAVE9 loaded, `D DS:9CBC` showed `00 00 53 69` → far pointer `6953:0000`; `D 6953:0` showed "Charack\0..."; `6953:0015 = 0x03` matched the city screen.

### Capture & verify

- [ ] Launch MoM via `launch-mom.sh`. Confirm `CAPTURE: 1 probe(s), 10 Hz -> .../mom.csv` appears in dosbox's startup log.
- [ ] Load SAVE1, play to a city screen, reassign farmers↔workers at noted moments. Exit.
- [ ] Inspect `mom.csv`:
  - [ ] Header row present: `sample,wall_ms,city0_farmers`.
  - [ ] Pre-game-load samples may be junk (DS not yet MoM's). Once in-game, probe values should be plausible (range 0–25).
  - [ ] Column changes align with in-game reassignments. Verify with: `awk -F, 'NR>1 && $3!=prev {print; prev=$3}' mom.csv`
- [ ] **Success criterion**: at least one in-game farmer reassignment produces a corresponding column transition in the CSV within one sample interval (≤ 100 ms at 10 Hz).

### Stretch (nice-to-have)

- [ ] Add probes for `cities[0].population` (offset +0x14), `cities[0].size` (+0x13), and other s_CITY fields per `MOM_DAT.h`.
- [ ] Probe a second city: `city1_farmers : farptr : ds : 9CBC : 7B : byte` (delta = `1 * 0x66 + 0x15 = 0x7B`) — confirms the struct stride.
- [ ] If `ds` proves unreliable, find WIZARDS.EXE's literal DS via the debugger and pin the probe to a hex segment value.

---

## v2 — Embedded Lua (future, not started)

### Foundation

- [ ] Vendor Lua 5.x source into [src/libs/lua/](src/libs/) (MIT-licensed, ~250 KB).
- [ ] Add Lua build wiring (`configure.ac`, autotools), with a `--disable-lua` opt-out.
- [ ] Provide a `[capture]`-adjacent `[lua]` section: `script=PATH`.
- [ ] Boot the Lua VM in a new `LUA_Init` function, source-equivalent to `CAPTURE_STATE_Init`.

### Read-side bindings (parity with v1)

- [ ] Bind `mem_readb/w/d(PhysPt)` and `real_readb/w/d(seg, off)`.
- [ ] Expose CPU register globals (EAX, EBX, …, CS, DS, …) as readable Lua values.
- [ ] Bind `PIC_AddEvent` so scripts can register their own periodic and one-shot callbacks.
- [ ] Provide a `capture_csv:write(label, value, ...)` helper for scripts that want v1-style CSV output without reinventing it.

### Write-side bindings (the closed-loop unlock)

- [ ] Bind `KEYBOARD_AddKey(scancode, pressed)` for synthetic keystrokes.
- [ ] Expose mouse motion + button injection.
- [ ] Expose joystick state injection.
- [ ] Bind `mem_writeb/w/d` for direct guest-memory poking (cheat/test scenarios).

### Event hooks

- [ ] INT 21h (DOS API) dispatch hook — fires before/after each DOS service call with AH and registers.
- [ ] Screen mode change hook.
- [ ] Vblank hook.

### Sandbox & safety

- [ ] Decide on Lua stdlib exposure. Default: math, string, table only. No `io`, no `os`, no `package`.
- [ ] Wallclock + instruction-count budget per callback, with a configurable kill switch.

### Migration

- [ ] Decide fate of `[capture]` config section. Either keep as sugar over a built-in Lua script, or deprecate. Either way, all v1 CSV outputs must remain reproducible from a Lua-only script.

### Documentation

- [ ] Write v2 PRD when ready to start.
- [ ] Document the Lua API surface and ship a `docs/lua-cookbook.md` with worked examples (capture loop, input injection, INT 21h tracing).
