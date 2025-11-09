/**
 * @file example_awaker.cpp
 * @brief Process that sleeps briefly, then notifies all waiters.
 */

#include "jh/synchronous/ipc/process_condition.h"
#include <thread>
#include <chrono>

using namespace std::chrono_literals;
using cond_t = jh::sync::ipc::process_condition<"demo_condition">;

int main() {
    auto &cond = cond_t::instance();

    std::this_thread::sleep_for(500ms);
    cond.notify_all();

    return 0;
}
