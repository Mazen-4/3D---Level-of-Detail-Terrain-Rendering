// =============================================================================
//  terrain.frag  -  Owner: Role A
//  Runs once per PIXEL.  Picks the pixel's colour.
//
//  PHASE 1 version: colour by height + simple sun light.
//  Phase 3 adds: slope -> rock, fog, LOD-colour debug mode.
// =============================================================================
#version 330 core

in vec3 vWorldPos;
in vec3 vNormal;

uniform float uHeightScale;    // the tallest possible height, to turn height into 0..1
uniform vec3  uSunDirection;   // unit vector pointing TOWARDS the sun

out vec4 fragColour;

void main() {
    // 1. Base colour from height: green valleys -> brown slopes -> white peaks.
    float h = clamp(vWorldPos.y / uHeightScale, 0.0, 1.0);
    vec3 green = vec3(0.25, 0.50, 0.20);
    vec3 brown = vec3(0.45, 0.38, 0.30);
    vec3 white = vec3(0.95, 0.95, 0.97);
    vec3 colour = mix(green, brown, smoothstep(0.35, 0.60, h));
    colour      = mix(colour, white, smoothstep(0.70, 0.85, h));

    // 2. Lambert lighting: brightest when the surface faces the sun.
    vec3  n       = normalize(vNormal);
    float diffuse = max(dot(n, uSunDirection), 0.0);
    float ambient = 0.35;

    fragColour = vec4(colour * (ambient + (1.0 - ambient) * diffuse), 1.0);
}
