// =============================================================================
//  main.cpp  -  program entry point.   Owner: Role A
//
//  1. start-up:  window, OpenGL, ImGui, terrain, shaders, LOD methods, culling
//  2. frame loop (plan Section 3, Fig. 1):
//       camera -> frustum cull -> occlusion cull -> LOD update -> draw -> debug -> UI
//
//  Every step of the loop is one line that calls one module, and each line
//  is tagged with the role that owns that module: [A], [B] or [C].
// =============================================================================

// GLAD must be included BEFORE GLFW (GLAD provides the OpenGL declarations).
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <cstdio>
#include <vector>

#include "core/Camera.h"
#include "core/Paths.h"
#include "core/Settings.h"
#include "core/Shader.h"
#include "core/Stats.h"
#include "terrain/Heightmap.h"
#include "terrain/TerrainGrid.h"
#include "lod/ChunkedLOD.h"
#include "lod/RoamLOD.h"
#include "culling/Frustum.h"
#include "culling/OcclusionCuller.h"
#include "ui/DebugDraw.h"
#include "ui/Panel.h"
#include "bench/Benchmark.h"

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
static const int   WINDOW_WIDTH      = 1600;
static const int   WINDOW_HEIGHT     = 900;
static const char* WINDOW_TITLE      = "LOD Terrain Rendering";
static const char* HEIGHTMAP_FILE    = "assets/heightmaps/test_1025.png";
static const int   TEST_TERRAIN_SIZE = 1025;   // used when no heightmap file can be loaded
static const glm::vec3 SKY_COLOUR(0.55f, 0.70f, 0.90f);
static const glm::vec3 SUN_DIRECTION = glm::normalize(glm::vec3(0.5f, 0.8f, 0.3f));  // towards the sun
static const double FPS_UPDATE_SECONDS = 0.5;  // the FPS number is averaged over this time
static const float FOG_START_FRACTION = 0.6f;  // fog starts at 0.6 x terrain width from the camera
static const float FOG_END_FRACTION   = 1.6f;  // and is complete at 1.6 x terrain width

// =============================================================================
//  Start-up helpers
// =============================================================================

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

// Loads the real heightmap if it exists, otherwise the built-in test terrain,
// then builds the grid (heights, normals, chunks).
static void buildTerrain(TerrainGrid& grid, const Settings& settings) {
    Heightmap heightmap;
    if (!heightmap.load(projectPath(HEIGHTMAP_FILE)))
        heightmap.makeTestTerrain(TEST_TERRAIN_SIZE);

    grid.build(heightmap, settings.heightScale);
    std::printf("Terrain: %d x %d vertices, %d x %d chunks\n",
                grid.size(), grid.size(), grid.chunksPerSide(), grid.chunksPerSide());
}

// Puts the camera above the near edge of the terrain, looking across it.
static void placeCameraOverTerrain(Camera& camera, const TerrainGrid& grid, const Settings& settings) {
    float size = (float)grid.size();
    camera.position     = glm::vec3(size * 0.5f, settings.heightScale * 1.5f, size * 1.05f);
    camera.yawDegrees   = -90.0f;   // look towards -Z, i.e. across the terrain
    camera.pitchDegrees = -20.0f;
}

// =============================================================================
//  Per-frame helpers
// =============================================================================

// Measures the time since the last frame.  Returns dt in seconds.
// stats.frameMs is exact per frame; stats.fps is averaged so the number is readable.
struct FrameTimer {
    double lastTime    = 0.0;
    double fpsStart    = 0.0;
    int    fpsFrames   = 0;
};

static float updateFrameTiming(FrameTimer& timer, Stats& stats) {
    double now = glfwGetTime();
    float  dt  = (float)(now - timer.lastTime);
    timer.lastTime = now;
    stats.frameMs  = dt * 1000.0f;

    timer.fpsFrames++;
    double elapsed = now - timer.fpsStart;
    if (elapsed >= FPS_UPDATE_SECONDS) {
        stats.fps        = (float)(timer.fpsFrames / elapsed);
        timer.fpsFrames  = 0;
        timer.fpsStart   = now;
    }
    return dt;
}

// Things that must be set at the start of every frame: viewport (window may have
// been resized), camera aspect ratio, V-Sync, clear the screen, wireframe on/off.
static void beginFrame(GLFWwindow* window, const Settings& settings, Camera& camera) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    if (height > 0)   // height is 0 while the window is minimised
        camera.aspectRatio = (float)width / (float)height;

    glfwSwapInterval(settings.vsync ? 1 : 0);

    glClearColor(SKY_COLOUR.r, SKY_COLOUR.g, SKY_COLOUR.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPolygonMode(GL_FRONT_AND_BACK, settings.wireframe ? GL_LINE : GL_FILL);
}

