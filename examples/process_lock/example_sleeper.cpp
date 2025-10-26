/**
 * @file example_sleeper.cpp
 * @brief Worker process that waits on a shared process_condition.
 */

#include "jh/asynchronous/process_condition.h"

using cond_t = jh::async::ipc::process_condition<"demo_condition">;

int main() {
    auto &cond = cond_t::instance();
    cond.wait_signal();
    return 0;
}
