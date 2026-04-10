/**
 * @file example_ranges.cpp
 * @brief Rich examples for <code>&lt;jh/ranges_ext&gt;</code> and <code>&lt;jh/views&gt;</code>.
 *
 * <p>
 * <code>&lt;jh/ranges_ext&gt;</code> exposes pipeline/materialization closures in
 * <code>jh::ranges</code> (e.g. <code>adapt</code>, <code>collect</code>, <code>to</code>).
 * </p>
 *
 * <p>
 * <code>&lt;jh/views&gt;</code> exposes all view adaptors from
 * <code>jh::ranges::views</code>, re-exported as <code>jh::views</code>.
 * </p>
 */

#include <jh/ranges_ext>
#include <jh/views>
#include "ensure_output.h"

#if IS_WINDOWS
static EnsureOutput ensure_output_setup;
#endif

#include <algorithm>
#include <iostream>
#include <memory_resource>
#include <ranges>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>
#include <jh/runtime_arr>

namespace example::simulated {
    struct mid {
        long index;
        int id;
        std::string name;
        int value;

        mid(long i, int id_, std::string n, int v)
                : index(i), id(id_), name(std::move(n)), value(v) {}

        [[nodiscard]] std::pair<int, std::string> as_pair() const {
            return {
                    static_cast<int>(index),
                    name + ":(" + std::to_string(value) + ", " + std::to_string(id) + ")"
            };
        }
    };
}

namespace example {

    void example_adapt_runtime_arr_streamable() {
        std::cout << "\n===== adapt runtime_arr streamable =====\n\n";

        jh::runtime_arr<int> arr(3);
        for (auto [i, x]: arr | jh::views::enumerate(1)) {
            x = static_cast<int>(i * 10);
        }

        std::ostringstream out;
        for (auto x: arr | jh::ranges::adapt() | std::views::all) {
            out << x << " ";
        }
        std::cout << out.str() << "\n";
    }

    void example_collect_to_unordered_map_from_tuple() {
        std::cout << "\n===== collect to unordered_map from vector<tuple> =====\n\n";

        std::vector<std::tuple<std::string, int>> pairs = {
                {"apple", 10},
                {"banana", 20},
                {"carrot", 30}
        };

        auto map1 = pairs | jh::ranges::collect<std::unordered_map<std::string, int>>();
        auto map2 = jh::ranges::collect<std::unordered_map<std::string, int>>(pairs);

        std::cout << "map1==map2: " << std::boolalpha << (map1 == map2) << "\n";
        std::cout << "apple=" << map1.at("apple")
                  << ", banana=" << map1.at("banana")
                  << ", carrot=" << map1.at("carrot") << "\n";
    }

    void example_flatten_collect_to_pmr_unordered_map() {
        std::cout << "\n===== flatten + collect + to pmr::unordered_map =====\n\n";

        jh::runtime_arr<int> ids(3);
        jh::runtime_arr<std::string> names(3);
        jh::runtime_arr<int> values(3);

        for (auto [i, x]: ids | jh::views::enumerate(1)) {
            x = static_cast<int>(i * 10);
        }
        for (auto [i, x]: values | jh::views::enumerate()) {
            x = static_cast<int>((i + 1) * 100);
        }
        names[0] = "Alice";
        names[1] = "Bob";
        names[2] = "Carol";

        std::pmr::monotonic_buffer_resource pool;
        auto alloc = std::pmr::polymorphic_allocator<std::pair<const int, std::string>>(&pool);

        auto pmr_map = ids
                       | jh::views::enumerate(100)
                       | jh::views::zip_pipe(names, values)
                       | jh::views::flatten()
                       | jh::ranges::collect<std::vector<simulated::mid>>()
                       | jh::views::transform(&simulated::mid::as_pair)
                       | jh::ranges::to<std::pmr::unordered_map<int, std::string>>(
                0,
                std::hash<int>{},
                std::equal_to<int>{},
                alloc
        );

        std::vector<int> keys;
        keys.reserve(pmr_map.size());
        for (const auto &[k, _]: pmr_map) {
            (void) _;
            keys.push_back(k);
        }
        std::sort(keys.begin(), keys.end());

        for (int k: keys) {
            std::cout << k << " -> " << pmr_map.at(k) << "\n";
        }
    }

    void example_vis_transform_closable_to_vector() {
        std::cout << "\n===== vis_transform pipeline closable with to<vector> =====\n\n";

        std::vector<int> src{1, 2, 3, 4, 5, 6};

        auto res = src
                   | jh::views::vis_transform([](int x) { return x * 2; })
                   | jh::ranges::to<std::vector<int>>();

        for (int x: res) {
            std::cout << x << " ";
        }
        std::cout << "\n";
    }

    void example_flatten_to_vector_of_tuple() {
        std::cout << "\n===== flatten + common + to<vector<tuple>> =====\n\n";

        jh::runtime_arr<int> ids(3);
        jh::runtime_arr<std::string> names(3);

        for (auto [i, x]: ids | jh::views::enumerate(1)) {
            x = static_cast<int>(i * 10);
        }
        names[0] = "Alice";
        names[1] = "Bob";
        names[2] = "Carol";

        auto result = ids
                      | jh::views::enumerate(100)
                      | jh::views::zip_pipe(names)
                      | jh::views::flatten()
                      | jh::views::common()
                      | jh::ranges::to<std::vector<std::tuple<int, int, std::string>>>();

        for (const auto &[idx, id, name]: result) {
            std::cout << "(" << idx << "," << id << "," << name << ") ";
        }
        std::cout << "\n";
    }
} // namespace example

int main() {
    example::example_adapt_runtime_arr_streamable();
    example::example_collect_to_unordered_map_from_tuple();
    example::example_flatten_collect_to_pmr_unordered_map();
    example::example_vis_transform_closable_to_vector();
    example::example_flatten_to_vector_of_tuple();
    return 0;
}
