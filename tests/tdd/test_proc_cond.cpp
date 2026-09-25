#include <catch2/catch_all.hpp>

#define JH_INTERPROCESS_ALLOW_PARENT_PATH 1

#include "jh/synchronous/ipc/process_cond_var.h"
#include "jh/synchronous/ipc/process_launcher.h"
#include <vector>

/**
 * @file test_proc_cond.cpp
 * @brief Tests inter-process synchronization of process_cond_var.
 *
 * <h3>Behavior</h3>
 * <ul>
 *   <li>Start N sleeper processes (waiting on the same named condition).</li>
 *   <li>Start 1 awaker process that sleeps 500 ms, then calls notify_all().</li>
 *   <li>Verify that every sleeper exits after the awaker notifies all waiters.</li>
 * </ul>
 */

using cond_t = jh::sync::ipc::process_cond_var<"demo_cond_var">;
using priv_cond_t = jh::sync::ipc::process_cond_var<"demo_cond_var", true>;
using sleeper_launcher = jh::sync::ipc::process_launcher<"../examples/process_lock/sleeper">;
using awaker_launcher  = jh::sync::ipc::process_launcher<"../examples/process_lock/awaker">;

TEST_CASE("process_cond_var notify_all wakes multiple sleepers") {

    constexpr int sleeper_count = 4;

    // Launch N sleepers (they will block on wait_signal)
    std::vector<decltype(sleeper_launcher::start())> handles;
    handles.reserve(sleeper_count);

    for (int i = 0; i < sleeper_count; ++i) {
        handles.push_back(sleeper_launcher::start());
    }

    // Start an awaker that signals all sleepers after they have been launched.
    auto aw = awaker_launcher::start();

    // Wait for all processes to complete
    aw.wait();
    for (auto &h : handles) {
        h.wait();
    }

    // unlink shared condition
    priv_cond_t::unlink();
}
