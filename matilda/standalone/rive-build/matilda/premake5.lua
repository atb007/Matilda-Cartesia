-- Matilda Rive build for macOS standalone.
-- MATILDA_RIVE_BACKEND=metal (default) | cg
local backend = os.getenv('MATILDA_RIVE_BACKEND') or 'metal'

dofile('../build/rive_build_config.lua')

RIVE_RUNTIME_DIR = path.getabsolute('..')

dofile(RIVE_RUNTIME_DIR .. '/premake5_v2.lua')
dofile(RIVE_RUNTIME_DIR .. '/decoders/premake5_v2.lua')

if backend == 'cg' then
    dofile(RIVE_RUNTIME_DIR .. '/cg_renderer/premake5.lua')
elseif backend == 'metal' then
    dofile(RIVE_RUNTIME_DIR .. '/renderer/premake5_pls_renderer.lua')
else
    error('Unknown MATILDA_RIVE_BACKEND: ' .. backend)
end
