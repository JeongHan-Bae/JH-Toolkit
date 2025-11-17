#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
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
static std::atomic<bool> upgrade_in_progress{false};

// --------------------------
// Reader Task
// --------------------------
void reader_task(int id, int start_delay_ms = 0) {
    auto& mtx = test_mutex_t::instance();
    std::mt19937 rng(id * 12345);
    std::uniform_int_distribution<int> dist(50, 150);
    std::this_thread::sleep_for(std::chrono::milliseconds(start_delay_ms));

    for (int i = 0; i < 3; ++i) {
        mtx.lock_shared();
        int r = ++active_readers;
        CHECK(active_writers.load() == 0); // sanity check
        std::cout << "[R" << id << "] acquired shared lock (total readers=" << r << ")\n";

        std::this_thread::sleep_for(std::chrono::milliseconds(dist(rng)));

        r = --active_readers;
        mtx.unlock_shared();
        std::cout << "[R" << id << "] released shared lock (remaining readers=" << r << ")\n";

        std::this_thread::sleep_for(std::chrono::milliseconds(dist(rng)));
    }
}

// --------------------------
// Writer Task
// --------------------------
void writer_task(int id, int start_delay_ms = 0) {
    auto& mtx = test_mutex_t::instance();
    std::mt19937 rng(id * 88888);
    std::uniform_int_distribution<int> dist(100, 180);
    std::this_thread::sleep_for(std::chrono::milliseconds(start_delay_ms));

    for (int i = 0; i < 2; ++i) {
        mtx.lock();
        ++active_writers;
        CHECK(active_writers.load() == 1);
        CHECK(active_readers.load() == 0);
        std::cout << ">>> [W" << id << "] acquired exclusive lock\n";

        std::this_thread::sleep_for(std::chrono::milliseconds(dist(rng)));

        --active_writers;
        mtx.unlock();
        std::cout << "<<< [W" << id << "] released exclusive lock\n";

        std::this_thread::sleep_for(std::chrono::milliseconds(dist(rng)));
    }
}

// --------------------------
// Upgrader Task
// --------------------------
void upgrader_task(int start_delay_ms = 0) {
    if (upgrade_in_progress.exchange(true))
        return;

    auto& mtx = high_priv_mutex_t::instance();
    std::this_thread::sleep_for(std::chrono::milliseconds(start_delay_ms));

    std::cout << "[U] trying shared lock...\n";
    mtx.lock_shared();
    std::cout << "[U] entered shared mode\n";

    std::this_thread::sleep_for(80ms);
    std::cout << "[U] upgrading to exclusive...\n";
    mtx.upgrade_lock();
    std::cout << "[U] exclusive mode\n";

    std::this_thread::sleep_for(120ms);
    mtx.unlock();
    std::cout << "[U] done (upgrade-only)\n";

    upgrade_in_progress.store(false);
}

// --------------------------
// Concurrency Scenario
// --------------------------
TEST_CASE("shared_process_mutex concurrency stress", "[shared_process_mutex][concurrency]") {

    STATIC_REQUIRE(jh::concepts::reentrant_mutex<high_priv_mutex_t>);
    STATIC_REQUIRE(jh::concepts::reentrant_mutex<test_mutex_t>);

    STATIC_REQUIRE(jh::concepts::reentrance_capable_mutex<high_priv_mutex_t>);
    STATIC_REQUIRE(jh::concepts::reentrance_capable_mutex<test_mutex_t>);

    high_priv_mutex_t::unlink();
    active_readers = 0;
    active_writers = 0;
    upgrade_in_progress = false;

    std::vector<std::thread> threads;

    threads.reserve(4);
    for (int i = 0; i < 4; ++i)
        threads.emplace_back(reader_task, i, 50 * i);

    threads.emplace_back(upgrader_task, 120);
    threads.emplace_back(writer_task, 1, 600);

    for (auto& t : threads)
        t.join();

    // lightweight sanity
    CHECK(active_readers.load() == 0);
    CHECK(active_writers.load() == 0);
    CHECK_FALSE(upgrade_in_progress.load());
}

// --------------------------
// Try-Lock Behavior
// --------------------------
TEST_CASE("shared_process_mutex try_lock_for behavior", "[shared_process_mutex][try_lock]") {
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

    CHECK_FALSE(result.load());
}

// --------------------------
// Reentrancy
// --------------------------
TEST_CASE("shared_process_mutex reentrancy", "[shared_process_mutex][reentrancy]") {
    auto& mtx = test_mutex_t::instance();

    REQUIRE_NOTHROW([&] {
        mtx.lock_shared();
        mtx.lock_shared();
        mtx.unlock_shared();
        mtx.unlock_shared();

        mtx.lock();
        mtx.lock();
        mtx.unlock();
        mtx.unlock();
    }());


    CHECK(active_readers.load() == 0);
    CHECK(active_writers.load() == 0);
    std::cout << "[Reentrancy] OK\n";
}
