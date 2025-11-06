#define CATCH_CONFIG_MAIN

#include <catch2/catch_all.hpp>
#include <vector>
#include <string>
#include <sstream>
#include <vector>
#include <string>
#include <type_traits>
#include <set>
#include <list>
#include <deque>
#include <stack>
#include <memory_resource>

#include "jh/ranges/views/flatten.h" // force include to compact with 1.3.x-support-dev branch
#include "jh/views"
#include "jh/pod"
#include "jh/runtime_arr.h"
#include "jh/ranges_ext"    // temporarily located ranges_ext tests here for 1.3.x series

// ------------------------------
//  local helper types
// ------------------------------
namespace test {

    struct DummyIter {
        int i = 0;

        int operator*() const { return i; }

        DummyIter &operator++() {
            ++i;
            return *this;
        }

        DummyIter operator++(int) {
            auto tmp = *this;
            ++i;
            return tmp;
        }

        bool operator==(const DummyIter &other) const { return i == other.i; }
    };

    struct MySeq {
        static DummyIter begin() { return DummyIter{0}; }

        static DummyIter end() { return DummyIter{5}; }
    };

    struct DummyWriteIter {
        int i = 0;
        int *target = nullptr;

        int &operator*() const noexcept { return *target; }

        DummyWriteIter &operator++() noexcept {
            ++target;
            ++i;
            return *this;
        }

        DummyWriteIter operator++(int) noexcept {
            auto tmp = *this;
            ++*this;
            return tmp;
        }

        bool operator==(const DummyWriteIter &other) const noexcept { return target == other.target; }
    };

    struct MyWritableSeq {
        int buf[5]{};

        DummyWriteIter begin() noexcept { return {0, buf}; }

        DummyWriteIter end() noexcept { return {5, buf + 5}; }

        [[nodiscard]] DummyWriteIter begin() const noexcept { return {0, const_cast<int *>(buf)}; }

        [[nodiscard]] DummyWriteIter end() const noexcept { return {5, const_cast<int *>(buf + 5)}; }
    };
} // namespace test

// =======================================================
//  enumerate tests
// =======================================================

/// @brief test for non-standard read sequence
TEST_CASE("enumerate read only seq", "[enumerate][ro]") {
    using test::MySeq;

    const MySeq s{};
    std::ostringstream out;
    for (auto [i, x]: jh::views::enumerate(s, 100)) {
        out << i << ":" << x << " ";
    }

    REQUIRE(out.str() == "100:0 101:1 102:2 103:3 104:4 ");
}

/// @brief test for non-standard write sequence
TEST_CASE("enumerate write then read", "[enumerate][rw]") {
    using test::MyWritableSeq;

    MyWritableSeq seq{};
    for (auto [i, x]: jh::views::enumerate(seq, 100)) {
        x = static_cast<int>(i * 10);
    }

    std::ostringstream out;
    for (auto [i, x]: jh::views::enumerate(seq, 100)) {
        out << i << ":" << x << " ";
    }

    REQUIRE(out.str() == "100:1000 101:1010 102:1020 103:1030 104:1040 ");
}

/// @brief test for immovable range
TEST_CASE("enumerate immovable seq", "[enumerate][immov]") {
    jh::runtime_arr<int> arr(3);
    REQUIRE(arr.size() == 3);
    for (auto [i, x]: jh::views::enumerate(arr, 0)) {
        x = static_cast<int>(i + 1);
    }

    std::ostringstream out;
    for (auto [i, x]: jh::views::enumerate(arr, 0)) {
        out << i << ":" << x << " ";
    }

    REQUIRE(out.str() == "0:1 1:2 2:3 ");
}


// =======================================================
//  zip tests (from old suite)
// =======================================================
TEST_CASE("zip two seq", "[zip]") {
    jh::pod::array<int, 4> nums = {1, 2, 3, 4};
    std::vector<std::string> words = {"one", "two", "three", "four"};

    const auto zipped = jh::views::zip(nums, words);
    std::size_t i = 0;
    for (const auto &[a, b]: zipped) {
        REQUIRE(a == nums[i]);
        REQUIRE(b == words[i]);
        ++i;
    }
    REQUIRE(i == nums.size());
}

TEST_CASE("zip trunc to shorter", "[zip][truncate]") {
    const jh::pod::array<int, 5> a = {1, 2, 3, 4, 5};
    constexpr jh::pod::array<int, 3> b = {10, 20, 30};

    const auto zipped = jh::views::zip(a, b);
    const std::vector<std::pair<int, int>> expect = {{1, 10},
                                                     {2, 20},
                                                     {3, 30}};

    std::size_t i = 0;
    for (auto p: zipped) {
        REQUIRE(p.get<0>() == expect[i].first);
        REQUIRE(p.get<1>() == expect[i].second);
        ++i;
    }
    REQUIRE(i == expect.size());
}

