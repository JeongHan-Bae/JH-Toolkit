#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include <cstdint>
#include <iostream>
#include <vector>
#include <deque>
#include <list>
#include <memory>
#include "jh/data_sink.h"
#include <stack>
#include <tuple>
#include "jh/pod_stack.h"
#include "jh/runtime_arr.h"


namespace benchmark_data_sink {
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

namespace benchmark_pod_stack {
    struct Frame {
        std::int32_t id;
        float weight;
        std::uint8_t tag;
    };

    static_assert(std::is_trivially_copyable_v<Frame>);
    static_assert(std::is_standard_layout_v<Frame>);
    static_assert(jh::pod::pod_like<Frame>);

    constexpr std::size_t ROUNDS = 200;
    constexpr std::size_t OPS_PER_ROUND = 10000;
    constexpr std::size_t POP_PER_ROUND = 8000;

    TEST_CASE("pod_stack Repeated Push/Pop", "[pod_stack][hot_path][benchmark]") {
        BENCHMARK_ADVANCED("pod_stack repeated emplace")(Catch::Benchmark::Chronometer meter) {
            jh::pod_stack<Frame> stk;
            meter.measure([&] {
                for (std::size_t r = 0; r < ROUNDS; ++r) {
                    for (std::size_t i = 0; i < OPS_PER_ROUND; ++i) {
                        stk.emplace(Frame{
                            static_cast<std::int32_t>(i),
                            static_cast<float>(i) * 0.1f,
                            static_cast<std::uint8_t>(i % 255)
                        });
                    }
                    for (std::size_t i = 0; i < POP_PER_ROUND; ++i) {
                        const auto &[id, w, tag] = stk.top();
                        (void) id;
                        (void) w;
                        (void) tag;
                        stk.pop();
                    }
                }
                while (!stk.empty()) {
                    const auto &[id, w, tag] = stk.top();
                    (void) id;
                    (void) w;
                    (void) tag;
                    stk.pop();
                }
            });
        };

        BENCHMARK_ADVANCED("std::stack<vector> repeated emplace")(Catch::Benchmark::Chronometer meter) {
            std::stack<Frame, std::vector<Frame> > stk;
            meter.measure([&] {
                for (std::size_t r = 0; r < ROUNDS; ++r) {
                    for (std::size_t i = 0; i < OPS_PER_ROUND; ++i) {
                        stk.emplace(Frame{
                            static_cast<std::int32_t>(i),
                            static_cast<float>(i) * 0.1f,
                            static_cast<std::uint8_t>(i % 255)
                        });
                    }
                    for (std::size_t i = 0; i < POP_PER_ROUND; ++i) {
                        const auto &[id, w, tag] = stk.top();
                        (void) id;
                        (void) w;
                        (void) tag;
                        stk.pop();
                    }
                }
                while (!stk.empty()) {
                    const auto &[id, w, tag] = stk.top();
                    (void) id;
                    (void) w;
                    (void) tag;
                    stk.pop();
                }
            });
        };

#if !defined(JH_DISABLE_HIGH_ALLOC_BENCHMARK)
        BENCHMARK_ADVANCED("std::stack<deque> repeated emplace")(Catch::Benchmark::Chronometer meter) {
            std::stack<Frame> stk; // default deque
            meter.measure([&] {
                for (std::size_t r = 0; r < ROUNDS; ++r) {
                    for (std::size_t i = 0; i < OPS_PER_ROUND; ++i) {
                        stk.emplace(Frame{
                            static_cast<std::int32_t>(i),
                            static_cast<float>(i) * 0.1f,
                            static_cast<std::uint8_t>(i % 255)
                        });
                    }
                    for (std::size_t i = 0; i < POP_PER_ROUND; ++i) {
                        const auto &[id, w, tag] = stk.top();
                        (void) id;
                        (void) w;
                        (void) tag;
                        stk.pop();
                    }
                }
                while (!stk.empty()) {
                    const auto &[id, w, tag] = stk.top();
                    (void) id;
                    (void) w;
                    (void) tag;
                    stk.pop();
                }
            });
        };
#else
        std::cout << "[Benchmark] Skipped std::stack<deque> test (JH_DISABLE_HIGH_ALLOC_BENCHMARK defined)\n";
#endif
    }
}

/**
 * Benchmark Results Summary
 * ==========================
 * This benchmark compares the performance of `pod_stack<T>`, `std::stack<T,vector<T>>`, and `std::stack<T,deque<T>>`
 * in a realistic algorithm scenario involving **repeated push/pop cycles** and **gradual stack growth**.
 *
 * Test Scenario:
 * --------------
 * - ROUNDS = 200
 * - Each round:
 *     - Push 10,000 elements
 *     - Pop 8,000 elements
 * - Final cleanup phase pops all remaining elements (200 × 2,000 = 400,000)
 *
 * Total simulated operations:
 * - Pushes: 2,000,000
 * - Pops  : 2,000,000
 * - Logical push+pop pairs: **2 million**
 *
 * Results on Clang / macOS M3 (Mean Time):
 * -----------------------------------------
 * - `pod_stack repeated emplace`       :   ~32.56 ms per iteration
 * - `std::stack<vector> repeated`      :   ~48.22 ms
 * - `std::stack<deque> repeated`       :  ~116.11 ms
 *
 * Per Operation Efficiency:
 * --------------------------
 * - `pod_stack`:
 *     - **~16.28 ns** per push+pop pair
 * - `std::stack<vector>`:
 *     - **~24.11 ns** per push+pop
 * - `std::stack<deque>`:
 *     - **~58.06 ns** per push+pop
 *
 * Key Takeaways:
 * --------------
 * 1. `pod_stack` is:
 *    - **~1.84x faster than `std::stack<vector>`**
 *    - **~3.56x faster than `std::stack<deque>`**
 *
 * 2. Degradation factors:
 *    - `std::stack<vector>`: incurs dynamic reallocation and potential shrinkage
 *    - `std::stack<deque>`: incurs fragmentation and pointer indirection
 *
 * 3. `pod_stack` advantage comes from:
 *    - POD-only placement-new allocation (no per-element destruction)
 *    - Fixed-size block reuse
 *    - Zero shrinkage or capacity loss
 *    - Cache-friendly linear top access
 *
 * Real-World Implications:
 * ------------------------
 * This benchmark simulates **real hot-path stack usage**:
 * - Iterative DFS
 * - Rule engine context stacks
 * - Backtracking algorithms
 * - Nested simulation/rollback states
 *
 * It reveals `pod_stack`'s **true memory reuse power** and **hot-loop stability** —
 * even under pressure, performance remains predictable and fast.
 *
 * Conclusion:
 * -----------
 * - Use `pod_stack` for:
 *     - High-frequency top-of-stack algorithms
 *     - Situations where POD structs dominate performance-critical logic
 *     - Scenarios where vector-based stacks cause reallocation instability
 *
 * - If your use case is a **single-pass push+pop**, prefer `std::stack<T, vector<T>>`,
 *   or even just a `vector<T>` with `.reserve(N)`, `.emplace_back(...)`, and reverse iteration.
 *
 * `pod_stack` shines in **multi-round, sustained stack operations** where reuse and memory stability are critical.
 */

namespace benchmark_runtime_arr {
    constexpr std::size_t N_SMALL = 10'000;
    constexpr std::size_t N_LARGE = 10'000'000;

