# Architecture: how the program fits together

This page explains the whole program in one sitting: what happens at start-up, what
happens every frame, which file does what, and the few rules that keep the three
parts working together. Links go straight to the code.

---

## 1. The whole program on one page

```text
 ONCE AT START-UP                                                      (main.cpp: main)
 ┌─────────────┐   ┌───────────────┐   ┌──────────────────────┐   ┌─────────────────────┐
 │ Heightmap   │──►│ TerrainGrid   │──►│ ChunkedLOD::build    │   │ RoamLOD::build      │
 │ PNG → 0..1  │   │ heights,      │   │ 1 VBO per chunk +    │   │ variance trees      │
 │       [B]   │   │ normals,      │   │ 4 shared index bufs  │   │ (precomputed)  [B]  │
 └─────────────┘   │ 8×8 chunks,   │   │                 [A]  │   └─────────────────────┘
                   │ boxes    [B]  │   └──────────────────────┘
                   └───────────────┘

 EVERY FRAME                                                     (main.cpp: the while loop)
 ┌────────┐  ┌───────────┐  ┌─────────────┐  ┌──────────────────┐  ┌──────┐  ┌──────────┐
 │ Camera │─►│ Frustum   │─►│ Occlusion   │─►│ LOD update       │─►│ Draw │─►│ Debug +  │
 │ move   │  │ cull      │  │ cull        │  │ Chunked or ROAM  │  │      │  │ UI panel │
 │   [A]  │  │      [C]  │  │ (last frame)│  │      [A] / [B]   │  │      │  │     [C]  │
 └────────┘  └───────────┘  │        [C]  │  └──────────────────┘  └──────┘  └──────────┘
                            └─────────────┘           │
                                   ▲                  ▼
                                   └── occlusion queries issued after drawing [C]
```

[A] = Engine & Chunked LOD · [B] = Terrain Data & ROAM · [C] = Culling, UI & Evaluation

---

## 2. Start-up, step by step

