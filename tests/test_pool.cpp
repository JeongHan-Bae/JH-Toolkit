#include <catch2/catch_all.hpp>

#include "jh/pool"
#include "jh/macros/platform.h"
#include <memory>
#include <memory_resource>
#include <thread>

/**
 * @brief Tests for pooling facilities:
 *        <code>jh::observe_pool</code>,
 *        <code>jh::conc::pointer_pool</code>,
 *        <code>jh::resource_pool</code>,
 *        and <code>jh::resource_pool_set</code>.
 *
 * @details
 * This suite validates acquisition, reuse, cleanup, resizing,
 * move semantics, and multi-threaded correctness.
 *
 * <hr>
 * <b>Concurrency Guarantees</b>
 *
 * All pool implementations in this module are algorithmically
 * data-race-free (DRF) and do not rely on undefined behavior under
 * the ISO C++ memory model.
 *
 * Additional strengthening is applied on Windows builds via
 * <code>std::atomic_thread_fence(std::memory_order_seq_cst)</code>
 * around lock boundaries. This eliminates ISO-level UB risks and
 * preserves correctness within the C++ abstract machine.
 *
 * <hr>
 * <b>Windows / MinGW Runtime Characteristics</b>
 *
 * On certain Windows configurations (e.g., MinGW-w64 with libstdc++
 * and UCRT/MSVCRT), system-level synchronization primitives may not
 * provide POSIX-equivalent global ordering behavior.
 *
 * Under extreme multi-core contention, rare visibility or reordering
 * effects may be observed. These effects arise from platform runtime
 * and kernel-level synchronization characteristics rather than from
 * violations of the C++ standard.
 *
 * Such behavior does not indicate undefined behavior or data races
 * within the pool implementations.
 *
 * <hr>
 * <b>posix_smtx_* Strengthening</b>
 *
 * The following wrappers are used to approximate POSIX-style ordering:
 *
 * <ul>
 *   <li><code>jh::sync::posix_smtx_unique_lock</code></li>
 *   <li><code>jh::sync::posix_smtx_shared_lock</code></li>
 * </ul>
 *
 * On Windows, these introduce sequentially-consistent fences at
 * lock boundaries to strengthen ordering at the language level.
 *
 * While this improves practical stability, it cannot fully enforce
 * hardware-level global ordering beyond what the platform provides.
 *
 * <hr>
 * <b>Test Policy</b>
 *
 * Due to platform-level ordering variance and CI constraints,
 * extreme high-concurrency stress tests are disabled on Windows.
 *
 * Windows builds are validated for:
 *
 * <ul>
 *   <li>single-threaded usage</li>
 *   <li>moderate multi-threaded workloads</li>
 * </ul>
 *
 * Full high-pressure concurrency validation is performed on POSIX
 * platforms (Linux / Darwin), which remain the primary reference
 * environments.
 *
 * <hr>
 * <b>Design Note</b>
 *
 * The pool modules are engineered to be DRF and standards-compliant.
 * Observed high-contention behavior differences stem from platform
 * synchronization semantics rather than from algorithmic defects.
 */

namespace test {
    // Test Object
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

    // Custom Hash Function
    struct TestObjectHash {
        std::size_t operator()(const std::weak_ptr<TestObject> &ptr) const noexcept {
            if (const auto sp = ptr.lock()) {
                return std::hash<int>{}(sp->value);
            }
            return 0; // Expired weak_ptrs share the same hash
        }
    };

    // Custom Equality Function (Expired weak_ptrs are considered different)
    struct TestObjectEq {
        bool operator()(const std::weak_ptr<TestObject> &lhs, const std::weak_ptr<TestObject> &rhs) const noexcept {
            const auto sp1 = lhs.lock();
            const auto sp2 = rhs.lock();
            if (!sp1 || !sp2) return false;
            return sp1->value == sp2->value;
        }
    };

    using CustomizedPool = jh::conc::pointer_pool<TestObject, TestObjectHash, TestObjectEq>;
    using DeducedPool = jh::observe_pool<AutoPoolingObject>;
} // namespace test

// Basic Functionality Test
TEST_CASE("pointer_pool basic functionality") {
    test::CustomizedPool pool;

    auto obj1 = pool.acquire(10);
    auto obj2 = pool.acquire(10);
    auto obj3 = pool.acquire(20);

    REQUIRE(obj1 == obj2); // Objects with the same value should be reused
    REQUIRE(obj1 != obj3); // Different values should not be reused
    REQUIRE(pool.size() == 2); // The pool should contain only two unique objects
}

