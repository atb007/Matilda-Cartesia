#!/usr/bin/env bash
# Build static Rive libraries for Matilda macOS standalone.
#
# Backends (MATILDA_RIVE_BACKEND):
#   metal — GPU PLS renderer (default, recommended)
#   cg    — CPU CoreGraphics fallback
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
STANDALONE_DIR="$SCRIPT_DIR/.."
RIVE_RUNTIME_DIR="$STANDALONE_DIR/third_party/rive-runtime"
RIVE_MATILDA_DIR="$RIVE_RUNTIME_DIR/matilda"
RIVE_BUILD_SH="$RIVE_RUNTIME_DIR/build/build_rive.sh"
RIVE_OVERLAY_SRC="$STANDALONE_DIR/rive-build/matilda/premake5.lua"
RIVE_PIN_FILE="$STANDALONE_DIR/RIVE_RUNTIME_PIN"

if [[ "$(uname -s)" != "Darwin" ]]; then
    echo "Matilda native Rive build is macOS-only." >&2
    exit 1
fi

if [[ ! -x "$RIVE_BUILD_SH" ]]; then
    echo "rive-runtime not found. Clone and checkout the pinned revision:" >&2
    echo "  git clone https://github.com/rive-app/rive-runtime.git $RIVE_RUNTIME_DIR" >&2
    if [[ -f "$RIVE_PIN_FILE" ]]; then
        echo "  cd $RIVE_RUNTIME_DIR && git checkout $(tr -d '[:space:]' < \"$RIVE_PIN_FILE\")" >&2
    fi
    exit 1
fi

mkdir -p "$RIVE_MATILDA_DIR"
if [[ ! -f "$RIVE_MATILDA_DIR/premake5.lua" ]]; then
    cp "$RIVE_OVERLAY_SRC" "$RIVE_MATILDA_DIR/premake5.lua"
fi

BACKEND="${MATILDA_RIVE_BACKEND:-metal}"
export MATILDA_RIVE_BACKEND="$BACKEND"
export RIVE_PREMAKE_ARGS="--with_rive_text --with_rive_layout --with_rive_scripting"
if [[ "$BACKEND" == "metal" ]]; then
    export RIVE_PREMAKE_ARGS="${RIVE_PREMAKE_ARGS} --with_objc_exceptions"
fi

cd "$RIVE_MATILDA_DIR"

COMMON_TARGETS=(
    zlib
    libpng
    libjpeg
    libwebp
    rive_yoga
    rive_harfbuzz
    rive_sheenbidi
    miniaudio
    luau_vm
    rive
    rive_decoders
)

if [[ "$BACKEND" == "metal" ]]; then
    TARGETS=("${COMMON_TARGETS[@]}" rive_pls_renderer)
elif [[ "$BACKEND" == "cg" ]]; then
    TARGETS=("${COMMON_TARGETS[@]}" rive_cg_renderer)
else
    echo "Unknown MATILDA_RIVE_BACKEND: $BACKEND (use metal or cg)" >&2
    exit 1
fi

"$RIVE_BUILD_SH" release clean -- "${TARGETS[@]}"

echo "Rive ($BACKEND) libraries built in: $RIVE_MATILDA_DIR/out/release"
