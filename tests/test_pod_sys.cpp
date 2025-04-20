#define CATCH_CONFIG_MAIN

#include <catch2/catch_all.hpp>
#include <array>
#include "jh/utils/platform.h"

#if IS_GCC || IS_CLANG
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
// No warning for jh::pod::tuple<>
#include <numeric>

#include "jh/pod.h"
#include "jh/sequence.h"

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
}

// ✅ Recognizing JH PODS
TEST_CASE("JH PODS Recognition") {
    STATIC_REQUIRE(pod::pod_like<pod::array<int, 128>>);
    STATIC_REQUIRE(pod::pod_like<pod::pair<int, float>>);
    STATIC_REQUIRE(pod::pod_like<pod::array<pod::pair<int, float>, 128>>);
    STATIC_REQUIRE(pod::pod_like<pod::bitflags<32>>);
    STATIC_REQUIRE(pod::pod_like<pod::bytes_view>);
    STATIC_REQUIRE(pod::pod_like<pod::optional<int>>);
    STATIC_REQUIRE(pod::pod_like<pod::optional<pod::pair<int, float>>>);
    STATIC_REQUIRE(pod::pod_like<pod::tuple<int, double, bool>>);
    STATIC_REQUIRE(pod::pod_like<pod::span<int>>);
    STATIC_REQUIRE(pod::pod_like<pod::string_view>);
    STATIC_REQUIRE(pod::pod_like<pod::span<pod::array<int, 128>>>);
    STATIC_REQUIRE(pod::pod_like<pod::array<pod::string_view, 128>>);
    STATIC_REQUIRE(pod::pod_like<pod::array<pod::span<pod::optional<pod::bytes_view>>, 128>>);
    STATIC_REQUIRE(pod::pod_like<pod::bitflags<8> >);
    STATIC_REQUIRE(pod::pod_like<pod::bitflags<16> >);
    STATIC_REQUIRE(pod::pod_like<pod::bitflags<24> >);
    STATIC_REQUIRE(pod::pod_like<pod::bitflags<32> >);
    STATIC_REQUIRE(pod::pod_like<pod::bitflags<40> >);
    STATIC_REQUIRE(pod::pod_like<pod::bitflags<64> >);
    STATIC_REQUIRE(std::is_same_v<jh::pod::array<std::uint8_t, 4>,
                           decltype(jh::pod::uint_to_bytes<std::uint32_t>(0)) >);
    STATIC_REQUIRE(std::is_same_v<jh::pod::array<std::uint8_t, 1>,
                           decltype(jh::pod::uint_to_bytes<std::uint8_t>(42)) >);
    STATIC_REQUIRE(std::is_same_v<std::uint32_t,
                           decltype(jh::pod::bytes_to_uint<4>(jh::pod::array<std::uint8_t, 4>{})) >);

}

TEST_CASE("JH_POD_STRUCT generated struct is pod_like") {
    STATIC_REQUIRE(pod::pod_like<test::SamplePacket>);
}

TEST_CASE("Manually asserted struct is pod_like") {
    STATIC_REQUIRE(pod::pod_like<test::Legacy>);
}

TEST_CASE("JH POD Containers Sequence Check") {
    STATIC_REQUIRE(jh::sequence<pod::array<int, 128>>);
    STATIC_REQUIRE_FALSE(jh::sequence<pod::bytes_view>);
    STATIC_REQUIRE(jh::sequence<pod::span<int>>);
    STATIC_REQUIRE(jh::sequence<pod::string_view>);
    STATIC_REQUIRE_FALSE(jh::sequence<pod::bitflags<64>>);
}

TEST_CASE("pod::array basic construction and access") {
    pod::array<int, 4> a = {{1, 2, 3, 4}};
    REQUIRE(a.size() == 4);
    REQUIRE(a[0] == 1);
    REQUIRE(a[3] == 4);

    a[2] = 42;
    REQUIRE(a[2] == 42);
}

TEST_CASE("pod::array supports range-based iteration") {
    pod::array<char, 3> chars = {{'a', 'b', 'c'}};

    std::string s;
    for (const char ch: chars) {
        s += ch;
    }

    REQUIRE(s == "abc");
}

TEST_CASE("pod::array equality comparison works") {
    pod::array<int, 3> a = {{1, 2, 3}};
    pod::array<int, 3> b = {{1, 2, 3}};
    pod::array<int, 3> c = {{1, 2, 4}};

    REQUIRE(a == b);
    REQUIRE_FALSE(a == c);
}

