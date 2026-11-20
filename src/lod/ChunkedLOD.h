#pragma once
// =============================================================================
//  ChunkedLOD  -  LOD method 1 (plan Section 4.2, Fig. 2 and Fig. 3).
//
//  Owner: Role A.
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
//
//  Choosing the LOD (plan Section 4.2, Fig. 2):
//      D = lodDistance x chunkSize        (the "LOD Distance" slider)
//      distance to chunk <  1 D   -> LOD 3
//      distance to chunk <  2 D   -> LOD 2
//      distance to chunk <  4 D   -> LOD 1
//      otherwise                  -> LOD 0
//  then, if that LOD has more triangles than "Max Triangles", step down a level.
//
//  Status: Phase 1 = buffers + brute force        (done)
//          Phase 2 = LOD by distance + budget     (done)
//          Phase 3 = skirts against cracks
// =============================================================================
#include <vector>
#include <glm/glm.hpp>
#include "lod/TerrainLOD.h"

class ChunkedLOD : public TerrainLOD {
public:
    static const int LOD_COUNT   = 4;   // LOD 0 .. LOD 3
    static const int FINEST_LOD  = 3;

    void build(const TerrainGrid& grid) override;
    void update(const Camera& camera, const Settings& settings,
                const std::vector<Chunk*>& visible) override;
    void draw(Shader& shader, const std::vector<Chunk*>& visible, Stats& stats) override;

private:
    // The GPU objects of one chunk: its vertex array (VAO) and vertex buffer (VBO).
    struct ChunkMesh {
        unsigned int vao = 0;
        unsigned int vbo = 0;
    };

    // One shared index buffer (EBO) per LOD, used by EVERY chunk.
    struct LodIndexBuffer {
        unsigned int ebo = 0;
        int indexCount   = 0;   // 3 indices per triangle
    };

    void buildChunkMesh(const TerrainGrid& grid, const Chunk& chunk, ChunkMesh& mesh);
    void buildLodIndexBuffer(int lod);

    int chooseChunkLod(const Chunk& chunk, const glm::vec3& cameraPosition,
                       const Settings& settings) const;
    int trianglesAtLod(int lod) const { return m_lodBuffers[lod].indexCount / 3; }

    int                    m_chunksPerSide = 0;
    std::vector<ChunkMesh> m_meshes;                 // index = gz * chunksPerSide + gx
    LodIndexBuffer         m_lodBuffers[LOD_COUNT];  // [0] coarsest .. [3] finest
};
