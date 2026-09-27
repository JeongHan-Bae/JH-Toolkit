#define TINY_TEST_MAIN
#include "jh/test/tiny_test/tiny_test.hpp"

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

#include <array>
#include <string>
#include <vector>
#include <ranges>
#include <numeric>

#include "jh/pod"
#include "jh/conceptual/sequence.h"

namespace pod = jh::pod;



namespace test {
void tiny_test_case_1() {
    pod::array<int, 4> a = {{1, 2, 3, 4}};
    jh::test::tiny_test::expect(static_cast<bool>((a.size() == 4)), "a.size() == 4");
    jh::test::tiny_test::expect(static_cast<bool>((a[0] == 1)), "a[0] == 1");
    jh::test::tiny_test::expect(static_cast<bool>((a[3] == 4)), "a[3] == 4");

    a[2] = 42;
    jh::test::tiny_test::expect(static_cast<bool>((a[2] == 42)), "a[2] == 42");

}
}
template<>
struct jh::test::tiny_test::test<"pod::array basic construction and access">
    : jh::test::tiny_test::test_definition<"pod::array basic construction and access", &::test::tiny_test_case_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 1">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 1", jh::test::tiny_test::test<"pod::array basic construction and access">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 1"> registration_1{};
}



namespace test {
void tiny_test_case_2() {
    pod::array<int, 3> mutable_values{{1, 2, 3}};
    auto item = mutable_values.at(1);
    jh::test::tiny_test::expect(static_cast<bool>((item)), "item");
    *item.value() = 20;
    jh::test::tiny_test::expect(static_cast<bool>((mutable_values[1] == 20)), "mutable_values[1] == 20");
    jh::test::tiny_test::expect_not(static_cast<bool>((mutable_values.at(3))), "mutable_values.at(3)");

}
}
template<>
struct jh::test::tiny_test::test<"pod::array checked at access">
    : jh::test::tiny_test::test_definition<"pod::array checked at access", &::test::tiny_test_case_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 2">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 2", jh::test::tiny_test::test<"pod::array checked at access">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 2"> registration_2{};
}



namespace test {
void tiny_test_case_3() {
    enum class result_error : std::uint8_t { failed };
    using result_type = jh::meta::expected<int, result_error>;

    result_type result{12};
    jh::test::tiny_test::expect(static_cast<bool>((result.has_value())), "result.has_value()");
    jh::test::tiny_test::expect(static_cast<bool>((result.value() == 12)), "result.value() == 12");
    jh::test::tiny_test::expect(static_cast<bool>((*result == 12)), "*result == 12");
    struct box { int value; };
    jh::meta::expected<box, result_error> boxed{box{24}};
    jh::test::tiny_test::expect(static_cast<bool>((boxed->value == 24)), "boxed->value == 24");
    jh::test::tiny_test::expect(static_cast<bool>(((*boxed).value == 24)), "(*boxed).value == 24");

    result = jh::meta::unexpected(result_error::failed);
    jh::test::tiny_test::expect(static_cast<bool>((result.has_error())), "result.has_error()");
    jh::test::tiny_test::expect(static_cast<bool>((result.error() == result_error::failed)), "result.error() == result_error::failed");
    jh::test::tiny_test::expect(static_cast<bool>((result.error_or(result_error::failed) == result_error::failed)), "result.error_or(result_error::failed) == result_error::failed");
    jh::test::tiny_test::expect(static_cast<bool>((result.value_or(5) == 5)), "result.value_or(5) == 5");

    result = 41;
    jh::test::tiny_test::expect(static_cast<bool>((result)), "result");
    jh::test::tiny_test::expect(static_cast<bool>((result.value() == 41)), "result.value() == 41");

}
}
template<>
struct jh::test::tiny_test::test<"jh::meta::expected value and error assignment">
    : jh::test::tiny_test::test_definition<"jh::meta::expected value and error assignment", &::test::tiny_test_case_3> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 3">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 3", jh::test::tiny_test::test<"jh::meta::expected value and error assignment">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 3"> registration_3{};
}



namespace test {
void tiny_test_case_4() {
    pod::array<char, 3> chars = {{'a', 'b', 'c'}};

    std::string s;
    for (const char ch: chars) {
        s += ch;
    }

    jh::test::tiny_test::expect(static_cast<bool>((s == "abc")), "s == \"abc\"");

}
}
template<>
struct jh::test::tiny_test::test<"pod::array supports range-based iteration">
    : jh::test::tiny_test::test_definition<"pod::array supports range-based iteration", &::test::tiny_test_case_4> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 4">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 4", jh::test::tiny_test::test<"pod::array supports range-based iteration">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 4"> registration_4{};
}



namespace test {
void tiny_test_case_5() {
    pod::array<int, 3> a = {{1, 2, 3}};
    pod::array<int, 3> b = {{1, 2, 3}};
    pod::array<int, 3> c = {{1, 2, 4}};

    jh::test::tiny_test::expect(static_cast<bool>((a == b)), "a == b");
    jh::test::tiny_test::expect_not(static_cast<bool>((a == c)), "a == c");

}
}
template<>
struct jh::test::tiny_test::test<"pod::array equality comparison works">
    : jh::test::tiny_test::test_definition<"pod::array equality comparison works", &::test::tiny_test_case_5> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 5">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 5", jh::test::tiny_test::test<"pod::array equality comparison works">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 5"> registration_5{};
}



namespace test {
void tiny_test_case_6() {
    using F = jh::pod::bitflags<32>;

    F f{};
    jh::test::tiny_test::expect(static_cast<bool>((f.count() == 0)), "f.count() == 0");

    f.set(0);
    jh::test::tiny_test::expect(static_cast<bool>((f.has(0))), "f.has(0)");
    jh::test::tiny_test::expect(static_cast<bool>((f.count() == 1)), "f.count() == 1");

    f.set(31);
    jh::test::tiny_test::expect(static_cast<bool>((f.has(31))), "f.has(31)");
    jh::test::tiny_test::expect(static_cast<bool>((f.count() == 2)), "f.count() == 2");

    f.clear(0);
    jh::test::tiny_test::expect_not(static_cast<bool>((f.has(0))), "f.has(0)");
    jh::test::tiny_test::expect(static_cast<bool>((f.count() == 1)), "f.count() == 1");

    f.flip(1);
    jh::test::tiny_test::expect(static_cast<bool>((f.has(1))), "f.has(1)");

    f.flip(1);
    jh::test::tiny_test::expect_not(static_cast<bool>((f.has(1))), "f.has(1)");

    f.set_all();
    jh::test::tiny_test::expect(static_cast<bool>((f.count() == 32)), "f.count() == 32");

    f.reset_all();
    jh::test::tiny_test::expect(static_cast<bool>((f.count() == 0)), "f.count() == 0");

    f.flip_all();         // invert all bits
    jh::test::tiny_test::expect(static_cast<bool>((f.count() == 32)), "f.count() == 32");

    f.flip_all();         // invert back
    jh::test::tiny_test::expect(static_cast<bool>((f.count() == 0)), "f.count() == 0");

}
}
template<>
struct jh::test::tiny_test::test<"bitflags basic API (native uint backend)">
    : jh::test::tiny_test::test_definition<"bitflags basic API (native uint backend)", &::test::tiny_test_case_6> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 6">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 6", jh::test::tiny_test::test<"bitflags basic API (native uint backend)">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 6"> registration_6{};
}



namespace test {
void tiny_test_case_7() {
    using F = jh::pod::bitflags<24>; // non-native, uses byte-array backend
    F f{};
    jh::test::tiny_test::expect(static_cast<bool>((f.count() == 0)), "f.count() == 0");

    f.set(0);
    f.set(23);
    jh::test::tiny_test::expect(static_cast<bool>((f.has(0))), "f.has(0)");
    jh::test::tiny_test::expect(static_cast<bool>((f.has(23))), "f.has(23)");

    f.flip(0);
    jh::test::tiny_test::expect_not(static_cast<bool>((f.has(0))), "f.has(0)");

    f.set_all();
    jh::test::tiny_test::expect(static_cast<bool>((f.count() == 24)), "f.count() == 24");

    f.reset_all();
    jh::test::tiny_test::expect(static_cast<bool>((f.count() == 0)), "f.count() == 0");

    f.flip_all();
    jh::test::tiny_test::expect(static_cast<bool>((f.count() == 24)), "f.count() == 24");

}
}
template<>
struct jh::test::tiny_test::test<"bitflags full API (bytes backend)">
    : jh::test::tiny_test::test_definition<"bitflags full API (bytes backend)", &::test::tiny_test_case_7> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 7">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 7", jh::test::tiny_test::test<"bitflags full API (bytes backend)">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 7"> registration_7{};
}



