// =============================================================================
//  Frustum.cpp   -  Owner: Role C
//
//  PHASE 0 STUB:  keeps every chunk (nothing is culled yet).
// =============================================================================
#include "culling/Frustum.h"
#include "core/Camera.h"

std::vector<Chunk*> Frustum::cull(const std::vector<Chunk*>& chunks, const Camera& /*camera*/) {
    // TODO [C]:
    //   extractPlanes(camera.viewProjection());
    //   keep a chunk only if !isBoxOutside(chunk->boxMin, chunk->boxMax)
    return chunks;
}
