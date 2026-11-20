#pragma once
// =============================================================================
//  Heightmap  -  a square grid of heights read from a grayscale PNG image.
//                Brightness of pixel (x, z) = height at (x, z).
//
//  Owner: Role B.   (plan Section 4.1)
//
//  Size must be 2^n + 1 pixels per side (1025, 2049, ...):
//  2^n quads need 2^n + 1 vertices, so chunks and LOD strides divide evenly.
// =============================================================================
#include <string>
#include <vector>

class Heightmap {
public:
    // Loads an 8-bit or 16-bit grayscale PNG.  Returns false if it fails.
    bool load(const std::string& path);

    // Makes a procedural test terrain (sine waves), so nobody is ever blocked
    // waiting for a real heightmap file.  size should be 2^n + 1.
    void makeTestTerrain(int size);

    int size() const { return m_size; }   // pixels per side, e.g. 1025

    // Height at pixel (x, z) in the range 0..1.  Coordinates outside the map
    // are clamped to the nearest edge pixel.
    float value(int x, int z) const;

private:
    int m_size = 0;
    std::vector<float> m_values;          // m_size * m_size values, row by row (index = z * m_size + x)
};