namespace test {
void tiny_test_case_8_body(const int selected_section) {
    if (selected_section == 1) {
        std::array<std::uint8_t, 4> a = {1, 2, 3, 4};
        std::array<std::uint8_t, 4> b = {1, 2, 3, 4};
        std::array<std::uint8_t, 4> c = {4, 3, 2, 1};

        auto va = pod::bytes_view::from(a.data(), a.size());
        auto vb = pod::bytes_view::from(b.data(), b.size());
        auto vc = pod::bytes_view::from(c.data(), c.size());

        jh::test::tiny_test::expect(static_cast<bool>((va == vb)), "va == vb");
        jh::test::tiny_test::expect_not(static_cast<bool>((va == vc)), "va == vc");
        jh::test::tiny_test::expect(static_cast<bool>((va != vc)), "va != vc");
    }

    if (selected_section == 2) {
        struct TestStruct {
            std::uint16_t a;
            std::uint16_t b;
        };
        jh::test::tiny_test::expect(static_cast<bool>((jh::pod::trivial_bytes<TestStruct>)), "jh::pod::trivial_bytes<TestStruct>");

        TestStruct original{0x1234, 0xABCD};
        auto view = pod::bytes_view::from(original);
        auto [a, b] = view.at<TestStruct>();
        jh::test::tiny_test::expect(static_cast<bool>((a == 0x1234)), "a == 0x1234");
        jh::test::tiny_test::expect(static_cast<bool>((b == 0xABCD)), "b == 0xABCD");
    }

    if (selected_section == 3) {
        pod::array<int, 3> arr = {10, 20, 30};
        auto view = pod::bytes_view::from(arr.data, pod::array<int, 3>::size());

        auto result = view.clone<pod::array<int, 3> >(); // NOLINT
        jh::test::tiny_test::expect(static_cast<bool>((result)), "result");
        auto clone = result.value();
        jh::test::tiny_test::expect(static_cast<bool>((clone[0] == 10)), "clone[0] == 10");
        jh::test::tiny_test::expect(static_cast<bool>((clone[1] == 20)), "clone[1] == 20");
        jh::test::tiny_test::expect(static_cast<bool>((clone[2] == 30)), "clone[2] == 30");
    }

    if (selected_section == 4) {
        std::uint32_t x = 0xAABBCCDD;
        auto view = pod::bytes_view::from(x);

        const auto ok = view.fetch<std::uint32_t>();
        const auto bad = view.fetch<std::uint32_t>(4); // too far

        jh::test::tiny_test::expect(static_cast<bool>((ok)), "ok");
        jh::test::tiny_test::expect(static_cast<bool>((ok.value() != nullptr)), "ok.value() != nullptr");
        jh::test::tiny_test::expect_not(static_cast<bool>((bad)), "bad");
        jh::test::tiny_test::expect(static_cast<bool>((bad.error() == pod::bytes_view::error_code::out_of_bounds)), "bad.error() == pod::bytes_view::error_code::out_of_bounds");
    }

    if (selected_section == 5) {
        struct PodTest {
            int a;
            float b;
        };
        jh::test::tiny_test::expect(static_cast<bool>((jh::pod::pod_like<PodTest>)), "jh::pod::pod_like<PodTest>");

        std::array<std::byte, 2> too_small{};
        auto view = pod::bytes_view{too_small.data(), too_small.size()};
        const auto clone = view.clone<PodTest>(); // NOLINT
        jh::test::tiny_test::expect_not(static_cast<bool>((clone)), "clone");
        jh::test::tiny_test::expect(static_cast<bool>((clone.error() == pod::bytes_view::error_code::size_mismatch)), "clone.error() == pod::bytes_view::error_code::size_mismatch");
    }

}
void tiny_test_case_8_section_1() { tiny_test_case_8_body(1); }
void tiny_test_case_8_section_2() { tiny_test_case_8_body(2); }
void tiny_test_case_8_section_3() { tiny_test_case_8_body(3); }
void tiny_test_case_8_section_4() { tiny_test_case_8_body(4); }
void tiny_test_case_8_section_5() { tiny_test_case_8_body(5); }
}
template<>
struct jh::test::tiny_test::test<"bytes_view basic reinterpret and comparison / from std::array and compare views">
    : jh::test::tiny_test::test_definition<"bytes_view basic reinterpret and comparison / from std::array and compare views", &::test::tiny_test_case_8_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 8">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 8", jh::test::tiny_test::test<"bytes_view basic reinterpret and comparison / from std::array and compare views">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 8"> registration_8{};
}
template<>
struct jh::test::tiny_test::test<"bytes_view basic reinterpret and comparison / reinterpret as struct using at<T>()">
    : jh::test::tiny_test::test_definition<"bytes_view basic reinterpret and comparison / reinterpret as struct using at<T>()", &::test::tiny_test_case_8_section_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 9">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 9", jh::test::tiny_test::test<"bytes_view basic reinterpret and comparison / reinterpret as struct using at<T>()">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 9"> registration_9{};
}
template<>
struct jh::test::tiny_test::test<"bytes_view basic reinterpret and comparison / clone to pod::array<int, N>">
    : jh::test::tiny_test::test_definition<"bytes_view basic reinterpret and comparison / clone to pod::array<int, N>", &::test::tiny_test_case_8_section_3> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 10">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 10", jh::test::tiny_test::test<"bytes_view basic reinterpret and comparison / clone to pod::array<int, N>">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 10"> registration_10{};
}
template<>
struct jh::test::tiny_test::test<"bytes_view basic reinterpret and comparison / fetch reports out-of-bounds access">
    : jh::test::tiny_test::test_definition<"bytes_view basic reinterpret and comparison / fetch reports out-of-bounds access", &::test::tiny_test_case_8_section_4> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 11">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 11", jh::test::tiny_test::test<"bytes_view basic reinterpret and comparison / fetch reports out-of-bounds access">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 11"> registration_11{};
}
template<>
struct jh::test::tiny_test::test<"bytes_view basic reinterpret and comparison / clone reports a length mismatch">
    : jh::test::tiny_test::test_definition<"bytes_view basic reinterpret and comparison / clone reports a length mismatch", &::test::tiny_test_case_8_section_5> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 12">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 12", jh::test::tiny_test::test<"bytes_view basic reinterpret and comparison / clone reports a length mismatch">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 12"> registration_12{};
}



namespace test {
void tiny_test_case_9() {
    constexpr std::size_t N = 64;
    std::array<std::uint32_t, N> original{};
    std::iota(original.begin(), original.end(), 100); // Fill with 100, 101, ..., 163

    const auto view = pod::bytes_view::from(original.data(), original.size());
    auto clone_result = view.clone<pod::array<std::uint32_t, N> >(); // NOLINT
    jh::test::tiny_test::expect(static_cast<bool>((clone_result)), "clone_result");
    auto cloned = clone_result.value();

    jh::test::tiny_test::expect(static_cast<bool>((cloned.size() == N)), "cloned.size() == N");
    for (std::size_t i = 0; i < N; ++i) {
        jh::test::tiny_test::expect(static_cast<bool>((cloned[i] == original[i])), "cloned[i] == original[i]");
    }

}
}
template<>
struct jh::test::tiny_test::test<"bytes_view clone from std::array to pod::array">
    : jh::test::tiny_test::test_definition<"bytes_view clone from std::array to pod::array", &::test::tiny_test_case_9> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 13">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 13", jh::test::tiny_test::test<"bytes_view clone from std::array to pod::array">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 13"> registration_13{};
}



namespace test {
void tiny_test_case_10_body(const int selected_section) {
    using pod::optional;
    using pod::make_optional;

    if (selected_section == 1) {
        optional<int> o{};
        jh::test::tiny_test::expect(static_cast<bool>((o.empty())), "o.empty()");
        jh::test::tiny_test::expect_not(static_cast<bool>((o.has())), "o.has()");
    }

    if (selected_section == 2) {
        optional<int> o{};
        o.store(42);
        jh::test::tiny_test::expect(static_cast<bool>((o.has())), "o.has()");
        jh::test::tiny_test::expect_not(static_cast<bool>((o.empty())), "o.empty()");
        jh::test::tiny_test::expect(static_cast<bool>((o.ref() == 42)), "o.ref() == 42");
        jh::test::tiny_test::expect(static_cast<bool>((*o.get() == 42)), "*o.get() == 42");
    }

    if (selected_section == 3) {
        optional<int> o{};
        o.store(99);
        jh::test::tiny_test::expect(static_cast<bool>((o.has())), "o.has()");
        o.clear();
        jh::test::tiny_test::expect_not(static_cast<bool>((o.has())), "o.has()");
        jh::test::tiny_test::expect(static_cast<bool>((o.empty())), "o.empty()");
    }

    if (selected_section == 4) {
        auto o = make_optional(1234);
        jh::test::tiny_test::expect(static_cast<bool>((o.has())), "o.has()");
        jh::test::tiny_test::expect(static_cast<bool>((o.ref() == 1234)), "o.ref() == 1234");
    }

    if (selected_section == 5) {
        struct Sample {
            int a;
            float b;
        };
        jh::test::tiny_test::expect(static_cast<bool>((pod::pod_like<Sample>)), "pod::pod_like<Sample>");

        Sample s{10, 3.5f};
        auto o = make_optional(s);
        jh::test::tiny_test::expect(static_cast<bool>((o.has())), "o.has()");
        jh::test::tiny_test::expect(static_cast<bool>((o.ref().a == 10)), "o.ref().a == 10");
        jh::test::tiny_test::expect(static_cast<bool>((o.ref().b == 3.5f)), "o.ref().b == 3.5f");
    }

}
void tiny_test_case_10_section_1() { tiny_test_case_10_body(1); }
void tiny_test_case_10_section_2() { tiny_test_case_10_body(2); }
void tiny_test_case_10_section_3() { tiny_test_case_10_body(3); }
void tiny_test_case_10_section_4() { tiny_test_case_10_body(4); }
void tiny_test_case_10_section_5() { tiny_test_case_10_body(5); }
}
template<>
struct jh::test::tiny_test::test<"pod::optional basic behavior / Default constructed is empty">
    : jh::test::tiny_test::test_definition<"pod::optional basic behavior / Default constructed is empty", &::test::tiny_test_case_10_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 14">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 14", jh::test::tiny_test::test<"pod::optional basic behavior / Default constructed is empty">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 14"> registration_14{};
}
template<>
struct jh::test::tiny_test::test<"pod::optional basic behavior / store sets value and has() returns true">
    : jh::test::tiny_test::test_definition<"pod::optional basic behavior / store sets value and has() returns true", &::test::tiny_test_case_10_section_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 15">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 15", jh::test::tiny_test::test<"pod::optional basic behavior / store sets value and has() returns true">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 15"> registration_15{};
}
template<>
struct jh::test::tiny_test::test<"pod::optional basic behavior / clear resets the optional">
    : jh::test::tiny_test::test_definition<"pod::optional basic behavior / clear resets the optional", &::test::tiny_test_case_10_section_3> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 16">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 16", jh::test::tiny_test::test<"pod::optional basic behavior / clear resets the optional">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 16"> registration_16{};
}
template<>
struct jh::test::tiny_test::test<"pod::optional basic behavior / make_optional returns filled optional">
    : jh::test::tiny_test::test_definition<"pod::optional basic behavior / make_optional returns filled optional", &::test::tiny_test_case_10_section_4> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 17">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 17", jh::test::tiny_test::test<"pod::optional basic behavior / make_optional returns filled optional">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 17"> registration_17{};
}
template<>
struct jh::test::tiny_test::test<"pod::optional basic behavior / copy from existing pod type">
    : jh::test::tiny_test::test_definition<"pod::optional basic behavior / copy from existing pod type", &::test::tiny_test_case_10_section_5> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 18">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 18", jh::test::tiny_test::test<"pod::optional basic behavior / copy from existing pod type">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 18"> registration_18{};
}



namespace test {
void tiny_test_case_11_body(const int selected_section) {
    using pod::optional;

    if (selected_section == 1) {
        optional<int> o{};
        jh::test::tiny_test::expect_not(static_cast<bool>((o.has())), "o.has()");
        jh::test::tiny_test::expect(static_cast<bool>((o.value_or(99) == 99)), "o.value_or(99) == 99");
    }

    if (selected_section == 2) {
        optional<int> o{};
        o.store(123);
        jh::test::tiny_test::expect(static_cast<bool>((o.has())), "o.has()");
        jh::test::tiny_test::expect(static_cast<bool>((o.value_or(999) == 123)), "o.value_or(999) == 123"); // fallback ignored
    }

    if (selected_section == 3) {
        struct S {
            int x;
            float y;
        };
        optional<S> o{};
        S def{5, 3.5f};
        jh::test::tiny_test::expect(static_cast<bool>((o.value_or(def).x == 5)), "o.value_or(def).x == 5");
        jh::test::tiny_test::expect(static_cast<bool>((o.value_or(def).y == 3.5f)), "o.value_or(def).y == 3.5f");

        o.store({42, 1.0f});
        jh::test::tiny_test::expect(static_cast<bool>((o.value_or(def).x == 42)), "o.value_or(def).x == 42");
        jh::test::tiny_test::expect(static_cast<bool>((o.value_or(def).y == 1.0f)), "o.value_or(def).y == 1.0f");
    }

}
void tiny_test_case_11_section_1() { tiny_test_case_11_body(1); }
void tiny_test_case_11_section_2() { tiny_test_case_11_body(2); }
void tiny_test_case_11_section_3() { tiny_test_case_11_body(3); }
}
template<>
struct jh::test::tiny_test::test<"pod::optional value_or behavior / Returns fallback when empty">
    : jh::test::tiny_test::test_definition<"pod::optional value_or behavior / Returns fallback when empty", &::test::tiny_test_case_11_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 19">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 19", jh::test::tiny_test::test<"pod::optional value_or behavior / Returns fallback when empty">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 19"> registration_19{};
}
template<>
struct jh::test::tiny_test::test<"pod::optional value_or behavior / Returns stored value when present">
    : jh::test::tiny_test::test_definition<"pod::optional value_or behavior / Returns stored value when present", &::test::tiny_test_case_11_section_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 20">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 20", jh::test::tiny_test::test<"pod::optional value_or behavior / Returns stored value when present">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 20"> registration_20{};
}
template<>
struct jh::test::tiny_test::test<"pod::optional value_or behavior / Works with trivial struct">
    : jh::test::tiny_test::test_definition<"pod::optional value_or behavior / Works with trivial struct", &::test::tiny_test_case_11_section_3> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 21">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 21", jh::test::tiny_test::test<"pod::optional value_or behavior / Works with trivial struct">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 21"> registration_21{};
}



