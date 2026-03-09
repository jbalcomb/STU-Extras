# Research: Butler itch.io Deployment

**Branch**: `002-butler-itchio-deploy` | **Date**: 2026-03-08

## R1: Butler CLI Push Command Semantics

**Decision**: Use `butler push <directory> <user/game:channel>` as the core deployment command.

**Rationale**: Butler's `push` command is the standard way to upload builds. It:
- Takes a directory path (not a zip file) and handles packaging internally
- Supports incremental patching (only uploads changed bytes after the first push)
- Assigns monotonically increasing version numbers automatically
- Requires the format `username/project-name:channel-name` for the target

**Alternatives considered**:
- Manual zip + itch.io web upload API: More complex, no incremental patching, requires API key management
- Butler `upload` command: Does not exist; `push` is the correct verb

## R2: Channel Naming Convention

**Decision**: Use `html5` as the default channel name for WASM/web builds.

**Rationale**: itch.io uses channel names to determine platform compatibility. The `html5` channel tells itch.io to serve the build as an in-browser game. This is the standard convention documented by itch.io.

**Alternatives considered**:
- `web`: Non-standard; itch.io wouldn't auto-detect platform
- `wasm`: Non-standard; same issue
- Custom channel names: Supported by Butler but would require manual itch.io dashboard configuration

## R3: Configuration File Format

**Decision**: Use a simple key=value `.env`-style configuration file (`deploy.conf`).

**Rationale**: Shell scripts can source key=value files with a single `source` command. No parsing libraries needed. The format is human-readable and easily editable. Three values are needed:
- `ITCHIO_USER` — itch.io username (e.g., `jbalcomb`)
- `ITCHIO_GAME` — project slug (e.g., `mom-scenario-editor`)
- `ITCHIO_CHANNEL` — channel name (default: `html5`)

**Alternatives considered**:
- JSON config: Requires `jq` or similar parser in shell scripts — adds a dependency
- YAML config: Same parsing issue
- Command-line arguments only: No persistence between runs
- Environment variables only: Works but easy to forget to set

## R4: Butler Installation Detection

**Decision**: Use `command -v butler` (POSIX) or `where butler` (Windows) to check availability.

**Rationale**: Standard shell idiom for checking whether a command exists. If Butler is not found, display installation instructions pointing to https://itch.io/docs/butler/installing.html

**Alternatives considered**:
- Checking a hardcoded path: Fragile, platform-dependent
- Running `butler version` and checking exit code: Works but slower than `command -v`

## R5: Build Output Directory

**Decision**: Default to `build_wasm/` relative to the repository root. This matches the existing `build_wasm.sh` script output location and the CMake WASM configuration.

**Rationale**: The existing project already uses `build_wasm/` for Emscripten output. Butler will push the contents of this directory.

**Alternatives considered**:
- `build/` — conflicts with the native Windows build directory
- Configurable via `deploy.conf` — could be added but defaults to `build_wasm/` per Simplicity First

## R6: Required Files Validation

**Decision**: Before pushing, verify that `RemomMapgenItchio.html`, `RemomMapgenItchio.js`, and `RemomMapgenItchio.wasm` exist in the build output directory.

**Rationale**: These three files are the minimum required for the application to run in-browser. If any is missing, the build was not completed or was cleaned. Checking prevents pushing an incomplete build.

**Alternatives considered**:
- Push without validation and let itch.io/Butler report errors: Poor developer experience
- Check only .wasm: The .html and .js are equally required for the app to function
