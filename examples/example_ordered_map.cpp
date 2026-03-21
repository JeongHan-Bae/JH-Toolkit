/**
 * @file example_ordered_map.cpp
 * @brief Example demonstrating the usage of <code>jh/ordered_map</code> and <code>jh/ordered_set</code>.
 *
 * <p>
 * This file demonstrates how to use <code>jh/ordered_map</code> and <code>jh/ordered_set</code>
 * and serves as a reference example for both developers and AI systems
 * learning how to use this library.
 * </p>
 *
 * <p>
 * The canonical include form is:
 * </p>
 *
 * @code
 * #include &lt;jh/ordered_map&gt;
 * @endcode
 *
 * <p>
 * All symbols used in this example are exported under the
 * <code>jh::</code> namespace.
 * </p>
 *
 * <p>
 * For complete API documentation, refer to
 * <code>jh/core/ordered_map.h</code>. The Doxygen comments in that
 * header define the official semantics of the component.
 * </p>
 *
 * <p>
 * <code>jh::ordered_map</code> and <code>jh::ordered_set</code> are ordered associative containers
 * implemented as a contiguous AVL tree. They address several issues with standard containers:
 * </p>
 *
 * <ul>
 *   <li><code>std::unordered_*</code> series have high bucket overhead and hash collision costs</li>
 *   <li><code>std::map</code>/<code>std::set</code> have high fragmentation and poor cache locality</li>
 * </ul>
 *
 * <p>
 * The design provides:
 * </p>
 *
 * <ul>
 *   <li>Contiguous storage in a single <code>std::vector</code></li>
 *   <li>Index-based tree links instead of pointers</li>
 *   <li>Cache-friendly traversal</li>
 *   <li>O(1) <code>clear()</code> under PMR</li>
 *   <li>Fast bulk construction from sorted data</li>
 * </ul>
 *
 * <p>
 * This example focuses on practical usage patterns, including:
 * <ul>
 *   <li>Basic operations (insert, find, erase)</li>
 *   <li>Range operations (lower_bound, upper_bound, equal_range)</li>
 *   <li>Bulk construction from sorted data</li>
 *   <li>Hash-based lookup optimization</li>
 *   <li>Integration with PMR allocators</li>
 * </ul>
 * </p>
 *
 * <p>
 * <b>Key Type Considerations:</b>
 * </p>
 *
 * <p>
 * It is recommended to avoid using heavy types as keys. The implementation is not fully transparent
 * for xvalues or prvalues, which can lead to unspecified behavior
 * with <code>operator&lt;</code>. This means temporary copies may be created during comparisons.
 * </p>
 *
 * <p>
 * Types like <code>std::string</code> are generally acceptable, but excessively heavy key types
 * should be avoided to maintain performance.
 * </p>
 */

#include <jh/ordered_map>
#include "ensure_output.h"

#if IS_WINDOWS
static EnsureOutput ensure_output_setup;
#endif

/**
 * @page OrderedMap_Basic_Usage Basic Usage Patterns
 *
 * <h3>Overview</h3>
 *
 * This section demonstrates basic operations with <code>jh::ordered_map</code> and <code>jh::ordered_set</code>.
 *
 * <h3>Key Operations</h3>
 *
 * <ul>
 *   <li>Insertion</li>
 *   <li>Lookup</li>
 *   <li>Erasure</li>
 *   <li>Iteration</li>
 * </ul>
 *
 * <h3>Important Notes</h3>
 *
 * <ul>
 *   <li>All iterators are invalidated on erase</li>
 *   <li>Insertions may also relocate nodes</li>
 *   <li>Iterators are index-based, not pointer-based</li>
 * </ul>
 */

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

namespace example {