/// @brief test for multi-sequence zip (diff lengths)
TEST_CASE("zip multi trunc", "[zip][multi][truncate]") {
    jh::pod::array<int, 5> nums = {10, 20, 30, 40, 50};
    const std::vector<std::string> words = {"ten", "twenty", "thirty"};
    std::vector<double> doubles = {1.1, 2.2, 3.3, 4.4};
    auto index = std::views::iota(0);

    const auto zipped = jh::views::zip(index, nums, words, doubles);

    std::vector<int> seen_idx;
    std::size_t i = 0;
    for (auto [idx, n, w, d]: zipped) {
        seen_idx.push_back(idx);
        REQUIRE(idx == static_cast<int>(i));
        REQUIRE(n == nums[i]);
        REQUIRE(w == words[i]);
        REQUIRE(d == doubles[i]);
        ++i;
    }

    REQUIRE(i == 3);
    REQUIRE(seen_idx.size() == 3);
    REQUIRE(seen_idx == std::vector<int>{0, 1, 2});
}

/// @brief Combined test for enumerate + zip adaptors using pipe syntax.
/// @details
/// Verifies that chained view adaptors correctly produce nested tuples:
/// - `enumerate` yields `(index, element)`
/// - `zip` packs multiple ranges into tuple-of-tuples
TEST_CASE("enumerate and zip combined with pipes", "[enumerate][zip][pipe]") {
    // ------------------------------------------------------
    // Setup input sequences
    // ------------------------------------------------------
    jh::runtime_arr<int> numbers(5);
    jh::runtime_arr<std::string> words(5);

    // Initialize numbers: 10, 20, 30, 40, 50
    for (auto [i, x]: numbers | jh::views::enumerate(1)) {
        x = static_cast<int>(i * 10);
    }

    // Initialize words
    words[0] = "ten";
    words[1] = "twenty";
    words[2] = "thirty";
    words[3] = "forty";
    words[4] = "fifty";

    // ------------------------------------------------------
    // Pipeline: enumerate + zip
    // ------------------------------------------------------
    std::ostringstream out;

    // Note:
    // `numbers | enumerate(100) | zip(words)` produces elements of type:
    //   std::tuple< std::tuple<index, value>, word >
    for (auto [pair, word]: numbers
                            | jh::views::enumerate(100)
                            | jh::views::zip(words)) {
        auto [idx, num] = pair;
        out << "(" << idx << "," << num << "," << word << ") ";
    }

    REQUIRE(out.str() ==
            "(100,10,ten) (101,20,twenty) (102,30,thirty) (103,40,forty) (104,50,fifty) ");

    // ------------------------------------------------------
    // Direct chained adaptors (non-pipe)
    // ------------------------------------------------------
    // Equivalent structure: zip( enumerate(numbers, 200), words )
    const auto combined = jh::views::zip(
            jh::views::enumerate(numbers, 200),
            words
    );

    std::ostringstream out2;
    for (auto [pair, word]: combined) {
        auto [idx, num] = pair;
        out2 << "[" << idx << ":" << num << ":" << word << "] ";
    }

    REQUIRE(out2.str() ==
            "[200:10:ten] [201:20:twenty] [202:30:thirty] [203:40:forty] [204:50:fifty] ");
}

/// @brief Complex pipeline test: zip with four sequences of different lengths.
/// @details
/// Demonstrates correct short-circuiting and structured binding when
/// combining multiple adaptors with different container lengths.
/// The resulting range should terminate at the shortest underlying range.
TEST_CASE("zip multiple sequences with pipes", "[zip][pipe][multi]") {
    // ------------------------------------------------------
    // Setup input sequences
    // ------------------------------------------------------
    jh::runtime_arr<int> ids(5);
    jh::runtime_arr<std::string> words(4);
    std::vector<double> prices = {1.1, 2.2, 3.3};
    std::vector<char> grades = {'A', 'B', 'C', 'D', 'E'};

    // Fill ids: 10, 20, 30, 40, 50
    for (auto [i, x]: ids | jh::views::enumerate(1)) {
        x = static_cast<int>(i * 10);
    }

    // Fill words
    words[0] = "apple";
    words[1] = "banana";
    words[2] = "carrot";
    words[3] = "durian";

    // ------------------------------------------------------
    // Pipeline composition:
    // ids | enumerate(100) | zip(words, prices, grades)
    // ------------------------------------------------------
    std::ostringstream out;

    for (auto [pair, word, price, grade]:
            ids
            | jh::views::enumerate(100)
            | jh::views::zip_pipe(words, prices, grades)) {
        auto [idx, id] = pair;
        out << "(" << idx << "," << id << "," << word
            << "," << price << "," << grade << ") ";
    }

    // The shortest input is `prices` (3 elements)
    // → only 3 iterations expected.
    REQUIRE(out.str() ==
            "(100,10,apple,1.1,A) "
            "(101,20,banana,2.2,B) "
            "(102,30,carrot,3.3,C) ");
}

