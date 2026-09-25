#include <catch2/catch_all.hpp>

#include "jh/metax/t_str.h"

using namespace jh::meta;

TEST_CASE("t_str construction from literals") {
    constexpr t_str a("abc");

    REQUIRE(std::string(a.view()) == "abc");
}

/**
 * @test Stream output operator.
 *
 * <ul>
 *   <li>Ensure <code>operator<<</code> writes underlying content.</li>
 * </ul>
 */
TEST_CASE("t_str ostream operator<<") {
    constexpr t_str s("ostream_check");
    std::ostringstream oss;
    oss << s;
    REQUIRE(oss.str() == "ostream_check");
}

/**
 * @test Conversion to and from byte arrays.
 *
 * <ul>
 *   <li>Runtime byte conversion yields identical reconstruction.</li>
 * </ul>
 */
TEST_CASE("t_str conversion to and from byte arrays") {
    using arr_t = jh::pod::array<std::uint8_t, 5>;

    // runtime memcpy path
    {
        t_str<6> s("world");
        auto bytes = static_cast<arr_t>(s);
        REQUIRE(bytes.data[0] == static_cast<std::uint8_t>('w'));

        auto restored = t_str{bytes};
        REQUIRE(restored.view() == "world");
        REQUIRE(restored == s);
    }

    // verify that mutation in bytes changes reconstructed string
    {
        t_str<6> s("abcde");
        auto bytes = static_cast<arr_t>(s);
        bytes.data[0] = static_cast<std::uint8_t>('A');
        auto modified = t_str{bytes};
        REQUIRE(modified.view() == "Abcde");
        REQUIRE(modified != s);
    }
}
/**
 * @test Substring operations: sub / sub_view / sub_pod_view.
 *
 * <ul>
 *   <li>Verify <code>sub_view()</code> and <code>sub_pod_view().to_std()</code> equivalence.</li>
 *   <li>Check runtime substring extraction against expected text.</li>
 * </ul>
 */
TEST_CASE("t_str substring operations") {

    constexpr t_str s("hello_world");

    /**
     * Runtime view validation
     */
    {
        auto v = s.sub_view<0, 5>();
        REQUIRE(v == "hello");
    }

    {
        auto v = s.sub_view<6>();
        REQUIRE(v == "world");
    }

    /**
     * sub_view vs sub_pod_view equivalence
     */
    {
        auto v1 = s.sub_view<0, 5>();
        auto v2 = s.sub_pod_view<0, 5>().to_std();

        REQUIRE(v1 == v2);
        REQUIRE(v1 == "hello");
    }

    {
        auto v1 = s.sub_view<6>();
        auto v2 = s.sub_pod_view<6>().to_std();

        REQUIRE(v1 == v2);
        REQUIRE(v1 == "world");
    }

    /**
     * Runtime substring check
     */
    {
        t_str runtime("run_time");

        auto sub = runtime.sub<4, 4>();
        REQUIRE(sub.view() == "time");

        auto v1 = runtime.sub_view<4, 4>();
        auto v2 = runtime.sub_pod_view<4, 4>().to_std();

        REQUIRE(v1 == "time");
        REQUIRE(v1 == v2);
    }
}

/**
 * @test Boundary conditions for substring extraction.
 *
 * <ul>
 *   <li>Verify that substring views agree with constexpr substring values.</li>
 * </ul>
 */
TEST_CASE("t_str substring boundary cases") {

    constexpr t_str s("hello_world");

    /**
     * view consistency
     */
    {
        auto v1 = s.sub_view<0, 11>();
        constexpr auto substring = s.sub<0, 11>();
        auto v2 = substring.view();

        REQUIRE(v1 == v2);
    }
}

/**
 * @test Verify that Count=-1 and default Count behave identically.
 */
TEST_CASE("t_str substring default count equals -1") {

    constexpr t_str s("hello_world");

    /**
     * runtime view check
     */
    {
        auto v1 = s.sub_view<6>();
        auto v2 = s.sub_view<6, static_cast<std::size_t>(-1)>();

        REQUIRE(v1 == v2);
        REQUIRE(v1 == "world");
    }

    /**
     * pod_view equivalence
     */
    {
        using namespace jh::pod::literals;
        auto p1 = s.sub_pod_view<6>();
        auto p2 = s.sub_pod_view<6, static_cast<std::size_t>(-1)>();

        REQUIRE(p1 == p2);
        REQUIRE(p1 == "world"_psv);
    }
}