namespace test {
void tiny_test_case_12_body(const int selected_section) {
    using pod::array;
    using pod::optional;
    using pod::make_optional;

    constexpr std::uint16_t N = 8;
    array<optional<int>, N> opt_arr{};

    if (selected_section == 1) {
        for (std::uint16_t i = 0; i < N; ++i) {
            jh::test::tiny_test::expect(static_cast<bool>((opt_arr[i].empty())), "opt_arr[i].empty()");
        }
    }

    if (selected_section == 2) {
        for (std::uint16_t i = 0; i < N; i += 2) {
            opt_arr[i].store(i * 10);
        }

        for (std::uint16_t i = 0; i < N; ++i) {
            if (i % 2 == 0) {
                jh::test::tiny_test::expect(static_cast<bool>((opt_arr[i].has())), "opt_arr[i].has()");
                jh::test::tiny_test::expect(static_cast<bool>((opt_arr[i].ref() == static_cast<int>(i * 10))), "opt_arr[i].ref() == static_cast<int>(i * 10)");
            } else {
                jh::test::tiny_test::expect(static_cast<bool>((opt_arr[i].empty())), "opt_arr[i].empty()");
            }
        }
    }

    if (selected_section == 3) {
        for (std::uint16_t i = 0; i < N; ++i) {
            opt_arr[i].store(i);
        }

        opt_arr[3].clear();
        opt_arr[5].clear();

        for (std::uint16_t i = 0; i < N; ++i) {
            if (i == 3 || i == 5) {
                jh::test::tiny_test::expect_not(static_cast<bool>((opt_arr[i].has())), "opt_arr[i].has()");
            } else {
                jh::test::tiny_test::expect(static_cast<bool>((opt_arr[i].has())), "opt_arr[i].has()");
                jh::test::tiny_test::expect(static_cast<bool>((opt_arr[i].ref() == static_cast<int>(i))), "opt_arr[i].ref() == static_cast<int>(i)");
            }
        }
    }

    if (selected_section == 4) {
        for (std::uint16_t i = 0; i < N; ++i)
            opt_arr[i] = make_optional(i * i);

        int sum = 0;
        for (const auto &o: opt_arr) {
            jh::test::tiny_test::expect(static_cast<bool>((o.has())), "o.has()");
            sum += o.ref();
        }

        int expected = 0;
        for (std::uint16_t i = 0; i < N; ++i)
            expected += i * i;

        jh::test::tiny_test::expect(static_cast<bool>((sum == expected)), "sum == expected");
    }

}
void tiny_test_case_12_section_1() { tiny_test_case_12_body(1); }
void tiny_test_case_12_section_2() { tiny_test_case_12_body(2); }
void tiny_test_case_12_section_3() { tiny_test_case_12_body(3); }
void tiny_test_case_12_section_4() { tiny_test_case_12_body(4); }
}
template<>
struct jh::test::tiny_test::test<"pod::array of optional<T> usage / Initially all optionals are empty">
    : jh::test::tiny_test::test_definition<"pod::array of optional<T> usage / Initially all optionals are empty", &::test::tiny_test_case_12_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 22">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 22", jh::test::tiny_test::test<"pod::array of optional<T> usage / Initially all optionals are empty">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 22"> registration_22{};
}
template<>
struct jh::test::tiny_test::test<"pod::array of optional<T> usage / Storing values into some elements">
    : jh::test::tiny_test::test_definition<"pod::array of optional<T> usage / Storing values into some elements", &::test::tiny_test_case_12_section_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 23">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 23", jh::test::tiny_test::test<"pod::array of optional<T> usage / Storing values into some elements">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 23"> registration_23{};
}
template<>
struct jh::test::tiny_test::test<"pod::array of optional<T> usage / Clear values selectively">
    : jh::test::tiny_test::test_definition<"pod::array of optional<T> usage / Clear values selectively", &::test::tiny_test_case_12_section_3> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 24">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 24", jh::test::tiny_test::test<"pod::array of optional<T> usage / Clear values selectively">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 24"> registration_24{};
}
template<>
struct jh::test::tiny_test::test<"pod::array of optional<T> usage / Use with algorithm-like access">
    : jh::test::tiny_test::test_definition<"pod::array of optional<T> usage / Use with algorithm-like access", &::test::tiny_test_case_12_section_4> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 25">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 25", jh::test::tiny_test::test<"pod::array of optional<T> usage / Use with algorithm-like access">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 25"> registration_25{};
}



namespace test {
void tiny_test_case_13_body(const int selected_section) {
    using pod::optional;
    using pod::make_optional;

    if (selected_section == 1) {
        optional<int> def{};              // Default constructed: has_value = false, storage is 0-initialized
        auto val0 = make_optional(0);     // Stored value 0: has_value = true, storage contains all zeroes

        jh::test::tiny_test::expect(static_cast<bool>((def != val0)), "def != val0");             // Different states: one empty, one holding a value
        val0.clear();                     // Clear: has_value = false
        jh::test::tiny_test::expect(static_cast<bool>((def == val0)), "def == val0");             // Both empty, considered equal regardless of storage content
    }

    if (selected_section == 2) {
        auto a = make_optional(234);
        auto b = make_optional(16);
        jh::test::tiny_test::expect(static_cast<bool>((a != b)), "a != b");                  // Both have values, raw storage differs
    }

    if (selected_section == 3) {
        auto a = make_optional(16);
        auto b = make_optional(16);
        jh::test::tiny_test::expect(static_cast<bool>((a == b)), "a == b");                  // Both have values, raw storage identical
    }

    if (selected_section == 4) {
        auto a = make_optional(234);
        auto b = make_optional(16);
        jh::test::tiny_test::expect(static_cast<bool>((a != b)), "a != b");                  // Initially different values

        a.clear();
        b.clear();
        jh::test::tiny_test::expect(static_cast<bool>((a == b)), "a == b");                  // After clear: both empty, considered equal
    }

}
void tiny_test_case_13_section_1() { tiny_test_case_13_body(1); }
void tiny_test_case_13_section_2() { tiny_test_case_13_body(2); }
void tiny_test_case_13_section_3() { tiny_test_case_13_body(3); }
void tiny_test_case_13_section_4() { tiny_test_case_13_body(4); }
}
template<>
struct jh::test::tiny_test::test<"pod::optional equality semantics / default vs value-initialized 0">
    : jh::test::tiny_test::test_definition<"pod::optional equality semantics / default vs value-initialized 0", &::test::tiny_test_case_13_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 26">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 26", jh::test::tiny_test::test<"pod::optional equality semantics / default vs value-initialized 0">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 26"> registration_26{};
}
template<>
struct jh::test::tiny_test::test<"pod::optional equality semantics / different stored values are not equal">
    : jh::test::tiny_test::test_definition<"pod::optional equality semantics / different stored values are not equal", &::test::tiny_test_case_13_section_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 27">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 27", jh::test::tiny_test::test<"pod::optional equality semantics / different stored values are not equal">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 27"> registration_27{};
}
template<>
struct jh::test::tiny_test::test<"pod::optional equality semantics / same stored values are equal">
    : jh::test::tiny_test::test_definition<"pod::optional equality semantics / same stored values are equal", &::test::tiny_test_case_13_section_3> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 28">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 28", jh::test::tiny_test::test<"pod::optional equality semantics / same stored values are equal">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 28"> registration_28{};
}
template<>
struct jh::test::tiny_test::test<"pod::optional equality semantics / clear makes them equal">
    : jh::test::tiny_test::test_definition<"pod::optional equality semantics / clear makes them equal", &::test::tiny_test_case_13_section_4> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 29">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 29", jh::test::tiny_test::test<"pod::optional equality semantics / clear makes them equal">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 29"> registration_29{};
}



namespace test {
void tiny_test_case_14_body(const int selected_section) {
    using pod::array;
    using pod::optional;
    using pod::make_optional;

    if (selected_section == 1) {
        array<optional<int>, 2> arr1{make_optional(16), make_optional(16)};
        array<optional<int>, 2> arr2{};

        arr1[0].clear();   // has_value = false, storage still contains "16"
        arr1[1].clear();
        // arr2[0] is default constructed: has_value = false, storage 0-initialized

        jh::test::tiny_test::expect(static_cast<bool>((arr1 == arr2)), "arr1 == arr2"); // Equality only depends on has_value and raw storage when present.
        // Since both are empty, they compare equal even if storage differs.
    }

}
void tiny_test_case_14_section_1() { tiny_test_case_14_body(1); }
}
template<>
struct jh::test::tiny_test::test<"pod::array of optional equality semantics / cleared optional equals default optional inside array">
    : jh::test::tiny_test::test_definition<"pod::array of optional equality semantics / cleared optional equals default optional inside array", &::test::tiny_test_case_14_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 30">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 30", jh::test::tiny_test::test<"pod::array of optional equality semantics / cleared optional equals default optional inside array">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 30"> registration_30{};
}




