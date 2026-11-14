# DOSBox State Capture — Implementation Plan

This is the as-built plan for v1, plus the procedure for the MoM next-step. For context, scope, and acceptance criteria see [CAPTURE_PRD.md](CAPTURE_PRD.md).

## 1. Architecture

```
                      dosbox.conf
                           |
                  ┌────────┴────────┐
                  │  [capture]      │   parsed as Section_line
                  │  capturefile=…  │
                  │  rate_hz=…      │
                  │  label:seg:off  │
                  └────────┬────────┘
                           │  capture_init(Section*)
                           ▼
                  ┌─────────────────┐
                  │ probes: vector  │
                  │ capfile: FILE*  │
                  └────────┬────────┘
                           │  PIC_AddEvent(capture_tick, tick_ms)
                           ▼
                  ┌─────────────────┐
       ┌────────► │  capture_tick   │  (re-scheduling)
       │          │  - real_read*   │
       │          │  - fprintf CSV  │
       │          │  - fflush       │
       │          └────────┬────────┘
       │                   │
       └───────────────────┘
                           ▼
                       out.csv
```

The implementation reuses the same configuration & lifecycle machinery as the existing `[autoexec]` and `[log]` sections: registration via `control->AddSection_line`, init callback dispatched when the config is parsed, destroy callback registered via `AddDestroyFunction` for clean teardown.

## 2. File-by-file changes (as built)

### 2.1 New file: [src/misc/capture_state.cpp](src/misc/capture_state.cpp) (~170 LoC)

Single translation unit, no associated header. Exports one C-linkage symbol:

```cpp
void CAPTURE_STATE_Init();   // called once from DOSBOX_Init
```

Internal state (file-scope static):

| Symbol | Purpose |
|---|---|
| `std::vector<Probe> probes` | parsed probe list |
| `FILE* capfile` | output stream (NULL when disabled) |
| `float tick_ms` | configured sample interval in **milliseconds** (PIC convention) |
| `Bit32u sample_idx` | monotonic row counter, 0-based |
| `bool tick_scheduled` | whether a PIC event is currently scheduled |

Internal helpers:

| Function | Role |
|---|---|
| `parse_uint16_hex(s, *out)` | strict hex parser, rejects empty/overlong/non-numeric |
| `parse_probe(spec, &out)` | splits on `:` into 4 fields, validates each |
| `read_probe(p)` | dispatches to `real_readb` / `real_readw` / `real_readd` by width |
| `capture_tick(Bitu)` | reads all probes, writes one row, reschedules itself |
| `capture_init(Section*)` | parses `Section_line::data`, opens file, schedules first event |
| `capture_destroy(Section*)` | removes event, closes file, clears state |

### 2.2 Edit: [src/dosbox.cpp](src/dosbox.cpp)

Two additions:

```cpp
// near the top, with the other extern decls
void CAPTURE_STATE_Init(void);

// inside DOSBOX_Init, just before the [autoexec] line
CAPTURE_STATE_Init();
secline = control->AddSection_line("autoexec", &AUTOEXEC_Init);
```

The call is **not** gated on `C_DEBUG` (unlike `LOG_StartUp`), so the `[capture]` section is available in release builds.

### 2.3 Edit: [src/misc/Makefile.am](src/misc/Makefile.am)

```diff
-libmisc_a_SOURCES = cross.cpp messages.cpp programs.cpp setup.cpp support.cpp
+libmisc_a_SOURCES = capture_state.cpp cross.cpp messages.cpp programs.cpp setup.cpp support.cpp
```

### 2.4 Edit: [src/misc/Makefile.in](src/misc/Makefile.in)

Three additions: object file in `am_libmisc_a_OBJECTS`, source in `libmisc_a_SOURCES`, dependency-include directive for `capture_state.Po`.

> The generated `Makefile.in` is edited directly here so the build works without re-running `autoreconf`. If `autogen.sh` is run later it will regenerate this file from the `.am`.

## 3. Public surface (config format)

