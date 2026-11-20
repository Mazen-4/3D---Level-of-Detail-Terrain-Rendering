// =============================================================================
//  ChunkedLOD.cpp   -  Owner: Role A      (plan Section 4.2)
//
//  build()   once:        one VBO per chunk (grid + skirt vertices)
//                         + 4 shared index buffers (terrain + skirt triangles)
//  update()  every frame: choose each visible chunk's LOD
//  draw()    every frame: per chunk, bind its VAO + the EBO of its LOD, draw
// =============================================================================
#include "lod/ChunkedLOD.h"

#include <glad/glad.h>

#include "core/Camera.h"
#include "core/Settings.h"
#include "core/Shader.h"
#include "core/Stats.h"
#include "lod/TerrainVertex.h"
#include "terrain/TerrainGrid.h"

// How many vertices we skip at each LOD (index = LOD number).
// LOD 3 uses every vertex, LOD 0 uses every 8th vertex.
static const int LOD_STRIDE[ChunkedLOD::LOD_COUNT] = { 8, 4, 2, 1 };

static const int QUADS    = TerrainGrid::CHUNK_QUADS;      // 128
static const int VERTICES = TerrainGrid::CHUNK_VERTICES;   // 129

// How far the skirts hang below the chunk edge, in world units (plan Fig. 3).
// It must be deeper than the biggest crack.  Increase it if you ever see sky through a crack.
static const float SKIRT_DEPTH = 20.0f;

// The 4 edges of a chunk, used to build skirts.
enum Edge { EDGE_NORTH = 0, EDGE_SOUTH = 1, EDGE_WEST = 2, EDGE_EAST = 3, EDGE_COUNT = 4 };

// Index (inside the chunk's vertex buffer) of the i-th grid vertex along an edge.
static unsigned int edgeVertexIndex(int edge, int i) {
    switch (edge) {
        case EDGE_NORTH: return 0 * VERTICES + i;           // row z = 0
        case EDGE_SOUTH: return QUADS * VERTICES + i;       // row z = 128
        case EDGE_WEST:  return i * VERTICES + 0;           // column x = 0
        default:         return i * VERTICES + QUADS;       // column x = 128 (EDGE_EAST)
    }
}

// Index of the skirt vertex hanging below edgeVertexIndex(edge, i).
// Skirt vertices are stored after the 129 x 129 grid, 129 per edge.
static unsigned int skirtVertexIndex(int edge, int i) {
    return VERTICES * VERTICES + edge * VERTICES + i;
}

// =============================================================================
//  build: runs once at start-up
// =============================================================================
void ChunkedLOD::build(const TerrainGrid& grid) {
    m_chunksPerSide = grid.chunksPerSide();
    m_meshes.resize(m_chunksPerSide * m_chunksPerSide);

    for (const Chunk& chunk : grid.chunks())
        buildChunkMesh(grid, chunk, m_meshes[chunk.gz * m_chunksPerSide + chunk.gx]);

    for (int lod = 0; lod < LOD_COUNT; ++lod)
        buildLodIndexBuffer(lod);
}

