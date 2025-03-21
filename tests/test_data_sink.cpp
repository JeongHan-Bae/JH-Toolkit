#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include <iostream>
#include <vector>
#include <memory>
#include <numeric>
#include "jh/data_sink.h"

// **Helper Function: Convert `data_sink<T>` to `std::vector<T>`**
template<typename T, std::uint32_t BLOCK_SIZE>
std::vector<T> to_vector(const jh::data_sink<T, BLOCK_SIZE> &sink) {
    std::vector<T> result;
    result.reserve(sink.size());
    for (const auto &val: sink) {
        result.emplace_back(val);
    }
    return result;
}

// **Basic Data Type Tests**
TEMPLATE_TEST_CASE("data_sink Basic Type Test", "[data_sink]", int, float, bool) {
    constexpr std::uint32_t BLOCK_SIZE = 1024;

    jh::data_sink<TestType, BLOCK_SIZE> sink;

    // **Test emplace_back**
    SECTION("Emplace Back") {
        constexpr int N = 5000;
        for (int i = 0; i < N; ++i) {
            sink.emplace_back(static_cast<TestType>(i));
        }

        // Convert to vector for validation
        std::vector<TestType> expected(N);
        std::iota(expected.begin(), expected.end(), static_cast<TestType>(0));

        REQUIRE(to_vector(sink) == expected);
    }

    // **Test bulk_append**
    SECTION("Bulk Append") {
        constexpr int N = 5000;
        std::vector<TestType> input(N);
        if constexpr (std::is_same_v<TestType, bool>) {
            std::generate(input.begin(), input.end(), [n = false]() mutable { return n = !n; });
        } else {
            std::iota(input.begin(), input.end(), static_cast<TestType>(0));
        }

        sink.bulk_append(input);

        REQUIRE(to_vector(sink) == input);
    }
}

// **Pointer Type Test**
TEST_CASE("data_sink Raw Pointer Test", "[data_sink]") {
    constexpr std::uint32_t BLOCK_SIZE = 1024;
    jh::data_sink<int *, BLOCK_SIZE> sink;

    // Create raw pointer data
    constexpr int N = 5000;
    std::vector<int> storage(N);
    for (int i = 0; i < N; ++i) {
        storage[i] = i;
    }

    // **Test emplace_back**
    SECTION("Emplace Back Pointers") {
        for (int i = 0; i < N; ++i) {
            sink.emplace_back(&storage[i]);
        }

        // Validate pointer values
        auto iter = sink.begin();
        for (int i = 0; i < N; ++i, ++iter) {
            REQUIRE(*(*iter) == storage[i]);
        }
    }

    // **Test bulk_append**
    SECTION("Bulk Append Pointers") {
        std::vector<int *> pointers(N);
        for (int i = 0; i < N; ++i) {
            pointers[i] = &storage[i];
        }

        sink.bulk_append(pointers);

        auto iter = sink.begin();
        for (int i = 0; i < N; ++i, ++iter) {
            REQUIRE(*(*iter) == storage[i]);
        }
    }
}

// **Unique Pointer Type Test**
TEST_CASE("data_sink Unique Pointer Test", "[data_sink]") {
    constexpr std::uint32_t BLOCK_SIZE = 1024;
    jh::data_sink<std::unique_ptr<std::vector<int> >, BLOCK_SIZE> sink;

    constexpr int N = 5000;

    // **Test emplace_back**
    SECTION("Emplace Back Unique Pointers") {
        for (int i = 0; i < N; ++i) {
            sink.emplace_back(std::vector{i});
            // or sink.emplace_back(1, i);
        }

        // Validate unique pointer values
        auto iter = sink.begin();
        for (int i = 0; i < N; ++i, ++iter) {
            REQUIRE((*iter)->size() == 1);
            REQUIRE((*iter)->at(0) == i);
        }
    }

    // **Test bulk_append**
    SECTION("Bulk Append Unique Pointers") {
        std::vector<std::vector<int> > input;
        for (int i = 0; i < N; ++i) {
            auto vec = std::vector<int>();
            vec.push_back(i);
            input.push_back(std::move(vec));
        }

        sink.bulk_append(input);

        auto iter = sink.begin();
        for (int i = 0; i < N; ++i, ++iter) {
            REQUIRE((*iter)->size() == 1);
            REQUIRE((*iter)->at(0) == i);
        }
    }
}

// **Block Capacity Test**
TEST_CASE("data_sink Block Capacity Test", "[data_sink]") {
    constexpr std::uint32_t BLOCK_SIZE = 1024;
    jh::data_sink<int, BLOCK_SIZE> sink;

    constexpr int N = BLOCK_SIZE * 3; // Fill at least 3 blocks

    for (int i = 0; i < N; ++i) {
        sink.emplace_back(i);
    }

    REQUIRE(sink.size() == N);
    REQUIRE(to_vector(sink).size() == N);
}

// **Clear and Clear_Reserve Test**
TEST_CASE("data_sink Clear and Clear_Reserve Test", "[data_sink]") {
    constexpr std::uint32_t BLOCK_SIZE = 1024;
    jh::data_sink<int, BLOCK_SIZE> sink;

    constexpr int N = BLOCK_SIZE * 3;

    for (int i = 0; i < N; ++i) {
        sink.emplace_back(i);
    }

    REQUIRE(sink.size() == N);
    sink.clear();
    REQUIRE(sink.empty());

    // Test clear_reserve
    for (int i = 0; i < N; ++i) {
        sink.emplace_back(i);
    }

    REQUIRE(sink.size() == N);
    sink.clear_reserve(1); // Only reserve 1 block
    REQUIRE(sink.empty());

    sink.emplace_back(42);
    REQUIRE(sink.size() == 1);
    REQUIRE(*sink.begin() == 42);
}

// **Iterator Test**
TEST_CASE("data_sink Iterator Test", "[data_sink]") {
    constexpr std::uint32_t BLOCK_SIZE = 1024;
    jh::data_sink<int, BLOCK_SIZE> sink;

    constexpr int N = BLOCK_SIZE * 2;

    for (int i = 0; i < N; ++i) {
        sink.emplace_back(i);
    }

    int count = 0;
    for (auto it = sink.begin(); it != jh::data_sink<int, 1024>::end(); ++it) {
        REQUIRE(*it == count);
        ++count;
    }

    REQUIRE(count == N);
}
