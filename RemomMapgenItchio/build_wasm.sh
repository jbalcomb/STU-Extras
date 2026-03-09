#!/bin/bash
# Build the MoM Scenario Editor for WebAssembly using Emscripten.
# Powered by Claude.
#
# Prerequisites:
#   - Emscripten SDK installed and activated (source emsdk_env.sh)
#   - CMake 4.2+ and Ninja build system
#
# Usage:
#   ./build_wasm.sh        # build
#   ./build_wasm.sh serve  # build + serve locally for testing

set -e

# Check for Emscripten
if ! command -v emcmake &>/dev/null; then
    echo "Error: Emscripten SDK not found. Please activate emsdk first:"
    echo "  source /path/to/emsdk/emsdk_env.sh"
    exit 1
fi

BUILD_DIR="build_wasm"

echo "=== Configuring WASM build ==="
emcmake cmake -B "$BUILD_DIR" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release

echo "=== Building ==="
cmake --build "$BUILD_DIR"

echo ""
echo "=== Build complete ==="
echo "Output files:"
ls -lh "$BUILD_DIR"/RemomMapgenItchio.html \
       "$BUILD_DIR"/RemomMapgenItchio.js \
       "$BUILD_DIR"/RemomMapgenItchio.wasm 2>/dev/null || true

echo ""
echo "Total WASM size: $(du -sh "$BUILD_DIR"/RemomMapgenItchio.wasm 2>/dev/null | cut -f1)"

# Serve locally if requested.
# Powered by Claude.
if [ "$1" = "serve" ]; then
    echo ""
    echo "=== Serving at http://localhost:8080 ==="
    echo "Open http://localhost:8080/RemomMapgenItchio.html in your browser"
    cd "$BUILD_DIR"
    python -m http.server 8080
fi
