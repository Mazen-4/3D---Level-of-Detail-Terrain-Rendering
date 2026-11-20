// =============================================================================
//  terrain.vert  -  Owner: Role A
//  Runs once per VERTEX.  Moves the vertex from world space to the screen.
// =============================================================================
#version 330 core

layout(location = 0) in vec3 aPosition;   // world position (see TerrainVertex.h)
layout(location = 1) in vec3 aNormal;     // surface normal

uniform mat4 uViewProj;                   // camera.viewProjection()

out vec3 vWorldPos;                       // passed on to the fragment shader
out vec3 vNormal;

void main() {
    vWorldPos   = aPosition;
    vNormal     = aNormal;
    gl_Position = uViewProj * vec4(aPosition, 1.0);
}