    void example_basic_operations() {
        std::cout << "\n===== Basic ordered_map operations =====\n\n";

        // Create an ordered_map
        jh::ordered_map<std::string, int> map{};

        // Insert elements using emplace for key-value pairs
        map.emplace("apple", 3);
        map.emplace("banana", 7);
        map.emplace("cherry", 5);
        map.emplace("date", 11);

        std::cout << "Map size: " << map.size() << "\n";

        // Lookup
        auto it = map.find(std::string("banana"));
        if (it != map.end()) {
            std::cout << "Found banana: " << it->second << "\n";
        }

        // Iteration
        std::cout << "All elements:\n";
        for (const auto &[key, value]: map) {
            std::cout << key << " \u2192 " << value << "\n";
        }

        // Erase
        map.erase("cherry");
        std::cout << "\nAfter erasing cherry:\n";
        for (const auto &[key, value]: map) {
            std::cout << key << " \u2192 " << value << "\n";
        }

        // Element access with operator[]
        map["elderberry"] = 13;
        std::cout << "\nAfter adding elderberry:\n";
        for (const auto &[key, value]: map) {
            std::cout << key << " \u2192 " << value << "\n";
        }

        std::cout << "\n===== end basic operations =====\n";
    }

    void example_range_operations() {
        std::cout << "\n===== Range operations (lower_bound, upper_bound, equal_range) =====\n\n";

        // Create an ordered_map
        jh::ordered_map<std::string, int> map;

        // Insert elements
        map.emplace("apple", 3);
        map.emplace("banana", 7);
        map.emplace("cherry", 5);
        map.emplace("date", 11);

        // example of how to do piecewise_construct
        map.emplace(
                std::piecewise_construct,
                std::forward_as_tuple("elderberry"),
                std::forward_as_tuple(13)
        );

        // lower_bound - first element >= key
        auto lower = map.lower_bound("cherry");
        if (lower != map.end()) {
            std::cout << "lower_bound(\"cherry\"): " << lower->first << " \u2192 " << lower->second << "\n";
        }

        // upper_bound - first element > key
        auto upper = map.upper_bound("cherry");
        if (upper != map.end()) {
            std::cout << "upper_bound(\"cherry\"): " << upper->first << " \u2192 " << upper->second << "\n";
        }

        // equal_range - [lower_bound, upper_bound)
        auto range = map.equal_range("cherry");
        std::cout << "equal_range(\"cherry\"): ";
        for (auto it = range.first; it != range.second; ++it) {
            std::cout << it->first << " \u2192 " << it->second << " ";
        }
        std::cout << "\n";

        // Note: For ordered_map, equal_range typically contains at most one element
        // since keys are unique, but it's still a required interface for associative containers

        // Demonstrate with a key that doesn't exist
        auto range_nonexistent = map.equal_range("grape");
        std::cout << "equal_range(\"grape\"): ";
        if (range_nonexistent.first == range_nonexistent.second) {
            std::cout << "empty range";
        }
        std::cout << "\n";

        std::cout << "\n===== end range operations =====\n";
    }

    void example_insert_variants() {
        std::cout << "\n===== Insert variants =====\n\n";

        // Create an ordered_map
        jh::ordered_map<std::string, int> map;

        // 1. emplace - for separate key and value
        map.emplace("apple", 3);
        std::cout << "After emplace(\"apple\", 3): size = " << map.size() << "\n";

        // 2. insert - for std::pair
        std::pair<std::string, int> banana_pair("banana", 7);
        map.insert(banana_pair);
        std::cout << "After insert(std::pair): size = " << map.size() << "\n";

        // 3. insert - for std::tuple
        auto cherry_tuple = std::make_tuple(std::string{"cherry"}, 5);
        map.insert(cherry_tuple);
        std::cout << "After insert(std::tuple): size = " << map.size() << "\n";

        // 4. insert_or_assign
        auto [it, inserted] = map.insert_or_assign(std::string{"date"}, 11);
        std::cout << "After insert_or_assign(\"date\", 11): inserted = " << (inserted ? "true" : "false") << "\n";

        // Insert_or_assign on existing key
        auto [it2, inserted2] = map.insert_or_assign(std::string{"apple"}, 10);
        std::cout << "After insert_or_assign(\"apple\", 10): inserted = " << (inserted2 ? "true" : "false")
                  << ", new value = " << it2->second << "\n";

        // Display all elements
        std::cout << "\nAll elements:\n";
        for (const auto &[key, value]: map) {
            std::cout << key << " \u2192 " << value << "\n";
        }

        std::cout << "\n===== end insert variants =====\n";
    }
}


