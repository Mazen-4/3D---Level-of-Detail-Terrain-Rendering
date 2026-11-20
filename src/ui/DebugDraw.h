#pragma once
// =============================================================================
//  DebugDraw  -  draws helper lines on top of the terrain:
//                chunk bounding boxes (grey = frustum-culled, dark grey = occluded)
//                and the frozen culling camera's frustum.
//
//  Owner: Role C.   Uses shaders/lines.vert + shaders/lines.frag (also Role C).
// =============================================================================

class Camera;
class TerrainGrid;
struct Settings;

class DebugDraw {
public:
    void init();

    // cullCamera = the (maybe frozen) camera used for culling, drawn as lines.
    // viewCamera = the camera we are actually looking through.
    void draw(const Settings& settings, const TerrainGrid& grid,
              const Camera& cullCamera, const Camera& viewCamera);
};
