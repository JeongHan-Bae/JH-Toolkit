#define CATCH_CONFIG_MAIN

#include <catch2/catch_all.hpp>
#include "jh/asynchronous/occ_box.h"
#include <thread>
#include <chrono>    // NOLINT force include for std::chrono_literals
#include <vector>
#include <atomic>
#include <string>

using namespace jh::async;

namespace test {

/// A simple type for testing.
    struct Counter {
        int value;

        explicit Counter(int v = 0) : value(v) {}
    };

/// A heavier type with string field for pointer-based tests.
    struct Foo {
        int x;
        std::string name;
    };

} // namespace test


// ==================== Tests ====================
TEST_CASE("occ_box construct from shared_ptr", "[occ_box]") {
    auto sp = std::make_shared<test::Counter>(123);
    occ_box<test::Counter> box(sp);

    REQUIRE(box.read([](const test::Counter &c) { return c.value; }) == 123);
}

TEST_CASE("occ_box basic read/write", "[occ_box]") {
    occ_box<test::Counter> box(0);

    box.write([](test::Counter &c) { c.value = 42; });
    REQUIRE(box.read([](const test::Counter &c) { return c.value; }) == 42);

    auto r = box.try_read([](const test::Counter &c) { return c.value; });
    REQUIRE(r.has_value());
    REQUIRE(r.value() == 42);

    bool ok = box.try_write([](test::Counter &c) { c.value = 77; });
    REQUIRE(ok);
    REQUIRE(box.read([](const test::Counter &c) { return c.value; }) == 77);

    box.write_ptr([](const std::shared_ptr<test::Counter> &old) {
        return std::make_shared<test::Counter>(old->value + 1);
    });
    REQUIRE(box.read([](const test::Counter &c) { return c.value; }) == 78);

    ok = box.try_write_ptr([](const std::shared_ptr<test::Counter> &old) {
        return std::make_shared<test::Counter>(old->value + 10);
    });
    REQUIRE(ok);
    REQUIRE(box.read([](const test::Counter &c) { return c.value; }) == 88);
}

TEST_CASE("occ_box get_version increases", "[occ_box]") {
    occ_box<test::Counter> box(0);
    auto v1 = box.get_version();
    box.write([](test::Counter &c) { c.value = 1; });
    auto v2 = box.get_version();
    REQUIRE(v2 > v1);
}

/**
 * @test occ_box concurrent writes
 *
 * @brief Stress-test for <code>occ_box</code> under high-frequency contention.
 *
 * <h3>Overview</h3>
 * <p>
 * This test deliberately performs <strong>concurrent increments</strong>
 * using <code>N = 8</code> threads and <code>ITER = 1000</code> iterations each,
 * in order to stress-test the atomicity guarantees of
 * <code>occ_box</code> updates.
 * </p>
 *
 * <h3>Test purpose</h3>
 * <ul>
 *   <li>Validate that updates via <code>write()</code> are atomic even under contention.</li>
 *   <li>Ensure readers never observe torn or partially written values.</li>
 *   <li>Confirm that the final accumulated value equals <code>N × ITER</code>.</li>
 * </ul>
 *
 * <h3>Important note</h3>
 * <p>
 * This test uses a <strong>tight loop of writes</strong> only as a
 * <em>stress pattern</em> to catch concurrency bugs.
 * </p>
 * <ul>
 *   <li>In real applications, repeatedly invoking <code>write()</code>
 *       (or <code>write_ptr()</code>) inside a loop is inefficient
 *       and not the intended usage model.</li>
 *   <li>Instead, heavy or repeated logic should be placed inside the
 *       <code>write</code> lambda itself, so that a single commit covers
 *       the whole batch of modifications.</li>
 * </ul>
 *
 * <h3>Practical considerations</h3>
 * <ul>
 *   <li>This test is a <strong>brute-force safety net</strong>:
 *       it checks correctness under contention but is not a performance benchmark.</li>
 *   <li>It exists only to ensure that <code>occ_box</code> internals
 *       cannot produce torn reads or broken atomicity under stress.</li>
 * </ul>
 *
 * @note The runtime cost grows with <code>N × ITER</code>,
 *       but this is acceptable for a stress-test unit.
 */
