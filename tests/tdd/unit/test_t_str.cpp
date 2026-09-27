#define TINY_TEST_MAIN

#include <cstddef>
#include <cstdint>
#include <sstream>
#include <string>

#include "jh/metax/t_str.h"
#include "jh/test/tiny_test/tiny_test.hpp"

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

namespace test {
    void construction_from_literals() {
        constexpr jh::meta::t_str value("abc");
        tiny_test::expect(std::string(value.view()) == "abc", "literal construction keeps content");
    }

    void ostream_operator() {
        constexpr jh::meta::t_str value("ostream_check");
        std::ostringstream output;
        output << value;
        tiny_test::expect(output.str() == "ostream_check", "stream output writes the string content");
    }

    void byte_array_conversion() {
        using array_type = jh::pod::array<std::uint8_t, 5>;

        jh::meta::t_str value("world");
        auto bytes = static_cast<array_type>(value);
        tiny_test::expect(bytes.data[0] == static_cast<std::uint8_t>('w'), "byte conversion keeps the first byte");

        auto restored = jh::meta::t_str{bytes};
        tiny_test::expect(restored.view() == "world", "byte conversion reconstructs the string");
        tiny_test::expect(restored == value, "reconstructed string compares equal to its source");

        jh::meta::t_str mutable_value("abcde");
        auto mutable_bytes = static_cast<array_type>(mutable_value);
        mutable_bytes.data[0] = static_cast<std::uint8_t>('A');
        auto modified = jh::meta::t_str{mutable_bytes};
        tiny_test::expect(modified.view() == "Abcde", "changed bytes are visible after reconstruction");
        tiny_test::expect(modified != mutable_value, "changed bytes produce a different string");
    }

    void substring_operations() {
        constexpr jh::meta::t_str value("hello_world");

        auto prefix = value.sub_view<0, 5>();
        tiny_test::expect(prefix == "hello", "sub_view extracts the prefix");

        auto suffix = value.sub_view<6>();
        tiny_test::expect(suffix == "world", "sub_view extracts the suffix");

        auto standard_view = value.sub_view<0, 5>();
        auto pod_view = value.sub_pod_view<0, 5>().to_std();
        tiny_test::expect(standard_view == pod_view, "standard and POD views agree for a prefix");

        standard_view = value.sub_view<6>();
        pod_view = value.sub_pod_view<6>().to_std();
        tiny_test::expect(standard_view == pod_view, "standard and POD views agree for a suffix");

        jh::meta::t_str runtime_value("run_time");
        auto substring = runtime_value.sub<4, 4>();
        tiny_test::expect(substring.view() == "time", "runtime substring extracts the requested text");

        auto runtime_view = runtime_value.sub_view<4, 4>();
        auto runtime_pod_view = runtime_value.sub_pod_view<4, 4>().to_std();
        tiny_test::expect(runtime_view == "time", "runtime view extracts the requested text");
        tiny_test::expect(runtime_view == runtime_pod_view, "runtime standard and POD views agree");
    }

    void substring_boundary_cases() {
        constexpr jh::meta::t_str value("hello_world");
        auto view = value.sub_view<0, 11>();
        constexpr auto substring = value.sub<0, 11>();
        tiny_test::expect(view == substring.view(), "full-length view agrees with the substring value");
    }

    void default_substring_count() {
        constexpr jh::meta::t_str value("hello_world");

        auto default_view = value.sub_view<6>();
        auto maximum_view = value.sub_view<6, static_cast<std::size_t>(-1)>();
        tiny_test::expect(default_view == maximum_view, "default and maximum counts agree for sub_view");
        tiny_test::expect(default_view == "world", "default sub_view count reaches the string end");

        using namespace jh::pod::literals;
        auto default_pod_view = value.sub_pod_view<6>();
        auto maximum_pod_view = value.sub_pod_view<6, static_cast<std::size_t>(-1)>();
        tiny_test::expect(default_pod_view == maximum_pod_view, "default and maximum counts agree for POD views");
        tiny_test::expect(default_pod_view == "world"_psv, "default POD view count reaches the string end");
    }
}

template<>
struct jh::test::tiny_test::test<"t_str literal construction">
    : jh::test::tiny_test::test_definition<"t_str literal construction", &::test::construction_from_literals> {};

template<>
struct jh::test::tiny_test::test<"t_str ostream output">
    : jh::test::tiny_test::test_definition<"t_str ostream output", &::test::ostream_operator> {};

template<>
struct jh::test::tiny_test::test<"t_str byte array conversion">
    : jh::test::tiny_test::test_definition<"t_str byte array conversion", &::test::byte_array_conversion> {};

template<>
struct jh::test::tiny_test::test<"t_str substring operations">
    : jh::test::tiny_test::test_definition<"t_str substring operations", &::test::substring_operations> {};

template<>
struct jh::test::tiny_test::test<"t_str substring boundaries">
    : jh::test::tiny_test::test_definition<"t_str substring boundaries", &::test::substring_boundary_cases> {};

template<>
struct jh::test::tiny_test::test<"t_str default substring count">
    : jh::test::tiny_test::test_definition<"t_str default substring count", &::test::default_substring_count> {};

template<>
struct jh::test::tiny_test::session<"t_str unit tests">
    : jh::test::tiny_test::session_definition<
          "t_str unit tests",
          jh::test::tiny_test::test<"t_str literal construction">,
          jh::test::tiny_test::test<"t_str ostream output">,
          jh::test::tiny_test::test<"t_str byte array conversion">,
          jh::test::tiny_test::test<"t_str substring operations">,
          jh::test::tiny_test::test<"t_str substring boundaries">,
          jh::test::tiny_test::test<"t_str default substring count">
      > {};

namespace test {
    [[maybe_unused]] const tiny_test::session<"t_str unit tests"> t_str_tests{};
}
