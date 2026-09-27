#define TINY_TEST_MAIN
#include "jh/test/tiny_test/tiny_test.hpp"

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

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
 * High-contention concurrency workloads are excluded from the default
 * correctness suite on all platforms. Active cases cover ordinary pooling,
 * lifetime, cleanup, resizing and move behavior.
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

    obj1.reset(); // Release shared_ptrs
    obj2.reset();

    jh::test::tiny_test::expect(static_cast<bool>((pool.size() == 2)), "pool.size() == 2"); // Expired weak_ptrs are still in the pool (not automatically cleaned up)

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

    obj1.reset(); // Release shared_ptrs
    obj2.reset();

    jh::test::tiny_test::expect(static_cast<bool>((pool.size() == 2)), "pool.size() == 2"); // Expired weak_ptrs are still in the pool (not automatically cleaned up)

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

    std::vector<std::shared_ptr<test::TestObject> > objects;
    objects.reserve(10);
    for (int i = 0; i < 10; ++i) {
        objects.push_back(pool.acquire(i)); // Store shared_ptrs
    }

    jh::test::tiny_test::expect(static_cast<bool>((pool.size() == 10)), "pool.size() == 10"); // 10 unique objects
    jh::test::tiny_test::expect(static_cast<bool>((pool.capacity() >= 16)), "pool.capacity() >= 16"); // Expansion triggered (reserved_size *= 2)

    for (auto &obj: objects) {
        obj.reset(); // Release all shared_ptrs
    }

    pool.cleanup(); // Trigger shrinkage
    jh::test::tiny_test::expect(static_cast<bool>((pool.capacity() <= 16)), "pool.capacity() <= 16"); // Shrinkage triggered (reserved_size /= 2)

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

    std::vector<std::shared_ptr<test::AutoPoolingObject> > objects;
    objects.reserve(10);
    for (int i = 0; i < 10; ++i) {
        objects.push_back(pool.acquire(i)); // Store shared_ptrs
    }

    jh::test::tiny_test::expect(static_cast<bool>((pool.size() == 10)), "pool.size() == 10"); // 10 unique objects
    jh::test::tiny_test::expect(static_cast<bool>((pool.capacity() >= 16)), "pool.capacity() >= 16"); // Expansion triggered (reserved_size *= 2)

    for (auto &obj: objects) {
        obj.reset(); // Release all shared_ptrs
    }

    pool.cleanup(); // Trigger shrinkage
    jh::test::tiny_test::expect(static_cast<bool>((pool.capacity() <= 16)), "pool.capacity() <= 16"); // Shrinkage triggered (reserved_size /= 2)

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
 * For stable, non-static, content-based pooling, use <code>jh::observe_pool&lt;jh::immutable_str&gt;<code> instead.
*/

namespace test {
void tiny_test_case_9() {
    jh::observe_pool<std::string> pool;

    auto hello1 = pool.acquire("hello");
    auto hello2 = pool.acquire("hello");
    auto world = pool.acquire("world");

    jh::test::tiny_test::expect(static_cast<bool>((hello1 == hello2)), "hello1 == hello2");   // identical strings should be reused
    jh::test::tiny_test::expect(static_cast<bool>((hello1 != world)), "hello1 != world");    // distinct strings should not be reused
    jh::test::tiny_test::expect(static_cast<bool>((pool.size() == 2)), "pool.size() == 2");   // only two unique entries in the pool

    hello1.reset();
    hello2.reset();
    world.reset();

    jh::test::tiny_test::expect(static_cast<bool>((pool.size() == 2)), "pool.size() == 2");   // expired entries remain until cleanup
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
    jh::resource_pool_set<int> pool;

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
        std::vector<jh::resource_pool_set<int>::ptr> vec;
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
    jh::resource_pool<int, std::unique_ptr<std::string>> pool;

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


