#!/bin/bash
# Build WASM and stage to dist. Used by /build skill.
# Powered by Claude.
set -e

cd "$(dirname "$0")"

source /c/emsdk/emsdk_env.sh 2>/dev/null

echo "=== Building WASM ==="
touch src/main.cpp
cmake --build build_wasm

DIST_DIR="build_wasm/dist"
rm -rf "$DIST_DIR"
mkdir -p "$DIST_DIR"
cp build_wasm/RemomMapgenItchio.html "$DIST_DIR/index.html"
cp build_wasm/RemomMapgenItchio.js "$DIST_DIR/"
cp build_wasm/RemomMapgenItchio.wasm "$DIST_DIR/"

echo "=== Build complete, staged to $DIST_DIR/ ==="
