#include <concepts>
#include <cstddef>
#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>

#include "jh/macros/platform.h"
#include "jh/meta"
#include "jh/pod"

namespace pod = jh::pod;

namespace test {
    // POD macro struct
    JH_POD_STRUCT(SamplePacket,
                  std::uint16_t id;
                          std::uint8_t flags;
                          std::uint8_t kind;
    );

    // Manual REQUIRE on user-defined type
    struct Legacy {
        int x;
        float y;
    };

    JH_ASSERT_POD_LIKE(Legacy);

    struct throwing_linear_view {
        int *data() const noexcept(false);
        std::size_t size() const noexcept;
    };
}

constexpr int checked_span_values[] = {1, 2, 3};
constexpr pod::array<int, 3> checked_array{{3, 5, 7}};
constexpr pod::array<int, 3> checked_at_values{{11, 22, 33}};
constexpr auto checked_at_value = checked_at_values.at(1);
constexpr auto checked_at_out_of_bounds = checked_at_values.at(3);


static_assert(pod::pod_like<test::SamplePacket>);
static_assert(pod::pod_like<test::Legacy>);
static_assert(checked_at_value);
static_assert(**checked_at_value == 22);
static_assert(!checked_at_out_of_bounds);
static_assert(checked_at_out_of_bounds.error() == pod::array<int, 3>::error_code::out_of_bounds);

constexpr auto literal_string_view = pod::string_view::from_literal("hello");
constexpr auto empty_string_view = pod::string_view::from_literal("");
static_assert(literal_string_view.size() == 5);
static_assert(empty_string_view.empty());

template<std::size_t N>
using bitflags = pod::bitflags<N>;
static_assert(std::same_as<decltype(std::declval<bitflags<8>>() | std::declval<bitflags<8>>()), bitflags<8>>);
static_assert(std::same_as<decltype(std::declval<bitflags<16>>() | std::declval<bitflags<16>>()), bitflags<16>>);
static_assert(std::same_as<decltype(std::declval<bitflags<32>>() | std::declval<bitflags<32>>()), bitflags<32>>);
static_assert(std::same_as<decltype(std::declval<bitflags<64>>() | std::declval<bitflags<64>>()), bitflags<64>>);
static_assert(std::same_as<decltype(std::declval<bitflags<8>>() & std::declval<bitflags<8>>()), bitflags<8>>);
static_assert(std::same_as<decltype(std::declval<bitflags<16>>() & std::declval<bitflags<16>>()), bitflags<16>>);
static_assert(std::same_as<decltype(std::declval<bitflags<32>>() & std::declval<bitflags<32>>()), bitflags<32>>);
static_assert(std::same_as<decltype(std::declval<bitflags<64>>() & std::declval<bitflags<64>>()), bitflags<64>>);
static_assert(std::same_as<decltype(std::declval<bitflags<8>>() ^ std::declval<bitflags<8>>()), bitflags<8>>);
static_assert(std::same_as<decltype(std::declval<bitflags<16>>() ^ std::declval<bitflags<16>>()), bitflags<16>>);
static_assert(std::same_as<decltype(std::declval<bitflags<32>>() ^ std::declval<bitflags<32>>()), bitflags<32>>);
static_assert(std::same_as<decltype(std::declval<bitflags<64>>() ^ std::declval<bitflags<64>>()), bitflags<64>>);
static_assert(std::same_as<decltype(~std::declval<bitflags<8>>()), bitflags<8>>);
static_assert(std::same_as<decltype(~std::declval<bitflags<16>>()), bitflags<16>>);
static_assert(std::same_as<decltype(~std::declval<bitflags<32>>()), bitflags<32>>);
static_assert(std::same_as<decltype(~std::declval<bitflags<64>>()), bitflags<64>>);

#define JH_STATIC_REQUIRE(...) static_assert(__VA_ARGS__)
#define JH_STATIC_REQUIRE_FALSE(...) static_assert(!(__VA_ARGS__))

