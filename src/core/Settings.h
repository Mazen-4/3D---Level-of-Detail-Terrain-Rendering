#pragma once
// =============================================================================
//  Settings  -  everything the user can change in the control panel.
//
//  SHARED HEADER (agreed by the whole team, plan Section 3).
//  Do not change it without asking the other two members first.
//
//  Who writes it:  the UI panel (Role C) writes these values.
//  Who reads it:   main.cpp, the LOD methods and the culling code.
// =============================================================================

// Which LOD method is drawing the terrain.
enum class Mode {
    Chunked,   // Chunked LOD   (Role A)  - precomputed, GPU friendly
    Roam       // ROAM          (Role B)  - adaptive, rebuilt on the CPU every frame
};

struct Settings {
    Mode mode = Mode::Chunked;

    // --- The toggles from the project brief ---------------------------------
    bool lodEnabled       = true;   // false = draw every chunk at full detail (brute force baseline)
    bool frustumCulling   = true;   // skip chunks outside the screen
    bool occlusionCulling = true;   // skip chunks hidden behind mountains

    // --- Debug views ----------------------------------------------------------
    bool wireframe     = false;     // draw triangle edges only
    bool lodColours    = false;     // colour each chunk by its LOD (red/green/blue/purple)
    bool freezeCulling = false;     // stop updating the culling camera, so we can fly out and look at it

    // --- Sliders from the project brief ---------------------------------------
    float lodDistance          = 2.5f;    // bigger = detail stays further away
    int   maxTrianglesPerChunk = 50000;   // per-chunk triangle budget
    float heightScale          = 200.0f;  // how tall the mountains are (world units)

    // --- Added in Phase 0 (not in the plan's Section 3 listing) ---------------
    bool vsync = true;              // turned OFF during benchmarks so FPS is not capped at 60 (plan Section 7)
};