TEST_CASE("observe_pool basic functionality") {
    test::DeducedPool pool;

    auto obj1 = pool.acquire(10);
    auto obj2 = pool.acquire(10);
    auto obj3 = pool.acquire(20);

    REQUIRE(obj1 == obj2); // Objects with the same value should be reused
    REQUIRE(obj1 != obj3); // Different values should not be reused
    REQUIRE(pool.size() == 2); // The pool should contain only two unique objects
}

// Cleanup Test (Effect of Eq)
TEST_CASE("pointer_pool cleanup") {
    test::CustomizedPool pool;

    auto obj1 = pool.acquire(10);
    auto obj2 = pool.acquire(20);

    REQUIRE(pool.size() == 2);

    obj1.reset(); // Release shared_ptrs
    obj2.reset();

    REQUIRE(pool.size() == 2); // Expired weak_ptrs are still in the pool (not automatically cleaned up)

    pool.cleanup(); // Manually trigger cleanup

    REQUIRE(pool.size() == 0); // The pool should now be empty
}

// Cleanup Test (Effect of Eq)
TEST_CASE("observe_pool cleanup") {
    test::DeducedPool pool;

    auto obj1 = pool.acquire(10);
    auto obj2 = pool.acquire(20);

    REQUIRE(pool.size() == 2);

    obj1.reset(); // Release shared_ptrs
    obj2.reset();

    REQUIRE(pool.size() == 2); // Expired weak_ptrs are still in the pool (not automatically cleaned up)

    pool.cleanup(); // Manually trigger cleanup

    REQUIRE(pool.size() == 0); // The pool should now be empty
}

// Dynamic Expansion & Contraction Test
TEST_CASE("pointer_pool dynamic expansion and contraction") {
    test::CustomizedPool pool(4); // Initial reserved_size = 4

    std::vector<std::shared_ptr<test::TestObject> > objects;
    objects.reserve(10);
    for (int i = 0; i < 10; ++i) {
        objects.push_back(pool.acquire(i)); // Store shared_ptrs
    }

    REQUIRE(pool.size() == 10); // 10 unique objects
    REQUIRE(pool.capacity() >= 16); // Expansion triggered (reserved_size *= 2)

    for (auto &obj: objects) {
        obj.reset(); // Release all shared_ptrs
    }

    pool.cleanup(); // Trigger shrinkage
    REQUIRE(pool.capacity() <= 16); // Shrinkage triggered (reserved_size /= 2)
}

// Dynamic Expansion & Contraction Test
TEST_CASE("observe_pool dynamic expansion and contraction") {
    test::DeducedPool pool(4); // Initial reserved_size = 4

    std::vector<std::shared_ptr<test::AutoPoolingObject> > objects;
    objects.reserve(10);
    for (int i = 0; i < 10; ++i) {
        objects.push_back(pool.acquire(i)); // Store shared_ptrs
    }

    REQUIRE(pool.size() == 10); // 10 unique objects
    REQUIRE(pool.capacity() >= 16); // Expansion triggered (reserved_size *= 2)

    for (auto &obj: objects) {
        obj.reset(); // Release all shared_ptrs
    }

    pool.cleanup(); // Trigger shrinkage
    REQUIRE(pool.capacity() <= 16); // Shrinkage triggered (reserved_size /= 2)
}

// Move Semantics Test
TEST_CASE("pointer_pool move semantics") {
    test::CustomizedPool pool1;
    auto obj1 = pool1.acquire(10);
    auto obj2 = pool1.acquire(20);

    REQUIRE(pool1.size() == 2);

    test::CustomizedPool pool2 = std::move(pool1); // Move constructor

    REQUIRE(pool2.size() == 2);
    REQUIRE(pool1.size() == 0); // pool1 should now be empty

    test::CustomizedPool pool3;
    pool3 = std::move(pool2); // Move assignment

    REQUIRE(pool3.size() == 2);
    REQUIRE(pool2.size() == 0); // pool2 should now be empty

    pool3.clear();
    REQUIRE(pool3.size() == 0); // pool3 should now be empty
    REQUIRE(pool3.capacity() == test::CustomizedPool::MIN_RESERVED_SIZE); // reserved_size should be reset
}

TEST_CASE("observe_pool move semantics") {
    test::DeducedPool pool1;
    auto obj1 = pool1.acquire(10);
    auto obj2 = pool1.acquire(20);

    REQUIRE(pool1.size() == 2);

    test::DeducedPool pool2 = std::move(pool1); // Move constructor

    REQUIRE(pool2.size() == 2);
    REQUIRE(pool1.size() == 0); // pool1 should now be empty

    test::DeducedPool pool3;
    pool3 = std::move(pool2); // Move assignment

    REQUIRE(pool3.size() == 2);
    REQUIRE(pool2.size() == 0); // pool2 should now be empty

    pool3.clear();
    REQUIRE(pool3.size() == 0); // pool3 should now be empty
    REQUIRE(pool3.capacity() == test::DeducedPool::MIN_RESERVED_SIZE); // reserved_size should be reset
}

