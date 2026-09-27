#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>

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

using value_tuple = std::tuple<int>;
using lvalue_proxy = jh::meta::flatten_proxy<value_tuple &>;
using const_lvalue_proxy = jh::meta::flatten_proxy<const value_tuple &>;
using owned_proxy = jh::meta::flatten_proxy<value_tuple>;
using wrapper_tuple = std::tuple<std::reference_wrapper<int>>;
using wrapper_proxy = jh::meta::flatten_proxy<wrapper_tuple>;
using move_only = std::unique_ptr<int>;
using move_only_reference_proxy = jh::meta::flatten_proxy<std::tuple<move_only &>>;
using move_only_wrapper_proxy = jh::meta::flatten_proxy<std::tuple<std::reference_wrapper<move_only>>>;

static_assert(std::is_convertible_v<lvalue_proxy &, std::tuple<int &>>);
static_assert(std::is_convertible_v<lvalue_proxy &, std::tuple<const int &>>);
static_assert(std::is_convertible_v<lvalue_proxy &, std::tuple<int>>);
static_assert(std::is_convertible_v<const_lvalue_proxy &, std::tuple<const int &>>);
static_assert(!std::is_convertible_v<owned_proxy &, std::tuple<int &&>>);
static_assert(!std::is_convertible_v<owned_proxy &&, std::tuple<int &&>>);
static_assert(!std::is_convertible_v<owned_proxy &&, std::tuple<const int &>>);
static_assert(std::is_convertible_v<owned_proxy &&, std::tuple<int> &&>);
static_assert(!std::is_convertible_v<const owned_proxy &&, std::tuple<const int &&>>);
static_assert(!std::is_convertible_v<const owned_proxy &&, std::tuple<const int &>>);
static_assert(std::is_constructible_v<wrapper_proxy, wrapper_tuple>);
static_assert(std::is_convertible_v<wrapper_proxy &, std::tuple<int &>>);
static_assert(std::is_convertible_v<wrapper_proxy &, std::tuple<int>>);
static_assert(!std::is_convertible_v<wrapper_proxy &, wrapper_tuple>);
static_assert(std::is_convertible_v<wrapper_proxy &&, std::tuple<int>>);
static_assert(std::is_convertible_v<wrapper_proxy &&, std::tuple<int> &&>);
static_assert(std::is_convertible_v<wrapper_proxy &&, std::tuple<int &>>);
static_assert(std::is_convertible_v<wrapper_proxy &&, std::tuple<const int &>>);
static_assert(!std::is_convertible_v<wrapper_proxy &&, std::tuple<int &&>>);
static_assert(std::is_convertible_v<move_only_reference_proxy &&, std::tuple<move_only &>>);
static_assert(!std::is_convertible_v<move_only_reference_proxy &&, std::tuple<move_only>>);
static_assert(!std::is_convertible_v<move_only_reference_proxy &&, std::tuple<move_only &&>>);
static_assert(std::is_convertible_v<move_only_wrapper_proxy &&, std::tuple<move_only &>>);
static_assert(!std::is_convertible_v<move_only_wrapper_proxy &&, std::tuple<move_only>>);
static_assert(!std::is_convertible_v<move_only_wrapper_proxy &&, std::tuple<move_only &&>>);

constexpr bool value_proxy_materialization_is_constexpr() {
    auto mutable_proxy = jh::meta::flatten_proxy{
        std::tuple{std::tuple{11, std::tuple{13}}, std::tuple{17, 19}}
    };
    std::tuple<int, int, int, int> from_lvalue = mutable_proxy;

    const auto const_proxy = jh::meta::flatten_proxy{
        std::tuple{std::tuple{23, std::tuple{24}}, std::tuple{29, std::tuple{31}}}
    };
    std::tuple<int, int, int, int> from_const_lvalue = const_proxy;

    std::tuple<int, int, int, int> from_rvalue = std::move(mutable_proxy);
    std::tuple<int, int, int, int> from_const_rvalue = std::move(const_proxy);

    return from_lvalue == std::tuple{11, 13, 17, 19} &&
           from_const_lvalue == std::tuple{23, 24, 29, 31} &&
           from_rvalue == std::tuple{11, 13, 17, 19} &&
           from_const_rvalue == std::tuple{23, 24, 29, 31};
}
static_assert(value_proxy_materialization_is_constexpr());

constexpr std::tuple<int, int, int> from_temporary_proxy =
    jh::meta::flatten_proxy{std::tuple{std::tuple{37}, std::tuple{41, 43}}};
static_assert(from_temporary_proxy == std::tuple{37, 41, 43});
