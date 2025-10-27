/**
 * @file example_pod_basics.cpp
 * @brief Demonstrates usage of `jh::pod` types in JH Toolkit
 *
 * ## Overview
 * This file showcases basic usage patterns for the following types from `jh::pod`:
 * - `pod::array`
 * - `pod::bitflags`
 * - `pod::bytes_view`
 * - `pod::optional`
 * - `pod::span`
 * - `pod::string_view`
 *
 * ## Key Goals
 * - POD-safe memory layout and manipulation
 * - Lightweight runtime usage
 * - Ideal for serialization, zero-cost abstraction, and memory-constrained code
 */

#include "jh/pod"
#include <iostream>
#include <cstring>
#include <numeric>
#include <iomanip> // for std::setw

namespace example {

    using namespace jh;

    /**
     * @brief Demonstrates construction, assignment, and iteration of `pod::array`
     */
    void array_usage() {
        std::cout << "\n\U0001F539 pod::array Usage:\n";

        pod::array<int, 4> arr = {{1, 2, 3, 4}};
        arr[2] = 99;

        for (int v : arr) {
            std::cout << v << " ";
        }
        std::cout << "\n";
    }

    /**
     * @brief Demonstrates basic bit manipulation via `pod::bitflags`
     */
    void bitflags_usage() {
        std::cout << "\n\U0001F539 pod::bitflags Usage:\n";

        pod::bitflags<16> flags{};
        flags.set(1);
        flags.set(4);
        flags.set(7);

        std::cout << "Flags count: " << flags.count() << "\n";
        flags.clear(4);
        std::cout << "Has bit 4: " << flags.has(4) << "\n";
    }

    /**
     * @brief Demonstrates `pod::bytes_view` with a trivially copyable struct
     */
    void bytes_view_usage() {
        std::cout << "\n\U0001F539 pod::bytes_view Usage:\n";

        struct Packet { uint32_t id; uint16_t len; };
        static_assert(pod::trivial_bytes<Packet>);

        Packet p{0x12345678, 42};
        const auto view = pod::bytes_view::from(p);
        const jh::pod::pod_like auto copy = view.clone<Packet>();

        std::cout << "Packet ID: 0x" << std::hex << copy.id
                  << ", len: " << std::dec << copy.len << "\n";
    }

    /**
     * @brief Demonstrates `pod::optional` for optional POD values
     */
    void optional_usage() {
        std::cout << "\n\U0001F539 pod::optional Usage:\n";

        pod::optional<int> opt{};
        std::cout << "Has value: " << opt.has() << "\n";

        opt.store(2024);
        std::cout << "Stored value: " << opt.ref() << "\n";
    }

    /**
     * @brief Demonstrates `pod::span` and range/view operations
     */
    void span_usage() {
        std::cout << "\n\U0001F539 pod::span Usage:\n";

        pod::array<int, 5> arr = {{10, 20, 30, 40, 50}};
        pod::span<int> s{arr.data, arr.size()}; // NOLINT

        std::cout << "Last 2 elements: ";
        for (int v : s.last(2)) {
            std::cout << v << " ";
        }
        std::cout << "\n";
    }

    /**
     * @brief Demonstrates basic string operations via `pod::string_view`
     */
    void string_view_usage() {
        std::cout << "\n\U0001F539 pod::string_view Usage:\n";

        constexpr const char* msg = "pod_string";
        const pod::string_view sv{msg, std::strlen(msg)};

        std::cout << "String: " << std::string_view(sv.data, sv.len) << "\n";
        std::cout << "Starts with 'pod': " << sv.starts_with({"pod", 3}) << "\n";
        std::cout << "Ends with 'ing': " << sv.ends_with({"ing", 3}) << "\n";
    }

    /**
     * @brief Combines `pod::array<char, N>` and `pod::string_view` as lightweight string buffer
     */
    void array_string_buffer_usage() {
        std::cout << "\n\U0001F539 pod::array<char, N> as string buffer:\n";

        pod::array<char, 32> buffer{};
        const char* message = "Hello, POD!";
        std::memcpy(buffer.data, message, std::strlen(message));

        const pod::string_view sv{buffer.data, std::strlen(message)};
        std::cout << "String view over buffer: " << std::string_view(sv.data, sv.len) << "\n";
    }
    /**
     * @brief Demonstrates matrix-style reinterpretation from flat array using bytes_view::fetch
     */
    void matrix_view_usage() {
        std::cout << "\n\U0001F539 pod::array as flat matrix view:\n";

        constexpr std::size_t Rows = 3;
        constexpr std::size_t Cols = 4;

        // Flat storage: 3 rows * 4 cols
        pod::array<int, Rows * Cols> flat{};
        std::iota(flat.begin(), flat.end(), 1); // Fill with 1..12

        const auto view = pod::bytes_view::from(flat);

        std::cout << "Matrix as rows:\n";

        for (std::size_t row = 0; row < Rows; ++row) {
            const auto* r = view.fetch<pod::array<int, Cols>>(row * sizeof(pod::array<int, Cols>));
            if (r != nullptr) {
                for (int val : *r) {
                    std::cout << std::setw(2) << val << " ";
                }
                std::cout << "\n";
            }
        }
    }
    /**
     * @brief Demonstrates full reinterpretation of flat array into array<array<int, Cols>, Rows>
     */
    void matrix_structured_view_usage() {
        std::cout << "\n\U0001F539 pod::array as structured matrix view (array<array<>>):\n";

        constexpr std::size_t Rows = 3;
        constexpr std::size_t Cols = 4;

        pod::array<int, Rows * Cols> flat{};
        std::iota(flat.begin(), flat.end(), 1); // Fill 1..12

        const auto view = pod::bytes_view::from(flat);
        const auto* matrix = view.fetch<pod::array<pod::array<int, Cols>, Rows>>();

        if (matrix != nullptr) {
            for (const jh::pod::pod_like auto& row : *matrix) {
                for (int val : row) {
                    std::cout << std::setw(2) << val << " ";
                }
                std::cout << "\n";
            }
        }
    }

} // namespace example

/**
 * @brief Main entry point to run all examples.
 */
int main() {
    example::array_usage();
    example::bitflags_usage();
    example::bytes_view_usage();
    example::optional_usage();
    example::span_usage();
    example::string_view_usage();
    example::array_string_buffer_usage();
    example::matrix_view_usage();
    example::matrix_structured_view_usage();
    return 0;
}
