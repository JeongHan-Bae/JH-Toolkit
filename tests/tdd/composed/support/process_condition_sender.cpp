#include <mutex>

#include "process_test_state.hpp"

int main() {
    auto &gate = test::ipc_state::scheduler_gate_t::instance();
    auto &scheduler_condition = test::ipc_state::scheduler_condition_t::instance();

    std::unique_lock gate_lock{gate};
    test::ipc_state::set_flow_bits(test::ipc_state::flow_sender_attempted);
    scheduler_condition.notify_one();
    gate_lock.unlock();
    test::ipc_state::set_flow_bits(test::ipc_state::flow_sender_completed);
    return 0;
}
