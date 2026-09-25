#include <array>
#include <bit>
#include <cstdint>
#include <string>
#include <string_view>

#include "jh/metax/hash.h"
#include "jh/metax/lookup_map.h"
#include "jh/metax/t_str.h"
#include "jh/pod"

template<jh::meta::c_hash Algo = jh::meta::c_hash::fnv1a64>
struct brute_hash {
    template<jh::pod::pod_like T>
    constexpr std::size_t operator()(const T& value) const noexcept {
        auto bytes = std::bit_cast<std::array<std::uint8_t, sizeof(T)>>(value);
        return jh::meta::hash(Algo, bytes.data(), bytes.size());
    }
};

using jh::pod::string_view;
using namespace std::literals;
using namespace jh::pod::literals;

constexpr auto standard_map = jh::meta::make_lookup_map(
    std::array{
        std::pair{"red"sv, 1},
        std::pair{"green"sv, 2},
        std::pair{"blue"sv, 3},
    },
    -1);
static_assert(standard_map["red"_psv] == 1);
static_assert(standard_map[jh::meta::TStr{"green"}] == 2);
static_assert(standard_map["blue"sv] == 3);
static_assert(standard_map[std::string{"purple"}] == -1);
static_assert(standard_map[string_view::from_literal("yellow")] == -1);

using ArrayKey = jh::pod::array<char, 6>;
constexpr auto explicit_hash_map = jh::meta::make_lookup_map<brute_hash<>>(
    std::array{
        std::pair{ArrayKey{"red"}, 1},
        std::pair{ArrayKey{"green"}, 2},
        std::pair{ArrayKey{"blue"}, 3},
    },
    -9);
static_assert(explicit_hash_map[ArrayKey{"red"}] == 1);
static_assert(explicit_hash_map[ArrayKey{"green"}] == 2);
static_assert(explicit_hash_map[ArrayKey{"blue"}] == 3);
static_assert(explicit_hash_map[ArrayKey{"zzz"}] == -9);

constexpr auto deduced_map = jh::meta::lookup_map{
    std::array{
        std::pair{string_view::from_literal("A"), 10},
        std::pair{string_view::from_literal("B"), 20},
        std::pair{string_view::from_literal("C"), 30},
    }};
static_assert(deduced_map["A"sv] == 10);
static_assert(deduced_map["C"] == 30);

constexpr auto default_map = jh::meta::lookup_map{
    std::array{
        std::pair{string_view::from_literal("hello"), 7},
        std::pair{string_view::from_literal("world"), 9},
    },
    -1};
static_assert(default_map["hello"] == 7);
static_assert(default_map["xxx"] == -1);

constexpr auto custom_hash_map = jh::meta::lookup_map{
    std::array{
        std::pair{ArrayKey{"red"}, 1},
        std::pair{ArrayKey{"green"}, 2},
        std::pair{ArrayKey{"blue"}, 3},
    },
    -2,
    brute_hash<>{}};
static_assert(custom_hash_map[ArrayKey{"blue"}] == 3);
static_assert(custom_hash_map[ArrayKey{"qqqqq"}] == -2);

