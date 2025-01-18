#define CATCH_CONFIG_MAIN
#include <iostream>
#include <numeric>
#include <random>
#include <catch2/catch_all.hpp>
#include "jh/generator.h"

namespace test {
    jh::generator<int> range(const int end) {
        for (int i = 0; i < end; ++i) {
            co_yield i;
        }
    }

    jh::generator<int> range(const int start, const int end) {
        for (int i = start; i < end; ++i) {
            co_yield i;
        }
    }

    jh::generator<int> range(const int start, const int end, const int step) {
        for (int i = start; i < end; i += step) {
            co_yield i;
        }
    }
    jh::generator<int, int> countdown(int start) {
        int step = 1; // Default step size if no value is sent
        while (start > 0) {
            volatile int next_step = step;
            step = co_await next_step;
            start -= step;
            co_yield start;
        }
    }

}

// **Simple Test**
TEST_CASE("Simple Test") {
    jh::generator<int> my_generator = []() -> jh::generator<int> {
        for (int i = 1; i <= 5; ++i) {
            co_yield i;
        }
    }();
    SECTION("Default") {
        auto i = 1;
        while (my_generator.next()) {
            REQUIRE(my_generator.value().value() == i);
            ++i;
        }
    }
}
// **Base Case**
TEST_CASE("Basic Generator Test") {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution dist(1, 10000);

    constexpr int total_tests = 1024;

    for (int i = 0; i < total_tests; ++i) {
        SECTION("Randomized Test " + std::to_string(i + 1)) {  // Creates a separate test case per iteration
            int start = dist(gen);
            int end = dist(gen);

            if (start > end) std::swap(start, end); // Ensure start < end
            if (start == end) end++; // Prevent empty ranges

            auto generator = test::range(start, end);
            int expected = start;

            while (generator.next()) {
                REQUIRE(generator.value().value() == expected);
                expected++;
            }

            REQUIRE(expected == end);
        }
    }
}


// **Edge Case: Empty Range**
TEST_CASE("Empty Generator Test") {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution dist(1, 10000);

    constexpr int total_tests = 1024;

    for (int i = 0; i < total_tests; ++i) {
        SECTION("Randomized Empty Test " + std::to_string(i + 1)) { // Each test case is independent
            const int end = dist(gen);
            const int start = end + dist(gen); // Ensure start >= end

            auto generator = test::range(start, end);

            REQUIRE_FALSE(generator.next()); // Generator should always be empty
        }
    }
}

// **Edge Case: Step**
TEST_CASE("Step Generator Test") {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution dist(1, 10000); // Range for start & end
    std::uniform_int_distribution step_dist(1, 100); // Step size must be > 0

    constexpr int total_tests = 1024;

    for (int i = 0; i < total_tests; ++i) {
        SECTION("Randomized Step Test " + std::to_string(i + 1)) {
            int start = dist(gen);
            int end = dist(gen);
            if (start > end) std::swap(start, end); // Ensure start < end
            if (start == end) end++; // Avoid empty range

            const int step = step_dist(gen); // Ensure step > 0

            auto generator = test::range(start, end, step);

            // Compute expected values
            std::vector<int> expected;
            for (int val = start; val < end; val += step) {
                expected.push_back(val);
            }

            // Compare generated sequence with expected
            int index = 0;
            while (generator.next()) {
                REQUIRE(generator.value().value() == expected[index]);
                index++;
            }

            REQUIRE(index == expected.size()); // Ensure full iteration
        }
    }
}

// **Convert Generator to Vector**
TEST_CASE("Generator to Vector Test") {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution dist(1, 10000); // Range for start & end

    constexpr int total_tests = 1024;

    for (int i = 0; i < total_tests; ++i) {
        SECTION("Randomized Vector Test " + std::to_string(i + 1)) {
            int start = dist(gen);
            int end = dist(gen);
            if (start > end) std::swap(start, end); // Ensure start < end
            if (start == end) end++; // Avoid empty range

            auto generator = test::range(start, end);

            // Generate expected vector using iota
            std::vector<int> expected(end - start);
            std::iota(expected.begin(), expected.end(), start);

            // Convert generator to vector
            const std::vector<int> generated = to_vector(generator);

            // Compare generated vector with expected vector
            REQUIRE(generated == expected);
        }
    }
}