/**
 * @page OrderedMap_Insert_Semantics Insert / Emplace / Insert-Or-Assign Semantics
 *
 * <h3>Overview</h3>
 *
 * This section explains the construction and type-matching rules for
 * <code>emplace</code>, <code>insert</code>, and <code>insert_or_assign</code>
 * in <code>jh::ordered_map</code>.
 *
 * <h3>emplace</h3>
 *
 * <p>
 * <code>emplace</code> constructs a <code>std::pair&lt;Key, Value&gt;</code>
 * directly from the provided arguments. Its behavior follows the constructors
 * of <code>std::pair</code>.
 * </p>
 *
 * <ul>
 *   <li>Accepts any arguments that can be used to construct <code>Key</code> and <code>Value</code></li>
 *   <li>Allows implicit conversions (e.g. <code>const char*</code> &rarr; <code>std::string</code>)</li>
 *   <li>Supports <code>std::piecewise_construct</code> for separate construction of key and value</li>
 * </ul>
 *
 * <p>
 * Therefore, passing types that are not exactly <code>Key</code> or <code>Value</code>,
 * but are constructible into them, is valid.
 * </p>
 *
 * <h3>insert</h3>
 *
 * <p>
 * <code>insert</code> accepts a "pair-like" object whose element types must match
 * <code>Key</code> and <code>Value</code> <b>exactly</b> after removing cv-ref qualifiers.
 * </p>
 *
 * <ul>
 *   <li>Accepts <code>std::pair&lt;Key, Value&gt;</code></li>
 *   <li>Accepts <code>std::tuple&lt;Key, Value&gt;</code></li>
 *   <li>Accepts custom types with tuple-like bindings (<code>std::tuple_size</code>, <code>get&lt;I&gt;</code>)</li>
 * </ul>
 *
 * <p>
 * Unlike <code>emplace</code>, <code>insert</code> does <b>not</b> perform type construction.
 * The types must already match exactly.
 * </p>
 *
 * <p>
 * Example:
 * </p>
 *
 * <ul>
 *   <li><code>std::pair&lt;std::string, int&gt;</code> &rarr; valid</li>
 *   <li><code>std::pair&lt;const char*, int&gt;</code> &rarr; <b>invalid</b> for <code>ordered_map&lt;std::string, int&gt;</code></li>
 * </ul>
 *
 * <h3>insert_or_assign</h3>
 *
 * <p>
 * <code>insert_or_assign</code> requires the key and value to be of type
 * <code>Key</code> and <code>Value</code> respectively.
 * </p>
 *
 * <ul>
 *   <li>Supports all value categories of <code>Key</code> and <code>Value</code>:
 *     <ul>
 *       <li>lvalue (mutable reference)</li>
 *       <li>const lvalue (const reference)</li>
 *       <li>xvalue (temporary moved object)</li>
 *       <li>rvalue (pure temporary)</li>
 *     </ul>
 *   </li>
 *   <li>Does <b>not</b> perform implicit construction</li>
 * </ul>
 *
 * <p>
 * Therefore, arguments must already be of type <code>Key</code> and <code>Value</code>.
 * For example:
 * </p>
 *
 * <ul>
 *   <li><code>std::string</code> &rarr; valid</li>
 *   <li><code>const char*</code> &rarr; <b>invalid</b> when <code>Key = std::string</code></li>
 * </ul>
 *
 * <h3>Summary</h3>
 *
 * <ul>
 *   <li><code>emplace</code>: most flexible, constructs in-place</li>
 *   <li><code>insert</code>: requires exact type match (pair-like)</li>
 *   <li><code>insert_or_assign</code>: requires exact <code>Key</code>/<code>Value</code> types</li>
 * </ul>
 */

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
// Specialize tuple traits for custom::Product
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

        // Create an ordered_map
        jh::ordered_map<std::string, int> map;

        // Create custom pair-like objects
        simulated::Product apple{"apple", 3};
        simulated::Product banana{"banana", 7};

        // Insert custom pair-like objects
        map.insert(apple);
        map.insert(banana);

        // Display all elements
        std::cout << "All elements from custom pair-like objects:\n";
        for (const auto &[key, value]: map) {
            std::cout << key << " \u2192 " << value << "\n";
        }

        std::cout << "\n===== end custom pair-like insertion =====\n";
    }

    void example_ordered_set() {
        std::cout << "\n===== Basic ordered_set operations =====\n\n";

        // Create an ordered_set
        jh::ordered_set<int> set;

        // Insert elements
        set.insert(5);
        set.insert(2);
        set.insert(8);
        set.insert(1);
        set.insert(9);

        // Lookup
        auto it = set.find(8);
        if (it != set.end()) {
            std::cout << "Found 8 in set\n";
        }

        // Iteration (sorted order)
        std::cout << "All elements (sorted):\n";
        for (int value: set) {
            std::cout << value << " ";
        }
        std::cout << "\n";

        // Erase
        set.erase(5);
        std::cout << "After erasing 5:\n";
        for (int value: set) {
            std::cout << value << " ";
        }
        std::cout << "\n";

        std::cout << "\n===== end ordered_set operations =====\n";
    }

}

