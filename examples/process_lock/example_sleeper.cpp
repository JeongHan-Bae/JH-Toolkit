/**
 * @file example_sleeper.cpp
 * @brief Worker process that waits on a shared process_cond_var.
 */

#include "jh/synchronous/ipc/process_cond_var.h"

using cond_t = jh::sync::ipc::process_cond_var<"demo_cond_var">;

int main() {
    auto &cond = cond_t::instance();
    cond.wait_signal();
    return 0;
}
