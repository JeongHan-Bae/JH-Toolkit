#define TINY_TEST_MAIN
#include "jh/test/tiny_test/tiny_test.hpp"

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

#include "jh/pool"
#include "jh/macros/platform.h"
#include "../support/pool_test_mutex.hpp"
#include <memory>
#include <memory_resource>
#include <atomic>
#include <latch>
#include <vector>
#include <thread>

/**
 * @brief Tests pooling, handle lifetime, slot reuse, and configured mutex behavior.
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

    struct TestObjectHash {
        std::size_t operator()(const TestObject &value) const noexcept {
            return std::hash<int>{}(value.value);
        }
    };

    struct TestObjectEq {
        bool operator()(const TestObject &lhs, const TestObject &rhs) const noexcept {
            return lhs.value == rhs.value;
        }
    };

    using CustomizedPool = jh::conc::pointer_pool<TestObject, TestObjectHash, TestObjectEq, pool_test_mutex>;
    using DeducedPool = jh::observe_pool<AutoPoolingObject, pool_test_mutex>;
    using ResourceSet = jh::resource_pool_set<int, std::allocator<int>, pool_test_mutex>;
    using ResourceMap = jh::resource_pool<
        int, std::unique_ptr<std::string>,
        std::allocator<std::pair<int, std::unique_ptr<std::string>>>, pool_test_mutex>;
} // namespace test

// Basic Functionality Test

namespace test {
void tiny_test_case_1() {
    test::CustomizedPool pool;

    auto obj1 = pool.acquire(10);
    auto obj2 = pool.acquire(10);
    auto obj3 = pool.acquire(20);

    jh::test::tiny_test::expect(static_cast<bool>((obj1 == obj2)), "obj1 == obj2"); // Objects with the same value should be reused
    jh::test::tiny_test::expect(static_cast<bool>((obj1 != obj3)), "obj1 != obj3"); // Different values should not be reused
    jh::test::tiny_test::expect(static_cast<bool>((pool.size() == 2)), "pool.size() == 2"); // The pool should contain only two unique objects

}
}
template<>
struct jh::test::tiny_test::test<"pointer_pool basic functionality">
    : jh::test::tiny_test::test_definition<"pointer_pool basic functionality", &::test::tiny_test_case_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_pool case 1">
    : jh::test::tiny_test::session_definition<
          "test module test_pool case 1", jh::test::tiny_test::test<"pointer_pool basic functionality">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pool case 1"> registration_1{};
}



namespace test {
void tiny_test_case_2() {
    test::DeducedPool pool;

    auto obj1 = pool.acquire(10);
    auto obj2 = pool.acquire(10);
    auto obj3 = pool.acquire(20);

    jh::test::tiny_test::expect(static_cast<bool>((obj1 == obj2)), "obj1 == obj2"); // Objects with the same value should be reused
    jh::test::tiny_test::expect(static_cast<bool>((obj1 != obj3)), "obj1 != obj3"); // Different values should not be reused
    jh::test::tiny_test::expect(static_cast<bool>((pool.size() == 2)), "pool.size() == 2"); // The pool should contain only two unique objects

}
}
template<>
struct jh::test::tiny_test::test<"observe_pool basic functionality">
    : jh::test::tiny_test::test_definition<"observe_pool basic functionality", &::test::tiny_test_case_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_pool case 2">
    : jh::test::tiny_test::session_definition<
          "test module test_pool case 2", jh::test::tiny_test::test<"observe_pool basic functionality">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pool case 2"> registration_2{};
}


// Cleanup Test (Effect of Eq)

namespace test {
void tiny_test_case_3() {
    test::CustomizedPool pool;

    auto obj1 = pool.acquire(10);
    auto obj2 = pool.acquire(20);

    jh::test::tiny_test::expect(static_cast<bool>((pool.size() == 2)), "pool.size() == 2");

    obj1.reset();
    obj2.reset();

    jh::test::tiny_test::expect(pool.size() == 0, "released handles leave no observed entries");

    pool.cleanup(); // Manually trigger cleanup

    jh::test::tiny_test::expect(static_cast<bool>((pool.size() == 0)), "pool.size() == 0"); // The pool should now be empty

}
}
template<>
struct jh::test::tiny_test::test<"pointer_pool cleanup">
    : jh::test::tiny_test::test_definition<"pointer_pool cleanup", &::test::tiny_test_case_3> {};
template<>
struct jh::test::tiny_test::session<"test module test_pool case 3">
    : jh::test::tiny_test::session_definition<
          "test module test_pool case 3", jh::test::tiny_test::test<"pointer_pool cleanup">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pool case 3"> registration_3{};
}


// Cleanup Test (Effect of Eq)

namespace test {
void tiny_test_case_4() {
    test::DeducedPool pool;

    auto obj1 = pool.acquire(10);
    auto obj2 = pool.acquire(20);

    jh::test::tiny_test::expect(static_cast<bool>((pool.size() == 2)), "pool.size() == 2");

    obj1.reset();
    obj2.reset();

    jh::test::tiny_test::expect(pool.size() == 0, "released handles leave no observed entries");

    pool.cleanup(); // Manually trigger cleanup

    jh::test::tiny_test::expect(static_cast<bool>((pool.size() == 0)), "pool.size() == 0"); // The pool should now be empty

}
}
template<>
struct jh::test::tiny_test::test<"observe_pool cleanup">
    : jh::test::tiny_test::test_definition<"observe_pool cleanup", &::test::tiny_test_case_4> {};
template<>
struct jh::test::tiny_test::session<"test module test_pool case 4">
    : jh::test::tiny_test::session_definition<
          "test module test_pool case 4", jh::test::tiny_test::test<"observe_pool cleanup">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pool case 4"> registration_4{};
}


// Dynamic Expansion & Contraction Test

namespace test {
void tiny_test_case_5() {
    test::CustomizedPool pool(4); // Initial reserved_size = 4

    std::vector<test::CustomizedPool::ptr> objects;
    objects.reserve(20);
    for (int i = 0; i < 20; ++i) {
        objects.push_back(pool.acquire(i));
    }

    jh::test::tiny_test::expect(pool.size() == 20, "twenty unique objects are active");
    jh::test::tiny_test::expect(pool.capacity() == 32, "full slot tables double capacity");

    for (auto &obj: objects) {
        obj.reset();
    }

    pool.cleanup();
    jh::test::tiny_test::expect(pool.size() == 0 && pool.capacity() == 32,
                                "cleanup redistributes empty slots without shrinking");
    pool.cleanup_shrink();
    jh::test::tiny_test::expect(pool.capacity() == 16, "explicit shrink halves capacity");

}
}
template<>
struct jh::test::tiny_test::test<"pointer_pool dynamic expansion and contraction">
    : jh::test::tiny_test::test_definition<"pointer_pool dynamic expansion and contraction", &::test::tiny_test_case_5> {};
template<>
struct jh::test::tiny_test::session<"test module test_pool case 5">
    : jh::test::tiny_test::session_definition<
          "test module test_pool case 5", jh::test::tiny_test::test<"pointer_pool dynamic expansion and contraction">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pool case 5"> registration_5{};
}


// Dynamic Expansion & Contraction Test

namespace test {
void tiny_test_case_6() {
    test::DeducedPool pool(4); // Initial reserved_size = 4

    std::vector<test::DeducedPool::ptr> objects;
    objects.reserve(20);
    for (int i = 0; i < 20; ++i) {
        objects.push_back(pool.acquire(i));
    }

    jh::test::tiny_test::expect(pool.size() == 20, "twenty unique objects are active");
    jh::test::tiny_test::expect(pool.capacity() == 32, "full slot tables double capacity");

    for (auto &obj: objects) {
        obj.reset();
    }

    pool.cleanup();
    jh::test::tiny_test::expect(pool.size() == 0 && pool.capacity() == 32,
                                "cleanup redistributes empty slots without shrinking");
    pool.cleanup_shrink();
    jh::test::tiny_test::expect(pool.capacity() == 16, "explicit shrink halves capacity");

}
}
template<>
struct jh::test::tiny_test::test<"observe_pool dynamic expansion and contraction">
    : jh::test::tiny_test::test_definition<"observe_pool dynamic expansion and contraction", &::test::tiny_test_case_6> {};
template<>
struct jh::test::tiny_test::session<"test module test_pool case 6">
    : jh::test::tiny_test::session_definition<
          "test module test_pool case 6", jh::test::tiny_test::test<"observe_pool dynamic expansion and contraction">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pool case 6"> registration_6{};
}


// Move Semantics Test

namespace test {
void tiny_test_case_7() {
    test::CustomizedPool pool1;
    auto obj1 = pool1.acquire(10);
    auto obj2 = pool1.acquire(20);

    jh::test::tiny_test::expect(static_cast<bool>((pool1.size() == 2)), "pool1.size() == 2");

    test::CustomizedPool pool2 = std::move(pool1); // Move constructor

    jh::test::tiny_test::expect(static_cast<bool>((pool2.size() == 2)), "pool2.size() == 2");
    jh::test::tiny_test::expect(static_cast<bool>((pool1.size() == 0)), "pool1.size() == 0"); // pool1 should now be empty

    test::CustomizedPool pool3;
    pool3 = std::move(pool2); // Move assignment

    jh::test::tiny_test::expect(static_cast<bool>((pool3.size() == 2)), "pool3.size() == 2");
    jh::test::tiny_test::expect(static_cast<bool>((pool2.size() == 0)), "pool2.size() == 0"); // pool2 should now be empty

    pool3.clear();
    jh::test::tiny_test::expect(static_cast<bool>((pool3.size() == 0)), "pool3.size() == 0"); // pool3 should now be empty
    jh::test::tiny_test::expect(static_cast<bool>((pool3.capacity() == test::CustomizedPool::MIN_RESERVED_SIZE)), "pool3.capacity() == test::CustomizedPool::MIN_RESERVED_SIZE"); // reserved_size should be reset

}
}
template<>
struct jh::test::tiny_test::test<"pointer_pool move semantics">
    : jh::test::tiny_test::test_definition<"pointer_pool move semantics", &::test::tiny_test_case_7> {};
template<>
struct jh::test::tiny_test::session<"test module test_pool case 7">
    : jh::test::tiny_test::session_definition<
          "test module test_pool case 7", jh::test::tiny_test::test<"pointer_pool move semantics">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pool case 7"> registration_7{};
}



namespace test {
void tiny_test_case_8() {
    test::DeducedPool pool1;
    auto obj1 = pool1.acquire(10);
    auto obj2 = pool1.acquire(20);

    jh::test::tiny_test::expect(static_cast<bool>((pool1.size() == 2)), "pool1.size() == 2");

    test::DeducedPool pool2 = std::move(pool1); // Move constructor

    jh::test::tiny_test::expect(static_cast<bool>((pool2.size() == 2)), "pool2.size() == 2");
    jh::test::tiny_test::expect(static_cast<bool>((pool1.size() == 0)), "pool1.size() == 0"); // pool1 should now be empty

    test::DeducedPool pool3;
    pool3 = std::move(pool2); // Move assignment

    jh::test::tiny_test::expect(static_cast<bool>((pool3.size() == 2)), "pool3.size() == 2");
    jh::test::tiny_test::expect(static_cast<bool>((pool2.size() == 0)), "pool2.size() == 0"); // pool2 should now be empty

    pool3.clear();
    jh::test::tiny_test::expect(static_cast<bool>((pool3.size() == 0)), "pool3.size() == 0"); // pool3 should now be empty
    jh::test::tiny_test::expect(static_cast<bool>((pool3.capacity() == test::DeducedPool::MIN_RESERVED_SIZE)), "pool3.capacity() == test::DeducedPool::MIN_RESERVED_SIZE"); // reserved_size should be reset

}
}
template<>
struct jh::test::tiny_test::test<"observe_pool move semantics">
    : jh::test::tiny_test::test_definition<"observe_pool move semantics", &::test::tiny_test_case_8> {};
template<>
struct jh::test::tiny_test::session<"test module test_pool case 8">
    : jh::test::tiny_test::session_definition<
          "test module test_pool case 8", jh::test::tiny_test::test<"observe_pool move semantics">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pool case 8"> registration_8{};
}



/**
 * @note
 * std::string itself is <b>not</b> an immutable type — its internal buffer may change.
 * This test only demonstrates that it <b>can</b> be pooled because it satisfies
 * <code>std::hash&lt;std::string&gt;</code> and <code>operator==</code<.
 * For stable, non-static, content-based pooling, use <code>jh::observe_pool&lt;jh::immutable_str&lt;&gt;&gt;</code> instead.
*/