/**
 * @page OrderedMap_From_Sorted Bulk Construction from Sorted Data
 *
 * <h3>Overview</h3>
 *
 * This section demonstrates the <code>from_sorted</code> method for bulk construction
 * from pre-sorted data. This method provides O(N) construction time with no rotations.
 *
 * <h3>Benefits</h3>
 *
 * <ul>
 *   <li>Perfect AVL shape</li>
 *   <li>No rotations</li>
 *   <li>Near O(N) construction</li>
 *   <li>Best possible iteration locality</li>
 * </ul>
 *
 * <h3>Recommended Pipeline</h3>
 *
 * @code
 * std::stable_sort(v.begin(), v.end());
 * v.erase(std::unique(v.begin(), v.end()), v.end());
 * auto m = jh::ordered_map&lt;K,V&gt;::from_sorted(v);
 * @endcode
 */

namespace example {

    void example_from_sorted() {
        std::cout << "\n===== from_sorted construction =====\n\n";

        // Create and populate a vector
        std::vector<std::pair<std::string, int>> data = {
                {"apple",      3},
                {"banana",     7},
                {"cherry",     5},
                {"date",       11},
                {"elderberry", 13},
                {"fig",        17}
        };

        // Sort and deduplicate
        std::stable_sort(data.begin(), data.end());
        data.erase(std::unique(data.begin(), data.end()), data.end());

        // Construct from sorted data
        auto map = jh::ordered_map<std::string, int>::from_sorted(data);

        // Verify the result
        std::cout << "Constructed from sorted data:\n";
        for (const auto &[key, value]: map) {
            std::cout << key << " \u2192 " << value << "\n";
        }

        std::cout << "\n===== end from_sorted construction =====\n";
    }

}

/**
 * @page OrderedMap_Hash_Optimization Hash-Based Lookup Optimization
 *
 * <h3>Overview</h3>
 *
 * This section demonstrates how to use <code>jh::ordered_set</code> with full hash values
 * to create a more efficient hash table alternative.
 *
 * <h3>Motivation</h3>
 *
 * <ul>
 *   <li><code>std::unordered_map</code> has high bucket overhead</li>
 *   <li>Hash collisions can lead to poor performance</li>
 *   <li><code>jh::ordered_set</code> provides O(log N) lookup with better cache locality</li>
 * </ul>
 *
 * <h3>Implementation</h3>
 *
 * By storing entries as <code>{full_hash, key, value}</code> in an <code>ordered_set</code>,
 * we can perform efficient hash-based lookup using <code>lower_bound</code>.
 */

