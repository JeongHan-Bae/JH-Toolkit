#define CATCH_CONFIG_MAIN

#include <catch2/catch_all.hpp>
#include <array>
#include <vector>
#include <ranges>
#include "jh/macros/platform.h"
#include <numeric>

#include "jh/pod"
#include "jh/conceptual/sequence.h"

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
    STATIC_REQUIRE(std::is_same_v<jh::pod::pair<int, double>::first_type, int>);
    STATIC_REQUIRE(std::is_same_v<jh::pod::pair<int, double>::second_type, double>);
    STATIC_REQUIRE(std::is_same_v<jh::pod::span<int>::element_type, int>);
}

TEST_CASE("JH_POD_STRUCT generated struct is pod_like") {
    STATIC_REQUIRE(pod::pod_like<test::SamplePacket>);
}

TEST_CASE("Manually asserted struct is pod_like") {
    STATIC_REQUIRE(pod::pod_like<test::Legacy>);
}

TEST_CASE("JH POD Containers Sequence Check") {
    STATIC_REQUIRE(jh::concepts::sequence<pod::array<int, 128>>);
    STATIC_REQUIRE_FALSE(jh::concepts::sequence<pod::bytes_view>);
    STATIC_REQUIRE(jh::concepts::sequence<pod::span<int>>);
    STATIC_REQUIRE(jh::concepts::sequence<pod::string_view>);
    STATIC_REQUIRE_FALSE(jh::concepts::sequence<pod::bitflags<64>>);
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

    f.flip_all();         // invert all bits
    REQUIRE(f.count() == 32);

    f.flip_all();         // invert back
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

TEST_CASE("bitflags full API (bytes backend)") {
    using F = jh::pod::bitflags<24>; // non-native, uses byte-array backend
    F f{};
    REQUIRE(f.count() == 0);

    f.set(0);
    f.set(23);
    REQUIRE(f.has(0));
    REQUIRE(f.has(23));

    f.flip(0);
    REQUIRE_FALSE(f.has(0));

    f.set_all();
    REQUIRE(f.count() == 24);

    f.reset_all();
    REQUIRE(f.count() == 0);

    f.flip_all();
    REQUIRE(f.count() == 24);
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
        REQUIRE(va != vc);
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

TEST_CASE("pod::optional value_or behavior") {
    using pod::optional;

    SECTION("Returns fallback when empty") {
        optional<int> o{};
        REQUIRE_FALSE(o.has());
        REQUIRE(o.value_or(99) == 99);
    }

    SECTION("Returns stored value when present") {
        optional<int> o{};
        o.store(123);
        REQUIRE(o.has());
        REQUIRE(o.value_or(999) == 123); // fallback ignored
    }

    SECTION("Works with trivial struct") {
        struct S {
            int x;
            float y;
        };
        static_assert(pod::pod_like<S>);

        optional<S> o{};
        S def{5, 3.5f};
        REQUIRE(o.value_or(def).x == 5);
        REQUIRE(o.value_or(def).y == 3.5f);

        o.store({42, 1.0f});
        REQUIRE(o.value_or(def).x == 42);
        REQUIRE(o.value_or(def).y == 1.0f);
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

TEST_CASE("pod::optional equality semantics") {
    using pod::optional;
    using pod::make_optional;

    SECTION("default vs value-initialized 0") {
        optional<int> def{};              // Default constructed: has_value = false, storage is 0-initialized
        auto val0 = make_optional(0);     // Stored value 0: has_value = true, storage contains all zeroes

        REQUIRE(def != val0);             // Different states: one empty, one holding a value
        val0.clear();                     // Clear: has_value = false
        REQUIRE(def == val0);             // Both empty, considered equal regardless of storage content
    }

    SECTION("different stored values are not equal") {
        auto a = make_optional(234);
        auto b = make_optional(16);
        REQUIRE(a != b);                  // Both have values, raw storage differs
    }

    SECTION("same stored values are equal") {
        auto a = make_optional(16);
        auto b = make_optional(16);
        REQUIRE(a == b);                  // Both have values, raw storage identical
    }

    SECTION("clear makes them equal") {
        auto a = make_optional(234);
        auto b = make_optional(16);
        REQUIRE(a != b);                  // Initially different values

        a.clear();
        b.clear();
        REQUIRE(a == b);                  // After clear: both empty, considered equal
    }
}

TEST_CASE("pod::array of optional equality semantics") {
    using pod::array;
    using pod::optional;
    using pod::make_optional;

    SECTION("cleared optional equals default optional inside array") {
        array<optional<int>, 2> arr1{make_optional(16), make_optional(16)};
        array<optional<int>, 2> arr2{};

        arr1[0].clear();   // has_value = false, storage still contains "16"
        arr1[1].clear();
        // arr2[0] is default constructed: has_value = false, storage 0-initialized

        REQUIRE(arr1 == arr2); // Equality only depends on has_value and raw storage when present.
        // Since both are empty, they compare equal even if storage differs.
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

TEST_CASE("string_view from_literal correctness") {
    using jh::pod::string_view;
    // case 1: simple literal
    constexpr auto sv = string_view::from_literal("hello");
    static_assert(sv.size() == 5, "string_view size from literal should be strlen(literal)");

    REQUIRE(sv.size() == 5);
    REQUIRE(sv == string_view{"hello", std::strlen("hello")});

    // case 2: empty string literal
    constexpr auto sv_empty = string_view::from_literal("");
    static_assert(sv_empty.empty());
    REQUIRE(sv_empty.empty());
    REQUIRE(sv_empty == string_view{"", std::strlen("")});

    // case 3: longer literal
    constexpr auto sv_long = string_view::from_literal("hello_pod_world");
    REQUIRE(sv_long.size() == std::strlen("hello_pod_world"));
    REQUIRE(sv_long == string_view{"hello_pod_world", std::strlen("hello_pod_world")});
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

TEST_CASE("bytes_view hash reflects exact byte content") {
    using jh::pod::bytes_view;
    using jh::pod::array;
    using jh::utils::hash_fn::c_hash;

    SECTION("Equal content produces same hash") {
        array<std::uint32_t, 4> a = {1, 2, 3, 4};
        std::array<std::uint32_t, 4> b = {1, 2, 3, 4};

        auto va = bytes_view::from(a);
        auto vb = bytes_view::from(b.data(), b.size());

        REQUIRE(va == vb);
        REQUIRE(va.hash() == vb.hash());
    }

    SECTION("Different content produces different hash") {
        array<std::uint32_t, 4> a = {1, 2, 3, 4};
        array<std::uint32_t, 4> c = {4, 3, 2, 1};

        auto va = bytes_view::from(a);
        auto vc = bytes_view::from(c);

        REQUIRE(va != vc);
        REQUIRE(va.hash() != vc.hash());
    }

    SECTION("Same layout different values changes hash") {
        struct P {
            std::uint32_t x;
        };

        P p1{0x11223344};
        P p2{0x55667788};

        auto h1 = bytes_view::from(p1).hash();
        auto h2 = bytes_view::from(p2).hash();

        REQUIRE(h1 != h2);
    }

    SECTION("pod::string_view vs bytes_view with same content") {
        using jh::pod::string_view;

        constexpr char raw[] = "hash_check_test";
        string_view sv{raw, sizeof(raw) - 1};
        auto bv = bytes_view::from(raw, sizeof(raw) - 1);

        REQUIRE(sv.size() == bv.len);
        REQUIRE(std::memcmp(sv.data, bv.data, sv.size()) == 0);
        REQUIRE(sv.hash() == bv.hash());
    }
}

TEST_CASE("string_view hash reflects exact character content") {
    using jh::pod::string_view;
    using jh::utils::hash_fn::c_hash;

    static constexpr char content1[] = "alpha_test";
    static constexpr char content2[] = "alpha_test";  // same content, different instance
    static constexpr char content3[] = "beta_test";

    const string_view sv1{content1, std::strlen(content1)};
    const string_view sv2{content2, std::strlen(content2)};
    const string_view sv3{content3, std::strlen(content3)};

    SECTION("Equal content produces same hash") {
        REQUIRE(sv1 == sv2);
        REQUIRE(sv1.hash() == sv2.hash());
    }

    SECTION("Different content produces different hash") {
        REQUIRE(sv1 != sv3);
        REQUIRE(sv1.hash() != sv3.hash());
    }

    SECTION("Hash consistency across methods") {
        auto h1 = sv1.hash(c_hash::fnv1a64);
        auto h2 = sv1.hash(c_hash::djb2);
        auto h3 = sv1.hash(c_hash::sdbm);
        auto h4 = sv1.hash(c_hash::fnv1_64);

        REQUIRE(h1 != -1);
        REQUIRE(h2 != -1);
        REQUIRE(h3 != -1);
        REQUIRE(h4 != -1);

        // Same view, multiple algorithms must differ
        REQUIRE(h1 != h2);
        REQUIRE(h2 != h3);
        REQUIRE(h3 != h4);
    }

    SECTION("string_view vs bytes_view from same buffer") {
        auto bv = jh::pod::bytes_view::from(content1, sizeof(content1) - 1);
        REQUIRE(sv1.hash() == bv.hash());
    }
}

namespace user_override {

    std::ostream &operator<<(std::ostream &os, const jh::pod::array<int, 3> &) {
        os << "user_defined_override";
        return os;
    }

} // namespace user_override

TEST_CASE("User-defined operator<< overrides default inline", "[ostream_override]") {
    std::ostringstream os1, os2;

    SECTION("user override active") {
        using user_override::operator<<;
        jh::pod::array<int, 3> a{};
        os1 << a;
        REQUIRE(os1.str() == "user_defined_override");
    }

    SECTION("default inline remains active when no override") {
        jh::pod::array<int, 5> b{};
        os2 << b;
        REQUIRE(os2.str() == "[0, 0, 0, 0, 0]");
    }
}

TEST_CASE("pod::ostream << overloads for built-in and custom POD types", "[ostream]") {
    using jh::pod::array;
    using jh::pod::pair;
    using jh::pod::optional;
    using jh::pod::bitflags;
    using jh::pod::bytes_view;
    using jh::pod::span;
    using jh::pod::string_view;
    using jh::typed::monostate;
    using jh::pod::tuple;

    SECTION("array<T, N> general printable") {
        array<int, 3> a = {1, 2, 3};
        std::ostringstream oss;
        oss << a;
        REQUIRE(oss.str() == "[1, 2, 3]");
    }

    SECTION("array<char, N> as escaped JSON string") {
        array<char, 6> s = {"hello"};
        std::ostringstream oss;
        oss << s;
        REQUIRE(oss.str() == "\"hello\"");
    }

    SECTION("pair<T1, T2>") {
        pair<int, float> p = {42, 3.14f};
        std::ostringstream oss;
        oss << p;
        REQUIRE(oss.str() == "{42, 3.14}");
    }

    SECTION("optional<T> with and without value") {
        std::ostringstream oss1, oss2;
        optional<int> o1{};
        o1.store(7);
        optional<int> o2{};
        oss1 << o1;
        oss2 << o2;
        REQUIRE(oss1.str() == "7");
        REQUIRE(oss2.str() == "nullopt");
    }

    SECTION("bitflags<N> output in hex format") {
        std::ostringstream oss;
        bitflags<8> f{};
        f.set(0);
        f.set(3);
        f.set(7);  // binary: 10001001 → hex: 0x'89
        oss << std::hex << f;
        REQUIRE(oss.str() == "0x'89'");
    }

    SECTION("bitflags<N> output in binary format") {
        std::ostringstream oss;
        bitflags<8> f{};
        f.set(1);
        f.set(2);
        oss << std::dec << f;
        REQUIRE(oss.str() == "0b'00000110'");
    }

    SECTION("bytes_view outputs base64") {
        std::ostringstream oss;
        const uint8_t raw[] = {0x48, 0x65, 0x6c, 0x6c, 0x6f};  // "Hello"
        bytes_view bv = bytes_view::from(raw, 5);
        oss << bv;
        REQUIRE(oss.str().starts_with("base64'"));
        REQUIRE(oss.str().ends_with("'"));
    }

    SECTION("span<T> prints container-like output") {
        array<int, 4> arr = {1, 2, 3, 4};
        span<int> sp{arr.data, std::size(arr)};
        std::ostringstream oss;
        oss << sp;
        REQUIRE(oss.str().starts_with("span<"));
        REQUIRE(oss.str().find("[1, 2, 3, 4]") != std::string::npos);
    }

    SECTION("string_view outputs quoted content") {
        constexpr char raw[] = "pod_string";
        string_view sv{raw, strlen(raw)};
        std::ostringstream oss;
        oss << sv;
        REQUIRE(oss.str() == "string_view\"pod_string\"");
    }

    SECTION("typed::monostate prints as null") {
        monostate m{};
        std::ostringstream oss;
        oss << m;
        REQUIRE(oss.str() == "null");
    }

    SECTION("pod::tuple ostream output formats correctly") {
        using jh::pod::make_tuple;

        std::ostringstream oss0, oss1, oss5;

        tuple<> t0{};
        tuple<int> t1{{ {42}, {} }};
        auto t5 = make_tuple(1, 2, 3, 4, 5);

        oss0 << t0;
        oss1 << t1;
        oss5 << t5;

        REQUIRE(oss0.str() == "()");
        REQUIRE(oss1.str() == "(42,)");
        REQUIRE(oss5.str() == "(1, 2, 3, 4, 5)");
    }
}

TEST_CASE("pod::array works with std::views pipelines") {
    using jh::pod::array;

    array<int, 6> arr = {{1, 2, 3, 4, 5, 6}};
    auto even = arr
                | std::views::filter([](int x) { return x % 2 == 0; })
                | std::views::transform([](int x) { return x * 10; });

    std::vector<int> result;
    std::ranges::copy(even, std::back_inserter(result));

    REQUIRE(result == std::vector<int>{20, 40, 60});
}

TEST_CASE("pod::array supports structured bindings") {
    using jh::pod::array;

    array<int, 3> arr = {{10, 20, 30}};
    auto &[a, b, c] = arr;
    REQUIRE(a == arr.data[0]);
    REQUIRE(b == arr.data[1]);
    REQUIRE(c == arr.data[2]);

    a = 42;
    REQUIRE(arr[0] == 42);
}

TEST_CASE("pod::make_pair and direct pair construction produce same result") {
    using jh::pod::pair;
    using jh::pod::make_pair;

    pair<int, double> p1{1, 2.5};
    auto p2 = make_pair(1, 2.5);

    REQUIRE(p1 == p2);
}

TEST_CASE("pod::tuple construction: nested braces vs make_tuple") {
    using jh::pod::tuple;
    using jh::pod::make_tuple;

    tuple<int, double> t1{{ {7}, {{3.14}, {}} }};
    auto t2 = make_tuple(7, 3.14);

    REQUIRE(t1 == t2);
}

TEST_CASE("bitflags native type operator return types are self type") {
    using namespace jh::pod;

    SECTION("operator| returns self type") {
        STATIC_REQUIRE(std::is_same_v<decltype(std::declval<bitflags<8>>()  | std::declval<bitflags<8>>()),  bitflags<8>>);
        STATIC_REQUIRE(std::is_same_v<decltype(std::declval<bitflags<16>>() | std::declval<bitflags<16>>()), bitflags<16>>);
        STATIC_REQUIRE(std::is_same_v<decltype(std::declval<bitflags<32>>() | std::declval<bitflags<32>>()), bitflags<32>>);
        STATIC_REQUIRE(std::is_same_v<decltype(std::declval<bitflags<64>>() | std::declval<bitflags<64>>()), bitflags<64>>);
    }

    SECTION("operator& returns self type") {
        STATIC_REQUIRE(std::is_same_v<decltype(std::declval<bitflags<8>>()  & std::declval<bitflags<8>>()),  bitflags<8>>);
        STATIC_REQUIRE(std::is_same_v<decltype(std::declval<bitflags<16>>() & std::declval<bitflags<16>>()), bitflags<16>>);
        STATIC_REQUIRE(std::is_same_v<decltype(std::declval<bitflags<32>>() & std::declval<bitflags<32>>()), bitflags<32>>);
        STATIC_REQUIRE(std::is_same_v<decltype(std::declval<bitflags<64>>() & std::declval<bitflags<64>>()), bitflags<64>>);
    }

    SECTION("operator^ returns self type") {
        STATIC_REQUIRE(std::is_same_v<decltype(std::declval<bitflags<8>>()  ^ std::declval<bitflags<8>>()),  bitflags<8>>);
        STATIC_REQUIRE(std::is_same_v<decltype(std::declval<bitflags<16>>() ^ std::declval<bitflags<16>>()), bitflags<16>>);
        STATIC_REQUIRE(std::is_same_v<decltype(std::declval<bitflags<32>>() ^ std::declval<bitflags<32>>()), bitflags<32>>);
        STATIC_REQUIRE(std::is_same_v<decltype(std::declval<bitflags<64>>() ^ std::declval<bitflags<64>>()), bitflags<64>>);
    }

    SECTION("operator~ returns self type") {
        STATIC_REQUIRE(std::is_same_v<decltype(~std::declval<bitflags<8>>()),  bitflags<8>>);
        STATIC_REQUIRE(std::is_same_v<decltype(~std::declval<bitflags<16>>()), bitflags<16>>);
        STATIC_REQUIRE(std::is_same_v<decltype(~std::declval<bitflags<32>>()), bitflags<32>>);
        STATIC_REQUIRE(std::is_same_v<decltype(~std::declval<bitflags<64>>()), bitflags<64>>);
    }
}

TEST_CASE("pod::string_view explicit conversion and to_std() behave identically") {
    using jh::pod::string_view;

    static constexpr char raw[] = "conversion_test";
    constexpr std::uint64_t len = sizeof(raw) - 1;
    const string_view sv{raw, len};

    const std::string_view a = static_cast<std::string_view>(sv); // explicit conversion
    const std::string_view b = sv.to_std();                        // to_std() method

    SECTION("Data pointer and size must match") {
        REQUIRE(a.data() == b.data());
        REQUIRE(a.size() == b.size());
    }

    SECTION("Content equality check") {
        REQUIRE(a == b);
        REQUIRE(a == "conversion_test");
    }

    SECTION("Both produce same hash via std::hash") {
        const auto ha = std::hash<std::string_view>{}(a);
        const auto hb = std::hash<std::string_view>{}(b);
        REQUIRE(ha == hb);
    }
}
