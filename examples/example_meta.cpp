/**
 * @file example_meta.cpp
 * @brief Recommended module-level examples for <code>&lt;jh/meta&gt;</code>.
 *
 * <p>
 * This example covers the Meta submodules documented in <code>docs/metax/</code>,
 * emphasizing stable, recommended usage patterns (not edge-case behavior):
 * </p>
 *
 * <ul>
 *   <li><code>char</code> and <code>hash</code> utilities</li>
 *   <li>compile-time Base64 / Base64URL</li>
 *   <li><code>enum_case</code> state-domain matching</li>
 *   <li><code>flatten_proxy</code> and <code>adl_apply</code></li>
 *   <li><code>lookup_map</code> compile-time table</li>
 *   <li><code>variant_adt</code> checks and transforms</li>
 * </ul>
 */

#include <jh/meta>
#include <jh/concepts>
#include <jh/flat_multimap>
#include "ensure_output.h"

#if IS_WINDOWS
static EnsureOutput ensure_output_setup;
#endif

#include <array>
#include <cstddef>
#include <algorithm>
#include <iostream>
#include <optional>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <variant>

namespace example {
    enum class pre_session_rule {
        require_active,
        require_some,
        ignore_actual,
        require_none
    };

    enum class session_state {
        disconnected,
        ready,
        active
    };

    struct session_action_rule {
        const char* accepted;
        const char* rejected;
    };

    [[nodiscard]] constexpr auto expected_session_case(const pre_session_rule pre)
            -> jh::meta::enum_case::value_type<session_state> {
        using C = jh::meta::enum_case;

        switch (pre) {
            case pre_session_rule::require_active:
                return C::of<session_state::active>;
            case pre_session_rule::require_some:
                return C::some<session_state>;
            case pre_session_rule::ignore_actual:
                return C::any<session_state>;
            case pre_session_rule::require_none:
                return C::none<session_state>;
        }

        return C::none<session_state>;
    }

    [[nodiscard]] constexpr const char* execute_session_rule(
            const jh::meta::enum_case::value_type<session_state> expected,
            const session_action_rule& rule,
            const std::optional<session_state>& actual
    ) {
        return expected.matches(actual) ? rule.accepted : rule.rejected;
    }

    void example_char_and_hash() {
        std::cout << "\n===== meta::char / meta::hash =====\n\n";

        static_assert(jh::meta::is_alpha('A'));
        static_assert(jh::meta::is_digit('7'));
        static_assert(jh::meta::is_alnum('Z'));
        static_assert(jh::meta::is_hex_char('F'));
        static_assert(jh::meta::is_base64_core('+'));
        static_assert(jh::meta::is_base64url_core('_'));
        static_assert(jh::meta::is_ascii('x'));
        static_assert(jh::meta::is_printable_ascii(' '));
        static_assert(jh::meta::is_valid_char('X'));
        static_assert(jh::meta::to_upper('a') == 'A');
        static_assert(jh::meta::to_lower('A') == 'a');
        static_assert(jh::meta::flip_case('a') == 'A');

        constexpr char text[] = "meta";
        constexpr std::size_t n = 4;

        constexpr auto h_fnv1a = jh::meta::hash(jh::meta::c_hash::fnv1a64, text, n);
        constexpr auto h_fnv1 = jh::meta::hash(jh::meta::c_hash::fnv1_64, text, n);
        constexpr auto h_djb2 = jh::meta::hash(jh::meta::c_hash::djb2, text, n);
        constexpr auto h_sdbm = jh::meta::hash(jh::meta::c_hash::sdbm, text, n);
        constexpr auto h_murmur = jh::meta::hash(jh::meta::c_hash::murmur64, text, n);
        constexpr auto h_xx = jh::meta::hash(jh::meta::c_hash::xxhash64, text, n);

        std::cout << "text      : " << text << "\n";
        std::cout << "fnv1a64   : " << h_fnv1a << "\n";
        std::cout << "fnv1_64   : " << h_fnv1 << "\n";
        std::cout << "djb2      : " << h_djb2 << "\n";
        std::cout << "sdbm      : " << h_sdbm << "\n";
        std::cout << "murmur64  : " << h_murmur << "\n";
        std::cout << "xxhash64  : " << h_xx << "\n";
    }

