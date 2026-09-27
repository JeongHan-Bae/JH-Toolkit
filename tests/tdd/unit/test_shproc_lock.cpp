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

    [&]() { bool did_not_throw = true; try { [&] {
        mtx.lock_shared();
        mtx.lock_shared();
        mtx.unlock_shared();
        mtx.unlock_shared();

        mtx.lock();
        mtx.lock();
        mtx.unlock();
        mtx.unlock();
    }(); } catch (...) { did_not_throw = false; } jh::test::tiny_test::expect(did_not_throw, "does not throw: [&] {\n        mtx.lock_shared();\n        mtx.lock_shared();\n        mtx.unlock_shared();\n        mtx.unlock_shared();\n\n        mtx.lock();\n        mtx.lock();\n        mtx.unlock();\n        mtx.unlock();\n    }()"); }();


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

