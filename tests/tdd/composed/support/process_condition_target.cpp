#include <chrono>
#include <mutex>

#include "process_test_state.hpp"

int main() {
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds{5};
    auto &gate = test::ipc_state::worker_gate_t::instance();
    auto &scheduler_condition = test::ipc_state::scheduler_condition_t::instance();
    auto &worker_condition = test::ipc_state::worker_condition_t::instance();

    std::unique_lock gate_lock{gate};
    test::ipc_state::set_flow_bits(test::ipc_state::flow_target_ready);
    while (!test::ipc_state::flow_has(test::ipc_state::flow_target_dispatched)) {
        if (test::ipc_state::flow_has(test::ipc_state::flow_failed)) {
            scheduler_condition.notify_all();
            return 31;
        }
        if (!worker_condition.wait_until(gate_lock, deadline)) {
            test::ipc_state::set_flow_bits(test::ipc_state::flow_failed);
            worker_condition.notify_all();
            scheduler_condition.notify_all();
            return 32;
        }
    }

    test::ipc_state::set_flow_bits(test::ipc_state::flow_target_notified);
    scheduler_condition.notify_one();
    gate_lock.unlock();
    test::ipc_state::set_flow_bits(test::ipc_state::flow_target_completed);
    return 0;
}