```ini
[capture]
# Output CSV path. Empty = feature disabled.
capturefile = /tmp/run1.csv

# Samples per second. Default 60. Clamped to [1, 10000].
rate_hz = 60

# Probes -- one per line. Two forms:
#
#   Direct:   LABEL : SEG : OFF : WIDTH
#   Indirect: LABEL : ptr : SEG : POFF : DELTA : WIDTH
#
# SEG may be a 4-hex-digit literal or the keyword `ds` (live DS register
# at sample time). All other numeric fields are hex without 0x.
# WIDTH = byte | word | dword. Whitespace around `:` is tolerated.
#
# Indirect: read a 16-bit near pointer at SEG:POFF, then sample WIDTH bytes
# at SEG:(pointer + DELTA). This is the path used to reach heap-allocated
# structures via a static-globals pointer.

# Sentinel guards (optional, AND'ed). Use when probes are `ds`-symbolic
# to gate out ticks where DS isn't the target program's data segment.
verify = ds : 0004 : Borland

bios_ticks   : 0040  : 006C : dword
city0_farmer : farptr : ds  : 9CBC : 15 : byte

# Lines starting with '#' are comments. Blank lines are skipped.
```

Output schema:

```csv
sample,wall_ms,score,lives,player_x
0,123,1200,3,256
1,140,1200,3,260
...
```

- `sample`: 0-based monotonic counter.
- `wall_ms`: `GetTicks()` (i.e. `SDL_GetTicks()`) — wall-clock ms since DOSBox start.
- Probe columns: decimal unsigned integer, regardless of width.

## 4. Test plan

### 4.1 Build verification

```bash
cd /home/jbalcomb/STU/developp/dosbox-0.74-3
make 2>&1 | tail -5
```

Expected: build succeeds, `src/dosbox` is regenerated.

### 4.2 Smoke test — BIOS tick counter

The BIOS at `0040:006C` maintains a 32-bit tick counter incremented every 55 ms (18.2 Hz) by INT 08h. This is a known-changing value and requires no game.

```ini
# /tmp/dosbox-capture-test/test.conf
[sdl]
output=surface

[capture]
capturefile = /tmp/dosbox-capture-test/out.csv
rate_hz     = 1000
bios_ticks    : 0040 : 006C : dword
bios_ticks_hi : 0040 : 006E : word

[autoexec]
echo CaptureTest
```

Run (3-second host-side timeout):

```bash
cd /tmp/dosbox-capture-test
timeout 3 env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
  /home/jbalcomb/STU/developp/dosbox-0.74-3/src/dosbox -conf test.conf
wc -l out.csv
head -5 out.csv
```

**Verified results** (2026-05-26): 2260 sample rows, `bios_ticks` advances 41 over 2240 ms (= 54.6 ms/tick, matches the 55 ms BIOS interval), sample rate 1009 Hz vs. 1000 Hz configured (within ±1%).

### 4.3 Negative test — disabled by default

Run without a `[capture]` section: no PIC events scheduled, no file created, no perceptible startup cost.

## 5. Risks & known limitations

