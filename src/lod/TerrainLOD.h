#pragma once
// =============================================================================
//  TerrainLOD  -  the ONLY interface (abstract class) in the whole project.
//
//  SHARED HEADER (agreed by the whole team, plan Section 3).
//
//  Both LOD methods follow these same three steps, so main.cpp can switch
//  between them with one line:
//
//      TerrainLOD& lod = (settings.mode == Mode::Chunked) ? chunked : roam;
//      lod.update(camera, settings, visible);
//      lod.draw(shader, visible, stats);
//
//  Implemented by:  ChunkedLOD (Role A)  and  RoamLOD (Role B).
// =============================================================================
#include <vector>
#include "terrain/Chunk.h"

// "Forward declarations": we only use references to these classes here,
// so we don't need their full headers (keeps compile times short).
class TerrainGrid;
class Camera;
class Shader;
struct Settings;
struct Stats;

class TerrainLOD {
public:
    virtual ~TerrainLOD() = default;

    // At start-up: build whatever this method needs (GPU buffers, variance trees, ...).
    // Called AGAIN when the "Height Scale" slider changes, so free or reuse what the
    // previous call created (see ChunkedLOD::releaseGpuBuffers).
    virtual void build(const TerrainGrid& grid) = 0;

    // EVERY frame: decide how detailed each visible chunk should be.
    virtual void update(const Camera& camera, const Settings& settings,
                        const std::vector<Chunk*>& visible) = 0;

    // EVERY frame: draw the visible chunks and add to stats.triangles / stats.drawCalls.
    // (Phase 0 change: the plan says "draw also fills stats", so Stats is now a parameter.)
    virtual void draw(Shader& shader, const std::vector<Chunk*>& visible, Stats& stats) = 0;
};
