# Matilda version naming

Matilda uses **SemVer** tags: `vMAJOR.MINOR.PATCH`  
Source of truth: `matilda/plugin/VERSION` (must match `matilda/standalone/CMakeLists.txt` `project(… VERSION …)`).

Pushing tag `v*` runs **Matilda Release** and publishes GitHub Release zips.

---

## When to bump

| Bump | Use when | Examples |
|------|----------|----------|
| **PATCH** `1.0.x → 1.0.x+1` | Bug fix, CI/build fix, or **restore intended UI/behavior** that already shipped in design | DXGI init fix, right-edge framing clip, crash fix, wrong log path |
| **MINOR** `1.x.0 → 1.(x+1).0` | **New user-visible capability** or feature set that wasn’t previously working/shipped | New UI module, preset pack as a feature drop, new host/platform capability “now works for real”, new Rive bindings users can see |
| **MAJOR** `x.0.0 → (x+1).0.0` | **Breaking** change | Patch JSON / preset incompatibility, intentional silent-instrument / MIDI routing break, remove a public workflow users rely on |

### Decision rule (when unsure)

1. Would a user describe it as **“a bug got fixed”** or **“the UI looks like it was supposed to”**? → **PATCH**
2. Would they describe it as **“Matilda can do something new”** (or a platform that never worked now does)? → **MINOR**
3. Would old presets / DAW setups **break or need migration**? → **MAJOR**

Hotfix series mid-flight (e.g. Windows Rive **v1.0.15–v1.0.17**) may stay on **PATCH** until the path is validated. The next comparable platform unlock should start a **MINOR** (e.g. `1.1.0`) unless it’s purely restorative polish.

---

## Release checklist

1. Bump `matilda/plugin/VERSION` **and** standalone `project(… VERSION …)` together.
2. Add `matilda/plugin/releases/vX.Y.Z-notes.md`.
3. Update “Latest” lines in `matilda/plugin/README.md` and `matilda/plugin/releases/README.md`.
4. Commit → `git tag -a vX.Y.Z` → push commit + tag.
5. Update docs milestones/spec only when the release **closes a real milestone** (not every PATCH).

### Retagging

- **Do not** force-move a tag after a **successful** GitHub Release with assets.
- If the release workflow **failed before publish**, fixing and moving the same tag is OK (as with the first v1.0.15 MSVC break). Prefer `vX.Y.Z+1` whenever users may already have downloaded builds.

---

## Historical note (Windows Rive)

| Tag | Role |
|-----|------|
| v1.0.15 | Offscreen D3D path (capability attempt) |
| v1.0.16 | DXGI factory fix (PATCH) |
| v1.0.17 | Framing parity (PATCH) — **Windows Rive milestone validated** |

Recorded in [docs/cartesia-vst/MILESTONES.md](../../../docs/cartesia-vst/MILESTONES.md#-windows-rive-hero-validated--aug-12-2026).
