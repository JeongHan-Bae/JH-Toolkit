#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include <vector>
#include <list>
#include <deque>
#include <set>
#include <array>
#include <forward_list>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include "jh/sequence.h"
#include "jh/pods/array.h"

namespace test {
    struct NonTemplateSequence {
        std::vector<int> data{1, 2, 3};
        [[nodiscard]] auto begin() const { return data.begin(); }
        [[nodiscard]] auto end() const { return data.end(); }
    };

    template<typename T>
    struct TemplateSequence {
        std::vector<T> data;
        TemplateSequence(std::initializer_list<T> init) : data(init) {}
        [[nodiscard]] auto begin() const { return data.begin(); }
        [[nodiscard]] auto end() const { return data.end(); }
    };

    struct ConstIterSequence {
        std::vector<int> data{4, 5, 6};
        [[nodiscard]] auto begin() const -> std::vector<int>::const_iterator { return data.begin(); }
        [[nodiscard]] auto end() const -> std::vector<int>::const_iterator { return data.end(); }
    };

    struct MutableIterSequence {
        std::vector<int> data{7, 8, 9};
        auto begin() { return data.begin(); }
        void begin() const = delete;
        auto end() { return data.end(); }
        void end() const = delete;
    };
    struct NoBeginEnd {}; // ❌ No `begin()` / `end()`

    struct FakeSequence { // ❌ Has `begin()` / `end()` but they are not iterators
        [[nodiscard]] static int begin() { return 42; }
        [[nodiscard]] static int end() { return 99; }
    };
}

// ✅ Recognizing STL Sequences
TEST_CASE("STL Sequences Recognition") {
    REQUIRE(jh::is_sequence<std::vector<int>>);
    REQUIRE(jh::is_sequence<std::list<double>>);
    REQUIRE(jh::is_sequence<std::deque<char>>);
    REQUIRE(jh::is_sequence<std::set<float>>);
    REQUIRE(jh::is_sequence<std::unordered_set<std::string>>);
    REQUIRE(jh::is_sequence<std::array<int, 5>>);
    REQUIRE(jh::is_sequence<std::forward_list<int>>);
    REQUIRE(jh::is_sequence<std::map<int, int>>);
    REQUIRE(jh::is_sequence<std::unordered_map<int, int>>);
}

// ✅ Extracting Sequence Value Types
TEST_CASE("Extracting Sequence Value Types") {
    REQUIRE(std::is_same_v<jh::sequence_value_type<std::vector<int>>, int>);
    REQUIRE(std::is_same_v<jh::sequence_value_type<std::array<const double, 3>>, double>);
    REQUIRE(std::is_same_v<jh::sequence_value_type<std::deque<char>>, char>);
    REQUIRE(std::is_same_v<jh::sequence_value_type<std::set<int>>, int>);
    REQUIRE(std::is_same_v<jh::sequence_value_type<std::array<float, 10>>, float>);
    REQUIRE(std::is_same_v<jh::sequence_value_type<std::map<int, double>>, std::pair<const int, double>>);
    REQUIRE(std::is_same_v<jh::sequence_value_type<std::unordered_map<std::string, float>>, std::pair<const std::string, float>>);
}

TEST_CASE("Non-Sequences Should Fail") {
    REQUIRE_FALSE(jh::is_sequence<int>);
    REQUIRE_FALSE(jh::is_sequence<double>);
    REQUIRE_FALSE(jh::is_sequence<char*>);
    REQUIRE_FALSE(jh::is_sequence<std::tuple<int, double, std::string>>);
    REQUIRE_FALSE(jh::is_sequence<std::optional<int>>);
    REQUIRE_FALSE(jh::is_sequence<test::NoBeginEnd>);
    REQUIRE_FALSE(jh::is_sequence<test::FakeSequence>);
}

// ✅ Handling `const` and `mutable`
TEST_CASE("Handling Modifiers in Sequences") {
    REQUIRE(jh::is_sequence<const std::vector<int>>);
    REQUIRE(jh::is_sequence<std::list<double>>);
    REQUIRE(jh::is_sequence<const std::deque<char>>);

    REQUIRE(std::is_same_v<jh::sequence_value_type<const std::vector<int>>, int>);
    REQUIRE(std::is_same_v<jh::sequence_value_type<std::list<double>>, double>);
}

// ✅ Custom Sequence Recognition
TEST_CASE("Custom Non-Template Sequence") {
    REQUIRE(jh::is_sequence<test::NonTemplateSequence>);
    REQUIRE(std::is_same_v<jh::sequence_value_type<test::NonTemplateSequence>, int>);
}

// ✅ Custom Template Sequence Recognition
TEST_CASE("Custom Template Sequence") {
    REQUIRE(jh::is_sequence<test::TemplateSequence<int>>);
    REQUIRE(jh::is_sequence<test::TemplateSequence<std::string>>);

    REQUIRE(std::is_same_v<jh::sequence_value_type<test::TemplateSequence<int>>, int>);
    REQUIRE(std::is_same_v<jh::sequence_value_type<test::TemplateSequence<std::string>>, std::string>);
}

// ✅ Custom Sequence with `const begin()`
TEST_CASE("Custom ConstIterSequence") {
    REQUIRE(jh::is_sequence<test::ConstIterSequence>);
    REQUIRE(std::is_same_v<jh::sequence_value_type<test::ConstIterSequence>, int>);
}

// ❌ Custom Sequence with Mutable Iterators (Should NOT be a sequence)
TEST_CASE("Mutable Iterator Sequence") {
    REQUIRE_FALSE(jh::is_sequence<test::MutableIterSequence>);
}


TEST_CASE("Sequence to Range") {
    constexpr jh::pod::array<int, 3> vec = {1, 2, 3}; // sequence, not a range
    auto range_ = jh::to_range(vec);
    auto it = range_.begin();

    std::ranges::for_each(range_, [&](const int a) {
                    REQUIRE(a == *it);
                    ++it;
                });
}