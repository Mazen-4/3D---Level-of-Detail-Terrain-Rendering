#pragma once
// =============================================================================
//  TerrainVertex  -  the ONE vertex format the terrain shader understands.
//
//  Owner: Role A.   Used by ChunkedLOD (A) and RoamLOD (B), so both LOD methods
//  draw with the same shader (shaders/terrain.vert):
//
//      layout(location = 0) in vec3 aPosition;   <-  TerrainVertex::position
//      layout(location = 1) in vec3 aNormal;     <-  TerrainVertex::normal
//
//  Positions are already in WORLD units (x, height, z), so the shader needs
//  no model matrix: gl_Position = uViewProj * vec4(aPosition, 1.0).
// =============================================================================
#include <glm/glm.hpp>

struct TerrainVertex {
    glm::vec3 position;   // world position (x, height, z)
    glm::vec3 normal;     // unit surface normal, used for lighting
};

// Tells OpenGL how a TerrainVertex is laid out in memory.
// Call it ONCE per VAO, while that VAO and its vertex buffer (VBO) are bound.
//
// For ROAM (Role B):
//     glBindVertexArray(vao);
//     glBindBuffer(GL_ARRAY_BUFFER, vbo);
//     setTerrainVertexLayout();
void setTerrainVertexLayout();
