/**
 * @file immutable_str.h
 * @author JeongHan-Bae <mastropseudo@gmail.com>
 * @brief A lightweight immutable string with optional automatic trimming.
 *
 * @details
 * This module implements an **immutable string class** designed for **safe and efficient storage**.
 * Unlike `const std::string`, which only **restricts modification at the API level**, `immutable_str` **ensures true immutability at the data level**.
 *
 * ## Motivation
 * In C++, `std::string` is inherently mutable, even when marked as `const`. This leads to issues such as:
 * - **Unintended modification** when cast away from `const`.
 * - **Reallocation overhead** due to copy-on-write or implicit resizing.
 * - **Thread safety concerns** since `std::string`'s internal buffer can be modified in some contexts.
 *
 * `immutable_str` is designed to:
 * - **Guarantee immutability at the memory level** (no API allows modification).
 * - **Avoid unnecessary memory reallocations** (fixed-size allocation).
 * - **Be naturally thread-safe** without requiring additional synchronization.
 *
 * ## Key Differences: `immutable_str` vs. `const std::string`
 * | Feature | `immutable_str` | `const std::string` |
 * |---------|----------------|----------------------|
 * | **True Immutability** | ✅ Enforced at memory level | ❌ Can be modified via `const_cast` |
 * | **Thread Safety** | ✅ No modifications possible | ❌ Potential unsafe modifications |
 * | **Memory Efficiency** | ✅ Fixed-size allocation | ❌ May trigger reallocation |
 * | **Copy Cost** | ✅ Can be shared via `std::shared_ptr` | ✅ Can be shared via `std::shared_ptr` |
 * | **Comparison & Hashing** | ✅ Custom methods avoid full data access | ❌ Default methods may require deep comparison |
 * | **Structure Simplicity** | ✅ Minimal overhead | ❌ Larger, manages dynamic capacity |
 *
 * ## Best Practice
 * - `immutable_str` is **recommended for shared immutable storage**, especially in multithreaded applications.
 * - Unlike `const std::string`, `immutable_str` **cannot be modified in any way**, making it safer for global or cached storage.
 * - **Using `std::shared_ptr<immutable_str>` (`atomic_str_ptr`) is the preferred way to distribute immutable strings** efficiently.
 * - **Custom hash and equality functions ensure efficient comparison without extracting objects**.
 *
 * ## Key Features
 * - **Immutable & Thread-Safe**: Once created, cannot be modified.
 * - **Configurable Trimming**: `static auto_trim` flag controls whitespace removal.
 * - **Memory Efficient**: Uses `std::unique_ptr<char[]>` for compact storage.
 * - **Optimized for Hash Containers**: Custom hashing and comparison methods allow efficient key lookup.
 * - **Recommended for Shared Use**: `std::shared_ptr<immutable_str>` enables efficient string sharing.
 * - **Seamless C-string Compatibility**: Provides `c_str()`[const char*], `view()`[std::string_view()],
 * and `str()`[std::string].
 *
 * @version 1.0
 * @date 2025
 */

#pragma once

#include <memory>           // for std::unique_ptr, std::shared_ptr
#include <unordered_map>    // for std::unordered_map
#include <unordered_set>    // for std::unordered_set
#include <string>           // for std::string
#include <string_view>      // for std::string_view

namespace jh {
    /**
     * @brief Immutable string with optional automatic trimming.
     *
     * @details
     * `immutable_str` provides a **true immutable** string implementation in C++.
     * Unlike `const std::string`, it ensures **no modification, no reallocation, and no reference counting overhead**.
     *
     * - **Recommended for shared use** via `std::shared_ptr<immutable_str>` to avoid unnecessary copies.
     * - **Automatically trims** leading and trailing whitespace unless `auto_trim` is set to `false`.
     */
    struct immutable_str {
        /**
         * @brief Constructs an immutable string from a C-string.
         *
         * @param str A null-terminated C-string (ownership transferred).
         * @note Trimming behavior depends on `immutable_str::auto_trim`.
         */
        explicit immutable_str(const char *str);

        /**
         * @brief Deleted constructor to prevent unintended conversions.
         */
        template<typename T>
        explicit immutable_str(T) = delete;

        // Deleted copy/move constructors and assignment operators
        immutable_str(const immutable_str &) = delete;

        immutable_str &operator=(const immutable_str &) = delete;

        immutable_str(immutable_str &&) = delete;

        immutable_str &operator=(immutable_str &&) = delete;

        /**
         * @brief Returns the raw C-string.
         * @return Immutable C-string pointer.
         */
        [[nodiscard]] const char *c_str() const noexcept;

        /**
         * @brief Converts to `std::string`.
         * @return A `std::string` containing the immutable string data.
         */
        [[nodiscard]] std::string str() const;

        /**
         * @brief Returns a `std::string_view` for efficient access.
         * @return A `std::string_view` to the immutable string.
         */
        [[nodiscard]] std::string_view view() const noexcept;

        /**
         * @brief Returns the length of the string.
         * @return The number of characters in the string.
         */
        [[nodiscard]] uint64_t size() const noexcept;

        /**
         * @brief Checks if two `immutable_str` objects are equal.
         * @param other The other immutable string.
         * @return `true` if the strings are identical.
         */
        bool operator==(const immutable_str &other) const noexcept;

        /**
         * @brief Computes the hash value of the immutable string.
         * @return Hash value based on the string content.
         */
        [[nodiscard]] std::uint64_t hash() const noexcept;

        /**
         * @brief Global flag for controlling automatic trimming.
         *
         * - `true` (default): Trim leading/trailing whitespace.
         * - `false`: Preserve original string.
         */
        static inline bool auto_trim = true;

    private:
        uint64_t size_ = 0; ///< Length of the string
        std::unique_ptr<char[]> data_; ///< Immutable string data

        /**
         * @brief Initializes the string from a C-string, with optional trimming.
         * @param input_str A null-terminated C-string.
         */
        void init_from_string(const char *input_str);
    };

    /**
     * @brief Shared pointer alias for `immutable_str`.
     *
     * @details
     * This is the **recommended way to distribute immutable strings** efficiently.
     * Using `std::shared_ptr<immutable_str>` avoids unnecessary deep copies of string data.
     */
    using atomic_str_ptr = std::shared_ptr<immutable_str>;

    /**
     * @brief Custom hash function for `atomic_str_ptr`.
     *
     * @details
     * The default `std::shared_ptr` hashing function compares the raw pointer itself,
     * which is not useful for comparing `immutable_str` content. This custom function
     * ensures the hash is computed based on the immutable string content instead.
     */
    struct atomic_str_hash {
        std::uint64_t operator()(const atomic_str_ptr &ptr) const noexcept;
    };

    /**
     * @brief Custom equality function for `atomic_str_ptr`.
     *
     * @details
     * The default `std::shared_ptr` equality operator only checks if two shared pointers
     * refer to the same memory address. This custom function allows content-based comparison
     * of `immutable_str` instances while keeping pointer safety intact.
     */
    struct atomic_str_eq {
        bool operator()(const atomic_str_ptr &lhs, const atomic_str_ptr &rhs) const noexcept;
    };

    template<typename T>
    atomic_str_ptr make_atomic(T str) = delete;

    /**
     * @brief Creates a shared pointer to an `immutable_str`.
     *
     * @param str A null-terminated C-string.
     * @return A shared pointer to the `immutable_str`.
     */
    inline atomic_str_ptr make_atomic(const char *str) {
        return std::make_shared<immutable_str>(str);
    }
} // namespace jh
