#define TINY_TEST_MAIN
#include "jh/test/tiny_test/tiny_test.hpp"

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

#include "jh/concurrency"
#include "jh/macros/platform.h"
#include <thread>
#include <chrono>    // NOLINT force include for std::chrono_literals
#include <vector>
#include <atomic>
#include <string>

using namespace jh::conc;

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

namespace test {
void tiny_test_case_1() {
    auto sp = std::make_shared<test::Counter>(123);
    occ_box<test::Counter> box(sp);

    jh::test::tiny_test::expect(static_cast<bool>((box.read([](const test::Counter &c) { return c.value; }) == 123)), "box.read([](const test::Counter &c) { return c.value; }) == 123");

}
}
template<>
struct jh::test::tiny_test::test<"occ_box construct from shared_ptr">
    : jh::test::tiny_test::test_definition<"occ_box construct from shared_ptr", &::test::tiny_test_case_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_occ case 1">
    : jh::test::tiny_test::session_definition<
          "test module test_occ case 1", jh::test::tiny_test::test<"occ_box construct from shared_ptr">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_occ case 1"> registration_1{};
}



namespace test {
void tiny_test_case_2() {
    occ_box<test::Counter> box(0);

    box.write([](test::Counter &c) { c.value = 42; });
    jh::test::tiny_test::expect(static_cast<bool>((box.read([](const test::Counter &c) { return c.value; }) == 42)), "box.read([](const test::Counter &c) { return c.value; }) == 42");

    auto r = box.try_read([](const test::Counter &c) { return c.value; });
    jh::test::tiny_test::expect(static_cast<bool>((r.has_value())), "r.has_value()");
    jh::test::tiny_test::expect(static_cast<bool>((r.value() == 42)), "r.value() == 42");

    bool ok = box.try_write([](test::Counter &c) { c.value = 77; });
    jh::test::tiny_test::expect(static_cast<bool>((ok)), "ok");
    jh::test::tiny_test::expect(static_cast<bool>((box.read([](const test::Counter &c) { return c.value; }) == 77)), "box.read([](const test::Counter &c) { return c.value; }) == 77");

    box.write_ptr([](const std::shared_ptr<test::Counter> &old) {
        return std::make_shared<test::Counter>(old->value + 1);
    });
    jh::test::tiny_test::expect(static_cast<bool>((box.read([](const test::Counter &c) { return c.value; }) == 78)), "box.read([](const test::Counter &c) { return c.value; }) == 78");

    ok = box.try_write_ptr([](const std::shared_ptr<test::Counter> &old) {
        return std::make_shared<test::Counter>(old->value + 10);
    });
    jh::test::tiny_test::expect(static_cast<bool>((ok)), "ok");
    jh::test::tiny_test::expect(static_cast<bool>((box.read([](const test::Counter &c) { return c.value; }) == 88)), "box.read([](const test::Counter &c) { return c.value; }) == 88");

}
}
template<>
struct jh::test::tiny_test::test<"occ_box basic read/write">
    : jh::test::tiny_test::test_definition<"occ_box basic read/write", &::test::tiny_test_case_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_occ case 2">
    : jh::test::tiny_test::session_definition<
          "test module test_occ case 2", jh::test::tiny_test::test<"occ_box basic read/write">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_occ case 2"> registration_2{};
}



namespace test {
void tiny_test_case_3() {
    occ_box<test::Counter> box(0);
    auto v1 = box.get_version();
    box.write([](test::Counter &c) { c.value = 1; });
    auto v2 = box.get_version();
    jh::test::tiny_test::expect(static_cast<bool>((v2 > v1)), "v2 > v1");

}
}
template<>
struct jh::test::tiny_test::test<"occ_box get_version increases">
    : jh::test::tiny_test::test_definition<"occ_box get_version increases", &::test::tiny_test_case_3> {};