    void example_base64_round_trip() {
        std::cout << "\n===== meta::base64 compile-time round-trip =====\n\n";

        constexpr auto raw = jh::meta::t_str{"Hello"}.to_bytes();
        constexpr auto b64 = jh::meta::encode_base64(raw);
        constexpr auto b64url = jh::meta::encode_base64url(raw);
        constexpr auto b64url_padded = jh::meta::encode_base64url(raw, std::true_type{});

        constexpr auto decoded = jh::meta::decode_base64<b64>();
        constexpr auto decoded_url = jh::meta::decode_base64url<b64url>();
        constexpr auto restored = jh::meta::t_str{decoded};
        constexpr auto restored_url = jh::meta::t_str{decoded_url};

        static_assert(restored == jh::meta::t_str{"Hello"});
        static_assert(restored_url == jh::meta::t_str{"Hello"});

        std::cout << "base64          : " << b64 << "\n";
        std::cout << "base64url       : " << b64url << "\n";
        std::cout << "base64url(pad)  : " << b64url_padded << "\n";
        std::cout << "decoded(base64) : " << restored << "\n";
    }

    void example_enum_case() {
        std::cout << "\n===== meta::enum_case =====\n\n";

        using C = jh::meta::enum_case;
        using session_case = C::value_type<session_state>;

        static_assert(C::of<session_state::active>.matches(session_state::active));
        static_assert(C::some<session_state>.matches(std::optional{session_state::ready}));
        static_assert(C::none<session_state>.matches(std::optional<session_state>{}));
        static_assert(C::any<session_state>.matches(std::optional<session_state>{}));

        std::unordered_map<session_case, session_action_rule, jh::hash<session_case>> rule_table{
                {
                        expected_session_case(pre_session_rule::require_active),
                        {"resume workflow", "block until active"}
                },
                {
                        expected_session_case(pre_session_rule::require_some),
                        {"sync current session", "create new session"}
                },
                {
                        expected_session_case(pre_session_rule::ignore_actual),
                        {"always audit", "unreachable"}
                },
                {
                        expected_session_case(pre_session_rule::require_none),
                        {"bootstrap session", "skip bootstrap"}
                }
        };

        jh::flat_multimap<session_case, session_action_rule> sorted_rules;
        sorted_rules.emplace(
                expected_session_case(pre_session_rule::require_active),
                session_action_rule{"resume workflow", "block until active"}
        );
        sorted_rules.emplace(
                expected_session_case(pre_session_rule::require_some),
                session_action_rule{"sync current session", "create new session"}
        );
        sorted_rules.emplace(
                expected_session_case(pre_session_rule::ignore_actual),
                session_action_rule{"always audit", "unreachable"}
        );
        sorted_rules.emplace(
                expected_session_case(pre_session_rule::require_none),
                session_action_rule{"bootstrap session", "skip bootstrap"}
        );

        const std::optional<session_state> actual_active = session_state::active;
        const std::optional<session_state> actual_ready = session_state::ready;
        const std::optional<session_state> actual_none{};

        const auto active_case = expected_session_case(pre_session_rule::require_active);
        const auto some_case = expected_session_case(pre_session_rule::require_some);
        const auto any_case = expected_session_case(pre_session_rule::ignore_actual);
        const auto none_case = expected_session_case(pre_session_rule::require_none);

        const auto& active_rule = rule_table.at(active_case);
        const auto& some_rule = rule_table.at(some_case);
        const auto& any_rule = rule_table.at(any_case);
        const auto none_rule = sorted_rules.find(none_case);

        std::cout << "recommended pattern 1: unordered_map<case, rule, jh::hash<case>>\n";
        std::cout << "pre=require_active, actual=active -> "
                  << execute_session_rule(active_case, active_rule, actual_active) << "\n";
        std::cout << "pre=require_active, actual=ready  -> "
                  << execute_session_rule(active_case, active_rule, actual_ready) << "\n";
        std::cout << "pre=require_some, actual=nullopt  -> "
                  << execute_session_rule(some_case, some_rule, actual_none) << "\n";
        std::cout << "pre=ignore_actual, actual=nullopt -> "
                  << execute_session_rule(any_case, any_rule, actual_none) << "\n";

        std::cout << "\nrecommended pattern 2: jh::flat_multimap<case, rule>\n";
        std::cout << "pre=require_none, actual=nullopt  -> "
                  << execute_session_rule(none_rule->first, none_rule->second, actual_none) << "\n";
    }

