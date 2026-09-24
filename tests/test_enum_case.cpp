#include <catch2/catch_all.hpp>

#include <algorithm>
#include <compare>
#include <optional>
#include <string_view>
#include <type_traits>
#include <unordered_map>

#include "jh/flat_multimap"
#include "jh/meta"

namespace {
    enum class State {
        Idle = 0,
        Running = 1,
        Done = 2
    };

    enum class PreState {
        RequireRunning,
        RequireSome,
        Ignore,
        RequireNone
    };

    enum Unsafe {
        A,
        B
    };

    using case_t = jh::meta::enum_case::value_type<State>;
    using action_t = std::string_view (*)();

    constexpr std::string_view accept_running() { return "accepted:running"; }
    constexpr std::string_view accept_present() { return "accepted:present"; }
    constexpr std::string_view accept_any() { return "accepted:any"; }
    constexpr std::string_view accept_empty() { return "accepted:empty"; }
    constexpr std::string_view reject_running() { return "rejected:need-running"; }
    constexpr std::string_view reject_present() { return "rejected:need-present"; }
    constexpr std::string_view reject_any() { return "rejected:unreachable"; }
    constexpr std::string_view reject_empty() { return "rejected:need-empty"; }

    struct rule {
        action_t on_accept;
        action_t on_reject;
    };

    [[nodiscard]] constexpr case_t expected_case(PreState pre) {
        using C = jh::meta::enum_case;

        switch (pre) {
            case PreState::RequireRunning:
                return C::of<State::Running>;
            case PreState::RequireSome:
                return C::some<State>;
            case PreState::Ignore:
                return C::any<State>;
            case PreState::RequireNone:
                return C::none<State>;
        }
        return C::none<State>; // NOLINT
    }

    [[nodiscard]] constexpr std::string_view run_rule(
            const case_t expected,
            const rule& current_rule,
            const std::optional<State>& actual
    ) {
        return expected.matches(actual)
               ? current_rule.on_accept()
               : current_rule.on_reject();
    }
}

TEST_CASE("enum_case concepts constrain enum categories", "[enum_case][concept]") {
    STATIC_REQUIRE(jh::concepts::enum_type<State>);
    STATIC_REQUIRE(jh::concepts::enum_type<Unsafe>);
    STATIC_REQUIRE(!jh::concepts::enum_type<int>);

    STATIC_REQUIRE(jh::concepts::scoped_enum<State>);
    STATIC_REQUIRE(!jh::concepts::scoped_enum<Unsafe>);
    STATIC_REQUIRE(!jh::concepts::scoped_enum<int>);
}

TEST_CASE("enum_case matches dynamic enum and optional states", "[enum_case][runtime]") {
    using C = jh::meta::enum_case;

    State actual_state = State::Running;
    std::optional<State> actual_optional = State::Idle;

    REQUIRE(C::of<State::Running>.matches(actual_state));
    REQUIRE(!C::of<State::Done>.matches(actual_state));
    REQUIRE(C::some<State>.matches(actual_state));
    REQUIRE(C::any<State>.matches(actual_state));
    REQUIRE(!C::none<State>.matches(actual_state));

    REQUIRE(!C::of<State::Running>.matches(actual_optional));
    REQUIRE(C::some<State>.matches(actual_optional));
    REQUIRE(C::any<State>.matches(actual_optional));
    REQUIRE(!C::none<State>.matches(actual_optional));

    actual_optional = std::nullopt;

    REQUIRE(!C::of<State::Running>.matches(actual_optional));
    REQUIRE(!C::some<State>.matches(actual_optional));
    REQUIRE(C::any<State>.matches(actual_optional));
    REQUIRE(C::none<State>.matches(actual_optional));
}

TEST_CASE("enum_case structural comparison follows tag then payload", "[enum_case][compare]") {
    using C = jh::meta::enum_case;

    STATIC_REQUIRE(C::of<State::Idle> == C::of<State::Idle>);
    STATIC_REQUIRE(C::of<State::Idle> != C::of<State::Running>);
    STATIC_REQUIRE(C::some<State> == C::some<State>);
    STATIC_REQUIRE(C::any<State> == C::any<State>);
    STATIC_REQUIRE(C::none<State> == C::none<State>);
    STATIC_REQUIRE(C::some<State> != C::any<State>);

    STATIC_REQUIRE((C::of<State::Idle> <=> C::of<State::Running>) == std::strong_ordering::less);
    STATIC_REQUIRE((C::of<State::Running> <=> C::some<State>) == std::strong_ordering::less);
    STATIC_REQUIRE((C::none<State> <=> C::any<State>) == std::strong_ordering::greater);
}

