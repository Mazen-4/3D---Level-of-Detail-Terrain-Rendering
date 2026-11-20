// =============================================================================
//  terrain.frag  -  Owner: Role A      (plan Section 4.6)
//  Runs once per PIXEL.  Picks the pixel's colour in 3 steps:
//
//    1. base colour : by HEIGHT (water -> sand -> grass -> rock -> snow)
//                     and by SLOPE (steep = rock)
//                     ... or the LOD colour in the "LOD colours" debug view
//    2. lighting    : one sun, Lambert diffuse + a little ambient light
//    3. fog         : blend towards the sky colour with distance
// =============================================================================
#version 330 core

in vec3 vWorldPos;
in vec3 vNormal;

uniform float uHeightScale;    // the tallest possible height, to turn height into 0..1
uniform vec3  uSunDirection;   // unit vector pointing TOWARDS the sun
uniform vec3  uCameraPos;      // for the fog distance
uniform vec3  uFogColour;      // = the sky colour, so far terrain fades into the sky
uniform float uFogStart;       // no fog closer than this distance
uniform float uFogEnd;         // full fog at this distance
uniform int   uDebugMode;      // 0 = normal shading, 1 = LOD colours
uniform int   uLod;            // LOD of the chunk being drawn (set by ChunkedLOD)

out vec4 fragColour;

const vec3  WATER   = vec3(0.16, 0.36, 0.60);
const vec3  SAND    = vec3(0.76, 0.70, 0.50);
const vec3  GRASS   = vec3(0.26, 0.50, 0.20);
const vec3  ROCK    = vec3(0.45, 0.40, 0.36);
const vec3  SNOW    = vec3(0.95, 0.95, 0.97);
const float AMBIENT = 0.35;    // light that reaches even the shadowed side

// Same colours as plan Fig. 2:  LOD 0 purple, 1 blue, 2 green, 3 red.
const vec3 LOD_COLOURS[4] = vec3[4](vec3(0.55, 0.55, 1.00),
                                    vec3(0.20, 0.45, 0.95),
                                    vec3(0.15, 0.65, 0.30),
                                    vec3(0.85, 0.20, 0.20));

// h = height 0..1,  flatness = normal.y (1 = flat ground, 0 = vertical wall).
// Each mix() blends smoothly into the next band as the height goes up.
vec3 terrainColour(float h, float flatness) {
    vec3 colour = WATER;
    colour = mix(colour, SAND,  smoothstep(0.06, 0.10, h));
    colour = mix(colour, GRASS, smoothstep(0.12, 0.18, h));
    colour = mix(colour, ROCK,  smoothstep(0.50, 0.65, h));
    colour = mix(colour, SNOW,  smoothstep(0.75, 0.85, h));

    // Steep slopes are bare rock (grass and snow don't stay on cliffs).
    float steepness = 1.0 - smoothstep(0.65, 0.80, flatness);
    colour = mix(colour, ROCK, steepness);
    return colour;
}

void main() {
    vec3  n = normalize(vNormal);
    float h = clamp(vWorldPos.y / uHeightScale, 0.0, 1.0);

    // 1. Base colour
    vec3 base;
    if (uDebugMode == 1)
        base = LOD_COLOURS[uLod];
    else
        base = terrainColour(h, n.y);

    // 2. Lambert lighting: brightest when the surface faces the sun.
    float diffuse = max(dot(n, uSunDirection), 0.0);
    vec3  lit     = base * (AMBIENT + (1.0 - AMBIENT) * diffuse);

    // 3. Linear fog: 0 before uFogStart, 1 after uFogEnd.
    //    (Off in the debug view, so the LOD colours stay readable.)
    float distance = length(vWorldPos - uCameraPos);
    float fog      = clamp((distance - uFogStart) / (uFogEnd - uFogStart), 0.0, 1.0);
    if (uDebugMode == 1)
        fog = 0.0;

    fragColour = vec4(mix(lit, uFogColour, fog), 1.0);
}
