// =============================================================================
//  RoamLOD.cpp   -  Owner: Role B
//
//  PHASE 0 STUB:  does nothing, so selecting ROAM in the UI draws no terrain yet.
//
//  Suggested build order (plan, "Tip for B") - check each step in wireframe:
//    1. one patch, split everything to level 4 -> a regular grid
//    2. add the distance-only split rule
//    3. add variance (roughness), precomputed once in build()
//    4. add forced splits (no cracks inside a patch)
//    5. many patches with linked neighbours (no cracks between patches)
//
//  Drawing: collect every leaf triangle into one CPU array, upload it to ONE
//  dynamic VBO with glBufferSubData, and draw it with the same terrain shader
//  as Chunked LOD (Role A will document the vertex layout the shader expects).
// =============================================================================
#include "lod/RoamLOD.h"
#include "core/Stats.h"

void RoamLOD::build(const TerrainGrid& /*grid*/) {
    // TODO [B]: allocate the node pool, compute variance trees for every patch,
    //           create the dynamic VAO/VBO.
}

void RoamLOD::update(const Camera& /*camera*/, const Settings& /*settings*/,
                     const std::vector<Chunk*>& /*visible*/) {
    // TODO [B]: reset the pool, create 2 root triangles per visible patch,
    //           link neighbours, then tessellate (split rule + forced split).
}

void RoamLOD::draw(Shader& /*shader*/, const std::vector<Chunk*>& /*visible*/, Stats& /*stats*/) {
    // TODO [B]: write leaf triangles to the VBO, glDrawArrays(...),
    //           stats.triangles += count;  stats.drawCalls += 1;
}
