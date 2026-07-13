#!/usr/bin/env bash
# Build static Rive libraries for Matilda (macOS + Windows via Git Bash).
#
# Backends (MATILDA_RIVE_BACKEND):
#   metal — GPU PLS renderer on macOS (default)
#   d3d   — GPU PLS renderer on Windows (default)
#   cg    — CPU CoreGraphics fallback (macOS only)
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
STANDALONE_DIR="$SCRIPT_DIR/.."
RIVE_RUNTIME_DIR="$STANDALONE_DIR/third_party/rive-runtime"
RIVE_MATILDA_DIR="$RIVE_RUNTIME_DIR/matilda"
RIVE_OVERLAY_SRC="$STANDALONE_DIR/rive-build/matilda/premake5.lua"
RIVE_PIN_FILE="$STANDALONE_DIR/RIVE_RUNTIME_PIN"

OS_NAME="$(uname -s)"
if [[ "$OS_NAME" == "Darwin" ]]; then
    RIVE_BUILD_SH="$RIVE_RUNTIME_DIR/build/build_rive.sh"
elif [[ "$OS_NAME" == MINGW* || "$OS_NAME" == MSYS* || "$OS_NAME" == CYGWIN* ]]; then
    RIVE_BUILD_SH="$RIVE_RUNTIME_DIR/build/build_rive.ps1"
else
    echo "Unsupported platform for Matilda native Rive build: $OS_NAME" >&2
    exit 1
fi

if [[ ! -x "$RIVE_BUILD_SH" && ! -f "$RIVE_BUILD_SH" ]]; then
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

if [[ -z "${MATILDA_RIVE_BACKEND:-}" ]]; then
    if [[ "$OS_NAME" == "Darwin" ]]; then
        BACKEND="metal"
    else
        BACKEND="d3d"
    fi
else
    BACKEND="$MATILDA_RIVE_BACKEND"
fi
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

if [[ "$BACKEND" == "metal" || "$BACKEND" == "d3d" ]]; then
    TARGETS=("${COMMON_TARGETS[@]}" rive_pls_renderer)
elif [[ "$BACKEND" == "cg" ]]; then
    TARGETS=("${COMMON_TARGETS[@]}" rive_cg_renderer)
else
    echo "Unknown MATILDA_RIVE_BACKEND: $BACKEND (use metal, d3d, or cg)" >&2
    exit 1
fi

if [[ "$OS_NAME" == "Darwin" ]]; then
    "$RIVE_BUILD_SH" release clean -- "${TARGETS[@]}"
else
    powershell.exe -NoProfile -ExecutionPolicy Bypass -File "$RIVE_BUILD_SH" release clean -- "${TARGETS[@]}"
fi

echo "Rive ($BACKEND) libraries built in: $RIVE_MATILDA_DIR/out/release"