#if !IS_WINDOWS

// Multithreading Test (128 Iterations for Data Race Detection): Not storing shared_ptr
TEST_CASE("pointer_pool multithreading without storing shared_ptr") {
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
                        // Not storing shared_ptr, not calling REQUIRE_NOTHROW() otherwise might cause dangling pointers
                    }
                });
            }

            for (auto &w: workers) {
                w.join();
            }

            // Since shared_ptrs are not stored, weak_ptrs may become expired,
            // but size() does not necessarily become 0 until expand_and_cleanup is triggered
            REQUIRE(pool.size() <= OBJECTS_PER_THREAD * THREADS);
            pool.cleanup_shrink(); // Explicit cleanup
            REQUIRE(pool.capacity() == test::CustomizedPool::MIN_RESERVED_SIZE);
            // reserved_size should remain unchanged
            REQUIRE(pool.size() == 0); // After cleanup, the pool should be empty
        }
    }
}

// Multithreading Test (128 Iterations for Data Race Detection): Not storing shared_ptr
TEST_CASE("observe_pool multithreading without storing shared_ptr") {
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
                        // Not storing shared_ptr, not calling REQUIRE_NOTHROW() otherwise might cause dangling pointers
                    }
                });
            }

            for (auto &w: workers) {
                w.join();
            }

            // Since shared_ptrs are not stored, weak_ptrs may become expired,
            // but size() does not necessarily become 0 until expand_and_cleanup is triggered
            REQUIRE(pool.size() <= OBJECTS_PER_THREAD * THREADS);
            pool.cleanup_shrink(); // Explicit cleanup
            REQUIRE(pool.capacity() == test::DeducedPool::MIN_RESERVED_SIZE);
            // reserved_size should remain unchanged
            REQUIRE(pool.size() == 0); // After cleanup, the pool should be empty
        }
    }
}

// Multithreading Test (128 Iterations for Data Race Detection): Storing shared_ptr
TEST_CASE("pointer_pool multithreading with storing shared_ptr") {
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
                        // Avoid duplicate values
                        {
                            auto obj = pool.acquire(i);
                            // Protect stored_objects with std::lock_guard
                            std::lock_guard lock(stored_mutex);
                            stored_objects.push_back(obj);
                        }
                    }
                });
            }

            for (auto &w: workers) {
                w.join();
            }

            REQUIRE(pool.size() == OBJECTS_PER_THREAD * THREADS); // Ensure all objects are alive
            REQUIRE(pool.capacity() >= OBJECTS_PER_THREAD * THREADS); // Ensure reserved_size has expanded
            stored_objects.clear(); // Release all shared_ptrs
            pool.cleanup(); // Trigger cleanup
            REQUIRE(pool.capacity() >= OBJECTS_PER_THREAD * THREADS);
            REQUIRE(pool.size() == 0); // After cleanup, the pool should be empty
        }
    }
}

// Multithreading Test (128 Iterations for Data Race Detection): Storing shared_ptr
TEST_CASE("observe_pool multithreading with storing shared_ptr") {
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
                        // Avoid duplicate values
                        {
                            auto obj = pool.acquire(i);
                            // Protect stored_objects with std::lock_guard
                            std::lock_guard lock(stored_mutex);
                            stored_objects.push_back(obj);
                        }
                    }
                });
            }

            for (auto &w: workers) {
                w.join();
            }

            REQUIRE(pool.size() == OBJECTS_PER_THREAD * THREADS); // Ensure all objects are alive
            REQUIRE(pool.capacity() >= OBJECTS_PER_THREAD * THREADS); // Ensure reserved_size has expanded
            stored_objects.clear(); // Release all shared_ptrs
            pool.cleanup(); // Trigger cleanup
            REQUIRE(pool.capacity() >= OBJECTS_PER_THREAD * THREADS);
            REQUIRE(pool.size() == 0); // After cleanup, the pool should be empty
        }
    }
}

#endif

