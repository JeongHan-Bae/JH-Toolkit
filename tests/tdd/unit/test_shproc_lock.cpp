#define TINY_TEST_MAIN
#include "jh/test/tiny_test/tiny_test.hpp"

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <mutex>
#include <latch>
#include <random>
#include "jh/synchronous/ipc/shared_process_mutex.h"
#include "jh/concepts"

using namespace jh::sync::ipc;
using namespace std::chrono_literals;

using test_mutex_t = shared_process_mutex<"test_shared_mutex">;
using high_priv_mutex_t = shared_process_mutex<"test_shared_mutex", true>;
static std::atomic<int> active_readers{0};
static std::atomic<int> active_writers{0};


// --------------------------
// Try-Lock Behavior
// --------------------------

namespace test {
void tiny_test_case_1() {
    auto& mtx = test_mutex_t::instance();

    mtx.lock();
    std::atomic<bool> result{true};
    std::thread t([&] {
        bool ok = mtx.try_lock_for(100ms);
        result.store(ok);
        std::cout << "[try_lock_for] result = " << std::boolalpha << ok << " (expected false)\n";
    });
    t.join();
    mtx.unlock();

    jh::test::tiny_test::expect_not(static_cast<bool>((result.load())), "result.load()");

}
}
template<>
struct jh::test::tiny_test::test<"shared_process_mutex try_lock_for behavior">
    : jh::test::tiny_test::test_definition<"shared_process_mutex try_lock_for behavior", &::test::tiny_test_case_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_shproc_lock case 1">
    : jh::test::tiny_test::session_definition<
          "test module test_shproc_lock case 1", jh::test::tiny_test::test<"shared_process_mutex try_lock_for behavior">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_shproc_lock case 1"> registration_1{};
}


// --------------------------
// Reentrancy
// --------------------------

namespace test {
void tiny_test_case_2() {
    auto& mtx = test_mutex_t::instance();

    mtx.lock_shared();
    mtx.lock_shared();
    mtx.unlock_shared();
    mtx.unlock_shared();

    mtx.lock();
    mtx.lock();
    mtx.unlock();
    mtx.unlock();


    jh::test::tiny_test::expect(static_cast<bool>((active_readers.load() == 0)), "active_readers.load() == 0");
    jh::test::tiny_test::expect(static_cast<bool>((active_writers.load() == 0)), "active_writers.load() == 0");
    std::cout << "[Reentrancy] OK\n";

}
}
template<>
struct jh::test::tiny_test::test<"shared_process_mutex reentrancy">
    : jh::test::tiny_test::test_definition<"shared_process_mutex reentrancy", &::test::tiny_test_case_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_shproc_lock case 2">
    : jh::test::tiny_test::session_definition<
          "test module test_shproc_lock case 2", jh::test::tiny_test::test<"shared_process_mutex reentrancy">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_shproc_lock case 2"> registration_2{};
}

namespace test {
void tiny_test_case_3() {
    using entrance_mutex_t = process_mutex<"test_shared_mutex.exc">;
    using priority_mutex_t = process_mutex<"test_shared_mutex.pri">;
    auto &mtx = test_mutex_t::instance();
    auto &entrance = entrance_mutex_t::instance();
    auto &priority = priority_mutex_t::instance();

    std::unique_lock priority_hold(priority);
    const bool acquired = mtx.try_lock();
    const bool entrance_open = entrance.try_lock();
    if (entrance_open) entrance.unlock();
    const bool timed_acquired = mtx.try_lock_for(1ms);
    const bool entrance_open_after_timeout = entrance.try_lock();
    if (entrance_open_after_timeout) entrance.unlock();
    if (acquired) mtx.unlock();
    if (timed_acquired) mtx.unlock();
    priority_hold.unlock();

    jh::test::tiny_test::expect_not(acquired, "priority gate rejects an exclusive try-lock");
    jh::test::tiny_test::expect(entrance_open, "failed try-lock releases the entrance gate");
    jh::test::tiny_test::expect_not(timed_acquired, "priority gate rejects a timed exclusive lock");
    jh::test::tiny_test::expect(entrance_open_after_timeout,
                                "failed timed lock releases the entrance gate");
}
}
template<>
struct jh::test::tiny_test::test<"shared_process_mutex releases entrance after failed priority lock">
    : jh::test::tiny_test::test_definition<
          "shared_process_mutex releases entrance after failed priority lock", &::test::tiny_test_case_3> {};
template<>
struct jh::test::tiny_test::session<"test module test_shproc_lock case 3">
    : jh::test::tiny_test::session_definition<
          "test module test_shproc_lock case 3",
          jh::test::tiny_test::test<"shared_process_mutex releases entrance after failed priority lock">> {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_shproc_lock case 3"> registration_3{};
}

namespace test {
void tiny_test_case_4() {
    using condition_type = process_cond_var<"test_predicate_wait">;
    auto &condition = condition_type::instance();
    std::atomic<bool> pending{true};
    std::atomic<bool> first_check{true};
    std::atomic<bool> completed{false};
    std::latch predicate_entered(1), resume_predicate(1), notifier_ready(1);

    std::thread waiter([&] {
        const bool ready = condition.wait_while_until([&] {
            if (first_check.exchange(false)) {
                const bool observed = pending.load();
                predicate_entered.count_down();
                resume_predicate.wait();
                return observed;
            }
            return pending.load();
        }, std::chrono::steady_clock::now() + 5s);
        completed.store(ready);
    });

    predicate_entered.wait();
    std::thread notifier([&] {
        pending.store(false);
        notifier_ready.count_down();
        condition.notify_one();
    });
    notifier_ready.wait();
    resume_predicate.count_down();
    waiter.join();
    notifier.join();

    jh::test::tiny_test::expect(completed.load(),
                                "predicate wait observes a notification after its first check");
}
}
template<>
struct jh::test::tiny_test::test<"process_cond_var predicate wait preserves notification">
    : jh::test::tiny_test::test_definition<
          "process_cond_var predicate wait preserves notification", &::test::tiny_test_case_4> {};
template<>
struct jh::test::tiny_test::session<"test module test_shproc_lock case 4">
    : jh::test::tiny_test::session_definition<
          "test module test_shproc_lock case 4",
          jh::test::tiny_test::test<"process_cond_var predicate wait preserves notification">> {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_shproc_lock case 4"> registration_4{};
}

