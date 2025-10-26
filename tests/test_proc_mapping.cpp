#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

#define JH_ALLOW_PARENT_PATH 1

#include "jh/asynchronous/process_counter.h"
#include "jh/asynchronous/shared_process_memory.h"
#include "jh/asynchronous/process_launcher.h"
#include "jh/pod"

#include <cmath>
#include <vector>
#include <iostream>

/**
 * @file test_proc_mapping.cpp
 * @brief Verifies correctness of inter-process shared-memory primitives:
 *        - `process_counter`
 *        - `shared_process_memory`
 *
 * <h3>Overview</h3>
 * <ul>
 *   <li>Spawns multiple worker processes that update shared objects concurrently.</li>
 *   <li>Checks final consistency of shared values across processes.</li>
 *   <li>Verifies correctness of synchronization and visibility fences.</li>
 * </ul>
 */

// -----------------------------------------------------------------------------
// process_counter setup
// -----------------------------------------------------------------------------
using counter_t       = jh::async::ipc::process_counter<"demo_counter">;
using priv_counter_t  = jh::async::ipc::process_counter<"demo_counter", true>;
using counter_launcher = jh::async::ipc::process_launcher<"../examples/process_lock/counter">;

// -----------------------------------------------------------------------------
// shared_process_memory setup
// -----------------------------------------------------------------------------
JH_POD_STRUCT(DemoPod,
    std::uint64_t xor_field;
    std::uint64_t add_field;
    double        mul_field;
);

using shm_t      = jh::async::ipc::shared_process_memory<"demo_shared_pod", DemoPod>;
using priv_shm_t = jh::async::ipc::shared_process_memory<"demo_shared_pod", DemoPod, true>;
using pod_writer_launcher = jh::async::ipc::process_launcher<"../examples/process_lock/pod_writer">;

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------
constexpr int WORKER_COUNT   = 4;
constexpr int ITERATIONS     = 200'000;
constexpr std::uint64_t ADD_INC  = 10;
constexpr double MUL_FACTOR      = 1.0001;

// -----------------------------------------------------------------------------
// Section 1: process_counter test
// -----------------------------------------------------------------------------
TEST_CASE("process_counter cross-process accumulation") {
    counter_t::instance().store(0);

    std::vector<decltype(counter_launcher::start())> handles;
    handles.reserve(WORKER_COUNT);
    for (int i = 0; i < WORKER_COUNT; ++i)
        handles.push_back(counter_launcher::start());

    for (auto &h : handles)
        h.wait();

    auto total = counter_t::instance().load_strong();
    REQUIRE(total == WORKER_COUNT * ITERATIONS);

    // API demonstration
    auto old = counter_t::instance().fetch_apply([](std::uint64_t v) { return v + 10; });
    REQUIRE(counter_t::instance().load_strong() == old + 10);

    counter_t::instance().store(777);
    REQUIRE(counter_t::instance().load_force() == 777);

    // Cleanup
    priv_counter_t::unlink();
}

// -----------------------------------------------------------------------------
// Section 2: shared_process_memory test
// -----------------------------------------------------------------------------
TEST_CASE("shared_process_memory consistency across processes") {
    auto &shm = shm_t::instance();

    // Initialize shared memory
    {
        std::lock_guard guard(shm.lock());
        shm.flush_acquire();
        shm.ref() = DemoPod{0, 0, 1.0};
        shm.flush_seq();
    }

    // Launch multiple POD writers
    std::vector<decltype(pod_writer_launcher::start())> writers;
    writers.reserve(WORKER_COUNT);

    for (int i = 0; i < WORKER_COUNT; ++i)
        writers.push_back(pod_writer_launcher::start());

    for (auto &w : writers)
        w.wait();

    // Verify shared result
    {
        shm.flush_acquire();
        const auto &ref = shm.ref();

        std::uint64_t expected_add = ADD_INC * ITERATIONS * WORKER_COUNT;
        double expected_mul = std::pow(MUL_FACTOR, ITERATIONS * WORKER_COUNT);

        // XOR field should remain invariant (even XORs cancel)
        REQUIRE(ref.xor_field == 0);

        // Addition field must match exactly
        REQUIRE(ref.add_field == expected_add);

        // Multiplication field must be approximately equal (floating-point drift allowed)
        REQUIRE(ref.mul_field == Catch::Approx(expected_mul).epsilon(1e-6));
    }

    // Cleanup
    priv_shm_t::unlink();
}