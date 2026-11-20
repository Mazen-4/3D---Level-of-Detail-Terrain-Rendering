# Report draft: Rendering Pipeline, Chunked LOD and Shading (Role A)

> **How to use this draft.** This is Role A's section of the final report (plan Section 5,
> "Report section: Pipeline, Chunked LOD, rendering"), written as report text ready to paste
> into the team document. Things to do before submitting are marked **[TODO]**:
> - replace the preliminary numbers with Role C's benchmark results;
> - take the listed screenshots on the real heightmap, once Role B's loader and normals are merged;
> - renumber sections and figures to match the full report.

---

## 1. Rendering pipeline

The terrain viewer is a C++17 application built directly on **OpenGL 3.3 Core**, with small helper
libraries for tasks outside the scope of the project: GLFW (window and input), GLAD (OpenGL
function loading), GLM (vector and matrix mathematics), Dear ImGui (control panel) and stb_image
(heightmap loading). Only vertex and fragment shaders are used; tessellation and compute shaders
were deliberately avoided to keep every stage easy to explain and debug.

The program has two phases. **At start-up** the heightmap is converted into a `TerrainGrid` of
world-space heights and normals, split into square chunks of 128 × 128 quads, and each LOD method
builds its data once (Chunked LOD: GPU buffers; ROAM: variance trees). **Every frame** then runs the
same fixed sequence:

1. update the fly camera from keyboard and mouse input;
2. start from the list of all chunks and remove those outside the view frustum;
3. remove chunks found to be occluded in the previous frame;
4. let the active LOD method choose the detail of each remaining chunk;
5. draw the chunks with the terrain shader, counting triangles and draw calls;
6. issue occlusion queries for the next frame, then draw debug lines and the control panel.

Both LOD methods implement one small interface, `TerrainLOD`, with three operations
(`build`, `update`, `draw`). The frame loop therefore does not depend on which method is active,
and switching between Chunked LOD and ROAM at runtime is a single pointer assignment. Culling runs
**before** the LOD update, so neither method spends time on chunks that will not be drawn.

**[TODO] Figure X.1:** the pipeline diagram (plan Fig. 1).

## 2. Terrain data on the GPU

The heightmap has 2ⁿ + 1 samples per side (1025 or 2049), so it divides into chunks of exactly
128 quads (129 × 129 vertices). Neighbouring chunks share their border row of vertices. A
1025 × 1025 map gives 8 × 8 = 64 chunks; a 2049 × 2049 map gives 16 × 16 = 256 chunks.

Each vertex stores a position and a normal (24 bytes). Positions are stored directly in world
coordinates (one heightmap sample = one world unit horizontally), so no per-chunk model matrix is
needed, and the vertex shader only multiplies by the combined view-projection matrix. Each chunk
owns one vertex array object (VAO) and one static vertex buffer (VBO) of 17,157 vertices: the
16,641 grid vertices plus 516 skirt vertices (Section 4). This is about 0.4 MB per chunk, or 26 MB
for the 1025 map.

## 3. Chunked level of detail

### 3.1 Shared index buffers

Chunked LOD (Ulrich, 2002) keeps every chunk at full resolution in its vertex buffer and draws it
more coarsely by **skipping vertices with a stride**. Four levels are used:

| LOD | Stride | Grid drawn | Triangles per chunk |
|---|---|---|---|
| 3 (finest) | 1 | 128 × 128 | 32,768 |
| 2 | 2 | 64 × 64 | 8,192 |
| 1 | 4 | 32 × 32 | 2,048 |
| 0 (coarsest) | 8 | 16 × 16 | 512 |

Because every chunk stores its vertices in the same layout (vertex (x, z) at index 129·z + x),
the index list for a given stride is identical for every chunk. The whole terrain therefore needs
only **four index buffers**, built once at start-up, instead of one per chunk and level. Changing a
chunk's LOD costs nothing: the draw call simply binds a different index buffer. All terrain data
stays on the GPU; the CPU only chooses an index buffer per chunk.

### 3.2 Choosing the level

Each frame, every visible chunk selects its level from the distance d between the camera and the
**closest point of the chunk's bounding box**:

```text
D = lodDistance × chunkSize
LOD = 3 if d < D,   2 if d < 2D,   1 if d < 4D,   0 otherwise
```

The thresholds double for each coarser level, matching the halving of triangle density, so the
projected size of a triangle stays roughly constant. Using the closest point rather than the chunk
centre prevents a large chunk directly under the camera from being treated as distant. The
*LOD Distance* slider scales D. The *Max Triangles* slider sets a per-chunk budget: if the chosen
level exceeds it, the chunk steps down one level at a time, with LOD 0 always permitted. Disabling
LOD forces every chunk to LOD 3, which gives the **brute-force baseline** without a separate renderer.

The cost of this selection is a clamp, a vector length and a few comparisons per chunk. We measured
it at about 0.001 ms per frame, which is negligible.

## 4. Cracks and skirts

When neighbouring chunks use different levels, their shared edge is sampled differently: the finer
chunk follows every height sample while the coarser one interpolates across several. The two edges
no longer coincide, and thin gaps (cracks) appear through which the background is visible.

