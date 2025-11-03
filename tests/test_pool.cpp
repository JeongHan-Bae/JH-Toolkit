#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include "jh/sim_pool.h"
#include "jh/pool.h"
#include "jh/macros/platform.h"
#include <memory>
#include <thread>
#include <iostream>

#if IS_WINDOWS
#include <cstdlib>

// Disable Windows UCRT debug heap in MSYS2 / MinGW-UCRT environment.
// Prevents spurious heap corruption (0xC0000374) during multi-threaded tests.
static struct DisableWinDebugHeap {
    DisableWinDebugHeap() noexcept {
        // Equivalent to: set _NO_DEBUG_HEAP=1
        // Ensures the loader skips debug-heap instrumentation in UCRT.
        ::putenv(const_cast<char*>("_NO_DEBUG_HEAP=1"));
    }
} _disable_debug_heap_guard;
#endif


namespace test {
    // 🎯 Test Object
    struct TestObject {
        int value;

        explicit TestObject(const int v) : value(v) {
        }

        bool operator==(const TestObject &other) const {
            return value == other.value;
        }
    };

    struct AutoPoolingObject {
        int id;

        explicit AutoPoolingObject(const int v) : id(v) {
        }

        [[nodiscard]] std::uint64_t hash() const noexcept {
            return std::hash<int>{}(id);
        }

        bool operator==(const AutoPoolingObject &other) const noexcept {
            return id == other.id;
        }
    };

    // 🎯 Custom Hash Function
    struct TestObjectHash {
        std::size_t operator()(const std::weak_ptr<TestObject> &ptr) const noexcept {
            if (const auto sp = ptr.lock()) {
                return std::hash<int>{}(sp->value);
            }
            return 0; // Expired weak_ptrs share the same hash
        }
    };

    // 🎯 Custom Equality Function (Expired weak_ptrs are considered different)
    struct TestObjectEq {
        bool operator()(const std::weak_ptr<TestObject> &lhs, const std::weak_ptr<TestObject> &rhs) const noexcept {
            const auto sp1 = lhs.lock();
            const auto sp2 = rhs.lock();
            if (!sp1 || !sp2) return false;
            return sp1->value == sp2->value;
        }
    };

    using CustomizedPool = jh::sim_pool<TestObject, TestObjectHash, TestObjectEq>;
    using DeducedPool = jh::pool<AutoPoolingObject>;
} // namespace test

// ✅ Basic Functionality Test
TEST_CASE("sim_pool basic functionality") {
    test::CustomizedPool pool;

    auto obj1 = pool.acquire(10);
    auto obj2 = pool.acquire(10);
    auto obj3 = pool.acquire(20);

    REQUIRE(obj1 == obj2); // 🎯 Objects with the same value should be reused
    REQUIRE(obj1 != obj3); // 🎯 Different values should not be reused
    REQUIRE(pool.size() == 2); // 🎯 The pool should contain only two unique objects
}

TEST_CASE("pool basic functionality") {
    test::DeducedPool pool;

    auto obj1 = pool.acquire(10);
    auto obj2 = pool.acquire(10);
    auto obj3 = pool.acquire(20);

    REQUIRE(obj1 == obj2); // 🎯 Objects with the same value should be reused
    REQUIRE(obj1 != obj3); // 🎯 Different values should not be reused
    REQUIRE(pool.size() == 2); // 🎯 The pool should contain only two unique objects
}

// ✅ Cleanup Test (Effect of Eq)
TEST_CASE("sim_pool cleanup") {
    test::CustomizedPool pool;

    auto obj1 = pool.acquire(10);
    auto obj2 = pool.acquire(20);

    REQUIRE(pool.size() == 2);

    obj1.reset(); // 🎯 Release shared_ptrs
    obj2.reset();

    REQUIRE(pool.size() == 2); // 🎯 Expired weak_ptrs are still in the pool (not automatically cleaned up)

    pool.cleanup(); // 🎯 Manually trigger cleanup

    REQUIRE(pool.size() == 0); // 🎯 The pool should now be empty
}

