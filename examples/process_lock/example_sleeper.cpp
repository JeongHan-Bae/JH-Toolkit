/**
 * @file example_sleeper.cpp
 * @brief Worker process that waits on a shared process_cond_var.
 */

#include <jh/ipc>

using cond_t = jh::ipc::process_cond_var<"demo_cond_var">;

int main() {
    auto &cond = cond_t::instance();
    cond.wait_signal();
    return 0;
}