TEST_CASE("bitflags basic API (native uint backend)") {
    using F = jh::pod::bitflags<32>;

    F f{};
    REQUIRE(f.count() == 0);

    f.set(0);
    REQUIRE(f.has(0));
    REQUIRE(f.count() == 1);

    f.set(31);
    REQUIRE(f.has(31));
    REQUIRE(f.count() == 2);

    f.clear(0);
    REQUIRE_FALSE(f.has(0));
    REQUIRE(f.count() == 1);

    f.flip(1);
    REQUIRE(f.has(1));

    f.flip(1);
    REQUIRE_FALSE(f.has(1));

    f.set_all();
    REQUIRE(f.count() == 32);

    f.reset_all();
    REQUIRE(f.count() == 0);
}

TEST_CASE("bitflags to_bytes and from_bytes roundtrip") {
    using F = pod::bitflags<16>;
    F f{};
    f.set(0);
    f.set(7);
    f.set(8);

    const auto snapshot = to_bytes(f);
    const auto restored = jh::pod::from_bytes<16>(snapshot);

    REQUIRE(restored.has(0));
    REQUIRE(restored.has(7));
    REQUIRE(restored.has(8));
    REQUIRE(restored.count() == 3);
    REQUIRE(restored == f);
}

TEST_CASE("bytes_view basic reinterpret and comparison") {
    SECTION("from std::array and compare views") {
        std::array<std::uint8_t, 4> a = {1, 2, 3, 4};
        std::array<std::uint8_t, 4> b = {1, 2, 3, 4};
        std::array<std::uint8_t, 4> c = {4, 3, 2, 1};

        auto va = pod::bytes_view::from(a.data(), a.size());
        auto vb = pod::bytes_view::from(b.data(), b.size());
        auto vc = pod::bytes_view::from(c.data(), c.size());

        REQUIRE(va == vb);
        REQUIRE_FALSE(va == vc);
    }

    SECTION("reinterpret as struct using at<T>()") {
        struct TestStruct {
            std::uint16_t a;
            std::uint16_t b;
        };
        REQUIRE(jh::pod::trivial_bytes<TestStruct>);

        TestStruct original{0x1234, 0xABCD};
        auto view = pod::bytes_view::from(original);
        auto [a, b] = view.at<TestStruct>();
        REQUIRE(a == 0x1234);
        REQUIRE(b == 0xABCD);
    }

    SECTION("clone to pod::array<int, N>") {
        pod::array<int, 3> arr = {10, 20, 30};
        auto view = pod::bytes_view::from(arr.data, pod::array<int, 3>::size());

        auto clone = view.clone<pod::array<int, 3> >();
        REQUIRE(clone[0] == 10);
        REQUIRE(clone[1] == 20);
        REQUIRE(clone[2] == 30);
    }

    SECTION("fetch returns nullptr if out of bounds") {
        std::uint32_t x = 0xAABBCCDD;
        auto view = pod::bytes_view::from(x);

        const auto *ok = view.fetch<std::uint32_t>();
        const auto *bad = view.fetch<std::uint32_t>(4); // too far

        REQUIRE(ok != nullptr);
        REQUIRE(bad == nullptr);
    }

    SECTION("fallback clone returns default on length mismatch") {
        struct PodTest {
            int a;
            float b;
        };
        REQUIRE(jh::pod::pod_like<PodTest>);

        std::array<std::byte, 2> too_small{};
        auto view = pod::bytes_view{too_small.data(), too_small.size()};
        const auto [a, b] = view.clone<PodTest>();

        REQUIRE(a == 0);
        REQUIRE(b == 0.0f);
    }
}

TEST_CASE("bytes_view clone from std::array to pod::array") {
    constexpr std::size_t N = 64;
    std::array<std::uint32_t, N> original{};
    std::iota(original.begin(), original.end(), 100); // Fill with 100, 101, ..., 163

    const auto view = pod::bytes_view::from(original.data(), original.size());
    auto cloned = view.clone<pod::array<std::uint32_t, N> >();

    REQUIRE(cloned.size() == N);
    for (std::size_t i = 0; i < N; ++i) {
        REQUIRE(cloned[i] == original[i]);
    }
}

