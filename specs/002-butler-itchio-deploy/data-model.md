# Data Model: Butler itch.io Deployment

**Branch**: `002-butler-itchio-deploy` | **Date**: 2026-03-08

## Entities

### Deployment Configuration (`deploy.conf`)

A key=value file at the repository root storing itch.io deployment target settings.

| Field           | Type   | Default              | Description                                  |
|-----------------|--------|----------------------|----------------------------------------------|
| ITCHIO_USER     | string | (required)           | itch.io username or organization             |
| ITCHIO_GAME     | string | (required)           | itch.io project slug                         |
| ITCHIO_CHANNEL  | string | `html5`              | Butler channel name for the platform target  |
| BUILD_DIR       | string | `build_wasm`         | Relative path to WASM build output directory |

**Validation Rules**:
- `ITCHIO_USER` must be non-empty
- `ITCHIO_GAME` must be non-empty
- `ITCHIO_CHANNEL` must be non-empty (defaults to `html5` if omitted)
- `BUILD_DIR` must be a valid directory path relative to repo root

**Relationships**:
- Used by `deploy_itchio.sh` to construct the Butler push target: `${ITCHIO_USER}/${ITCHIO_GAME}:${ITCHIO_CHANNEL}`
- The `BUILD_DIR` value corresponds to the output of the Emscripten build (default `build_wasm/`)

### Build Output (directory)

The set of files produced by `cmake --build build_wasm` that constitute the deployable WASM application.

| File                        | Required | Description                     |
|-----------------------------|----------|---------------------------------|
| RemomMapgenItchio.html      | Yes      | HTML shell (entry point)        |
| RemomMapgenItchio.js        | Yes      | Emscripten JS glue code         |
| RemomMapgenItchio.wasm      | Yes      | WebAssembly binary              |
| RemomMapgenItchio.data      | No       | Preloaded asset bundle (if any) |

**Validation Rules**:
- All three required files must exist before deployment
- The directory must not be empty

### Deployment Result (transient — not persisted)

The outcome of a `butler push` invocation, captured from Butler's stdout/stderr.

| Field        | Type    | Description                                      |
|--------------|---------|--------------------------------------------------|
| success      | boolean | Whether Butler exited with code 0                |
| build_id     | integer | Version number assigned by itch.io (from output) |
| project_url  | string  | Constructed as `https://{user}.itch.io/{game}`   |
| error_output | string  | Butler's stderr if the push failed               |

**State Transitions**: None — this is a one-shot operation with no persistent state.