Everything happens in [`main()`](../src/main.cpp#L210):

| Step | Code | What it does |
|---|---|---|
| 1 | [`createWindow()`](../src/main.cpp#L67) | Opens a window with an **OpenGL 3.3 Core** context and loads OpenGL functions with GLAD |
| 2 | [`startImGui()`](../src/main.cpp#L98) | Connects Dear ImGui (the control panel library) to the window |
| 3 | [`buildTerrain()`](../src/main.cpp#L114) | Loads the heightmap PNG (or the built-in test terrain) and builds the `TerrainGrid` |
| 4 | [`placeCameraOverTerrain()`](../src/main.cpp#L125) | Puts the camera above the near edge, looking across the map |
| 5 | `terrainShader.load(...)` | Compiles `shaders/terrain.vert` + `terrain.frag` |
| 6 | `chunked.build(grid)`, `roam.build(grid)` | Each LOD method prepares its data (GPU buffers, variance trees) |
| 7 | `occlusion.init(grid)`, `debugDraw.init()` | Culling and debug helpers create their GPU objects |

---

## 3. One frame, step by step

The [frame loop](../src/main.cpp#L249) is the plan's Section 3, line for line:

| # | Code | Owner | What it does |
|---|---|---|---|
| 1 | `updateFrameTiming` | A | Measures the frame time → `stats.frameMs`, `stats.fps` |
| 2 | `beginFrame` | A | Viewport + aspect ratio (window may be resized), V-Sync, clear screen, wireframe on/off |
| 3 | `camera.update(window, dt)` | A | WASD + mouse. Skipped while the mouse is over the UI panel |
| 4 | `cullCamera = camera` | C | Unless **Freeze culling** is on: then culling keeps using the old camera, so you can fly out and look at it |
| 5 | `visible = grid.allChunks()` | B | Start with all 64 chunks |
| 6 | `frustum.cull(...)` | C | Remove chunks outside the screen |
| 7 | `occlusion.cull(...)` | C | Remove chunks that were hidden **last frame** |
| 8 | `lod->update(...)` | A / B | Chunked: pick an LOD per chunk. ROAM: rebuild the adaptive mesh. Timed → `stats.lodUpdateMs` |
| 9 | `setTerrainUniforms` + `lod->draw(...)` | A / B | Draw the visible chunks; count triangles and draw calls |
| 10 | `occlusion.issueQueries(...)` | C | Ask the GPU which chunk boxes are hidden (answer read next frame) |
| 11 | `debugDraw.draw`, `panel.draw` | C | Debug lines and the control panel |
| 12 | `benchmark.record(stats)` | C | Write this frame's numbers to the CSV (only during a benchmark) |
| 13 | `glfwSwapBuffers` | A | Show the finished picture |

**Why the order matters:** culling (6–7) must happen **before** the LOD update, so the
LOD methods only spend time on chunks that will be drawn. Occlusion queries (10) must
happen **after** the terrain is drawn, because they test against the terrain's depth buffer.

---

## 4. The 4 shared types (the "contract")

All three parts talk to each other only through these. They are frozen: change them only if all three agree.

| Type | File | Written by | Read by |
|---|---|---|---|
| `Settings` | [Settings.h](../src/core/Settings.h) | UI panel [C] | everyone |
| `Stats` | [Stats.h](../src/core/Stats.h) | `main.cpp` [A], LOD `draw()` [A/B] | panel, benchmark [C] |
| `Chunk` | [Chunk.h](../src/terrain/Chunk.h) | TerrainGrid [B] creates; ChunkedLOD [A] sets `lod`; OcclusionCuller [C] sets `occludedLastFrame` | everyone |
| `TerrainLOD` | [TerrainLOD.h](../src/lod/TerrainLOD.h) | interface | implemented by ChunkedLOD [A] and RoamLOD [B] |

`TerrainLOD` is the **only** interface (abstract class) in the project. Because both LOD
methods have the same three functions (`build`, `update`, `draw`), `main.cpp` switches
between them with one `if`:

```cpp
TerrainLOD* lod = &chunked;
if (settings.mode == Mode::Roam)
    lod = &roam;
```

---

## 5. Files and owners

| File | Owner | One-line job |
|---|---|---|
| [main.cpp](../src/main.cpp) | A | Window, start-up, frame loop |
| [core/Camera](../src/core/Camera.cpp) | A | Fly camera → view and projection matrices |
| [core/Shader](../src/core/Shader.cpp) | A | Load/compile/link GLSL, set uniforms |
| [core/Paths.h](../src/core/Paths.h) | A | Finds `shaders/` and `assets/` from any working folder |
| [core/Settings.h](../src/core/Settings.h), [core/Stats.h](../src/core/Stats.h) | all | Shared types |
| [terrain/Heightmap](../src/terrain/Heightmap.cpp) | B | PNG → grid of heights 0..1 |
| [terrain/TerrainGrid](../src/terrain/TerrainGrid.cpp) | B | World heights, normals, chunks + boxes |
| [terrain/Chunk.h](../src/terrain/Chunk.h) | all | Shared type |
| [lod/TerrainLOD.h](../src/lod/TerrainLOD.h) | all | The interface |
| [lod/TerrainVertex](../src/lod/TerrainVertex.h) | A | The vertex format both LOD methods use |
| [lod/ChunkedLOD](../src/lod/ChunkedLOD.cpp) | A | LOD method 1 |
| [lod/RoamLOD](../src/lod/RoamLOD.cpp) | B | LOD method 2 |
| [culling/Frustum](../src/culling/Frustum.cpp) | C | Off-screen chunks |
| [culling/OcclusionCuller](../src/culling/OcclusionCuller.cpp) | C | Chunks hidden behind terrain |
| [ui/Panel](../src/ui/Panel.cpp) | C | Control panel |
| [ui/DebugDraw](../src/ui/DebugDraw.cpp) | C | Chunk boxes, frozen frustum lines |
| [bench/Benchmark](../src/bench/Benchmark.cpp) | C | Camera path replay + CSV |
| [shaders/terrain.*](../shaders/terrain.frag) | A | Terrain look |

---

## 6. Rules that keep the parts from breaking each other

1. **Positions are in world units.** 1 heightmap pixel = 1 unit along X and Z; height = value × `heightScale`.
   Terrain vertices are stored in world space, so there is no model matrix.
2. **Chunk index = `gz * chunksPerSide + gx`**, everywhere (grid, ChunkedLOD meshes, occlusion queries).
3. **The terrain shader is already active when `lod->draw()` is called**, with all frame uniforms set
   (see [`setTerrainUniforms`](../src/main.cpp#L179)). A LOD method only sets `uLod`.
4. **Restore OpenGL state you change.** If you turn off depth writes, colour writes or change the
   polygon mode, turn them back before returning. The next module expects the defaults:
   depth test on, depth and colour writes on, fill mode (unless wireframe), no VAO bound.
5. **Count what you draw:** every `glDraw...` of terrain adds to `stats.triangles` and `stats.drawCalls`.
6. **Never read the GPU's answer in the same frame** (occlusion queries); it would make the CPU wait.

---

## 7. Where do I add…?

| I want to… | Put it in |
|---|---|
| a new checkbox or slider | a field in `Settings` (ask the team) + a widget in `Panel.cpp` |
| a new number in the stats box | a field in `Stats` (ask the team), fill it where it is measured, show it in `Panel.cpp` |
| a new uniform for the terrain look | `terrain.frag` + `setTerrainUniforms` in `main.cpp` |
| a new `.cpp` file | anywhere under `src/`. CMake picks it up after re-running `cmake -S . -B build` |
