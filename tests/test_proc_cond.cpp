#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

#define JH_ALLOW_PARENT_PATH 1

#include "jh/synchronous/ipc/process_condition.h"
#include "jh/synchronous/ipc/process_launcher.h"
#include <chrono>
#include <iostream>
#include <vector>

/**
 * @file test_proc_cond.cpp
 * @brief Tests inter-process synchronization of process_condition.
 *
 * <h3>Behavior</h3>
 * <ul>
 *   <li>Start N sleeper processes (waiting on the same named condition).</li>
 *   <li>Start 1 awaker process that sleeps 500 ms, then calls notify_all().</li>
 *   <li>Measure total wake-up duration — should be ≥ 500 ms but ≪ N × 500 ms.</li>
 * </ul>
 */

using cond_t = jh::sync::ipc::process_condition<"demo_condition">;
using priv_cond_t = jh::sync::ipc::process_condition<"demo_condition", true>;
using sleeper_launcher = jh::sync::ipc::process_launcher<"../examples/process_lock/sleeper">;
using awaker_launcher  = jh::sync::ipc::process_launcher<"../examples/process_lock/awaker">;

TEST_CASE("process_condition notify_all wakes multiple sleepers") {
    constexpr int sleeper_count = 4;

    // Launch N sleepers (they will block on wait_signal)
    std::vector<decltype(sleeper_launcher::start())> handles;
    handles.reserve(sleeper_count);

    for (int i = 0; i < sleeper_count; ++i) {
        handles.push_back(sleeper_launcher::start());
    }

    auto start = std::chrono::steady_clock::now();
    // start awaker (sleeps 500ms, then notify_all)
    auto aw = awaker_launcher::start();

    // Wait for all processes to complete
    aw.wait();
    for (auto &h : handles) {
        h.wait();
    }

    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start).count();

    std::cout << "[test] total wake time = " << elapsed_ms << " ms\n";

    // check timing
    REQUIRE(elapsed_ms >= 500);
    REQUIRE(elapsed_ms < sleeper_count * 500);

    // unlink shared condition
    priv_cond_t::unlink();
}
