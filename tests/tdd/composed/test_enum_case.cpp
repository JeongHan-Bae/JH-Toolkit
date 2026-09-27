#define TINY_TEST_MAIN
#include "jh/test/tiny_test/tiny_test.hpp"

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

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


namespace test {
void tiny_test_case_1() {
    using C = jh::meta::enum_case;

    State actual_state = State::Running;
    std::optional<State> actual_optional = State::Idle;

    jh::test::tiny_test::expect(static_cast<bool>((C::of<State::Running>.matches(actual_state))), "C::of<State::Running>.matches(actual_state)");
    jh::test::tiny_test::expect(static_cast<bool>((!C::of<State::Done>.matches(actual_state))), "!C::of<State::Done>.matches(actual_state)");
    jh::test::tiny_test::expect(static_cast<bool>((C::some<State>.matches(actual_state))), "C::some<State>.matches(actual_state)");
    jh::test::tiny_test::expect(static_cast<bool>((C::any<State>.matches(actual_state))), "C::any<State>.matches(actual_state)");
    jh::test::tiny_test::expect(static_cast<bool>((!C::none<State>.matches(actual_state))), "!C::none<State>.matches(actual_state)");

    jh::test::tiny_test::expect(static_cast<bool>((!C::of<State::Running>.matches(actual_optional))), "!C::of<State::Running>.matches(actual_optional)");
    jh::test::tiny_test::expect(static_cast<bool>((C::some<State>.matches(actual_optional))), "C::some<State>.matches(actual_optional)");
    jh::test::tiny_test::expect(static_cast<bool>((C::any<State>.matches(actual_optional))), "C::any<State>.matches(actual_optional)");
    jh::test::tiny_test::expect(static_cast<bool>((!C::none<State>.matches(actual_optional))), "!C::none<State>.matches(actual_optional)");

    actual_optional = std::nullopt;

    jh::test::tiny_test::expect(static_cast<bool>((!C::of<State::Running>.matches(actual_optional))), "!C::of<State::Running>.matches(actual_optional)");
    jh::test::tiny_test::expect(static_cast<bool>((!C::some<State>.matches(actual_optional))), "!C::some<State>.matches(actual_optional)");
    jh::test::tiny_test::expect(static_cast<bool>((C::any<State>.matches(actual_optional))), "C::any<State>.matches(actual_optional)");
    jh::test::tiny_test::expect(static_cast<bool>((C::none<State>.matches(actual_optional))), "C::none<State>.matches(actual_optional)");

}
}
template<>
struct jh::test::tiny_test::test<"enum_case matches dynamic enum and optional states">
    : jh::test::tiny_test::test_definition<"enum_case matches dynamic enum and optional states", &::test::tiny_test_case_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_enum_case case 1">
    : jh::test::tiny_test::session_definition<
          "test module test_enum_case case 1", jh::test::tiny_test::test<"enum_case matches dynamic enum and optional states">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_enum_case case 1"> registration_1{};
}



