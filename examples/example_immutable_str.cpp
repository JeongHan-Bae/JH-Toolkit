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
 * - **Efficient Storage**: Uses `std::unique_ptr<const char[]>` to minimize memory overhead.
 * - **Automatic Trimming**: Optionally removes leading/trailing whitespace.
 * - **Optimized for Hashing**: Designed for use in hash containers.
 * - **Supports Shared Ownership**: `std::shared_ptr<immutable_str>` allows efficient reference sharing.
 *
 * ## Best Practices
 * ✅ **Use `immutable_str` for fixed strings**, especially in **multithreaded applications**.
 * ✅ **Avoid direct construction from `std::string_view` or `std::string`** to prevent unintended behavior.
 * ✅ **Single-parameter constructor only accepts `const char*`**, ensuring safe type conversions and LLVM extern "C" compatibility.
 * ✅ **For `std::string_view` inputs, use a protecting mutex** to prevent data race issues.
 * ✅ **Use `make_atomic()` or `safe_from()`** for shared immutable string creation.
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
     * - **When `auto_trim` is enabled**, leading/trailing spaces are removed:
     *   ```cpp
     *   jh::immutable_str::auto_trim = true;
     *   jh::immutable_str str("  Trimmed  ");
     *   std::cout << str.view();  // Output: "Trimmed"
     *   ```
     * - **When `auto_trim` is disabled**, spaces are preserved:
     *   ```cpp
     *   jh::immutable_str::auto_trim = false;
     *   jh::immutable_str str("  Not Trimmed  ");
     *   std::cout << str.view();  // Output: "  Not Trimmed  "
     *   ```
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

    /**
     * @brief Demonstrates safe construction from `std::string_view` with a mutex.
     *
     * - Ensures the **source data is properly synchronized** before creating an immutable string.
     */
    void safe_construct() {
        std::cout << "\n🔹 Safe Construction with `std::string_view`:\n";

        std::mutex mtx;
        const std::string shared_data = "Thread-safe string";

        // Safe construction with mutex protection
        const jh::atomic_str_ptr safe_str = jh::safe_from(shared_data, mtx);

        std::cout << "Safely constructed immutable string: " << safe_str->view() << "\n";
    }

    /**
     * @brief Demonstrates pooling of immutable strings.
     *
     * - Uses `jh::pool<immutable_str>` to avoid duplicate allocations.
     * - Shows that **identical strings are pooled together**.
     */
    void pooling() {
        std::cout << "\n🔹 Pooling Immutable Strings:\n";

        jh::pool<jh::immutable_str> string_pool;

        auto pooled1 = string_pool.acquire("Pooled String");
        auto pooled2 = string_pool.acquire("Pooled String");
        const auto pooled3 = string_pool.acquire("Different String");

        std::cout << "Pooled1 == Pooled2: " << (pooled1 == pooled2) << "\n";
        std::cout << "Pooled1 != Pooled3: " << (pooled1 != pooled3) << "\n";
        std::cout << "Pool size: " << string_pool.size() << "\n";

        pooled1.reset();
        pooled2.reset();

        string_pool.cleanup(); // Remove expired references
        std::cout << "After cleanup, pool size: " << string_pool.size() << "\n";
    }

    /**
     * @brief Demonstrates how to safely use `switch` with `jh::immutable_str` hashing.
     * .
     * ## Understanding `jh::immutable_str::hash()`
     * - Just like `std::string` and `std::string_view`, `jh::immutable_str` provides a **runtime hash**.
     * - The default hash function (`jh::immutable_str::hash()`) uses `std::hash<std::string_view>` internally.
     * - Since this hash is **computed at runtime**, it is **not `constexpr`** and **cannot be directly used in `switch-case`**.
     * .
     * ## Why `std::unordered_map<size_t, size_t>` is Not Recommended
     * - A common workaround is mapping hashes to unique identifiers in a `std::unordered_map<size_t, size_t>`.
     * - This allows `switch` to work by providing **constexpr-compatible values**.
     * - ❗ However, **hash collisions can occur**, potentially causing incorrect matches if different strings map to the same hash value.
     * - While additional validation (`str_ptr->view() == "case"`) can mitigate this issue, it **is not a foolproof solution**.
     * .
     * ## Recommended Approach: Use `jh::atomic_str_ptr` as the Key
     * - Instead of storing raw hashes (`size_t`), use `std::shared_ptr<jh::immutable_str>` (`jh::atomic_str_ptr`) as the key.
     * - This approach **completely eliminates hash collisions** because keys are immutable string objects rather than numeric hash values.
     * - `std::unordered_map<jh::atomic_str_ptr, size_t, jh::atomic_str_hash, jh::atomic_str_eq>` ensures **safe and precise matching**.
     * - This method is both **efficient and scalable**, making it the preferred way to implement `switch`-like behavior.
     */
    void switch_case_usage(const char *str) {
        jh::pool<jh::immutable_str> string_pool;

        /*
         * Recommended method: Directly map atomic immutable strings to unique identifiers
         * **Use independently created `make_atomic()` instead of `pool.acquire()`**
         *
         * ## Design Rationale:
         * 1. **Avoid affecting object lifecycle in the pool**
         *    - If `unordered_map` stores keys acquired from `pool.acquire()`, those objects
         *      will persist in the pool, preventing proper cleanup.
         *
         * 2. **Ensure correct matching behavior**
         *    - `unordered_map` should store unique immutable string objects for consistent key lookup.
         *    - This ensures `atomic_str_ptr` comparison works correctly using `jh::atomic_str_hash` and `jh::atomic_str_eq`.
         *
         * 3. **Enable proper memory cleanup**
         *    - By using `make_atomic()` directly, `unordered_map` does not retain references to pooled objects.
         *    - This allows `string_pool.cleanup()` or auto-cleanup behaviors to properly release unreferenced objects.
         */

        static const auto immutable_map = std::unordered_map<jh::atomic_str_ptr, size_t, jh::atomic_str_hash, jh::atomic_str_eq>{
            {jh::make_atomic("hello world"), 1},
            {jh::make_atomic("example string"), 2},
            {jh::make_atomic("another_string"), 3}
        };

        const auto it = immutable_map.find(str);

        if (it == immutable_map.end()) {
            std::cout << "String not matched" << std::endl;
            return;
        }

        switch (it->second) {
            case 1:
                std::cout << "Matched String: 'hello world'" << std::endl;
            break;
            case 2:
                std::cout << "Matched String: 'example string'" << std::endl;
            break;
            case 3:
                std::cout << "Matched String: 'another_string'" << std::endl;
            break;
            default:
                std::cout << "String not matched" << std::endl;
        }
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
    example::safe_construct();
    example::pooling();
    example::switch_case_usage("hello world");
    example::switch_case_usage("example string");
    example::switch_case_usage("another_string");
    example::switch_case_usage("some random string");
    return 0;
}
