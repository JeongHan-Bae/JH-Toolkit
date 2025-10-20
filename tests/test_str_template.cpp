#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include "jh/str_template.h"

using namespace jh::str_template;

namespace test {
    template<CStr S>
    struct tag {};
}

/**
 * @test Basic compile-time property checks for <code>cstr</code>.
 *
 * <ul>
 *   <li>Validate <b>size</b>, <b>is_alpha</b>, <b>is_digit</b>, <b>ASCII</b> checks.</li>
 *   <li>Ensure <code>view()</code> provides correct <code>std::string_view</code>.</li>
 * </ul>
 */
TEST_CASE("cstr basic constexpr properties") {
    constexpr cstr hello("hello");
    STATIC_REQUIRE(hello.size() == 5);
    STATIC_REQUIRE(hello.is_alpha());
    STATIC_REQUIRE_FALSE(hello.is_digit());
    STATIC_REQUIRE(hello.is_ascii());
    STATIC_REQUIRE(hello.is_printable_ascii());
    STATIC_REQUIRE(hello.is_legal());
    STATIC_REQUIRE(hello.view() == std::string_view("hello"));
}

/**
 * @test Construction and equality semantics.
 *
 * <ul>
 *   <li>Two identical string literals → same content and equality.</li>
 *   <li>Different literals → inequality.</li>
 * </ul>
 */
TEST_CASE("cstr construction from literals") {
    constexpr cstr a("abc");
    constexpr cstr b("abc");
    constexpr cstr c("xyz");

    STATIC_REQUIRE(a == b);
    STATIC_REQUIRE(a != c);
    REQUIRE(std::string(a.view()) == "abc");
}

/**
 * @test Transformation functions: <code>to_upper</code>, <code>to_lower</code>, <code>flip_case</code>.
 */
TEST_CASE("cstr transformations (upper/lower/flip)") {
    constexpr cstr lower("aBcD");
    constexpr auto upper = lower.to_upper();
    constexpr auto lower2 = lower.to_lower();
    constexpr auto flipped = lower.flip_case();

    STATIC_REQUIRE(upper.view() == "ABCD");
    STATIC_REQUIRE(lower2.view() == "abcd");
    STATIC_REQUIRE(flipped.view() == "AbCd");
}

/**
 * @test Numeric checks.
 *
 * <ul>
 *   <li><code>is_digit()</code> for pure digit strings.</li>
 *   <li><code>is_number()</code> for decimal / exponent formats.</li>
 *   <li>Rejects mixed alphanumeric.</li>
 * </ul>
 */
TEST_CASE("cstr numeric checks") {
    constexpr cstr digits("12345");
    constexpr cstr number("-12.34e+5");
    constexpr cstr not_number("12ab");

    STATIC_REQUIRE(digits.is_digit());
    STATIC_REQUIRE(digits.is_number());
    STATIC_REQUIRE(number.is_number());
    STATIC_REQUIRE_FALSE(not_number.is_number());
}

/**
 * @test Compile-time concatenation.
 */
TEST_CASE("cstr concatenation") {
    constexpr cstr a("hello_");
    constexpr cstr b("world");
    constexpr auto c = a + b;

    STATIC_REQUIRE(c.size() == 11);
    STATIC_REQUIRE(c.view() == "hello_world");
}

/**
 * @test Hash and equality semantics.
 *
 * <ul>
 *   <li>Same content → identical hash and equality.</li>
 *   <li>Different content → different hash and inequality.</li>
 * </ul>
 */
TEST_CASE("cstr hash and equality semantics") {
    using jh::utils::hash_fn::c_hash;
    constexpr cstr a("a_string");
    constexpr cstr b("a_string");
    constexpr cstr c("another_string");

    STATIC_REQUIRE(a.hash() == b.hash());
    STATIC_REQUIRE(a.hash(c_hash::djb2) != c.hash(c_hash::djb2));

    STATIC_REQUIRE(a == b);
    STATIC_REQUIRE_FALSE(a == c);
}