// ✅ Cleanup Test (Effect of Eq)
TEST_CASE("pool cleanup") {
    test::DeducedPool pool;

    auto obj1 = pool.acquire(10);
    auto obj2 = pool.acquire(20);

    REQUIRE(pool.size() == 2);

    obj1.reset(); // 🎯 Release shared_ptrs
    obj2.reset();

    REQUIRE(pool.size() == 2); // 🎯 Expired weak_ptrs are still in the pool (not automatically cleaned up)

    pool.cleanup(); // 🎯 Manually trigger cleanup

    REQUIRE(pool.size() == 0); // 🎯 The pool should now be empty
}

// ✅ Dynamic Expansion & Contraction Test
TEST_CASE("sim_pool dynamic expansion and contraction") {
    test::CustomizedPool pool(4); // 🎯 Initial reserved_size = 4

    std::vector<std::shared_ptr<test::TestObject> > objects;
    objects.reserve(10);
    for (int i = 0; i < 10; ++i) {
        objects.push_back(pool.acquire(i)); // 🎯 Store shared_ptrs
    }

    REQUIRE(pool.size() == 10); // 🎯 10 unique objects
    REQUIRE(pool.reserved_size() >= 16); // 🎯 Expansion triggered (reserved_size *= 2)

    for (auto &obj: objects) {
        obj.reset(); // 🎯 Release all shared_ptrs
    }

    pool.cleanup(); // 🎯 Trigger shrinkage
    REQUIRE(pool.reserved_size() <= 16); // 🎯 Shrinkage triggered (reserved_size /= 2)
}

// ✅ Dynamic Expansion & Contraction Test
TEST_CASE("pool dynamic expansion and contraction") {
    test::DeducedPool pool(4); // 🎯 Initial reserved_size = 4

    std::vector<std::shared_ptr<test::AutoPoolingObject> > objects;
    objects.reserve(10);
    for (int i = 0; i < 10; ++i) {
        objects.push_back(pool.acquire(i)); // 🎯 Store shared_ptrs
    }

    REQUIRE(pool.size() == 10); // 🎯 10 unique objects
    REQUIRE(pool.reserved_size() >= 16); // 🎯 Expansion triggered (reserved_size *= 2)

    for (auto &obj: objects) {
        obj.reset(); // 🎯 Release all shared_ptrs
    }

    pool.cleanup(); // 🎯 Trigger shrinkage
    REQUIRE(pool.reserved_size() <= 16); // 🎯 Shrinkage triggered (reserved_size /= 2)
}

// ✅ Move Semantics Test
TEST_CASE("sim_pool move semantics") {
    test::CustomizedPool pool1;
    auto obj1 = pool1.acquire(10);
    auto obj2 = pool1.acquire(20);

    REQUIRE(pool1.size() == 2);

    test::CustomizedPool pool2 = std::move(pool1); // 🎯 Move constructor

    REQUIRE(pool2.size() == 2);
    REQUIRE(pool1.size() == 0); // 🎯 pool1 should now be empty

    test::CustomizedPool pool3;
    pool3 = std::move(pool2); // 🎯 Move assignment

    REQUIRE(pool3.size() == 2);
    REQUIRE(pool2.size() == 0); // 🎯 pool2 should now be empty

    pool3.clear();
    REQUIRE(pool3.size() == 0); // 🎯 pool3 should now be empty
    REQUIRE(pool3.reserved_size() == test::CustomizedPool::MIN_RESERVED_SIZE); // 🎯 reserved_size should be reset
}

TEST_CASE("pool move semantics") {
    test::DeducedPool pool1;
    auto obj1 = pool1.acquire(10);
    auto obj2 = pool1.acquire(20);

    REQUIRE(pool1.size() == 2);

    test::DeducedPool pool2 = std::move(pool1); // 🎯 Move constructor

    REQUIRE(pool2.size() == 2);
    REQUIRE(pool1.size() == 0); // 🎯 pool1 should now be empty

    test::DeducedPool pool3;
    pool3 = std::move(pool2); // 🎯 Move assignment

    REQUIRE(pool3.size() == 2);
    REQUIRE(pool2.size() == 0); // 🎯 pool2 should now be empty

    pool3.clear();
    REQUIRE(pool3.size() == 0); // 🎯 pool3 should now be empty
    REQUIRE(pool3.reserved_size() == test::DeducedPool::MIN_RESERVED_SIZE); // 🎯 reserved_size should be reset
}

