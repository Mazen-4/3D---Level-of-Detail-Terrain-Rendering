// =============================================================================
//  DebugDraw.cpp   -  Owner: Role C
//
//  PHASE 0 STUB:  draws nothing.
// =============================================================================
#include "ui/DebugDraw.h"

void DebugDraw::init() {
    // TODO [C]: load shaders/lines.vert + lines.frag, create a dynamic line VBO.
}

void DebugDraw::draw(const Settings& /*settings*/, const TerrainGrid& /*grid*/,
                     const Camera& /*cullCamera*/, const Camera& /*viewCamera*/) {
    // TODO [C]: when settings.freezeCulling is on:
    //   * draw each chunk box as 12 lines (colour by culled / occluded / visible)
    //   * draw the 8 corners of cullCamera's frustum as 12 lines
    //     (tip: corners = inverse(cullCamera.viewProjection()) * NDC cube corners)
}
