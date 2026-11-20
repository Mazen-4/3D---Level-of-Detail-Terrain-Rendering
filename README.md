# Level-of-Detail Terrain Rendering (ROAM & Chunked LOD)

CS427 Course Project 2. An OpenGL 3.3 terrain viewer that draws a large heightmap
smoothly by drawing detail only where the viewer can see it.

- **Two LOD methods**, switchable at runtime: Chunked LOD and ROAM
- **Frustum culling** and **occlusion culling**
- A **control panel** (Enable LOD, culling toggles, LOD Distance, Max Triangles, FPS)
- **Debug views** and a **benchmark mode**

The full design is in [`LOD Terrain - Project Plan & Team Roles.pdf`](LOD%20Terrain%20-%20Project%20Plan%20%26%20Team%20Roles.pdf).
**New to the code? Read [`docs/TEAM_START_HERE.md`](docs/TEAM_START_HERE.md) first.**

---

## 1. Build and run (Windows)

You need **CMake** (3.16 or newer) and **one** C++ compiler. All libraries are already
in `external/`, so there is nothing else to download.

Install CMake (pick one):
- `pip install cmake`, or
- the installer from <https://cmake.org/download/> (tick "Add CMake to PATH").

### Option A: MinGW g++ (MSYS2 / VS Code)
```bash
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build -j 8
build\LODTerrain.exe
```
`g++` and `mingw32-make` must be on your PATH (for MSYS2 this is the `ucrt64\bin` folder).

### Option B: Visual Studio 2022
```bash
cmake -S . -B build
cmake --build build --config Release
build\Release\LODTerrain.exe
```
Or in Visual Studio: **File → Open → Folder…**, pick this folder, choose `LODTerrain.exe` as the startup item, and press F5.

> The first build compiles GLFW and ImGui and takes about a minute. Later builds only
> recompile the files you changed.
>
> Added a new `.cpp` file? Just run the first `cmake -S . -B build ...` line again.
> Every `.cpp` under `src/` is picked up automatically.

### Tested on
Windows 11 with MinGW g++ 13.2 and with Visual Studio 2022 (MSVC). Both build with no warnings in our code.
Any GPU with **OpenGL 3.3** works (all NVIDIA, AMD and Intel GPUs from roughly 2010 on).

### Troubleshooting
| Problem | Fix |
|---|---|
| `cmake` is not recognised | Install CMake (see above). With pip you can also run `python -m cmake ...` |
| `CMAKE_CXX_COMPILER not set` | No compiler found. Use Option B (Visual Studio), or add MinGW's `bin` folder to PATH |
| "Failed to create window" | Update your GPU driver; the program needs OpenGL 3.3 |
| Laptop with two GPUs is slow | The program already asks for the powerful GPU. If it's still slow, pick it in the NVIDIA Control Panel / Windows Graphics settings |
| Linux | `sudo apt install build-essential cmake xorg-dev`, then build like Option A without `-G ...` |
| macOS | `brew install cmake`, then build like Option A without `-G ...` (OpenGL 3.3 works but is deprecated by Apple) |

---

## 2. Folder layout (who owns what)

| Folder / file | What is in it | Owner |
|---|---|---|
| `CMakeLists.txt` | Build file | A |
| `external/` | GLFW, GLAD, GLM, Dear ImGui, stb_image (do not edit) | n/a |
| `assets/heightmaps/` | Heightmap PNGs | B |
| `shaders/terrain.*` | Terrain lighting, colours, fog, LOD-colour mode | A |
| `shaders/lines.*` | Debug boxes and frustum lines | C |
| `src/main.cpp` | Window and frame loop | A |
| `src/core/` | `Camera`, `Shader`, `Paths` (A); `Settings`, `Stats` (shared, all) | A / all |
| `src/terrain/` | `Heightmap`, `TerrainGrid` (+ `Chunk` shared) | B |
| `src/lod/` | `TerrainLOD` (shared interface), `ChunkedLOD` (A), `RoamLOD` (B) | A / B |
| `src/culling/` | `Frustum`, `OcclusionCuller` | C |
| `src/ui/` | `Panel`, `DebugDraw` | C |
| `src/bench/` | `Benchmark`, `plot_results.py` | C |
| `docs/` | Explanations for the team | all |

**A** = Engine & Chunked LOD · **B** = Terrain Data & ROAM · **C** = Culling, UI & Evaluation

---

## 3. Controls

| Input | Action |
|---|---|
| Hold **right mouse button** + move | Look around |
| **W A S D** | Move forward / left / back / right |
| **Q / E** | Move down / up |
| **Shift** | Move 4× faster |
| **Esc** | Quit |

---

## 4. Project status

| Phase | Content | Status |
|---|---|---|
| 0 | Libraries, build, shared headers, stubs for every role | ✅ done |
| 1 | Window, camera, shader, real frame loop, brute-force terrain (2.1 M triangles, 64 draw calls) | ✅ done |
| 2 | Chunked LOD: 4 shared index buffers, distance LOD, budget (default view: 2.1 M → ~0.6 M triangles) | ✅ done |
| 3 | Skirts, shading, LOD colours, wireframe | ⏳ (A) |
| n/a | Heightmap loading, normals, chunk boxes, ROAM | ⏳ (B) |
| n/a | Frustum and occlusion culling, full panel, debug draw, benchmark | ⏳ (C) |
