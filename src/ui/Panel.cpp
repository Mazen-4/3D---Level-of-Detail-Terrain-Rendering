// =============================================================================
//  Panel.cpp   -  Owner: Role C
//
//  PHASE 0 STUB:  a TEMPORARY minimal panel, just enough for Roles A and B to
//  test their work.  Role C replaces it with the full panel.
// =============================================================================
#include "ui/Panel.h"
#include "core/Settings.h"
#include "core/Stats.h"

#include <imgui.h>

void Panel::draw(Settings& settings, const Stats& stats) {
    ImGui::Begin("LOD Control", nullptr, ImGuiWindowFlags_AlwaysAutoResize);   // grows to fit its widgets

    ImGui::Text("FPS: %.0f   (%.2f ms)", stats.fps, stats.frameMs);
    ImGui::Text("Triangles: %d   Draw calls: %d", stats.triangles, stats.drawCalls);
    ImGui::Separator();

    // Which LOD method is active.  RadioButton writes into settings.mode.
    int mode = (settings.mode == Mode::Chunked) ? 0 : 1;
    ImGui::RadioButton("Chunked LOD", &mode, 0);
    ImGui::SameLine();
    ImGui::RadioButton("ROAM", &mode, 1);
    settings.mode = (mode == 0) ? Mode::Chunked : Mode::Roam;

    ImGui::Checkbox("Enable LOD", &settings.lodEnabled);
    ImGui::SliderFloat("LOD Distance", &settings.lodDistance, 0.5f, 8.0f);
    ImGui::SliderInt("Max Triangles", &settings.maxTrianglesPerChunk, 512, 50000);
    ImGui::SliderFloat("Height Scale", &settings.heightScale, 20.0f, 600.0f);   // main.cpp rebuilds the terrain
    ImGui::Checkbox("Wireframe", &settings.wireframe);
    ImGui::Checkbox("LOD colours", &settings.lodColours);
    ImGui::Checkbox("V-Sync", &settings.vsync);

    // TODO [C]: keep the controls above (A and B test with them), and add:
    //           Frustum / Occlusion culling toggles, freeze culling,
    //           culled + occluded counters, lod update time, benchmark buttons.

    ImGui::End();
}
