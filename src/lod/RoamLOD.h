#pragma once
// =============================================================================
//  RoamLOD  -  LOD method 2: ROAM, split-only version (plan Section 4.3, Fig. 4, Fig. 5).
//
//  Owner: Role B.
//
//  The idea: start every frame with 2 triangles per chunk ("patch") and keep
//  splitting triangles where the terrain is rough and close to the camera.
//  The mesh is thrown away and rebuilt from scratch every frame (simple version).
// =============================================================================
#include "lod/TerrainLOD.h"

class RoamLOD : public TerrainLOD {
public:
    void build(const TerrainGrid& grid) override;
    void update(const Camera& camera, const Settings& settings,
                const std::vector<Chunk*>& visible) override;
    void draw(Shader& shader, const std::vector<Chunk*>& visible, Stats& stats) override;

    // TODO [B]: add your private members here, for example:
    //   struct TriNode { TriNode* leftChild; TriNode* rightChild;
    //                    TriNode* leftNeighbour; TriNode* rightNeighbour; TriNode* base; };
    //   std::vector<TriNode> m_pool;        // fixed-size node pool, reset every frame
    //   std::vector<float>   m_variance;    // per patch, heap-indexed (children of i: 2i, 2i+1)
    //   unsigned m_vao, m_vbo;              // ONE dynamic vertex buffer, filled with glBufferSubData
};