consteval bool check_pod_compile_time_contracts()
{
    using namespace jh::pod::literals;
    JH_STATIC_REQUIRE(pod::pod_like<pod::array<int, 128>>);
    JH_STATIC_REQUIRE(pod::pod_like<pod::pair<int, float>>);
    JH_STATIC_REQUIRE(pod::pod_like<pod::array<pod::pair<int, float>, 128>>);
    JH_STATIC_REQUIRE(pod::pod_like<pod::bitflags<32>>);
    JH_STATIC_REQUIRE(pod::pod_like<pod::bytes_view>);
    JH_STATIC_REQUIRE(pod::pod_like<pod::optional<int>>);
    JH_STATIC_REQUIRE(pod::pod_like<pod::optional<pod::pair<int, float>>>);
    JH_STATIC_REQUIRE(pod::pod_like<pod::tuple<int, double, bool>>);
    JH_STATIC_REQUIRE(pod::pod_like<pod::span<int>>);
    JH_STATIC_REQUIRE(pod::pod_like<pod::string_view>);
    enum class result_error : std::uint8_t { failed };
    using int_result = jh::meta::expected<int, result_error>;
    using rebound_result = int_result::rebind<long>;
    JH_STATIC_REQUIRE(pod::pod_like<int_result>);
    JH_STATIC_REQUIRE(pod::pod_like<jh::meta::unexpected<result_error>>);
    JH_STATIC_REQUIRE(std::is_same_v<rebound_result, jh::meta::expected<long, result_error>>);
    JH_STATIC_REQUIRE(std::is_constructible_v<jh::meta::expected<std::string, result_error>, std::string>);
    JH_STATIC_REQUIRE(std::is_constructible_v<jh::meta::expected<std::string, result_error>,
                                           jh::meta::unexpected<result_error>>);
    JH_STATIC_REQUIRE_FALSE(pod::pod_like<jh::meta::expected<std::string, result_error>>);
    JH_STATIC_REQUIRE(noexcept(std::declval<int_result &>().value()));
    JH_STATIC_REQUIRE(noexcept(std::declval<int_result &>().error()));
    JH_STATIC_REQUIRE(noexcept(std::declval<pod::array<int, 3> &>().at(0)));
    JH_STATIC_REQUIRE(pod::pod_like<decltype(std::declval<pod::array<int, 3> &>().at(0))>);
    JH_STATIC_REQUIRE(pod::pod_like<decltype(std::declval<pod::array<int, 3> const &>().at(0))>);
    JH_STATIC_REQUIRE(noexcept(std::declval<pod::bytes_view const &>().fetch<std::uint32_t>(0)));
    JH_STATIC_REQUIRE(noexcept(std::declval<pod::bytes_view const &>().clone<std::uint32_t>()));
    JH_STATIC_REQUIRE(noexcept(std::declval<pod::bytes_view const &>().hash()));
    JH_STATIC_REQUIRE(noexcept(std::declval<pod::optional<int> &>().store(1)));
    JH_STATIC_REQUIRE(noexcept(std::declval<pod::span<int> const &>().sub(0)));
    JH_STATIC_REQUIRE(noexcept(std::declval<pod::span<int> const &>().first(0)));
    JH_STATIC_REQUIRE(noexcept(std::declval<pod::span<int> const &>().last(0)));
    JH_STATIC_REQUIRE(noexcept(pod::to_span(std::declval<pod::array<int, 3> &>())));
    JH_STATIC_REQUIRE_FALSE(noexcept(pod::to_span(std::declval<test::throwing_linear_view &>())));
    JH_STATIC_REQUIRE(noexcept(std::declval<pod::string_view const &>().sub(0)));
    JH_STATIC_REQUIRE(noexcept(std::declval<pod::string_view const &>().semantic_len()));
    JH_STATIC_REQUIRE(noexcept(std::declval<pod::string_view const &>().copy_to(nullptr, 0)));
    JH_STATIC_REQUIRE(noexcept(std::declval<pod::string_view const &>().hash()));
    JH_STATIC_REQUIRE(noexcept(pod::uint_to_bytes<std::uint32_t>(1)));
    JH_STATIC_REQUIRE(noexcept(pod::bytes_to_uint<4>(std::declval<const pod::array<std::uint8_t, 4> &>())));
    JH_STATIC_REQUIRE(noexcept(pod::bitflags<8>::size()));
    JH_STATIC_REQUIRE(noexcept(pod::bitflags<24>::size()));
    JH_STATIC_REQUIRE(noexcept(pod::to_bytes(std::declval<pod::bitflags<24>>())));
    JH_STATIC_REQUIRE(noexcept(pod::from_bytes<24>(std::declval<pod::array<std::uint8_t, 3>>())));

    constexpr auto little_endian = pod::uint_to_bytes<std::uint32_t>(0x12345678u);
    JH_STATIC_REQUIRE(little_endian.data[0] == 0x78);
    JH_STATIC_REQUIRE(little_endian.data[1] == 0x56);
    JH_STATIC_REQUIRE(pod::bytes_to_uint<4>(little_endian) == 0x12345678u);
    constexpr auto flag_roundtrip = [] {
        pod::bitflags<24> flags{};
        flags.set(3);
        flags.set(17);
        return pod::from_bytes<24>(pod::to_bytes(flags));
    }();
    JH_STATIC_REQUIRE(flag_roundtrip.size() == 24);
    JH_STATIC_REQUIRE(flag_roundtrip.has(3));
    JH_STATIC_REQUIRE(flag_roundtrip.has(17));

    constexpr int_result value_result{42};
    constexpr int_result error_result = jh::meta::unexpected(result_error::failed);
    constexpr int_result assigned_error = [] {
        int_result result{};
        result = jh::meta::unexpected(result_error::failed);
        return result;
    }();
    constexpr int_result assigned_value = [] {
        int_result result = jh::meta::unexpected(result_error::failed);
        result = 13;
        return result;
    }();
    JH_STATIC_REQUIRE(value_result && value_result.value() == 42);
    JH_STATIC_REQUIRE(value_result.value_or(0) == 42);
    JH_STATIC_REQUIRE(error_result.has_error());
    JH_STATIC_REQUIRE(error_result.error() == result_error::failed);
    JH_STATIC_REQUIRE(assigned_error.error() == result_error::failed);
    JH_STATIC_REQUIRE(assigned_value && assigned_value.value() == 13);
    JH_STATIC_REQUIRE(error_result.value_or(7) == 7);
    JH_STATIC_REQUIRE(error_result.error_or(result_error::failed) == result_error::failed);
    JH_STATIC_REQUIRE(value_result.error_or(result_error::failed) == result_error::failed);

    constexpr auto maybe = pod::make_optional(17);
    JH_STATIC_REQUIRE(maybe.has());
    JH_STATIC_REQUIRE(maybe.value_or(0) == 17);
    constexpr auto optional_text_a = pod::make_optional("optional"_psv);
    constexpr auto optional_text_b = pod::make_optional("optional"_psv);
    JH_STATIC_REQUIRE(optional_text_a == optional_text_b);

    constexpr pod::span<const int> checked_span{checked_span_values, 3};
    constexpr auto bad_slice = checked_span.sub(4);
    JH_STATIC_REQUIRE_FALSE(bad_slice);
    JH_STATIC_REQUIRE(bad_slice.error() == pod::span<const int>::error_code::out_of_bounds);

    constexpr bool copied_at_compile_time = [] {
        char output[4]{};
        const auto result = "abc"_psv.copy_to(output, sizeof(output));
        return result && result.value() == 3 && output[0] == 'a' && output[3] == '\0';
    }();
    JH_STATIC_REQUIRE(copied_at_compile_time);
    constexpr auto invalid_hash = pod::string_view{nullptr, 1}.hash();
    JH_STATIC_REQUIRE_FALSE(invalid_hash);
    JH_STATIC_REQUIRE(invalid_hash.error() == pod::string_view::error_code::null_data);
    constexpr auto invalid_copy = pod::string_view::from_literal("x").copy_to(nullptr, 0);
    JH_STATIC_REQUIRE_FALSE(invalid_copy);
    JH_STATIC_REQUIRE(invalid_copy.error() == pod::string_view::error_code::invalid_buffer);

    constexpr auto in_bounds = checked_array.at(1);
    constexpr auto out_of_bounds = checked_array.at(3);
    JH_STATIC_REQUIRE(in_bounds && **in_bounds == 5);
    JH_STATIC_REQUIRE(!out_of_bounds);
    JH_STATIC_REQUIRE(out_of_bounds.error() == pod::array<int, 3>::error_code::out_of_bounds);
    constexpr auto checked_view = pod::to_span(checked_array);
    JH_STATIC_REQUIRE(checked_view && checked_view.value().size() == 3);
    JH_STATIC_REQUIRE(pod::pod_like<pod::span<pod::array<int, 128>>>);
    JH_STATIC_REQUIRE(pod::pod_like<pod::array<pod::string_view, 128>>);
    JH_STATIC_REQUIRE(pod::pod_like<pod::array<pod::span<pod::optional<pod::bytes_view>>, 128>>);
    JH_STATIC_REQUIRE(pod::pod_like<pod::bitflags<8> >);
    JH_STATIC_REQUIRE(pod::pod_like<pod::bitflags<16> >);
    JH_STATIC_REQUIRE(pod::pod_like<pod::bitflags<24> >);
    JH_STATIC_REQUIRE(pod::pod_like<pod::bitflags<32> >);
    JH_STATIC_REQUIRE(pod::pod_like<pod::bitflags<40> >);
    JH_STATIC_REQUIRE(pod::pod_like<pod::bitflags<64> >);
    JH_STATIC_REQUIRE(std::is_same_v<jh::pod::array<std::uint8_t, 4>,
                           decltype(jh::pod::uint_to_bytes<std::uint32_t>(0)) >);
    JH_STATIC_REQUIRE(std::is_same_v<jh::pod::array<std::uint8_t, 1>,
                           decltype(jh::pod::uint_to_bytes<std::uint8_t>(42)) >);
    JH_STATIC_REQUIRE(std::is_same_v<std::uint32_t,
                           decltype(jh::pod::bytes_to_uint<4>(jh::pod::array<std::uint8_t, 4>{})) >);
    JH_STATIC_REQUIRE(std::is_same_v<jh::pod::pair<int, double>::first_type, int>);
    JH_STATIC_REQUIRE(std::is_same_v<jh::pod::pair<int, double>::second_type, double>);
    JH_STATIC_REQUIRE(std::is_same_v<jh::pod::span<int>::element_type, int>);
    JH_STATIC_REQUIRE(jh::pod::string_view::from_literal("hello") == "hello"_psv);
    JH_STATIC_REQUIRE(""_psv.empty());
    JH_STATIC_REQUIRE("hello"_psv.size() == 5);
    JH_STATIC_REQUIRE("hello"_psv.begin()[0] == 'h');
    JH_STATIC_REQUIRE("hello"_psv.end()[-1] == 'o');
    constexpr auto hw = "hello_world"_psv;
    constexpr auto pre = "hello"_psv;
    constexpr auto suf = "world"_psv;
    constexpr auto mid = "ello_w"_psv;
    JH_STATIC_REQUIRE(hw.starts_with(pre));
    JH_STATIC_REQUIRE(hw.ends_with(suf));
    JH_STATIC_REQUIRE(!hw.starts_with("holla"_psv));
    JH_STATIC_REQUIRE(!hw.ends_with("wurld"_psv));
    constexpr auto hw_sub = hw.sub(1, 6);
    JH_STATIC_REQUIRE(hw_sub);
    JH_STATIC_REQUIRE(hw_sub.value() == mid);
    JH_STATIC_REQUIRE("abc"_psv.compare("abc"_psv) == 0);
    JH_STATIC_REQUIRE("abc"_psv.compare("abd"_psv) < 0);
    JH_STATIC_REQUIRE("abd"_psv.compare("abc"_psv) > 0);
    JH_STATIC_REQUIRE("abc"_psv.hash() == "abc"_psv.hash());
    JH_STATIC_REQUIRE("abc"_psv.hash() != "xyz"_psv.hash());
    constexpr auto s = "podsystem"_psv;
    JH_STATIC_REQUIRE(s.sub(0, 3).value() == "pod"_psv);
    JH_STATIC_REQUIRE(s.find('s') == 3);
    JH_STATIC_REQUIRE(s.find('x') == static_cast<std::uint64_t>(-1));
    constexpr auto a = "abc"_psv;
    constexpr auto b = "abd"_psv;
    constexpr auto c = "abc"_psv;
    JH_STATIC_REQUIRE((a <=> b) == std::strong_ordering::less);
    JH_STATIC_REQUIRE((b <=> a) == std::strong_ordering::greater);
    JH_STATIC_REQUIRE((a <=> c) == std::strong_ordering::equal);
    JH_STATIC_REQUIRE(a < b);
    JH_STATIC_REQUIRE(b > a);
    JH_STATIC_REQUIRE(!(a > b));
    JH_STATIC_REQUIRE(a == c);
    JH_STATIC_REQUIRE(a <= c);
    JH_STATIC_REQUIRE(a >= c);
    constexpr auto s1 = "hello"_psv;
    static_assert(s1.semantic_len().value() == 5);

    constexpr auto s2 = "\U00004F60\U0000597D"_psv;
    static_assert(s2.semantic_len().value() == 2);

    constexpr auto s3 = "\U0001F30D"_psv;
    static_assert(s3.semantic_len().value() == 1);

    return true;
}

static_assert(check_pod_compile_time_contracts());