/**
 * @test Hex / Base64 / Base64URL validation.
 */
TEST_CASE("cstr hex/base64/base64url checks") {
    constexpr cstr hex("deadbeef");
    constexpr cstr not_hex("deadbexf");

    STATIC_REQUIRE(hex.is_hex());
    STATIC_REQUIRE_FALSE(not_hex.is_hex());

    constexpr cstr b64("QUJDRA==");   ///< "ABCD"
    constexpr cstr b64url("QUJDRA");  ///< Base64URL without padding

    STATIC_REQUIRE(b64.is_base64());
    STATIC_REQUIRE(b64url.is_base64url());
}

/**
 * @test Type identity using NTTP.
 *
 * <ul>
 *   <li>Same string literal → same template instantiation.</li>
 *   <li>Different string literal → different instantiation.</li>
 * </ul>
 */
TEST_CASE("cstr NTTP type identity") {
    using Foo1 = test::tag<"foo">;
    using Foo2 = test::tag<"foo">;
    using Bar  = test::tag<"bar">;

    STATIC_REQUIRE(std::is_same_v<Foo1, Foo2>);
    STATIC_REQUIRE_FALSE(std::is_same_v<Foo1, Bar>);
}

/**
 * @test Alphanumeric classification.
 *
 * <ul>
 *   <li>Alphabet-only → <code>is_alpha</code>.</li>
 *   <li>Digit-only → <code>is_digit</code>.</li>
 *   <li>Alnum mixed → <code>is_alnum</code>.</li>
 *   <li>Symbols → not alnum.</li>
 * </ul>
 */
TEST_CASE("cstr alnum checks") {
    constexpr cstr letters("Hello");
    constexpr cstr digits("12345");
    constexpr cstr alnum("abc123");
    constexpr cstr not_alnum("abc_123");

    STATIC_REQUIRE(letters.is_alpha());
    STATIC_REQUIRE_FALSE(letters.is_digit());
    STATIC_REQUIRE(letters.is_alnum());

    STATIC_REQUIRE(digits.is_digit());
    STATIC_REQUIRE_FALSE(digits.is_alpha());
    STATIC_REQUIRE(digits.is_alnum());

    STATIC_REQUIRE(alnum.is_alnum());
    STATIC_REQUIRE_FALSE(not_alnum.is_alnum());
}

/**
 * @test Legality checks.
 *
 * <ul>
 *   <li>Valid ASCII.</li>
 *   <li>Valid UTF-8 (Chinese "你好").</li>
 *   <li>Contains control characters → invalid.</li>
 *   <li>Malformed UTF-8 sequence → invalid.</li>
 * </ul>
 */
TEST_CASE("cstr legality checks") {
    constexpr cstr ascii("Hello123");
    STATIC_REQUIRE(ascii.is_ascii());
    STATIC_REQUIRE(ascii.is_printable_ascii());
    STATIC_REQUIRE(ascii.is_legal());

    constexpr char utf8_str[] = u8"你好";
    constexpr cstr utf8(utf8_str);
    STATIC_REQUIRE(utf8.is_legal());

    constexpr char ctrl_str[] = {'a', 'b', 'c', '\n', '\0'};
    constexpr cstr ctrl(ctrl_str);
    STATIC_REQUIRE_FALSE(ctrl.is_legal());

    constexpr char invalid_utf8[] = {'\xF0', '\x28', '\x8C', '\x28', '\0'};
    constexpr cstr bad(invalid_utf8);
    STATIC_REQUIRE_FALSE(bad.is_legal());
}

/**
 * @test Stream output operator.
 *
 * <ul>
 *   <li>Ensure <code>operator<<</code> writes underlying content.</li>
 * </ul>
 */
TEST_CASE("cstr ostream operator<<") {
    constexpr cstr s("ostream_check");
    std::ostringstream oss;
    oss << s;
    REQUIRE(oss.str() == "ostream_check");
}