namespace test {
void tiny_test_case_2() {
    using C = jh::meta::enum_case;

    State exact = State::Running;
    std::optional<State> present = State::Running;
    std::optional<State> empty{};

    jh::test::tiny_test::expect(static_cast<bool>((C::of<State::Running> == exact)), "C::of<State::Running> == exact");
    jh::test::tiny_test::expect(static_cast<bool>((exact == C::of<State::Running>)), "exact == C::of<State::Running>");
    jh::test::tiny_test::expect(static_cast<bool>((C::of<State::Running> != State::Idle)), "C::of<State::Running> != State::Idle");
    jh::test::tiny_test::expect(static_cast<bool>((State::Idle != C::of<State::Running>)), "State::Idle != C::of<State::Running>");

    jh::test::tiny_test::expect(static_cast<bool>((C::some<State> == exact)), "C::some<State> == exact");
    jh::test::tiny_test::expect(static_cast<bool>((State::Done == C::some<State>)), "State::Done == C::some<State>");
    jh::test::tiny_test::expect(static_cast<bool>((C::none<State> != State::Done)), "C::none<State> != State::Done");

    jh::test::tiny_test::expect(static_cast<bool>((C::of<State::Running> == present)), "C::of<State::Running> == present");
    jh::test::tiny_test::expect(static_cast<bool>((present == C::of<State::Running>)), "present == C::of<State::Running>");
    jh::test::tiny_test::expect(static_cast<bool>((C::of<State::Running> != empty)), "C::of<State::Running> != empty");
    jh::test::tiny_test::expect(static_cast<bool>((empty != C::of<State::Running>)), "empty != C::of<State::Running>");

    jh::test::tiny_test::expect(static_cast<bool>((C::some<State> == present)), "C::some<State> == present");
    jh::test::tiny_test::expect(static_cast<bool>((present == C::some<State>)), "present == C::some<State>");
    jh::test::tiny_test::expect(static_cast<bool>((C::none<State> == empty)), "C::none<State> == empty");
    jh::test::tiny_test::expect(static_cast<bool>((empty == C::none<State>)), "empty == C::none<State>");
    jh::test::tiny_test::expect(static_cast<bool>((C::any<State> == empty)), "C::any<State> == empty");

}
}
template<>
struct jh::test::tiny_test::test<"enum_case comparison with actual states means matching">
    : jh::test::tiny_test::test_definition<"enum_case comparison with actual states means matching", &::test::tiny_test_case_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_enum_case case 2">
    : jh::test::tiny_test::session_definition<
          "test module test_enum_case case 2", jh::test::tiny_test::test<"enum_case comparison with actual states means matching">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_enum_case case 2"> registration_2{};
}



namespace test {
void tiny_test_case_3() {
    using C = jh::meta::enum_case;

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

    jh::test::tiny_test::expect(static_cast<bool>((running_rule != table.end())), "running_rule != table.end()");
    jh::test::tiny_test::expect(static_cast<bool>((some_rule != table.end())), "some_rule != table.end()");
    jh::test::tiny_test::expect(static_cast<bool>((any_rule != table.end())), "any_rule != table.end()");
    jh::test::tiny_test::expect(static_cast<bool>((none_rule != table.end())), "none_rule != table.end()");

    jh::test::tiny_test::expect(static_cast<bool>((run_rule(running_rule->first, running_rule->second, std::optional<State>{State::Running}) == "accepted:running")), "run_rule(running_rule->first, running_rule->second, std::optional<State>{State::Running}) == \"accepted:running\"");
    jh::test::tiny_test::expect(static_cast<bool>((run_rule(running_rule->first, running_rule->second, std::optional<State>{State::Done}) == "rejected:need-running")), "run_rule(running_rule->first, running_rule->second, std::optional<State>{State::Done}) == \"rejected:need-running\"");

    jh::test::tiny_test::expect(static_cast<bool>((run_rule(some_rule->first, some_rule->second, std::optional<State>{State::Idle}) == "accepted:present")), "run_rule(some_rule->first, some_rule->second, std::optional<State>{State::Idle}) == \"accepted:present\"");
    jh::test::tiny_test::expect(static_cast<bool>((run_rule(some_rule->first, some_rule->second, std::optional<State>{}) == "rejected:need-present")), "run_rule(some_rule->first, some_rule->second, std::optional<State>{}) == \"rejected:need-present\"");

    jh::test::tiny_test::expect(static_cast<bool>((run_rule(any_rule->first, any_rule->second, std::optional<State>{}) == "accepted:any")), "run_rule(any_rule->first, any_rule->second, std::optional<State>{}) == \"accepted:any\"");
    jh::test::tiny_test::expect(static_cast<bool>((run_rule(any_rule->first, any_rule->second, std::optional<State>{State::Done}) == "accepted:any")), "run_rule(any_rule->first, any_rule->second, std::optional<State>{State::Done}) == \"accepted:any\"");

    jh::test::tiny_test::expect(static_cast<bool>((run_rule(none_rule->first, none_rule->second, std::optional<State>{}) == "accepted:empty")), "run_rule(none_rule->first, none_rule->second, std::optional<State>{}) == \"accepted:empty\"");
    jh::test::tiny_test::expect(static_cast<bool>((run_rule(none_rule->first, none_rule->second, std::optional<State>{State::Idle}) == "rejected:need-empty")), "run_rule(none_rule->first, none_rule->second, std::optional<State>{State::Idle}) == \"rejected:need-empty\"");

}
}
template<>
struct jh::test::tiny_test::test<"enum_case works as a hash key through jh::hash">
    : jh::test::tiny_test::test_definition<"enum_case works as a hash key through jh::hash", &::test::tiny_test_case_3> {};