// Fills the chunk's own GPU vertex buffer:
//   1. its 129 x 129 grid vertices   (vertex (x, z) is stored at  z * 129 + x)
//   2. its skirt vertices: a copy of every edge vertex, moved down by SKIRT_DEPTH
// Every chunk uses this SAME layout, which is why the index buffers can be shared.
void ChunkedLOD::buildChunkMesh(const TerrainGrid& grid, const Chunk& chunk, ChunkMesh& mesh) {
    const int startX = chunk.gx * QUADS;   // first grid column of this chunk
    const int startZ = chunk.gz * QUADS;   // first grid row of this chunk

    std::vector<TerrainVertex> vertices;
    vertices.reserve(VERTICES * VERTICES + EDGE_COUNT * VERTICES);

    // 1. Grid vertices
    for (int z = 0; z < VERTICES; ++z) {
        for (int x = 0; x < VERTICES; ++x) {
            TerrainVertex v;
            v.position = grid.position(startX + x, startZ + z);
            v.normal   = grid.normal(startX + x, startZ + z);
            vertices.push_back(v);
        }
    }

    // 2. Skirt vertices.  Same normal as the edge, so the skirt is lit like the terrain.
    for (int edge = 0; edge < EDGE_COUNT; ++edge) {
        for (int i = 0; i < VERTICES; ++i) {
            TerrainVertex v = vertices[edgeVertexIndex(edge, i)];
            v.position.y -= SKIRT_DEPTH;
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
//
// Then the skirt: along each edge, with the SAME stride, join two neighbouring
// edge vertices (top) to their two skirt vertices (bottom) with 2 triangles.
void ChunkedLOD::buildLodIndexBuffer(int lod) {
    const int stride = LOD_STRIDE[lod];
    std::vector<unsigned int> indices;

    // --- Terrain triangles ----------------------------------------------------
    for (int z = 0; z < QUADS; z += stride) {
        for (int x = 0; x < QUADS; x += stride) {
            unsigned int a = z * VERTICES + x;
            unsigned int b = a + stride;
            unsigned int c = (z + stride) * VERTICES + x;
            unsigned int d = c + stride;
            indices.insert(indices.end(), { a, c, b,   b, c, d });
        }
    }
    const int terrainIndexCount = (int)indices.size();

    // --- Skirt triangles (a vertical strip below each edge) --------------------
    for (int edge = 0; edge < EDGE_COUNT; ++edge) {
        for (int i = 0; i < QUADS; i += stride) {
            unsigned int top0    = edgeVertexIndex(edge, i);
            unsigned int top1    = edgeVertexIndex(edge, i + stride);
            unsigned int bottom0 = skirtVertexIndex(edge, i);
            unsigned int bottom1 = skirtVertexIndex(edge, i + stride);
            indices.insert(indices.end(), { top0, bottom0, top1,   top1, bottom0, bottom1 });
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

    buffer.terrainIndexCount = terrainIndexCount;
    buffer.totalIndexCount   = (int)indices.size();
}

// =============================================================================
//  update: runs every frame
// =============================================================================
void ChunkedLOD::update(const Camera& camera, const Settings& settings,
                        const std::vector<Chunk*>& visible) {
    for (Chunk* chunk : visible) {
        if (settings.lodEnabled)
            chunk->lod = chooseChunkLod(*chunk, camera.position, settings);
        else
            chunk->lod = FINEST_LOD;   // "Enable LOD" off = brute force baseline
    }

    // With LOD off, every chunk has the same LOD, so there can be no cracks and
    // the skirts are not needed.  This keeps the brute-force triangle count exact.
    m_drawSkirts = settings.lodEnabled;
}

// Distance from a point to the CLOSEST point of a box (0 if the point is inside).
// Clamping the point into the box gives that closest point.
static float distanceToBox(const glm::vec3& point, const glm::vec3& boxMin, const glm::vec3& boxMax) {
    glm::vec3 closest = glm::clamp(point, boxMin, boxMax);
    return glm::length(point - closest);
}

// Picks the LOD of one chunk (plan Section 4.2, Fig. 2).
// The thresholds double for each coarser level: 1 D, 2 D, 4 D.
int ChunkedLOD::chooseChunkLod(const Chunk& chunk, const glm::vec3& cameraPosition,
                               const Settings& settings) const {
    // 1. Distance rule.  D grows with the "LOD Distance" slider,
    //    so a bigger slider value keeps fine detail further away.
    const float D        = settings.lodDistance * QUADS;
    const float distance = distanceToBox(cameraPosition, chunk.boxMin, chunk.boxMax);

    int lod;
    if      (distance < 1.0f * D) lod = 3;
    else if (distance < 2.0f * D) lod = 2;
    else if (distance < 4.0f * D) lod = 1;
    else                          lod = 0;

    // 2. Budget rule ("Max Triangles" slider): step down while this level is too heavy.
    //    LOD 0 is always allowed, so every chunk is still drawn.
    while (lod > 0 && trianglesAtLod(lod) > settings.maxTrianglesPerChunk)
        lod--;

    return lod;
}

// =============================================================================
//  draw: runs every frame.  One draw call per visible chunk.
// =============================================================================
void ChunkedLOD::draw(Shader& shader, const std::vector<Chunk*>& visible, Stats& stats) {
    for (Chunk* chunk : visible) {
        const ChunkMesh&      mesh   = m_meshes[chunk->gz * m_chunksPerSide + chunk->gx];
        const LodIndexBuffer& buffer = m_lodBuffers[chunk->lod];
        const int indexCount = m_drawSkirts ? buffer.totalIndexCount : buffer.terrainIndexCount;

        shader.setInt("uLod", chunk->lod);                 // for the "LOD colours" debug view

        glBindVertexArray(mesh.vao);                       // this chunk's vertices
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.ebo); // changing LOD = binding another EBO
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);

        stats.triangles += indexCount / 3;
        stats.drawCalls += 1;
    }
    glBindVertexArray(0);
}
