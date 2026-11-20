#pragma once
// =============================================================================
//  Stats  -  the numbers we measure every frame.
//
//  SHARED HEADER (agreed by the whole team, plan Section 3).
//
//  Who fills it:   main.cpp (fps, frame time, culled counts, lod time)
//                  and the LOD method's draw() (triangles, draw calls).
//  Who reads it:   the UI panel (shows it) and the Benchmark (writes it to CSV).
// =============================================================================

struct Stats {
    float fps     = 0.0f;   // frames per second
    float frameMs = 0.0f;   // time of one frame in milliseconds (= 1000 / fps)

    int triangles           = 0;   // triangles sent to the GPU this frame
    int drawCalls           = 0;   // number of glDraw... calls this frame
    int chunksFrustumCulled = 0;   // chunks skipped because they are off-screen
    int chunksOccluded      = 0;   // chunks skipped because they are hidden behind terrain

    // --- Added in Phase 0 (not in the plan's Section 3 listing) ---------------
    float lodUpdateMs = 0.0f;      // CPU time spent in lod.update(); ROAM's cost (plan Section 7)

    // Called at the start of every frame: the per-frame counters start again from 0.
    void resetFrameCounters() {
        triangles = 0;
        drawCalls = 0;
        chunksFrustumCulled = 0;
        chunksOccluded = 0;
    }
};
