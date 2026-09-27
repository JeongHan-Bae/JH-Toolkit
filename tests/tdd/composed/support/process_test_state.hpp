/**
 * @file process_test_state.hpp
 * @brief Named IPC objects shared by process synchronization test workers.
 */

#pragma once

#include <cstdint>

#include "jh/synchronous/ipc/process_cond_var.h"
#include "jh/synchronous/ipc/process_counter.h"
#include "jh/synchronous/ipc/process_mutex.h"

namespace test::ipc_state {
    inline constexpr std::uint64_t flow_idle = 0;
    inline constexpr std::uint64_t flow_gate_locked = 1u << 0;
    inline constexpr std::uint64_t flow_sender_attempted = 1u << 1;
    inline constexpr std::uint64_t flow_scheduler_acknowledged = 1u << 2;
    inline constexpr std::uint64_t flow_target_dispatched = 1u << 3;
    inline constexpr std::uint64_t flow_target_notified = 1u << 4;
    inline constexpr std::uint64_t flow_gate_released = 1u << 5;
    inline constexpr std::uint64_t flow_sender_completed = 1u << 6;
    inline constexpr std::uint64_t flow_target_completed = 1u << 7;
    inline constexpr std::uint64_t flow_target_ready = 1u << 8;
    inline constexpr std::uint64_t flow_failed = 1ull << 63;

    inline constexpr std::uint64_t flow_complete =
        flow_gate_locked | flow_sender_attempted | flow_scheduler_acknowledged |
        flow_target_dispatched | flow_target_notified | flow_gate_released |
        flow_sender_completed | flow_target_completed | flow_target_ready;

    using mutex_t = jh::sync::ipc::process_mutex<"jhtdd.mutex", true>;
    using mutex_total_t = jh::sync::ipc::process_counter<"jhtdd.mutex.total", true>;
    using mutex_active_t = jh::sync::ipc::process_counter<"jhtdd.mutex.active", true>;
    using mutex_overlap_t = jh::sync::ipc::process_counter<"jhtdd.mutex.overlap", true>;

    using scheduler_gate_t = jh::sync::ipc::process_mutex<"jhtdd.cond.gate", true>;
    using worker_gate_t = jh::sync::ipc::process_mutex<"jhtdd.cond.worker.gate", true>;
    using scheduler_condition_t = jh::sync::ipc::process_cond_var<"jhtdd.scheduler", true>;
    using worker_condition_t = jh::sync::ipc::process_cond_var<"jhtdd.worker", true>;
    using process_stage_t = jh::sync::ipc::process_counter<"jhtdd.flow.stage", true>;

    inline void set_flow_bits(const std::uint64_t bits) {
        process_stage_t::instance().fetch_apply([bits](const std::uint64_t current) {
            return current | bits;
        });
    }

    [[nodiscard]] inline std::uint64_t flow_snapshot() {
        return process_stage_t::instance().load_force();
    }

    [[nodiscard]] inline bool flow_has(const std::uint64_t bits) {
        return (flow_snapshot() & bits) == bits;
    }
}
