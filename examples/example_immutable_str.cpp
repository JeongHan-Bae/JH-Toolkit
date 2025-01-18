/**
 * @file example_immutable_str.cpp
 * @brief Demonstrates the usage of `immutable_str` in `jh-toolkit`.
 *
 * ## Overview
 * `jh::immutable_str` is a lightweight, immutable string designed for **safe and efficient storage**.
 * Unlike `std::string`, it enforces **true immutability** at the memory level.
 *
 * ## Key Features
 * - **Immutable & Thread-Safe**: Once created, it cannot be modified.
 * - **Efficient Storage**: Uses `std::unique_ptr<char[]>` to minimize memory overhead.
 * - **Automatic Trimming**: Optionally removes leading/trailing whitespace.
 * - **Optimized for Hashing**: Designed for use in hash containers.
 * - **Supports Shared Ownership**: `std::shared_ptr<immutable_str>` allows efficient reference sharing.
 *
 * ## Best Practices
 * - Use `immutable_str` when **storing fixed strings**, especially in **multithreaded applications**.
 * - Avoid passing `std::string_view` or `std::string` directly to the constructor to prevent unintended behavior.
 * - Utilize `make_atomic()` to create shared immutable strings efficiently.
 *
 * ## Linking Requirement
 * Since this module has an associated **source file**, ensure that you **link against `jh-toolkit-impl`**
 * when building your project.
 *
 * ```cmake
 * target_link_libraries({your_project} PRIVATE jh-toolkit-impl)
 * ```
 * This ensures all necessary header files are available for compilation.
 */

#include "jh/immutable_str.h"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace example {
    /**
     * @brief Demonstrates basic usage of `immutable_str`
     *
     * - Directly creates an `immutable_str` from a **string literal**.
     * - Simulates a real-world scenario where **a buffer is mutable** but converted into an immutable string.
     */
    void basic_usage() {
        std::cout << "🔹 Basic Usage:\n";

        // Directly create an immutable string from a string literal
        const jh::immutable_str imm_str1("Hello, Immutable World!");
        std::cout << "Immutable String: " << imm_str1.view() << "\n";

        // Simulating a real-world buffer scenario where data is mutable
        std::vector<char> buffer;
        const std::vector helper{'T', 'e', 's', 't', '\0'};
        buffer.insert(buffer.end(), helper.begin(), helper.end());

        // Explicit cast to const char* to ensure immutability
        const jh::immutable_str imm_str2(static_cast<const char *>(buffer.data()));
        std::cout << "Immutable from Buffer: " << imm_str2.view() << "\n";

        std::cout << "Size of imm_str1: " << imm_str1.size() << "\n";
        std::cout << "Size of imm_str2: " << imm_str2.size() << "\n";
    }

    /**
     * @brief Demonstrates hashing and comparison of immutable strings.
     *
     * - Uses `make_atomic()` to create **shared immutable strings**.
     * - Tests whether **hash values and string contents match**.
     */
    void hashing_and_comparison() {
        std::cout << "\n🔹 Hashing & Comparison:\n";

        // Creating shared immutable strings
        const auto atomic1 = jh::make_atomic("Shared Immutable String");
        const auto atomic2 = jh::make_atomic("Shared Immutable String");
        const auto atomic3 = jh::make_atomic("Different String");

        std::cout << "Hash match (atomic1 vs atomic2): " << (atomic1->hash() == atomic2->hash()) << "\n";
        std::cout << "String match (atomic1 vs atomic2): " << (*atomic1 == *atomic2) << "\n";
        std::cout << "String match (atomic1 vs atomic3): " << (*atomic1 == *atomic3) << "\n";
    }

    /**
     * @brief Demonstrates the automatic trimming feature of `immutable_str`.
     *
     * - **When `auto_trim` is enabled**, leading/trailing spaces are removed.
     * - **When `auto_trim` is disabled**, spaces are preserved.
     */
    void auto_trim_behavior() {
        std::cout << "\n🔹 Auto Trim Behavior:\n";

        jh::immutable_str::auto_trim = true;
        const jh::immutable_str trimmed("   Trimmed String   ");
        const jh::immutable_str normal("Trimmed String");

        std::cout << "Auto-trim enabled: " << trimmed.view() << "\n";
        std::cout << "Trimmed equals normal: " << (trimmed == normal) << "\n";

        jh::immutable_str::auto_trim = false;
        const jh::immutable_str untrimmed("   Trimmed String   ");

        std::cout << "Auto-trim disabled: " << untrimmed.view() << "\n";
        std::cout << "Untrimmed equals normal: " << (untrimmed == normal) << "\n";
    }

    /**
     * @brief Demonstrates using `atomic_str_ptr` (`std::shared_ptr<immutable_str>`) in **hash containers**.
     *
     * - Stores **immutable strings as keys** in `std::unordered_map`.
     * - Stores **unique immutable strings** in `std::unordered_set`.
     */
    void hash_container_usage() {
        std::cout << "\n🔹 Using `atomic_str_ptr` in Hash Containers:\n";

        // Using immutable strings as keys in a hash map
        std::unordered_map<jh::atomic_str_ptr, int, jh::atomic_str_hash, jh::atomic_str_eq> immutable_map;

        const auto key1 = jh::make_atomic("Immutable Key 1");
        const auto key2 = jh::make_atomic("Immutable Key 2");
        const auto key3 = jh::make_atomic("Immutable Key 1"); // Same content as key1

        immutable_map[key1] = 100;
        immutable_map[key2] = 200;
        immutable_map[key3] = 300; // Overwrites key1 since they have the same content

        std::cout << "Map size: " << immutable_map.size() << "\n";
        std::cout << "Value for '" << key1->view() << "': " << immutable_map[key1] << "\n";
        std::cout << "Value for '" << key2->view() << "': " << immutable_map[key2] << "\n";

        // Using immutable strings in a hash set
        std::unordered_set<jh::atomic_str_ptr, jh::atomic_str_hash, jh::atomic_str_eq> immutable_set;

        immutable_set.insert(jh::make_atomic("Unique String 1"));
        immutable_set.insert(jh::make_atomic("Unique String 2"));
        immutable_set.insert(jh::make_atomic("Unique String 1")); // Duplicate, should not be inserted again

        std::cout << "Set size (should be 2): " << immutable_set.size() << "\n";
    }
} // namespace example

/**
 * @brief Main entry point to run all examples.
 */
int main() {
    example::basic_usage();
    example::hashing_and_comparison();
    example::auto_trim_behavior();
    example::hash_container_usage();
    return 0;
}
