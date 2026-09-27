#define TINY_TEST_MAIN
#include "jh/test/tiny_test/tiny_test.hpp"

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

#include <chrono>
#include <cstdint>
#include <vector>

#include "jh/macros/platform.h"
#if IS_POSIX
#include <csignal>
#endif
#include "jh/synchronous/ipc/process_launcher.h"
#include "support/process_test_state.hpp"

namespace {
    using try_mutex_t = jh::sync::ipc::process_mutex<"jhtdd.try", true>;
    using mutex_worker = jh::sync::ipc::process_launcher<"tdd_ipc_mutex_worker">;
    using nonzero_exit_worker = jh::sync::ipc::process_launcher<"tdd_ipc_process_exit_worker">;
    using abnormal_exit_worker = jh::sync::ipc::process_launcher<"tdd_ipc_abnormal_worker">;

    template<class Object>
    void unlink_safely() noexcept {
        try {
            Object::unlink();
        } catch (...) {
        }
    }

    struct ipc_cleanup final {
        ~ipc_cleanup() {
            unlink_safely<try_mutex_t>();
            unlink_safely<test::ipc_state::mutex_t>();
            unlink_safely<test::ipc_state::mutex_total_t>();
            unlink_safely<test::ipc_state::mutex_active_t>();
            unlink_safely<test::ipc_state::mutex_overlap_t>();
        }
    };
}

namespace test {
    void process_mutex_contracts() {
        ipc_cleanup cleanup;

        auto& try_mutex = try_mutex_t::instance();
        try_mutex.lock();

        const bool immediate_relock = try_mutex.try_lock();
        jh::test::tiny_test::expect_not(immediate_relock, "process mutex rejects an immediate relock");
        if (immediate_relock) try_mutex.unlock();

        const bool timed_relock = try_mutex.try_lock_for(std::chrono::milliseconds{30});
        jh::test::tiny_test::expect_not(timed_relock, "process mutex times out while held");
        if (timed_relock) try_mutex.unlock();

        try_mutex.unlock();
        const bool acquired_after_unlock = try_mutex.try_lock_for(std::chrono::milliseconds{100});
        jh::test::tiny_test::expect(acquired_after_unlock, "process mutex is available after unlock");
        if (acquired_after_unlock) try_mutex.unlock();

        auto& total = test::ipc_state::mutex_total_t::instance();
        auto& active = test::ipc_state::mutex_active_t::instance();
        auto& overlap = test::ipc_state::mutex_overlap_t::instance();
        total.store(0);
        active.store(0);
        overlap.store(0);

        constexpr int worker_count = 4;
        constexpr std::uint64_t operations_per_worker = 24;
        std::vector<decltype(mutex_worker::start())> workers;
        workers.reserve(worker_count);
        for (int worker = 0; worker < worker_count; ++worker) {
            workers.push_back(mutex_worker::start());
        }
        for (auto& worker : workers) {
            const auto result = worker.wait();
            jh::test::tiny_test::expect(
                result.has_value() && result.value() == 0,
                "critical-section worker exits normally with code zero"
            );
        }

        jh::test::tiny_test::expect(
            total.load_strong() == worker_count * operations_per_worker,
            "all processes complete their protected operations"
        );
        jh::test::tiny_test::expect(active.load_strong() == 0, "no process remains in the critical section");
        jh::test::tiny_test::expect(overlap.load_strong() == 0, "critical sections never overlap across processes");

        auto nonzero_exit = nonzero_exit_worker::start();
        const auto normal_result = nonzero_exit.wait();
        jh::test::tiny_test::expect(
            normal_result.has_value() && normal_result.value() == 23,
            "a nonzero normal exit code is returned as a value"
        );

        auto abnormal_exit = abnormal_exit_worker::start();
        const auto abnormal_result = abnormal_exit.wait();
#if IS_POSIX
        jh::test::tiny_test::expect(
            abnormal_result.has_error() &&
                jh::sync::ipc::process_exit_is_signal(abnormal_result.error()) &&
                jh::sync::ipc::process_exit_signal_number(abnormal_result.error()) == SIGSEGV,
            "the POSIX terminating signal is returned as unexpected"
        );
#elif IS_WINDOWS
        jh::test::tiny_test::expect(
            abnormal_result.has_error() &&
                abnormal_result.error() == jh::sync::ipc::process_exit_error::abnormal_termination,
            "a recognized Windows exception termination is returned as unexpected"
        );
#endif

        auto after_abnormal_exit = nonzero_exit_worker::start();
        const auto after_abnormal_result = after_abnormal_exit.wait();
        jh::test::tiny_test::expect(
            after_abnormal_result.has_value() && after_abnormal_result.value() == 23,
            "the launcher remains usable after an abnormal child termination"
        );
    }
}

template<>
struct jh::test::tiny_test::test<"process_mutex coordinates a shared critical section">
    : jh::test::tiny_test::test_definition<
          "process_mutex coordinates a shared critical section",
          &::test::process_mutex_contracts
      > {};

template<>
struct jh::test::tiny_test::session<"process_mutex">
    : jh::test::tiny_test::session_definition<
          "process_mutex",
          jh::test::tiny_test::test<"process_mutex coordinates a shared critical section">
      > {};

namespace test {
    [[maybe_unused]] const tiny_test::session<"process_mutex"> process_mutex_session{};
}
