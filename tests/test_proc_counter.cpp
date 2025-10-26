#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

#define JH_ALLOW_PARENT_PATH 1

#include "jh/asynchronous/process_counter.h"
#include "jh/asynchronous/process_launcher.h"
#include <iostream>
#include <vector>

/**
 * @file test_proc_counter.cpp
 * @brief Test suite verifying process_counter cross-process synchronization.
 *
 * <h3>Overview</h3>
 * <ul>
 *   <li>Spawns multiple counter worker processes using process_launcher.</li>
 *   <li>Each worker performs 200_000 increments on the shared counter.</li>
 *   <li>After all workers exit, verifies that total = N × 200_000.</li>
 *   <li>Demonstrates correctness of fetch_apply(), store(), and load_force().</li>
 * </ul>
 */

using counter_t      = jh::async::ipc::process_counter<"demo_counter">;
using priv_counter_t = jh::async::ipc::process_counter<"demo_counter", true>;
using counter_launcher = jh::async::ipc::process_launcher<"../examples/process_lock/counter">;

TEST_CASE("process_counter basic multi-process behavior") {
    constexpr int worker_count = 4;
    constexpr int increments_per_worker = 200'000;

    // Reset counter to 0 before starting
    counter_t::instance().store(0);

    std::vector<decltype(counter_launcher::start())> handles;
    handles.reserve(worker_count);

    for (int i = 0; i < worker_count; ++i) {
        handles.push_back(counter_launcher::start());
    }

    for (auto &h : handles) {
        h.wait();
    }

    // ---- Verify shared counter value ----
    auto total = counter_t::instance().load_strong();
    REQUIRE(total == worker_count * increments_per_worker);

    // ---- API behavior demonstration ----
    auto old = counter_t::instance().fetch_apply([](std::uint64_t v) { return v + 10; });
    REQUIRE(counter_t::instance().load_strong() == old + 10);

    counter_t::instance().store(9999);
    REQUIRE(counter_t::instance().load_force() == 9999);

    // Cleanup shared memory
    priv_counter_t::unlink();
}