namespace test {
void tiny_test_case_9() {
    jh::observe_pool<std::string, test::pool_test_mutex> pool;

    auto hello1 = pool.acquire("hello");
    auto hello2 = pool.acquire("hello");
    auto world = pool.acquire("world");

    jh::test::tiny_test::expect(static_cast<bool>((hello1 == hello2)), "hello1 == hello2");   // identical strings should be reused
    jh::test::tiny_test::expect(static_cast<bool>((hello1 != world)), "hello1 != world");    // distinct strings should not be reused
    jh::test::tiny_test::expect(static_cast<bool>((pool.size() == 2)), "pool.size() == 2");   // only two unique entries in the pool

    hello1.reset();
    hello2.reset();
    world.reset();

    jh::test::tiny_test::expect(pool.size() == 0, "released string handles leave no entries");
    pool.cleanup();
    jh::test::tiny_test::expect(static_cast<bool>((pool.size() == 0)), "pool.size() == 0");   // after cleanup, pool becomes empty

}
}
template<>
struct jh::test::tiny_test::test<"observe_pool with std::string">
    : jh::test::tiny_test::test_definition<"observe_pool with std::string", &::test::tiny_test_case_9> {};
template<>
struct jh::test::tiny_test::session<"test module test_pool case 9">
    : jh::test::tiny_test::session_definition<
          "test module test_pool case 9", jh::test::tiny_test::test<"observe_pool with std::string">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pool case 9"> registration_9{};
}



