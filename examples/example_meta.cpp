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
 *   <li><code>flatten_proxy</code> and <code>adl_apply</code></li>
 *   <li><code>lookup_map</code> compile-time table</li>
 *   <li><code>variant_adt</code> checks and transforms</li>
 * </ul>
 */

#include <jh/meta>
#include "ensure_output.h"

#if IS_WINDOWS
static EnsureOutput ensure_output_setup;
#endif

#include <array>
#include <iostream>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <variant>

namespace example {

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
        constexpr std::uint64_t n = 4;

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
        constexpr auto restored = jh::meta::t_str<decoded.size() + 1>::from_bytes(decoded);
        constexpr auto restored_url = jh::meta::t_str<decoded_url.size() + 1>::from_bytes(decoded_url);

        static_assert(restored == jh::meta::t_str{"Hello"});
        static_assert(restored_url == jh::meta::t_str{"Hello"});

        std::cout << "base64          : " << b64 << "\n";
        std::cout << "base64url       : " << b64url << "\n";
        std::cout << "base64url(pad)  : " << b64url_padded << "\n";
        std::cout << "decoded(base64) : " << restored << "\n";
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
    example::example_flatten_and_adl_apply();
    example::example_lookup_map();
    example::example_variant_adt();
    return 0;
}
