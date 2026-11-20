# Heightmaps (owner: Role B)

- Grayscale PNG, **8-bit or 16-bit**, square, size **2^n + 1** (1025 × 1025 or 2049 × 2049).
- Brightness = height. Height in the world = `pixel / max × heightScale`.
- `main.cpp` loads **`test_1025.png`** from this folder. If it is missing, the program
  uses a built-in sine-wave test terrain instead, so nobody is ever blocked.
- Files **larger than 20 MB** go in `large/` (ignored by Git). Share them with a link.
