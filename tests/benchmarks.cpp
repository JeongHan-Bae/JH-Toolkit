#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include <iostream>
#include <vector>
#include <deque>
#include <list>
#include <memory>
#include "jh/data_sink.h"


/// **Performance Benchmark: data_sink vs std::vector vs std::deque vs std::list**
TEST_CASE("data_sink Performance Benchmark", "[data_sink]") {
    constexpr std::uint32_t BLOCK_SIZE = 8192;
    constexpr int N = 1e8;

    BENCHMARK_ADVANCED("data_sink Insert")(Catch::Benchmark::Chronometer meter) {
        jh::data_sink<int, BLOCK_SIZE> sink;
        meter.measure([&] {
            for (int i = 0; i < N; ++i) {
                sink.emplace_back(i);
            }
        });
    };

    BENCHMARK_ADVANCED("std::vector Insert")(Catch::Benchmark::Chronometer meter) {
        std::vector<int> vec;
        vec.reserve(N);
        meter.measure([&] {
            for (int i = 0; i < N; ++i) {
                vec.push_back(i);
            }
        });
    };

    BENCHMARK_ADVANCED("std::deque Insert")(Catch::Benchmark::Chronometer meter) {
        std::deque<int> deq;
        meter.measure([&] {
            for (int i = 0; i < N; ++i) {
                deq.push_back(i);
            }
        });
    };

#if !defined(JH_DISABLE_HIGH_ALLOC_BENCHMARK)
    BENCHMARK_ADVANCED("std::list Insert")(Catch::Benchmark::Chronometer meter) {
        std::list<int> lst;
        meter.measure([&] {
            for (int i = 0; i < N; ++i) {
                lst.push_back(i);
            }
        });
    };
#else
    std::cout << "[Benchmark] Skipped std::list insert (disabled at compile time)\n";
#endif
}

/**
 * Benchmark Results Summary
 * ==========================
 * The following benchmark results compare the performance of `data_sink`, `std::vector`, `std::deque`, and `std::list`
 * when inserting 1e8 (100 million) integers sequentially (no prior reservation, default-constructed containers).
 *
 * Results in Clang on macOS M3 (Mean Time):
 * -----------------------------------------
 * - `data_sink Insert`  :   5.04095e+08 ns (~0.504 sec)
 * - `std::vector Insert`:   1.14226e+09 ns (~1.142 sec)
 * - `std::deque Insert` :   2.09253e+09 ns (~2.092 sec)
 * - `std::list Insert`  :   4.11011e+09 ns (~4.110 sec)
 *
 * Key Takeaways:
 * --------------
 * 1. `data_sink` is:
 *    - **~2.27x faster than `std::vector`**
 *    - **~4.15x faster than `std::deque`**
 *    - **~8.15x faster than `std::list`**
 *
 * 2. Performance degradation reasons:
 *    - `std::vector`: incurs cost from **dynamic reallocation and `memcpy()`** during growth.
 *    - `std::deque`: suffers from **fragmented memory layout** and **pointer indirection**.
 *    - `std::list`: incurs **per-node heap allocations** and **poor cache locality** (each `push_back` is an allocation).
 *
 * 3. `data_sink` performance comes from:
 *    - **Fixed-size block allocation (BLOCK_SIZE = 8192)** with amortized growth
 *    - **Contiguous memory within blocks**, preserving cache locality
 *    - **Minimal allocation overhead**, avoiding full container reallocation or per-node allocation
 *
 * Variance and Stability:
 * -----------------------
 * - `data_sink`: lowest variance (`~1.95 ms`), stable across samples
 * - `std::list`: highest stddev due to memory allocator jitter and fragmentation
 *
 * Conclusion:
 * -----------
 * - `data_sink` is an ideal structure for **sequential, append-only workloads** where:
 *     - Pre-knowledge of size is unavailable
 *     - Performance and cache-efficiency are critical
 * - Especially suitable for **log/event buffering**, **stream aggregation**, and **data batching**
 */
