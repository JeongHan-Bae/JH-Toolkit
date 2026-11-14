#include <concepts>
#include <type_traits>
#include <utility>

#include "jh/generator"
#include "jh/typed"

template<class, template<class> class, class = void>
struct is_detected : std::false_type {};

template<class T, template<class> class Op>
struct is_detected<T, Op, std::void_t<Op<T>>> : std::true_type {};

template<class T, template<class> class Op>
inline constexpr bool is_detected_v = is_detected<T, Op>::value;

template<class T>
using has_begin_t = decltype(std::declval<T>().begin());
template<class T>
using has_end_t = decltype(std::declval<T>().end());

namespace static_generator {
    jh::async::generator<int> range(int end)
    {
        for (int value = 0; value < end; ++value) co_yield value;
    }

    jh::async::generator<int, int> countdown(int start)
    {
        int step = 1;
        while (start > 0) {
            volatile int next_step = step;
            step = co_await next_step;
            start -= step;
            co_yield start;
        }
    }
}

using Countdown = decltype(static_generator::countdown(10));
using Range = decltype(static_generator::range(10));

static_assert(!is_detected_v<Countdown, has_begin_t>);
static_assert(!is_detected_v<Countdown, has_end_t>);
static_assert(is_detected_v<Range, has_begin_t>);
static_assert(is_detected_v<Range, has_end_t>);
static_assert(std::same_as<jh::async::generator<int, double>::iterator,
                           jh::concepts::iterator_t<jh::async::generator<int, double>>>);
static_assert(std::same_as<jh::async::generator<int, jh::typed::monostate>::iterator,
                           jh::concepts::iterator_t<jh::async::generator<int>>>);
static_assert(!jh::concepts::sequence<jh::async::generator<int>>);
static_assert(jh::concepts::is_iterator<jh::async::generator<int>::iterator>);
static_assert(jh::concepts::input_iterator<jh::async::generator<int>::iterator>);
static_assert(jh::concepts::sequence<jh::async::generator_range<int>>);

struct non_assignable_value {
    const int value{};
    non_assignable_value(const non_assignable_value&) = default;
    non_assignable_value(non_assignable_value&&) = default;
    non_assignable_value& operator=(const non_assignable_value&) = delete;
    non_assignable_value& operator=(non_assignable_value&&) = delete;
};

struct copy_only {
    copy_only(const copy_only&) = default;
    copy_only(copy_only&&) = delete;
};

template<class T>
concept generator_range_name_available = requires { typename jh::async::generator_range<T>; };

static_assert(std::copy_constructible<non_assignable_value>);
static_assert(generator_range_name_available<non_assignable_value>);
static_assert(!generator_range_name_available<std::unique_ptr<int>>);
static_assert(!generator_range_name_available<copy_only>);

