# External libraries (do not edit)

These are copied into the repo so the project builds offline, the same way on every laptop.

| Folder | Library | Version | What it does for us |
|---|---|---|---|
| `glfw/` | GLFW | 3.4 | Window, keyboard, mouse (source, built by CMake) |
| `glad/` | GLAD | 0.1.36, generated for OpenGL 3.3 Core | Loads the OpenGL functions |
| `glm/` | GLM | 1.0.1 | Vectors and matrices (header-only) |
| `imgui/` | Dear ImGui | 1.91.9 (+ GLFW and OpenGL3 backends) | Control panel |
| `stb/` | stb_image | latest `master` | Loads PNG heightmaps (header-only) |

GLAD was generated with:
`python -m glad --profile core --api gl=3.3 --generator c --spec gl --no-loader --out-path external/glad`

Each library keeps its own license file.