// **Convert Generator to List**
TEST_CASE("Generator to List Test") {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution dist(1, 10000); // Range for start & end

    constexpr int total_tests = 1024;

    for (int i = 0; i < total_tests; ++i) {
        SECTION("Randomized List Test " + std::to_string(i + 1)) {
            int start = dist(gen);
            int end = dist(gen);
            if (start > end) std::swap(start, end); // Ensure start < end
            if (start == end) end++; // Avoid empty range

            auto generator = test::range(start, end);

            // Generate expected sequence using std::iota
            std::vector<int> expected_vec(end - start);
            std::iota(expected_vec.begin(), expected_vec.end(), start);

            // Convert expected vector to list for comparison
            std::list expected_list(expected_vec.begin(), expected_vec.end());

            // Convert generator to list
            const std::list<int> generated_list = to_list(generator);

            // Compare generated list with expected list
            REQUIRE(generated_list == expected_list);
        }
    }
}

// **Convert Generator with Steps to Vector**
TEST_CASE("Step Generator to Vector Test") {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution dist(0, 10000);  // Range for start & end
    std::uniform_int_distribution step_dist(1, 100); // Ensure step > 0

    constexpr int total_tests = 1024;

    for (int i = 0; i < total_tests; ++i) {
        SECTION("Randomized Step Vector Test " + std::to_string(i + 1)) {
            int start = dist(gen);
            int end = dist(gen);
            if (start > end) std::swap(start, end); // Ensure start < end
            if (start == end) end++; // Avoid empty range

            const int step = step_dist(gen); // Ensure step > 0

            auto generator = test::range(start, end, step);

            // Generate expected values manually
            std::vector<int> expected;
            for (int val = start; val < end; val += step) {
                expected.push_back(val);
            }

            // Convert generator to vector
            const std::vector<int> generated = to_vector(generator);

            // Compare generated vector with expected vector
            REQUIRE(generated == expected);
        }
    }
}

// **Convert Generator with Steps to List**
TEST_CASE("Step Generator to List Test") {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution dist(0, 10000);  // Range for start & end
    std::uniform_int_distribution step_dist(1, 100); // Ensure step > 0

    constexpr int total_tests = 1024;

    for (int i = 0; i < total_tests; ++i) {
        SECTION("Randomized Step List Test " + std::to_string(i + 1)) {
            int start = dist(gen);
            int end = dist(gen);
            if (start > end) std::swap(start, end); // Ensure start < end
            if (start == end) end++; // Avoid empty range

            const int step = step_dist(gen); // Ensure step > 0

            auto generator = test::range(start, end, step);

            // Generate expected values manually
            std::vector<int> expected_vec;
            for (int val = start; val < end; val += step) {
                expected_vec.push_back(val);
            }

            // Convert expected vector to list for comparison
            std::list expected_list(expected_vec.begin(), expected_vec.end());

            // Convert generator to list
            const std::list<int> generated_list = to_list(generator);

            // Compare generated list with expected list
            REQUIRE(generated_list == expected_list);
        }
    }
}

// **Negative Step Range (Should be Empty)**
TEST_CASE("Negative Step Generator Test") {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution dist(1, 10000);  // Range for start & end
    std::uniform_int_distribution step_dist(1, 100); // Positive values, later negated

    constexpr int total_tests = 1024;

    for (int i = 0; i < total_tests; ++i) {
        SECTION("Randomized Negative Step Test " + std::to_string(i + 1)) {
            const int end = dist(gen);  // Random end
            const int start = end + dist(gen); // Ensure start > end
            const int step = -step_dist(gen); // Ensure step < 0

            auto generator = test::range(start, end, step);

            REQUIRE_FALSE(generator.next()); // Should always be empty
        }
    }
}

// **Large Step Size (Should Only Yield One Value)**
TEST_CASE("Large Step Generator Test") {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution dist(0, 10000);  // Range for start
    std::uniform_int_distribution range_dist(1, 1000); // Range for end (ensuring it's larger than start)

    constexpr int total_tests = 1024;

    for (int i = 0; i < total_tests; ++i) {
        SECTION("Randomized Large Step Test " + std::to_string(i + 1)) {
            const int start = dist(gen);
            const int range = range_dist(gen);
            const int end = start + range; // Ensure end > start

            const int step = end - start + dist(gen) + 1; // Step is guaranteed to be larger than the range

            auto generator = test::range(start, end, step);

            REQUIRE(generator.next()); // Should yield once
            REQUIRE(generator.value().value() == start); // First value must be `start`
            REQUIRE_FALSE(generator.next()); // No more values should be yielded
        }
    }
}