namespace test {
void tiny_test_case_15_body(const int selected_section) {
    using pod::span;
    using pod::array;

    constexpr std::uint16_t N = 10;
    array<int, N> arr{};

    for (std::uint16_t i = 0; i < N; ++i)
        arr[i] = i * 2;

    span<int> s = {arr.data, array<int, N>::size()};

    if (selected_section == 1) {
        jh::test::tiny_test::expect(static_cast<bool>((s.size() == N)), "s.size() == N");
        jh::test::tiny_test::expect_not(static_cast<bool>((s.empty())), "s.empty()");
        for (std::uint16_t i = 0; i < N; ++i)
            jh::test::tiny_test::expect(static_cast<bool>((s[i] == arr[i])), "s[i] == arr[i]");
    }

    if (selected_section == 2) {
        int expected = 0;
        for (auto v: s) {
            jh::test::tiny_test::expect(static_cast<bool>((v == expected)), "v == expected");
            expected += 2;
        }
    }

    if (selected_section == 3) {
        auto mid_result = s.sub(3, 4);
        jh::test::tiny_test::expect(static_cast<bool>((mid_result)), "mid_result");
        auto mid = mid_result.value();
        jh::test::tiny_test::expect(static_cast<bool>((mid.size() == 4)), "mid.size() == 4");
        jh::test::tiny_test::expect(static_cast<bool>((mid[0] == arr[3])), "mid[0] == arr[3]");
        jh::test::tiny_test::expect(static_cast<bool>((mid[3] == arr[6])), "mid[3] == arr[6]");

        auto first_result = s.first(5);
        jh::test::tiny_test::expect(static_cast<bool>((first_result)), "first_result");
        auto first = first_result.value();
        jh::test::tiny_test::expect(static_cast<bool>((first.size() == 5)), "first.size() == 5");
        jh::test::tiny_test::expect(static_cast<bool>((first[0] == arr[0])), "first[0] == arr[0]");
        jh::test::tiny_test::expect(static_cast<bool>((first[4] == arr[4])), "first[4] == arr[4]");

        auto last_result = s.last(3);
        jh::test::tiny_test::expect(static_cast<bool>((last_result)), "last_result");
        auto last = last_result.value();
        jh::test::tiny_test::expect(static_cast<bool>((last.size() == 3)), "last.size() == 3");
        jh::test::tiny_test::expect(static_cast<bool>((last[0] == arr[N - 3])), "last[0] == arr[N - 3]");
    }

    if (selected_section == 4) {
        span<int> same = {arr.data, array<int, N>::size()};
        jh::test::tiny_test::expect(static_cast<bool>((s == same)), "s == same");

        span<int> shorty = {arr.data, N - 1};
        jh::test::tiny_test::expect(static_cast<bool>((s != shorty)), "s != shorty");
    }

}
void tiny_test_case_15_section_1() { tiny_test_case_15_body(1); }
void tiny_test_case_15_section_2() { tiny_test_case_15_body(2); }
void tiny_test_case_15_section_3() { tiny_test_case_15_body(3); }
void tiny_test_case_15_section_4() { tiny_test_case_15_body(4); }
}
template<>
struct jh::test::tiny_test::test<"pod::span works with pod::array / Basic span properties">
    : jh::test::tiny_test::test_definition<"pod::span works with pod::array / Basic span properties", &::test::tiny_test_case_15_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 31">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 31", jh::test::tiny_test::test<"pod::span works with pod::array / Basic span properties">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 31"> registration_31{};
}
template<>
struct jh::test::tiny_test::test<"pod::span works with pod::array / Range-for iteration over span">
    : jh::test::tiny_test::test_definition<"pod::span works with pod::array / Range-for iteration over span", &::test::tiny_test_case_15_section_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 32">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 32", jh::test::tiny_test::test<"pod::span works with pod::array / Range-for iteration over span">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 32"> registration_32{};
}
template<>
struct jh::test::tiny_test::test<"pod::span works with pod::array / sub(), first(), last() slicing">
    : jh::test::tiny_test::test_definition<"pod::span works with pod::array / sub(), first(), last() slicing", &::test::tiny_test_case_15_section_3> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 33">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 33", jh::test::tiny_test::test<"pod::span works with pod::array / sub(), first(), last() slicing">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 33"> registration_33{};
}
template<>
struct jh::test::tiny_test::test<"pod::span works with pod::array / Equality comparison">
    : jh::test::tiny_test::test_definition<"pod::span works with pod::array / Equality comparison", &::test::tiny_test_case_15_section_4> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 34">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 34", jh::test::tiny_test::test<"pod::span works with pod::array / Equality comparison">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 34"> registration_34{};
}



namespace test {
void tiny_test_case_16_body(const int selected_section) {
    using jh::pod::array;
    using jh::pod::to_span;

    if (selected_section == 1) {
        array<int, 5> arr = {{1, 2, 3, 4, 5}};

        auto result = to_span(arr);
        jh::test::tiny_test::expect(static_cast<bool>((result)), "result");
        auto s = result.value();

        jh::test::tiny_test::expect(static_cast<bool>((s.size() == 5)), "s.size() == 5");
        jh::test::tiny_test::expect(static_cast<bool>((s[0] == 1)), "s[0] == 1");
        jh::test::tiny_test::expect(static_cast<bool>((s[4] == 5)), "s[4] == 5");
    }

    if (selected_section == 2) {
        const jh::pod::array<int, 3> arr = {{7, 8, 9}};

        auto result = jh::pod::to_span(arr);
        jh::test::tiny_test::expect(static_cast<bool>((result)), "result");
        auto s = result.value();

        jh::test::tiny_test::expect(static_cast<bool>((s.size() == 3)), "s.size() == 3");
        jh::test::tiny_test::expect(static_cast<bool>((s[0] == 7)), "s[0] == 7");
        jh::test::tiny_test::expect(static_cast<bool>((s[2] == 9)), "s[2] == 9");
    }

}
void tiny_test_case_16_section_1() { tiny_test_case_16_body(1); }
void tiny_test_case_16_section_2() { tiny_test_case_16_body(2); }
}
template<>
struct jh::test::tiny_test::test<"pod::to_span supports direct pod::array input / non-const pod::array">
    : jh::test::tiny_test::test_definition<"pod::to_span supports direct pod::array input / non-const pod::array", &::test::tiny_test_case_16_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 35">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 35", jh::test::tiny_test::test<"pod::to_span supports direct pod::array input / non-const pod::array">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 35"> registration_35{};
}
template<>
struct jh::test::tiny_test::test<"pod::to_span supports direct pod::array input / const pod::array">
    : jh::test::tiny_test::test_definition<"pod::to_span supports direct pod::array input / const pod::array", &::test::tiny_test_case_16_section_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 36">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 36", jh::test::tiny_test::test<"pod::to_span supports direct pod::array input / const pod::array">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 36"> registration_36{};
}



namespace test {
void tiny_test_case_17_body(const int selected_section) {
    using pod::array;
    using pod::to_span;

    if (selected_section == 1) {
        int raw[5] = {1, 2, 3, 4, 5};
        auto s = to_span<int>(raw);
        jh::test::tiny_test::expect(static_cast<bool>((s.size() == 5)), "s.size() == 5");
        jh::test::tiny_test::expect(static_cast<bool>((s[2] == 3)), "s[2] == 3");
    }

    if (selected_section == 2) {
        constexpr int raw[3] = {10, 20, 30};
        auto s = to_span<int>(raw);
        jh::test::tiny_test::expect(static_cast<bool>((s.size() == 3)), "s.size() == 3");
        jh::test::tiny_test::expect(static_cast<bool>((s[1] == 20)), "s[1] == 20");
    }

    if (selected_section == 3) {
        array<std::uint16_t, 4> a = {{11, 22, 33, 44}};
        auto s = to_span<std::uint16_t>(a.data);
        jh::test::tiny_test::expect(static_cast<bool>((s.size() == 4)), "s.size() == 4");
        jh::test::tiny_test::expect(static_cast<bool>((s[3] == 44)), "s[3] == 44");
    }

    if (selected_section == 4) {
        constexpr array<std::uint8_t, 2> a = {9, 99};
        auto s = to_span<std::uint8_t>(a.data);
        jh::test::tiny_test::expect(static_cast<bool>((s.size() == 2)), "s.size() == 2");
        jh::test::tiny_test::expect(static_cast<bool>((s[0] == 9)), "s[0] == 9");
        jh::test::tiny_test::expect(static_cast<bool>((s[1] == 99)), "s[1] == 99");
    }

    if (selected_section == 5) {
        struct DummyVec {
            int buf[3] = {7, 14, 21};

            [[nodiscard]] const int *data() const noexcept { return buf; }

            [[nodiscard]] static std::uint64_t size() noexcept { return 3; }
        };

        DummyVec v{};
        auto result = to_span(v);
        jh::test::tiny_test::expect(static_cast<bool>((result)), "result");
        auto s = result.value();
        jh::test::tiny_test::expect(static_cast<bool>((s.size() == 3)), "s.size() == 3");
        jh::test::tiny_test::expect(static_cast<bool>((s[2] == 21)), "s[2] == 21");
    }

    if (selected_section == 6) {
        struct ConstVec {
            const int buf[2] = {42, 88};

            [[nodiscard]] const int *data() const noexcept { return buf; }

            [[nodiscard]] static std::uint64_t size() noexcept { return 2; }
        };

        ConstVec v{};
        auto result = to_span(v);
        jh::test::tiny_test::expect(static_cast<bool>((result)), "result");
        auto s = result.value();
        jh::test::tiny_test::expect(static_cast<bool>((s.size() == 2)), "s.size() == 2");
        jh::test::tiny_test::expect(static_cast<bool>((s[0] == 42)), "s[0] == 42");
    }

}
void tiny_test_case_17_section_1() { tiny_test_case_17_body(1); }
void tiny_test_case_17_section_2() { tiny_test_case_17_body(2); }
void tiny_test_case_17_section_3() { tiny_test_case_17_body(3); }
void tiny_test_case_17_section_4() { tiny_test_case_17_body(4); }
void tiny_test_case_17_section_5() { tiny_test_case_17_body(5); }
void tiny_test_case_17_section_6() { tiny_test_case_17_body(6); }
}
template<>
struct jh::test::tiny_test::test<"pod::to_span from array and containers / T[N] raw array">
    : jh::test::tiny_test::test_definition<"pod::to_span from array and containers / T[N] raw array", &::test::tiny_test_case_17_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 37">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 37", jh::test::tiny_test::test<"pod::to_span from array and containers / T[N] raw array">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 37"> registration_37{};
}
template<>
struct jh::test::tiny_test::test<"pod::to_span from array and containers / const T[N] raw array">
    : jh::test::tiny_test::test_definition<"pod::to_span from array and containers / const T[N] raw array", &::test::tiny_test_case_17_section_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 38">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 38", jh::test::tiny_test::test<"pod::to_span from array and containers / const T[N] raw array">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 38"> registration_38{};
}
template<>
struct jh::test::tiny_test::test<"pod::to_span from array and containers / pod::array<T, N>">
    : jh::test::tiny_test::test_definition<"pod::to_span from array and containers / pod::array<T, N>", &::test::tiny_test_case_17_section_3> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 39">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 39", jh::test::tiny_test::test<"pod::to_span from array and containers / pod::array<T, N>">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 39"> registration_39{};
}
template<>
struct jh::test::tiny_test::test<"pod::to_span from array and containers / const pod::array<T, N>">
    : jh::test::tiny_test::test_definition<"pod::to_span from array and containers / const pod::array<T, N>", &::test::tiny_test_case_17_section_4> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 40">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 40", jh::test::tiny_test::test<"pod::to_span from array and containers / const pod::array<T, N>">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 40"> registration_40{};
}
template<>
struct jh::test::tiny_test::test<"pod::to_span from array and containers / to_span with vector-like struct">
    : jh::test::tiny_test::test_definition<"pod::to_span from array and containers / to_span with vector-like struct", &::test::tiny_test_case_17_section_5> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 41">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 41", jh::test::tiny_test::test<"pod::to_span from array and containers / to_span with vector-like struct">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 41"> registration_41{};
}
template<>
struct jh::test::tiny_test::test<"pod::to_span from array and containers / const container concept with data/size">
    : jh::test::tiny_test::test_definition<"pod::to_span from array and containers / const container concept with data/size", &::test::tiny_test_case_17_section_6> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 42">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 42", jh::test::tiny_test::test<"pod::to_span from array and containers / const container concept with data/size">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 42"> registration_42{};
}



