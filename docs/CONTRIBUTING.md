# How we work together (Git, reviews, code style)

These are the rules from the plan (Section 6), written as steps. They keep `main` working and
make sure every one of us can explain every part of the project.

---

## 1. The five rules

1. **`main` must always build and run.** Never push straight to `main`; always use a pull request.
2. **One branch per feature**, named `<role letter>/<feature>`: `a/chunked-lod`, `b/heightmap`, `b/roam-forced-split`, `c/frustum`, `c/occlusion`.
3. **Only edit your own files** (owners are listed in [ARCHITECTURE.md §5](ARCHITECTURE.md#5-files-and-owners)). Found a bug in someone else's file? Tell them, or open a PR and ask them to review it.
4. **The 4 shared headers are frozen:** `Settings.h`, `Stats.h`, `Chunk.h`, `TerrainLOD.h`. Change them only after all three agree.
5. **No build folders or big files in Git.** `build/` is ignored. Heightmaps over 20 MB go in `assets/heightmaps/large/` (ignored) and are shared by link.

---

## 2. Step by step: from idea to `main`

```bash
# 1. start from the latest main
git checkout main
git pull

# 2. make your branch
git checkout -b b/heightmap

# 3. work in small steps; build and run after each one
python -m cmake --build build -j 8
build\LODTerrain.exe

# 4. commit with a message that says WHAT and WHY
git add src/terrain/Heightmap.cpp
git commit -m "Heightmap: load 8- and 16-bit PNGs with stb_image"

# 5. push and open a pull request into main
git push -u origin b/heightmap
gh pr create --base main        # or use the GitHub website
```

If `main` changed while you were working: `git checkout main && git pull`, then
`git checkout b/heightmap && git merge main`, fix any conflicts, build and run again, and push.

---

## 3. Pull requests

**Reviewer rotation** (plan Section 6): **A's PRs are reviewed by B, B's by C, C's by A.**
Reviewing is how we learn each other's code, so read it properly. If you can't explain a
part after reading it, ask in the PR before approving.

**Every PR description has:**
1. **What changed:** 2–5 bullet points.
2. **How to see it in the app:** exact steps, e.g. "tick Freeze culling, fly up, the grey boxes are the culled chunks".
3. **Tested:** "builds and runs on my laptop", plus any numbers (triangles, FPS).

**Before you approve someone's PR:**
- [ ] I pulled the branch, built it and ran it.
- [ ] The "how to see it" steps work.
- [ ] No files from another role were changed (or the owner agreed).
- [ ] I can explain what the new code does.

**Merging:** the author merges after one approval, using **"Create a merge commit"** (not squash), so branches
that build on this one stay clean.

---

## 4. Code style (so we can explain every line)

- **One idea per file.** Short functions (under ~40 lines) named after what they do: `chooseChunkLod`, `extractFrustumPlanes`, `forceSplit`.
- **A plain-English comment above every function.** Point to the plan's figures where it helps ("see Fig. 5").
- **Named constants, no magic numbers:** `CHUNK_QUADS = 128`, `SKIRT_DEPTH = 20.0f`.
- **Plain C++ only:** structs and simple classes. No templates, no inheritance (except `TerrainLOD`), no multithreading, no clever tricks.
- **Tag your TODOs with your role:** `// TODO [B]: ...`, so everyone can search for theirs.
- **Restore OpenGL state you change** (depth mask, colour mask, polygon mode, bound VAO). See [ARCHITECTURE.md §6](ARCHITECTURE.md#6-rules-that-keep-the-parts-from-breaking-each-other).
- **Count what you draw** in `stats.triangles` and `stats.drawCalls`.

---

## 5. Commit messages

```text
Short summary in the imperative, under 72 characters

Optional longer explanation: why this change, what it fixes,
any numbers you measured.
```

Good: `Frustum: extract the 6 planes from projection x view`.
Bad: `fix`, `update`, `changes`.
