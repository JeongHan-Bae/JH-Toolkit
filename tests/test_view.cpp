#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include <vector>
#include <string>
#include <sstream>

#include "jh/views"
#include "jh/pod.h"
#include "jh/runtime_arr.h"

// ------------------------------
//  local helper types
// ------------------------------
namespace test {

    struct DummyIter {
        int i = 0;
        int operator*() const { return i; }
        DummyIter& operator++() { ++i; return *this; }
        DummyIter operator++(int) { auto tmp = *this; ++i; return tmp; }
        bool operator==(const DummyIter& other) const { return i == other.i; }
    };

    struct MySeq {
        static DummyIter begin() { return DummyIter{0}; }
        static DummyIter end() { return DummyIter{5}; }
    };

    struct DummyWriteIter {
        int i = 0;
        int* target = nullptr;
        int& operator*() const noexcept { return *target; }
        DummyWriteIter& operator++() noexcept { ++target; ++i; return *this; }
        DummyWriteIter operator++(int) noexcept { auto tmp = *this; ++*this; return tmp; }
        bool operator==(const DummyWriteIter& other) const noexcept { return target == other.target; }
    };

    struct MyWritableSeq {
        int buf[5]{};
        DummyWriteIter begin() { return {0, buf}; }
        DummyWriteIter end() { return {5, buf + 5}; }
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
    for (auto [i, x] : jh::views::enumerate(s, 100)) {
        out << i << ":" << x << " ";
    }

    REQUIRE(out.str() == "100:0 101:1 102:2 103:3 104:4 ");
}

/// @brief test for non-standard write sequence
TEST_CASE("enumerate write then read", "[enumerate][rw]") {
    using test::MyWritableSeq;

    MyWritableSeq seq{};
    for (auto [i, x] : jh::views::enumerate(seq, 100)) {
        x = i * 10;
    }

    std::ostringstream out;
    for (auto [i, x] : jh::views::enumerate(seq, 100)) {
        out << i << ":" << x << " ";
    }

    REQUIRE(out.str() == "100:1000 101:1010 102:1020 103:1030 104:1040 ");
}

/// @brief test for immovable range
TEST_CASE("enumerate immovable seq", "[enumerate][immov]") {
    jh::runtime_arr<int> arr{3};
    for (auto [i, x] : jh::views::enumerate(arr, 0)) {
        x = i + 1;
    }

    std::ostringstream out;
    for (auto [i, x] : jh::views::enumerate(arr, 0)) {
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
    for (const auto& [a, b] : zipped) {
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
    const std::vector<std::pair<int, int>> expect = {{1,10},{2,20},{3,30}};

    std::size_t i = 0;
    for (auto p : zipped) {
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
    for (auto [idx, n, w, d] : zipped) {
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