TEST_CASE("pod::optional basic behavior") {
    using pod::optional;
    using pod::make_optional;

    SECTION("Default constructed is empty") {
        optional<int> o{};
        REQUIRE(o.empty());
        REQUIRE_FALSE(o.has());
    }

    SECTION("store sets value and has() returns true") {
        optional<int> o{};
        o.store(42);
        REQUIRE(o.has());
        REQUIRE_FALSE(o.empty());
        REQUIRE(o.ref() == 42);
        REQUIRE(*o.get() == 42);
    }

    SECTION("clear resets the optional") {
        optional<int> o{};
        o.store(99);
        REQUIRE(o.has());
        o.clear();
        REQUIRE_FALSE(o.has());
        REQUIRE(o.empty());
    }

    SECTION("make_optional returns filled optional") {
        auto o = make_optional(1234);
        REQUIRE(o.has());
        REQUIRE(o.ref() == 1234);
    }

    SECTION("copy from existing pod type") {
        struct Sample {
            int a;
            float b;
        };
        REQUIRE(pod::pod_like<Sample>);

        Sample s{10, 3.5f};
        auto o = make_optional(s);
        REQUIRE(o.has());
        REQUIRE(o.ref().a == 10);
        REQUIRE(o.ref().b == 3.5f);
    }
}

TEST_CASE("pod::array of optional<T> usage") {
    using pod::array;
    using pod::optional;
    using pod::make_optional;

    constexpr std::uint16_t N = 8;
    array<optional<int>, N> opt_arr{};

    SECTION("Initially all optionals are empty") {
        for (std::uint16_t i = 0; i < N; ++i) {
            REQUIRE(opt_arr[i].empty());
        }
    }

    SECTION("Storing values into some elements") {
        for (std::uint16_t i = 0; i < N; i += 2) {
            opt_arr[i].store(i * 10);
        }

        for (std::uint16_t i = 0; i < N; ++i) {
            if (i % 2 == 0) {
                REQUIRE(opt_arr[i].has());
                REQUIRE(opt_arr[i].ref() == static_cast<int>(i * 10));
            } else {
                REQUIRE(opt_arr[i].empty());
            }
        }
    }

    SECTION("Clear values selectively") {
        for (std::uint16_t i = 0; i < N; ++i) {
            opt_arr[i].store(i);
        }

        opt_arr[3].clear();
        opt_arr[5].clear();

        for (std::uint16_t i = 0; i < N; ++i) {
            if (i == 3 || i == 5) {
                REQUIRE_FALSE(opt_arr[i].has());
            } else {
                REQUIRE(opt_arr[i].has());
                REQUIRE(opt_arr[i].ref() == static_cast<int>(i));
            }
        }
    }

    SECTION("Use with algorithm-like access") {
        for (std::uint16_t i = 0; i < N; ++i)
            opt_arr[i] = make_optional(i * i);

        int sum = 0;
        for (const auto &o: opt_arr) {
            REQUIRE(o.has());
            sum += o.ref();
        }

        int expected = 0;
        for (std::uint16_t i = 0; i < N; ++i)
            expected += i * i;

        REQUIRE(sum == expected);
    }
}

TEST_CASE("pod::span works with pod::array") {
    using pod::span;
    using pod::array;

    constexpr std::uint16_t N = 10;
    array<int, N> arr{};

    for (std::uint16_t i = 0; i < N; ++i)
        arr[i] = i * 2;

    span<int> s = {arr.data, array<int, N>::size()};

    SECTION("Basic span properties") {
        REQUIRE(s.size() == N);
        REQUIRE_FALSE(s.empty());
        for (std::uint16_t i = 0; i < N; ++i)
            REQUIRE(s[i] == arr[i]);
    }

    SECTION("Range-for iteration over span") {
        int expected = 0;
        for (auto v: s) {
            REQUIRE(v == expected);
            expected += 2;
        }
    }

    SECTION("sub(), first(), last() slicing") {
        auto mid = s.sub(3, 4);
        REQUIRE(mid.size() == 4);
        REQUIRE(mid[0] == arr[3]);
        REQUIRE(mid[3] == arr[6]);

        auto first = s.first(5);
        REQUIRE(first.size() == 5);
        REQUIRE(first[0] == arr[0]);
        REQUIRE(first[4] == arr[4]);

        auto last = s.last(3);
        REQUIRE(last.size() == 3);
        REQUIRE(last[0] == arr[N - 3]);
    }

    SECTION("Equality comparison") {
        span<int> same = {arr.data, array<int, N>::size()};
        REQUIRE(s == same);

        span<int> shorty = {arr.data, N - 1};
        REQUIRE(s != shorty);
    }
}

