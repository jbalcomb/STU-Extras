# DOSBox 0.74-3 — Logging Subsystem Review

> Handoff document. The reader is an AI agent unfamiliar with this codebase. Goal: give a complete picture of how logging works, where the seams are, and what to watch out for when modifying it.

## 1. Source tree map

| File | Role |
|---|---|
| `include/logging.h` | Public API. Declares `LOG`, `LOG_MSG`, the `LOG_TYPES` / `LOG_SEVERITIES` enums. Selects implementation via the `C_DEBUG` macro. |
| `src/gui/sdlmain.cpp` (≈line 1618) | `GFX_ShowMsg` — release-build implementation of `LOG_MSG`. Plain `printf` to stdout. |
| `src/debug/debug_gui.cpp` | Debug-build implementation: `DEBUG_ShowMsg`, `LOG::operator()`, `LOG_StartUp` (config registration), `LOG_Init` (config consumption), `LOG_Destroy`, plus the ncurses TUI scaffolding (`DBGUI_StartUp`, `DEBUG_RefreshPage`, etc.). |
| `src/debug/debug.cpp` (line 2026, 2043) | Calls `DBGUI_StartUp` from `DEBUG_Init`. |
| `src/dosbox.cpp` (line 420–422) | `LOG_StartUp()` is invoked here — **but only inside `#if C_DEBUG`**. |
| `src/misc/support.cpp` (line 174) | `E_Exit` — fatal-error path. Does **not** route through logging; it throws a `char*`. |

## 2. Public API surface

Defined in [include/logging.h](include/logging.h):

```cpp
enum LOG_TYPES { LOG_ALL, LOG_VGA, LOG_VGAGFX, LOG_VGAMISC, LOG_INT10,
                 LOG_SB, LOG_DMACONTROL, LOG_FPU, LOG_CPU, LOG_PAGING,
                 LOG_FCB, LOG_FILES, LOG_IOCTL, LOG_EXEC, LOG_DOSMISC,
                 LOG_PIT, LOG_KEYBOARD, LOG_PIC, LOG_MOUSE, LOG_BIOS,
                 LOG_GUI, LOG_MISC, LOG_IO, LOG_MAX };

enum LOG_SEVERITIES { LOG_NORMAL, LOG_WARN, LOG_ERROR };
```

Two call shapes are used throughout the codebase (≈600 sites between them):

```cpp
LOG_MSG("formatted %s", thing);                       // always-on, untyped
LOG(LOG_CPU, LOG_ERROR)("CPU:GRP5:Illegal opcode 0xff"); // categorized, severity-tagged
```

The `LOG(type, severity)(...)` form is a temporary object that overloads `operator()` — it's not a function call; it's two consecutive calls (constructor, then `operator()`).

## 3. Two implementations, selected by `C_DEBUG`

`C_DEBUG` is a build-time configure flag (`config.h.in`). The same source file generates radically different behavior depending on whether it's defined.

### 3a. Release build (`C_DEBUG` undefined)

