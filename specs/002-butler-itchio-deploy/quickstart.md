# Quickstart: Butler itch.io Deployment

**Branch**: `002-butler-itchio-deploy` | **Date**: 2026-03-08

## Prerequisites

- Butler CLI installed and on PATH ([installation guide](https://itch.io/docs/butler/installing.html))
- Butler authenticated (`butler login`)
- Emscripten SDK installed and configured
- An itch.io project page created for the game

## One-Time Setup

1. Copy the example configuration:

```bash
cp deploy.conf.example deploy.conf
```

2. Edit `deploy.conf` with your itch.io details:

```
ITCHIO_USER=your-username
ITCHIO_GAME=mom-scenario-editor
ITCHIO_CHANNEL=html5
BUILD_DIR=build_wasm
```

3. `deploy.conf` is already in `.gitignore` — your credentials stay local. The example file is safe to commit.

## Deploy Only (WASM build already exists)

```bash
# Build WASM first (if not already done)
bash build_wasm.sh

# Deploy to itch.io
bash deploy_itchio.sh
```

## Build and Deploy in One Step

```bash
bash deploy_itchio.sh --build
```

This runs the WASM build first, then deploys on success.

## Windows

```batch
REM Deploy only
deploy_itchio.bat

REM Build and deploy
deploy_itchio.bat --build
```

## Verify Deployment

After a successful push, open your itch.io project page:

```
https://your-username.itch.io/mom-scenario-editor
```

The game should be playable in the browser immediately.

## Troubleshooting

| Problem | Solution |
|---------|----------|
| `butler: command not found` | Install Butler: https://itch.io/docs/butler/installing.html |
| `Authentication required` | Run `butler login` to authenticate |
| `Build output not found` | Run `bash build_wasm.sh` first, or use `--build` flag |
| `Project not found on itch.io` | Create the project on https://itch.io/game/new first |
| `Network error during upload` | Butler retries automatically; check your internet connection |
