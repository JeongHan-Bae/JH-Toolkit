#include <mutex>
#include <thread>

#include "process_test_state.hpp"

int main() {
    auto& mutex = test::ipc_state::mutex_t::instance();
    auto& total = test::ipc_state::mutex_total_t::instance();
    auto& active = test::ipc_state::mutex_active_t::instance();
    auto& overlap = test::ipc_state::mutex_overlap_t::instance();

    constexpr int iterations = 24;
    for (int iteration = 0; iteration < iterations; ++iteration) {
        std::lock_guard guard{mutex};
        if (active.fetch_add() != 0) overlap.fetch_add();
        std::this_thread::yield();
        total.fetch_add();
        active.fetch_sub();
    }

    return 0;
}
