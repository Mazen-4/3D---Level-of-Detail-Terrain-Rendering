#pragma once
// =============================================================================
//  Paths  -  turns "shaders/terrain.vert" into a full path on this laptop.
//
//  CMake writes the project folder into PROJECT_ROOT when it builds the program,
//  so the program finds its files whatever folder you run it from.
// =============================================================================
#include <string>

#ifndef PROJECT_ROOT
#define PROJECT_ROOT "."
#endif

inline std::string projectPath(const std::string& relativePath) {
    return std::string(PROJECT_ROOT) + "/" + relativePath;
}