TEST_CASE("adapt runtime_arr streamable", "[adapt][runtime_arr]") {
    jh::runtime_arr<int> arr(3);
    for (auto [i, x]: arr | jh::ranges::views::enumerate(1)) {
        x = static_cast<int>(i * 10);
    }

    std::ostringstream out;
    for (auto x: arr | jh::ranges::adapt() | std::views::all) {
        out << x << " ";
    }
    // check arr can be adapted and streamed correctly
    // (non-copyable ranges are normally reject by std::views::all)

    REQUIRE(out.str() == "10 20 30 ");
}

TEST_CASE("flatten after enumerate+zip_pipe", "[flatten][zip][enumerate]") {
    jh::runtime_arr<int> ids(3);
    jh::runtime_arr<std::string> names(3);

    for (auto [i, x]: ids | jh::ranges::views::enumerate(1))
        x = static_cast<int>(i * 10);
    names[0] = "Alice";
    names[1] = "Bob";
    names[2] = "Carol";

    auto zipped =
            ids | jh::ranges::views::enumerate(100) | jh::ranges::views::zip_pipe(names);

    // flatten the nested tuples into single-level tuples
    std::ostringstream out;
    for (auto e: zipped | jh::ranges::views::flatten()) {
        auto [i, v, n] = e;
        out << "(" << i << "," << v << "," << n << ") ";
    }

    REQUIRE(out.str() == "(100,10,Alice) (101,20,Bob) (102,30,Carol) ");
}

TEST_CASE("adapt: direct call vs pipe form equivalence", "[adapt][equiv]") {
    jh::runtime_arr<int> arr(3);
    for (auto [i, x]: arr | jh::ranges::views::enumerate(1))
        x = static_cast<int>(i * 10);

    // direct call
    auto r1 = jh::ranges::adapt(arr);
    std::ostringstream out1;
    for (auto x: r1) out1 << x << " ";

    // pipe form
    std::ostringstream out2;
    for (auto x: arr | jh::ranges::adapt()) out2 << x << " ";

    REQUIRE(out1.str() == out2.str());
    REQUIRE(out1.str() == "10 20 30 ");
}

TEST_CASE("flatten: direct call vs pipe form equivalence", "[flatten][equiv]") {
    jh::runtime_arr<int> a(3);
    jh::runtime_arr<std::string> b(3);

    for (auto [i, x]: a | jh::ranges::views::enumerate(1))
        x = static_cast<int>(i * 10);
    b[0] = "A";
    b[1] = "B";
    b[2] = "C";

    // Build zipped enumerate view
    auto zipped = jh::ranges::views::zip(
            jh::ranges::views::enumerate(a, 100),
            b
    );

    // direct flatten
    std::ostringstream out1;
    for (auto e: jh::ranges::views::flatten(zipped)) {
        auto [i, v, s] = e;
        out1 << "(" << i << "," << v << "," << s << ") ";
    }

    // pipe flatten
    std::ostringstream out2;
    for (auto e: zipped | jh::ranges::views::flatten()) {
        auto [i, v, s] = e;
        out2 << "(" << i << "," << v << "," << s << ") ";
    }

    REQUIRE(out1.str() == out2.str());
    REQUIRE(out1.str() == "(100,10,A) (101,20,B) (102,30,C) ");
}

