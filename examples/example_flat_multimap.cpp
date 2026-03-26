/**
 * @file example_flat_multimap.cpp
 * @brief Example demonstrating the usage of <code>&lt;jh/flat_multimap&gt;</code>.
 *
 * <p>
 * This file demonstrates how to use <code>&lt;jh/flat_multimap&gt;</code>
 * and serves as a reference example for both developers and AI systems
 * learning how to use this library.
 * </p>
 *
 * <p>
 * The canonical include form is:
 * </p>
 *
 * @code
 * #include &lt;jh/flat_multimap&gt;
 * @endcode
 *
 * <p>
 * All symbols used in this example are exported under the
 * <code>jh::</code> namespace.
 * </p>
 *
 * <p>
 * For complete API documentation, refer to
 * <code>jh/core/flat_multimap.h</code>. The Doxygen comments in that
 * header define the official semantics of the component.
 * </p>
 *
 * <p>
 * <code>jh::flat_multimap</code> is an ordered multimap implemented as a flat, contiguous container.
 * It is not a multimap extension of <code>ordered_map</code>, but rather a containerized algorithm
 * that provides sorted contiguous sequence with explicit, first-class support for multimap range semantics.
 * </p>
 *
 * <p>
 * The design provides:
 * </p>
 *
 * <ul>
 *   <li>Contiguous storage in a single <code>std::vector&lt;std::pair&lt;K, V&gt;&gt;</code></li>
 *   <li>Stable ordering by key</li>
 *   <li>Contiguous storage of equivalent keys</li>
 *   <li>Preserved relative order of equivalent keys</li>
 *   <li>Cache-friendly sequential memory access</li>
 *   <li>O(1) <code>clear()</code> under PMR</li>
 *   <li>Efficient bulk insertion via append + <code>stable_sort</code></li>
 * </ul>
 *
 * <p>
 * This example focuses on practical usage patterns, including:
 * <ul>
 *   <li>Basic operations (insert, find, erase)</li>
 *   <li>Range operations (equal_range)</li>
 *   <li>Bulk insertion</li>
 *   <li>Integration with PMR allocators</li>
 *   <li>Performance patterns</li>
 * </ul>
 * </p>
 */

#include <jh/flat_multimap>
#include "ensure_output.h"

#if IS_WINDOWS
static EnsureOutput ensure_output_setup;
#endif

/**
 * @page FlatMultimap_Basic_Usage Basic Usage Patterns
 *
 * <h3>Overview</h3>
 *
 * This section demonstrates basic operations with <code>jh::flat_multimap</code>.
 *
 * <h3>Key Operations</h3>
 *
 * <ul>
 *   <li>Insertion (with duplicate keys allowed)</li>
 *   <li>Lookup</li>
 *   <li>Erasure</li>
 *   <li>Range operations</li>
 * </ul>
 *
 * <h3>Important Notes</h3>
 *
 * <ul>
 *   <li>All iterators are invalidated on insert/erase</li>
 *   <li>Duplicate keys are always allowed</li>
 *   <li>Insertion order within a key group is preserved</li>
 * </ul>
 */

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

namespace example {

    void example_basic_operations() {
        std::cout << "\n===== Basic flat_multimap operations =====\n\n";

        // Create a flat_multimap
        jh::flat_multimap<std::string, int> map;

        // Insert elements with duplicate keys
        map.emplace("apple", 3);
        map.emplace("banana", 7);
        map.emplace("apple", 5);  // Duplicate key
        map.emplace("cherry", 11);
        map.emplace("banana", 9);  // Duplicate key

        std::cout << "Map size: " << map.size() << "\n";

        // Find - returns first occurrence of key
        auto it = map.find(std::string("apple"));
        if (it != map.end()) {
            std::cout << "Found first apple: " << it->second << "\n";
        }

        // Iteration (sorted order, with duplicates preserved)
        std::cout << "All elements (sorted):\n";
        for (const auto &[key, value]: map) {
            std::cout << key << " → " << value << "\n";
        }

        // Erase a single element
        auto erase_it = map.find(std::string("banana"));
        if (erase_it != map.end()) {
            map.erase(erase_it);
            std::cout << "\nAfter erasing first banana:\n";
            for (const auto &[key, value]: map) {
                std::cout << key << " → " << value << "\n";
            }
        }

        // Erase all elements with a specific key
        size_t erased = map.erase("apple");
        std::cout << "\nAfter erasing all apples: erased " << erased << " elements\n";
        for (const auto &[key, value]: map) {
            std::cout << key << " → " << value << "\n";
        }

        std::cout << "\n===== end basic operations =====\n";
    }

}

