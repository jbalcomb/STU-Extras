# DOSBox State Capture — Product Requirements Document

## 1. Summary

A new opt-in subsystem in DOSBox 0.74-3 that periodically samples user-specified guest memory addresses while a DOS game is running and writes the values to a host-side CSV file. The output is intended for consumption by external tools — analytics scripts, plots, or AI agents observing emulator state.

## 2. Background

DOSBox provides excellent emulation of DOS-era hardware but no machine-readable channel for inspecting guest state in real time. The existing `[log]` section emits human-formatted text into a curses pane (debug build only) and the existing `captures=` directory captures audio/video — neither is suited for structured numeric data extraction.

MAME ships an embedded Lua interpreter (`-autoboot_script`) and exposes its memory bus to scripts, which is one of the main reasons MAME is preferred over DOSBox for research and automation work today. This feature begins closing that gap.

## 3. Goals

- **v1 (this PRD)**: Let a user configure a list of fixed guest-memory probes in `dosbox.conf` and produce a uniform-rate CSV time-series of their values during a game session.
- **v2 (future)**: Replace the static probe list with an embedded Lua interpreter exposing both *read* (memory, registers) and *write* (keyboard, mouse, joystick) interfaces — enabling closed-loop external AI agents.

## 4. Non-goals (v1)

- Protected-mode / linear-address probes. Real-mode `SEG:OFF` only.
- Dynamic probe management at runtime (debugger commands, hot-reload).
- Built-in memory scanner / Cheat-Engine-style discovery. The user finds addresses externally.
- Compressed, binary, or alternative output formats. CSV only.
- Input injection from host. Out of scope until v2.
- Hooking specific DOS API calls (INT 21h etc.). Pure memory sampling.

## 5. Users

- **Primary (v1)**: A human researcher reverse-engineering a DOS game, who already knows the address(es) of interest (from community memory maps, prior RE, or interactive use of the DOSBox curses debugger).
- **Secondary (v2)**: External AI agents and automation scripts that observe game state through the CSV and (eventually) inject input through Lua.

## 6. Functional requirements

| ID | Requirement |
|---|---|
| F1 | A new `[capture]` section exists in `dosbox.conf`, registered as a `Section_line` (free-form multi-line, like `[autoexec]`). |
| F2 | The section accepts `capturefile=PATH` to set the output CSV path. Empty path → feature disabled. |
| F3 | The section accepts `rate_hz=N` (default 60, clamped to `[1, 10000]`). |
| F4 | The section accepts three probe-line forms: **direct** `LABEL:SEG:OFF:WIDTH`, **indirect near** `LABEL:ptr:SEG:POFF:DELTA:WIDTH`, and **indirect far** `LABEL:farptr:SEG:POFF:DELTA:WIDTH`. `SEG`/`OFF`/`POFF`/`DELTA` are hex 16-bit values without prefix; `SEG` may also be the keyword `ds`, in which case the live DS register is used at sample time. `WIDTH` is `byte`, `word`, or `dword`. Whitespace around `:` is tolerated. |
| F4a | Indirect-near probes read a 16-bit pointer at `SEG:POFF`, then sample `WIDTH` bytes at `SEG:(pointer+DELTA)`. Use for pointers to data in the same segment. |
| F4b | Indirect-far probes read a 32-bit far pointer at `SEG:POFF` (little-endian: offset then segment), then sample `WIDTH` bytes at `resolved_seg:(resolved_off+DELTA)`. Use for pointers to data in a *different* segment than the pointer itself — typical for Borland C++ large-model DOS binaries like Master of Magic. |
| F5 | Optional sentinel guard: `verify = SEG:OFF:ASCII_BYTES`. Multiple lines AND'ed. If any check fails on a given PIC tick, the row is dropped entirely (no header-mismatched data lands in the CSV). Used to gate `ds`-symbolic probes on a stable program-identifying string so that ticks landing during DOS/BIOS interrupts don't produce garbage. |
| F5 | `#` introduces a comment; blank lines are ignored. |
| F6 | Malformed probe lines are skipped with a `LOG_MSG` warning, not fatal. |
| F7 | On startup, the CSV is opened (`w` mode, truncating) and a header row `sample,wall_ms,<labels...>` is written. |
| F8 | A PIC-scheduled tick samples every probe at the configured rate and emits one CSV row per sample, `fflush`ed after each. |
| F9 | The row contains: monotonic `sample` index (0-based), `GetTicks()` wall-millisecond timestamp, then each probe value as a decimal unsigned integer. |
| F10 | Probes are read via `real_readb` / `real_readw` / `real_readd` from [include/mem.h](include/mem.h). |
| F11 | Feature works in both release and `C_DEBUG` builds. |
| F12 | On section destroy (program exit, `-resetconf`, etc.) the PIC event is removed and the file is closed. |

