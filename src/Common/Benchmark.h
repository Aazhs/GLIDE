/*
 * Benchmark.h
 * -----------
 * Measures how long an algorithm really takes on this computer.
 *
 * The step-recording version of each algorithm (execute()) spends most of
 * its time building explanation text, so it cannot be timed fairly.
 * Each algorithm therefore also provides a lean rasterize() function with
 * IDENTICAL maths that only writes pixel coordinates into an array.
 * That function is what gets timed here.
 *
 * Method: run the function in batches (doubling the batch size until one
 * batch takes ~0.2 ms), keep timing batches for a few milliseconds and
 * report the FASTEST per-call time. The minimum is the least affected by
 * other programs and interrupts.
 *
 * Results depend on the CPU and compiler, and only make sense in an
 * optimised (Release) build.
 */

#ifndef GLIDE_BENCHMARK_H
#define GLIDE_BENCHMARK_H

#include <chrono>
#include <algorithm>

namespace Bench {

    /* Results are accumulated here so the compiler cannot optimise the work away */
    inline volatile long long sink = 0;

    template <class F>
    double nsPerCall(F f, double budgetMs = 3.0) {
        using clock = std::chrono::steady_clock;
        auto elapsedNs = [](clock::time_point t0) {
            return (double)std::chrono::duration_cast<std::chrono::nanoseconds>(clock::now() - t0).count();
        };

        sink += f();   // warm-up (caches, branch predictors)

        long reps = 1;
        double best = 1e30;
        while (true) {
            auto t0 = clock::now();
            for (long i = 0; i < reps; i++) sink += f();
            double ns = elapsedNs(t0);
            if (ns >= 200000.0 || reps >= (1L << 22)) { best = ns / reps; break; }
            reps *= 2;
        }

        auto start = clock::now();
        while (elapsedNs(start) < budgetMs * 1e6) {
            auto t0 = clock::now();
            for (long i = 0; i < reps; i++) sink += f();
            best = std::min(best, elapsedNs(t0) / reps);
        }
        return best;
    }

} // namespace Bench

#endif // GLIDE_BENCHMARK_H