// ✅ Multithreading Test (128 Iterations for Data Race Detection): Not storing shared_ptr
TEST_CASE("sim_pool multithreading without storing shared_ptr") {
    test::CustomizedPool pool;
    constexpr int total_tests = 128;

    for (int idx = 0; idx < total_tests; ++idx) {
        SECTION("Sim Pool Stress Test Run " + std::to_string(idx + 1)) {
            constexpr int OBJECTS_PER_THREAD = 200;
            constexpr int THREADS = 8;

            std::vector<std::thread> workers;
            workers.reserve(THREADS);
            for (int t = 0; t < THREADS; ++t) {
                workers.emplace_back([&pool] {
                    for (int i = 0; i < OBJECTS_PER_THREAD; ++i) {
                        pool.acquire(i);
                        // 🎯 Not storing shared_ptr, not calling REQUIRE_NOTHROW() otherwise might cause dangling pointers
                    }
                });
            }

            for (auto &w: workers) {
                w.join();
            }

            // 🎯 Since shared_ptrs are not stored, weak_ptrs may become expired,
            // but size() does not necessarily become 0 until expand_and_cleanup is triggered
            REQUIRE(pool.size() <= OBJECTS_PER_THREAD * THREADS);
            pool.cleanup_shrink(); // 🎯 Explicit cleanup
            REQUIRE(pool.reserved_size() == test::CustomizedPool::MIN_RESERVED_SIZE);
            // 🎯 reserved_size should remain unchanged
            REQUIRE(pool.size() == 0); // 🎯 After cleanup, the pool should be empty
        }
    }
}

// ✅ Multithreading Test (128 Iterations for Data Race Detection): Not storing shared_ptr
TEST_CASE("pool multithreading without storing shared_ptr") {
    test::DeducedPool pool;
    constexpr int total_tests = 128;

    for (int idx = 0; idx < total_tests; ++idx) {
        SECTION("Sim Pool Stress Test Run " + std::to_string(idx + 1)) {
            constexpr int OBJECTS_PER_THREAD = 200;
            constexpr int THREADS = 8;

            std::vector<std::thread> workers;
            workers.reserve(THREADS);
            for (int t = 0; t < THREADS; ++t) {
                workers.emplace_back([&pool] {
                    for (int i = 0; i < OBJECTS_PER_THREAD; ++i) {
                        pool.acquire(i);
                        // 🎯 Not storing shared_ptr, not calling REQUIRE_NOTHROW() otherwise might cause dangling pointers
                    }
                });
            }

            for (auto &w: workers) {
                w.join();
            }

            // 🎯 Since shared_ptrs are not stored, weak_ptrs may become expired,
            // but size() does not necessarily become 0 until expand_and_cleanup is triggered
            REQUIRE(pool.size() <= OBJECTS_PER_THREAD * THREADS);
            pool.cleanup_shrink(); // 🎯 Explicit cleanup
            REQUIRE(pool.reserved_size() == test::DeducedPool::MIN_RESERVED_SIZE);
            // 🎯 reserved_size should remain unchanged
            REQUIRE(pool.size() == 0); // 🎯 After cleanup, the pool should be empty
        }
    }
}

