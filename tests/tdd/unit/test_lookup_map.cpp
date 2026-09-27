#define TINY_TEST_MAIN
#include "jh/test/tiny_test/tiny_test.hpp"

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

#include <array>
#include <string>
#include <string_view>
#include <sstream>
#include <type_traits>

#include "jh/metax/lookup_map.h"
#include "jh/pods/string_view.h"
#include "jh/pods/array.h"
#include "jh/metax/t_str.h"
#include "jh/metax/hash.h"


/// @brief Compile-time Hash Wrapper for POD
template<jh::meta::c_hash Algo = jh::meta::c_hash::fnv1a64>
struct brute_hash {
    template<jh::pod::pod_like T>
    constexpr size_t operator()(const T &t) const noexcept {
        auto bytes = std::bit_cast<std::array<uint8_t, sizeof(T)>>(t);
        return jh::meta::hash(Algo, bytes.data(), bytes.size());
    }
};


namespace test {
void tiny_test_case_1() {
    using namespace std::literals;

    constexpr auto map = jh::meta::make_lookup_map(
        std::array{
            std::pair{"red"sv, 1},
            std::pair{"green"sv, 2},
            std::pair{"blue"sv, 3},
        },
        -1);
    jh::test::tiny_test::expect(static_cast<bool>((map[jh::immutable_str("blue")] == 3)), "map[jh::immutable_str(\"blue\")] == 3");

}
}
template<>
struct jh::test::tiny_test::test<"lookup_map accepts immutable_str runtime keys">
    : jh::test::tiny_test::test_definition<"lookup_map accepts immutable_str runtime keys", &::test::tiny_test_case_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_lookup_map case 1">
    : jh::test::tiny_test::session_definition<
          "test module test_lookup_map case 1", jh::test::tiny_test::test<"lookup_map accepts immutable_str runtime keys">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_lookup_map case 1"> registration_1{};
}



namespace test {
void tiny_test_case_2() {
    using namespace std::literals;
    using jh::pod::string_view;
    using namespace jh::pod::literals;

    constexpr auto color_map = jh::meta::make_lookup_map(
            std::array{
                    std::pair{"red"sv,   1},
                    std::pair{"green"sv, 2},
                    std::pair{"blue"sv,  3},
            },
            -1
    );

    struct T { const char* k; int v; };
    T list[] = {
            {"red", 1},
            {"green", 2},
            {"blue", 3},
            {"purple", -1}
    };

    for (auto &t : list) {
        jh::pod::string_view ks{t.k, std::strlen(t.k)};
        jh::test::tiny_test::expect(static_cast<bool>((color_map[ks] == t.v)), "color_map[ks] == t.v");
    }

}
}
template<>
struct jh::test::tiny_test::test<" Compile-Time Construction with Runtime Verification">
    : jh::test::tiny_test::test_definition<" Compile-Time Construction with Runtime Verification", &::test::tiny_test_case_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_lookup_map case 2">
    : jh::test::tiny_test::session_definition<
          "test module test_lookup_map case 2", jh::test::tiny_test::test<" Compile-Time Construction with Runtime Verification">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_lookup_map case 2"> registration_2{};
}



namespace test {
void tiny_test_case_3() {
    using namespace std::literals;

    auto m = jh::meta::lookup_map{
            std::array{
                    std::pair{"red"sv,   1},
                    std::pair{"green"sv, 2},
                    std::pair{"blue"sv,  3},
            },
            -1
    };

    struct T { std::string k; int v; };
    T list[] = {
            {"red", 1},
            {"green", 2},
            {"blue", 3},
            {"purple", -1}
    };

    for (auto &t : list) {
        std::string_view v = t.k;
        jh::test::tiny_test::expect(static_cast<bool>((m[v] == t.v)), "m[v] == t.v");
    }

}
}
template<>
struct jh::test::tiny_test::test<"Runtime Construction (CTAD) with Runtime Verification">
    : jh::test::tiny_test::test_definition<"Runtime Construction (CTAD) with Runtime Verification", &::test::tiny_test_case_3> {};
template<>
struct jh::test::tiny_test::session<"test module test_lookup_map case 3">
    : jh::test::tiny_test::session_definition<
          "test module test_lookup_map case 3", jh::test::tiny_test::test<"Runtime Construction (CTAD) with Runtime Verification">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_lookup_map case 3"> registration_3{};
}



