// =============================================================================
//  TerrainGrid.cpp   -  Owner: Role B
//
//  PHASE 0 STUB:  heights and chunk positions are correct, so everyone can draw
//  and cull.  Two things are simplified on purpose and are Role B's to finish:
//    1. normals all point straight up  -> lighting looks flat
//    2. chunk boxes use the full height range (0 .. heightScale) instead of the
//       real min/max height inside each chunk -> culling still correct, but loose
// =============================================================================
#include "terrain/TerrainGrid.h"
#include "terrain/Heightmap.h"

#include <algorithm>   // std::clamp

void TerrainGrid::build(const Heightmap& map, float heightScale) {
    m_size        = map.size();
    m_heightScale = heightScale;

    // --- 1. Heights: heightmap value (0..1) times the height scale ----------
    m_heights.resize(m_size * m_size);
    for (int z = 0; z < m_size; ++z)
        for (int x = 0; x < m_size; ++x)
            m_heights[z * m_size + x] = map.value(x, z) * heightScale;

    // --- 2. Normals ----------------------------------------------------------
    // TODO [B]: central differences (plan Section 4.1):
    //   n = normalize( h(x-1,z) - h(x+1,z),  2,  h(x,z-1) - h(x,z+1) )
    m_normals.assign(m_size * m_size, glm::vec3(0.0f, 1.0f, 0.0f));

    // --- 3. Chunks and their bounding boxes ----------------------------------
    const int n = chunksPerSide();
    m_chunks.resize(n * n);
    for (int gz = 0; gz < n; ++gz) {
        for (int gx = 0; gx < n; ++gx) {
            Chunk& c = m_chunks[gz * n + gx];
            c.gx = gx;
            c.gz = gz;
            // TODO [B]: use the real min/max height of the 129 x 129 points in this chunk.
            c.boxMin = glm::vec3(gx * CHUNK_QUADS,       0.0f,        gz * CHUNK_QUADS);
            c.boxMax = glm::vec3((gx + 1) * CHUNK_QUADS, heightScale, (gz + 1) * CHUNK_QUADS);
        }
    }
}

float TerrainGrid::height(int x, int z) const {
    x = std::clamp(x, 0, m_size - 1);
    z = std::clamp(z, 0, m_size - 1);
    return m_heights[z * m_size + x];
}

glm::vec3 TerrainGrid::position(int x, int z) const {
    return glm::vec3((float)x, height(x, z), (float)z);
}

glm::vec3 TerrainGrid::normal(int x, int z) const {
    x = std::clamp(x, 0, m_size - 1);
    z = std::clamp(z, 0, m_size - 1);
    return m_normals[z * m_size + x];
}

std::vector<Chunk*> TerrainGrid::allChunks() {
    std::vector<Chunk*> list;
    list.reserve(m_chunks.size());
    for (Chunk& c : m_chunks)
        list.push_back(&c);
    return list;
}