## 7. Non-functional requirements

| ID | Requirement |
|---|---|
| N1 | Zero observable performance impact when `capturefile=` is empty (no scheduled events, no allocations beyond a small static struct). |
| N2 | Negligible impact at typical rates (60 Hz × ≤ 16 probes): a few hundred bytes/sec of stdio. |
| N3 | No new external dependencies. Pure C++03 + DOSBox internal APIs. |
| N4 | Build system change is additive: one new source file in `src/misc/`, no header churn. |
| N5 | Lua-shaped architectural hole preserved: `capture_tick` iterates a `std::vector<Probe>`; v2 can plug a second probe source into the same tick without rework. |

## 8. Success criteria

| ID | Check |
|---|---|
| S1 | Release build (no `--enable-debug`) compiles without warnings introduced by the new module. |
| S2 | Smoke test: probing BIOS tick counter at `0040:006C` (dword) for ≥ 2 seconds produces a CSV whose probe column advances at the BIOS's real 55 ms (18.2 Hz) interval. — **MET** at 2260 samples / 2.24 s with 1000 Hz config, 41-tick delta in 2240 ms = 54.6 ms/tick. |
| S3 | Sample count over the run matches `rate_hz × elapsed_seconds` within ±2%. — **MET** (2260 samples / 2.24 s ≈ 1009 Hz vs. 1000 Hz configured). |
| S4 | Feature has no effect when `capturefile=` is empty. — **MET** by guard in `capture_init`. |
| S5 | Header row is followed by exactly one data row per PIC tick. — **MET**. |

## 9. Open questions / known limitations

- **Real-mode only**: protected-mode DOS games (DOS4GW-era titles like Doom, Wing Commander Privateer 2) cannot be probed in v1. Workaround: wait for v2 Lua, which can expose `mem_readb(PhysPt)` directly.
- **Probe address discovery is the user's problem.** v1 ships no scanner. Discovery options: (a) the existing DOSBox curses debugger (`MEMDUMP`, watch breakpoints); (b) community-published memory maps for known games; (c) external tools (`scanmem` against the host process). v2 should consider a built-in scanner.
- **Indirect probes are near-pointer only.** Far pointers (4-byte seg:off) aren't supported in v1. If a target field lives in a different segment than the pointer, wait for v2 Lua.
- **`ds` symbolic segment**: when a probe uses `ds`, the value of DS at the moment the PIC tick fires depends on what guest code was last executing. In practice this is wrong *often* — possibly >50% of samples during gameplay land during DOS/BIOS/video interrupts, where DS is some other segment. Mitigation: use a `verify = SEG:OFF:ASCII_BYTES` sentinel guard so only ticks where DS holds the correct program's data segment emit a row. Without verify, the CSV is dominated by noise from wrong-segment reads. For maximum determinism, find the program's DS via the debugger once and use a literal SEG instead of `ds`.
- **No timestamps in walltime UTC**, only `GetTicks()` ms relative to DOSBox start. Adequate for relative timing; user must record the start time externally if absolute timestamps are needed.
- **`PIC_AddEvent` units gotcha**: the header doesn't say so but `delay` is **milliseconds**. Documented in [include/pic.h](include/pic.h#L62) only by convention. Future maintainers should be aware.