[include/logging.h](include/logging.h#L41-L62):

- `LOG` is an **empty inline struct**. Every `operator()` overload has body `{ }`. **The compiler eliminates the entire categorized-logging mechanism**, including any side effects in the format-arg expressions (they're still evaluated — only the call body is empty).
- `LOG_MSG` is `#define`d to `GFX_ShowMsg`, implemented in [src/gui/sdlmain.cpp:1618-1626](src/gui/sdlmain.cpp#L1618):

```cpp
static bool no_stdout = false;
void GFX_ShowMsg(char const* format,...) {
    char buf[512];
    va_list msg;
    va_start(msg,format);
    vsprintf(buf,format,msg);
    strcat(buf,"\n");
    va_end(msg);
    if(!no_stdout) printf("%s",buf);
}
```

The release path is stateless: no logfile, no buffering, no categories, no severity, no timestamp. Just stdout.

### 3b. Debug build (`C_DEBUG` defined)

`LOG_MSG` → `DEBUG_ShowMsg` in [src/debug/debug_gui.cpp:55-81](src/debug/debug_gui.cpp#L55-L81). Each call:

1. Formats into a 512-byte stack buffer.
2. Appends `\n` if not already present.
3. Writes to the optional logfile (`FILE* debuglog`, `fopen(..., "wt+")`).
4. Pushes the line into `logBuff` — an in-memory `std::list<std::string>` ring buffer capped at `MAX_LOG_BUFFER = 500` lines.
5. Renders to the ncurses pane `dbg.win_out` via `wprintw` + `wrefresh`.

The `LOG::operator()` in [debug_gui.cpp:104-114](src/debug/debug_gui.cpp#L104-L114) wraps `DEBUG_ShowMsg`:

```cpp
void LOG::operator() (char const* format, ...) {
    char buf[512];
    /* vsprintf into buf */
    if (d_type >= LOG_MAX) return;
    if ((d_severity != LOG_ERROR) && (!loggrp[d_type].enabled)) return;
    DEBUG_ShowMsg("%10u: %s:%s\n", cycle_count, loggrp[d_type].front, buf);
}
```

So in debug builds:
- Each categorized log line is prefixed with the emulator's **cycle count** (10-digit, right-justified) and the **category label** (`VGA`, `CPU`, `SBLASTER`, …).
- A category can be silenced via config — **except** when the severity is `LOG_ERROR`, which always fires.

## 4. Configuration ([log] section)

Registered in `LOG_StartUp` ([debug_gui.cpp:218-261](src/debug/debug_gui.cpp#L218-L261)). The section exposes:

```ini
[log]
logfile=        # path; empty means no file
all=true
vga=true
vgagfx=true
... (one bool per LOG_TYPES entry, lowercased)
```

`LOG_Init` reads the values back into `loggrp[i].enabled` and opens `debuglog`. A destroy hook (`LOG_Destroy`) closes the file on shutdown.

Category labels are set in `LOG_StartUp` (uppercase: `"VGA"`, `"SBLASTER"`, `"DMA_CONTROL"`, …). The same labels are lowercased to derive config-key names.

## 5. Initialization order (debug build)

1. `main` → `DOSBOX_Init` ([src/dosbox.cpp:421](src/dosbox.cpp#L421)) → `LOG_StartUp()`.
   - This is guarded by `#if C_DEBUG`. **In release builds the `[log]` config section never gets registered.**
2. Later, when the user's config is parsed, `LOG_Init` runs (registered via `control->AddSection_prop("log", LOG_Init)`) and:
   - opens `debuglog` if `logfile=` is set,
   - copies the per-category bools into `loggrp[]`.
3. Independently, `DEBUG_Init` ([src/debug/debug.cpp:2043](src/debug/debug.cpp#L2043)) calls `DBGUI_StartUp` ([debug.cpp:2026](src/debug/debug.cpp#L2026)) which calls `initscr()` + `MakeSubWindows()` to build the ncurses TUI. This happens at startup in debug builds regardless of whether the user actively enters the debugger.

## 6. Platform / console notes

[src/gui/sdlmain.cpp:1862-1883](src/gui/sdlmain.cpp#L1862) (Windows only, release builds):
- `-noconsole`: `FreeConsole()`, then `freopen("stdout.txt", "w", stdout)` and `freopen("stderr.txt", "w", stderr)`. If the `stdout.txt` redirect fails (read-only working dir), `no_stdout` flips to `true` so `GFX_ShowMsg` skips its `printf` call — preventing a crash on the closed stream.
- Default Windows path (no `-noconsole`): `AllocConsole()` and bind stdio to `CONIN$`/`CONOUT$`, then `SetConsoleTitle("DOSBox Status Window")`. This is the "DOSBox Status Window" the README references.
- Whole block is excluded when `C_DEBUG` is on (`#if defined(WIN32) && !(C_DEBUG)`) — the debug build owns the terminal for its ncurses UI.

## 7. Fatal errors

`E_Exit` in [src/misc/support.cpp:174](src/misc/support.cpp#L174) is the project's fatal-error sink:

```cpp
void E_Exit(const char * format,...) {
#if C_DEBUG && C_HEAVY_DEBUG
    DEBUG_HeavyWriteLogInstruction();
#endif
    /* vsprintf into static buf, append \n */
    throw(buf);
}
```

Two things to know:
- It **throws a `char*`**, caught in `sdlmain.cpp`'s top-level handler. It does not go through `LOG_MSG` / `LOG`.
- If built with both `C_DEBUG` and `C_HEAVY_DEBUG`, the heavy-debug path dumps the recent instruction trace before unwinding. That's a separate logging system (see `src/debug/debug.cpp`'s heavy-debug machinery) layered on top of the basic logging described here.

## 8. Quirks and footguns

These are the non-obvious things future modifications need to respect.

### 8.1 Severity ≠ filter in release

In release builds `LOG(_, LOG_ERROR)("...")` is just as silent as `LOG(_, LOG_NORMAL)("...")`. The empty struct doesn't inspect arguments. If you want an error to actually be visible in a shipped build, use `LOG_MSG`, not `LOG(..., LOG_ERROR)`. This is the opposite of what the symbol name implies and is the single biggest gotcha.

### 8.2 The release `LOG` struct has an *incomplete* overload set

[include/logging.h:41-59](include/logging.h#L41-L59) lists only specific arg-type combinations of `double` and `char const*`. A `LOG(...)("%s %s %s", a, b, c)` call (three strings) has **no matching overload in release builds** — it compiles in debug (where `operator()` is variadic) but fails to compile in release. The header even has a TODO comment: `}; //add missing operators to here`.

When adding new `LOG(...)("fmt", args)` call sites with novel arg-type combinations, **build with `--enable-debug` *and* `--disable-debug` before declaring done**. Otherwise you can ship breakage to the build flavor you didn't test.

### 8.3 `loggrp[]` static init is partial

[debug_gui.cpp:48](src/debug/debug_gui.cpp#L48):

```cpp
static _LogGroup loggrp[LOG_MAX]={{"",true},{0,false}};
```

Only `loggrp[0]` (`LOG_ALL`) gets `enabled=true` at static-init time; the rest are zero-initialized (`enabled=false`). They get overwritten by `LOG_Init` reading the config, but any `LOG(...)` call that fires *before* `LOG_Init` runs will be filtered out (except `LOG_ALL` and `LOG_ERROR`). If you're adding logging to very early startup code, expect it to be silently dropped.

### 8.4 `LOG_ALL` is a misleading name

`LOG_ALL` is just the first enum entry. It is not a "log to all categories" toggle and it is not a master enable. Its `.front` label is `"ALL"` in startup but its `loggrp` entry is initialized with `""` at file scope — only [debug_gui.cpp:220](src/debug/debug_gui.cpp#L220) sets it to `"ALL"`, in `LOG_StartUp`. It's just a category like any other.

### 8.5 The `[log]` section does not exist in release builds

`LOG_StartUp` is only called inside `#if C_DEBUG` in [src/dosbox.cpp:420-422](src/dosbox.cpp#L420). So in a stock release build, `logfile=` and the per-category bools in the user's config are silently ignored. A user expecting to enable `logfile=dosbox.log` in their config will get nothing unless they have a debug build.

### 8.6 No timestamps

The line prefix is `cycle_count` (emulated 8086 cycles), not wall time. This is great for correlating with emulator state, useless for correlating with anything external (system clock, host process events). If you need wall-clock timing, you have to add it.

### 8.7 512-byte format buffer, no overflow guard

Both `GFX_ShowMsg` and `DEBUG_ShowMsg` use a 512-byte local `char buf[512]` with `vsprintf` (not `vsnprintf`). Long lines can overrun. Most call sites format short strings, but any user-supplied content in the format args could in principle overflow. Worth knowing if extending the system to handle arbitrary strings (e.g. file paths from DOS programs).

### 8.8 Curses output and stdout are mutually exclusive

In debug builds, `initscr()` takes over the terminal, so `printf`/`std::cout` from anywhere else in the program will corrupt the curses display. All log output must go through `DEBUG_ShowMsg`. If you add a feature that prints diagnostics to stdout, gate it on `!C_DEBUG`.

## 9. Suggested directions (if asked to modernize)

The next agent may be asked to extend or replace this system. Some realistic improvement vectors, ordered by reach:

1. **Make `LOG_ERROR` always emit, even in release.** Currently the most useful severity is the one most aggressively compiled away. A trivial fix: in the release `LOG` struct, give `operator()` a body that delegates to `LOG_MSG` when `d_severity == LOG_ERROR`. (Requires storing severity, which is already a struct member in the debug path but absent in release.)
2. **Replace the overload list with a variadic template.** The footgun in §8.2 disappears, and the header gets shorter.
3. **Unify the release and debug paths.** Always register the `[log]` config section, always allow file logging, keep the ncurses TUI as a debug-only consumer of the same stream.
4. **Add timestamps** alongside `cycle_count`.
5. **Switch from `vsprintf` to `vsnprintf`** — one-line change, removes the overflow risk.
6. **Promote `E_Exit` to log its message before throwing**, so a fatal error always appears in the logfile rather than only on stderr via the exception handler.

Any of these is a self-contained change; (1) and (5) are the highest-value lowest-risk picks.

## 10. Quick reference

```text
                +---------------------------+
                |   call site (anywhere)    |
                +-------------+-------------+
                              |
              ____________if (C_DEBUG)____________
             /                                    \
       (defined)                              (undefined)
            |                                       |
            v                                       v
  +---------+----------+                +-----------+----------+
  |   LOG(t,s)(...)    |                |    LOG(t,s)(...)     |
  | debug_gui.cpp:104  |                |    empty inline      |
  |  - filter by cat   |                |    (compiled away)   |
  |  - prefix cycles   |                |                      |
  +---------+----------+                +-----------+----------+
            |                                       |
            v                                       |
  +--------------------+                            |
  |  DEBUG_ShowMsg     |                            |
  |  debug_gui.cpp:55  |                            |
  |  - logfile FILE*   |                            |
  |  - ring buf (500)  |                            |
  |  - ncurses pane    |                            |
  +--------------------+                            |
                                                    |
            +----------+ LOG_MSG +------------------+
                       (#define)
                          |
                          v
            +---------------------------+
            | GFX_ShowMsg               |
            | sdlmain.cpp:1618          |
            | printf to stdout          |
            | unless no_stdout (Win)    |
            +---------------------------+
```
