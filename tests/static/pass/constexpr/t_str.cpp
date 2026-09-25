#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>

#include "jh/metax/t_str.h"

using namespace jh::meta;

namespace test {
    template<TStr S>
    struct tag {};
}

#define JH_STATIC_REQUIRE(...) static_assert(__VA_ARGS__)
#define JH_STATIC_REQUIRE_FALSE(...) static_assert(!(__VA_ARGS__))

consteval void check_t_str_compile_time_contracts()
{
    {
        constexpr t_str a("abc");
        constexpr t_str b("abc");
        constexpr t_str c("xyz");
        JH_STATIC_REQUIRE(a == b);
        JH_STATIC_REQUIRE(a != c);
    }
    {
        using arr_t = jh::pod::array<std::uint8_t, 5>;
        constexpr t_str value("hello");
        constexpr auto bytes = static_cast<arr_t>(value);
        constexpr auto restored = t_str{bytes};
        JH_STATIC_REQUIRE(restored == value);
        JH_STATIC_REQUIRE(bytes.data[0] == static_cast<std::uint8_t>('h'));
        JH_STATIC_REQUIRE(bytes.data[4] == static_cast<std::uint8_t>('o'));
    }
    {
        constexpr t_str value("hello_world");
        constexpr auto hello = value.sub<0, 5>();
        constexpr auto world = value.sub<6, 5>();
        constexpr auto world_to_end = value.sub<6>();
        constexpr auto all_to_end = value.sub<0>();
        JH_STATIC_REQUIRE(hello == "hello");
        JH_STATIC_REQUIRE(hello.size() == 5);
        JH_STATIC_REQUIRE(world == "world");
        JH_STATIC_REQUIRE(world.size() == 5);
        JH_STATIC_REQUIRE(world_to_end == "world");
        JH_STATIC_REQUIRE(all_to_end == "hello_world");
        using HelloTag = test::tag<value.sub<0, 5>()>;
        using HelloTag2 = test::tag<"hello">;
        JH_STATIC_REQUIRE(std::is_same_v<HelloTag, HelloTag2>);
    }
    {
        constexpr t_str value("hello_world");
        constexpr auto empty_beginning = value.sub<0, 0>();
        constexpr auto empty_end = value.sub<11, 0>();
        constexpr auto last = value.sub<10, 1>();
        constexpr auto full = value.sub<0, 11>();
        JH_STATIC_REQUIRE(empty_beginning.size() == 0);
        JH_STATIC_REQUIRE(empty_beginning == "");
        JH_STATIC_REQUIRE(empty_end.size() == 0);
        JH_STATIC_REQUIRE(empty_end == "");
        JH_STATIC_REQUIRE(last == "d");
        JH_STATIC_REQUIRE(full == "hello_world");
    }
    {
        constexpr t_str value("hello_world");
        constexpr auto default_hello = value.sub<0>();
        constexpr auto explicit_hello = value.sub<0, static_cast<std::size_t>(-1)>();
        constexpr auto default_world = value.sub<6>();
        constexpr auto explicit_world = value.sub<6, static_cast<std::size_t>(-1)>();
        JH_STATIC_REQUIRE(default_hello == explicit_hello);
        JH_STATIC_REQUIRE(default_hello == "hello_world");
        JH_STATIC_REQUIRE(default_world == explicit_world);
        JH_STATIC_REQUIRE(default_world == "world");
    }

    // t_str basic constexpr properties
    {
    constexpr t_str hello("hello");
    JH_STATIC_REQUIRE(hello.size() == 5);
    JH_STATIC_REQUIRE(hello.is_alpha());
    JH_STATIC_REQUIRE_FALSE(hello.is_digit());
    JH_STATIC_REQUIRE(hello.is_ascii());
    JH_STATIC_REQUIRE(hello.is_printable_ascii());
    JH_STATIC_REQUIRE(hello.is_legal());
    JH_STATIC_REQUIRE(hello.view() == std::string_view("hello"));

    }
    // t_str transformations (upper/lower/flip)
    {
    constexpr t_str lower("aBcD");
    constexpr auto upper = lower.to_upper();
    constexpr auto lower2 = lower.to_lower();
    constexpr auto flipped = lower.flip_case();

    JH_STATIC_REQUIRE(upper.view() == "ABCD");
    JH_STATIC_REQUIRE(lower2.view() == "abcd");
    JH_STATIC_REQUIRE(flipped.view() == "AbCd");

    }
    // t_str numeric checks
    {
    constexpr t_str digits("12345");
    constexpr t_str number("-12.34e+5");
    constexpr t_str not_number("12ab");

    JH_STATIC_REQUIRE(digits.is_digit());
    JH_STATIC_REQUIRE(digits.is_number());
    JH_STATIC_REQUIRE(number.is_number());
    JH_STATIC_REQUIRE_FALSE(not_number.is_number());

    }
    // t_str concatenation
    {
    constexpr t_str a("hello_");
    constexpr t_str b("world");
    constexpr auto c = a + b;

    JH_STATIC_REQUIRE(c.size() == 11);
    JH_STATIC_REQUIRE(c.view() == "hello_world");

    }
    // t_str hash and equality semantics
    {
    using jh::meta::c_hash;
    constexpr t_str a("a_string");
    constexpr t_str b("a_string");
    constexpr t_str c("another_string");

    JH_STATIC_REQUIRE(a.hash() == b.hash());
    JH_STATIC_REQUIRE(a.hash(c_hash::djb2) != c.hash(c_hash::djb2));

    JH_STATIC_REQUIRE(a == b);
    JH_STATIC_REQUIRE_FALSE(a == c);

    }
    // t_str hex/base64/base64url checks
    {
    constexpr t_str hex("deadbeef");
    constexpr t_str not_hex("deadbexf");

    JH_STATIC_REQUIRE(hex.is_hex());
    JH_STATIC_REQUIRE_FALSE(not_hex.is_hex());

    constexpr t_str b64("QUJDRA==");   ///< "ABCD"
    constexpr t_str b64url("QUJDRA");  ///< Base64URL without padding

    JH_STATIC_REQUIRE(b64.is_base64());
    JH_STATIC_REQUIRE(b64url.is_base64url());

    }
    // t_str NTTP type identity
    {
    using Foo1 = test::tag<"foo">;
    using Foo2 = test::tag<"foo">;
    using Bar = test::tag<"bar">;

    JH_STATIC_REQUIRE(std::is_same_v<Foo1, Foo2>);
    JH_STATIC_REQUIRE_FALSE(std::is_same_v<Foo1, Bar>);

    }
    // t_str alnum checks
    {
    constexpr t_str letters("Hello");
    constexpr t_str digits("12345");
    constexpr t_str alnum("abc123");
    constexpr t_str not_alnum("abc_123");

    JH_STATIC_REQUIRE(letters.is_alpha());
    JH_STATIC_REQUIRE_FALSE(letters.is_digit());
    JH_STATIC_REQUIRE(letters.is_alnum());

    JH_STATIC_REQUIRE(digits.is_digit());
    JH_STATIC_REQUIRE_FALSE(digits.is_alpha());
    JH_STATIC_REQUIRE(digits.is_alnum());

    JH_STATIC_REQUIRE(alnum.is_alnum());
    JH_STATIC_REQUIRE_FALSE(not_alnum.is_alnum());

    }
    // t_str legality checks
    {
    constexpr t_str ascii("Hello123");
    JH_STATIC_REQUIRE(ascii.is_ascii());
    JH_STATIC_REQUIRE(ascii.is_printable_ascii());
    JH_STATIC_REQUIRE(ascii.is_legal());

    constexpr char utf8_str[] = u8"你好";
    constexpr t_str utf8(utf8_str);
    JH_STATIC_REQUIRE(utf8.is_legal());

    constexpr char ctrl_str[] = {'a', 'b', 'c', '\n', '\0'};
    constexpr t_str ctrl(ctrl_str);
    JH_STATIC_REQUIRE_FALSE(ctrl.is_legal());

    constexpr char invalid_utf8[] = {'\xF0', '\x28', '\x8C', '\x28', '\0'};
    constexpr t_str bad(invalid_utf8);
    JH_STATIC_REQUIRE_FALSE(bad.is_legal());

    }
}

static_assert((check_t_str_compile_time_contracts(), true));
