#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include <vector>
#include <string>
#include <sstream>

#include "jh/views.h"
#include "jh/pod.h"

TEST_CASE("jh::views::enumerate produces correct index-value pairs", "[enumerate]") {
    jh::pod::array<char, 4> chars = {'a', 'b', 'c', 'd'};

    const auto enumerated = jh::views::enumerate(chars);
    std::uint64_t index = 0;

    for (auto [first, second] : enumerated) {
        REQUIRE(first == index);
        REQUIRE(second == chars[index]);
        ++index;
    }

    REQUIRE(index == chars.size());
}

TEST_CASE("jh::views::enumerate produces correct index-value ref pairs", "[enumerate]") {
    std::vector<std::string> words = {"one", "two", "three", "four"};

    const auto enumerated = jh::views::enumerate(words);
    std::uint64_t index = 0;

    for (auto [first, second] : enumerated) {
        REQUIRE(first == index);
        REQUIRE(second == words[index]);
        ++index;
    }

    REQUIRE(index == words.size());
}

TEST_CASE("jh::views::zip combines two sequences correctly", "[zip]") {
    jh::pod::array<int, 4> nums = {1, 2, 3, 4};
    std::vector<std::string> words = {"one", "two", "three", "four"};

    const auto zipped = jh::views::zip(nums, words);

    std::size_t i = 0;
    for (const auto& [first, second] : zipped) {
        REQUIRE(first == nums[i]);
        REQUIRE(second == words[i]);
        ++i;
    }

    REQUIRE(i == nums.size());
}

TEST_CASE("jh::views::zip truncates to shorter sequence", "[zip][truncate]") {
    const jh::pod::array<int, 5> a = {1, 2, 3, 4, 5};
    constexpr jh::pod::array<int, 3> b = {10, 20, 30};

    const auto zipped = jh::views::zip(a, b);

    const std::vector<std::pair<int, int>> expected = {
        {1, 10}, {2, 20}, {3, 30}
    };

    std::size_t i = 0;
    for (auto p : zipped) {
        REQUIRE(p.get<0>() == expected[i].first);
        REQUIRE(p.get<1>() == expected[i].second);
        ++i;
    }

    REQUIRE(i == expected.size());
}