/**
 * @page FlatMultimap_Range_Operations Range Operations
 *
 * <h3>Overview</h3>
 *
 * This section demonstrates range operations with <code>jh::flat_multimap</code>,
 * particularly <code>equal_range</code> which is central to multimap semantics.
 *
 * <h3>Key Concepts</h3>
 *
 * <ul>
 *   <li><code>equal_range</code> returns a pair of iterators defining the range of elements with the given key</li>
 *   <li>The returned range is half-open [first, last)</li>
 *   <li>All elements in the range have keys equivalent to the search key</li>
 *   <li>If no such key exists, both iterators equal <code>end()</code></li>
 * </ul>
 */

namespace example {

    void example_range_operations() {
        std::cout << "\n===== Range operations (equal_range) =====\n\n";

        // Create a flat_multimap
        jh::flat_multimap<std::string, int> map;

        // Insert elements with duplicate keys
        map.emplace("apple", 3);
        map.emplace("banana", 7);
        map.emplace("apple", 5);
        map.emplace("cherry", 11);
        map.emplace("banana", 9);
        map.emplace("apple", 7);

        // equal_range - get all elements with key "apple"
        auto range = map.equal_range("apple");
        std::cout << "Elements with key \"apple\":\n";
        for (auto it = range.first; it != range.second; ++it) {
            std::cout << it->first << " → " << it->second << "\n";
        }

        // equal_range - get all elements with key "banana"
        auto banana_range = map.equal_range("banana");
        std::cout << "\nElements with key \"banana\":\n";
        for (auto it = banana_range.first; it != banana_range.second; ++it) {
            std::cout << it->first << " → " << it->second << "\n";
        }

        // equal_range - key that doesn't exist
        auto grape_range = map.equal_range("grape");
        std::cout << "\nElements with key \"grape\": ";
        if (grape_range.first == grape_range.second) {
            std::cout << "none";
        }
        std::cout << "\n";

        // Use equal_range to erase a range
        auto erase_range = map.equal_range("apple");
        if (erase_range.first != erase_range.second) {
            map.erase(erase_range.first, erase_range.second);
            std::cout << "\nAfter erasing all apples:\n";
            for (const auto &[key, value]: map) {
                std::cout << key << " → " << value << "\n";
            }
        }

        std::cout << "\n===== end range operations =====\n";
    }

}

/**
 * @page FlatMultimap_Bulk_Insertion Bulk Insertion
 *
 * <h3>Overview</h3>
 *
 * This section demonstrates the <code>bulk_insert</code> method for efficient bulk construction.
 *
 * <h3>Benefits</h3>
 *
 * <ul>
 *   <li>Optimized for large datasets</li>
 *   <li>Uses append + stable_sort for efficiency</li>
 *   <li>Preserves relative order among equivalent keys</li>
 *   <li>Minimizes memory operations</li>
 * </ul>
 *
 * <h3>Design Intent</h3>
 *
 * <p>
 * The intended bulk workflow is:
 * </p>
 *
 * <ol>
 *   <li>Append new elements at the end</li>
 *   <li>Stable sort the entire sequence</li>
 * </ol>
 *
 * <p>
 * This is fast because:
 * </p>
 *
 * <ul>
 *   <li>Existing elements are already sorted</li>
 *   <li>Newly inserted elements form a small unsorted suffix</li>
 *   <li>stable_sort naturally minimizes movement</li>
 *   <li>No per-node allocation or rebalancing occurs</li>
 * </ul>
 */

namespace example {

