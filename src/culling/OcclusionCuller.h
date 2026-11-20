#pragma once
// =============================================================================
//  OcclusionCuller  -  removes chunks hidden behind mountains (plan Section 4.5, Fig. 7).
//
//  Owner: Role C.
//
//  The idea: after drawing the terrain, draw each chunk's bounding box
//  invisibly inside an OpenGL "occlusion query".  The GPU tells us if ANY pixel
//  of the box passed the depth test.  We read that answer NEXT frame (so the
//  CPU never waits for the GPU) and skip chunks whose box was fully hidden.
// =============================================================================
#include <vector>
#include "terrain/Chunk.h"

class Camera;
class TerrainGrid;

class OcclusionCuller {
public:
    // Once at start-up: create one query object per chunk, box geometry, shader.
    void init(const TerrainGrid& grid);

    // Every frame, BEFORE drawing: read last frame's query results into
    // chunk->occludedLastFrame and return only the chunks that are not occluded.
    // main.cpp counts how many were removed for stats.chunksOccluded.
    std::vector<Chunk*> cull(const std::vector<Chunk*>& chunks);

    // Every frame, AFTER drawing the terrain: draw each chunk's box inside a query.
    void issueQueries(const std::vector<Chunk*>& chunks, const Camera& camera);

    // TODO [C]: private members, e.g.  std::vector<unsigned> m_queries;
};
