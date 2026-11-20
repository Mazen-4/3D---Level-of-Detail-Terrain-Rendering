#pragma once
// =============================================================================
//  Frustum  -  removes chunks that are outside the camera's view (plan Section 4.4, Fig. 6).
//
//  Owner: Role C.
//
//  The idea:
//    1. take the 6 planes (left, right, bottom, top, near, far) out of the
//       matrix  M = projection * view   (Gribb & Hartmann method);
//    2. for each chunk box, test the corner furthest along each plane's normal
//       (the "positive vertex").  If it is behind ANY plane, the box is outside.
// =============================================================================
#include <vector>
#include <glm/glm.hpp>
#include "terrain/Chunk.h"

class Camera;

// A plane:  dot(normal, point) + d = 0.   Points with  dot(normal, p) + d >= 0  are "inside".
struct Plane {
    glm::vec3 normal{0.0f};
    float d = 0.0f;
};

class Frustum {
public:
    // Returns only the chunks that are (at least partly) inside the camera's view.
    // main.cpp counts how many were removed for stats.chunksFrustumCulled.
    std::vector<Chunk*> cull(const std::vector<Chunk*>& chunks, const Camera& camera);

    // TODO [C]: add helpers, for example:
    //   void extractPlanes(const glm::mat4& viewProjection);   // fills m_planes
    //   bool isBoxOutside(const glm::vec3& boxMin, const glm::vec3& boxMax) const;
    Plane m_planes[6];
};