TEST_CASE("occ_box concurrent writes", "[occ_box][thread]") {
    occ_box<test::Counter> box(0);

    constexpr int N = 8;
    constexpr int ITER = 1000;
    std::vector<std::thread> threads;
    threads.reserve(N);

    for (int i = 0; i < N; i++) {
        threads.emplace_back([&ITER, &box]() { // NOLINT for gcc
            for (int j = 0; j < ITER; j++) {
                box.write([](test::Counter &c) { c.value++; });
            }
        });
    }
    for (auto &t: threads) t.join();

    int final = box.read([](const test::Counter &c) { return c.value; });
    REQUIRE(final == N * ITER);
}

TEST_CASE("occ_box try_read retry statistics", "[occ_box][thread]") {
    occ_box<test::Counter> box(0);

    constexpr int WRITER_ITER = 2000;
    constexpr int READER_ITER = 2000;

    std::atomic<bool> stop{false};

    int success_count = 0;
    int fail_count = 0;

    // Writer thread
    std::thread writer([&box, &stop, &WRITER_ITER]() { // NOLINT for gcc
        for (int i = 0; i < WRITER_ITER; i++) {
            bool ok = false;
            while (!ok) {
                ok = box.try_write([](test::Counter &c) { c.value++; }, 1);
            }
        }
        stop = true;
    });

    // Reader thread
    std::thread reader([&box, &READER_ITER, &success_count, &fail_count]() { // NOLINT for gcc
        for (int i = 0; i < READER_ITER; i++) {
            auto r = box.try_read([](const test::Counter &c) { return c.value; }, 3);
            if (r.has_value()) {
                success_count++;
            } else {
                fail_count++;
            }
        }
    });

    writer.join();
    reader.join();

    REQUIRE(success_count > 0);
    REQUIRE(success_count + fail_count == READER_ITER);
}


TEST_CASE("occ_box apply_to with two boxes", "[occ_box]") {
    occ_box<test::Counter> a(1);
    occ_box<test::Counter> b(2);

    bool ok = apply_to(
            std::tie(a, b),
            std::tuple{
                    [](test::Counter &x) { x.value += 10; },
                    [](test::Counter &y) { y.value += 20; }
            }
    );

    REQUIRE(ok);
    REQUIRE(a.read([](const test::Counter &c) { return c.value; }) == 11);
    REQUIRE(b.read([](const test::Counter &c) { return c.value; }) == 22);
}

TEST_CASE("occ_box apply_to_ptr with two boxes", "[occ_box][ptr]") {
    occ_box<test::Foo> a(std::make_shared<test::Foo>(test::Foo{1, "Alice"}));
    occ_box<test::Foo> b(std::make_shared<test::Foo>(test::Foo{2, "Bob"}));

    bool ok = apply_to(
            std::tie(a, b),
            std::make_tuple(
                    [](const std::shared_ptr<test::Foo> &old) {
                        return std::make_shared<test::Foo>(test::Foo{old->x + 10, old->name + "-updated"});
                    },
                    [](const std::shared_ptr<test::Foo> &old) {
                        return std::make_shared<test::Foo>(test::Foo{old->x + 20, old->name + "-updated"});
                    }
            )
    );

    REQUIRE(ok);

    auto resA = a.read([](const test::Foo &f) { return f; });
    auto resB = b.read([](const test::Foo &f) { return f; });

    REQUIRE(resA.x == 11);
    REQUIRE(resA.name == "Alice-updated");
    REQUIRE(resB.x == 22);
    REQUIRE(resB.name == "Bob-updated");
}

TEST_CASE("occ_box apply_to with lambda captures", "[occ_box][capture]") {
    occ_box<test::Counter> a(5);
    occ_box<test::Counter> b(10);

    int add_a = 7;
    int add_b = 15;

    bool ok = apply_to(
            std::tie(a, b),
            std::tuple{
                    [=](test::Counter &x) { x.value += add_a; },   // capture add_a
                    [=](test::Counter &y) { y.value += add_b; }    // capture add_b
            }
    );

    REQUIRE(ok);
    REQUIRE(a.read([](const test::Counter &c) { return c.value; }) == 5 + add_a);
    REQUIRE(b.read([](const test::Counter &c) { return c.value; }) == 10 + add_b);
}
