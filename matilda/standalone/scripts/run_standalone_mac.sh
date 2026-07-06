#!/usr/bin/env bash
# Configure, build, and open Matilda macOS standalone (Rive GPU path).
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
STANDALONE_DIR="$SCRIPT_DIR/.."
BUILD_DIR="$STANDALONE_DIR/build"
BACKEND="${MATILDA_RIVE_BACKEND:-metal}"

echo "==> Rive backend: $BACKEND"
"$SCRIPT_DIR/build_rive_native.sh"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake .. \
    -DMATILDA_RIVE_HERO=ON \
    -DMATILDA_RIVE_BACKEND="$BACKEND"
cmake --build . --config Release --target Matilda_Standalone -j"$(sysctl -n hw.ncpu 2>/dev/null || echo 4)"

open "$BUILD_DIR/Matilda_artefacts/Release/Standalone/Matilda.app"