// **to_vector with Input Value**
TEST_CASE("Generator with Single Input") {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution start_dist(5, 10000);  // Random starting value
    std::uniform_int_distribution step_dist(1, 50); // Ensure positive step

    constexpr int total_tests = 1024;

    for (int i = 0; i < total_tests; ++i) {
        SECTION("Randomized Single Input Test " + std::to_string(i + 1)) {
            int start = start_dist(gen);
            const int step = step_dist(gen);

            auto generator = test::countdown(start);

            // Compute expected values manually
            std::vector<int> expected;
            while (start > 0) {
                start -= step;
                expected.push_back(start);
                if (start <= 0) break; // Stop at 0
            }

            // Convert generator to vector with input step
            const std::vector<int> generated = to_vector(generator, step);

            // Compare generated vector with expected vector
            REQUIRE(generated == expected);
        }
    }
}

// **to_vector with Input Sequence**
TEST_CASE("Generator with Vector Input") {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution start_dist(5, 10000);  // Random starting value
    std::uniform_int_distribution step_dist(1, 50); // Random step values
    std::uniform_int_distribution step_count_dist(1, 20); // Number of steps

    constexpr int total_tests = 1024;

    for (int i = 0; i < total_tests; ++i) {
        SECTION("Randomized Vector Input Test " + std::to_string(i + 1)) {
            int start = start_dist(gen);
            const int step_count = step_count_dist(gen);

            // Generate a sequence of random steps
            std::vector<int> steps(step_count);
            for (int& step : steps) {
                step = step_dist(gen);
            }

            auto generator = test::countdown(start);

            // Compute expected values manually
            std::vector<int> expected;
            size_t index = 0;
            while (start > 0 && index < steps.size()) {
                start -= steps[index++];
                expected.push_back(start);
                if (start <= 0) break; // Stop at 0
            }

            // Convert generator to vector with input steps
            const std::vector<int> generated = to_vector(generator, steps);

            // Compare generated vector with expected vector
            REQUIRE(generated == expected);
        }
    }
}

TEST_CASE("Generator with 'Send' Step by Step") {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution step_dist(1, 50); // Step values
    std::uniform_int_distribution step_count_dist(1, 20); // Number of steps

    constexpr int total_tests = 1024;

    for (int i = 0; i < total_tests; ++i) {
        SECTION("Randomized Send Step by Step Test " + std::to_string(i + 1)) {
            const int step_count = step_count_dist(gen);

            // Generate a sequence of random steps
            std::vector<int> steps(step_count);
            for (int& step : steps) {
                step = step_dist(gen);
            }

            // Compute total sum as the starting value
            int sum = std::accumulate(steps.begin(), steps.end(), 0);

            auto generator = test::countdown(sum);
            size_t index = 0;

            while (generator.next()) {
                REQUIRE(index < steps.size()); // Ensure we don't access out of bounds

                const int decrement = steps[index++];
                if (!generator.send(decrement)) break; // Stop if generator ends
                sum -= decrement;

                REQUIRE(generator.value() == sum);
            }

            REQUIRE(sum == 0); // Ensure countdown fully decremented to 0
        }
    }
}

TEST_CASE("Generator with 'Send_Ite' Step by Step") {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution step_dist(1, 50); // Step values
    std::uniform_int_distribution step_count_dist(1, 20); // Number of steps

    constexpr int total_tests = 1024;

    for (int i = 0; i < total_tests; ++i) {
        SECTION("Randomized Send_Ite Step by Step Test " + std::to_string(i + 1)) {
            const int step_count = step_count_dist(gen);

            // Generate a sequence of random steps
            std::vector<int> steps(step_count);
            for (int& step : steps) {
                step = step_dist(gen);
            }

            // Compute total sum as the starting value
            int sum = std::accumulate(steps.begin(), steps.end(), 0);

            auto generator = test::countdown(sum);
            size_t index = 0;

            while (index < steps.size() && generator.send_ite(steps[index])) {
                sum -= steps[index];

                REQUIRE(generator.value() == sum);
                ++index;
            }

            REQUIRE(sum == 0); // Ensure countdown fully decremented to 0
        }
    }
}

TEST_CASE("List -> Generator -> List Equivalence Test") {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution size_dist(1, 100);  // List size
    std::uniform_int_distribution value_dist(-10000, 10000); // Random values

    constexpr int total_tests = 1024;

    for (int i = 0; i < total_tests; ++i) {
        SECTION("Randomized List Equivalence Test " + std::to_string(i + 1)) {
            const int size = size_dist(gen);

            // Generate a random list
            std::list<int> original_list;
            for (int j = 0; j < size; ++j) {
                original_list.push_back(value_dist(gen));
            }

            // Convert list to generator
            auto generator = jh::make_generator(original_list);

            // Convert generator back to list
            const std::list<int> generated_list = to_list(generator);

            // Ensure the final list matches the original list
            REQUIRE(generated_list == original_list);
        }
    }
}
