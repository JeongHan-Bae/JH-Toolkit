/**
 * @file example_process_counter_worker.cpp
 * @brief Worker process that increments a shared counter aggressively.
 */

#include "jh/asynchronous/process_counter.h"


using counter_t = jh::async::ipc::process_counter<"demo_counter">;

int main() {
    auto &c = counter_t::instance();

    constexpr int iterations = 200'000;
    for (int i = 0; i < iterations; ++i) {
        c.fetch_add(1);
    }

    // optional small sanity operation
    c.fetch_sub(0);

    return 0;
}