namespace test {
void tiny_test_case_10() {
    test::ResourceSet pool;

    auto p1 = pool.acquire(42);
    jh::test::tiny_test::expect(static_cast<bool>((*p1 == 42)), "*p1 == 42");

    {
        auto p2 = p1;
        jh::test::tiny_test::expect(static_cast<bool>((*p2 == 42)), "*p2 == 42");
        jh::test::tiny_test::expect(static_cast<bool>((pool.size() == 1)), "pool.size() == 1");
    }

    jh::test::tiny_test::expect(static_cast<bool>((*p1 == 42)), "*p1 == 42");

    p1.reset();

    {
        std::vector<test::ResourceSet::ptr> vec;
        vec.reserve(20);
        for (int i = 0; i < 20; ++i)
            vec.emplace_back(pool.acquire(i));
        jh::test::tiny_test::expect(static_cast<bool>((pool.size() == 20)), "pool.size() == 20");
    }

    auto p3 = pool.acquire(99);
    jh::test::tiny_test::expect(static_cast<bool>((*p3 == 99)), "*p3 == 99");

    {
        const auto [capacity, size] = pool.occupancy_rate();
        jh::test::tiny_test::expect(static_cast<bool>((capacity >= size)), "capacity >= size");
    }

    pool.resize_pool();

    {
        const auto [capacity, size] = pool.occupancy_rate();
        jh::test::tiny_test::expect(static_cast<bool>((capacity >= size)), "capacity >= size");
    }

    p3.reset();

    auto p4 = pool.find(99);
    jh::test::tiny_test::expect(static_cast<bool>((p4 == nullptr)), "p4 == nullptr");

    jh::test::tiny_test::expect(static_cast<bool>((pool.empty())), "pool.empty()");

}
}
template<>
struct jh::test::tiny_test::test<"resource_pool_set single-thread basic usage">
    : jh::test::tiny_test::test_definition<"resource_pool_set single-thread basic usage", &::test::tiny_test_case_10> {};
