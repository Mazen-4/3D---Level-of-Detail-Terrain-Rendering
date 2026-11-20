// =============================================================================
//  ChunkedLOD.cpp   -  Owner: Role A      (plan Section 4.2)
//
//  build()   once:        one VBO per chunk + 4 shared index buffers
//  update()  every frame: choose each visible chunk's LOD
//  draw()    every frame: per chunk, bind its VAO + the EBO of its LOD, draw
// =============================================================================
#include "lod/ChunkedLOD.h"

#include <glad/glad.h>

#include "core/Settings.h"
#include "core/Stats.h"
#include "lod/TerrainVertex.h"
#include "terrain/TerrainGrid.h"

// How many vertices we skip at each LOD (index = LOD number).
// LOD 3 uses every vertex, LOD 0 uses every 8th vertex.
static const int LOD_STRIDE[ChunkedLOD::LOD_COUNT] = { 8, 4, 2, 1 };

static const int QUADS    = TerrainGrid::CHUNK_QUADS;      // 128
static const int VERTICES = TerrainGrid::CHUNK_VERTICES;   // 129

// -----------------------------------------------------------------------------
//  build: runs once at start-up
// -----------------------------------------------------------------------------
void ChunkedLOD::build(const TerrainGrid& grid) {
    m_chunksPerSide = grid.chunksPerSide();
    m_meshes.resize(m_chunksPerSide * m_chunksPerSide);

    for (const Chunk& chunk : grid.chunks())
        buildChunkMesh(grid, chunk, m_meshes[chunk.gz * m_chunksPerSide + chunk.gx]);

    for (int lod = 0; lod < LOD_COUNT; ++lod)
        buildLodIndexBuffer(lod);
}

// Copies the chunk's 129 x 129 vertices into its own GPU vertex buffer.
// Vertex (x, z) of the chunk is stored at position  z * 129 + x  in the buffer.
// Every chunk uses this SAME layout, which is why the index buffers can be shared.
void ChunkedLOD::buildChunkMesh(const TerrainGrid& grid, const Chunk& chunk, ChunkMesh& mesh) {
    const int startX = chunk.gx * QUADS;   // first grid column of this chunk
    const int startZ = chunk.gz * QUADS;   // first grid row of this chunk

    std::vector<TerrainVertex> vertices;
    vertices.reserve(VERTICES * VERTICES);
    for (int z = 0; z < VERTICES; ++z) {
        for (int x = 0; x < VERTICES; ++x) {
            TerrainVertex v;
            v.position = grid.position(startX + x, startZ + z);
            v.normal   = grid.normal(startX + x, startZ + z);
            vertices.push_back(v);
        }
    }

    // VAO = remembers "which buffer + how to read it".  VBO = the vertex data itself.
    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);

    glBindVertexArray(mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(TerrainVertex),
                 vertices.data(), GL_STATIC_DRAW);   // STATIC: uploaded once, drawn many times
    setTerrainVertexLayout();
    glBindVertexArray(0);
}

// Builds the triangle list for one LOD by walking the grid with a stride
// (plan Section 4.2).  Each quad becomes 2 triangles:
//
//      a ---- b        a = (x,          z)
//      |    / |        b = (x + stride, z)
//      |  /   |        c = (x,          z + stride)
//      c ---- d        d = (x + stride, z + stride)
//
//  triangles (a, c, b) and (b, c, d)  - both wound counter-clockwise seen from above.
void ChunkedLOD::buildLodIndexBuffer(int lod) {
    const int stride = LOD_STRIDE[lod];

    std::vector<unsigned int> indices;
    for (int z = 0; z < QUADS; z += stride) {
        for (int x = 0; x < QUADS; x += stride) {
            unsigned int a = z * VERTICES + x;
            unsigned int b = a + stride;
            unsigned int c = (z + stride) * VERTICES + x;
            unsigned int d = c + stride;
            indices.insert(indices.end(), { a, c, b,   b, c, d });
        }
    }

    // Upload.  Note: we bind it to GL_ARRAY_BUFFER only to copy the data in;
    // a buffer is just GPU memory.  It becomes an "index buffer" in draw(),
    // when we bind it to GL_ELEMENT_ARRAY_BUFFER.  (Binding an element buffer
    // here, with no VAO bound, is not allowed in OpenGL core.)
    LodIndexBuffer& buffer = m_lodBuffers[lod];
    glGenBuffers(1, &buffer.ebo);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.ebo);
    glBufferData(GL_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                 indices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    buffer.indexCount = (int)indices.size();
}

// -----------------------------------------------------------------------------
//  update: runs every frame
// -----------------------------------------------------------------------------
void ChunkedLOD::update(const Camera& /*camera*/, const Settings& /*settings*/,
                        const std::vector<Chunk*>& visible) {
    // PHASE 1 = brute force: every chunk at full detail.
    // Phase 2 replaces this with a distance-based choice (chooseChunkLod).
    for (Chunk* chunk : visible)
        chunk->lod = FINEST_LOD;
}

// -----------------------------------------------------------------------------
//  draw: runs every frame.  One draw call per visible chunk.
// -----------------------------------------------------------------------------
void ChunkedLOD::draw(Shader& /*shader*/, const std::vector<Chunk*>& visible, Stats& stats) {
    for (Chunk* chunk : visible) {
        const ChunkMesh&      mesh   = m_meshes[chunk->gz * m_chunksPerSide + chunk->gx];
        const LodIndexBuffer& buffer = m_lodBuffers[chunk->lod];

        glBindVertexArray(mesh.vao);                       // this chunk's vertices
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.ebo); // changing LOD = binding another EBO
        glDrawElements(GL_TRIANGLES, buffer.indexCount, GL_UNSIGNED_INT, nullptr);

        stats.triangles += buffer.indexCount / 3;
        stats.drawCalls += 1;
    }
    glBindVertexArray(0);
}
