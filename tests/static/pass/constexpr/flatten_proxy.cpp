#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>

#include "jh/meta"
#include "jh/pod"

namespace throwing_access_test {
    struct tuple { int value; };

    template<std::size_t I, typename T>
    requires std::is_same_v<std::remove_cvref_t<T>, tuple>
    constexpr decltype(auto) get(T &&source) noexcept(false) {
        static_assert(I == 0);
        return (std::forward<T>(source).value);
    }
}

template<>
struct std::tuple_size<throwing_access_test::tuple>
        : std::integral_constant<std::size_t, 1> {};

template<>
struct std::tuple_element<0, throwing_access_test::tuple> { using type = int; };

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

struct throwing_copy {
    throwing_copy() = default;
    throwing_copy(const throwing_copy&) noexcept(false) {}
};

using nested_values = std::tuple<std::tuple<throwing_copy, int>>;
using nested_proxy = jh::meta::flatten_proxy<nested_values &>;
static_assert(!noexcept(std::declval<const nested_proxy &>().get<0>()));
static_assert(noexcept(std::declval<const nested_proxy &>().get<1>()));
static_assert(!noexcept(jh::meta::tuple_materialize(std::declval<const nested_values &>())));
static_assert(std::is_same_v<decltype(std::declval<nested_proxy &>().get<1>()), int>);
static_assert(std::is_same_v<decltype(std::declval<nested_proxy &>().get_ref<1>()), int &>);
static_assert(std::is_same_v<decltype(std::declval<const nested_proxy &>().get_ref<1>()), int &>);

using owned_nested_proxy = jh::meta::flatten_proxy<nested_values>;
static_assert(std::is_same_v<decltype(std::declval<const owned_nested_proxy &>().get_ref<1>()), const int &>);

template<typename Proxy, std::size_t I>
concept has_lvalue_get_ref = requires(Proxy &proxy) { proxy.template get_ref<I>(); };

template<typename Proxy, std::size_t I>
concept has_rvalue_get_ref = requires(Proxy &&proxy) {
    std::move(proxy).template get_ref<I>();
};

static_assert(!has_lvalue_get_ref<decltype(flattened), 2>);
static_assert(!has_rvalue_get_ref<owned_nested_proxy, 0>);

using throwing_access_source = std::tuple<throwing_access_test::tuple, int>;
using throwing_access_proxy = jh::meta::flatten_proxy<throwing_access_source &>;
static_assert(!noexcept(std::declval<const throwing_access_proxy &>().get<0>()));
static_assert(noexcept(std::declval<const throwing_access_proxy &>().get<1>()));
static_assert(!noexcept(jh::meta::tuple_materialize(std::declval<const throwing_access_source &>())));
static_assert(!noexcept(std::declval<const throwing_access_proxy &>().get_ref<0>()));
static_assert(!noexcept(jh::meta::get<0>(std::declval<const throwing_access_proxy &>())));

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
