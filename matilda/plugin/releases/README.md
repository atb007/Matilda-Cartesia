# Matilda pre-built binaries

**Do not commit plugin binaries here.** Builds are published via GitHub Actions:

| Workflow | Trigger | Output |
|----------|---------|--------|
| [Matilda Release](https://github.com/atb007/Matilda-Cartesia/actions/workflows/matilda-release.yml) | Push tag `v*` (e.g. `v1.0.1`) or **Run workflow** in Actions | GitHub Release zips |
| [Matilda CI](https://github.com/atb007/Matilda-Cartesia/actions/workflows/matilda-ci.yml) | Push / PR to `main` | Build verification only |

## Download (latest release)

1. Open [github.com/atb007/Matilda-Cartesia/releases](https://github.com/atb007/Matilda-Cartesia/releases)
2. Download the zip for your platform:

| File | Platform |
|------|----------|
| `Matilda-Windows-vst3.zip` | **FL Studio / Windows DAWs** |
| `Matilda-Windows-standalone.zip` | Windows standalone app |
| `Matilda-macOS-vst3.zip` | macOS VST3 (Logic, Ableton, etc.) |
| `Matilda-macOS-standalone.zip` | macOS Standalone |

## Install

### Windows (FL Studio)

1. Unzip `Matilda-Windows-vst3.zip`
2. Copy the `Matilda.vst3` folder to:
   ```
   C:\Program Files\Common Files\VST3\
   ```
3. FL Studio → **Options → Manage plugins** → add folder if needed → **Find plugins**

### macOS

1. Unzip `Matilda-macOS-vst3.zip`
2. Copy `Matilda.vst3` to:
   ```
   ~/Library/Audio/Plug-Ins/VST3/
   ```
3. Rescan plugins in your DAW

Universal binary (Apple Silicon + Intel).

## Cut a new release (maintainers)

**When to bump MAJOR / MINOR / PATCH:** see [VERSIONING.md](./VERSIONING.md).

```bash
# 1) Bump matilda/plugin/VERSION + standalone CMake project VERSION
# 2) Add releases/vX.Y.Z-notes.md + update Latest lines
# 3) Commit, then:
git tag -a vX.Y.Z -m "vX.Y.Z — short why"
git push origin HEAD
git push origin vX.Y.Z
```

Latest release: **v1.0.17** — Windows Rive right-edge framing (mask extend / Metal parity).  
**Milestone:** Windows Rive hero validated (v1.0.15–v1.0.17) — see docs `MILESTONES.md`.

Or: Actions → **Matilda Release** → **Run workflow** → enter tag name.
