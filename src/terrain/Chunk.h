#pragma once
// =============================================================================
//  Chunk  -  one square piece of the terrain (128 x 128 quads = 129 x 129 vertices).
//
//  SHARED HEADER (agreed by the whole team, plan Section 3).
//
//  A 1025 x 1025 heightmap is cut into 8 x 8 = 64 chunks (plan Fig. in 4.1).
//  Culling only ever looks at the chunk's bounding box, never at its triangles.
//
//  Who creates it:  TerrainGrid (Role B)
//  Who uses it:     culling (Role C) reads the box,
//                   ChunkedLOD (Role A) writes 'lod',
//                   OcclusionCuller (Role C) writes 'occludedLastFrame'.
// =============================================================================
#include <glm/glm.hpp>

struct Chunk {
    int gx = 0, gz = 0;          // position in the chunk grid: (0,0) .. (7,7) for a 1025 map

    glm::vec3 boxMin{0.0f};      // bounding box corner with the smallest x, y, z
    glm::vec3 boxMax{0.0f};      // bounding box corner with the largest  x, y, z

    int  lod = 3;                    // 3 = finest .. 0 = coarsest (chosen by Chunked LOD only)
    bool occludedLastFrame = false;  // true = last frame's occlusion query said "hidden"
};