/**
 * @note
 * std::string itself is <b>not</b> an immutable type — its internal buffer may change.
 * This test only demonstrates that it <b>can</b> be pooled because it satisfies
 * <code>std::hash&lt;std::string&gt;</code> and <code>operator==</code<.
 * For stable, non-static, content-based pooling, use <code>jh::observe_pool&lt;jh::immutable_str&gt;<code> instead.
*/
TEST_CASE("observe_pool with std::string") {
    jh::observe_pool<std::string> pool;

    auto hello1 = pool.acquire("hello");
    auto hello2 = pool.acquire("hello");
    auto world = pool.acquire("world");

    REQUIRE(hello1 == hello2);   // identical strings should be reused
    REQUIRE(hello1 != world);    // distinct strings should not be reused
    REQUIRE(pool.size() == 2);   // only two unique entries in the pool

    hello1.reset();
    hello2.reset();
    world.reset();

    REQUIRE(pool.size() == 2);   // expired entries remain until cleanup
    pool.cleanup();
    REQUIRE(pool.size() == 0);   // after cleanup, pool becomes empty
}

TEST_CASE("resource_pool_set single-thread basic usage") {
    jh::resource_pool_set<int> pool;

    auto p1 = pool.acquire(42);
    REQUIRE(*p1 == 42);

    {
        auto p2 = p1;
        REQUIRE(*p2 == 42);
        REQUIRE(pool.size() == 1);
    }

    REQUIRE(*p1 == 42);

    p1.reset();

    {
        std::vector<jh::resource_pool_set<int>::ptr> vec;
        vec.reserve(20);
        for (int i = 0; i < 20; ++i)
            vec.emplace_back(pool.acquire(i));
        REQUIRE(pool.size() == 20);
    }

    auto p3 = pool.acquire(99);
    REQUIRE(*p3 == 99);

    {
        const auto [capacity, size] = pool.occupancy_rate();
        REQUIRE(capacity >= size);
    }

    pool.resize_pool();

    {
        const auto [capacity, size] = pool.occupancy_rate();
        REQUIRE(capacity >= size);
    }

    p3.reset();

    auto p4 = pool.find(99);
    REQUIRE(p4 == nullptr);

    REQUIRE(pool.empty());
}

TEST_CASE("resource_pool single-thread key-value") {
    jh::resource_pool<int, std::unique_ptr<std::string>> pool;

    auto p1 = pool.acquire(1, std::tie("hello"));
    REQUIRE(p1->first == 1);
    REQUIRE(*p1->second == "hello");

    auto p2 = pool.acquire(2, std::forward_as_tuple("world"));
    REQUIRE(*p2->second == "world");

    // same key: value constructor must not be evaluated
    auto p3 = pool.acquire(1, std::tie("ignored"));
    REQUIRE(*p3->second == "hello");

    p1.reset();
    p2.reset();
    p3.reset();

    auto p4 = pool.acquire(3, std::forward_as_tuple("new"));
    REQUIRE(*p4->second == "new");

    auto check0 = static_cast<bool>(pool.find(3));
    auto check1 = static_cast<bool>(pool.find(1));

    REQUIRE(check0);
    REQUIRE(check1 == false);
}

#if !IS_WINDOWS

TEST_CASE("resource_pool_set multithreading without storing ptr") {
    jh::resource_pool_set<int> pool;
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
                    }
                });
            }

            for (auto &w: workers) {
                w.join();
            }

            REQUIRE(pool.size() <= OBJECTS_PER_THREAD * THREADS);
            pool.resize_pool();
            REQUIRE(pool.capacity() < OBJECTS_PER_THREAD * THREADS);
        }
    }
}

TEST_CASE("resource_pool_set multithreading with storing ptr") {
    jh::resource_pool_set<int> pool;
    constexpr int total_tests = 128;

    for (int idx = 0; idx < total_tests; ++idx) {
        SECTION("Sim Pool Stress Test Run " + std::to_string(idx + 1)) {
            constexpr int OBJECTS_PER_THREAD = 200;
            constexpr int THREADS = 8;

            std::vector<jh::resource_pool_set<int>::ptr> stored;
            std::mutex mtx;
            std::vector<std::thread> workers;
            workers.reserve(THREADS);

            for (int t = 0; t < THREADS; ++t) {
                workers.emplace_back([&pool, &stored, &mtx, t] {
                    for (int i = t * OBJECTS_PER_THREAD;
                         i < (t + 1) * OBJECTS_PER_THREAD;
                         ++i) {
                        auto p = pool.acquire(i);
                        std::lock_guard lock(mtx);
                        stored.push_back(p);
                    }
                });
            }

            for (auto &w: workers) {
                w.join();
            }

            REQUIRE(pool.size() == OBJECTS_PER_THREAD * THREADS);
            REQUIRE(pool.capacity() >= OBJECTS_PER_THREAD * THREADS);

            stored.clear();
            pool.resize_pool();
            REQUIRE(pool.size() == 0);
        }
    }
}