We hide cracks with **skirts**. Each chunk's vertex buffer is extended with a copy of its four edges,
lowered by a fixed depth (20 world units), and each LOD's index buffer appends a strip of triangles
connecting every edge vertex to its lowered copy, using the same stride as that level. The skirt is
a vertical curtain under the edge that fills any gap to the neighbour. It is lit with the edge's
normal, so it is indistinguishable from the terrain. Skirts need no knowledge of the neighbouring
chunks' levels, which is why we chose them over stitched edge index buffers. Those would need a
separate index-buffer variant for each combination of neighbour levels.

To verify the method, we rendered a top-down view across borders between LOD 0 and LOD 1 chunks and
counted background-coloured pixels, which can only come from cracks in that view:
**328 pixels without skirts and 0 with skirts.** Skirts add 128–1,024 triangles per chunk
depending on the level. They are not drawn when LOD
is disabled, since all chunks then share the same level and cannot crack.

**[TODO] Figure X.2:** close-up of a LOD border in wireframe, with skirts visible below the edge.

## 5. Shading

The terrain is shaded procedurally in the fragment shader, without textures:

- **Colour by height and slope.** The normalised height selects water, sand, grass, rock and snow,
  blended with `smoothstep` to avoid hard bands. Surfaces whose normal is far from vertical
  (steep slopes) are blended towards rock.
- **Lighting.** A single directional sun with Lambertian diffuse reflection,
  `diffuse = max(N · L, 0)`, plus a constant ambient term of 0.35, so faces turned away from the
  sun are not black.
- **Distance fog.** The lit colour is blended linearly towards the sky colour between a start and an
  end distance (0.6 and 1.6 times the terrain width). Besides adding depth, fog hides most of the
  "popping" when far chunks change level.

Two debug views support the evaluation. **Wireframe** mode (`glPolygonMode`) shows the triangle
density. **LOD colours** mode replaces the shading with one colour per level (red, green, blue,
purple from finest to coarsest), so the distance rings of Section 3.2 are visible on screen.

**[TODO] Figure X.3:** the same view in normal shading, LOD colours, and wireframe.

## 6. Preliminary results

**[TODO] Replace with Role C's benchmark results** (recorded camera path, V-Sync off, averages and
worst 1 % frame times). The numbers below are from a single fixed camera position on an RTX 3060
laptop GPU, with V-Sync off, averaged over 300 frames, and with culling not yet enabled.

| Map | Configuration | Triangles / frame | Frame time | FPS |
|---|---|---|---|---|
| 1025 × 1025 | Brute force (LOD off) | 2,097,152 | 0.67 ms | ~1,500 |
| 1025 × 1025 | Chunked LOD | 641,536 | 0.30 ms | ~3,360 |
| 2049 × 2049 | Brute force (LOD off) | 8,388,608 | 2.13 ms | ~470 |
| 2049 × 2049 | Chunked LOD | 916,480 | 0.41 ms | ~2,460 |

Chunked LOD reduces the triangle count by a factor of 3.3 on the 1025 map and 9.2 on the 2049 map,
and the frame time by 2.2× and 5.2×. The benefit grows with terrain size because brute-force cost
grows with the whole terrain area, while Chunked LOD's cost depends mainly on the area near the
camera. The CPU cost of choosing the levels is negligible, which is the main practical advantage of
Chunked LOD over ROAM's per-frame CPU tessellation (compared in the ROAM section).

## 7. Limitations and future work

- **Popping.** A chunk changes level instantly, which can be visible near the thresholds. Fog
  hides most of it at distance. **Geomorphing**, blending vertex heights between two levels over
  a transition band, would remove it.
- **Distance-only metric.** The level depends only on distance, not on how rough the chunk is. Flat
  chunks receive as much detail as mountains at the same distance. Ulrich (2002) chooses levels by
  projected geometric error in screen space, which would put triangles where the terrain needs them.
- **Skirt depth.** The fixed 20-unit depth is enough for our maps, but very steep terrain at large
  stride could produce cracks deeper than the skirt. The depth could instead be computed per chunk
  from the largest height difference between levels.
- **Draw calls.** One draw call per visible chunk (64 at most here) is cheap, but for much larger
  terrains instancing or multi-draw indirect would reduce the overhead.
- **Very large terrains.** All chunks are resident in GPU memory. Streaming chunks from disk would be
  needed beyond a few thousand chunks.

## References

- Ulrich, T. (2002). *Rendering Massive Terrains using Chunked Level of Detail Control.* SIGGRAPH Course Notes.
- de Boer, W. H. (2000). *Fast Terrain Rendering Using Geometrical MipMapping.*
- Duchaineau, M. et al. (1997). *ROAMing Terrain: Real-time Optimally Adapting Meshes.* IEEE Visualization.

## Screenshot checklist [TODO]

Take these on the **real heightmap**, after Role B's loader and normals are merged:

1. Normal view with everything on: FPS counter visible.
2. Same view with **LOD colours** (red/green/blue/purple rings).
3. Same view in **wireframe**, showing the density falling off with distance.
4. Close-up of a LOD border in wireframe, showing the skirt hanging below the edge.
5. Panel with **Enable LOD** off: the triangle count at ~2.1 M.
