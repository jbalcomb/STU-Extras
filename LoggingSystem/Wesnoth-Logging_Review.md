# Wesnoth In-Game Logging — Review and Handoff Notes

This document describes the developer-facing diagnostic logging system used
inside the Wesnoth game engine. It is intended as a handoff for an AI agent
that will work on logging-related features, debugging, or instrumentation.

All paths are relative to the repository root (`/home/jbalcomb/STU/developp/wesnoth`).
The system is contained in two files plus a GUI dialog; everything else is
call-site usage.

---

## 1. Scope

"In-game logging" here refers to the C++ diagnostic logging used by the
running game binary. It is distinct from:

- **WML `[message]` chat** — story/UI dialogue shown to the player.
- **Lua `wesnoth.message` / `std_print`** — Lua-side output, which ultimately
  routes through this same C++ system.
- **The in-game chat log viewer** ([src/chat_log.cpp](src/chat_log.cpp)) —
  this is for **player chat history** in multiplayer, unrelated to diagnostics.
- **`gettext` / translations** — unrelated.

---

## 2. Architecture

### 2.1 Files

| File | Role |
|------|------|
| [src/log.hpp](src/log.hpp) | Public API: severity, log_domain, logger, macros |
| [src/log.cpp](src/log.cpp) | Implementation: domain registry, file redirection, formatting, rotation |
| [src/gui/dialogs/log_settings.hpp](src/gui/dialogs/log_settings.hpp) / [`.cpp`](src/gui/dialogs/log_settings.cpp) | In-game Preferences dialog for adjusting domain severities at runtime |
| [src/gui/dialogs/lua_interpreter.cpp](src/gui/dialogs/lua_interpreter.cpp) | Developer REPL (debug builds); independent of the diagnostic stream |

### 2.2 Core types

Defined in [src/log.hpp](src/log.hpp):

