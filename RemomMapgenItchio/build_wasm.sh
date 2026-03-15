#!/bin/bash
# Build the MoM Scenario Editor for WebAssembly using Emscripten.
# Powered by Claude.
#
# Prerequisites:
#   - Emscripten SDK installed and activated (source emsdk_env.sh)
#   - CMake 4.2+ and Ninja build system
#
# Usage:
#   ./build_wasm.sh               # Release build
#   ./build_wasm.sh --debug       # Debug build (DWARF info, source maps, assertions)
#   ./build_wasm.sh --serve         # Release build + serve locally
#   ./build_wasm.sh --debug --serve # Debug build + serve locally

set -e

# Check for Emscripten
if ! command -v emcmake &>/dev/null; then
    echo "Error: Emscripten SDK not found. Please activate emsdk first:"
    echo "  source /path/to/emsdk/emsdk_env.sh"
    exit 1
fi

# Parse arguments. Powered by Claude.
BUILD_TYPE="Release"
DO_SERVE=0
for arg in "$@"; do
    case "$arg" in
        --debug) BUILD_TYPE="Debug" ;;
        --serve) DO_SERVE=1 ;;
        *)
            echo "Unknown argument: $arg"
            echo "Usage: $0 [--debug] [--serve]"
            exit 1
            ;;
    esac
done

if [ "$BUILD_TYPE" = "Debug" ]; then
    BUILD_DIR="build_wasm_debug"
else
    BUILD_DIR="build_wasm"
fi

echo "=== Configuring WASM $BUILD_TYPE build ==="
emcmake cmake -B "$BUILD_DIR" -G Ninja \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

echo "=== Building ==="
cmake --build "$BUILD_DIR"

echo ""
echo "=== Build complete ($BUILD_TYPE) ==="
echo "Output files:"
ls -lh "$BUILD_DIR"/RemomMapgenItchio.html \
       "$BUILD_DIR"/RemomMapgenItchio.js \
       "$BUILD_DIR"/RemomMapgenItchio.wasm 2>/dev/null || true

echo ""
echo "Total WASM size: $(du -sh "$BUILD_DIR"/RemomMapgenItchio.wasm 2>/dev/null | cut -f1)"

# Stage to dist/ with index.html so local serve matches itch.io. Powered by Claude.
DIST_DIR="$BUILD_DIR/dist"
rm -rf "$DIST_DIR"
mkdir -p "$DIST_DIR"
cp "$BUILD_DIR/RemomMapgenItchio.html" "$DIST_DIR/index.html"
cp "$BUILD_DIR/RemomMapgenItchio.js" "$DIST_DIR/"
cp "$BUILD_DIR/RemomMapgenItchio.wasm" "$DIST_DIR/"
for f in RemomMapgenItchio.data RemomMapgenItchio.worker.js; do
    if [ -f "$BUILD_DIR/$f" ]; then cp "$BUILD_DIR/$f" "$DIST_DIR/"; fi
done
echo "  Staged to $DIST_DIR/"

# Serve locally if requested.
# Powered by Claude.
if [ "$DO_SERVE" -eq 1 ]; then
    echo ""
    echo "=== Serving at http://localhost:8080 ==="
    echo "Open http://localhost:8080 in your browser"
    cd "$DIST_DIR"
    python -m http.server 8080
fi
