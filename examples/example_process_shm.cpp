/**
 * @file example_process_shm.cpp
 * @brief Demonstrations of inter-process synchronization primitives:
 *        - process_counter (shared-memory atomic counter)
 *        - process_cond_var (cross-process condition variable)
 *        - process_shm_obj (shared POD object across processes)
 */

#include "jh/macros/platform.h"
#include "ensure_output.h"  // NOLINT for Windows output
#include "jh/synchronous/ipc/process_counter.h"
#include "jh/synchronous/ipc/process_cond_var.h"
#include "jh/synchronous/ipc/process_shm_obj.h"
#include "jh/synchronous/ipc/process_launcher.h"
#include "jh/pod"

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <cmath>

#if IS_WINDOWS
static EnsureOutput ensure_output_setup;
#endif

// -----------------------------------------------------------------------------
// Shared types
// -----------------------------------------------------------------------------
using counter_t       = jh::sync::ipc::process_counter<"demo_counter">;
using priv_counter_t  = jh::sync::ipc::process_counter<"demo_counter", true>;
using cond_t          = jh::sync::ipc::process_cond_var<"demo_cond_var">;
using priv_cond_t     = jh::sync::ipc::process_cond_var<"demo_cond_var", true>;

// Define shared POD type
JH_POD_STRUCT(DemoPod,
    std::uint64_t xor_field;
    std::uint64_t add_field;
    double        mul_field;
);

using shm_t      = jh::sync::ipc::process_shm_obj<"demo_shared_pod", DemoPod>;
using priv_shm_t = jh::sync::ipc::process_shm_obj<"demo_shared_pod", DemoPod, true>;

using counter_launcher  = jh::sync::ipc::process_launcher<"process_lock/counter">;
using sleeper_launcher  = jh::sync::ipc::process_launcher<"process_lock/sleeper">;
using awaker_launcher   = jh::sync::ipc::process_launcher<"process_lock/awaker">;
using pod_writer_launcher = jh::sync::ipc::process_launcher<"process_lock/pod_writer">;

// -----------------------------------------------------------------------------
// Example 1: Shared counter (process_counter)
// -----------------------------------------------------------------------------
void run_counter_example() {
    std::cout << "\n==================== process_counter example ====================\n";

    constexpr int worker_count = 4;
    constexpr int increments_per_worker = 20'000;

    counter_t::instance().store(0);
    std::cout << "Launching " << worker_count << " counter workers...\n";

    std::vector<decltype(counter_launcher::start())> handles;
    handles.reserve(worker_count);

    for (int i = 0; i < worker_count; ++i)
        handles.push_back(counter_launcher::start());

    for (auto &h : handles)
        h.wait();

    std::cout << "All counter processes finished.\n";

    auto total = counter_t::instance().load_strong();
    std::uint64_t expected = worker_count * increments_per_worker;

    std::cout << "Total = " << total
              << " (expected " << expected << ")\n";

    auto old = counter_t::instance().fetch_apply([](std::uint64_t v) { return v + 10; });
    std::cout << "fetch_apply(+10): old=" << old
              << ", new=" << counter_t::instance().load_strong() << "\n";

    counter_t::instance().store(12345);
    std::cout << "store(12345), load_force() = "
              << counter_t::instance().load_force() << "\n";

    priv_counter_t::unlink();
    std::cout << "Unlinked shared counter.\n";
}

// -----------------------------------------------------------------------------
// Example 2: Cross-process condition variable (process_cond_var)
// -----------------------------------------------------------------------------
void run_cond_var_example() {
    std::cout << "\n==================== process_cond_var example ====================\n";

    constexpr int sleeper_count = 4;
    using namespace std::chrono;

    std::vector<decltype(sleeper_launcher::start())> handles;
    handles.reserve(sleeper_count);

    for (int i = 0; i < sleeper_count; ++i)
        handles.push_back(sleeper_launcher::start());

    auto start = steady_clock::now();
    auto aw = awaker_launcher::start();

    aw.wait();
    for (auto &h : handles)
        h.wait();

    auto elapsed_ms = duration_cast<milliseconds>(steady_clock::now() - start).count();
    auto min_expected = 500;
    auto max_expected = sleeper_count * 500;

    std::cout << "Expected wake range: [" << min_expected
              << ", " << max_expected << "] ms\n";
    std::cout << "Observed wake time : " << elapsed_ms << " ms\n";

    if (elapsed_ms >= min_expected && elapsed_ms < max_expected)
        std::cout << "→ Wake timing is within expected range.\n";
    else
        std::cout << "→ Wake timing outside expected range (possible contention).\n";

    priv_cond_t::unlink();
    std::cout << "Unlinked shared condition.\n";
}

// -----------------------------------------------------------------------------
// Example 3: Shared POD object (process_shm_obj)
// -----------------------------------------------------------------------------
void run_shared_pod_example() {
    std::cout << "\n==================== process_shm_obj example ====================\n";

    auto &shm = shm_t::instance();

    // Initialize shared object
    {
        std::lock_guard guard(shm.lock());
        shm.flush_acquire();
        shm.ref() = DemoPod{0, 0, 1.0};
        shm.flush_seq();
    }

    constexpr int writer_count = 4;
    std::cout << "Launching " << writer_count << " POD writer processes...\n";

    std::vector<decltype(pod_writer_launcher::start())> writers;
    writers.reserve(writer_count);

    for (int i = 0; i < writer_count; ++i)
        writers.push_back(pod_writer_launcher::start());

    for (auto &w : writers)
        w.wait();

    // Verify results
    {
        shm.flush_acquire();
        const auto &ref = shm.ref();

        constexpr std::uint64_t add_inc  = 10;
        constexpr double mul_factor      = 1.0001;
        constexpr int iterations         = 20'000;

        std::uint64_t expected_add = add_inc * iterations * writer_count;
        double expected_mul = std::pow(mul_factor, iterations * writer_count);

        std::cout << "xor_field = " << ref.xor_field
                  << " (expected invariant 0)\n";
        std::cout << "add_field = " << ref.add_field
                  << " (expected " << expected_add << ")\n";
        std::cout << "mul_field = " << ref.mul_field
                  << " (expected ≈ " << expected_mul << ")\n";
    }

    priv_shm_t::unlink();
    std::cout << "Unlinked shared POD memory.\n";
}

// -----------------------------------------------------------------------------
// Main entry: run all IPC examples
// -----------------------------------------------------------------------------
int main() {
    try {
        run_counter_example();
        run_cond_var_example();
        run_shared_pod_example();

        std::cout << "\nAll shared-memory synchronization examples completed successfully.\n";
    } catch (const std::exception &ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}