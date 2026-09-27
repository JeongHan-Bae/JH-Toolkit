#include <chrono>
#include <mutex>

#include "process_test_state.hpp"

int main() {
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds{5};
    auto &gate = test::ipc_state::scheduler_gate_t::instance();
    auto &scheduler_condition = test::ipc_state::scheduler_condition_t::instance();
    auto &worker_gate = test::ipc_state::worker_gate_t::instance();
    auto &worker_condition = test::ipc_state::worker_condition_t::instance();

    std::unique_lock scheduler_lock{gate};
    test::ipc_state::set_flow_bits(test::ipc_state::flow_gate_locked);

    while (!test::ipc_state::flow_has(test::ipc_state::flow_sender_attempted)) {
        if (test::ipc_state::flow_has(test::ipc_state::flow_failed)) return 22;
        if (!scheduler_condition.wait_until(scheduler_lock, deadline)) {
            test::ipc_state::set_flow_bits(test::ipc_state::flow_failed);
            scheduler_condition.notify_all();
            worker_condition.notify_all();
            return 23;
        }
    }

    test::ipc_state::set_flow_bits(test::ipc_state::flow_scheduler_acknowledged);
    scheduler_lock.unlock();
    test::ipc_state::set_flow_bits(test::ipc_state::flow_gate_released);

    std::unique_lock worker_lock{worker_gate};
    test::ipc_state::set_flow_bits(test::ipc_state::flow_target_dispatched);
    worker_condition.notify_one();

    while (!test::ipc_state::flow_has(test::ipc_state::flow_target_notified)) {
        if (test::ipc_state::flow_has(test::ipc_state::flow_failed)) return 24;
        if (!scheduler_condition.wait_until(worker_lock, deadline)) {
            test::ipc_state::set_flow_bits(test::ipc_state::flow_failed);
            worker_condition.notify_all();
            scheduler_condition.notify_all();
            return 25;
        }
    }

    return 0;
}