// ✅ Multithreading Test (128 Iterations for Data Race Detection): Storing shared_ptr
TEST_CASE("sim_pool multithreading with storing shared_ptr") {
    test::CustomizedPool pool;
    constexpr int total_tests = 128;

    for (int idx = 0; idx < total_tests; ++idx) {
        SECTION("Sim Pool Stress Test Run " + std::to_string(idx + 1)) {
            constexpr int OBJECTS_PER_THREAD = 200;
            constexpr int THREADS = 8;

            std::vector<std::shared_ptr<test::TestObject> > stored_objects;
            std::mutex stored_mutex;
            std::vector<std::thread> workers;

            workers.reserve(THREADS);
            for (int t = 0; t < THREADS; ++t) {
                workers.emplace_back([&pool, &stored_objects, &stored_mutex, t] {
                    for (int i = t * OBJECTS_PER_THREAD; i < (t + 1) * OBJECTS_PER_THREAD; ++i) {
                        // 🎯 Avoid duplicate values
                        {
                            auto obj = pool.acquire(i);
                            // 🎯 Protect stored_objects with std::lock_guard
                            std::lock_guard lock(stored_mutex);
                            stored_objects.push_back(obj);
                        }
                    }
                });
            }

            for (auto &w: workers) {
                w.join();
            }

            REQUIRE(pool.size() == OBJECTS_PER_THREAD * THREADS); // 🎯 Ensure all objects are alive
            REQUIRE(pool.reserved_size() >= OBJECTS_PER_THREAD * THREADS / 2); // 🎯 Ensure reserved_size has expanded

            stored_objects.clear(); // 🎯 Release all shared_ptrs
            pool.cleanup(); // 🎯 Trigger cleanup
            REQUIRE(pool.reserved_size() >= OBJECTS_PER_THREAD * THREADS);
            REQUIRE(pool.size() == 0); // 🎯 After cleanup, the pool should be empty
        }
    }
}

// ✅ Multithreading Test (128 Iterations for Data Race Detection): Storing shared_ptr
TEST_CASE("pool multithreading with storing shared_ptr") {
    test::DeducedPool pool;
    constexpr int total_tests = 128;

    for (int idx = 0; idx < total_tests; ++idx) {
        SECTION("Sim Pool Stress Test Run " + std::to_string(idx + 1)) {
            constexpr int OBJECTS_PER_THREAD = 200;
            constexpr int THREADS = 8;

            std::vector<std::shared_ptr<test::AutoPoolingObject> > stored_objects;
            std::mutex stored_mutex;
            std::vector<std::thread> workers;

            workers.reserve(THREADS);
            for (int t = 0; t < THREADS; ++t) {
                workers.emplace_back([&pool, &stored_objects, &stored_mutex, t]() {
                    for (int i = t * OBJECTS_PER_THREAD; i < (t + 1) * OBJECTS_PER_THREAD; ++i) {
                        // 🎯 Avoid duplicate values
                        {
                            auto obj = pool.acquire(i);
                            // 🎯 Protect stored_objects with std::lock_guard
                            std::lock_guard lock(stored_mutex);
                            stored_objects.push_back(obj);
                        }
                    }
                });
            }

            for (auto &w: workers) {
                w.join();
            }

            REQUIRE(pool.size() == OBJECTS_PER_THREAD * THREADS); // 🎯 Ensure all objects are alive
            REQUIRE(pool.reserved_size() >= OBJECTS_PER_THREAD * THREADS / 2); // 🎯 Ensure reserved_size has expanded

            stored_objects.clear(); // 🎯 Release all shared_ptrs
            pool.cleanup(); // 🎯 Trigger cleanup
            REQUIRE(pool.reserved_size() >= OBJECTS_PER_THREAD * THREADS);
            REQUIRE(pool.size() == 0); // 🎯 After cleanup, the pool should be empty
        }
    }
}

/// ⚠️ Note:
/// std::string itself is *not* an immutable type — its internal buffer may change.
/// This test only demonstrates that it *can* be pooled because it satisfies
/// std::hash<std::string> and operator==.
/// For stable, non-static, content-based pooling, use jh::pool&lt;jh::immutable_str&gt; instead.
TEST_CASE("pool with std::string") {
    jh::pool<std::string> pool;

    auto hello1 = pool.acquire("hello");
    auto hello2 = pool.acquire("hello");
    auto world  = pool.acquire("world");

    REQUIRE(hello1 == hello2);   // 🎯 identical strings should be reused
    REQUIRE(hello1 != world);    // 🎯 distinct strings should not be reused
    REQUIRE(pool.size() == 2);   // 🎯 only two unique entries in the pool

    hello1.reset();
    hello2.reset();
    world.reset();

    REQUIRE(pool.size() == 2);   // 🎯 expired entries remain until cleanup
    pool.cleanup();
    REQUIRE(pool.size() == 0);   // 🎯 after cleanup, pool becomes empty
}