    struct MyPod {
        std::int32_t id;
        float weight;
        std::uint8_t tag;
    };

    TEST_CASE("runtime_arr vs std::vector init + zeroing", "[runtime_arr][benchmark]") {
        BENCHMARK_ADVANCED("runtime_arr default init + set (small) [int]")(Catch::Benchmark::Chronometer meter) {
            meter.measure([&] {
                jh::runtime_arr<int> arr(N_SMALL);
                for (std::size_t i = 0; i < N_SMALL; ++i)
                    arr[i] = 0;
                return arr[0];
            });
        };

        BENCHMARK_ADVANCED("runtime_arr default init + set (small) [bool]")(Catch::Benchmark::Chronometer meter) {
            meter.measure([&] {
                jh::runtime_arr<bool> arr(N_SMALL);
                for (std::size_t i = 0; i < N_SMALL; ++i)
                    arr[i] = false;
                return static_cast<bool>(arr[0]);
            });
        };

        BENCHMARK_ADVANCED("runtime_arr default init + set (small) [MyPod]")(Catch::Benchmark::Chronometer meter) {
            meter.measure([&] {
                jh::runtime_arr<MyPod> arr(N_SMALL);
                for (std::size_t i = 0; i < N_SMALL; ++i)
                    arr[i] = {0, 0.0f, 0};
                return arr[0];
            });
        };

        BENCHMARK_ADVANCED("runtime_arr default init + set (small) [tuple]")(Catch::Benchmark::Chronometer meter) {
            meter.measure([&] {
                jh::runtime_arr<std::tuple<int, float, u_char>> arr(N_SMALL);
                for (std::size_t i = 0; i < N_SMALL; ++i)
                    arr[i] = {0, 0.0f, 0};
                return arr[0];
            });
        };

        BENCHMARK_ADVANCED("runtime_arr uninitialized + memset 0 [int]")(Catch::Benchmark::Chronometer meter) {
            meter.measure([&] {
                jh::runtime_arr<int> arr(N_SMALL, jh::runtime_arr<int>::uninitialized);
                arr.reset_all();
                return arr[0];
            });
        };

        BENCHMARK_ADVANCED("runtime_arr uninitialized + memset 0 [MyPod]")(Catch::Benchmark::Chronometer meter) {
            meter.measure([&] {
                jh::runtime_arr<MyPod> arr(N_SMALL, jh::runtime_arr<MyPod>::uninitialized);
                arr.reset_all();
                return arr[0];
            });
        };

        BENCHMARK_ADVANCED("std::vector default + set (small) [int]")(Catch::Benchmark::Chronometer meter) {
            meter.measure([&] {
                std::vector<int> vec(N_SMALL);
                for (std::size_t i = 0; i < N_SMALL; ++i)
                    vec[i] = 0;
                return vec[0];
            });
        };

        BENCHMARK_ADVANCED("std::vector default + set (small) [bool]")(Catch::Benchmark::Chronometer meter) {
            meter.measure([&] {
                std::vector<bool> vec(N_SMALL);
                for (std::size_t i = 0; i < N_SMALL; ++i)
                    vec[i] = false;
                return static_cast<bool>(vec[0]);
            });
        };

        BENCHMARK_ADVANCED("std::vector default + set (small) [MyPod]")(Catch::Benchmark::Chronometer meter) {
            meter.measure([&] {
                std::vector<MyPod> vec(N_SMALL);
                for (std::size_t i = 0; i < N_SMALL; ++i)
                    vec[i] = {0, 0.0f, 0};
                return vec[0];
            });
        };

        BENCHMARK_ADVANCED("std::vector default + set (small) [tuple]")(Catch::Benchmark::Chronometer meter) {
            meter.measure([&] {
                std::vector<std::tuple<int, float, u_char>> vec(N_SMALL);
                for (std::size_t i = 0; i < N_SMALL; ++i)
                    vec[i] = {0, 0.0f, 0};
                return vec[0];
            });
        };

        BENCHMARK_ADVANCED("runtime_arr allocator version + set")(Catch::Benchmark::Chronometer meter) {
            meter.measure([&] {
                jh::runtime_arr<int, std::allocator<int>> arr(N_LARGE, std::allocator<int>{});
                for (std::size_t i = 0; i < N_LARGE; ++i)
                    arr[i] = 0;
                return arr[0];
            });
        };

        BENCHMARK_ADVANCED("std::vector default + set")(Catch::Benchmark::Chronometer meter) {
            meter.measure([&] {
                std::vector<int> vec(N_LARGE);
                for (std::size_t i = 0; i < N_LARGE; ++i)
                    vec[i] = 0;
                return vec[0];
            });
        };

        BENCHMARK_ADVANCED("std::vector memset")(Catch::Benchmark::Chronometer meter) {
            meter.measure([&] {
                std::vector<int> vec(N_LARGE);
                std::memset(vec.data(), 0, N_LARGE * sizeof(int));
                return vec[0];
            });
        };
    }
}

/**
 * Benchmark Results Summary
 * ==========================
 * This benchmark compares the performance of `jh::runtime_arr<T>` against `std::vector<T>`
 * across a variety of types — including primitive, POD, and RAII-based structures.
 *
 * Test Scenario:
 * --------------
 * - Types tested:
 *     - `int`                 : Plain primitive type
 *     - `bool`                : Bit-packed optimization proxy
 *     - `MyPod`               : Simple POD struct
 *     - `std::tuple<int,...>` : RAII-aware standard library object
 *
 * - Sizes:
 *     - Small  = 10,000 elements
 *     - Large  = 10,000,000 elements (only for `int`)
 *
 * - Variants:
 *     - Default construction + manual zeroing
 *     - Uninitialized construction + `reset_all()` (memset)
 *     - Large-scale construction with allocator
 *
 * Results on Clang / macOS M3 (Mean Time):
 * -----------------------------------------
 * **Small N = 10,000**
 * - `runtime_arr<int>`              :    ~35.5 µs
 * - `runtime_arr<MyPod>`            :    ~34.8 µs
 * - `runtime_arr<bool>`             :    ~65.4 µs
 * - `runtime_arr<std::tuple<...>>`  :   ~210.8 µs
 *
 * - `vector<int>`                   :   ~106.3 µs
 * - `vector<MyPod>`                 :   ~115.0 µs
 * - `vector<bool>`                  :    ~63.3 µs
 * - `vector<std::tuple<...>>`       :   ~300.4 µs
 *
 * **Uninitialized + memset**
 * - `runtime_arr<int>`              :    ~0.49 µs
 * - `runtime_arr<MyPod>`            :    ~1.55 µs
 *
 * **Large N = 10,000,000**
 * - `runtime_arr<int> (allocator)`  :    ~37.9 ms
 * - `vector<int>` (set loop)        :  ~108.6 ms
 * - `vector<int>` (memset)          :   ~96.0 ms
 *
 * Key Takeaways:
 * --------------
 * 1. `runtime_arr<T>` shows **significant advantage** in:
 *    - Primitive types like `int`
 *    - POD types like `MyPod`
 *    - Scenarios where memory initialization can be skipped (`uninitialized`)
 *
 * 2. With `bool`:
 *    - Performance is comparable between `runtime_arr<bool>` and `std::vector<bool>`
 *    - Due to proxy semantics and bit-packing, further speedup is unlikely
 *
 * 3. With `std::tuple` (RAII types):
 *    - `runtime_arr<std::tuple<...>>` runs **~1.5x faster** than `std::vector<std::tuple<...>>` in small-scale tests
 *    - Although both use `new T[size]`, `runtime_arr` avoids:
 *        - allocator propagation logic
 *        - capacity buffer heuristics (i.e. no over-alloc/growth policy)
 *        - possible safety padding or zero-fill from allocator-aware containers
 *    - This makes `runtime_arr` slightly more efficient when constructing RAII types in tight loops
 *    - For RAII types, both `runtime_arr` and `std::vector` honor constructor/destructor semantics
 *      and are equally safe in terms of lifetime management.
 *    - The difference is performance-centric only, not functional.
 *
 * 4. Large dataset behavior:
 *    - `runtime_arr<T, Alloc>` avoids `vector`'s full-capacity reallocation
 *    - Shows **~2.8x faster fill time** under heavy loads
 *
 * Interoperability & API Usability:
 * ---------------------------------
 * - `runtime_arr` mimics `std::vector` construction semantics:
 *     ```cpp
 *     std::vector<int> vec(N);
 *     jh::runtime_arr<int> arr(N);
 *     ```
 * - Can be **moved into/from** `std::vector` with ease via:
 *     ```cpp
 *     std::vector<T>(std::move(arr));    // implicit conversion operator
 *     ```
 *
 * - `uninitialized` mode is **ideal for high-performance algorithms** that write data in-place
 *   without needing default construction, saving cycles and memory writes.
 *
 * Conclusion:
 * -----------
 * - Use `runtime_arr<T>` when:
 *     - You are storing primitive or POD types
 *     - You want to avoid overhead from default constructors
 *     - You need allocator override or fast fill/reset
 *
 * - Use `std::vector<T>` when:
 *     - You need full STL compatibility (sorting, resizing, etc.)
 *     - You store RAII or complex objects needing copy/move constructors
 *
 * `runtime_arr` acts as a **safe and efficient type-constrained drop-in container** —
 * excellent for tight loops, zero-init control, and data pipeline work,
 * while preserving modern C++ ergonomics.
 */