TEST_CASE("flatten deep nested enumerate+zip_pipe", "[flatten][nested]") {
    jh::runtime_arr<int> ids(3);
    jh::runtime_arr<std::string> names(3);
    jh::runtime_arr arrays = {
            jh::pod::array{{10, 20, 30}},
            jh::pod::array{{40, 50, 60}},
            jh::pod::array{{70, 80, 90}}
    };
    for (auto [i, x]: ids | jh::ranges::views::enumerate(1))
        x = static_cast<int>(i * 10);
    names[0] = "A";
    names[1] = "B";
    names[2] = "C";

    // inner enumerate for duplication
    auto inner = ids | jh::ranges::views::enumerate(10);

    // deep nested: ( (i,v), name, (j,v2) )
    auto zipped =
            ids | jh::ranges::views::enumerate(100)
            | jh::ranges::views::zip_pipe(names, inner, arrays);

    std::ostringstream out;
    for (auto e: zipped | jh::ranges::views::flatten()) {
        auto [i, v, n, j, v2, a0, a1, a2] = e;
        out << "(" << i << "," << v << "," << n << "," << j << ","
            << v2 << "," << a0 << "," << a1 << "," << a2 << ") ";
    }

    REQUIRE(out.str() ==
            "(100,10,A,10,10,10,20,30) (101,20,B,11,20,40,50,60) (102,30,C,12,30,70,80,90) ");
}

TEST_CASE("constexpr flatten_proxy recursion and tuple_materialize", "[flatten][meta][constexpr]") {
    using jh::meta::flatten_proxy;
    using jh::pod::make_tuple;

    constexpr auto t = std::tuple{
            std::tuple{1, 2},
            make_tuple(3, 4),
            std::tuple{make_tuple(5, 6), 7},
    };

    constexpr auto fp = flatten_proxy{t};

    constexpr auto m = jh::meta::tuple_materialize(t);

    static_assert(std::tuple_size_v<decltype(fp)> == 7);
    static_assert(std::tuple_size_v<decltype(m)> == 7);

    constexpr auto check = []<typename Tup>(const Tup &tup) {
        const auto &[a, b, c, d, e, f, g] = tup;
        return a == 1 && b == 2 && c == 3 && d == 4 && e == 5 && f == 6 && g == 7;
    };

    static_assert(check(fp));
    static_assert(check(m));

    std::ostringstream out;
    const auto [a, b, c, d, e, f, g] = fp;
    out << a << "," << b << "," << c << "," << d << "," << e << "," << f << "," << g;
    REQUIRE(out.str() == "1,2,3,4,5,6,7");
}

struct DeclaredOnly {
    using value_type = double;
};

struct DeducedOnly {
    int *begin();

    int *end();
};

struct CompatibleProxy {
    using value_type = bool;

    struct proxy {
        operator bool() const { return true; }
    };

    proxy *begin();

    proxy *end();
};

struct Conflict {
    using value_type = int;

    struct proxy {
        operator std::string() const { return {}; }
    };

    proxy *begin();

    proxy *end();
};

struct Voidish {
};

class my_vector : public std::vector<char> {
};

template<>
struct jh::container_deduction<my_vector> {
    using value_type = unsigned char;
};

// ---- static trait checks ----
static_assert(std::same_as<jh::concepts::container_value_t<DeclaredOnly>, double>);
static_assert(std::same_as<jh::concepts::container_value_t<DeducedOnly>, int>);
static_assert(std::same_as<jh::concepts::container_value_t<CompatibleProxy>, bool>);
static_assert(std::same_as<jh::concepts::container_value_t<Conflict>, void>);
static_assert(std::same_as<jh::concepts::container_value_t<Voidish>, void>);
static_assert(std::same_as<jh::concepts::container_value_t<my_vector>, unsigned char>);

// ---- closable_container_for static checks ----
static_assert(jh::concepts::closable_container_for<std::vector<int>, std::vector<int>>);
static_assert(jh::concepts::closable_container_for<std::set<int>, std::vector<int>>);
static_assert(jh::concepts::closable_container_for<std::vector<double>, std::vector<int>>);
static_assert(!jh::concepts::closable_container_for<std::vector<std::string>, std::vector<int>>);

// =======================================================
//  dynamic collect / to verification
// =======================================================