// Uploads the values the terrain shader needs this frame (see shaders/terrain.frag).
// (uLod is set per chunk by ChunkedLOD::draw.)
static void setTerrainUniforms(const Shader& shader, const Camera& camera,
                               const Settings& settings, float terrainSize) {
    shader.use();
    shader.setMat4("uViewProj", camera.viewProjection());
    shader.setFloat("uHeightScale", settings.heightScale);
    shader.setVec3("uSunDirection", SUN_DIRECTION);
    shader.setVec3("uCameraPos", camera.position);
    shader.setVec3("uFogColour", SKY_COLOUR);
    shader.setFloat("uFogStart", FOG_START_FRACTION * terrainSize);
    shader.setFloat("uFogEnd", FOG_END_FRACTION * terrainSize);
    shader.setInt("uDebugMode", settings.lodColours ? 1 : 0);
    shader.setInt("uLod", ChunkedLOD::FINEST_LOD);   // default; ChunkedLOD sets it per chunk
}

// Starts a new ImGui frame.  Done early in the frame so we know, before moving
// the camera, whether the mouse/keyboard is being used by the UI.
static void beginUiFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

static void endUiFrame() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);   // the UI is never drawn in wireframe
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// =============================================================================
//  main
// =============================================================================
int main() {
    // ---------------------------------------------------------------- start-up
    GLFWwindow* window = createWindow();
    if (!window)
        return 1;
    startImGui(window);
    glEnable(GL_DEPTH_TEST);   // near triangles hide far ones

    Settings settings;
    Stats    stats;

    TerrainGrid grid;                                   // [B]
    buildTerrain(grid, settings);

    Camera camera;                                      // [A]
    placeCameraOverTerrain(camera, grid, settings);
    Camera cullCamera = camera;                         // [C] the camera used for culling (can be frozen)

    Shader terrainShader;                               // [A]
    if (!terrainShader.load(projectPath("shaders/terrain.vert"), projectPath("shaders/terrain.frag")))
        return 1;

    ChunkedLOD chunked;                                 // [A]
    RoamLOD    roam;                                    // [B]
    chunked.build(grid);
    roam.build(grid);

    Frustum         frustum;                            // [C]
    OcclusionCuller occlusion;                          // [C]
    DebugDraw       debugDraw;                          // [C]
    Panel           panel;                              // [C]
    Benchmark       benchmark;                          // [C]
    occlusion.init(grid);
    debugDraw.init();

    FrameTimer timer;
    timer.lastTime = timer.fpsStart = glfwGetTime();

    // -------------------------------------------------------------- frame loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        float dt = updateFrameTiming(timer, stats);
        stats.resetFrameCounters();
        beginFrame(window, settings, camera);
        beginUiFrame();

        // 1. Move the camera, unless the mouse/keyboard is busy with the UI panel.   [A]
        ImGuiIO& io = ImGui::GetIO();
        if (!io.WantCaptureMouse && !io.WantCaptureKeyboard)
            camera.update(window, dt);
        if (!settings.freezeCulling)                                                  // [C]
            cullCamera = camera;

        // 2. Start with every chunk, then remove what can't be seen.
        std::vector<Chunk*> visible = grid.allChunks();                               // [B]
        if (settings.frustumCulling) {                                                // [C]
            size_t before = visible.size();
            visible = frustum.cull(visible, cullCamera);
            stats.chunksFrustumCulled = (int)(before - visible.size());
        }
        if (settings.occlusionCulling) {                                              // [C]
            size_t before = visible.size();
            visible = occlusion.cull(visible);
            stats.chunksOccluded = (int)(before - visible.size());
        }

        // 3. Pick the LOD method chosen in the UI, let it choose detail, and draw.   [A]/[B]
        //    Both classes implement the TerrainLOD interface, so the rest of the
        //    loop doesn't care which one it is talking to.
        TerrainLOD* lod = &chunked;
        if (settings.mode == Mode::Roam)
            lod = &roam;

        double updateStart = glfwGetTime();
        lod->update(camera, settings, visible);
        stats.lodUpdateMs = (float)((glfwGetTime() - updateStart) * 1000.0);

        setTerrainUniforms(terrainShader, camera, settings, (float)grid.size());
        lod->draw(terrainShader, visible, stats);

        // 4. Ask the GPU which chunks were hidden (answers are read next frame).     [C]
        if (settings.occlusionCulling)
            occlusion.issueQueries(grid.allChunks(), cullCamera);

        // 5. Debug lines, control panel, benchmark recording.                        [C]
        debugDraw.draw(settings, grid, cullCamera, camera);
        panel.draw(settings, stats);
        endUiFrame();
        benchmark.record(stats);

        glfwSwapBuffers(window);
    }

    // ----------------------------------------------------------------- shut down
    // (GPU buffers are freed automatically when the OpenGL context is destroyed.)
    stopImGui();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
