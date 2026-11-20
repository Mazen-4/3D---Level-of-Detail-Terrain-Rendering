# Start here: how the code is organised and what you do next

This page is for all three of us. It explains how the program fits together after
Phase 0, and which files each role fills in.

## 1. The big idea: one program, three owners, no waiting

Phase 0 already created **every file the project needs**. The files that belong to
someone else's role are **stubs**: they compile and run but do nothing useful yet
(e.g. `Frustum::cull` keeps every chunk). Each stub has `TODO [B]` or `TODO [C]`
comments that say exactly what to write.

That means:
- the program **always builds and runs**, even while parts are unfinished;
- you **only edit your own files**, so we never get merge conflicts;
- when you finish a stub, everyone else gets your feature on the next `git pull`.

Search for your tag in the code (`TODO [B]` or `TODO [C]`) to find your work.

## 2. How data flows through one frame

This is the frame loop from the plan (Section 3). `main.cpp` runs exactly this (simplified here):

```
camera.update(window, dt);                                    // [A] WASD + mouse
if (!settings.freezeCulling) cullCamera = camera;             // [C] debug: freeze
visible = grid.allChunks();                                   // [B] 64 chunks for a 1025 map
if (settings.frustumCulling)   visible = frustum.cull(visible, cullCamera);   // [C]
if (settings.occlusionCulling) visible = occlusion.cull(visible);             // [C]
TerrainLOD* lod = &chunked;  if (settings.mode == Mode::Roam) lod = &roam;    // [A]/[B]
lod->update(camera, settings, visible);                       // choose detail per chunk
lod->draw(terrainShader, visible, stats);                     // draw + count triangles
if (settings.occlusionCulling) occlusion.issueQueries(grid.allChunks(), cullCamera); // [C]
debugDraw.draw(settings, grid, cullCamera, camera);           // [C]
panel.draw(settings, stats);                                  // [C]
benchmark.record(stats);                                      // [C]
```

Everything passes through **4 shared types**:

| Type | File | What it is |
|---|---|---|
| `Settings` | `src/core/Settings.h` | Everything the UI controls (toggles, sliders) |
| `Stats` | `src/core/Stats.h` | Everything we measure (FPS, triangles, culled chunks) |
| `Chunk` | `src/terrain/Chunk.h` | One 128×128-quad piece of terrain + its bounding box |
| `TerrainLOD` | `src/lod/TerrainLOD.h` | The one interface both LOD methods implement |

**These 4 headers are frozen.** Changing one needs agreement from all three of us.

### Small changes made in Phase 0 (please check you agree)

Compared to the plan's Section 3 listing:
1. `TerrainLOD::draw(...)` now takes **`Stats& stats`**. The plan says draw "also fills stats", but gave it no way to.
2. `Settings` gained **`vsync`**. The benchmark must turn V-Sync off (plan Section 7).
3. `Stats` gained **`lodUpdateMs`** (the CPU cost of `lod.update()`, plan Section 7) and `resetFrameCounters()`.
4. `main.cpp` counts the culled chunks (list size before − after), so `Frustum::cull` and `OcclusionCuller::cull` don't need a `Stats` parameter.
5. `DebugDraw::draw` also takes the **view camera** (it needs it to draw the lines on screen), and `OcclusionCuller` has an `init(grid)` to create its query objects.

## 3. What each role does next

### Role A: Engine & Chunked LOD (Mazen)
Phase 1 is **done**: `Camera`, `Shader`, terrain shaders, the real frame loop in `main.cpp`,
and brute-force drawing (every chunk at LOD 3 = ~2.1 M triangles).
Next: Chunked LOD (Phase 2), then skirts + shading (Phase 3).

**What you can use now:**

| Need | Use |
|---|---|
| Camera matrices | `camera.view()`, `camera.projection()`, `camera.viewProjection()`, `camera.position` |
| A shader | `Shader s; s.load(projectPath("shaders/x.vert"), projectPath("shaders/x.frag")); s.use(); s.setMat4("uViewProj", ...)` |
| Terrain vertex format | `TerrainVertex` + `setTerrainVertexLayout()` in `src/lod/TerrainVertex.h` |

**Terrain shader contract** (`shaders/terrain.vert/.frag`). `main.cpp` sets these before calling `lod->draw(...)`, so **ROAM gets them for free**:
`uViewProj` (mat4), `uHeightScale` (float), `uSunDirection` (vec3). The shader is already active (`use()`d) when `draw()` is called.

**How ROAM draws (for B):** make one VAO + one VBO in `build()`:
```cpp
glGenVertexArrays(1, &m_vao);  glGenBuffers(1, &m_vbo);
glBindVertexArray(m_vao);      glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(TerrainVertex), nullptr, GL_DYNAMIC_DRAW);
setTerrainVertexLayout();
```
then in `draw()` fill a `std::vector<TerrainVertex>` with 3 vertices per leaf triangle, upload with
`glBufferSubData`, and call `glDrawArrays(GL_TRIANGLES, 0, count)`. Positions are world positions:
use `grid.position(x, z)` and `grid.normal(x, z)`.

### Role B: Terrain Data & ROAM
Your files: `src/terrain/Heightmap.*`, `src/terrain/TerrainGrid.*`, `src/lod/RoamLOD.*`, `assets/heightmaps/`.

Start with these. None of them need a window:
1. `Heightmap::load()`: 8- and 16-bit PNG with stb_image. Put a test map at `assets/heightmaps/test_1025.png`; `main.cpp` loads it automatically and falls back to the sine-wave test terrain if it's missing.
2. `TerrainGrid::build()`: central-difference **normals** and **tight chunk boxes** (real min/max height). Both are marked `TODO [B]`.
3. `RoamLOD`: follow the 5-step order at the top of `RoamLOD.cpp`.

### Role C: Culling, UI & Evaluation
Your files: `src/culling/*`, `src/ui/*`, `src/bench/*`, `shaders/lines.*`.

Start with these. None of them need Phase 1:
1. `Frustum`: plane extraction + positive-vertex box test. You can unit-test it on a hand-made matrix with GLM alone.
2. `Panel`: the current panel is a **temporary minimal version**. Replace it with the full one (culling toggles, freeze, counters, benchmark buttons).
3. `OcclusionCuller`, `DebugDraw` and `Benchmark`. Phase 1 (camera + shader) is now in, so these are unblocked.
   For `lines.vert/.frag` you can copy the structure of `shaders/terrain.vert` (one `uViewProj` uniform) and output a `uColour`.
   Restore any GL state you change (colour mask, depth mask, polygon mode).

## 4. Git workflow (plan Section 6)

1. `git checkout main && git pull`
2. `git checkout -b b/heightmap` (your role letter / feature name)
3. Commit small steps. `main` must **always build and run**.
4. Push and open a **pull request** to `main`. It is reviewed by one other member, rotating A→B→C→A.
5. In the PR description, say **what changed** and **how to see it in the app**.

Don't commit `build/` or heightmaps over 20 MB (put those in `assets/heightmaps/large/`, which Git ignores, and share a link).

## 5. Code style (so all of us can explain every line)

- One idea per file. Short functions (< ~40 lines) named after what they do.
- A plain-English comment above every function; point to the plan's figures ("see Fig. 5").
- Named constants instead of magic numbers: `CHUNK_QUADS = 128`, `SKIRT_DEPTH = 20.0f`.
- No templates, no inheritance (except `TerrainLOD`), no multithreading, no clever tricks.
