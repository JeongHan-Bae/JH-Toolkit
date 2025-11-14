#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

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

TEST_CASE("Compile-Time Construction with All make_lookup_map Versions") {
    using jh::pod::string_view;
    using namespace std::literals;
    using namespace jh::pod::literals;

    SECTION("Deduction: std::string_view -> pod::string_view") {
        constexpr auto m = jh::meta::make_lookup_map(
                std::array{
                        std::pair{"red"sv,   1},
                        std::pair{"green"sv, 2},
                        std::pair{"blue"sv,  3},
                },
                -1
        );

        STATIC_REQUIRE(m["red"_psv] == 1);
        STATIC_REQUIRE(m[jh::meta::TStr{"green"}] == 2);
        STATIC_REQUIRE(m["blue"sv] == 3);
        STATIC_REQUIRE(m[std::string("purple")] == -1);
        STATIC_REQUIRE(m[string_view::from_literal("yellow")] == -1);
    }

    SECTION("Explicit Hash Version") {
        using Arr = jh::pod::array<char, 6>;

        constexpr auto m = jh::meta::make_lookup_map<brute_hash<>>(
                std::array{
                        std::pair{Arr{"red"},   1},
                        std::pair{Arr{"green"}, 2},
                        std::pair{Arr{"blue"},  3},
                },
                -9
        );

        STATIC_REQUIRE(m[Arr{"red"}]   == 1);
        STATIC_REQUIRE(m[Arr{"green"}] == 2);
        STATIC_REQUIRE(m[Arr{"blue"}]  == 3);
        STATIC_REQUIRE(m[Arr{"zzz"}]   == -9);
    }


    SECTION("CTAD lookup_map(arr)") {
        constexpr auto m =
                jh::meta::lookup_map{
                        std::array{
                                std::pair{string_view::from_literal("A"), 10},
                                std::pair{string_view::from_literal("B"), 20},
                                std::pair{string_view::from_literal("C"), 30},
                        }
                };

        STATIC_REQUIRE(m["A"sv] == 10);
        STATIC_REQUIRE(m["C"] == 30);
    }

    SECTION("CTAD lookup_map(arr, default)") {
        constexpr auto m =
                jh::meta::lookup_map{
                        std::array{
                                std::pair{string_view::from_literal("hello"), 7},
                                std::pair{string_view::from_literal("world"), 9},
                        },
                        -1
                };

        STATIC_REQUIRE(m["hello"] == 7);
        STATIC_REQUIRE(m["xxx"] == -1);
    }

    SECTION("CTAD lookup_map(arr, default, hash)") {
        constexpr auto m =
                jh::meta::lookup_map{
                        std::array{
                                std::pair{jh::pod::array<char, 6>{"red"},   1},
                                std::pair{jh::pod::array<char, 6>{"green"}, 2},
                                std::pair{jh::pod::array<char, 6>{"blue"},  3},
                        },
                        -2,
                        brute_hash<>{}
                };

        STATIC_REQUIRE(m[jh::pod::array<char, 6>{"blue"}] == 3);
        STATIC_REQUIRE(m[jh::pod::array<char, 6>{"qqqqq"}] == -2);
    }
}

TEST_CASE(" Compile-Time Construction with Runtime Verification") {
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
        REQUIRE(color_map[ks] == t.v);
    }
}

TEST_CASE("Runtime Construction (CTAD) with Runtime Verification") {
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
        REQUIRE(m[v] == t.v);
    }
}

TEST_CASE("Simulated output with ostringstream") {
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

    REQUIRE(out.str() ==
            "red -> 1\n"
            "green -> 2\n"
            "blue -> 3\n"
            "purple -> -1\n"
    );
}

TEST_CASE("POD array key with brute hash") {
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

    REQUIRE(m[array<char, 6>{"red"}] == 1);
    REQUIRE(m[array<char, 6>{"green"}] == 2);
    REQUIRE(m[array<char, 6>{"blue"}] == 3);
    REQUIRE(m[array<char, 6>{"xxxxx"}] == -2);
}
