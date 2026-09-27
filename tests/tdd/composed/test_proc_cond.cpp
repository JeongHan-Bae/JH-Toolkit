#define TINY_TEST_MAIN
#include "jh/test/tiny_test/tiny_test.hpp"

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

#include <chrono>
#include <mutex>
#include <thread>

#include "jh/synchronous/ipc/process_launcher.h"
#include "support/process_test_state.hpp"

namespace {
    using scheduler_launcher = jh::sync::ipc::process_launcher<"tdd_ipc_condition_scheduler">;
    using sender_launcher = jh::sync::ipc::process_launcher<"tdd_ipc_condition_sender">;
    using target_launcher = jh::sync::ipc::process_launcher<"tdd_ipc_condition_target">;

    template<class Object>
    void unlink_safely() noexcept {
        try {
            Object::unlink();
        } catch (...) {
        }
    }

    struct ipc_cleanup final {
        ~ipc_cleanup() {
            unlink_safely<test::ipc_state::scheduler_gate_t>();
            unlink_safely<test::ipc_state::worker_gate_t>();
            unlink_safely<test::ipc_state::scheduler_condition_t>();
            unlink_safely<test::ipc_state::worker_condition_t>();
            unlink_safely<test::ipc_state::process_stage_t>();
        }
    };
}

namespace test {
    void process_condition_notification_chain() {
        ipc_cleanup cleanup;

        ipc_state::scheduler_gate_t::instance();
        ipc_state::scheduler_condition_t::instance();
        ipc_state::worker_condition_t::instance();
        auto& stage = ipc_state::process_stage_t::instance();
        stage.store(ipc_state::flow_idle);

        auto& worker_gate = ipc_state::worker_gate_t::instance();
        auto target = target_launcher::start();
        const auto target_start_deadline = std::chrono::steady_clock::now() + std::chrono::seconds{3};
        while (!ipc_state::flow_has(ipc_state::flow_target_ready) &&
               !ipc_state::flow_has(ipc_state::flow_failed) &&
               std::chrono::steady_clock::now() < target_start_deadline) {
            std::this_thread::yield();
        }

        const bool target_started = ipc_state::flow_has(ipc_state::flow_target_ready) &&
                                    !ipc_state::flow_has(ipc_state::flow_failed);
        jh::test::tiny_test::expect(target_started, "target starts before the scheduler dispatches it");
        if (!target_started) {
            ipc_state::set_flow_bits(ipc_state::flow_failed);
            ipc_state::worker_condition_t::instance().notify_all();
            [[maybe_unused]] const auto target_result = target.wait();
            return;
        }

        // Acquiring the gate proves the target has atomically entered its condition wait.
        worker_gate.lock();
        worker_gate.unlock();

        auto scheduler = scheduler_launcher::start();
        const auto scheduler_start_deadline = std::chrono::steady_clock::now() + std::chrono::seconds{3};
        while (!ipc_state::flow_has(ipc_state::flow_gate_locked) &&
               !ipc_state::flow_has(ipc_state::flow_failed) &&
               std::chrono::steady_clock::now() < scheduler_start_deadline) {
            std::this_thread::yield();
        }

        const bool scheduler_started = ipc_state::flow_has(ipc_state::flow_gate_locked) &&
                                       !ipc_state::flow_has(ipc_state::flow_failed);
        jh::test::tiny_test::expect(scheduler_started, "scheduler starts before the sender notifies it");
        if (!scheduler_started) {
            ipc_state::set_flow_bits(ipc_state::flow_failed);
            ipc_state::scheduler_condition_t::instance().notify_all();
            ipc_state::worker_condition_t::instance().notify_all();
            [[maybe_unused]] const auto scheduler_result = scheduler.wait();
            [[maybe_unused]] const auto target_result = target.wait();
            return;
        }

        // The scheduler publishes readiness while holding this gate, then atomically releases it to wait.
        auto& scheduler_gate = ipc_state::scheduler_gate_t::instance();
        scheduler_gate.lock();
        scheduler_gate.unlock();

        auto sender = sender_launcher::start();

        const auto scheduler_result = scheduler.wait();
        const auto sender_result = sender.wait();
        const auto target_result = target.wait();

        jh::test::tiny_test::expect(
            scheduler_result.has_value() && scheduler_result.value() == 0,
            "scheduler exits normally"
        );
        jh::test::tiny_test::expect(sender_result.has_value() && sender_result.value() == 0,
                                   "sender exits normally");
        jh::test::tiny_test::expect(target_result.has_value() && target_result.value() == 0,
                                   "target exits normally");

        jh::test::tiny_test::expect(
            stage.load_force() == ipc_state::flow_complete,
            "the scheduler receives one worker notification and notifies the next worker"
        );
    }
}

template<>
struct jh::test::tiny_test::test<"process_cond_var drives a scheduler notification chain">
    : jh::test::tiny_test::test_definition<
          "process_cond_var drives a scheduler notification chain",
          &::test::process_condition_notification_chain
      > {};

template<>
struct jh::test::tiny_test::session<"process_cond_var">
    : jh::test::tiny_test::session_definition<
          "process_cond_var",
          jh::test::tiny_test::test<"process_cond_var drives a scheduler notification chain">
      > {};

namespace test {
    [[maybe_unused]] const tiny_test::session<"process_cond_var"> process_cond_var_session{};
}
