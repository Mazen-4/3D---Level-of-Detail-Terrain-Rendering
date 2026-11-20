#pragma once
// =============================================================================
//  TerrainGrid  -  the terrain in world units, ready for drawing and culling:
//                  heights, normals, and the list of chunks with their boxes.
//
//  Owner: Role B.   (plan Section 4.1)
//
//  The PUBLIC functions below are the API the rest of the team calls.
//  Keep their names and meaning; the inside (private part, .cpp) is yours.
//
//  Units: 1 heightmap pixel = 1 world unit along X and Z.
//         Height (Y) = heightmap value (0..1) * heightScale.
// =============================================================================
#include <vector>
#include <glm/glm.hpp>
#include "terrain/Chunk.h"

class Heightmap;

class TerrainGrid {
public:
    static const int CHUNK_QUADS    = 128;               // quads per chunk side
    static const int CHUNK_VERTICES = CHUNK_QUADS + 1;   // 129 vertices per chunk side

    // Builds heights, normals and chunks from a heightmap.
    void build(const Heightmap& map, float heightScale);

    int size() const          { return m_size; }                        // vertices per side, e.g. 1025
    int chunksPerSide() const { return (m_size - 1) / CHUNK_QUADS; }    // e.g. 8

    float     height(int x, int z) const;     // world height at grid point (x, z), edges clamped
    glm::vec3 position(int x, int z) const;   // world position (x, height, z)
    glm::vec3 normal(int x, int z) const;     // unit surface normal at (x, z)

    // All chunks, as pointers (this is the starting "visible" list every frame).
    std::vector<Chunk*> allChunks();
    // Read-only access to all chunks (for debug drawing).
    const std::vector<Chunk>& chunks() const { return m_chunks; }
    // One chunk by its grid position.
    Chunk&       chunk(int gx, int gz)       { return m_chunks[gz * chunksPerSide() + gx]; }
    const Chunk& chunk(int gx, int gz) const { return m_chunks[gz * chunksPerSide() + gx]; }

private:
    int   m_size = 0;
    float m_heightScale = 1.0f;
    std::vector<float>     m_heights;   // m_size * m_size, index = z * m_size + x
    std::vector<glm::vec3> m_normals;   // same layout as m_heights
    std::vector<Chunk>     m_chunks;    // chunksPerSide^2, index = gz * chunksPerSide + gx
};
