#pragma once
// =============================================================================
//  Benchmark  -  records a camera path, replays it, and writes one CSV row per
//                frame so every configuration is measured on the SAME flight.
//                (plan Section 7)
//
//  Owner: Role C.   Charts are made afterwards by bench/plot_results.py.
// =============================================================================

struct Stats;

class Benchmark {
public:
    // Called once per frame.  Does nothing unless a benchmark is running.
    void record(const Stats& stats);

    bool isRunning() const { return m_running; }

    // TODO [C]: startRecordingPath(), startReplay(camera), writeCsv(path), ...

private:
    bool m_running = false;
};
