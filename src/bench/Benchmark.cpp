// =============================================================================
//  Benchmark.cpp   -  Owner: Role C
//
//  PHASE 0 STUB:  records nothing.
// =============================================================================
#include "bench/Benchmark.h"
#include "core/Stats.h"

void Benchmark::record(const Stats& /*stats*/) {
    if (!m_running)
        return;
    // TODO [C]: append one CSV row: frame, frameMs, fps, triangles, drawCalls,
    //           chunksFrustumCulled, chunksOccluded, lodUpdateMs
}
