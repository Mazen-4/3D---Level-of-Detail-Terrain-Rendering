# =============================================================================
#  LOD Terrain Rendering  -  build file
#
#  How to build (see README.md for details):
#      cmake -S . -B build
#      cmake --build build
#
#  Every library we use lives in external/ so the project builds offline,
#  the same way on all three laptops.
# =============================================================================
cmake_minimum_required(VERSION 3.16)
project(LODTerrain C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default to a Release build (fast) when no build type is given.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release)
endif()

# -----------------------------------------------------------------------------
# 1) GLFW  -  window, keyboard, mouse.  We build it from source.
#    Turn off everything we don't need (docs, tests, examples).
# -----------------------------------------------------------------------------
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_WAYLAND  OFF CACHE BOOL "" FORCE)  # Linux only: X11 alone needs fewer packages
add_subdirectory(external/glfw)

# -----------------------------------------------------------------------------
# 2) GLAD  -  loads the OpenGL 3.3 Core functions at runtime.
# -----------------------------------------------------------------------------
add_library(glad STATIC external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

# -----------------------------------------------------------------------------
# 3) Dear ImGui  -  the control panel.  Core files + the GLFW/OpenGL3 backends.
# -----------------------------------------------------------------------------
add_library(imgui STATIC
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_demo.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC external/imgui external/imgui/backends)
target_link_libraries(imgui PUBLIC glfw)

# -----------------------------------------------------------------------------
# 4) Our program.  EVERY .cpp file under src/ is compiled automatically,
#    so when you add a new file you do NOT need to edit this file
#    (just re-run "cmake -S . -B build").
# -----------------------------------------------------------------------------
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_executable(LODTerrain ${PROJECT_SOURCES})

target_include_directories(LODTerrain PRIVATE
    src                 # so we can write  #include "core/Settings.h"
    external/glm        # GLM is header-only (vectors and matrices)
    external/stb        # stb_image is header-only (loads PNG heightmaps)
)
target_link_libraries(LODTerrain PRIVATE glfw glad imgui)

# MinGW only: put the C++ runtime inside the .exe, so it runs on any laptop
# without copying extra DLLs (libstdc++, libgcc) next to it.
if(MINGW)
    target_link_options(LODTerrain PRIVATE -static)
endif()

# Visual Studio only: read source files as UTF-8.  Without this, Windows set to
# a non-English language (e.g. Arabic) can misread special characters in comments.
if(MSVC)
    target_compile_options(LODTerrain PRIVATE /utf-8)
    target_compile_options(imgui PRIVATE /utf-8)
endif()

# The program finds shaders/ and assets/ through this path,
# so it runs correctly no matter which folder you start it from.
target_compile_definitions(LODTerrain PRIVATE PROJECT_ROOT="${CMAKE_SOURCE_DIR}")