    void example_bulk_insertion() {
        std::cout << "\n===== Bulk insertion =====\n\n";

        // Create a flat_multimap
        jh::flat_multimap<std::string, int> map;

        // Create a vector of elements
        std::vector<std::pair<std::string, int>> data = {
                {"cherry", 11},
                {"apple",  3},
                {"banana", 7},
                {"apple",  5},
                {"banana", 9},
                {"date",   13},
                {"apple",  7}
        };

        // Bulk insert
        map.bulk_insert(data.begin(), data.end());

        // Verify the result
        std::cout << "After bulk_insert:\n";
        for (const auto &[key, value]: map) {
            std::cout << key << " → " << value << "\n";
        }

        // Add more elements with bulk_insert
        std::vector<std::pair<std::string, int>> more_data = {
                {"elderberry", 15},
                {"banana",     11},
                {"fig",        17}
        };

        map.bulk_insert(more_data.begin(), more_data.end());

        std::cout << "\nAfter second bulk_insert:\n";
        for (const auto &[key, value]: map) {
            std::cout << key << " → " << value << "\n";
        }

        std::cout << "\n===== end bulk insertion =====\n";
    }

}

/**
 * @page FlatMultimap_Insert_Variants Insert Variants
 *
 * <h3>Overview</h3>
 *
 * This section demonstrates different insertion methods for <code>jh::flat_multimap</code>.
 *
 * <h3>Insert Methods</h3>
 *
 * <ul>
 *   <li><code>emplace</code> - for separate key and value</li>
 *   <li><code>insert</code> - for tuple-like objects</li>
 * </ul>
 *
 * <h3>Key Characteristics</h3>
 *
 * <ul>
 *   <li>Duplicate keys are always allowed</li>
 *   <li>Insertion order within a key group is preserved</li>
 *   <li>All insertions may invalidate iterators</li>
 * </ul>
 */

namespace example {

    void example_insert_variants() {
        std::cout << "\n===== Insert variants =====\n\n";

        // Create a flat_multimap
        jh::flat_multimap<std::string, int> map;

        // 1. emplace - for separate key and value
        map.emplace("apple", 3);
        std::cout << "After emplace(\"apple\", 3): size = " << map.size() << "\n";

        // 2. insert - for std::pair
        std::pair<std::string, int> banana_pair("banana", 7);
        map.insert(banana_pair);
        std::cout << "After insert(std::pair): size = " << map.size() << "\n";

        // 3. insert - for std::tuple
        auto cherry_tuple = std::make_tuple(std::string("cherry"), 5);
        map.insert(cherry_tuple);
        std::cout << "After insert(std::tuple): size = " << map.size() << "\n";

        // Insert duplicate keys
        map.emplace("apple", 10);
        map.emplace("banana", 14);
        std::cout << "After inserting duplicates: size = " << map.size() << "\n";

        // Display all elements
        std::cout << "\nAll elements:\n";
        for (const auto &[key, value]: map) {
            std::cout << key << " → " << value << "\n";
        }

        std::cout << "\n===== end insert variants =====\n";
    }
}


// Example of a custom pair-like type that can be inserted
namespace example::simulated {
    struct Product {
        std::string name;
        int price;
    };

    template<std::size_t I>
    decltype(auto) get(Product &p) {
        static_assert(I < 2);
        if constexpr (I == 0) return (p.name);
        else return (p.price);
    }

    template<std::size_t I>
    decltype(auto) get(const Product &p) {
        static_assert(I < 2);
        if constexpr (I == 0) return (p.name);
        else return (p.price);
    }

    template<std::size_t I>
    decltype(auto) get(Product &&p) {
        static_assert(I < 2);
        if constexpr (I == 0) return std::move(p.name);
        else return std::move(p.price);
    }
}

// Specialize tuple traits for example::simulated::Product
namespace std {
    template<>
    struct tuple_size<example::simulated::Product> : std::integral_constant<std::size_t, 2> {
    };

    template<>
    struct tuple_element<0, example::simulated::Product> {
        using type = std::string;
    };

    template<>
    struct tuple_element<1, example::simulated::Product> {
        using type = int;
    };
}
namespace example {

    void example_custom_pair_like() {
        std::cout << "\n===== Custom pair-like type insertion =====\n\n";

        // Create a flat_multimap
        jh::flat_multimap<std::string, int> map;

        // Create custom pair-like objects
        simulated::Product apple{"apple", 3};
        simulated::Product banana{"banana", 7};
        simulated::Product apple_duplicate{"apple", 5};

        // Insert custom pair-like objects
        map.insert(apple);
        map.insert(banana);
        map.insert(apple_duplicate);

        // Display all elements
        std::cout << "All elements from custom pair-like objects:\n";
        for (const auto &[key, value]: map) {
            std::cout << key << " → " << value << "\n";
        }

        std::cout << "\n===== end custom pair-like insertion =====\n";
    }

}