TEST_CASE("pod::to_span from array and containers") {
    using pod::array;
    using pod::to_span;

    SECTION("T[N] raw array") {
        int raw[5] = {1, 2, 3, 4, 5};
        auto s = to_span<int>(raw);
        REQUIRE(s.size() == 5);
        REQUIRE(s[2] == 3);
    }

    SECTION("const T[N] raw array") {
        constexpr int raw[3] = {10, 20, 30};
        auto s = to_span<int>(raw);
        REQUIRE(s.size() == 3);
        REQUIRE(s[1] == 20);
    }

    SECTION("pod::array<T, N>") {
        array<std::uint16_t, 4> a = {{11, 22, 33, 44}};
        auto s = to_span<std::uint16_t>(a.data);
        REQUIRE(s.size() == 4);
        REQUIRE(s[3] == 44);
    }

    SECTION("const pod::array<T, N>") {
        constexpr array<std::uint8_t, 2> a = {9, 99};
        auto s = to_span<std::uint8_t>(a.data);
        REQUIRE(s.size() == 2);
        REQUIRE(s[0] == 9);
        REQUIRE(s[1] == 99);
    }

    SECTION("to_span with vector-like struct") {
        struct DummyVec {
            int buf[3] = {7, 14, 21};

            [[nodiscard]] const int *data() const noexcept { return buf; }

            [[nodiscard]] static std::uint64_t size() noexcept { return 3; }
        };

        DummyVec v{};
        auto s = to_span(v);
        REQUIRE(s.size() == 3);
        REQUIRE(s[2] == 21);
    }

    SECTION("const container concept with data/size") {
        struct ConstVec {
            const int buf[2] = {42, 88};

            [[nodiscard]] const int *data() const noexcept { return buf; }

            [[nodiscard]] static std::uint64_t size() noexcept { return 2; }
        };

        ConstVec v{};
        auto s = to_span(v);
        REQUIRE(s.size() == 2);
        REQUIRE(s[0] == 42);
    }
}

TEST_CASE("pod::string_view basic usage", "[string_view]") {
    static constexpr char raw[] = "hello_pod_world";
    constexpr std::uint64_t len = 15;
    using pod::string_view;

    string_view sv{raw, len};

    SECTION("Correct length and data") {
        REQUIRE(sv.size() == len);
        REQUIRE(sv[0] == 'h');
        REQUIRE(sv[len - 1] == 'd');
    }

    SECTION("Equality comparison") {
        static constexpr char raw2[] = "hello_pod_world";
        string_view other{raw2, len};
        REQUIRE(sv == other);
    }

    SECTION("Subrange works") {
        string_view sub = sv.sub(6, 3); // expect "pod"
        REQUIRE(sub.size() == 3);
        REQUIRE(sub == string_view{"pod", 3});
        // temporary, do NOT use this for long life-time pod::string_view
    }

    SECTION("Starts with / Ends with") {
        REQUIRE(sv.starts_with(string_view{"hello", 5}));
        REQUIRE(sv.ends_with(string_view{"world", 5}));
    }

    SECTION("Find character") {
        REQUIRE(sv.find('p') == 6);
        REQUIRE(sv.find('z') == static_cast<std::uint64_t>(-1));
    }

    SECTION("Hash is deterministic and non-zero") {
        auto hash = sv.hash();
        REQUIRE(hash != 0);
        REQUIRE(hash != static_cast<std::uint64_t>(-1));
    }

    SECTION("Copy to buffer") {
        char buffer[32] = {};
        sv.copy_to(buffer, sizeof(buffer));
        REQUIRE(std::strcmp(buffer, "hello_pod_world") == 0);
    }
}

TEST_CASE("pod::array<pod::string_view> comparison") {
    using pod::array;
    using pod::string_view;
    const auto str1 = "abcd";
    const auto str2 = "abcd"; // same str

    const array<string_view, 4> a1 =
            {
                    {
                            {str1, 1},
                            {str1, 2},
                            {str1, 3},
                            {str1, 4}
                    }
            };

    const array<string_view, 4> a2 =
            {
                    {
                            {str2, 1},
                            {str2, 2},
                            {str2, 3},
                            {str2, 4}
                    }
            };

    REQUIRE(a1 == a2);  // different memories but same semantics
}
