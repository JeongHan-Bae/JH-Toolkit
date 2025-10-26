/**
 * @file example_process_shm.cpp
 * @brief Demonstrations of inter-process synchronization primitives:
 *        - process_counter (shared-memory atomic counter)
 *        - process_condition (cross-process condition variable)
 */

#include "jh/macros/platform.h"
#include "ensure_output.h"  // NOLINT for Windows output
#include "jh/asynchronous/process_counter.h"
#include "jh/asynchronous/process_condition.h"
#include "jh/asynchronous/process_launcher.h"

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

#if IS_WINDOWS
static EnsureOutput ensure_output_setup;
#endif

// -----------------------------------------------------------------------------
// Shared types
// -----------------------------------------------------------------------------
using counter_t       = jh::async::ipc::process_counter<"demo_counter">;
using priv_counter_t  = jh::async::ipc::process_counter<"demo_counter", true>;
using cond_t          = jh::async::ipc::process_condition<"demo_condition">;
using priv_cond_t     = jh::async::ipc::process_condition<"demo_condition", true>;

using counter_launcher  = jh::async::ipc::process_launcher<"process_lock/counter">;
using sleeper_launcher  = jh::async::ipc::process_launcher<"process_lock/sleeper">;
using awaker_launcher   = jh::async::ipc::process_launcher<"process_lock/awaker">;

// -----------------------------------------------------------------------------
// Example 1: Shared counter (process_counter)
// -----------------------------------------------------------------------------
void run_counter_example() {
    std::cout << "\n==================== process_counter example ====================\n";

    constexpr int worker_count = 4;
    constexpr int increments_per_worker = 200'000;

    counter_t::instance().store(0);

    std::cout << "Launching " << worker_count << " counter workers...\n";


    std::vector<decltype(counter_launcher::start())> handles;
    handles.reserve(worker_count);

    for (int i = 0; i < worker_count; ++i) {
        handles.push_back(counter_launcher::start());
    }

    for (auto &h : handles) {
        h.wait();
    }


    std::cout << "All counter processes finished.\n";

    auto total = counter_t::instance().load_strong();
    std::uint64_t expected = worker_count * increments_per_worker;

    std::cout << "Total = " << total
              << " (expected " << expected << ")\n";

    // Demonstrate API usage
    auto old = counter_t::instance().fetch_apply([](std::uint64_t v) { return v + 10; });
    std::cout << "fetch_apply(+10): old=" << old
              << ", new=" << counter_t::instance().load_strong() << "\n";

    counter_t::instance().store(12345);
    std::cout << "store(12345), load_force() = "
              << counter_t::instance().load_force() << "\n";

    // Cleanup
    priv_counter_t::unlink();
    std::cout << "Unlinked shared counter.\n";
}

// -----------------------------------------------------------------------------
// Example 2: Cross-process condition variable (process_condition)
// -----------------------------------------------------------------------------
void run_condition_example() {
    std::cout << "\n==================== process_condition example ====================\n";

    constexpr int sleeper_count = 4;
    using namespace std::chrono;

    // Launch N sleepers (they will block on wait_signal)
    std::vector<decltype(sleeper_launcher::start())> handles;
    handles.reserve(sleeper_count);

    for (int i = 0; i < sleeper_count; ++i) {
        handles.push_back(sleeper_launcher::start());
    }

    // Start timing *after* all sleepers are likely waiting
    auto start = steady_clock::now();

    // Launch awaker (sleeps 500ms, then notify_all)
    auto aw = awaker_launcher::start();

    // Wait for all processes to complete
    aw.wait();
    for (auto &h : handles) {
        h.wait();
    }

    auto elapsed_ms = duration_cast<milliseconds>(steady_clock::now() - start).count();

    // Expected wake timing: sleepers wake ~simultaneously after awaker's 500ms delay
    auto min_expected = 500;
    auto max_expected = sleeper_count * 500;

    std::cout << "Expected wake range: [" << min_expected
              << ", " << max_expected << "] ms\n";
    std::cout << "Observed wake time : " << elapsed_ms << " ms\n";

    if (elapsed_ms >= min_expected && elapsed_ms < max_expected)
        std::cout << "→ Wake timing is within expected range.\n";
    else
        std::cout << "→ Wake timing outside expected range (possible contention).\n";

    // Cleanup shared condition object
    priv_cond_t::unlink();
    std::cout << "Unlinked shared condition.\n";
}


// -----------------------------------------------------------------------------
// Main entry: run all IPC examples
// -----------------------------------------------------------------------------
int main() {
    try {
        run_counter_example();
        run_condition_example();

        std::cout << "\nAll shared-memory synchronization examples completed successfully.\n";
    } catch (const std::exception &ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}