#include <memory>

namespace example {

    // Entry structure for hash-based lookup
    struct HashEntry {
        std::size_t hash;
        std::string key;
        std::shared_ptr<int> value;

        // Comparison operator for ordered_set
        bool operator<(const HashEntry &other) const {
            if (hash != other.hash) {
                return hash < other.hash;
            }
            return key < other.key;
        }
    };

    void example_hash_optimization() {
        std::cout << "\n===== Hash-based lookup optimization =====\n\n";

        // Create ordered_set for hash-based lookup
        jh::ordered_set<HashEntry> hash_set;

        // Helper function to create hash entry
        auto make_entry = [](const std::string &key, int value) {
            return HashEntry{std::hash<std::string>{}(key), key,
                             std::make_shared<int>(value)};
        };

        // Insert entries
        hash_set.insert(make_entry("apple", 3));
        hash_set.insert(make_entry("banana", 7));
        hash_set.insert(make_entry("cherry", 5));
        hash_set.insert(make_entry("date", 11));

        // Lookup function
        auto lookup = [&](const std::string &key) -> std::shared_ptr<int> {
            std::size_t hash = std::hash<std::string>{}(key);
            HashEntry probe{hash, key, nullptr};

            auto it = hash_set.lower_bound(probe);
            if (it != hash_set.end() && it->hash == hash && it->key == key) {
                return it->value;
            }
            return nullptr;
        };

        // Test lookup
        std::vector<std::string> keys = {"banana", "cherry", "grape"};
        for (const auto &key: keys) {
            auto value = lookup(key);
            if (value) {
                std::cout << "Found " << key << " \u2192 " << *value << "\n";
            } else {
                std::cout << "Not found: " << key << "\n";
            }
        }

        std::cout << "\n===== end hash-based lookup =====\n";
    }

    // Alternative: {full_hash, index} to point to another container
    void example_hash_index() {
        std::cout << "\n===== Hash index with external storage =====\n\n";

        // External storage
        std::vector<std::pair<std::string, int>> values = {
                {"apple",  3},
                {"banana", 7},
                {"cherry", 5},
                {"date",   11}
        };

        // Hash index structure
        struct HashIndexEntry {
            std::size_t hash;
            std::size_t index;

            bool operator<(const HashIndexEntry &other) const {
                return hash < other.hash;
            }
        };

        // Create hash index
        jh::ordered_set<HashIndexEntry> hash_index;

        // Populate index
        for (std::size_t i = 0; i < values.size(); ++i) {
            std::size_t hash = std::hash<std::string>{}(values[i].first);
            hash_index.insert({hash, i});
        }

        // Lookup function
        auto lookup = [&](const std::string &key) -> std::optional<int> {
            std::size_t hash = std::hash<std::string>{}(key);
            HashIndexEntry probe{hash, 0};

            auto it = hash_index.lower_bound(probe);
            while (it != hash_index.end() && it->hash == hash) {
                const auto &[stored_key, stored_value] = values[it->index];
                if (stored_key == key) {
                    return stored_value;
                }
                ++it;
            }
            return std::nullopt;
        };

        // Test lookup
        std::vector<std::string> keys = {"banana", "cherry", "grape"};
        for (const auto &key: keys) {
            auto value = lookup(key);
            if (value) {
                std::cout << "Found " << key << " \u2192 " << *value << "\n";
            } else {
                std::cout << "Not found: " << key << "\n";
            }
        }

        std::cout << "\n===== end hash index =====\n";
    }

}

