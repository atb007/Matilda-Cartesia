# Build static Rive libraries for Matilda (macOS + Windows).
#
# Backends (MATILDA_RIVE_BACKEND):
#   metal — GPU PLS renderer on macOS (default)
#   d3d   — GPU PLS renderer on Windows (default)
#   cg    — CPU CoreGraphics fallback (macOS only)
$ErrorActionPreference = 'Stop'

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$StandaloneDir = Resolve-Path (Join-Path $ScriptDir '..')
$RiveRuntimeDir = Join-Path $StandaloneDir 'third_party/rive-runtime'
$RiveMatildaDir = Join-Path $RiveRuntimeDir 'matilda'
$RiveBuildPs1 = Join-Path $RiveRuntimeDir 'build/build_rive.ps1'
$RiveOverlaySrc = Join-Path $StandaloneDir 'rive-build/matilda/premake5.lua'
$RivePinFile = Join-Path $StandaloneDir 'RIVE_RUNTIME_PIN'

if (-not (Test-Path $RiveBuildPs1)) {
    Write-Error @"
rive-runtime not found. Clone and checkout the pinned revision:
  git clone https://github.com/rive-app/rive-runtime.git $RiveRuntimeDir
"@
    if (Test-Path $RivePinFile) {
        $pin = (Get-Content $RivePinFile -Raw).Trim()
        Write-Error "  cd $RiveRuntimeDir; git checkout $pin"
    }
    exit 1
}

New-Item -ItemType Directory -Force -Path $RiveMatildaDir | Out-Null
$overlayDest = Join-Path $RiveMatildaDir 'premake5.lua'
if (-not (Test-Path $overlayDest)) {
    Copy-Item $RiveOverlaySrc $overlayDest
}

if (-not $env:MATILDA_RIVE_BACKEND -or $env:MATILDA_RIVE_BACKEND -eq '') {
    $env:MATILDA_RIVE_BACKEND = 'd3d'
}
$Backend = $env:MATILDA_RIVE_BACKEND

$env:RIVE_PREMAKE_ARGS = '--with_rive_text --with_rive_layout --with_rive_scripting'

Push-Location $RiveMatildaDir
try {
    $commonTargets = @(
        'zlib', 'libpng', 'libjpeg', 'libwebp', 'rive_yoga', 'rive_harfbuzz',
        'rive_sheenbidi', 'miniaudio', 'luau_vm', 'rive', 'rive_decoders'
    )

    if ($Backend -eq 'metal' -or $Backend -eq 'd3d') {
        $targets = $commonTargets + @('rive_pls_renderer')
    }
    elseif ($Backend -eq 'cg') {
        $targets = $commonTargets + @('rive_cg_renderer')
    }
    else {
        Write-Error "Unknown MATILDA_RIVE_BACKEND: $Backend (use metal, d3d, or cg)"
    }

    & $RiveBuildPs1 release clean -- @targets
}
finally {
    Pop-Location
}

Write-Host "Rive ($Backend) libraries built in: $RiveMatildaDir/out/release"