template<>
struct jh::test::tiny_test::session<"test module test_pool case 10">
    : jh::test::tiny_test::session_definition<
          "test module test_pool case 10", jh::test::tiny_test::test<"resource_pool_set single-thread basic usage">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pool case 10"> registration_10{};
}



namespace test {
void tiny_test_case_11() {
    test::ResourceMap pool;

    auto p1 = pool.acquire(1, std::tie("hello"));
    jh::test::tiny_test::expect(static_cast<bool>((p1->first == 1)), "p1->first == 1");
    jh::test::tiny_test::expect(static_cast<bool>((*p1->second == "hello")), "*p1->second == \"hello\"");

    auto p2 = pool.acquire(2, std::forward_as_tuple("world"));
    jh::test::tiny_test::expect(static_cast<bool>((*p2->second == "world")), "*p2->second == \"world\"");

    // same key: value constructor must not be evaluated
    auto p3 = pool.acquire(1, std::tie("ignored"));
    jh::test::tiny_test::expect(static_cast<bool>((*p3->second == "hello")), "*p3->second == \"hello\"");

    p1.reset();
    p2.reset();
    p3.reset();

    auto p4 = pool.acquire(3, std::forward_as_tuple("new"));
    jh::test::tiny_test::expect(static_cast<bool>((*p4->second == "new")), "*p4->second == \"new\"");

    auto check0 = static_cast<bool>(pool.find(3));
    auto check1 = static_cast<bool>(pool.find(1));

    jh::test::tiny_test::expect(static_cast<bool>((check0)), "check0");
    jh::test::tiny_test::expect(static_cast<bool>((check1 == false)), "check1 == false");

}
}
template<>
struct jh::test::tiny_test::test<"resource_pool single-thread key-value">
    : jh::test::tiny_test::test_definition<"resource_pool single-thread key-value", &::test::tiny_test_case_11> {};
template<>
struct jh::test::tiny_test::session<"test module test_pool case 11">
    : jh::test::tiny_test::session_definition<
          "test module test_pool case 11", jh::test::tiny_test::test<"resource_pool single-thread key-value">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pool case 11"> registration_11{};
}



namespace test {
    struct observed_mutex {
        pool_test_mutex inner;
        static inline std::atomic<bool> observe{false};
        static inline std::atomic<int> attempts{0};
        static inline std::latch *second_attempt = nullptr;
        static inline std::atomic<bool> observe_shared{false};
        static inline std::latch *shared_attempt = nullptr;
        static inline std::atomic<int> exclusive_calls{0};
        static inline std::atomic<int> shared_calls{0};
        static inline std::atomic<bool> pause_first_lock{false};
        static inline std::latch *first_locked = nullptr;
        static inline std::latch *release_first = nullptr;

