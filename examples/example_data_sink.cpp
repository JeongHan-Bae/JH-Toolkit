#include <iostream>
#include <memory>  // NOLINT
#include <vector>
#include <random>
#include <thread>
#include <ranges>
#include "jh/data_sink.h"
#include "jh/generator.h"
#include "jh/sequence.h"
#include "radix_sort.h"


int main() {
    jh::data_sink<int> sink;

    // Bulk_append of regular range-supporting types
    std::vector v = {1, 2, 3, 4, 5};
    sink.bulk_append(v);

    // Bulk_append of span types
    std::array arr_ = {10, 20, 30, 40, 50};
    std::span<int> sp(arr_);
    sink.bulk_append(sp);

    // Bulk_append of pure range types
    auto range = std::views::iota(6, 11); // from 6 to 11
    sink.bulk_append(range);

    // Bulk_append of stream-range types
    std::istringstream iss("100 200 300 400 500");
    auto stream_range = std::ranges::istream_view<int>(iss);
    sink.bulk_append(stream_range);

    std::cout << "jh::data_sink " << (jh::sequence<jh::data_sink<int> > ? "is " : "is not ") // NOLINT
            << "a sequence type." << std::endl;

    std::cout << "Example printing data_sink from a generator:\n"
            << "(Should be 1 2 3 4 5 10 20 30 40 50 6 7 8 9 10 100 200 300 400 500)\n";
    auto gen = make_generator(sink); // Make jh::generator from jh::data_sink
    for (const auto &val: gen) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    using type_t = std::uint32_t;
    constexpr std::size_t N = 1e8;
    std::vector<type_t> arr(N);

    std::cout << "Sorting with data_sink-based radix sort:\n"
            << "(Should output 1 1 2 2 3 5 6 8 43 255 17342)\n";
    auto example = std::vector<uint32_t>{1, 17342, 2, 8, 6, 5, 43, 2, 1, 255, 3};
    jh::radix_sort(example, false);
    for (const auto i: example) {
        std::cout << static_cast<uint64_t>(i) << " ";
    }
    std::cout << std::endl;

    // Generate random data
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<type_t> dist(0, 0xFFFFFF);
    for (std::size_t i = 0; i < N; ++i) {
        arr[i] = dist(rng);
    }
    auto arr_copy = arr;

    std::cout << "Example sorting large scaled data with data_sink-based radix sort.\n"
            << "N = " << N << std::endl;
    // **Radix Sort Benchmark**
    auto start = std::chrono::high_resolution_clock::now();
    jh::uint_sort(arr, false);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> radix_time = end - start;
    std::cout << "Uint Sort Time: " << radix_time.count() << " seconds\n";

    // **std::sort Benchmark**
    start = std::chrono::high_resolution_clock::now();
    std::ranges::sort(arr_copy);
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> std_sort_time = end - start;
    std::cout << "std::sort Time: " << std_sort_time.count() << " seconds\n";

    std::cout << "The result " << (arr == arr_copy ? "is " : "is not ")
            << "the same." << std::endl;

    return 0;
}
