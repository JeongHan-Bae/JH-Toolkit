/**
 * @file example_generator.cpp
 * @brief Demonstrates the usage of `generator` in `jh-toolkit`.
 *
 * ## Overview
 * This example showcases the `generator` concept and its integration with `sequence.h`.
 * It demonstrates how to use **lazy evaluation**, **iterable generators**, and **interactive sending** in C++.
 *
 * ## Key Features
 * - **Lazy Evaluation**: Generates values **on demand**, reducing memory usage.
 * - **Iterable Generators**: Supports **range-based loops** and **custom iteration logic**.
 * - **Composable Sequences**: Easily integrates with `sequence.h` for **chaining and transformations**.
 * - **Interactive Input (`send()`)**: Allows sending values into a generator **to dynamically modify its state**.
 * - **Generator Consumer**: Converts generated sequences into `std::vector` or `std::deque`.
 *
 * ## Best Practices
 * - Use `generator` when dealing with **large data streams** to avoid unnecessary memory allocation.
 * - Combine `generator` with `sequence` to build **functional-style pipelines**.
 * - Use `send()` for **interactive modifications**, or `send_ite()` to **combine sending and advancing**.
 * - Use `to_vector(generator)` when you need **to collect all values into a concrete container**.
 *
 * ## Linking Requirement
 * Since `generator.h` and `sequence.h` are **header-only modules**, ensure that you **link against `jh-toolkit`**
 * when building your project.
 */

#include "jh/generator"
#include <iostream>
#include <vector>
#include <ranges>
#include <deque>
#include "ensure_output.h"

#if IS_WINDOWS
static EnsureOutput ensure_output_setup;
#endif


namespace example {

    /**
     * @brief Generates a sequence from `[0, end)`.
     */
    jh::generator<int> range(const int end) {
        for (int i = 0; i < end; ++i) {
            co_yield i;
        }
    }

    /**
     * @brief Generates a sequence from `[start, end)`.
     */
    jh::generator<int> range(const int start, const int end) {
        for (int i = start; i < end; ++i) {
            co_yield i;
        }
    }

    /**
     * @brief Generates a sequence from `[start, end)` with a custom step size.
     */
    jh::generator<int> range(const int start, const int end, const int step) {
        for (int i = start; i < end; i += step) {
            co_yield i;
        }
    }

    /**
     * @brief Generates a countdown sequence interactively.
     * - Users can **send a step value** to decrease the countdown dynamically.
     */
    jh::generator<int, int> countdown(int start) {
        while (start > 0) {
            const int step = co_await int{};  // NOLINT Receive new step size via send()
            start -= step;
            co_yield start;
        }
    }


    /**
     * @brief Demonstrates collecting a generator's output into a `std::vector<int>`.
     */
    void generator_to_vector_demo() {
        std::cout << "\n\U0001F539 Collecting Generator to `std::vector<int>`:\n";

        auto generator = range(1, 6);
        const auto values = to_vector(generator);

        std::cout << "Collected values: ";
        for (const int val : values) {
            std::cout << val << " ";
        }
        std::cout << "\n";
    }

    /**
     * @brief Demonstrates sending values into a countdown generator.
     */
    void interactive_generator_demo() {
        std::cout << "\n\U0001F539 Interactive Generator with `send()`:\n";

        auto countdown_gen = countdown(10);
        const std::vector steps = {1, 2, 3, 2, 1, 1};

        std::cout << "Countdown steps: ";
        auto ite = steps.begin();
        while (countdown_gen.next() && ite != steps.end()) {
            if (!countdown_gen.send(*ite)) break;
            std::cout << countdown_gen.value().value() << " ";
            ++ite;
        }
        std::cout << "\n";
    }

    /**
     * @brief Demonstrates `send_ite()`, which combines `send()` and `next()`.
     */
    void send_ite_demo() {
        std::cout << "\n\U0001F539 Interactive Generator with `send_ite()`:\n";

        auto countdown_gen = countdown(10);
        const std::vector steps = {1, 2, 3, 2, 1, 1};

        std::cout << "Countdown steps: ";
        for (const int step : steps) {
            if (!countdown_gen.send_ite(step)) break;
            std::cout << countdown_gen.value().value() << " ";
        }
        std::cout << "\n";
    }

    /**
     * @brief Converts a generator with a step size into a `std::vector<int>`.
     */
    void step_generator_to_vector_demo() {
        std::cout << "\n\U0001F539 Step Generator to `std::vector<int>`:\n";

        auto generator = range(1, 20, 3);
        const std::vector<int> values = to_vector(generator);

        std::cout << "Collected values: ";
        for (const int val : values) {
            std::cout << val << " ";
        }
        std::cout << "\n";
    }

    /**
     * @brief Converts a generator with steps into a `std::deque<int>`.
     */
    void step_generator_to_deque_demo() {
        std::cout << "\n\U0001F539 Step Generator to `std::deque<int>`:\n";

        auto generator = range(1, 20, 4);
        const std::deque<int> values = to_deque(generator);

        std::cout << "Collected values: ";
        for (const int val : values) {
            std::cout << val << " ";
        }
        std::cout << "\n";
    }

    void range_constructing() {
        std::cout << "\n\U0001F539 Constructing Generator with `std::views::iota`:\n";
        for (auto gen = jh::make_generator(std::views::iota(0, 10)); const auto v : gen) {
            std::cout << v << " ";
        }
        std::cout << "\n";
    }

    void example_to_range() {
        std::cout << "\n\U0001F539 Constructing Range with Lambda[Generator]:\n";
        constexpr auto view = std::views::iota(0, 10);
        const auto range_ = jh::to_range([view] {
            return jh::make_generator(view);
        });
        std::cout << "First Loop:\n";
        for (const auto v : range_) {
            std::cout << v << " ";
        }
        std::cout << "\nSecond Loop:\n";
        for (const auto v : range_) {
            std::cout << v << " ";
        }
        std::cout << "\n";
    }

} // namespace example

/**
 * @brief Main entry point to run all examples.
 */
int main() {
    example::generator_to_vector_demo();
    example::step_generator_to_vector_demo();
    example::step_generator_to_deque_demo();
    example::interactive_generator_demo();
    example::send_ite_demo();
    example::range_constructing();
    example::example_to_range();
    return 0;
}
