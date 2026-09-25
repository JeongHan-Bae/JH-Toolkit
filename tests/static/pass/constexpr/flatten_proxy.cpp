#include <tuple>

#include "jh/meta"
#include "jh/pod"

constexpr auto nested = std::tuple{
    std::tuple{1, 2},
    jh::pod::make_tuple(3, 4),
    std::tuple{jh::pod::make_tuple(5, 6), 7},
};
constexpr auto flattened = jh::meta::flatten_proxy{nested};
constexpr auto materialized = jh::meta::tuple_materialize(nested);

static_assert(std::tuple_size_v<decltype(flattened)> == 7);
static_assert(std::tuple_size_v<decltype(materialized)> == 7);

constexpr auto elements_match = []<class Tuple>(const Tuple& value) {
    const auto& [a, b, c, d, e, f, g] = value;
    return a == 1 && b == 2 && c == 3 && d == 4 && e == 5 && f == 6 && g == 7;
};
static_assert(elements_match(flattened));
static_assert(elements_match(materialized));

