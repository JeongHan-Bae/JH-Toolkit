#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include <vector>
#include <numeric>
#include "jh/pod_stack.h"

#include "jh/pod.h"
template<typename T>
std::vector<T> generate_sequence(std::size_t N) {
    std::vector<T> result(N);
    std::iota(result.begin(), result.end(), static_cast<T>(0));
    return result;
}

using int_pair = jh::pod::pair<int, int>;


JH_POD_STRUCT(int_triplet, int first; int second; int third;);


TEMPLATE_TEST_CASE(
    "pod_stack push/pop basic",
    "[pod_stack]",
    (int_pair),
    (int_triplet)
) {
    constexpr std::size_t N = 1024 + 137;

    std::vector<TestType> inserted;

    SECTION("Push sequence") {
        constexpr std::uint32_t BLOCK_SIZE = 256;
        jh::pod_stack<TestType, BLOCK_SIZE> s;
        for (std::size_t i = 0; i < N; ++i) {
            if constexpr (std::is_same_v<TestType, int>) {
                s.push(static_cast<int>(i));
                inserted.emplace_back(i);
            } else if constexpr (std::is_same_v<TestType, int_pair>) {
                s.push(i, i + 1);
                inserted.emplace_back(int_pair(static_cast<int>(i), static_cast<int>(i) + 1));
            } else if constexpr (std::is_same_v<TestType, int_triplet>) {
                s.push(i, i + 1, i + 2);
                inserted.emplace_back(
                    int_triplet(static_cast<int>(i), static_cast<int>(i) + 1, static_cast<int>(i) + 2));
            }
        }

        REQUIRE(s.size() == N);

        // Pop and compare in reverse order (stack behavior)
        for (std::size_t i = 0; i < N; ++i) {
            auto &top = s.top();
            REQUIRE(top == inserted[N - 1 - i]);
            s.pop();
        }

        REQUIRE(s.empty());
    }
}

TEST_CASE("pod_stack clear and reuse", "[pod_stack]") {
    constexpr std::uint32_t BLOCK_SIZE = 256;
    jh::pod_stack<int, BLOCK_SIZE> s;

    constexpr std::size_t N = 768;

    SECTION("Clear should reset size but keep block for reuse") {
        for (std::size_t i = 0; i < N; ++i) {
            s.push(static_cast<int>(i));
        }
        REQUIRE(s.size() == N);

        s.clear();
        REQUIRE(s.size() == 0); // NOLINT
        REQUIRE(s.empty());

        s.push(42);
        REQUIRE(s.top() == 42);
        s.pop();
        REQUIRE(s.empty());
    }

    SECTION("clear_reserve should retain blocks up to count") {
        for (std::size_t i = 0; i < N * 2; ++i) {
            s.push(static_cast<int>(i));
        }

        REQUIRE(s.size() == N * 2);
        s.clear_reserve(1);
        REQUIRE(s.empty());

        s.push(99);
        REQUIRE(s.top() == 99);
    }
}

TEST_CASE("Static Checks", "[pods]") {
    /// @brief Compile-time check for POD compliance.
    static_assert(jh::pod::pod_like<decltype(jh::pod::make_optional(1))>);
    static_assert(sizeof(jh::pod::optional<std::uint32_t>) == 8);
    static_assert(alignof(jh::pod::optional<std::uint32_t>) == 4);
    static_assert(jh::pod::pod_like<jh::pod::optional<std::uint32_t>>);
    static_assert(jh::pod::pod_like<jh::pod::array<int, 4>>);
    static_assert(jh::pod::pod_like<jh::pod::pair<int, int>>);
    static_assert(jh::pod::pod_like<jh::pod::string_view>);
}

TEST_CASE("pod_stack clean_pop memory recycle", "[pod_stack]") {
    SECTION("clean_pop also pops correctly") {
        constexpr std::size_t N = 700;
        constexpr std::uint32_t BLOCK_SIZE = 256;
        jh::pod_stack<int_pair, BLOCK_SIZE> s;
        for (int i = 0; i < N; ++i) {
            s.push(i, i + 1);
        }

        REQUIRE(s.size() == N);

        for (int i = N - 1; i >= 0; --i) {
            const auto &[a, b] = s.top();
            REQUIRE(a == i);
            REQUIRE(b == i + 1);
            s.clean_pop(); // Forces memory recycling
        }

        REQUIRE(s.empty());
    }
}
