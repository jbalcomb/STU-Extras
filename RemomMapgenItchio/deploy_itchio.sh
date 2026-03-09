#!/bin/bash
# Deploy the MoM Scenario Editor WASM build to itch.io using Butler.
# Powered by Claude.
#
# Prerequisites:
#   - Butler CLI installed and on PATH (https://itch.io/docs/butler/installing.html)
#   - Butler authenticated (butler login)
#   - WASM build completed (./build_wasm.sh)
#
# Usage:
#   ./deploy_itchio.sh           # deploy existing WASM build
#   ./deploy_itchio.sh --build   # build WASM first, then deploy

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# --- Defaults (overridden by deploy.conf if present) ---
ITCHIO_USER=""
ITCHIO_GAME=""
ITCHIO_CHANNEL="html5"
BUILD_DIR="build_wasm"

# --- Parse arguments ---
DO_BUILD=0
for arg in "$@"; do
    case "$arg" in
        --build)
            DO_BUILD=1
            ;;
        *)
            echo "Unknown argument: $arg"
            echo "Usage: $0 [--build]"
            exit 1
            ;;
    esac
done

# --- Load configuration ---
CONF_FILE="$SCRIPT_DIR/deploy.conf"
if [ -f "$CONF_FILE" ]; then
    # Source the config file to load ITCHIO_USER, ITCHIO_GAME, etc.
    # Powered by Claude.
    source "$CONF_FILE"
else
    echo "Warning: deploy.conf not found."
    echo "  Copy the example and fill in your itch.io details:"
    echo "    cp deploy.conf.example deploy.conf"
    echo ""
    echo "  Required settings:"
    echo "    ITCHIO_USER=your-username"
    echo "    ITCHIO_GAME=mom-scenario-editor"
    echo ""
    exit 1
fi

# --- Validate configuration ---
if [ -z "$ITCHIO_USER" ] || [ "$ITCHIO_USER" = "your-username" ]; then
    echo "Error: ITCHIO_USER is not set in deploy.conf."
    echo "  Edit deploy.conf and set your itch.io username."
    exit 1
fi

if [ -z "$ITCHIO_GAME" ]; then
    echo "Error: ITCHIO_GAME is not set in deploy.conf."
    echo "  Edit deploy.conf and set your itch.io project slug."
    exit 1
fi

if [ -z "$ITCHIO_CHANNEL" ]; then
    ITCHIO_CHANNEL="html5"
fi

# --- Check Butler is installed ---
if ! command -v butler &>/dev/null; then
    echo "Error: Butler is not installed or not on PATH."
    echo ""
    echo "  Install Butler from: https://itch.io/docs/butler/installing.html"
    echo ""
    echo "  After installing, make sure 'butler' is on your PATH."
    echo "  Then authenticate with: butler login"
    exit 1
fi

# --- Build if requested (--build flag) ---
if [ "$DO_BUILD" -eq 1 ]; then
    echo "=== Building WASM target ==="
    BUILD_SCRIPT="$SCRIPT_DIR/build_wasm.sh"
    if [ ! -f "$BUILD_SCRIPT" ]; then
        echo "Error: build_wasm.sh not found at $BUILD_SCRIPT"
        exit 1
    fi
    if ! bash "$BUILD_SCRIPT"; then
        echo ""
        echo "Error: WASM build failed. Deployment aborted."
        echo "  Fix the build errors above and try again."
        exit 1
    fi
    echo ""
fi

# --- Validate build output ---
BUILD_PATH="$SCRIPT_DIR/$BUILD_DIR"

if [ ! -d "$BUILD_PATH" ]; then
    echo "Error: Build output directory not found: $BUILD_DIR/"
    echo "  Run the WASM build first: ./build_wasm.sh"
    echo "  Or use: $0 --build"
    exit 1
fi

REQUIRED_FILES=("RemomMapgenItchio.html" "RemomMapgenItchio.js" "RemomMapgenItchio.wasm")
MISSING=()
for f in "${REQUIRED_FILES[@]}"; do
    if [ ! -f "$BUILD_PATH/$f" ]; then
        MISSING+=("$f")
    fi
done

if [ ${#MISSING[@]} -gt 0 ]; then
    echo "Error: Required build output files are missing from $BUILD_DIR/:"
    for f in "${MISSING[@]}"; do
        echo "  - $f"
    done
    echo ""
    echo "  Run the WASM build first: ./build_wasm.sh"
    echo "  Or use: $0 --build"
    exit 1
fi

# --- Install deployment files ---
# Use CMake install target to copy only deployment files to a clean dist
# directory. This also renames RemomMapgenItchio.html to index.html as
# required by itch.io. Powered by Claude.
DIST_DIR="$BUILD_PATH/dist"

echo "=== Installing deployment files ==="
cmake --install "$BUILD_PATH" --prefix "$DIST_DIR" 2>&1
echo ""

if [ ! -f "$DIST_DIR/index.html" ]; then
    echo "Error: cmake --install failed to produce index.html in $BUILD_DIR/dist/"
    echo "  Try rebuilding: ./build_wasm.sh"
    exit 1
fi

# --- Deploy with Butler ---
BUTLER_TARGET="${ITCHIO_USER}/${ITCHIO_GAME}:${ITCHIO_CHANNEL}"
PROJECT_URL="https://${ITCHIO_USER}.itch.io/${ITCHIO_GAME}"

echo "=== Deploying to itch.io ==="
echo "  Target: $BUTLER_TARGET"
echo "  Source: $BUILD_DIR/dist/"
echo ""

if ! butler push "$DIST_DIR" "$BUTLER_TARGET" 2>&1; then
    EXIT_CODE=$?
    echo ""
    echo "Error: Butler push failed (exit code $EXIT_CODE)."
    echo ""
    # Check for common failure modes.
    # Powered by Claude.
    echo "  Troubleshooting:"
    echo "    - Authentication expired? Run: butler login"
    echo "    - Project not found? Create it at: https://itch.io/game/new"
    echo "    - Network error? Check your internet connection (Butler retries automatically)."
    exit 1
fi

echo ""
echo "=== Deployment successful ==="
echo "  View your game at: $PROJECT_URL"
echo ""