TEST_CASE("resource_pool<int, string> multithreading without storing ptr") {
    jh::resource_pool<int, std::string> pool;
    constexpr int total_tests = 128;

    for (int idx = 0; idx < total_tests; ++idx) {
        SECTION("Sim Pool Stress Test Run " + std::to_string(idx + 1)) {
            constexpr int OBJECTS_PER_THREAD = 200;
            constexpr int THREADS = 8;

            std::vector<std::thread> workers;
            workers.reserve(THREADS);
            for (int t = 0; t < THREADS; ++t) {
                workers.emplace_back([&pool, t] {
                    for (int i = 0; i < OBJECTS_PER_THREAD; ++i) {
                        const int key = i;
                        pool.acquire(
                                key,
                                std::forward_as_tuple(
                                        std::to_string(key) + "-" + std::to_string(t)
                                )
                        );
                    }
                });
            }

            for (auto &w: workers) {
                w.join();
            }

            REQUIRE(pool.size() <= OBJECTS_PER_THREAD * THREADS);
            pool.resize_pool();
            REQUIRE(pool.capacity() < OBJECTS_PER_THREAD * THREADS);
        }
    }
}

TEST_CASE("resource_pool<int, string> multithreading with storing ptr") {
    jh::resource_pool<int, std::string> pool;
    constexpr int total_tests = 128;

    for (int idx = 0; idx < total_tests; ++idx) {
        SECTION("Sim Pool Stress Test Run " + std::to_string(idx + 1)) {
            constexpr int OBJECTS_PER_THREAD = 200;
            constexpr int THREADS = 8;

            std::vector<jh::resource_pool<int, std::string>::ptr> stored;
            std::mutex mtx;
            std::vector<std::thread> workers;
            workers.reserve(THREADS);

            for (int t = 0; t < THREADS; ++t) {
                workers.emplace_back([&pool, &stored, &mtx, t] {
                    for (int i = t * OBJECTS_PER_THREAD;
                         i < (t + 1) * OBJECTS_PER_THREAD;
                         ++i) {

                        auto p = pool.acquire(
                                i,
                                std::forward_as_tuple(
                                        std::to_string(i) + "-" + std::to_string(t)
                                )
                        );

                        std::lock_guard lock(mtx);
                        stored.push_back(p);
                    }
                });
            }

            for (auto &w: workers) {
                w.join();
            }

            REQUIRE(pool.size() == OBJECTS_PER_THREAD * THREADS);
            REQUIRE(pool.capacity() >= OBJECTS_PER_THREAD * THREADS);

            stored.clear();
            pool.resize_pool();

            REQUIRE(pool.size() == 0);
        }
    }
}

TEST_CASE("pmr resource_pool multithreading with duplicated keys") {
    constexpr int total_tests = 128;

    for (int idx = 0; idx < total_tests; ++idx) {
        SECTION("Sim Pool Stress Test Run " + std::to_string(idx + 1)) {
            constexpr int OBJECTS_PER_THREAD = 200;
            constexpr int THREADS = 8;
            constexpr int UNIQUE_KEYS = OBJECTS_PER_THREAD * THREADS / 2;

            std::pmr::monotonic_buffer_resource rsrc{1024 * 1024};

            using pool_t =
                    jh::resource_pool<
                            int,
                            std::pmr::string,
                            std::pmr::polymorphic_allocator<
                                    jh::conc::detail::value_t<int, std::pmr::string>
                            >
                    >;

            pool_t pool{&rsrc};

            std::vector<pool_t::ptr> stored;
            std::mutex mtx;
            std::vector<std::thread> workers;
            workers.reserve(THREADS);

            for (int t = 0; t < THREADS; ++t) {
                workers.emplace_back([&pool, &stored, &mtx, t] {
                    for (int i = t * OBJECTS_PER_THREAD;
                         i < (t + 1) * OBJECTS_PER_THREAD;
                         ++i) {

                        const int key = i % UNIQUE_KEYS;

                        auto p = pool.acquire(
                                key,
                                std::forward_as_tuple(
                                        std::to_string(key) + "-" + std::to_string(t)
                                )
                        );

                        std::lock_guard lock(mtx);
                        stored.push_back(p);
                    }
                });
            }

            for (auto &w: workers) {
                w.join();
            }

            auto [capacity, size] = pool.occupancy_rate();
            REQUIRE(size == UNIQUE_KEYS);
            REQUIRE(capacity >= UNIQUE_KEYS);

            stored.clear();
            pool.resize_pool();

            REQUIRE(pool.size() == 0);
        }
    }
}
#endif