    void example_flatten_and_adl_apply() {
        std::cout << "\n===== meta::flatten_proxy / meta::adl_apply =====\n\n";

        constexpr auto nested = std::tuple{
                1,
                std::tuple{2, 3},
                std::pair{4, 5}
        };

        constexpr auto flat = jh::meta::tuple_materialize(nested);
        static_assert(std::tuple_size_v<decltype(flat)> == 5);

        constexpr auto sum = jh::meta::adl_apply(
                [](auto... xs) { return (xs + ... + 0); },
                jh::meta::flatten_proxy{nested}
        );
        static_assert(sum == 15);

        const auto proxy = jh::meta::flatten_proxy{nested};
        const auto [a, b, c, d, e] = proxy; // NOLINT

        std::cout << "flattened values: "
                  << a << ", " << b << ", " << c << ", " << d << ", " << e << "\n";
        std::cout << "sum(adl_apply)  : " << sum << "\n";
    }

    void example_lookup_map() {
        std::cout << "\n===== meta::lookup_map =====\n\n";

        using namespace std::literals;

        constexpr auto http_code = jh::meta::make_lookup_map(
                std::array{
                        std::pair{"ok"sv, 200},
                        std::pair{"created"sv, 201},
                        std::pair{"accepted"sv, 202},
                },
                -1
        );

        static_assert(http_code["ok"] == 200);
        static_assert(http_code["created"] == 201);
        static_assert(http_code[jh::meta::t_str{"accepted"}] == 202);
        static_assert(http_code["missing"] == -1);

        std::cout << "ok       -> " << http_code["ok"] << "\n";
        std::cout << "created  -> " << http_code["created"] << "\n";
        std::cout << "accepted -> " << http_code["accepted"] << "\n";
        std::cout << "missing  -> " << http_code["missing"] << "\n";
    }

    template<typename T, typename Variant>
    struct default_constructible_rule final {
        using _unused [[maybe_unused]] = Variant;
        static constexpr bool value = std::is_default_constructible_v<T>;
    };

    template<typename T>
    struct add_const_ref final {
        using type = std::add_lvalue_reference_t<std::add_const_t<T>>;
    };

    template<typename>
    struct to_size_t final {
        using type = std::size_t;
    };

    template<typename T>
    struct int_or_double final {
        using type = std::conditional_t<std::is_integral_v<T>, int, double>;
    };

    void example_variant_adt() {
        std::cout << "\n===== meta::variant_adt =====\n\n";

        using variant_t = std::variant<int, long, double>;

        static_assert(jh::meta::check_all<default_constructible_rule, variant_t>);

        using transformed = jh::meta::variant_transform_t<variant_t, add_const_ref>;
        static_assert(std::is_same_v<transformed, std::variant<const int &, const long &, const double &>>);

        using collapsed_uniform = jh::meta::variant_collapse_t<variant_t, to_size_t>;
        static_assert(std::is_same_v<collapsed_uniform, std::size_t>);

        using collapsed_mixed = jh::meta::variant_collapse_t<variant_t, int_or_double>;
        static_assert(std::is_void_v<collapsed_mixed>);

        std::cout << "check_all(default_constructible_rule): true\n";
        std::cout << "variant_transform_t -> variant<const T&...>: OK\n";
        std::cout << "variant_collapse_t (uniform) -> size_t: OK\n";
        std::cout << "variant_collapse_t (mixed) -> void: OK\n";
    }

} // namespace example

int main() {
    example::example_char_and_hash();
    example::example_base64_round_trip();
    example::example_enum_case();
    example::example_flatten_and_adl_apply();
    example::example_lookup_map();
    example::example_variant_adt();
    return 0;
}
