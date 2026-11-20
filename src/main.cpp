// =============================================================================
//  main.cpp  -  program entry point.   Owner: Role A
//
//  PHASE 0 VERSION ("build check"):
//    opens a window, loads OpenGL, starts ImGui, builds the (stub) terrain,
//    and shows the control panel over a sky-coloured screen.
//    If you can see that, all libraries are installed and linked correctly.
//
//  PHASE 1 turns this into the real frame loop from the plan (Section 3):
//    camera -> frustum cull -> occlusion cull -> LOD update -> draw -> UI
// =============================================================================

// GLAD must be included BEFORE GLFW (GLAD provides the OpenGL declarations).
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <cstdio>

#include "core/Paths.h"
#include "core/Settings.h"
#include "core/Stats.h"
#include "terrain/Heightmap.h"
#include "terrain/TerrainGrid.h"
#include "ui/Panel.h"

// --- Laptops with two GPUs (Intel + NVIDIA/AMD) --------------------------------
// These two exported variables ask the driver to run us on the powerful GPU
// instead of the integrated one.  Without them, benchmarks could be measured
// on the weak GPU by accident.  (Windows only; ignored everywhere else.)
#ifdef _WIN32
extern "C" {
__declspec(dllexport) unsigned long NvOptimusEnablement = 1;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

// --- Constants (no magic numbers in the code below) --------------------------
static const int   WINDOW_WIDTH   = 1600;
static const int   WINDOW_HEIGHT  = 900;
static const char* WINDOW_TITLE   = "LOD Terrain Rendering";
static const char* HEIGHTMAP_FILE = "assets/heightmaps/test_1025.png";
static const int   TEST_TERRAIN_SIZE = 1025;   // used when no heightmap file can be loaded

// Creates the window with an OpenGL 3.3 Core context and loads OpenGL with GLAD.
// Returns nullptr if anything fails.
static GLFWwindow* createWindow() {
    if (!glfwInit()) {
        std::printf("Failed to start GLFW\n");
        return nullptr;
    }
    // Ask for OpenGL 3.3, "core" profile = modern OpenGL only (no old fixed-function API).
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);   // macOS refuses 3.3 Core without this
#endif

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr);
    if (!window) {
        std::printf("Failed to create window (does your GPU support OpenGL 3.3?)\n");
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);

    // Load all OpenGL functions (glDrawElements, glGenBuffers, ...) through GLFW.
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::printf("Failed to load OpenGL functions with GLAD\n");
        return nullptr;
    }
    std::printf("OpenGL %s on %s\n", glGetString(GL_VERSION), glGetString(GL_RENDERER));
    return window;
}

// Starts Dear ImGui and connects it to our window and OpenGL.
static void startImGui(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

static void stopImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

int main() {
    GLFWwindow* window = createWindow();
    if (!window)
        return 1;
    startImGui(window);

    Settings settings;
    Stats    stats;
    Panel    panel;

    // --- Terrain (Role B).  Real heightmap if it loads, otherwise the test terrain.
    Heightmap heightmap;
    if (!heightmap.load(projectPath(HEIGHTMAP_FILE)))
        heightmap.makeTestTerrain(TEST_TERRAIN_SIZE);

    TerrainGrid grid;
    grid.build(heightmap, settings.heightScale);
    std::printf("Terrain: %d x %d vertices, %d x %d chunks\n",
                grid.size(), grid.size(), grid.chunksPerSide(), grid.chunksPerSide());

    // --- Frame loop ------------------------------------------------------------
    double lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Frame timing -> stats
        double now = glfwGetTime();
        float  dt  = (float)(now - lastTime);
        lastTime = now;
        stats.frameMs = dt * 1000.0f;
        stats.fps     = (dt > 0.0f) ? 1.0f / dt : 0.0f;
        glfwSwapInterval(settings.vsync ? 1 : 0);

        // Clear the screen to a sky colour (the terrain is drawn here from Phase 1 on).
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClearColor(0.55f, 0.70f, 0.90f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // UI: start the ImGui frame, let the panel add its widgets, then draw it.
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        panel.draw(settings, stats);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    stopImGui();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