/**
 * @page OrderedMap_PMR_Integration PMR Allocator Integration
 *
 * <h3>Overview</h3>
 *
 * This section demonstrates how <code>jh::ordered_map</code> works with PMR allocators,
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

        // Create ordered_map with PMR allocator
        using PMRMap = jh::ordered_map<std::string, int, std::pmr::polymorphic_allocator<std::byte>>;
        PMRMap map(&pool);

        // Insert elements using emplace for key-value pairs
        map.emplace("apple", 3);
        map.emplace("banana", 7);
        map.emplace("cherry", 5);

        std::cout << "Initial size: " << map.size() << "\n";

        // Clear the map (O(1) with PMR)
        map.clear();
        std::cout << "Size after clear: " << map.size() << "\n";

        // Reuse the same map
        map.emplace("date", 11);
        map.emplace("elderberry", 13);

        std::cout << "Size after reinsertion: " << map.size() << "\n";
        for (const auto &[key, value]: map) {
            std::cout << key << " \u2192 " << value << "\n";
        }

        std::cout << "\n===== end PMR integration =====\n";
    }

}

/**
 * @page OrderedMap_Performance_Scenarios Performance Scenarios
 *
 * <h3>Overview</h3>
 *
 * This section demonstrates performance-oriented usage patterns for <code>jh::ordered_map</code>.
 *
 * <h3>Recommended Patterns</h3>
 *
 * <ul>
 *   <li>Bulk construction from sorted data</li>
 *   <li>Periodic rebuild for optimal locality</li>
 *   <li>PMR for efficient memory management</li>
 * </ul>
 */

namespace example {

    void example_performance_patterns() {
        std::cout << "\n===== Performance patterns =====\n\n";

        // Simulate data collection
        std::vector<std::pair<std::string, int>> data;
        data.reserve(100);
        for (int i = 0; i < 100; ++i) {
            data.emplace_back("key" + std::to_string(i), i);
        }

        // Sort and deduplicate
        std::stable_sort(data.begin(), data.end());
        data.erase(std::unique(data.begin(), data.end()), data.end());

        // Build from sorted data (optimal)
        auto map = jh::ordered_map<std::string, int>::from_sorted(data);
        std::cout << "Built map with " << map.size() << " elements\n";

        // Simulate some modifications
        for (int i = 100; i < 110; ++i) {
            map.emplace("key" + std::to_string(i), i);
        }
        std::cout << "After additions: " << map.size() << " elements\n";

        // Rebuild for optimal locality
        map = jh::ordered_map<std::string, int>::from_sorted(std::move(map));
        std::cout << "After rebuild: " << map.size() << " elements\n";

        // Demonstrate fast clear
        map.clear();
        std::cout << "After clear: " << map.size() << " elements\n";

        std::cout << "\n===== end performance patterns =====\n";
    }

}

/**
 * @page OrderedMap_Ranges_Integration ordered_map with jh::ranges
 *
 * <h3>Overview</h3>
 *
 * This section demonstrates the recommended way to construct a
 * <code>jh::ordered_map</code> from a <code>jh::ranges</code> pipeline.
 *
 * <p>
 * The key idea is:
 * </p>
 *
 * <ul>
 *   <li>Use ranges to generate structured data (tuple / pair-like)</li>
 *   <li>Materialize into a contiguous container (e.g. <code>std::vector</code>)</li>
 *   <li>Sort and deduplicate</li>
 *   <li>Construct using <code>ordered_map::from_sorted</code></li>
 * </ul>
 *
 * <p>
 * This approach provides:
 * </p>
 *
 * <ul>
 *   <li>O(N) construction time</li>
 *   <li>Perfect AVL tree shape</li>
 *   <li>Optimal cache locality</li>
 * </ul>
 *
 * <h3>Important Note</h3>
 *
 * <p>
 * It is <b>NOT recommended</b> to construct <code>ordered_map</code> using
 * <code>jh::ranges::collect</code> or repeated insertion from a range:
 * </p>
 *
 * @code
 * // NOT recommended
 * auto map = range | jh::ranges::collect&lt;jh::ordered_map&lt;K,V&gt;&gt;();
 * @endcode
 *
 * <p>
 * This leads to O(N log N) complexity and unnecessary AVL rotations.
 * </p>
 *
 * <p>
 * Instead, always prefer:
 * </p>
 *
 * @code
 * // Recommended pipeline
 * auto vec = range | ... | collect&lt;std::vector&lt;std::pair&lt;K,V&gt;&gt;&gt;();
 * std::stable_sort(...);
 * vec.erase(std::unique(...), vec.end());
 * auto map = jh::ordered_map&lt;K,V&gt;::from_sorted(vec);
 * @endcode
 *
 * <h3>Example</h3>
 *
 * <p>
 * The following example demonstrates a full pipeline:
 * </p>
 *
 * <ul>
 *   <li>enumerate + zip to combine multiple sequences</li>
 *   <li>flatten nested tuples</li>
 *   <li>transform into key-value pairs</li>
 *   <li>build ordered_map efficiently</li>
 * </ul>
 */


