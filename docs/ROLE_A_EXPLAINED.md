# Role A explained: Engine, Chunked LOD and rendering

This page answers every "**You must be able to explain**" point for Role A in the plan
(page 9), plus the questions a teacher is likely to ask. Each answer has a short version
(say this first) and a longer one (if they ask "why?"), with a link to the exact code.

Everyone on the team should be able to answer these, not only Role A (plan Section 9).

---

## 1. What does one frame do?

**Short:** move the camera → throw away chunks we can't see → pick the detail of each
remaining chunk → draw them → draw the UI.

**Longer:** see [ARCHITECTURE.md §3](ARCHITECTURE.md#3-one-frame-step-by-step). The loop is in
[main.cpp](../src/main.cpp#L249) and each line is tagged with the role that owns it.

---

## 2. VAO, VBO, EBO, and what `glDrawElements` does

**Short:**
- **VBO** (vertex buffer): GPU memory holding the vertices (position + normal).
- **EBO** (element / index buffer): GPU memory holding a list of vertex **numbers**; every 3 numbers make one triangle.
- **VAO** (vertex array): a small object that **remembers the setup**: which VBO, how to read a vertex from it, and which EBO.
- **`glDrawElements`**: "draw triangles, taking vertex numbers from the bound EBO and the vertices from the bound VAO's VBO".

**Longer:** an index buffer lets a vertex be **shared** by up to 6 triangles instead of being
stored 6 times. In our code:

| Step | Where |
|---|---|
| Create VAO + VBO, upload vertices | [`buildChunkMesh`](../src/lod/ChunkedLOD.cpp#L68) |
| Tell OpenGL the vertex layout (attribute 0 = position, 1 = normal) | [`setTerrainVertexLayout`](../src/lod/TerrainVertex.cpp#L9) |
| Create the index buffers | [`buildLodIndexBuffer`](../src/lod/ChunkedLOD.cpp#L118) |
| Bind VAO + EBO and draw | [`draw`](../src/lod/ChunkedLOD.cpp#L210) |

```cpp
glBindVertexArray(mesh.vao);                        // this chunk's vertices
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.ebo);  // changing LOD = binding another EBO
glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
```

> **Tricky question:** *"Why do you upload the index buffer with `GL_ARRAY_BUFFER`?"*
> A buffer is just memory; the binding point only matters when it is used. Binding to
> `GL_ELEMENT_ARRAY_BUFFER` while no VAO is bound is not allowed in OpenGL core, so we copy
> the data in through `GL_ARRAY_BUFFER`, and bind it as an index buffer later, in `draw()`,
> when a VAO is bound.

---

## 3. Model, view and projection

**Short:**
- **Model** matrix: moves an object from its own coordinates into the world. **We don't need one**: our terrain vertices are already stored in world coordinates (model = identity).
- **View** matrix: moves the whole world so the camera is at the origin looking down −Z. Built with `glm::lookAt(position, position + forward, up)`.
- **Projection** matrix: perspective, so far things look smaller. Built with `glm::perspective(fov, aspect, near, far)`.

The vertex shader does `gl_Position = uViewProj * vec4(aPosition, 1.0)` with
`uViewProj = projection × view` ([terrain.vert](../shaders/terrain.vert#L18)).

**Why projection × view and not view × projection?** Matrices apply right to left: first
view (world → camera), then projection (camera → screen). See
[`Camera::viewProjection`](../src/core/Camera.cpp#L101).

---

## 4. How does the camera work?

**Short:** a position plus two angles. **Yaw** turns left/right, **pitch** looks up/down.
From the two angles we compute the forward direction:

```text
forward = ( cos(yaw)·cos(pitch),  sin(pitch),  sin(yaw)·cos(pitch) )
```

W/S move along forward, A/D along "right" (= forward × up), E/Q straight up/down. Speed is
multiplied by `dt` (seconds since the last frame), so the camera moves at the same speed at
30 FPS and at 300 FPS. Pitch is clamped to ±89° because `lookAt` breaks when looking straight up.
Code: [Camera.cpp](../src/core/Camera.cpp#L30).

---

## 5. What is a shader, and what does the Shader class do?

**Short:** two small programs that run **on the GPU**. The **vertex shader** runs once per vertex and
says where it is on screen. The **fragment shader** runs once per pixel and says what colour it is.
The Shader class reads both files, **compiles** each, and **links** them into one program
([`Shader::load`](../src/core/Shader.cpp#L49)). If there is a typo, it prints the GPU's error message.

**Uniforms** are values the CPU sends to the shader that stay the same for a whole draw call
(camera matrix, sun direction, fog distances). They are set by name with `setMat4`, `setVec3` and so on.

---

## 6. How does a stride build a coarser index buffer?

**Short:** to draw a chunk coarser we **skip vertices**. With stride 2 we only use every
2nd vertex in each direction, so each quad is 2×2 times bigger and there are 4× fewer triangles.

**Example:** a tiny chunk of 4×4 quads (5×5 vertices, numbered `z * 5 + x`):

```text
 stride 1 (finest): 16 quads            stride 2: 4 quads (uses only the marked vertices)
  0  1  2  3  4                          [0]  1 [2]  3 [4]
  5  6  7  8  9                           5   6  7   8  9
 10 11 12 13 14                         [10] 11 [12] 13 [14]
 15 16 17 18 19                          15  16 17  18 19
 20 21 22 23 24                         [20] 21 [22] 23 [24]
```

With stride 2 the first quad is `a=0, b=2, c=10, d=12` → triangles `(0,10,2)` and `(2,10,12)`.
The vertex buffer **doesn't change**; only the list of numbers does.

Our real chunks have 129×129 vertices, and the strides are 1, 2, 4, 8:

| LOD | Stride | Quads per side | Terrain triangles | + skirt triangles | Total |
|---|---|---|---|---|---|
| 3 (finest) | 1 | 128 | 32,768 | 1,024 | 33,792 |
| 2 | 2 | 64 | 8,192 | 512 | 8,704 |
| 1 | 4 | 32 | 2,048 | 256 | 2,304 |
| 0 (coarsest) | 8 | 16 | 512 | 128 | 640 |

Code: [`buildLodIndexBuffer`](../src/lod/ChunkedLOD.cpp#L118), strides in
[`LOD_STRIDE`](../src/lod/ChunkedLOD.cpp#L22).

> **Why maps are 2ⁿ+1:** 128 = 2⁷ quads per chunk, so strides 1, 2, 4 and 8 all divide 128
> exactly and always land on real vertices.

---

## 7. Why can ONE index buffer serve EVERY chunk?

**Short:** indices are positions **inside a chunk's own vertex buffer**, and every chunk stores its
vertices in the **same layout** (vertex (x, z) at position `z * 129 + x`). So "every 4th vertex" is the
same list of numbers for any chunk. We build **4 index buffers for the whole terrain**, not 4 per chunk.

**What it saves:** 64 chunks × 4 LODs would be 256 index buffers. We have 4. Changing a chunk's LOD costs
nothing: we just **bind a different EBO** before drawing it.

---

## 8. How does each chunk choose its LOD?

**Short:** by its distance to the camera; detail halves every time the distance doubles.

```text
D = lodDistance × 128          (the "LOD Distance" slider × chunk size)

distance <  1·D  → LOD 3  (red in "LOD colours")
distance <  2·D  → LOD 2  (green)
distance <  4·D  → LOD 1  (blue)
otherwise        → LOD 0  (purple)
```

- **Distance** is from the camera to the **closest point of the chunk's bounding box**. We clamp the camera position into the box to get that point ([`distanceToBox`](../src/lod/ChunkedLOD.cpp#L179)). Using the closest point, not the centre, means a big chunk right under you is never treated as far away.
- **Max Triangles** budget: if the chosen LOD has more triangles than the slider allows, step down one level until it fits. LOD 0 is always allowed, so every chunk is still drawn.
- **Enable LOD** off: every chunk gets LOD 3. That is our **brute-force baseline**, so there is no separate renderer for it.

Code: [`chooseChunkLod`](../src/lod/ChunkedLOD.cpp#L186).

**Why far away can be coarse:** a far triangle covers only a few pixels on screen, so removing
vertices there changes almost nothing you can see.

---

## 9. Where do cracks come from, and why do skirts hide them?

**Short:** two neighbouring chunks at different LODs don't agree on the height of their shared edge.
The fine chunk follows every bump; the coarse one draws a straight line over it. A thin gap opens and
the sky shows through. A **skirt** is a vertical strip hanging down from every chunk edge; it fills the gap.

```text
  side view of a shared edge

  LOD 3 edge:   /\/\/\          ← follows the bumps
  LOD 1 edge:  ‾‾‾‾‾‾‾‾         ← skips them  → gap between the two lines
  skirt:        ||||||          ← hangs down 20 units from each edge and covers the gap
```

**How we built them:**
1. Each chunk's vertex buffer gets **4 × 129 extra vertices**: a copy of every edge vertex, moved down by
   [`SKIRT_DEPTH`](../src/lod/ChunkedLOD.cpp#L29) = 20 units ([`buildChunkMesh`](../src/lod/ChunkedLOD.cpp#L68)).
2. Each LOD's index buffer adds a strip of triangles joining each edge vertex to its copy below,
   **with the same stride as that LOD**, so it lines up with that LOD's edge.

**Why skirts and not "stitching"?** Skirts need **no knowledge of the neighbours**: every chunk
handles itself. Stitching would need different index buffers for every combination of neighbour LODs
(16 variants per LOD). That's harder, so it's future work.

**Proof that it works:** we looked straight down at borders between LOD 0 and LOD 1 chunks and counted
sky-coloured pixels: **328 without skirts, 0 with skirts.**

**Why skirts are off when LOD is off:** all chunks are LOD 3 then, so edges match exactly and there
are no cracks. Skipping them keeps the brute-force count exactly 64 × 32,768 = 2,097,152
([`m_drawSkirts`](../src/lod/ChunkedLOD.cpp#L174)).

---

## 10. Lambert lighting

**Short:** a surface is brightest when it faces the sun and dark when it faces away.

```glsl
diffuse = max(dot(N, L), 0.0);          // N = surface normal, L = direction to the sun
colour  = base * (ambient + (1.0 - ambient) * diffuse);
```

`dot(N, L)` is the cosine of the angle between them: 1 when the sun is straight above the surface,
0 when it's side-on. `max(…, 0)` stops faces turned away from the sun getting negative light.
**Ambient** (0.35) is a little light everywhere, so shadowed sides aren't pure black.
Code: [terrain.frag](../shaders/terrain.frag#L67).

---

## 11. Fog

**Short:** the further a pixel is from the camera, the more we blend its colour towards the sky colour.

```glsl
fog    = clamp((distance - fogStart) / (fogEnd - fogStart), 0.0, 1.0);   // 0 = none, 1 = full
colour = mix(litColour, skyColour, fog);
```

It looks realistic **and** it hides the far LOD switches ("popping"), because far chunks are mostly
fog-coloured when they change detail. Fog is turned off in the "LOD colours" view so the colours
stay readable. Code: [terrain.frag](../shaders/terrain.frag#L73).

---

## 12. Colour by height and slope

**Short:** no textures, just a few `mix()` calls. Height 0..1 picks
**water → sand → grass → rock → snow**, each blended smoothly into the next with `smoothstep`.
Then **steep slopes become rock**: `normal.y` is 1 on flat ground and near 0 on a cliff.
Code: [`terrainColour`](../shaders/terrain.frag#L42).

---

## 13. Debug views

| View | How |
|---|---|
| **Wireframe** | `glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)` in [`beginFrame`](../src/main.cpp#L162); the UI is always switched back to fill mode |
| **LOD colours** | `uDebugMode = 1`, and `ChunkedLOD::draw` sets `uLod` per chunk; the shader picks red/green/blue/purple |

---

## 14. Measured numbers (1025 × 1025 test terrain, RTX 3060)

| Configuration | Triangles | Draw calls |
|---|---|---|
| Brute force (LOD off) | 2,097,152 | 64 |
| Chunked LOD, default view and sliders | 641,536 (10 chunks at LOD 3, 28 at LOD 2, 26 at LOD 1) | 64 |
| Max Triangles = 2,048 | 147,456 (every chunk at LOD 1, skirts included) | 64 |

**Speed (preliminary: default camera, V-Sync off, average of 300 frames, no culling yet):**

| Map | Brute force | Chunked LOD | Speed-up |
|---|---|---|---|
| 1025 × 1025 (64 chunks) | 2.10 M triangles · 0.67 ms · ~1,500 FPS | 0.64 M · 0.30 ms · ~3,360 FPS | 2.2× |
| 2049 × 2049 (256 chunks) | 8.39 M triangles · 2.13 ms · ~470 FPS | 0.92 M · 0.41 ms · ~2,460 FPS | 5.2× |

- The **bigger the map, the bigger the win**: brute force grows with the map area, while Chunked LOD mostly grows with what is near the camera.
- Chunked LOD's **CPU cost is ~0.001 ms per frame** (`lod.update`): just a distance and a few comparisons per chunk. That is why it beats ROAM on modern GPUs (plan Section 9).
- Rebuilding everything after a **Height Scale** change takes ~27 ms (1025) / ~115 ms (2049), only when the slider moves.

Culling (Role C) will cut the draw calls further; the benchmark (Role C) will give the final numbers
for the report.

---

## 15. What we skipped on purpose (future work)

| Skipped | Why | Effect |
|---|---|---|
| **Geomorphing** | Needs per-vertex blending between two LODs | A chunk visibly "pops" when its LOD changes; fog hides most of it |
| **Stitched edges** | 16 index-buffer variants per LOD, depending on neighbours | Skirts do the same job with no neighbour knowledge |
| **Model matrix** | Vertices are already in world space | One less matrix to explain |
| **Back-face culling** | Skirts would need careful winding | Small GPU cost; simpler code |

---

## 16. Quick-fire questions

- **Why `unsigned int` indices?** A chunk has 17,157 vertices (16,641 grid + 516 skirt), which would fit in 16-bit indices too. We use 32-bit for simplicity; the memory difference is tiny.
- **Why one draw call per chunk?** Each chunk has its own VBO and may have its own LOD. 64 draw calls is very cheap, and culling lowers it further.
- **How much GPU memory?** Per chunk: 17,157 vertices × 24 bytes ≈ 0.4 MB, so 64 chunks ≈ 26 MB. The 4 shared index buffers add about 0.6 MB.
- **What is V-Sync and why switch it off?** V-Sync waits for the monitor (usually 60 Hz), so FPS can't go higher. For benchmarks we switch it off to see the real speed.
- **Why is the camera's dt important?** Movement = speed × dt, so it doesn't depend on the frame rate.