        void lock() {
            exclusive_calls.fetch_add(1);
            const int order = observe.load() ? attempts.fetch_add(1) : -1;
            if (order == 1) second_attempt->count_down();
            inner.lock();
            if (order == 0 && pause_first_lock.load()) {
                first_locked->count_down();
                release_first->wait();
            }
        }
        void unlock() { inner.unlock(); }
        bool try_lock() { return inner.try_lock(); }
        void lock_shared() {
            shared_calls.fetch_add(1);
            if (observe_shared.load()) shared_attempt->count_down();
            inner.lock_shared();
        }
        void unlock_shared() { inner.unlock_shared(); }
        bool try_lock_shared() { return inner.try_lock_shared(); }
    };

    struct controlled_value {
        int key;
        explicit controlled_value(int value) : key(value) {}
        std::size_t hash() const noexcept { return static_cast<std::size_t>(key); }
        bool operator==(const controlled_value &other) const noexcept { return key == other.key; }
    };

    void capacity_covers_concurrent_insertions() {
        using pool_type = jh::conc::pointer_pool<
            controlled_value, jh::hash<controlled_value>,
            std::equal_to<controlled_value>, observed_mutex>;
        pool_type pool;
        std::vector<pool_type::ptr> held(17);
        for (int key = 0; key < 15; ++key) held[key] = pool.acquire(key);

        std::latch first_locked(1), release_first(1), second_attempt(1);
        observed_mutex::second_attempt = &second_attempt;
        observed_mutex::first_locked = &first_locked;
        observed_mutex::release_first = &release_first;
        observed_mutex::attempts.store(0);
        observed_mutex::observe.store(true);
        observed_mutex::pause_first_lock.store(true);
        std::thread first([&] { held[15] = pool.acquire(15); });
        first_locked.wait();
        std::thread second([&] { held[16] = pool.acquire(16); });
        second_attempt.wait();
        release_first.count_down();
        first.join();
        second.join();
        observed_mutex::pause_first_lock.store(false);
        observed_mutex::observe.store(false);

        jh::test::tiny_test::expect(pool.size() == 17, "both inserts complete");
        jh::test::tiny_test::expect(pool.capacity() >= pool.size(),
                                    "capacity covers all stored objects");
    }
}
template<>
struct jh::test::tiny_test::test<"concurrent pool capacity">
    : jh::test::tiny_test::test_definition<
          "concurrent pool capacity", &::test::capacity_covers_concurrent_insertions> {};
template<>
struct jh::test::tiny_test::session<"test module test_pool case 12">
    : jh::test::tiny_test::session_definition<
          "test module test_pool case 12", jh::test::tiny_test::test<"concurrent pool capacity">> {};
namespace test {
    [[maybe_unused]] const jh::test::tiny_test::session<"test module test_pool case 12"> registration_12{};
}

namespace test {
    struct blocking_pool_value {
        int payload;
        static inline std::atomic<int> constructions{0};

        explicit blocking_pool_value(int value, std::latch *entered, std::latch *release)
            : payload(value) {
            constructions.fetch_add(1);
            entered->count_down();
            release->wait();
        }
    };

    void injected_pool_mutex_coordinates_acquisition() {
        using pool_type = jh::resource_pool<
            int, blocking_pool_value, std::allocator<std::pair<int, blocking_pool_value>>, observed_mutex>;
        pool_type pool;
        blocking_pool_value::constructions.store(0);
        observed_mutex::exclusive_calls.store(0);
        observed_mutex::shared_calls.store(0);
        std::latch constructor_entered(1), release_constructor(1), shared_attempt(1);
        observed_mutex::shared_attempt = &shared_attempt;
        pool_type::ptr first, second;
        std::thread writer([&] {
            first = pool.acquire(7, std::forward_as_tuple(42, &constructor_entered, &release_constructor));
        });
        constructor_entered.wait();
        observed_mutex::observe_shared.store(true);
        std::thread reader([&] {
            second = pool.acquire(7, std::forward_as_tuple(99, &constructor_entered, &release_constructor));
        });
        shared_attempt.wait();
        observed_mutex::observe_shared.store(false);
        release_constructor.count_down();
        writer.join();
        reader.join();

        jh::test::tiny_test::expect(blocking_pool_value::constructions.load() == 1,
                                    "shared lock prevents duplicate value construction");
        jh::test::tiny_test::expect(&*first == &*second, "contending acquisitions share one slot");
        jh::test::tiny_test::expect(first->second.payload == 42, "first committed value remains visible");
        jh::test::tiny_test::expect(observed_mutex::exclusive_calls.load() >= 2,
                                    "index and storage exclusive locks were acquired");
        jh::test::tiny_test::expect(observed_mutex::shared_calls.load() >= 2,
                                    "both acquisitions reached the index shared lock");

        jh::resource_pool_set<int, std::allocator<int>, observed_mutex> set;
        auto set_item = set.acquire(11);
        jh::test::tiny_test::expect(*set_item == 11, "set alias accepts an injected RW mutex");

        jh::observe_pool<int, observed_mutex> observed;
        auto observed_item = observed.acquire(13);
        jh::test::tiny_test::expect(*observed_item == 13, "observer alias accepts an injected RW mutex");
    }
}
template<>
struct jh::test::tiny_test::test<"injected pool RW mutex">
    : jh::test::tiny_test::test_definition<
          "injected pool RW mutex", &::test::injected_pool_mutex_coordinates_acquisition> {};
