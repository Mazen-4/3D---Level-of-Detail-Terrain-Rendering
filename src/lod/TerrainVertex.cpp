// =============================================================================
//  TerrainVertex.cpp   -  Owner: Role A
// =============================================================================
#include "lod/TerrainVertex.h"

#include <cstddef>      // offsetof
#include <glad/glad.h>

void setTerrainVertexLayout() {
    const GLsizei stride = sizeof(TerrainVertex);   // bytes from one vertex to the next (24)

    // Attribute 0 = position: 3 floats, starting at the beginning of each vertex.
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
                          (void*)offsetof(TerrainVertex, position));

    // Attribute 1 = normal: 3 floats, starting right after the position (byte 12).
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                          (void*)offsetof(TerrainVertex, normal));
}