TEST_CASE("enum_case comparison with actual states means matching", "[enum_case][operators]") {
    using C = jh::meta::enum_case;

    State exact = State::Running;
    std::optional<State> present = State::Running;
    std::optional<State> empty{};

    REQUIRE(C::of<State::Running> == exact);
    REQUIRE(exact == C::of<State::Running>);
    REQUIRE(C::of<State::Running> != State::Idle);
    REQUIRE(State::Idle != C::of<State::Running>);

    REQUIRE(C::some<State> == exact);
    REQUIRE(State::Done == C::some<State>);
    REQUIRE(C::none<State> != State::Done);

    REQUIRE(C::of<State::Running> == present);
    REQUIRE(present == C::of<State::Running>);
    REQUIRE(C::of<State::Running> != empty);
    REQUIRE(empty != C::of<State::Running>);

    REQUIRE(C::some<State> == present);
    REQUIRE(present == C::some<State>);
    REQUIRE(C::none<State> == empty);
    REQUIRE(empty == C::none<State>);
    REQUIRE(C::any<State> == empty);
}

TEST_CASE("enum_case works as a hash key through jh::hash", "[enum_case][hash][unordered_map]") {
    using C = jh::meta::enum_case;

    STATIC_REQUIRE(jh::concepts::extended_hashable<case_t>);

    std::unordered_map<case_t, rule, jh::hash<case_t>> table{
            {C::of<State::Running>, {&accept_running, &reject_running}},
            {C::some<State>,        {&accept_present, &reject_present}},
            {C::any<State>,         {&accept_any,     &reject_any}},
            {C::none<State>,        {&accept_empty,   &reject_empty}},
    };

    const auto running_rule = table.find(expected_case(PreState::RequireRunning));
    const auto some_rule = table.find(expected_case(PreState::RequireSome));
    const auto any_rule = table.find(expected_case(PreState::Ignore));
    const auto none_rule = table.find(expected_case(PreState::RequireNone));

    REQUIRE(running_rule != table.end());
    REQUIRE(some_rule != table.end());
    REQUIRE(any_rule != table.end());
    REQUIRE(none_rule != table.end());

    REQUIRE(run_rule(running_rule->first, running_rule->second, std::optional<State>{State::Running}) == "accepted:running");
    REQUIRE(run_rule(running_rule->first, running_rule->second, std::optional<State>{State::Done}) == "rejected:need-running");

    REQUIRE(run_rule(some_rule->first, some_rule->second, std::optional<State>{State::Idle}) == "accepted:present");
    REQUIRE(run_rule(some_rule->first, some_rule->second, std::optional<State>{}) == "rejected:need-present");

    REQUIRE(run_rule(any_rule->first, any_rule->second, std::optional<State>{}) == "accepted:any");
    REQUIRE(run_rule(any_rule->first, any_rule->second, std::optional<State>{State::Done}) == "accepted:any");

    REQUIRE(run_rule(none_rule->first, none_rule->second, std::optional<State>{}) == "accepted:empty");
    REQUIRE(run_rule(none_rule->first, none_rule->second, std::optional<State>{State::Idle}) == "rejected:need-empty");
}

TEST_CASE("enum_case works with jh::flat_multimap ordered lookup", "[enum_case][flat_multimap]") {
    using C = jh::meta::enum_case;

    jh::flat_multimap<case_t, rule> rules;

    rules.emplace(C::none<State>, rule{&accept_empty, &reject_empty});
    rules.emplace(C::of<State::Running>, rule{&accept_running, &reject_running});
    rules.emplace(C::any<State>, rule{&accept_any, &reject_any});
    rules.emplace(C::some<State>, rule{&accept_present, &reject_present});

    const auto running = rules.find(expected_case(PreState::RequireRunning));
    const auto some = rules.find(expected_case(PreState::RequireSome));
    const auto any = rules.find(expected_case(PreState::Ignore));
    const auto none = rules.find(expected_case(PreState::RequireNone));

    REQUIRE(running != rules.end());
    REQUIRE(some != rules.end());
    REQUIRE(any != rules.end());
    REQUIRE(none != rules.end());

    REQUIRE(run_rule(running->first, running->second, std::optional<State>{State::Running}) == "accepted:running");
    REQUIRE(run_rule(some->first, some->second, std::optional<State>{State::Done}) == "accepted:present");
    REQUIRE(run_rule(some->first, some->second, std::optional<State>{}) == "rejected:need-present");
    REQUIRE(run_rule(any->first, any->second, std::optional<State>{}) == "accepted:any");
    REQUIRE(run_rule(none->first, none->second, std::optional<State>{}) == "accepted:empty");
}

TEST_CASE("enum_case value_type aliases the concrete case type", "[enum_case][alias]") {
    STATIC_REQUIRE(std::is_same_v<case_t, jh::meta::enum_case_v<State>>);

    const case_t exact = expected_case(PreState::RequireRunning);
    const case_t fallback = expected_case(PreState::Ignore);

    REQUIRE(exact.matches(State::Running));
    REQUIRE(!exact.matches(State::Done));
    REQUIRE(fallback.matches(std::optional<State>{}));
}
