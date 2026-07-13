-- Matilda Rive build overlay.
-- MATILDA_RIVE_BACKEND=metal (macOS) | d3d (Windows) | cg (macOS CPU fallback)
local backend = os.getenv('MATILDA_RIVE_BACKEND')
if backend == nil or backend == '' then
    if os.target() == 'windows' then
        backend = 'd3d'
    else
        backend = 'metal'
    end
end

dofile('../build/rive_build_config.lua')

RIVE_RUNTIME_DIR = path.getabsolute('..')

-- Match JUCE/CMake on Windows (/MD) — rive defaults to /MT for static libs.
filter { 'system:windows' }
    staticruntime 'Off'
filter {}

dofile(RIVE_RUNTIME_DIR .. '/premake5_v2.lua')
dofile(RIVE_RUNTIME_DIR .. '/decoders/premake5_v2.lua')

if backend == 'cg' then
    dofile(RIVE_RUNTIME_DIR .. '/cg_renderer/premake5.lua')
elseif backend == 'metal' or backend == 'd3d' then
    dofile(RIVE_RUNTIME_DIR .. '/renderer/premake5_pls_renderer.lua')
else
    error('Unknown MATILDA_RIVE_BACKEND: ' .. backend)
end
