// =============================================================================
//  Shader.cpp   -  Owner: Role A
//
//  A GPU "program" is made of two small programs written in GLSL:
//    * the VERTEX shader runs once per vertex   -> where is it on screen?
//    * the FRAGMENT shader runs once per pixel  -> what colour is it?
//  Steps: read both text files -> compile each -> link them into one program.
// =============================================================================
#include "core/Shader.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>   // glm::value_ptr: gives OpenGL a pointer to the matrix floats

#include <cstdio>
#include <fstream>
#include <sstream>

// Reads a whole text file into a string.  Returns "" if the file can't be opened.
static std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        std::printf("[Shader] Cannot open file: %s\n", path.c_str());
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Compiles one shader stage (vertex or fragment).  Returns its id, or 0 on error.
static GLuint compileStage(GLenum stageType, const std::string& source, const std::string& path) {
    GLuint stage = glCreateShader(stageType);
    const char* text = source.c_str();
    glShaderSource(stage, 1, &text, nullptr);
    glCompileShader(stage);

    GLint ok = 0;
    glGetShaderiv(stage, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(stage, sizeof(log), nullptr, log);
        std::printf("[Shader] Compile error in %s:\n%s\n", path.c_str(), log);
        glDeleteShader(stage);
        return 0;
    }
    return stage;
}

bool Shader::load(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexSource   = readFile(vertexPath);
    std::string fragmentSource = readFile(fragmentPath);
    if (vertexSource.empty() || fragmentSource.empty())
        return false;

    GLuint vertexStage   = compileStage(GL_VERTEX_SHADER,   vertexSource,   vertexPath);
    GLuint fragmentStage = compileStage(GL_FRAGMENT_SHADER, fragmentSource, fragmentPath);
    if (vertexStage == 0 || fragmentStage == 0)
        return false;

    // Link: connect the vertex shader's outputs to the fragment shader's inputs.
    id = glCreateProgram();
    glAttachShader(id, vertexStage);
    glAttachShader(id, fragmentStage);
    glLinkProgram(id);

    // The stages are now copied into the program, so we can delete them.
    glDeleteShader(vertexStage);
    glDeleteShader(fragmentStage);

    GLint ok = 0;
    glGetProgramiv(id, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(id, sizeof(log), nullptr, log);
        std::printf("[Shader] Link error (%s + %s):\n%s\n", vertexPath.c_str(), fragmentPath.c_str(), log);
        glDeleteProgram(id);
        id = 0;
        return false;
    }
    return true;
}

// Every draw call after this uses this program, until another one is used.
void Shader::use() const {
    glUseProgram(id);
}

// Each setter: find the uniform's "location" by name, then upload the value.
// (If the name doesn't exist in the shader, location is -1 and OpenGL ignores it.)
void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setMat4(const std::string& name, const glm::mat4& value) const {
    glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}
