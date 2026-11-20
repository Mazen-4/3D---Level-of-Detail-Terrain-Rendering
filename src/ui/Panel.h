#pragma once
// =============================================================================
//  Panel  -  the ImGui control panel ("LOD Control" box from the project brief).
//
//  Owner: Role C.
//
//  main.cpp (Role A) starts and ends the ImGui frame; Panel::draw() only
//  adds the widgets.  Every widget writes straight into Settings, e.g.
//      ImGui::Checkbox("Frustum Culling", &settings.frustumCulling);
// =============================================================================

struct Settings;
struct Stats;

class Panel {
public:
    void draw(Settings& settings, const Stats& stats);
};