## 10. Future work (v2 — embedded Lua, not in this PRD's scope)

Vendor Lua 5.x into `src/libs/`, expose bindings for:

- **Read**: `mem_readb/w/d`, `real_readb/w/d`, CPU register globals, the PIC scheduler (so scripts can register their own periodic and event-driven callbacks).
- **Write**: `KEYBOARD_AddKey`, mouse motion injection, joystick state — enabling external AI agents to drive a game in a tight feedback loop.
- **Hooks**: INT 21h dispatch, screen mode changes, vblank — so scripts can react to game events rather than poll.

v1's static probe list is intentionally a stepping stone; when v2 ships the `[capture]` section will likely remain as a thin sugar layer over a Lua probe definition, or be deprecated in favor of pure Lua scripts.

## 11. Next step — Master of Magic 1.31 farmer-count capture

**Goal**: Use the v1 feature to capture the farmer count of the human player's first city (`city_idx: 0`) from Master of Magic 1.31, installed at `/home/jbalcomb/STU_DBWD/MoM131`.

**Why**: First real-world validation. Confirms the feature is usable on an actual DOS game, exposes any rough edges around mounting/launch, and produces a concrete artifact (a per-tick CSV trace of in-game farmer count) that demonstrates value.

**Approach**:

1. **Stand up a launch config** — `dosbox.conf` that `MOUNT`s the MoM directory, runs `MAGIC.EXE`, and includes a placeholder `[capture]` section. See [CAPTURE_PLAN.md](CAPTURE_PLAN.md#next-step-procedure) for the exact file.
2. **Discover the farmer-count address** — *this is the load-bearing unknown*. Two paths in order of preference:
   - Check community resources. Master of Magic has an active modding scene (Caster of Magic, Insecticide patch, various wiki pages) and the city struct's memory layout has been documented. Pull the offset from there if available.
   - Hands-on RE with the DOSBox debugger (`--enable-debug` build). Save a game with a known farmer count `N`, search memory for `N`, increment farmers in-game, re-search the candidate set, repeat until one address remains. Note: MoM cities are stored in a fixed-size array, so `cities[0].farmers` is at a constant offset.
3. **Add the probe line** to `[capture]`. Gameplay runs in **WIZARDS.EXE** (MAGIC.EXE is just the front-end / new-game setup). `_CITIES` in WIZARDS.EXE is a 4-byte **far** pointer at `DS:0x9CBC`. The probe:

   ```ini
   city0_farmers : farptr : ds : 9CBC : 15 : byte
   ```

   `0x15` is `offsetof(s_CITY, farmer_count)` from [ReMoM/MoX/src/MOM_DAT.h](../devel/ReMoM/MoX/src/MOM_DAT.h) line 1672. For a different `city_idx` N, the delta becomes `N * 0x66 + 0x15` (since `sizeof(s_CITY) = 0x66`).

   Hand-verified 2026-05-26: with SAVE9 loaded, the address chain resolved to `6953:0000` (cities[0].name = "Charack"), and `6953:0015 = 0x03` matched the in-game farmer count.
4. **Verify**: launch MoM, play a few turns including manually changing the worker/farmer split, exit DOSBox, inspect the CSV — the column should track the in-game value.

**Success criterion for the next step**: a CSV where `city0_farmers` changes value at the exact moment the user re-assigns workers in the MoM city screen, and remains stable otherwise.

**Estimated effort**:

- Launch config: 5 minutes.
- Address discovery: 30 min – 4 hours depending on whether a community memory map is available. (Hands-on RE in DOSBox's debugger is the long tail.)
- Verification: 15 minutes once the address is known.
