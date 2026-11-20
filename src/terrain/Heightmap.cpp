// =============================================================================
//  Heightmap.cpp   -  Owner: Role B
//
//  PHASE 0 STUB:  makeTestTerrain() and value() work.  load() is still a TODO,
//  so main.cpp falls back to the test terrain.
// =============================================================================
#include "terrain/Heightmap.h"

#include <algorithm>   // std::clamp
#include <cmath>       // std::sin, std::cos
#include <cstdio>      // std::printf

bool Heightmap::load(const std::string& path) {
    // TODO [B]: load the PNG with stb_image:
    //   * 16-bit:  stbi_load_16(path, &w, &h, &channels, 1)  -> divide by 65535
    //   *  8-bit:  stbi_load   (path, &w, &h, &channels, 1)  -> divide by 255
    //   * check that w == h and that (w - 1) is a power of two (2^n + 1)
    //   * fill m_size and m_values, then stbi_image_free(...)
    //   (stb_image is already compiled in StbImage.cpp; just  #include <stb_image.h>)
    std::printf("[Heightmap] load('%s') not implemented yet (TODO [B]).\n", path.c_str());
    return false;
}

// A few overlapping sine waves: gives hills and valleys that look roughly like terrain.
void Heightmap::makeTestTerrain(int size) {
    m_size = size;
    m_values.resize(size * size);

    for (int z = 0; z < size; ++z) {
        for (int x = 0; x < size; ++x) {
            float h = 0.5f
                    + 0.25f * std::sin(x * 0.010f) * std::cos(z * 0.012f)   // big hills
                    + 0.10f * std::sin(x * 0.037f + z * 0.021f)             // medium ridges
                    + 0.03f * std::sin(x * 0.110f) * std::sin(z * 0.130f);  // small bumps
            m_values[z * size + x] = std::clamp(h, 0.0f, 1.0f);
        }
    }
}

float Heightmap::value(int x, int z) const {
    // Clamp so that asking for a pixel just outside the map is safe
    // (needed by the normal calculation at the borders).
    x = std::clamp(x, 0, m_size - 1);
    z = std::clamp(z, 0, m_size - 1);
    return m_values[z * m_size + x];
}