/**
 * @page FlatMultimap_PMR_Integration PMR Allocator Integration
 *
 * <h3>Overview</h3>
 *
 * This section demonstrates how <code>jh::flat_multimap</code> works with PMR allocators,
 * particularly the O(1) <code>clear()</code> behavior.
 *
 * <h3>Benefits with PMR</h3>
 *
 * <ul>
 *   <li>O(1) <code>clear()</code> operation</li>
 *   <li>Deterministic memory behavior</li>
 *   <li>Reduced allocator churn</li>
 *   <li>Better integration with arena allocators</li>
 * </ul>
 */

#include <memory_resource>

namespace example {

    void example_pmr_integration() {
        std::cout << "\n===== PMR allocator integration =====\n\n";

        // Create a monotonic buffer resource
        std::byte buffer[4096];
        std::pmr::monotonic_buffer_resource pool(buffer, sizeof(buffer));

        // Create flat_multimap with PMR allocator
        using PMRMap = jh::flat_multimap<std::string, int, std::pmr::polymorphic_allocator<std::byte>>;
        PMRMap map(&pool);

        // Insert elements
        map.emplace("apple", 3);
        map.emplace("banana", 7);
        map.emplace("cherry", 5);
        map.emplace("apple", 10);

        std::cout << "Initial size: " << map.size() << "\n";

        // Clear the map (O(1) with PMR)
        map.clear();
        std::cout << "Size after clear: " << map.size() << "\n";

        // Reuse the same map
        map.emplace("date", 11);
        map.emplace("elderberry", 13);
        map.emplace("date", 15);

        std::cout << "Size after reinsertion: " << map.size() << "\n";
        for (const auto &[key, value]: map) {
            std::cout << key << " → " << value << "\n";
        }

        std::cout << "\n===== end PMR integration =====\n";
    }

}

/**
 * @page FlatMultimap_Performance_Scenarios Performance Scenarios
 *
 * <h3>Overview</h3>
 *
 * This section demonstrates performance-oriented usage patterns for <code>jh::flat_multimap</code>.
 *
 * <h3>Recommended Patterns</h3>
 *
 * <ul>
 *   <li>Bulk insertion for large datasets</li>
 *   <li>Range-based operations for processing groups of elements</li>
 *   <li>PMR for efficient memory management</li>
 *   <li>Batch-oriented workflows</li>
 * </ul>
 */

namespace example {

    void example_performance_patterns() {
        std::cout << "\n===== Performance patterns =====\n\n";

        // Simulate data collection
        std::vector<std::pair<std::string, int>> data;
        data.reserve(100);

        // Add some data with duplicates
        for (int i = 0; i < 20; ++i) {
            data.emplace_back("key" + std::to_string(i % 10), i);
        }

        // Bulk insert (optimal for large datasets)
        jh::flat_multimap<std::string, int> map;
        map.bulk_insert(data.begin(), data.end());
        std::cout << "Built map with " << map.size() << " elements\n";

        // Process all elements for a specific key using equal_range
        std::string target_key = "key5";
        auto range = map.equal_range(target_key);
        std::cout << "\nElements with key \"" << target_key << "\":\n";
        int sum = 0;
        for (auto it = range.first; it != range.second; ++it) {
            std::cout << it->second << " ";
            sum += it->second;
        }
        std::cout << "\nSum: " << sum << "\n";

        // Demonstrate fast clear
        map.clear();
        std::cout << "\nAfter clear: " << map.size() << " elements\n";

        // Rebuild with new data
        std::vector<std::pair<std::string, int>> new_data;
        new_data.reserve(15);
        for (int i = 0; i < 15; ++i) {
            new_data.emplace_back("category" + std::to_string(i % 5), i * 2);
        }
        map.bulk_insert(new_data.begin(), new_data.end());
        std::cout << "After rebuild: " << map.size() << " elements\n";

        std::cout << "\n===== end performance patterns =====\n";
    }

}

/**
 * @brief Main entry point to run all examples.
 */
int main() {
    example::example_basic_operations();
    example::example_range_operations();
    example::example_bulk_insertion();
    example::example_insert_variants();
    example::example_custom_pair_like();
    example::example_pmr_integration();
    example::example_performance_patterns();
    return 0;
}