TEST_CASE("collect and to dynamic examples", "[collect][to][dynamic]") {
    std::vector<int> v = {1, 2, 3};

    // normal
    auto s = jh::ranges::to<std::set<int>>(v);
    auto dq = jh::ranges::to<std::deque<double>>(v);
    auto st = jh::ranges::to<std::stack<int>>(v);
    REQUIRE(s.size() == 3);
    REQUIRE(dq.size() == 3);
    REQUIRE(st.size() == 3);

    // allocator-aware
    std::pmr::monotonic_buffer_resource pool;
    std::pmr::polymorphic_allocator<int> alloc(&pool);
    auto pmr_vec = jh::ranges::to<std::pmr::vector<int>>(v, alloc);
    auto p2 = v | jh::ranges::to<std::pmr::vector<int>>(alloc);
    REQUIRE(pmr_vec.size() == v.size());
    REQUIRE(p2.size() == v.size());

    // pipeline sanity
    std::vector<int> input = {1, 2, 3};
    std::vector<char> other = {'a', 'b', 'c'};

    // collect and adapt chain
    auto aaaa = input
                | jh::ranges::to<std::vector<int>>()
                | jh::ranges::collect<std::set<int>>()
                | jh::ranges::adapt();

    std::ostringstream out_a;
    for (auto x: aaaa) out_a << x << " ";
    REQUIRE(out_a.str() == "1 2 3 ");

    // flatten and zip
    auto bbbb = input
                | jh::ranges::views::zip(other)
                | jh::ranges::to<std::vector<std::tuple<int, char>>>();

    std::ostringstream out_b;
    for (auto &[a, b]: bbbb)
        out_b << "(" << a << "," << b << ") ";
    REQUIRE(out_b.str() == "(1,a) (2,b) (3,c) ");

    // enumerate + flatten
    std::ostringstream out_c;
    for (auto &&[i, ch0, ch1]:
            input
            | jh::ranges::views::zip(other)
            | jh::ranges::views::enumerate(100)
            | jh::ranges::views::flatten()) {
        out_c << i << ":(" << ch0 << "," << ch1 << ") ";
    }
    REQUIRE(out_c.str() == "100:(1,a) 101:(2,b) 102:(3,c) ");

    // collect + to chained
    auto rg = input
              | jh::ranges::views::zip(other)
              | jh::ranges::views::enumerate(100)
              | jh::ranges::views::flatten()
              | jh::ranges::collect<std::vector<std::tuple<int, char, char>>>()
              | jh::ranges::to<std::deque<std::tuple<int, char, char>>>();

    REQUIRE(rg.size() == 3);
    auto [i0, c0, c1] = rg.front();
    REQUIRE(i0 == 100);
}

TEST_CASE("collect to unordered_map from vector of tuple", "[collect][unordered_map]") {
    // ------------------------------------------------------
    // Prepare input: vector of tuple<key, value>
    // ------------------------------------------------------
    std::vector<std::tuple<std::string, int>> pairs = {
            {"apple",  10},
            {"banana", 20},
            {"carrot", 30}
    };

    // ------------------------------------------------------
    // Use collect() to produce unordered_map
    // ------------------------------------------------------
    auto map1 = pairs | jh::ranges::collect<std::unordered_map<std::string, int>>();

    REQUIRE(map1.size() == 3);
    REQUIRE(map1["apple"] == 10);
    REQUIRE(map1["banana"] == 20);
    REQUIRE(map1["carrot"] == 30);

    // ------------------------------------------------------
    // Check that collect<> also works directly
    // ------------------------------------------------------
    auto map2 = jh::ranges::collect<std::unordered_map<std::string, int>>(pairs);
    REQUIRE(map2 == map1);

    // ------------------------------------------------------
    // Pipeline chain with adapt (ensures it's view-compatible)
    // ------------------------------------------------------
    std::ostringstream out;
    for (auto &&[k, v]: pairs
                        | jh::ranges::collect<std::unordered_map<std::string, int>>()
                        | jh::ranges::adapt()) {
        out << "(" << k << "," << v << ") ";
    }

    // The order of unordered_map is unspecified — just check presence
    std::string result = out.str();
    REQUIRE(result.find("(apple,10)") != std::string::npos);
    REQUIRE(result.find("(banana,20)") != std::string::npos);
    REQUIRE(result.find("(carrot,30)") != std::string::npos);
}

TEST_CASE("collect+to continuous chain to pmr::unordered_map", "[collect][to][pmr][unordered_map]") {
    std::vector<std::string> input = {
            {"apple"},
            {"banana"},
            {"cherry"}
    };

    std::pmr::monotonic_buffer_resource pool;
    std::pmr::polymorphic_allocator<std::pair<const std::string, int>> alloc(&pool);

    auto result = input
                  | jh::ranges::views::enumerate()
                  | jh::ranges::collect<std::vector<std::pair<size_t, std::string>>>()
                  | jh::ranges::to<std::pmr::unordered_map<size_t, std::string>>(
            0,
            std::hash<size_t>{},
            std::equal_to<size_t>{},
            alloc
    );
    REQUIRE(result.size() == 3);
    REQUIRE(result[0] == "apple");
    REQUIRE(result[1] == "banana");
    REQUIRE(result[2] == "cherry");
}