namespace test {
void tiny_test_case_18_body(const int selected_section) {
    static constexpr char raw[] = "hello_pod_world";
    constexpr std::uint64_t len = 15;
    using pod::string_view;

    string_view sv{raw, len};

    if (selected_section == 1) {
        jh::test::tiny_test::expect(static_cast<bool>((sv.size() == len)), "sv.size() == len");
        jh::test::tiny_test::expect(static_cast<bool>((sv[0] == 'h')), "sv[0] == 'h'");
        jh::test::tiny_test::expect(static_cast<bool>((sv[len - 1] == 'd')), "sv[len - 1] == 'd'");
    }

    if (selected_section == 2) {
        static constexpr char raw2[] = "hello_pod_world";
        string_view other{raw2, len};
        jh::test::tiny_test::expect(static_cast<bool>((sv == other)), "sv == other");
    }

    if (selected_section == 3) {
        const auto sub_result = sv.sub(6, 3);
        jh::test::tiny_test::expect(static_cast<bool>((sub_result)), "sub_result");
        string_view sub = sub_result.value(); // expect "pod"
        jh::test::tiny_test::expect(static_cast<bool>((sub.size() == 3)), "sub.size() == 3");
        jh::test::tiny_test::expect(static_cast<bool>((sub == string_view{"pod", 3})), "sub == string_view{\"pod\", 3}");
        // temporary, do NOT use this for long life-time pod::string_view
    }

    if (selected_section == 4) {
        jh::test::tiny_test::expect(static_cast<bool>((sv.starts_with(string_view{"hello", 5}))), "sv.starts_with(string_view{\"hello\", 5})");
        jh::test::tiny_test::expect(static_cast<bool>((sv.ends_with(string_view{"world", 5}))), "sv.ends_with(string_view{\"world\", 5})");
    }

    if (selected_section == 5) {
        jh::test::tiny_test::expect(static_cast<bool>((sv.find('p') == 6)), "sv.find('p') == 6");
        jh::test::tiny_test::expect(static_cast<bool>((sv.find('z') == static_cast<std::uint64_t>(-1))), "sv.find('z') == static_cast<std::uint64_t>(-1)");
    }

    if (selected_section == 6) {
        auto hash = sv.hash();
        jh::test::tiny_test::expect(static_cast<bool>((hash)), "hash");
        jh::test::tiny_test::expect(static_cast<bool>((hash.value() != 0)), "hash.value() != 0");
    }

    if (selected_section == 7) {
        char buffer[32] = {};
        jh::test::tiny_test::expect(static_cast<bool>((sv.copy_to(buffer, sizeof(buffer)))), "sv.copy_to(buffer, sizeof(buffer))");
        jh::test::tiny_test::expect(static_cast<bool>((std::strcmp(buffer, "hello_pod_world") == 0)), "std::strcmp(buffer, \"hello_pod_world\") == 0");
    }if (selected_section == 8) {
        using namespace std;
        string_view a{"abc", 3};
        string_view b{"abd", 3};
        string_view c{"abc", 3};

        jh::test::tiny_test::expect(static_cast<bool>((a.compare(b) < 0)), "a.compare(b) < 0");
        jh::test::tiny_test::expect(static_cast<bool>((b.compare(a) > 0)), "b.compare(a) > 0");
        jh::test::tiny_test::expect(static_cast<bool>((a.compare(c) == 0)), "a.compare(c) == 0");

        auto ab = (a <=> b);
        auto ba = (b <=> a);
        auto ac = (a <=> c);

        jh::test::tiny_test::expect(static_cast<bool>((ab == strong_ordering::less)), "ab == strong_ordering::less");
        jh::test::tiny_test::expect(static_cast<bool>((ba == strong_ordering::greater)), "ba == strong_ordering::greater");
        jh::test::tiny_test::expect(static_cast<bool>((ac == strong_ordering::equal)), "ac == strong_ordering::equal");

        auto cmp_to_order = [](int cmp) {
            if (cmp < 0) return strong_ordering::less;
            if (cmp > 0) return strong_ordering::greater;
            return strong_ordering::equal;
        };

        jh::test::tiny_test::expect(static_cast<bool>(((a <=> b) == cmp_to_order(a.compare(b)))), "(a <=> b) == cmp_to_order(a.compare(b))");
        jh::test::tiny_test::expect(static_cast<bool>(((b <=> a) == cmp_to_order(b.compare(a)))), "(b <=> a) == cmp_to_order(b.compare(a))");
        jh::test::tiny_test::expect(static_cast<bool>(((a <=> c) == cmp_to_order(a.compare(c)))), "(a <=> c) == cmp_to_order(a.compare(c))");

        jh::test::tiny_test::expect(static_cast<bool>((a < b)), "a < b");
        jh::test::tiny_test::expect(static_cast<bool>((b > a)), "b > a");
        jh::test::tiny_test::expect(static_cast<bool>((!(a > b))), "!(a > b)");
        jh::test::tiny_test::expect(static_cast<bool>((a == c)), "a == c");
        jh::test::tiny_test::expect(static_cast<bool>((a <= c)), "a <= c");
        jh::test::tiny_test::expect(static_cast<bool>((a >= c)), "a >= c");
    }


}
void tiny_test_case_18_section_1() { tiny_test_case_18_body(1); }
void tiny_test_case_18_section_2() { tiny_test_case_18_body(2); }
void tiny_test_case_18_section_3() { tiny_test_case_18_body(3); }
void tiny_test_case_18_section_4() { tiny_test_case_18_body(4); }
void tiny_test_case_18_section_5() { tiny_test_case_18_body(5); }
void tiny_test_case_18_section_6() { tiny_test_case_18_body(6); }
void tiny_test_case_18_section_7() { tiny_test_case_18_body(7); }
void tiny_test_case_18_section_8() { tiny_test_case_18_body(8); }
}
template<>
struct jh::test::tiny_test::test<"pod::string_view basic usage / Correct length and data">
    : jh::test::tiny_test::test_definition<"pod::string_view basic usage / Correct length and data", &::test::tiny_test_case_18_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 43">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 43", jh::test::tiny_test::test<"pod::string_view basic usage / Correct length and data">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 43"> registration_43{};
}
template<>
struct jh::test::tiny_test::test<"pod::string_view basic usage / Equality comparison">
    : jh::test::tiny_test::test_definition<"pod::string_view basic usage / Equality comparison", &::test::tiny_test_case_18_section_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 44">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 44", jh::test::tiny_test::test<"pod::string_view basic usage / Equality comparison">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 44"> registration_44{};
}
template<>
struct jh::test::tiny_test::test<"pod::string_view basic usage / Subrange works">
    : jh::test::tiny_test::test_definition<"pod::string_view basic usage / Subrange works", &::test::tiny_test_case_18_section_3> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 45">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 45", jh::test::tiny_test::test<"pod::string_view basic usage / Subrange works">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 45"> registration_45{};
}
template<>
struct jh::test::tiny_test::test<"pod::string_view basic usage / Starts with / Ends with">
    : jh::test::tiny_test::test_definition<"pod::string_view basic usage / Starts with / Ends with", &::test::tiny_test_case_18_section_4> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 46">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 46", jh::test::tiny_test::test<"pod::string_view basic usage / Starts with / Ends with">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 46"> registration_46{};
}
template<>
struct jh::test::tiny_test::test<"pod::string_view basic usage / Find character">
    : jh::test::tiny_test::test_definition<"pod::string_view basic usage / Find character", &::test::tiny_test_case_18_section_5> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 47">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 47", jh::test::tiny_test::test<"pod::string_view basic usage / Find character">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 47"> registration_47{};
}
template<>
struct jh::test::tiny_test::test<"pod::string_view basic usage / Hash is deterministic and non-zero">
    : jh::test::tiny_test::test_definition<"pod::string_view basic usage / Hash is deterministic and non-zero", &::test::tiny_test_case_18_section_6> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 48">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 48", jh::test::tiny_test::test<"pod::string_view basic usage / Hash is deterministic and non-zero">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 48"> registration_48{};
}
template<>
struct jh::test::tiny_test::test<"pod::string_view basic usage / Copy to buffer">
    : jh::test::tiny_test::test_definition<"pod::string_view basic usage / Copy to buffer", &::test::tiny_test_case_18_section_7> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 49">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 49", jh::test::tiny_test::test<"pod::string_view basic usage / Copy to buffer">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 49"> registration_49{};
}
template<>
struct jh::test::tiny_test::test<"pod::string_view basic usage / Three-way comparison and compare() consistency">
    : jh::test::tiny_test::test_definition<"pod::string_view basic usage / Three-way comparison and compare() consistency", &::test::tiny_test_case_18_section_8> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 50">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 50", jh::test::tiny_test::test<"pod::string_view basic usage / Three-way comparison and compare() consistency">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 50"> registration_50{};
}



namespace test {
void tiny_test_case_19() {
    using jh::pod::string_view;
    // case 1: simple literal
    constexpr auto sv = string_view::from_literal("hello");

    jh::test::tiny_test::expect(static_cast<bool>((sv.size() == 5)), "sv.size() == 5");
    jh::test::tiny_test::expect(static_cast<bool>((sv == string_view{"hello", std::strlen("hello")})), "sv == string_view{\"hello\", std::strlen(\"hello\")}");

    // case 2: empty string literal
    constexpr auto sv_empty = string_view::from_literal("");
    jh::test::tiny_test::expect(static_cast<bool>((sv_empty.empty())), "sv_empty.empty()");
    jh::test::tiny_test::expect(static_cast<bool>((sv_empty == string_view{"", std::strlen("")})), "sv_empty == string_view{\"\", std::strlen(\"\")}");

    // case 3: longer literal
    constexpr auto sv_long = string_view::from_literal("hello_pod_world");
    jh::test::tiny_test::expect(static_cast<bool>((sv_long.size() == std::strlen("hello_pod_world"))), "sv_long.size() == std::strlen(\"hello_pod_world\")");
    jh::test::tiny_test::expect(static_cast<bool>((sv_long == string_view{"hello_pod_world", std::strlen("hello_pod_world")})), "sv_long == string_view{\"hello_pod_world\", std::strlen(\"hello_pod_world\")}");

}
}
template<>
struct jh::test::tiny_test::test<"string_view from_literal correctness">
    : jh::test::tiny_test::test_definition<"string_view from_literal correctness", &::test::tiny_test_case_19> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 51">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 51", jh::test::tiny_test::test<"string_view from_literal correctness">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 51"> registration_51{};
}