template<>
struct jh::test::tiny_test::session<"test module test_enum_case case 3">
    : jh::test::tiny_test::session_definition<
          "test module test_enum_case case 3", jh::test::tiny_test::test<"enum_case works as a hash key through jh::hash">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_enum_case case 3"> registration_3{};
}



namespace test {
void tiny_test_case_4() {
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

    jh::test::tiny_test::expect(static_cast<bool>((running != rules.end())), "running != rules.end()");
    jh::test::tiny_test::expect(static_cast<bool>((some != rules.end())), "some != rules.end()");
    jh::test::tiny_test::expect(static_cast<bool>((any != rules.end())), "any != rules.end()");
    jh::test::tiny_test::expect(static_cast<bool>((none != rules.end())), "none != rules.end()");

    jh::test::tiny_test::expect(static_cast<bool>((run_rule(running->first, running->second, std::optional<State>{State::Running}) == "accepted:running")), "run_rule(running->first, running->second, std::optional<State>{State::Running}) == \"accepted:running\"");
    jh::test::tiny_test::expect(static_cast<bool>((run_rule(some->first, some->second, std::optional<State>{State::Done}) == "accepted:present")), "run_rule(some->first, some->second, std::optional<State>{State::Done}) == \"accepted:present\"");
    jh::test::tiny_test::expect(static_cast<bool>((run_rule(some->first, some->second, std::optional<State>{}) == "rejected:need-present")), "run_rule(some->first, some->second, std::optional<State>{}) == \"rejected:need-present\"");
    jh::test::tiny_test::expect(static_cast<bool>((run_rule(any->first, any->second, std::optional<State>{}) == "accepted:any")), "run_rule(any->first, any->second, std::optional<State>{}) == \"accepted:any\"");
    jh::test::tiny_test::expect(static_cast<bool>((run_rule(none->first, none->second, std::optional<State>{}) == "accepted:empty")), "run_rule(none->first, none->second, std::optional<State>{}) == \"accepted:empty\"");

}
}
template<>
struct jh::test::tiny_test::test<"enum_case works with jh::flat_multimap ordered lookup">
    : jh::test::tiny_test::test_definition<"enum_case works with jh::flat_multimap ordered lookup", &::test::tiny_test_case_4> {};
template<>
struct jh::test::tiny_test::session<"test module test_enum_case case 4">
    : jh::test::tiny_test::session_definition<
          "test module test_enum_case case 4", jh::test::tiny_test::test<"enum_case works with jh::flat_multimap ordered lookup">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_enum_case case 4"> registration_4{};
}



namespace test {
void tiny_test_case_5() {
    const case_t exact = expected_case(PreState::RequireRunning);
    const case_t fallback = expected_case(PreState::Ignore);

    jh::test::tiny_test::expect(static_cast<bool>((exact.matches(State::Running))), "exact.matches(State::Running)");
    jh::test::tiny_test::expect(static_cast<bool>((!exact.matches(State::Done))), "!exact.matches(State::Done)");
    jh::test::tiny_test::expect(static_cast<bool>((fallback.matches(std::optional<State>{}))), "fallback.matches(std::optional<State>{})");

}
}
template<>
struct jh::test::tiny_test::test<"enum_case value_type aliases the concrete case type">
    : jh::test::tiny_test::test_definition<"enum_case value_type aliases the concrete case type", &::test::tiny_test_case_5> {};
template<>
struct jh::test::tiny_test::session<"test module test_enum_case case 5">
    : jh::test::tiny_test::session_definition<
          "test module test_enum_case case 5", jh::test::tiny_test::test<"enum_case value_type aliases the concrete case type">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_enum_case case 5"> registration_5{};
}

