/**
 * @file example_t_str.cpp
 * @brief Recommended usage examples for <code>jh::meta::t_str / TStr</code> via <code>&lt;jh/meta&gt;</code>.
 *
 * <p>
 * This file follows documentation-oriented usage from:
 * </p>
 *
 * <ul>
 *   <li><code>docs/metax/t_str.md</code></li>
 *   <li><code>docs/metax/base64.md</code></li>
 *   <li><code>docs/metax/hash.md</code></li>
 * </ul>
 *
 * <p>
 * The focus is on mainstream, recommended API usage:
 * </p>
 *
 * <ul>
 *   <li>NTTP identity with <code>TStr</code></li>
 *   <li>Accessors and validation helpers</li>
 *   <li>Compile-time transform / concat / substring</li>
 *   <li>Compile-time hashing and byte bridging</li>
 * </ul>
 */

#include <jh/meta>
#include "ensure_output.h"

#if IS_WINDOWS
static EnsureOutput ensure_output_setup;
#endif

#include <iostream>
#include <string>
#include <type_traits>

namespace example {

    template<jh::meta::TStr Name>
    struct field_tag final {
        static constexpr auto value = Name;
    };

    void example_nttp_identity() {
        std::cout << "\n===== t_str NTTP identity =====\n\n";

        using user_id_a = field_tag<"user_id">;
        using user_id_b = field_tag<"user_id">;
        using user_name = field_tag<"user_name">;

        static_assert(std::is_same_v<user_id_a, user_id_b>);
        static_assert(!std::is_same_v<user_id_a, user_name>);

        std::cout << "field_tag<\"user_id\"> value: " << user_id_a::value << "\n";
        std::cout << "same literal -> same type: " << std::boolalpha
                  << std::is_same_v<user_id_a, user_id_b> << "\n";
    }

    void example_accessors_and_validation() {
        std::cout << "\n===== t_str accessors and validation =====\n\n";

        constexpr jh::meta::t_str key{"user_name"};
        constexpr jh::meta::t_str digits{"12345"};
        constexpr jh::meta::t_str number{"-12.5e+3"};
        constexpr jh::meta::t_str hex{"C0FFEE"};
        constexpr jh::meta::t_str b64{"SGVsbG8="};
        constexpr jh::meta::t_str b64url{"SGVsbG8"};

        static_assert(key.size() == 9);
        static_assert(digits.is_digit());
        static_assert(number.is_number());
        static_assert(hex.is_hex());
        static_assert(b64.is_base64());
        static_assert(b64url.is_base64url());
        static_assert(key.is_ascii());
        static_assert(key.is_printable_ascii());
        static_assert(key.is_legal());

#if !JH_GCC_LE_13
        constexpr jh::meta::t_str rel_path{"assets/icons/logo.svg"};
        constexpr jh::meta::t_str rel_parent{"../assets/logo.svg"};
        static_assert(rel_path.is_valid_relative_path());
        static_assert(rel_parent.is_valid_relative_path<true>());
#endif

        std::cout << "key.val()      : " << key.val() << "\n";
        std::cout << "key.size()     : " << key.size() << "\n";
        std::cout << "key.view()     : " << key.view() << "\n";
        std::cout << "key.pod_view() : " << std::string(key.pod_view().data, key.pod_view().size()) << "\n";
        std::cout << "key.str()      : " << key.str() << "\n";
    }

    void example_transform_concat_substring() {
        std::cout << "\n===== t_str transform / concat / sub =====\n\n";

        constexpr auto word = jh::meta::t_str{"AbC"};
        constexpr auto lower = word.to_lower();
        constexpr auto upper = word.to_upper();
        constexpr auto flipped = word.flip_case();

        static_assert(lower == jh::meta::t_str{"abc"});
        static_assert(upper == jh::meta::t_str{"ABC"});
        static_assert(flipped == jh::meta::t_str{"aBc"});

        constexpr auto hello = jh::meta::t_str{"Hello"};
        constexpr auto world = jh::meta::t_str{"World"};
        constexpr auto both = hello + world;
        static_assert(both == jh::meta::t_str{"HelloWorld"});

        constexpr auto left = both.sub<0, 5>();
        constexpr auto right = both.sub<5>();
        static_assert(left == hello);
        static_assert(right == world);
        static_assert(both.sub_view<0, 5>() == std::string_view{"Hello"});
        static_assert(both.sub_pod_view<5>().size() == 5);

        std::cout << "word    : " << word << "\n";
        std::cout << "lower   : " << lower << "\n";
        std::cout << "upper   : " << upper << "\n";
        std::cout << "flipped : " << flipped << "\n";
        std::cout << "concat  : " << both << "\n";
        std::cout << "sub<0,5>: " << left << "\n";
        std::cout << "sub<5>  : " << right << "\n";
    }

    void example_hash_and_bytes() {
        std::cout << "\n===== t_str hash / bytes bridge =====\n\n";

        constexpr auto text = jh::meta::t_str{"Hello"};
        constexpr auto h_default = text.hash();
        constexpr auto h_xx = text.hash(jh::meta::c_hash::xxhash64);
        constexpr auto h_with_null = text.hash(jh::meta::c_hash::fnv1a64, true);

        constexpr auto bytes = text.to_bytes();
        constexpr auto restored = jh::meta::t_str<bytes.size() + 1>::from_bytes(bytes);
        static_assert(restored == text);

        std::cout << "text               : " << text << "\n";
        std::cout << "hash(default)      : " << h_default << "\n";
        std::cout << "hash(xxhash64)     : " << h_xx << "\n";
        std::cout << "hash(include_null) : " << h_with_null << "\n";
        std::cout << "bytes.size()       : " << bytes.size() << "\n";
        std::cout << "restored           : " << restored << "\n";
    }

} // namespace example

int main() {
    example::example_nttp_identity();
    example::example_accessors_and_validation();
    example::example_transform_concat_substring();
    example::example_hash_and_bytes();
    return 0;
}

