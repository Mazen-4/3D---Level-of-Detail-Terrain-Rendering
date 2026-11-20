#pragma once
// =============================================================================
//  ChunkedLOD  -  LOD method 1 (plan Section 4.2, Fig. 2 and Fig. 3).
//
//  Owner: Role A.   (Implementation: ChunkedLOD.cpp, written in Phases 1-3.)
//
//  The idea in three lines:
//    * every chunk has ONE vertex buffer with all 129 x 129 vertices;
//    * a coarser LOD just skips vertices (stride 1, 2, 4, 8), so the index
//      buffer for "LOD k" is the SAME for every chunk -> only 4 index buffers in total;
//    * each frame we pick an LOD per chunk from its distance to the camera.
//
//      LOD 3 (finest)   stride 1   128x128 quads   32,768 triangles
//      LOD 2            stride 2    64x64  quads    8,192 triangles
//      LOD 1            stride 4    32x32  quads    2,048 triangles
//      LOD 0 (coarsest) stride 8    16x16  quads      512 triangles
// =============================================================================
#include "lod/TerrainLOD.h"

class ChunkedLOD : public TerrainLOD {
public:
    void build(const TerrainGrid& grid) override;
    void update(const Camera& camera, const Settings& settings,
                const std::vector<Chunk*>& visible) override;
    void draw(Shader& shader, const std::vector<Chunk*>& visible, Stats& stats) override;

    // Phase 1-3 will add the private members here:
    //   one VAO/VBO per chunk, the 4 shared index buffers, chooseChunkLod(), ...
};