#include <jh/ranges_ext>
#include <jh/views>
#include <sstream>
#include <jh/runtime_arr>

namespace example {

    void example_ordered_map_ranges() {
        std::cout << "\n===== ordered_map with ranges =====\n\n";

        // ------------------------------------------------------
        // Prepare input sequences
        // ------------------------------------------------------
        jh::runtime_arr<int> ids(5);
        jh::runtime_arr<std::string> names(5);
        jh::runtime_arr<int> values(5);

        for (auto [i, x]: ids | jh::ranges::views::enumerate(1))
            x = static_cast<int>(i * 10);

        for (auto [i, x]: values | jh::ranges::views::enumerate())
            x = static_cast<int>((i + 1) * 100);

        names[0] = "Alice";
        names[1] = "Bob";
        names[2] = "Carol";
        names[3] = "Dave";
        names[4] = "Eve";

        // ------------------------------------------------------
        // Step 1: Build vector from ranges pipeline
        // ------------------------------------------------------
        auto vec =
                ids
                | jh::ranges::views::enumerate(100)
                | jh::ranges::views::zip_pipe(names, values)
                | jh::ranges::views::flatten()
                | jh::ranges::views::transform([](auto &&t) {
                    auto [idx, id, name, value] = t;
                    return std::pair{
                            id,
                            name + ":" + std::to_string(value)
                    };
                })
                | jh::ranges::collect<std::vector<std::pair<int, std::string>>>();

        // ------------------------------------------------------
        // Step 2: Sort and deduplicate
        // ------------------------------------------------------
        std::stable_sort(vec.begin(), vec.end(),
                         [](const auto &a, const auto &b) {
                             return a.first < b.first;
                         });

        vec.erase(std::unique(vec.begin(), vec.end(),
                              [](const auto &a, const auto &b) {
                                  return a.first == b.first;
                              }),
                  vec.end());

        // ------------------------------------------------------
        // Step 3: Construct ordered_map (O(N))
        // ------------------------------------------------------
        auto map = jh::ordered_map<int, std::string>::from_sorted(vec);

        // ------------------------------------------------------
        // Verify result
        // ------------------------------------------------------
        std::cout << "Constructed ordered_map:\n";
        for (const auto &[k, v]: map) {
            std::cout << k << " \u2192 " << v << "\n";
        }

        std::cout << "\n===== end ordered_map with ranges =====\n";
    }

} // namespace example

/**
 * @brief Main entry point to run all examples.
 */
int main() {
    example::example_basic_operations();
    example::example_ordered_set();
    example::example_range_operations();
    example::example_insert_variants();
    example::example_custom_pair_like();
    example::example_from_sorted();
    example::example_hash_optimization();
    example::example_hash_index();
    example::example_pmr_integration();
    example::example_performance_patterns();
    example::example_ordered_map_ranges();
    return 0;
}