template<>
struct jh::test::tiny_test::session<"test module test_pool case 13">
    : jh::test::tiny_test::session_definition<
          "test module test_pool case 13", jh::test::tiny_test::test<"injected pool RW mutex">> {};
namespace test {
    [[maybe_unused]] const jh::test::tiny_test::session<"test module test_pool case 13"> registration_13{};
}

namespace test {
    void observed_handle_uses_configured_mutex() {
        jh::observe_pool<int, observed_mutex> pool;
        auto first = pool.acquire(42);
        const int before_copy = observed_mutex::exclusive_calls.load();
        auto second = first;
        jh::test::tiny_test::expect(observed_mutex::exclusive_calls.load() > before_copy,
                                    "copying an observed handle uses the configured mutex");
        const int before_release = observed_mutex::exclusive_calls.load();
        second.reset();
        jh::test::tiny_test::expect(observed_mutex::exclusive_calls.load() > before_release,
                                    "releasing an observed handle uses the configured mutex");
    }
}
template<>
struct jh::test::tiny_test::test<"observed handle uses configured mutex">
    : jh::test::tiny_test::test_definition<
          "observed handle uses configured mutex", &::test::observed_handle_uses_configured_mutex> {};
template<>
struct jh::test::tiny_test::session<"test module test_pool case 14">
    : jh::test::tiny_test::session_definition<
          "test module test_pool case 14", jh::test::tiny_test::test<"observed handle uses configured mutex">> {};
namespace test {
    [[maybe_unused]] const jh::test::tiny_test::session<"test module test_pool case 14"> registration_14{};
}

namespace test {
    struct collision_hash {
        std::size_t operator()(const TestObject &) const noexcept { return 7; }
    };

    void observed_handle_survives_pool_and_hash_collisions() {
        using pool_type = jh::conc::pointer_pool<TestObject, collision_hash, TestObjectEq, observed_mutex>;
        pool_type::ptr held;
        {
            pool_type pool;
            auto first = pool.acquire(10);
            auto different = pool.acquire(20);
            held = pool.acquire(10);
            jh::test::tiny_test::expect(first == held && first != different,
                                        "equal values deduplicate within a hash collision");
            jh::test::tiny_test::expect(pool.size() == 2, "collision bucket has two distinct objects");
            auto third = pool.acquire(30);
            different.reset();
            jh::test::tiny_test::expect(third == pool.acquire(30),
                                        "probing finds a value beyond an expired collision slot");
            pool.cleanup();
            jh::test::tiny_test::expect(third == pool.acquire(30),
                                        "index rebuild preserves colliding values");
            pool.clear();
            jh::test::tiny_test::expect(held->value == 10, "clear preserves held objects");
            auto after_clear = pool.acquire(10);
            jh::test::tiny_test::expect(after_clear != held, "clear resets the observation index");
        }
        jh::test::tiny_test::expect(held->value == 10, "handle remains valid after pool destruction");
        held.reset();
    }
}
template<>
struct jh::test::tiny_test::test<"observed handle lifetime and collisions">
    : jh::test::tiny_test::test_definition<
          "observed handle lifetime and collisions", &::test::observed_handle_survives_pool_and_hash_collisions> {};
template<>
struct jh::test::tiny_test::session<"test module test_pool case 15">
    : jh::test::tiny_test::session_definition<
          "test module test_pool case 15", jh::test::tiny_test::test<"observed handle lifetime and collisions">> {};
namespace test {
    [[maybe_unused]] const jh::test::tiny_test::session<"test module test_pool case 15"> registration_15{};
}

namespace test {
    struct counting_hash {
        static inline std::atomic<int> calls{0};
        std::size_t operator()(const TestObject &value) const noexcept {
            calls.fetch_add(1);
            return std::hash<int>{}(value.value);
        }
    };

    void pointer_pool_hashes_each_candidate_once() {
        using pool_type = jh::conc::pointer_pool<TestObject, counting_hash, TestObjectEq, pool_test_mutex>;
        counting_hash::calls.store(0);
        pool_type pool;
        auto first = pool.acquire(7);
        auto duplicate = pool.acquire(7);
        auto distinct = pool.acquire(9);
        jh::test::tiny_test::expect(first == duplicate && first != distinct,
                                    "cached hashes retain content-based deduplication");
        jh::test::tiny_test::expect(counting_hash::calls.load() == 3,
                                    "each candidate is hashed once");
        duplicate.reset();
        first.reset();
        distinct.reset();
        pool.cleanup();
        jh::test::tiny_test::expect(counting_hash::calls.load() == 3,
                                    "release and cleanup reuse cached hashes");
    }
}
template<>
struct jh::test::tiny_test::test<"pointer pool caches candidate hashes">
    : jh::test::tiny_test::test_definition<
          "pointer pool caches candidate hashes", &::test::pointer_pool_hashes_each_candidate_once> {};