template<>
struct jh::test::tiny_test::session<"test module test_occ case 3">
    : jh::test::tiny_test::session_definition<
          "test module test_occ case 3", jh::test::tiny_test::test<"occ_box get_version increases">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_occ case 3"> registration_3{};
}






namespace test {
void tiny_test_case_4() {
    occ_box<test::Counter> a(1);
    occ_box<test::Counter> b(2);

    bool ok = apply_to(
            std::tie(a, b),
            std::tuple{
                    [](test::Counter &x) { x.value += 10; },
                    [](test::Counter &y) { y.value += 20; }
            }
    );

    jh::test::tiny_test::expect(static_cast<bool>((ok)), "ok");
    jh::test::tiny_test::expect(static_cast<bool>((a.read([](const test::Counter &c) { return c.value; }) == 11)), "a.read([](const test::Counter &c) { return c.value; }) == 11");
    jh::test::tiny_test::expect(static_cast<bool>((b.read([](const test::Counter &c) { return c.value; }) == 22)), "b.read([](const test::Counter &c) { return c.value; }) == 22");

}
}
template<>
struct jh::test::tiny_test::test<"occ_box apply_to with two boxes">
    : jh::test::tiny_test::test_definition<"occ_box apply_to with two boxes", &::test::tiny_test_case_4> {};
template<>
struct jh::test::tiny_test::session<"test module test_occ case 4">
    : jh::test::tiny_test::session_definition<
          "test module test_occ case 4", jh::test::tiny_test::test<"occ_box apply_to with two boxes">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_occ case 4"> registration_4{};
}



namespace test {
void tiny_test_case_5() {
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

    jh::test::tiny_test::expect(static_cast<bool>((ok)), "ok");

    auto resA = a.read([](const test::Foo &f) { return f; });
    auto resB = b.read([](const test::Foo &f) { return f; });

    jh::test::tiny_test::expect(static_cast<bool>((resA.x == 11)), "resA.x == 11");
    jh::test::tiny_test::expect(static_cast<bool>((resA.name == "Alice-updated")), "resA.name == \"Alice-updated\"");
    jh::test::tiny_test::expect(static_cast<bool>((resB.x == 22)), "resB.x == 22");
    jh::test::tiny_test::expect(static_cast<bool>((resB.name == "Bob-updated")), "resB.name == \"Bob-updated\"");

}
}
template<>
struct jh::test::tiny_test::test<"occ_box apply_to_ptr with two boxes">
    : jh::test::tiny_test::test_definition<"occ_box apply_to_ptr with two boxes", &::test::tiny_test_case_5> {};
template<>
struct jh::test::tiny_test::session<"test module test_occ case 5">
    : jh::test::tiny_test::session_definition<
          "test module test_occ case 5", jh::test::tiny_test::test<"occ_box apply_to_ptr with two boxes">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_occ case 5"> registration_5{};
}



namespace test {
void tiny_test_case_6() {
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

    jh::test::tiny_test::expect(static_cast<bool>((ok)), "ok");
    jh::test::tiny_test::expect(static_cast<bool>((a.read([](const test::Counter &c) { return c.value; }) == 5 + add_a)), "a.read([](const test::Counter &c) { return c.value; }) == 5 + add_a");
    jh::test::tiny_test::expect(static_cast<bool>((b.read([](const test::Counter &c) { return c.value; }) == 10 + add_b)), "b.read([](const test::Counter &c) { return c.value; }) == 10 + add_b");

}
}
template<>
struct jh::test::tiny_test::test<"occ_box apply_to with lambda captures">
    : jh::test::tiny_test::test_definition<"occ_box apply_to with lambda captures", &::test::tiny_test_case_6> {};
template<>
struct jh::test::tiny_test::session<"test module test_occ case 6">
    : jh::test::tiny_test::session_definition<
          "test module test_occ case 6", jh::test::tiny_test::test<"occ_box apply_to with lambda captures">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_occ case 6"> registration_6{};
}

