#include <type_traits>

#include "jh/pod"

using jh::pod::get;
using jh::pod::make_tuple;
using jh::pod::tuple;

constexpr auto values = make_tuple(10, 2.5, true);
static_assert(get<0>(values) == 10);
static_assert(get<1>(values) == 2.5);
static_assert(get<2>(values));
static_assert(values == make_tuple(10, 2.5, true));
static_assert(values != make_tuple(11, 2.5, true));

consteval int tuple_sum()
{
    constexpr auto value = make_tuple(1, 2, 3);
    return get<0>(value) + get<1>(value) + get<2>(value);
}
static_assert(tuple_sum() == 6);

constexpr auto nested = make_tuple(make_tuple(1, 2), 3);
static_assert(get<0>(nested) == make_tuple(1, 2));
static_assert(get<1>(nested) == 3);
static_assert(get<1>(get<0>(nested)) == 2);

constexpr tuple<int, float> aggregate_tuple{{{7}, {{3.14f}, {}}}};
static_assert(aggregate_tuple == make_tuple(7, 3.14f));
static_assert(std::is_trivial_v<tuple<int, double>>);
static_assert(std::is_standard_layout_v<tuple<int, double>>);
static_assert(jh::pod::pod_like<tuple<int, double>>);