template<>
struct jh::test::tiny_test::session<"test module test_pool case 16">
    : jh::test::tiny_test::session_definition<
          "test module test_pool case 16", jh::test::tiny_test::test<"pointer pool caches candidate hashes">> {};
namespace test {
    [[maybe_unused]] const jh::test::tiny_test::session<"test module test_pool case 16"> registration_16{};
}

namespace test {
    struct tracked_observed_value {
        static inline std::atomic<int> live{0};
        int key;

        explicit tracked_observed_value(int value) : key(value) { ++live; }
        ~tracked_observed_value() { --live; }
    };

    struct tracked_observed_hash {
        std::size_t operator()(const tracked_observed_value &value) const noexcept {
            return static_cast<std::size_t>(value.key);
        }
    };

    struct tracked_observed_eq {
        bool operator()(const tracked_observed_value &lhs,
                        const tracked_observed_value &rhs) const noexcept {
            return lhs.key == rhs.key;
        }
    };

    void observed_slot_reuses_a_different_hash() {
        using pool_type = jh::conc::pointer_pool<
            tracked_observed_value, tracked_observed_hash, tracked_observed_eq, pool_test_mutex>;
        tracked_observed_value::live.store(0);
        {
            pool_type pool;
            std::vector<pool_type::ptr> held;
            for (int key = 0; key < 16; ++key) held.push_back(pool.acquire(key));
            const auto capacity = pool.capacity();
            held[7].reset();
            jh::test::tiny_test::expect(tracked_observed_value::live.load() == 15,
                                        "last handle destroys the observed object immediately");
            held[7] = pool.acquire(100);
            jh::test::tiny_test::expect(pool.capacity() == capacity,
                                        "a different hash reuses an expired slot without expansion");
            jh::test::tiny_test::expect(pool.size() == 16 && tracked_observed_value::live.load() == 16,
                                        "replacement occupies the vacated slot");
        }
        jh::test::tiny_test::expect(tracked_observed_value::live.load() == 0,
                                    "all observed objects are destroyed after their handles");
    }
}
template<>
struct jh::test::tiny_test::test<"observed slots reuse different hashes">
    : jh::test::tiny_test::test_definition<
          "observed slots reuse different hashes", &::test::observed_slot_reuses_a_different_hash> {};
template<>
struct jh::test::tiny_test::session<"test module test_pool case 17">
    : jh::test::tiny_test::session_definition<
          "test module test_pool case 17", jh::test::tiny_test::test<"observed slots reuse different hashes">> {};
namespace test {
    [[maybe_unused]] const jh::test::tiny_test::session<"test module test_pool case 17"> registration_17{};
}

namespace test {
    void observed_sorted_slots_keep_identity_across_reuse() {
        using pool_type = jh::conc::pointer_pool<
            tracked_observed_value, tracked_observed_hash, tracked_observed_eq, pool_test_mutex>;
        tracked_observed_value::live.store(0);
        {
            pool_type pool(8);
            std::vector<pool_type::ptr> held;
            for (int key : {0, 10, 20, 30, 40, 50, 60, -1})
                held.push_back(pool.acquire(key));
            held[1].reset();
            held[5].reset();
            held[1] = pool.acquire(5);
            held[5] = pool.acquire(35);
            jh::test::tiny_test::expect(pool.capacity() == 8,
                                        "nearby empty slots accept new hashes without expansion");
            for (const auto &handle : held)
                jh::test::tiny_test::expect(pool.acquire(handle->key) == handle,
                                            "sorted lookup keeps each canonical object after local shifts");
            auto expanded = pool.acquire(70);
            jh::test::tiny_test::expect(pool.capacity() >= 9 && pool.size() == 9,
                                        "full live storage expands and retains all objects");
            held[2].reset();
            pool.cleanup();
            jh::test::tiny_test::expect(pool.acquire(expanded->key) == expanded,
                                        "rebuilding distributed empty slots preserves lookup");
            jh::test::tiny_test::expect(pool.acquire(-1) == held[7],
                                        "maximum hash remains findable");
        }
        {
            pool_type pool(4);
            std::vector<pool_type::ptr> held(64);
            for (int key = 63; key >= 0; --key) held[key] = pool.acquire(key * 3);
            for (int key = 0; key < 64; key += 3) held[key].reset();
            const auto capacity = pool.capacity();
            for (int key = 0; key < 64; key += 3) held[key] = pool.acquire(1000 + key);
            jh::test::tiny_test::expect(pool.capacity() == capacity,
                                        "repeated non-colliding insertions reuse the existing slots");
            for (const auto &handle : held)
                jh::test::tiny_test::expect(pool.acquire(handle->key) == handle,
                                            "all canonical objects survive repeated local shifts");
        }
        jh::test::tiny_test::expect(tracked_observed_value::live.load() == 0,
                                    "sparse slot test releases every object");
    }
}
template<>
struct jh::test::tiny_test::test<"observed sorted slots preserve identity">
    : jh::test::tiny_test::test_definition<
          "observed sorted slots preserve identity", &::test::observed_sorted_slots_keep_identity_across_reuse> {};