namespace test {
void tiny_test_case_20() {
    using pod::array;
    using pod::string_view;
    const auto str1 = "abcd";
    const auto str2 = "abcd"; // same str

    const array<string_view, 4> a1 =
            {
                    {
                            {str1, 1},
                            {str1, 2},
                            {str1, 3},
                            {str1, 4}
                    }
            };

    const array<string_view, 4> a2 =
            {
                    {
                            {str2, 1},
                            {str2, 2},
                            {str2, 3},
                            {str2, 4}
                    }
            };

    jh::test::tiny_test::expect(static_cast<bool>((a1 == a2)), "a1 == a2");  // different memories but same semantics

}
}
template<>
struct jh::test::tiny_test::test<"pod::array<pod::string_view> comparison">
    : jh::test::tiny_test::test_definition<"pod::array<pod::string_view> comparison", &::test::tiny_test_case_20> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 52">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 52", jh::test::tiny_test::test<"pod::array<pod::string_view> comparison">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 52"> registration_52{};
}



namespace test {
void tiny_test_case_21_body(const int selected_section) {
    using jh::pod::bytes_view;
    using jh::pod::array;
    using jh::meta::c_hash;

    if (selected_section == 1) {
        array<std::uint32_t, 4> a = {1, 2, 3, 4};
        std::array<std::uint32_t, 4> b = {1, 2, 3, 4};

        auto va = bytes_view::from(a);
        auto vb = bytes_view::from(b.data(), b.size());

        jh::test::tiny_test::expect(static_cast<bool>((va == vb)), "va == vb");
    jh::test::tiny_test::expect(static_cast<bool>((va.hash().value() == vb.hash().value())), "va.hash().value() == vb.hash().value()");
    }

    if (selected_section == 2) {
        array<std::uint32_t, 4> a = {1, 2, 3, 4};
        array<std::uint32_t, 4> c = {4, 3, 2, 1};

        auto va = bytes_view::from(a);
        auto vc = bytes_view::from(c);

        jh::test::tiny_test::expect(static_cast<bool>((va != vc)), "va != vc");
    jh::test::tiny_test::expect(static_cast<bool>((va.hash().value() != vc.hash().value())), "va.hash().value() != vc.hash().value()");
    }

    if (selected_section == 3) {
        struct P {
            std::uint32_t x;
        };

        P p1{0x11223344};
        P p2{0x55667788};

        auto h1 = bytes_view::from(p1).hash();
        auto h2 = bytes_view::from(p2).hash();

        jh::test::tiny_test::expect(static_cast<bool>((h1.value() != h2.value())), "h1.value() != h2.value()");
    }

    if (selected_section == 4) {
        using jh::pod::string_view;

        constexpr char raw[] = "hash_check_test";
        string_view sv{raw, sizeof(raw) - 1};
        auto bv = bytes_view::from(raw, sizeof(raw) - 1);

        jh::test::tiny_test::expect(static_cast<bool>((sv.size() == bv.len)), "sv.size() == bv.len");
        jh::test::tiny_test::expect(static_cast<bool>((std::memcmp(sv.data, bv.data, sv.size()) == 0)), "std::memcmp(sv.data, bv.data, sv.size()) == 0");
        const auto string_hash = sv.hash();
        const auto byte_hash = bv.hash();
        jh::test::tiny_test::expect(static_cast<bool>((string_hash)), "string_hash");
        jh::test::tiny_test::expect(static_cast<bool>((byte_hash)), "byte_hash");
        jh::test::tiny_test::expect(static_cast<bool>((string_hash.value() == byte_hash.value())), "string_hash.value() == byte_hash.value()");
    }

}
void tiny_test_case_21_section_1() { tiny_test_case_21_body(1); }
void tiny_test_case_21_section_2() { tiny_test_case_21_body(2); }
void tiny_test_case_21_section_3() { tiny_test_case_21_body(3); }
void tiny_test_case_21_section_4() { tiny_test_case_21_body(4); }
}
template<>
struct jh::test::tiny_test::test<"bytes_view hash reflects exact byte content / Equal content produces same hash">
    : jh::test::tiny_test::test_definition<"bytes_view hash reflects exact byte content / Equal content produces same hash", &::test::tiny_test_case_21_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 53">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 53", jh::test::tiny_test::test<"bytes_view hash reflects exact byte content / Equal content produces same hash">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 53"> registration_53{};
}
template<>
struct jh::test::tiny_test::test<"bytes_view hash reflects exact byte content / Different content produces different hash">
    : jh::test::tiny_test::test_definition<"bytes_view hash reflects exact byte content / Different content produces different hash", &::test::tiny_test_case_21_section_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 54">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 54", jh::test::tiny_test::test<"bytes_view hash reflects exact byte content / Different content produces different hash">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 54"> registration_54{};
}
template<>
struct jh::test::tiny_test::test<"bytes_view hash reflects exact byte content / Same layout different values changes hash">
    : jh::test::tiny_test::test_definition<"bytes_view hash reflects exact byte content / Same layout different values changes hash", &::test::tiny_test_case_21_section_3> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 55">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 55", jh::test::tiny_test::test<"bytes_view hash reflects exact byte content / Same layout different values changes hash">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 55"> registration_55{};
}
template<>
struct jh::test::tiny_test::test<"bytes_view hash reflects exact byte content / pod::string_view vs bytes_view with same content">
    : jh::test::tiny_test::test_definition<"bytes_view hash reflects exact byte content / pod::string_view vs bytes_view with same content", &::test::tiny_test_case_21_section_4> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 56">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 56", jh::test::tiny_test::test<"bytes_view hash reflects exact byte content / pod::string_view vs bytes_view with same content">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 56"> registration_56{};
}



namespace test {
void tiny_test_case_22_body(const int selected_section) {
    using jh::pod::string_view;
    using jh::meta::c_hash;

    static constexpr char content1[] = "alpha_test";
    static constexpr char content2[] = "alpha_test";  // same content, different instance
    static constexpr char content3[] = "beta_test";

    const string_view sv1{content1, std::strlen(content1)};
    const string_view sv2{content2, std::strlen(content2)};
    const string_view sv3{content3, std::strlen(content3)};

    if (selected_section == 1) {
        jh::test::tiny_test::expect(static_cast<bool>((sv1 == sv2)), "sv1 == sv2");
        jh::test::tiny_test::expect(static_cast<bool>((sv1.hash() == sv2.hash())), "sv1.hash() == sv2.hash()");
    }

    if (selected_section == 2) {
        jh::test::tiny_test::expect(static_cast<bool>((sv1 != sv3)), "sv1 != sv3");
        jh::test::tiny_test::expect(static_cast<bool>((sv1.hash() != sv3.hash())), "sv1.hash() != sv3.hash()");
    }

    if (selected_section == 3) {
        auto h1 = sv1.hash(c_hash::fnv1a64);
        auto h2 = sv1.hash(c_hash::djb2);
        auto h3 = sv1.hash(c_hash::sdbm);
        auto h4 = sv1.hash(c_hash::fnv1_64);
        auto h5 = sv1.hash(c_hash::murmur64);
        auto h6 = sv1.hash(c_hash::xxhash64);

        jh::test::tiny_test::expect(static_cast<bool>((h1)), "h1");
        jh::test::tiny_test::expect(static_cast<bool>((h2)), "h2");
        jh::test::tiny_test::expect(static_cast<bool>((h3)), "h3");
        jh::test::tiny_test::expect(static_cast<bool>((h4)), "h4");
        jh::test::tiny_test::expect(static_cast<bool>((h5)), "h5");
        jh::test::tiny_test::expect(static_cast<bool>((h6)), "h6");

        // Same view, multiple algorithms must differ
        jh::test::tiny_test::expect(static_cast<bool>((h1.value() != h2.value())), "h1.value() != h2.value()");
        jh::test::tiny_test::expect(static_cast<bool>((h2.value() != h3.value())), "h2.value() != h3.value()");
        jh::test::tiny_test::expect(static_cast<bool>((h3.value() != h4.value())), "h3.value() != h4.value()");
        jh::test::tiny_test::expect(static_cast<bool>((h4.value() != h5.value())), "h4.value() != h5.value()");
        jh::test::tiny_test::expect(static_cast<bool>((h5.value() != h6.value())), "h5.value() != h6.value()");
    }

    if (selected_section == 4) {
        auto bv = jh::pod::bytes_view::from(content1, sizeof(content1) - 1);
        const auto string_hash = sv1.hash();
        const auto byte_hash = bv.hash();
        jh::test::tiny_test::expect(static_cast<bool>((string_hash)), "string_hash");
        jh::test::tiny_test::expect(static_cast<bool>((byte_hash)), "byte_hash");
        jh::test::tiny_test::expect(static_cast<bool>((string_hash.value() == byte_hash.value())), "string_hash.value() == byte_hash.value()");
    }

}
void tiny_test_case_22_section_1() { tiny_test_case_22_body(1); }
void tiny_test_case_22_section_2() { tiny_test_case_22_body(2); }
void tiny_test_case_22_section_3() { tiny_test_case_22_body(3); }
void tiny_test_case_22_section_4() { tiny_test_case_22_body(4); }
}
template<>
struct jh::test::tiny_test::test<"string_view hash reflects exact character content / Equal content produces same hash">
    : jh::test::tiny_test::test_definition<"string_view hash reflects exact character content / Equal content produces same hash", &::test::tiny_test_case_22_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 57">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 57", jh::test::tiny_test::test<"string_view hash reflects exact character content / Equal content produces same hash">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 57"> registration_57{};
}
template<>
struct jh::test::tiny_test::test<"string_view hash reflects exact character content / Different content produces different hash">
    : jh::test::tiny_test::test_definition<"string_view hash reflects exact character content / Different content produces different hash", &::test::tiny_test_case_22_section_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 58">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 58", jh::test::tiny_test::test<"string_view hash reflects exact character content / Different content produces different hash">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 58"> registration_58{};
}
template<>
struct jh::test::tiny_test::test<"string_view hash reflects exact character content / Hash consistency across methods">
    : jh::test::tiny_test::test_definition<"string_view hash reflects exact character content / Hash consistency across methods", &::test::tiny_test_case_22_section_3> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 59">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 59", jh::test::tiny_test::test<"string_view hash reflects exact character content / Hash consistency across methods">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 59"> registration_59{};
}
template<>
struct jh::test::tiny_test::test<"string_view hash reflects exact character content / string_view vs bytes_view from same buffer">
    : jh::test::tiny_test::test_definition<"string_view hash reflects exact character content / string_view vs bytes_view from same buffer", &::test::tiny_test_case_22_section_4> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 60">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 60", jh::test::tiny_test::test<"string_view hash reflects exact character content / string_view vs bytes_view from same buffer">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 60"> registration_60{};
}


namespace user_override {

    std::ostream &operator<<(std::ostream &os, const jh::pod::array<int, 3> &) {
        os << "user_defined_override";
        return os;
    }

} // namespace user_override


