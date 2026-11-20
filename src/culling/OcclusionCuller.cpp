// =============================================================================
//  OcclusionCuller.cpp   -  Owner: Role C
//
//  PHASE 0 STUB:  nothing is ever occluded.
// =============================================================================
#include "culling/OcclusionCuller.h"

void OcclusionCuller::init(const TerrainGrid& /*grid*/) {
    // TODO [C]: glGenQueries(chunkCount, ...), box VAO, lines/box shader.
}

std::vector<Chunk*> OcclusionCuller::cull(const std::vector<Chunk*>& chunks) {
    // TODO [C]: for each chunk, glGetQueryObjectuiv(query, GL_QUERY_RESULT_AVAILABLE, ...);
    //           if available, read GL_QUERY_RESULT -> 0 samples = occluded.
    //           Not available yet -> treat as visible (avoids flicker).
    //           Sort the kept chunks front-to-back.
    return chunks;
}

void OcclusionCuller::issueQueries(const std::vector<Chunk*>& /*chunks*/, const Camera& /*camera*/) {
    // TODO [C]: glColorMask(false...), glDepthMask(false),
    //           for each chunk: glBeginQuery(GL_ANY_SAMPLES_PASSED, q); draw box; glEndQuery(...)
    //           then restore colour and depth writes.
}