template<>
struct jh::test::tiny_test::session<"test module test_pool case 18">
    : jh::test::tiny_test::session_definition<
          "test module test_pool case 18", jh::test::tiny_test::test<"observed sorted slots preserve identity">> {};
namespace test {
    [[maybe_unused]] const jh::test::tiny_test::session<"test module test_pool case 18"> registration_18{};
}

namespace test {
    struct state_switch_mutex {
        pool_test_mutex inner;
        const int identity = next_identity.fetch_add(1);
        static inline std::atomic<int> next_identity{0};
        static inline std::atomic<bool> watch_exclusive{false};
        static inline std::atomic<bool> watch_shared{false};
        static inline std::atomic<int> observed_shared_identity{-1};
        static inline std::latch *exclusive_attempt = nullptr;
        static inline std::latch *shared_attempt = nullptr;

        void lock() {
            if (watch_exclusive.exchange(false)) exclusive_attempt->count_down();
            inner.lock();
        }
        void unlock() { inner.unlock(); }
        bool try_lock() { return inner.try_lock(); }
        void lock_shared() {
            if (watch_shared.exchange(false)) {
                observed_shared_identity.store(identity);
                shared_attempt->count_down();
            }
            inner.lock_shared();
        }
        void unlock_shared() { inner.unlock_shared(); }
        bool try_lock_shared() { return inner.try_lock_shared(); }
    };

    struct pausing_eq {
        static inline std::atomic<bool> pause{false};
        static inline std::latch *entered = nullptr;
        static inline std::latch *release = nullptr;

        bool operator()(const TestObject &lhs, const TestObject &rhs) const {
            if (pause.exchange(false)) {
                entered->count_down();
                release->wait();
            }
            return lhs.value == rhs.value;
        }
    };

    void clear_and_acquire_share_the_state_boundary() {
        using pool_type = jh::conc::pointer_pool<
            TestObject, TestObjectHash, pausing_eq, state_switch_mutex>;
        pool_type pool;
        auto held = pool.acquire(42);

        std::latch baseline_attempt(1);
        state_switch_mutex::shared_attempt = &baseline_attempt;
        state_switch_mutex::watch_shared.store(true);
        (void)pool.size();
        const int baseline_identity = state_switch_mutex::observed_shared_identity.load();

        std::latch eq_entered(1), release_eq(1), clear_attempt(1), size_attempt(1);
        pausing_eq::entered = &eq_entered;
        pausing_eq::release = &release_eq;
        pausing_eq::pause.store(true);
        pool_type::ptr duplicate;
        std::thread acquiring([&] { duplicate = pool.acquire(42); });
        eq_entered.wait();

        state_switch_mutex::exclusive_attempt = &clear_attempt;
        state_switch_mutex::watch_exclusive.store(true);
        std::thread clearing([&] { pool.clear(); });
        clear_attempt.wait();

        state_switch_mutex::shared_attempt = &size_attempt;
        state_switch_mutex::watch_shared.store(true);
        std::thread reading([&] { (void)pool.size(); });
        size_attempt.wait();
        const int during_clear_identity = state_switch_mutex::observed_shared_identity.load();

        release_eq.count_down();
        acquiring.join();
        clearing.join();
        reading.join();
        jh::test::tiny_test::expect(during_clear_identity == baseline_identity,
                                    "clear and acquire keep the same configured mutex boundary");
        jh::test::tiny_test::expect(held == duplicate && pool.size() == 0,
                                    "clearing preserves existing handles and starts a fresh index");
    }
}
template<>
struct jh::test::tiny_test::test<"clear and acquire share the configured mutex">
    : jh::test::tiny_test::test_definition<
          "clear and acquire share the configured mutex", &::test::clear_and_acquire_share_the_state_boundary> {};
template<>
struct jh::test::tiny_test::session<"test module test_pool case 19">
    : jh::test::tiny_test::session_definition<
          "test module test_pool case 19", jh::test::tiny_test::test<"clear and acquire share the configured mutex">> {};
namespace test {
    [[maybe_unused]] const jh::test::tiny_test::session<"test module test_pool case 19"> registration_19{};
}