namespace test {
void tiny_test_case_23_body(const int selected_section) {
    std::ostringstream os1, os2;

    if (selected_section == 1) {
        using user_override::operator<<;
        jh::pod::array<int, 3> a{};
        os1 << a;
        jh::test::tiny_test::expect(static_cast<bool>((os1.str() == "user_defined_override")), "os1.str() == \"user_defined_override\"");
    }

    if (selected_section == 2) {
        jh::pod::array<int, 5> b{};
        os2 << b;
        jh::test::tiny_test::expect(static_cast<bool>((os2.str() == "[0, 0, 0, 0, 0]")), "os2.str() == \"[0, 0, 0, 0, 0]\"");
    }

}
void tiny_test_case_23_section_1() { tiny_test_case_23_body(1); }
void tiny_test_case_23_section_2() { tiny_test_case_23_body(2); }
}
template<>
struct jh::test::tiny_test::test<"User-defined operator<< overrides default inline / user override active">
    : jh::test::tiny_test::test_definition<"User-defined operator<< overrides default inline / user override active", &::test::tiny_test_case_23_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 61">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 61", jh::test::tiny_test::test<"User-defined operator<< overrides default inline / user override active">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 61"> registration_61{};
}
template<>
struct jh::test::tiny_test::test<"User-defined operator<< overrides default inline / default inline remains active when no override">
    : jh::test::tiny_test::test_definition<"User-defined operator<< overrides default inline / default inline remains active when no override", &::test::tiny_test_case_23_section_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 62">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 62", jh::test::tiny_test::test<"User-defined operator<< overrides default inline / default inline remains active when no override">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 62"> registration_62{};
}



namespace test {
void tiny_test_case_24_body(const int selected_section) {
    using jh::pod::array;
    using jh::pod::pair;
    using jh::pod::optional;
    using jh::pod::bitflags;
    using jh::pod::bytes_view;
    using jh::pod::span;
    using jh::pod::string_view;
    using jh::typed::monostate;
    using jh::pod::tuple;

    if (selected_section == 1) {
        array<int, 3> a = {1, 2, 3};
        std::ostringstream oss;
        oss << a;
        jh::test::tiny_test::expect(static_cast<bool>((oss.str() == "[1, 2, 3]")), "oss.str() == \"[1, 2, 3]\"");
    }

    if (selected_section == 2) {
        array<char, 6> s = {"hello"};
        std::ostringstream oss;
        oss << s;
        jh::test::tiny_test::expect(static_cast<bool>((oss.str() == "\"hello\"")), "oss.str() == \"\\\"hello\\\"\"");
    }

    if (selected_section == 3) {
        pair<int, float> p = {42, 3.14f};
        std::ostringstream oss;
        oss << p;
        jh::test::tiny_test::expect(static_cast<bool>((oss.str() == "{42, 3.14}")), "oss.str() == \"{42, 3.14}\"");
    }

    if (selected_section == 4) {
        std::ostringstream oss1, oss2;
        optional<int> o1{};
        o1.store(7);
        optional<int> o2{};
        oss1 << o1;
        oss2 << o2;
        jh::test::tiny_test::expect(static_cast<bool>((oss1.str() == "7")), "oss1.str() == \"7\"");
        jh::test::tiny_test::expect(static_cast<bool>((oss2.str() == "nullopt")), "oss2.str() == \"nullopt\"");
    }

    if (selected_section == 5) {
        std::ostringstream oss;
        bitflags<8> f{};
        f.set(0);
        f.set(3);
        f.set(7);  // binary: 10001001 → hex: 0x'89
        oss << std::hex << f;
        jh::test::tiny_test::expect(static_cast<bool>((oss.str() == "0x'89'")), "oss.str() == \"0x'89'\"");
    }

    if (selected_section == 6) {
        std::ostringstream oss;
        bitflags<8> f{};
        f.set(1);
        f.set(2);
        oss << std::dec << f;
        jh::test::tiny_test::expect(static_cast<bool>((oss.str() == "0b'00000110'")), "oss.str() == \"0b'00000110'\"");
    }

    if (selected_section == 7) {
        std::ostringstream oss;
        const uint8_t raw[] = {0x48, 0x65, 0x6c, 0x6c, 0x6f};  // "Hello"
        bytes_view bv = bytes_view::from(raw, 5);
        oss << bv;
        jh::test::tiny_test::expect(static_cast<bool>((oss.str() == "base64'SGVsbG8='")), "oss.str() == \"base64'SGVsbG8='\"");
    }

    if (selected_section == 8) {
        array<int, 4> arr = {1, 2, 3, 4};
        span<int> sp{arr.data, std::size(arr)};
        std::ostringstream oss;
        oss << sp;
        jh::test::tiny_test::expect(static_cast<bool>((oss.str().starts_with("span<"))), "oss.str().starts_with(\"span<\")");
        jh::test::tiny_test::expect(static_cast<bool>((oss.str().find("[1, 2, 3, 4]") != std::string::npos)), "oss.str().find(\"[1, 2, 3, 4]\") != std::string::npos");
    }

    if (selected_section == 9) {
        constexpr char raw[] = "pod_string";
        string_view sv{raw, strlen(raw)};
        std::ostringstream oss;
        oss << sv;
        jh::test::tiny_test::expect(static_cast<bool>((oss.str() == "string_view\"pod_string\"")), "oss.str() == \"string_view\\\"pod_string\\\"\"");
    }

    if (selected_section == 10) {
        monostate m{};
        std::ostringstream oss;
        oss << m;
        jh::test::tiny_test::expect(static_cast<bool>((oss.str() == "null")), "oss.str() == \"null\"");
    }

    if (selected_section == 11) {
        using jh::pod::make_tuple;

        std::ostringstream oss0, oss1, oss5;

        tuple<> t0{};
        tuple<int> t1{{{42}, {}}};
        auto t5 = make_tuple(1, 2, 3, 4, 5);

        oss0 << t0;
        oss1 << t1;
        oss5 << t5;

        jh::test::tiny_test::expect(static_cast<bool>((oss0.str() == "()")), "oss0.str() == \"()\"");
        jh::test::tiny_test::expect(static_cast<bool>((oss1.str() == "(42,)")), "oss1.str() == \"(42,)\"");
        jh::test::tiny_test::expect(static_cast<bool>((oss5.str() == "(1, 2, 3, 4, 5)")), "oss5.str() == \"(1, 2, 3, 4, 5)\"");
    }

}
void tiny_test_case_24_section_1() { tiny_test_case_24_body(1); }
void tiny_test_case_24_section_2() { tiny_test_case_24_body(2); }
void tiny_test_case_24_section_3() { tiny_test_case_24_body(3); }
void tiny_test_case_24_section_4() { tiny_test_case_24_body(4); }
void tiny_test_case_24_section_5() { tiny_test_case_24_body(5); }
void tiny_test_case_24_section_6() { tiny_test_case_24_body(6); }
void tiny_test_case_24_section_7() { tiny_test_case_24_body(7); }
void tiny_test_case_24_section_8() { tiny_test_case_24_body(8); }
void tiny_test_case_24_section_9() { tiny_test_case_24_body(9); }
void tiny_test_case_24_section_10() { tiny_test_case_24_body(10); }
void tiny_test_case_24_section_11() { tiny_test_case_24_body(11); }
}
template<>
struct jh::test::tiny_test::test<"pod::ostream << overloads for built-in and custom POD types / array<T, N> general printable">
    : jh::test::tiny_test::test_definition<"pod::ostream << overloads for built-in and custom POD types / array<T, N> general printable", &::test::tiny_test_case_24_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 63">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 63", jh::test::tiny_test::test<"pod::ostream << overloads for built-in and custom POD types / array<T, N> general printable">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 63"> registration_63{};
}
template<>
struct jh::test::tiny_test::test<"pod::ostream << overloads for built-in and custom POD types / array<char, N> as escaped JSON string">
    : jh::test::tiny_test::test_definition<"pod::ostream << overloads for built-in and custom POD types / array<char, N> as escaped JSON string", &::test::tiny_test_case_24_section_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 64">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 64", jh::test::tiny_test::test<"pod::ostream << overloads for built-in and custom POD types / array<char, N> as escaped JSON string">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 64"> registration_64{};
}
template<>
struct jh::test::tiny_test::test<"pod::ostream << overloads for built-in and custom POD types / pair<T1, T2>">
    : jh::test::tiny_test::test_definition<"pod::ostream << overloads for built-in and custom POD types / pair<T1, T2>", &::test::tiny_test_case_24_section_3> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 65">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 65", jh::test::tiny_test::test<"pod::ostream << overloads for built-in and custom POD types / pair<T1, T2>">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 65"> registration_65{};
}
template<>
struct jh::test::tiny_test::test<"pod::ostream << overloads for built-in and custom POD types / optional<T> with and without value">
    : jh::test::tiny_test::test_definition<"pod::ostream << overloads for built-in and custom POD types / optional<T> with and without value", &::test::tiny_test_case_24_section_4> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 66">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 66", jh::test::tiny_test::test<"pod::ostream << overloads for built-in and custom POD types / optional<T> with and without value">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 66"> registration_66{};
}
template<>
struct jh::test::tiny_test::test<"pod::ostream << overloads for built-in and custom POD types / bitflags<N> output in hex format">
    : jh::test::tiny_test::test_definition<"pod::ostream << overloads for built-in and custom POD types / bitflags<N> output in hex format", &::test::tiny_test_case_24_section_5> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 67">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 67", jh::test::tiny_test::test<"pod::ostream << overloads for built-in and custom POD types / bitflags<N> output in hex format">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 67"> registration_67{};
}
template<>
struct jh::test::tiny_test::test<"pod::ostream << overloads for built-in and custom POD types / bitflags<N> output in binary format">
    : jh::test::tiny_test::test_definition<"pod::ostream << overloads for built-in and custom POD types / bitflags<N> output in binary format", &::test::tiny_test_case_24_section_6> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 68">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 68", jh::test::tiny_test::test<"pod::ostream << overloads for built-in and custom POD types / bitflags<N> output in binary format">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 68"> registration_68{};
}
template<>
struct jh::test::tiny_test::test<"pod::ostream << overloads for built-in and custom POD types / bytes_view outputs base64">
    : jh::test::tiny_test::test_definition<"pod::ostream << overloads for built-in and custom POD types / bytes_view outputs base64", &::test::tiny_test_case_24_section_7> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 69">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 69", jh::test::tiny_test::test<"pod::ostream << overloads for built-in and custom POD types / bytes_view outputs base64">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 69"> registration_69{};
}
template<>
struct jh::test::tiny_test::test<"pod::ostream << overloads for built-in and custom POD types / span<T> prints container-like output">
    : jh::test::tiny_test::test_definition<"pod::ostream << overloads for built-in and custom POD types / span<T> prints container-like output", &::test::tiny_test_case_24_section_8> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 70">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 70", jh::test::tiny_test::test<"pod::ostream << overloads for built-in and custom POD types / span<T> prints container-like output">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 70"> registration_70{};
}
template<>
struct jh::test::tiny_test::test<"pod::ostream << overloads for built-in and custom POD types / string_view outputs quoted content">
    : jh::test::tiny_test::test_definition<"pod::ostream << overloads for built-in and custom POD types / string_view outputs quoted content", &::test::tiny_test_case_24_section_9> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 71">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 71", jh::test::tiny_test::test<"pod::ostream << overloads for built-in and custom POD types / string_view outputs quoted content">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 71"> registration_71{};
}
template<>
struct jh::test::tiny_test::test<"pod::ostream << overloads for built-in and custom POD types / typed::monostate prints as null">
    : jh::test::tiny_test::test_definition<"pod::ostream << overloads for built-in and custom POD types / typed::monostate prints as null", &::test::tiny_test_case_24_section_10> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 72">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 72", jh::test::tiny_test::test<"pod::ostream << overloads for built-in and custom POD types / typed::monostate prints as null">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 72"> registration_72{};
}
template<>
struct jh::test::tiny_test::test<"pod::ostream << overloads for built-in and custom POD types / pod::tuple ostream output formats correctly">
    : jh::test::tiny_test::test_definition<"pod::ostream << overloads for built-in and custom POD types / pod::tuple ostream output formats correctly", &::test::tiny_test_case_24_section_11> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 73">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 73", jh::test::tiny_test::test<"pod::ostream << overloads for built-in and custom POD types / pod::tuple ostream output formats correctly">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 73"> registration_73{};
}