- **`lg::severity`** ([src/log.hpp:81-88](src/log.hpp#L81-L88)) — enum class:
  `LG_NONE = -1`, `LG_ERROR = 0`, `LG_WARN = 1`, `LG_INFO = 2`, `LG_DEBUG = 3`.
  Larger value = more verbose.
- **`lg::log_domain`** ([src/log.hpp:124-129](src/log.hpp#L124-L129)) —
  named category. Default severity at construction is `LG_WARN`. Each `log_domain`
  registers itself with a process-wide registry (`*domains`) when constructed.
- **`lg::logger`** ([src/log.hpp:206-228](src/log.hpp#L206-L228)) —
  carries a severity and a name (`"error"`, `"warning"`, `"info"`, `"debug"`).
  Singletons returned by `lg::err()`, `lg::warn()`, `lg::info()`, `lg::debug()`
  ([src/log.cpp:339-361](src/log.cpp#L339-L361)).
- **`lg::log_in_progress`** ([src/log.hpp:191-204](src/log.hpp#L191-L204)) —
  RAII holder for a single emitted line. Acquires `log_mutex` so writes are
  atomic across threads.

### 2.3 The decision rule

A log call **emits** iff `logger.severity_ <= domain.current_severity_`. See
the gate at [src/log.cpp:468](src/log.cpp#L468). Because the comparison uses
the numeric enum values, raising a domain to `debug` (3) implicitly enables
every lower-numbered severity for that domain.

Domains default to `LG_WARN`, so out of the box only `err()` and `warn()`
calls reach the log file.

---

## 3. Call-site idiom

Every translation unit that logs declares its own domain and a small set of
per-severity macros at the top of the file. Example from [src/ai/manager.cpp:63-66](src/ai/manager.cpp#L63-L66):

```cpp
static lg::log_domain log_ai_manager("ai/manager");
#define DBG_AI_MANAGER LOG_STREAM(debug, log_ai_manager)
#define LOG_AI_MANAGER LOG_STREAM(info,  log_ai_manager)
#define ERR_AI_MANAGER LOG_STREAM(err,   log_ai_manager)
```

The underlying macro is in [src/log.hpp:278](src/log.hpp#L278):

```cpp
#define LOG_STREAM(level, domain) \
    if (lg::level().dont_log(domain)) ; else lg::level()(domain) | formatter()
```

The `if (...) ; else ...` guard means that when the domain's severity is
below the call's level, the formatter is never constructed and the
streamed expressions are never evaluated — so leaving `DBG_*` calls in
production hot paths is cheap.

Usage at the call site looks like ordinary `operator<<` streaming:

```cpp
DBG_AI_MANAGER << "side " << side << ": total turn time: "
               << (turn_end_time - turn_start_time).count() << " ms";
```

### 3.1 Macro variants

Defined in [src/log.hpp](src/log.hpp):

- `LOG_STREAM` — standard form; prefixes severity + domain.
- `LOG_STREAM_NAMELESS` — same gating, no prefix added.
- `LOG_STREAM_INDENT` — emits with the current `lg::scope_logger` indent level.
- `PLAIN_LOG`, `STREAMING_LOG` — bypass formatting; for raw writes.
- `lg::scope_logger` ([src/log.cpp:540](src/log.cpp#L540)) — RAII helper that
  emits `{ START: <name>` on construction and `} END: <name> (took Nus)` on
  destruction, used to time and bracket scopes.

---

## 4. Output destinations

### 4.1 File redirection

`lg::set_log_to_file()` ([src/log.cpp:266-310](src/log.cpp#L266-L310)) runs
early in startup, **before** command-line parsing
(see comment at [src/log.hpp:146-148](src/log.hpp#L146-L148)). It:

1. Verifies the logs directory under the user's data dir is writable.
2. Redirects `stderr` to a new file
   `wesnoth-YYYYMMDD-HHMMSS-RAND.log` ([src/log.cpp:283](src/log.cpp#L283)).
3. On Windows, also redirects `stdout` to a parallel `.out.log`
   ([src/log.cpp:293](src/log.cpp#L293)). On Unix, `stdout` is `dup2`'d to
   `stderr` so both streams land in the same file
   ([src/log.cpp:297](src/log.cpp#L297)).
4. Rotates older logs, keeping the most recent **8** files (16 on Windows
   because of the second stream) — `lg::max_logs`
   ([src/log.hpp:73-79](src/log.hpp#L73-L79)).

Filename prefix/suffix are constants: `lg::log_file_prefix = "wesnoth-"`,
`lg::log_file_suffix = ".log"` ([src/log.hpp:67-70](src/log.hpp#L67-L70)).

Per-platform default log directories:

| Platform | Directory |
|----------|-----------|
| Linux    | `~/.local/share/wesnoth/<version>/logs/` |
| macOS    | `~/Library/Application Support/Wesnoth_<version>/logs/` |
| Windows  | `%APPDATA%\Battle for Wesnoth <version>\logs\` |
| Android  | logcat via `android_log_buf` ([src/log.cpp:61-82](src/log.cpp#L61-L82)) |

`lg::move_log_file()` ([src/log.hpp:150-154](src/log.hpp#L150-L154)) moves
the active log if the user passed a custom `--userdata-dir`.

### 4.2 Line format

Built in `logger::operator()` ([src/log.cpp:460-488](src/log.cpp#L460-L488))
and `log_in_progress::operator|` ([src/log.cpp:494-513](src/log.cpp#L494-L513)):

```
[YYYYMMDD HH:MM:SS[.uuuuuu] ]<severity-name> <domain>: <message>
```

- Timestamps are added only if the per-logger `timestamp` flag is set; the
  default is on. `--log-precise` toggles the microsecond fractional part
  ([src/log.cpp:499-507](src/log.cpp#L499-L507)).
- Strict mode (`--log-strict-...`) raises a flag the first time a logger at
  or above the strict level fires, causing the test harness to fail.

### 4.3 Sanitization

`sanitize_log()` ([src/log.cpp:509](src/log.cpp#L509), called inside
`operator|`) anonymizes the user's home directory and similar path
fragments out of log messages. Disabled with `--no-log-sanitize`. Set the
toggle programmatically via `lg::set_log_sanitize()`
([src/log.hpp:141](src/log.hpp#L141)).

### 4.4 The chat-log side channel

`lg::log_to_chat()` ([src/log.cpp:550-554](src/log.cpp#L550-L554)) is a
process-global `std::stringstream` that game code writes into when it wants
a problem **shown to the user inside the running game**, not just dumped to
the log file. Drained by the play controller and surfaced via in-game chat.

Producers include:
- WML deprecation warnings — [src/deprecation.cpp:83](src/deprecation.cpp#L83)
- AI configuration conflicts — [src/ai/configuration.cpp:299](src/ai/configuration.cpp#L299)
- Side-filter formula errors — [src/side_filter.cpp:237](src/side_filter.cpp#L237)
- Engine memory/overflow guards — [src/mouse_events.cpp:1580](src/mouse_events.cpp#L1580)

This is the only path by which the diagnostic logger surfaces *inside* the
running game window. It is one-way — `log_to_chat` does not also write to
the on-disk log automatically.

---

## 5. Runtime control

### 5.1 Command-line flags

Parsed in [src/commandline_options.cpp](src/commandline_options.cpp); applied
to the global registry via `lg::set_log_domain_severity()`
([src/log.cpp:379-396](src/log.cpp#L379-L396)).

| Flag | Effect |
|------|--------|
| `--log-error=DOM[,DOM…]` | Set listed domains to error |
| `--log-warning=…` | … to warn |
| `--log-info=…` | … to info |
| `--log-debug=…` | … to debug (most verbose) |
| `--log-none=…` | Silence listed domains |
| `--log-precise` | Microsecond timestamps |
| `--log-strict-error` / `--log-strict-warning` | Treat first match at level as fatal (for CI) |
| `--logdomains [FILTER]` | Print registered domain names and exit |
| `--no-log-to-file` / `--log-to-file` | Keep output on terminal / force redirect |
| `--no-log-sanitize` | Disable home-dir sanitization |

Environment variable: **`WESNOTH_NO_LOG_FILE`** — equivalent to `--no-log-to-file`.

### 5.2 Wildcards — non-obvious

`set_log_domain_severity()` uses `utils::wildcard_string_match()`
([src/log.cpp:389](src/log.cpp#L389)). The match is **whole-string**, not
prefix. The special name `"all"` shortcuts the loop and sets every domain
([src/log.cpp:381](src/log.cpp#L381)).

Consequences:

- `--log-debug=ai` matches a domain literally named `"ai"` and **nothing
  else**. There is no such domain — every AI domain is namespaced
  (`ai/manager`, `ai/attack`, `ai/general`, `ai/composite`, …). The
  user-intuitive command is **`--log-debug=ai/*`**.
- `--log-debug=*` and `--log-debug=all` both turn on everything globally,
  but the former goes through the regex matcher while the latter shortcuts.

### 5.3 In-game runtime control

[src/gui/dialogs/log_settings.cpp](src/gui/dialogs/log_settings.cpp) provides
the **Preferences → Advanced → Logging** dialog. It calls
`lg::list_log_domains("")` ([src/log.cpp:410-418](src/log.cpp#L410-L418)) to
populate a list of every registered domain, with per-row radio buttons for
`none / error / warn / info / debug`. Changes take effect immediately for
subsequent log calls. The dialog does **not** persist across restarts —
preferences are stored separately, see `prefs::get().disabled_log_domains()`
and the read at dialog open time.

---

## 6. Domain catalog (sampled)

The full list comes from `--logdomains` on a built binary. Common ones:

| Domain | Defined in |
|--------|-----------|
| `engine` | [src/game_events/manager.cpp](src/game_events/manager.cpp) and many others |
| `display` | [src/display.cpp](src/display.cpp) |
| `network` | [src/wesnothd_connection.cpp](src/wesnothd_connection.cpp) |
| `config` | [src/config.cpp](src/config.cpp) |
| `gui/layout` | GUI2 layout engine |
| `ai/general`, `ai/manager`, `ai/attack`, `ai/composite`, `ai/stage`, `ai/aspect`, `ai/actions`, `ai/sim_actions`, `ai/mod`, `ai/engine`, `ai/engine/cpp`, `ai/engine/lua`, `ai/testing`, `ai/ca/move_to_targets`, `ai/ca/testing_ai_default`, `ai/aspect/attacks`, `ai/stage/rca` | various files under [src/ai/](src/ai/) |

Domains are first-class strings, not hierarchical objects — the slashes are
purely a naming convention, although wildcards make the convention useful.

---

## 7. Notable observations and gotchas

These are the things most likely to bite a fresh contributor.

1. **Default severity is `LG_WARN`**, not `info`. Running the game with no
   flags produces almost nothing in the log file beyond warnings and errors.
   New `LOG_*` (info-level) calls are invisible without `--log-info=<domain>`.

2. **`--log-debug=foo` is exact-match, not prefix-match.** Use `foo/*` to
   capture sub-domains. The CLI flag does not validate; an unmatched name
   silently does nothing. `set_log_domain_severity()` returns `bool` but its
   caller in `commandline_options.cpp` does not always surface that.

3. **`set_log_to_file()` runs before CLI parsing.** A log file is created
   under the **default** userdata dir even when the user later passes
   `--userdata-dir`. `move_log_file()` shuffles it after the fact. Anything
   logged in the very early startup path lives in the original location
   briefly.

4. **The macro short-circuits.** Stream operands inside `DBG_*(...) << expr`
   are not evaluated when the domain is below `debug`. This is usually
   desirable but means side-effecting expressions in log arguments will
   silently disappear in production. Never put work with side effects on
   the right of `<<`.

5. **`log_to_chat()` is a global stringstream, not a logger.** It bypasses
   the severity/domain machinery entirely, which means messages routed
   there are not gated, not timestamped, and not written to the log file.
   If something is important enough to show the user, log it *both* ways.

6. **Log file rotation is by count, not size.** A long debugging session at
   `--log-debug=all` can produce a multi-GB single file. There is no
   built-in size cap.

7. **Thread safety is per-line.** `log_in_progress::operator|` takes
   `log_mutex` so a fully-streamed line is atomic, but the streaming
   builder itself (between `<<` operations) is not — though in practice
   the builder is stack-local per call.

8. **Strict mode is for CI, not interactive use.** `--log-strict-warning`
   converts the first warning into a process-level failure flag that the
   boost test harness checks. Do not enable it casually.

9. **Android is special.** All log calls funnel through `android_log_buf`
   ([src/log.cpp:61-82](src/log.cpp#L61-L82)) into Android's logcat;
   file rotation is bypassed.

10. **The Lua-side `std::cerr` lines also end up in the log.** Because
    `stderr` is redirected, anything a third-party library prints to it
    (SDL, Cairo, Pango, etc.) arrives in the log file without going through
    the domain system. These lines have no severity/domain prefix.

---

## 8. Quick reference for a follow-up agent

To investigate or modify logging behavior, the high-value entry points are:

- **Read first:** [src/log.hpp](src/log.hpp), [src/log.cpp](src/log.cpp).
  Total ~600 lines combined; the whole system fits in one sitting.
- **Add a new domain:** declare `static lg::log_domain log_foo("foo");`
  at file scope and define `DBG_FOO`/`LOG_FOO`/`WRN_FOO`/`ERR_FOO` macros.
  No registration is needed beyond the static constructor.
- **List domains at runtime:** `--logdomains [filter]`.
- **Enable everything for a session:**
  `./wesnoth --no-log-to-file --log-debug=all --log-precise`
- **Test logging in isolation:** the unit tests use
  `lg::redirect_output_setter` ([src/log.hpp:97-118](src/log.hpp#L97-L118))
  to capture output into a `stringstream`.
- **In-game adjustment without restart:** Preferences → Advanced → Logging.

Anything outside these touchpoints is call-site usage; the system is small
and the patterns are uniform across the codebase.