| # | Risk | Mitigation |
|---|---|---|
| R1 | `PIC_AddEvent` units are not documented in [include/pic.h](include/pic.h#L62) — easy to pass microseconds and get 1000× wrong rate. | Convention pinned in this plan + saved to project memory. Already burned once during smoke-test; symptom is a 1000× longer sample interval than expected. |
| R2 | Probes that point at unmapped memory may segfault on `mem_read*`. | DOSBox's memory layer returns `0xFF` for unmapped reads; not actually a crash. Confirmed by code inspection of `mem_readb` in [src/cpu/paging.cpp](src/cpu/paging.cpp). |
| R3 | Static `fopen("w")` truncates the previous CSV on each run. | Intentional. Append mode would mix runs in one file. Caller can rename pre-run if accumulating. |
| R4 | Real-mode addressing only — DOS4GW protected-mode games can't be probed. | Acknowledged non-goal; v2 (Lua) gets `mem_readb(PhysPt)` access. |
| R5 | High `rate_hz` (e.g. 10000) can produce large files and may modestly slow the emulator. | `rate_hz` is clamped at 10000 in `capture_init`. |

## 6. Reproducible build & test commands

```bash
# Clean & rebuild after editing capture_state.cpp
cd /home/jbalcomb/STU/developp/dosbox-0.74-3
make

# Single-source compile (faster iteration)
cd src/misc
make capture_state.o

# Run the BIOS-tick smoke test
mkdir -p /tmp/dosbox-capture-test
# (write test.conf as shown in §4.2)
cd /tmp/dosbox-capture-test
timeout 3 env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
  /home/jbalcomb/STU/developp/dosbox-0.74-3/src/dosbox -conf test.conf
wc -l out.csv && head -5 out.csv && tail -3 out.csv
```

## 7. Next-step procedure — MoM 1.31 farmer-count capture

**Goal**: capture `cities[0].farmers` (farmer count of the human player's first city) of Master of Magic 1.31 to CSV while playing.

**Game install**: [/home/jbalcomb/STU_DBWD/MoM131](/home/jbalcomb/STU_DBWD/MoM131) — main executable is `MAGIC.EXE` (real-mode DOS, not DOS4GW protected mode — confirmed by directory listing).

### 7.1 Launch config (committed)

Lives at [/home/jbalcomb/STU_DBWD/dosbox-capture/mom.conf](../STU_DBWD/dosbox-capture/mom.conf) and is launched by [/home/jbalcomb/STU_DBWD/dosbox-capture/launch-mom.sh](../STU_DBWD/dosbox-capture/launch-mom.sh). Mounts the MoM install as C:, runs MAGIC.EXE, and probes `cities[0].farmer_count` via the indirect form.

### 7.2 Address resolution (done)

Resolved via the user's ReMoM/MoX RE project at [/home/jbalcomb/STU/devel/ReMoM/MoX/src/MOM_DAT.h](../devel/ReMoM/MoX/src/MOM_DAT.h) **plus hands-on debugger verification**:

| Symbol | Value |
|---|---|
| Gameplay binary | **WIZARDS.EXE** (MAGIC.EXE is the front-end only) |
| `&_CITIES` (location of the pointer in WIZARDS.EXE) | `DS:0x9CBC` |
| `_CITIES` storage | **4-byte FAR pointer** (Borland C++ large-model build) |
| `sizeof(s_CITY)` | `0x66` (102 bytes) |
| `offsetof(s_CITY, farmer_count)` | `0x15` |
| Probe delta for `city_idx=0` | `0 * 0x66 + 0x15 = 0x15` |

Cross-checked against save-file diff: SAVE1 (3 farmers) vs SAVE2 (4 farmers) differ at file offset `0x8AC1`, which matches `cities[0].farmer_count` per the struct layout.

**Hand-verified in the curses debugger (2026-05-26)** with SAVE9 loaded:
- `DS = 284C` (varies per session)
- `D 284C:9CBC` showed `00 00 53 69 ...` → far pointer = `6953:0000`
- `D 6953:0` showed `43 68 61 72 61 63 6B 00 ...` = "Charack\0" (cities[0].name)
- `6953:0015 = 0x03` = farmer count, matching the city screen.

For the front-end / new-game setup phase, `_CITIES` in MAGIC.EXE is at `DS:0x89D2`. Only meaningful during wizard selection / new-game creation; null otherwise.

### 7.3 Run it

```bash
/home/jbalcomb/STU_DBWD/dosbox-capture/launch-mom.sh
```

Load `SAVE1` (or any save with a city), open the city screen, reassign farmers↔workers at known moments. The CSV at `/home/jbalcomb/STU_DBWD/dosbox-capture/mom.csv` should track the in-game farmer count with at most one sample (100 ms at 10 Hz) of lag.

### 7.4 Verify

```bash
head /tmp/mom-capture/mom.csv
# Expected: header + uniform 30 Hz time series of byte values 0–25.
awk -F, 'NR>1 && $3 != prev {print NR, $0; prev=$3}' /tmp/mom-capture/mom.csv
# Expected: rows printed only when the farmer count changes,
# aligned with the in-game reassignments.
```

If the column values look like sensible farmer counts and the change events align with user actions, the next step is complete.

### 7.5 Limitations / things to expect

- MoM is keyboard- and mouse-driven; interaction with the captured value is asynchronous to the sampling clock. Set `rate_hz=30` (matching VGA refresh) for legible traces; faster sampling will produce many redundant rows.
- If `MAGIC.EXE` segfaults or hangs on launch, that's an emulator-side issue unrelated to the capture feature — verify a clean run *without* the `[capture]` section first.
- If the CSV is non-empty but the probe column is always `0` or always `0xFF`, the address is wrong (unmapped or in code segment, not data).
- The data segment value may differ between save-game loads vs. fresh starts. Re-verify the address against `SEG:OFF` immediately after entering the game, not from a memory map alone.
