#include <concepts>
#include <memory>
#include <string_view>
#include <type_traits>
#include <utility>

#include "jh/pattern_matcher"

struct number_rule {
    static constexpr std::size_t rank = 10;
    std::pair<bool, std::size_t> match(std::string_view) const { return {true, 0}; }
};
struct lower_priority_rule {
    static constexpr std::size_t rank = 20;
    std::pair<bool, std::size_t> match(std::string_view) const { return {true, 0}; }
};
struct count_only_rule {
    static constexpr std::size_t count = 30;
    std::pair<bool, std::size_t> match(std::string_view) const { return {true, 0}; }
};
struct equal_rank_rule {
    static constexpr std::size_t rank = 10;
    std::pair<bool, std::size_t> match(std::string_view) const { return {true, 0}; }
};
struct invalid_rule {};
struct not_movable {
    not_movable() = default;
    not_movable(const not_movable&) = delete;
    not_movable(not_movable&&) = delete;
};
struct nondefault_copy {
    explicit nondefault_copy(int) {}
    nondefault_copy(const nondefault_copy&) = default;
    nondefault_copy(nondefault_copy&&) = default;
    nondefault_copy& operator=(const nondefault_copy&) = default;
    nondefault_copy& operator=(nondefault_copy&&) = default;
};

template<class Matcher>
concept can_match_by_value = requires(const Matcher& matcher) {
    matcher.match(std::string_view{});
};

template<class Matcher>
concept can_match_by_reference = requires(const Matcher& matcher) {
    matcher.match_ref(std::string_view{});
};

template<class Captures>
concept can_resize_captures = requires(Captures& captures) { captures.resize(2); };

template<class T, auto... Rules>
concept valid_pattern_model = requires { typename jh::radix::detail::model<T, Rules...>; };

static_assert(jh::pattern::rule<number_rule>);
static_assert(jh::pattern::rule<lower_priority_rule>);
static_assert(!jh::pattern::rule<count_only_rule>);
static_assert(!jh::pattern::rule<invalid_rule>);
static_assert(jh::radix::detail::unique_rule_ranks<number_rule{}, lower_priority_rule{}>);
static_assert(!jh::radix::detail::unique_rule_ranks<number_rule{}, equal_rank_rule{}>);
static_assert(valid_pattern_model<int, number_rule{}, lower_priority_rule{}>);
static_assert(!valid_pattern_model<int, number_rule{}, equal_rank_rule{}>);
static_assert(!valid_pattern_model<int, count_only_rule{}>);
static_assert(!valid_pattern_model<not_movable>);
static_assert(sizeof(jh::pattern_matcher<int, 512, number_rule{}, lower_priority_rule{}>) > 0);
static_assert(sizeof(jh::pattern_matcher_builder<int, 512, number_rule{}, lower_priority_rule{}>) > 0);
static_assert(std::default_initializable<jh::runtime_arr<std::string_view>>);
static_assert(!std::copy_constructible<jh::runtime_arr<std::string_view>>);
static_assert(!can_resize_captures<jh::runtime_arr<std::string_view>>);
static_assert(!std::copy_constructible<jh::pattern::match_result<int>>);
static_assert(std::movable<jh::pattern::match_result<int>>);
static_assert(std::same_as<decltype(std::declval<jh::pattern_matcher<int>>().match("")),
                           jh::pattern::match_result<int>>);
static_assert(std::same_as<typename jh::pattern::match_result<int>::value_type::first_type,
                           jh::runtime_arr<std::string_view>>);
static_assert(can_match_by_value<jh::pattern_matcher<int>>);
static_assert(!can_match_by_value<jh::pattern_matcher<std::unique_ptr<int>>>);
static_assert(can_match_by_reference<jh::pattern_matcher<std::unique_ptr<int>>>);
static_assert(!can_match_by_value<jh::pattern_matcher<nondefault_copy>>);
static_assert(can_match_by_reference<jh::pattern_matcher<nondefault_copy>>);