namespace test {
void tiny_test_case_4() {
    using namespace std::literals;

    constexpr auto map = jh::meta::make_lookup_map(
        std::array{
            std::pair{"red"sv, 1},
            std::pair{"green"sv, 2},
            std::pair{"blue"sv, 3},
        },
        -1);

    const std::string purple{"purple"};
    jh::test::tiny_test::expect(static_cast<bool>((map[jh::meta::TStr{"green"}] == 2)), "map[jh::meta::TStr{\"green\"}] == 2");
    jh::test::tiny_test::expect(static_cast<bool>((map[purple] == -1)), "map[purple] == -1");

}
}
template<>
struct jh::test::tiny_test::test<"lookup_map accepts runtime TStr and std::string keys">
    : jh::test::tiny_test::test_definition<"lookup_map accepts runtime TStr and std::string keys", &::test::tiny_test_case_4> {};
template<>
struct jh::test::tiny_test::session<"test module test_lookup_map case 4">
    : jh::test::tiny_test::session_definition<
          "test module test_lookup_map case 4", jh::test::tiny_test::test<"lookup_map accepts runtime TStr and std::string keys">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_lookup_map case 4"> registration_4{};
}



namespace test {
void tiny_test_case_5() {
    using namespace std::literals;

    constexpr auto m = jh::meta::make_lookup_map(
            std::array{
                    std::pair{"red"sv,   1},
                    std::pair{"green"sv, 2},
                    std::pair{"blue"sv,  3}
            },
            -1
    );

    std::ostringstream out;

    struct T { const char* k; int v; };
    T list[] = {
            {"red", 1},
            {"green", 2},
            {"blue", 3},
            {"purple", -1}
    };

    for (auto &t : list) {
        int value = m[std::string_view(t.k)];
        out << t.k << " -> " << value << "\n";
    }

    jh::test::tiny_test::expect(static_cast<bool>((out.str() ==
            "red -> 1\n"
            "green -> 2\n"
            "blue -> 3\n"
            "purple -> -1\n")), "out.str() ==\n            \"red -> 1\\n\"\n            \"green -> 2\\n\"\n            \"blue -> 3\\n\"\n            \"purple -> -1\\n\"");

}
}
template<>
struct jh::test::tiny_test::test<"Simulated output with ostringstream">
    : jh::test::tiny_test::test_definition<"Simulated output with ostringstream", &::test::tiny_test_case_5> {};
template<>
struct jh::test::tiny_test::session<"test module test_lookup_map case 5">
    : jh::test::tiny_test::session_definition<
          "test module test_lookup_map case 5", jh::test::tiny_test::test<"Simulated output with ostringstream">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_lookup_map case 5"> registration_5{};
}



namespace test {
void tiny_test_case_6() {
    using jh::pod::array;

    constexpr auto m = jh::meta::lookup_map(
            std::array{
                    std::pair{array<char, 6>{"red"},   1},
                    std::pair{array<char, 6>{"green"}, 2},
                    std::pair{array<char, 6>{"blue"},  3},
            },
            -2,
            brute_hash<>{}
    );

    jh::test::tiny_test::expect(static_cast<bool>((m[array<char, 6>{"red"}] == 1)), "m[array<char, 6>{\"red\"}] == 1");
    jh::test::tiny_test::expect(static_cast<bool>((m[array<char, 6>{"green"}] == 2)), "m[array<char, 6>{\"green\"}] == 2");
    jh::test::tiny_test::expect(static_cast<bool>((m[array<char, 6>{"blue"}] == 3)), "m[array<char, 6>{\"blue\"}] == 3");
    jh::test::tiny_test::expect(static_cast<bool>((m[array<char, 6>{"xxxxx"}] == -2)), "m[array<char, 6>{\"xxxxx\"}] == -2");

}
}
template<>
struct jh::test::tiny_test::test<"POD array key with brute hash">
    : jh::test::tiny_test::test_definition<"POD array key with brute hash", &::test::tiny_test_case_6> {};
template<>
struct jh::test::tiny_test::session<"test module test_lookup_map case 6">
    : jh::test::tiny_test::session_definition<
          "test module test_lookup_map case 6", jh::test::tiny_test::test<"POD array key with brute hash">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_lookup_map case 6"> registration_6{};
}

