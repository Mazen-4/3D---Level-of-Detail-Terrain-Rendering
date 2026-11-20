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
Phase 2 is **done**: each chunk picks its LOD from its distance to the camera; the
**Enable LOD**, **LOD Distance** and **Max Triangles** controls work. `chunk->lod` is now
set every frame, so Role C can show it in debug views.
Phase 3 is **done**: skirts hide the cracks between LODs, and the shader has height/slope
colours, a sun, fog, and the **LOD colours** debug view.
Phase 4 is **done**: [ARCHITECTURE.md](ARCHITECTURE.md), [ROLE_A_EXPLAINED.md](ROLE_A_EXPLAINED.md)
and [CONTRIBUTING.md](CONTRIBUTING.md).
Also done: the **Height Scale** slider rebuilds the terrain, and a draft of A's report section is in
[report/ROLE_A_SECTION_DRAFT.md](report/ROLE_A_SECTION_DRAFT.md).
Next for A: review B's and C's pull requests, fix bugs found in benchmarks, and finish the report
section with C's final numbers and screenshots on the real heightmap.

**What you can use now:**

| Need | Use |
|---|---|
| Camera matrices | `camera.view()`, `camera.projection()`, `camera.viewProjection()`, `camera.position` |
| A shader | `Shader s; s.load(projectPath("shaders/x.vert"), projectPath("shaders/x.frag")); s.use(); s.setMat4("uViewProj", ...)` |
| Terrain vertex format | `TerrainVertex` + `setTerrainVertexLayout()` in `src/lod/TerrainVertex.h` |

**Terrain shader contract** (`shaders/terrain.vert/.frag`). `main.cpp` sets all of these before calling `lod->draw(...)`, so **ROAM gets them for free**:
`uViewProj`, `uHeightScale`, `uSunDirection`, `uCameraPos`, `uFogColour`, `uFogStart`, `uFogEnd`, `uDebugMode`, and `uLod` (default 3).
The shader is already active (`use()`d) when `draw()` is called.
The only one a LOD method sets itself is **`uLod`**, which picks the colour in the "LOD colours" view.
ChunkedLOD sets it per chunk. ROAM can leave it at the default or call `shader.setInt("uLod", ...)` with any 0–3 value it likes.

**How ROAM draws (for B):** make one VAO + one VBO in `build()`:
```cpp
glGenVertexArrays(1, &m_vao);  glGenBuffers(1, &m_vbo);
glBindVertexArray(m_vao);      glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(TerrainVertex), nullptr, GL_DYNAMIC_DRAW);
setTerrainVertexLayout();
```
**Note:** `build()` is called again whenever the **Height Scale** slider moves (main.cpp rebuilds the
grid and both LOD methods). Delete or reuse the VAO/VBO and other data from the previous call;
`ChunkedLOD::releaseGpuBuffers` shows how. `TerrainGrid::build` is also called again, with the same heightmap.

Then in `draw()` fill a `std::vector<TerrainVertex>` with 3 vertices per leaf triangle, upload with
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

## 4. Git workflow and code style

Short version: branch `<role>/<feature>` from `main`, small commits, pull request into `main`,
reviewed by the next person in the rotation A→B→C→A. The full steps, the PR checklist and the
code style are in **[CONTRIBUTING.md](CONTRIBUTING.md)**.

## 5. All the docs

| Doc | Read it when |
|---|---|
| [TEAM_START_HERE.md](TEAM_START_HERE.md) (this page) | You are new, or want to know what to do next |
| [ARCHITECTURE.md](ARCHITECTURE.md) | You want to understand how the whole program fits together |
| [CONTRIBUTING.md](CONTRIBUTING.md) | You are about to commit, open a PR, or review one |
| [ROLE_A_EXPLAINED.md](ROLE_A_EXPLAINED.md) | You need to explain the engine, Chunked LOD or shading (everyone must!) |
| [report/ROLE_A_SECTION_DRAFT.md](report/ROLE_A_SECTION_DRAFT.md) | Role A's report section, ready to paste (needs C's final numbers) |
