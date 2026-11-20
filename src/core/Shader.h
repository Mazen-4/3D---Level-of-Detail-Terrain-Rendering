#pragma once
// =============================================================================
//  Shader  -  loads a vertex + fragment shader from files and links them
//             into one GPU program.  Also sets "uniforms" (values passed to the GPU).
//
//  Owner: Role A.   (Implementation: Shader.cpp)
//
//  Usage:
//      Shader s;
//      s.load("shaders/terrain.vert", "shaders/terrain.frag");
//      s.use();
//      s.setMat4("uViewProj", camera.viewProjection());
//
//  Paths are relative to the project folder (see PROJECT_ROOT in CMakeLists.txt).
// =============================================================================
#include <string>
#include <glm/glm.hpp>

class Shader {
public:
    unsigned int id = 0;   // OpenGL program id (0 = not loaded)

    // Reads, compiles and links both files.  Prints the error and returns false on failure.
    bool load(const std::string& vertexPath, const std::string& fragmentPath);

    // Makes this the active program for the next draw calls.
    void use() const;

    // Set a uniform variable in the shader by its name.
    void setInt  (const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec3 (const std::string& name, const glm::vec3& value) const;
    void setMat4 (const std::string& name, const glm::mat4& value) const;
};