namespace test {
void tiny_test_case_25_body(const int selected_section) {
    using namespace jh::pod;

    auto test_ops = []<std::size_t N>() {
        using F = bitflags<N>;

        F a{}, b{};

        a.set(0);
        a.set(2);

        b.set(2);
        b.set(3);

        // OR
        auto r_or = a | b;
        jh::test::tiny_test::expect(static_cast<bool>((r_or.has(0))), "r_or.has(0)");
        jh::test::tiny_test::expect(static_cast<bool>((r_or.has(2))), "r_or.has(2)");
        jh::test::tiny_test::expect(static_cast<bool>((r_or.has(3))), "r_or.has(3)");

        // AND
        auto r_and = a & b;
        jh::test::tiny_test::expect_not(static_cast<bool>((r_and.has(0))), "r_and.has(0)");
        jh::test::tiny_test::expect(static_cast<bool>((r_and.has(2))), "r_and.has(2)");
        jh::test::tiny_test::expect_not(static_cast<bool>((r_and.has(3))), "r_and.has(3)");

        // XOR
        auto r_xor = a ^ b;
        jh::test::tiny_test::expect(static_cast<bool>((r_xor.has(0))), "r_xor.has(0)");
        jh::test::tiny_test::expect_not(static_cast<bool>((r_xor.has(2))), "r_xor.has(2)");
        jh::test::tiny_test::expect(static_cast<bool>((r_xor.has(3))), "r_xor.has(3)");

        // NOT
        auto r_not = ~a;
        jh::test::tiny_test::expect_not(static_cast<bool>((r_not.has(0))), "r_not.has(0)");
        jh::test::tiny_test::expect(static_cast<bool>((r_not.has(1))), "r_not.has(1)");  // was unset → now set
    };

    if (selected_section == 1) { test_ops.template operator()<8>(); }if (selected_section == 2) { test_ops.template operator()<16>(); }if (selected_section == 3) { test_ops.template operator()<32>(); }if (selected_section == 4) { test_ops.template operator()<64>(); }

}
void tiny_test_case_25_section_1() { tiny_test_case_25_body(1); }
void tiny_test_case_25_section_2() { tiny_test_case_25_body(2); }
void tiny_test_case_25_section_3() { tiny_test_case_25_body(3); }
void tiny_test_case_25_section_4() { tiny_test_case_25_body(4); }
}
template<>
struct jh::test::tiny_test::test<"bitflags native ops with auto deduction behave correctly / bitflags<8>">
    : jh::test::tiny_test::test_definition<"bitflags native ops with auto deduction behave correctly / bitflags<8>", &::test::tiny_test_case_25_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 74">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 74", jh::test::tiny_test::test<"bitflags native ops with auto deduction behave correctly / bitflags<8>">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 74"> registration_74{};
}
template<>
struct jh::test::tiny_test::test<"bitflags native ops with auto deduction behave correctly / bitflags<16>">
    : jh::test::tiny_test::test_definition<"bitflags native ops with auto deduction behave correctly / bitflags<16>", &::test::tiny_test_case_25_section_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 75">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 75", jh::test::tiny_test::test<"bitflags native ops with auto deduction behave correctly / bitflags<16>">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 75"> registration_75{};
}
template<>
struct jh::test::tiny_test::test<"bitflags native ops with auto deduction behave correctly / bitflags<32>">
    : jh::test::tiny_test::test_definition<"bitflags native ops with auto deduction behave correctly / bitflags<32>", &::test::tiny_test_case_25_section_3> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 76">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 76", jh::test::tiny_test::test<"bitflags native ops with auto deduction behave correctly / bitflags<32>">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 76"> registration_76{};
}
template<>
struct jh::test::tiny_test::test<"bitflags native ops with auto deduction behave correctly / bitflags<64>">
    : jh::test::tiny_test::test_definition<"bitflags native ops with auto deduction behave correctly / bitflags<64>", &::test::tiny_test_case_25_section_4> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 77">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 77", jh::test::tiny_test::test<"bitflags native ops with auto deduction behave correctly / bitflags<64>">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 77"> registration_77{};
}



namespace test {
void tiny_test_case_26() {
    using jh::pod::array;

    array<int, 6> arr = {{1, 2, 3, 4, 5, 6}};
    auto even = arr
                | std::views::filter([](int x) { return x % 2 == 0; })
                | std::views::transform([](int x) { return x * 10; });

    std::vector<int> result;
    std::ranges::copy(even, std::back_inserter(result));

    jh::test::tiny_test::expect(static_cast<bool>((result == std::vector<int>{20, 40, 60})), "result == std::vector<int>{20, 40, 60}");

}
}
template<>
struct jh::test::tiny_test::test<"pod::array works with std::views pipelines">
    : jh::test::tiny_test::test_definition<"pod::array works with std::views pipelines", &::test::tiny_test_case_26> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 78">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 78", jh::test::tiny_test::test<"pod::array works with std::views pipelines">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 78"> registration_78{};
}



namespace test {
void tiny_test_case_27() {
    using jh::pod::array;

    array<int, 3> arr = {{10, 20, 30}};
    auto &[a, b, c] = arr;
    jh::test::tiny_test::expect(static_cast<bool>((a == arr.data[0])), "a == arr.data[0]");
    jh::test::tiny_test::expect(static_cast<bool>((b == arr.data[1])), "b == arr.data[1]");
    jh::test::tiny_test::expect(static_cast<bool>((c == arr.data[2])), "c == arr.data[2]");

    a = 42;
    jh::test::tiny_test::expect(static_cast<bool>((arr[0] == 42)), "arr[0] == 42");

}
}
template<>
struct jh::test::tiny_test::test<"pod::array supports structured bindings">
    : jh::test::tiny_test::test_definition<"pod::array supports structured bindings", &::test::tiny_test_case_27> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 79">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 79", jh::test::tiny_test::test<"pod::array supports structured bindings">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 79"> registration_79{};
}



namespace test {
void tiny_test_case_28() {
    using jh::pod::pair;
    using jh::pod::make_pair;

    pair<int, double> p1{1, 2.5};
    auto p2 = make_pair(1, 2.5);

    jh::test::tiny_test::expect(static_cast<bool>((p1 == p2)), "p1 == p2");

}
}
template<>
struct jh::test::tiny_test::test<"pod::make_pair and direct pair construction produce same result">
    : jh::test::tiny_test::test_definition<"pod::make_pair and direct pair construction produce same result", &::test::tiny_test_case_28> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 80">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 80", jh::test::tiny_test::test<"pod::make_pair and direct pair construction produce same result">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 80"> registration_80{};
}



namespace test {
void tiny_test_case_29() {
    using jh::pod::tuple;
    using jh::pod::make_tuple;

    tuple<int, double> t1{{{7}, {{3.14}, {}}}};
    auto t2 = make_tuple(7, 3.14);

    jh::test::tiny_test::expect(static_cast<bool>((t1 == t2)), "t1 == t2");

}
}
template<>
struct jh::test::tiny_test::test<"pod::tuple construction: nested braces vs make_tuple">
    : jh::test::tiny_test::test_definition<"pod::tuple construction: nested braces vs make_tuple", &::test::tiny_test_case_29> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 81">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 81", jh::test::tiny_test::test<"pod::tuple construction: nested braces vs make_tuple">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 81"> registration_81{};
}



namespace test {
void tiny_test_case_30_body(const int selected_section) {
    using jh::pod::string_view;

    static constexpr char raw[] = "conversion_test";
    constexpr std::uint64_t len = sizeof(raw) - 1;
    const string_view sv{raw, len};

    const std::string_view a = static_cast<std::string_view>(sv); // explicit conversion
    const std::string_view b = sv.to_std();                        // to_std() method

    if (selected_section == 1) {
        jh::test::tiny_test::expect(static_cast<bool>((a.data() == b.data())), "a.data() == b.data()");
        jh::test::tiny_test::expect(static_cast<bool>((a.size() == b.size())), "a.size() == b.size()");
    }

    if (selected_section == 2) {
        jh::test::tiny_test::expect(static_cast<bool>((a == b)), "a == b");
        jh::test::tiny_test::expect(static_cast<bool>((a == "conversion_test")), "a == \"conversion_test\"");
    }

    if (selected_section == 3) {
        const auto ha = std::hash<std::string_view>{}(a);
        const auto hb = std::hash<std::string_view>{}(b);
        jh::test::tiny_test::expect(static_cast<bool>((ha == hb)), "ha == hb");
    }

}
void tiny_test_case_30_section_1() { tiny_test_case_30_body(1); }
void tiny_test_case_30_section_2() { tiny_test_case_30_body(2); }
void tiny_test_case_30_section_3() { tiny_test_case_30_body(3); }
}
template<>
struct jh::test::tiny_test::test<"pod::string_view explicit conversion and to_std() behave identically / Data pointer and size must match">
    : jh::test::tiny_test::test_definition<"pod::string_view explicit conversion and to_std() behave identically / Data pointer and size must match", &::test::tiny_test_case_30_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 82">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 82", jh::test::tiny_test::test<"pod::string_view explicit conversion and to_std() behave identically / Data pointer and size must match">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 82"> registration_82{};
}
template<>
struct jh::test::tiny_test::test<"pod::string_view explicit conversion and to_std() behave identically / Content equality check">
    : jh::test::tiny_test::test_definition<"pod::string_view explicit conversion and to_std() behave identically / Content equality check", &::test::tiny_test_case_30_section_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 83">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 83", jh::test::tiny_test::test<"pod::string_view explicit conversion and to_std() behave identically / Content equality check">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 83"> registration_83{};
}
template<>
struct jh::test::tiny_test::test<"pod::string_view explicit conversion and to_std() behave identically / Both produce same hash via std::hash">
    : jh::test::tiny_test::test_definition<"pod::string_view explicit conversion and to_std() behave identically / Both produce same hash via std::hash", &::test::tiny_test_case_30_section_3> {};
template<>
struct jh::test::tiny_test::session<"test module test_pod_sys case 84">
    : jh::test::tiny_test::session_definition<
          "test module test_pod_sys case 84", jh::test::tiny_test::test<"pod::string_view explicit conversion and to_std() behave identically / Both produce same hash via std::hash">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_pod_sys case 84"> registration_84{};
}


namespace test {
    consteval auto f() {
        constexpr auto t = jh::pod::make_tuple(1, 2, 3);
        return get<0>(t) + get<1>(t) + get<2>(t);
    }
}
