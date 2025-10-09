/**
 * Copyright 2025 JeongHan-Bae <mastropseudo@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


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
 * In C++, `std::string` is inherently mutable, even when marked as `const`. This leads to issues such as
 * - **Unintended modification** when cast away from `const`.
 * - **Reallocation overhead** due to *copy-on-write* or implicit resizing.
 * - **Thread safety concerns** since `std::string`'s internal buffer can be modified in some contexts.
 *
 * `immutable_str` is designed to:
 * - **Guarantee immutability at the memory level** (no API allows modification).
 * - **Avoid unnecessary memory reallocations** (fixed-size allocation).
 * - **Be naturally thread-safe** without requiring additional synchronization.
 *
 * ## Key Differences: `immutable_str` vs. `const std::string`
 * | Feature                  | `immutable_str`                         | `const std::string`                           |
 * |--------------------------|-----------------------------------------|-----------------------------------------------|
 * | **True Immutability**    | ✅ Enforced at memory level             | ❌ Can be modified via `const_cast`           |
 * | **Thread Safety**        | ✅ No modifications possible            | ❌ Potential unsafe modifications             |
 * | **Memory Efficiency**    | ✅ Fixed-size allocation                | ❌ May trigger reallocation                   |
 * | **Copy Cost**            | ✅ Can be shared via `std::shared_ptr`  | ✅ Can be shared via `std::shared_ptr`        |
 * | **Comparison & Hashing** | ✅ Custom methods avoid full data access| ❌ Default methods may require deep comparison|
 * | **Structure Simplicity** | ✅ Minimal overhead                     | ❌ Larger, manages dynamic capacity           |
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
 * - **Memory Efficient**: Uses `std::unique_ptr<const char[]>` for compact storage.
 * - **Optimized for Hash Containers**: Custom hashing and comparison methods allow efficient key lookup.
 * - **Recommended for Shared Use**: `std::shared_ptr<immutable_str>` enables efficient string sharing.
 * - **Seamless C-string Compatibility**: Provides `c_str()`[const char*], `view()`[std::string_view()],
 * and `str()`[std::string].
 * - **Constructing**: Supports construction from [C-strings] and [`std::string_view` with mutex protection].
 * - **Pooling Support**: Compatible with `jh::pool` for efficient object pooling.
 *
 * @version 1.3.x
 * @date 2025
 */

#pragma once

#ifndef JH_IMMUTABLE_STR_AUTO_TRIM
#define JH_IMMUTABLE_STR_AUTO_TRIM true
#endif

#include <algorithm>        // for std::max
#include <memory>           // for std::unique_ptr, std::shared_ptr
#include <unordered_map>    // NOLINT for std::unordered_map
#include <unordered_set>    // NOLINT for std::unordered_set
#include <string>           // for std::string
#include <cstring>          // for strlen
#include <string_view>      // for std::string_view
#include <cstdint>          // for std::uint64_t
#include <optional>         // for std::optional
#include <type_traits>      // for std::remove_cvref_t
#include "jh/pool.h"
#include "jh/pods/string_view.h"

namespace jh::detail {
    constexpr bool is_space_ascii(const unsigned char ch) {
        return ch == ' ' || ch == '\t' || ch == '\n' ||
               ch == '\v' || ch == '\f' || ch == '\r';
    }
}

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
    struct immutable_str final {
        /**
         * @brief Constructs an immutable string from a C-string.
         *
         * @param str A null-terminated C-string (ownership transferred).
         * @note
         * - This constructor is `explicit` to **prevent unintended implicit conversions** when passing single values.
         *   This ensures that initialization does not involve unexpected conversions that could lead to **data races**
         *   or modification of the intended input.
         * - The `const char*` parameter is specifically chosen for **seamless compatibility** with
         *   LLVM `extern "C"` APIs, ensuring safe and efficient interoperability with C-style strings.
         * - Trimming behavior depends on `immutable_str::auto_trim`.
         */
        explicit immutable_str(const char *str);

        /**
         * @brief Deleted constructor to prevent unintended conversions.
         */
        template<typename T>
        explicit immutable_str(T) = delete;

        /**
         * @brief Constructs an immutable string from a `std::string_view` with a mutex.
         *
         * @param sv A `std::string_view` representing the string data.
         * @param mtx A reference to the `std::mutex` that protects the lifetime of the base-struct containing `sv`.
         *
         * @throws std::logic_error If `sv` contains embedded null (`\0`) characters.
         *
         * @warning The caller must ensure that `mtx` is the correct mutex protecting `sv`.
         *          If an unrelated mutex is provided, undefined behavior may occur.
         */
        immutable_str(std::string_view sv, std::mutex &mtx);

        /**
         * @brief Deleted copy constructor.
         * @details
         * `immutable_str` manages its string data using `std::unique_ptr<const char[]>`, which
         * enforces exclusive ownership. Copying would require duplicating the underlying string
         * data, which is not permitted.
         * - To prevent unintended shallow copies and enforce immutability, the copy constructor
         *   is explicitly deleted.
         */
        immutable_str(const immutable_str &) = delete;

        /**
         * @brief Deleted move constructor.
         * @details
         * Unlike typical movable types, `immutable_str` does not support move semantics because
         * immutable objects should not be transferred but rather shared.
         * - Movement would mean transferring ownership of the internal buffer, which contradicts
         *   the intended immutability.
         * - Instead of moving, `immutable_str` instances should be managed with
         *   `std::shared_ptr<jh::immutable_str>` to enable safe sharing.
         */
        immutable_str(immutable_str &&) = delete;

        /**
         * @brief Deleted move assignment operator.
         * @details
         * Move assignment is disallowed because `immutable_str` is designed to be immutable, meaning
         * its internal state should not be modified after creation.
         * - Transferring ownership through assignment would break this principle.
         * - Rather than moving, use `std::shared_ptr<jh::immutable_str>` to safely share instances.
         */
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
         * @brief Returns a `pod::string_view` for efficient access.
         * @return A `jh::pod::string_view` to the immutable string.
         */
        [[nodiscard]] pod::string_view pod_view() const noexcept;

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
         *
         * @note
         * `auto_trim` is a static global setting. Modifying it at runtime in a multithreaded environment
         * may lead to race conditions. All instances of `immutable_str` will respect the current policy.
         */
        static constexpr bool auto_trim = JH_IMMUTABLE_STR_AUTO_TRIM;

        [[maybe_unused]] static bool is_static_built();

    private:
        uint64_t size_ = 0; ///< Length of the string
        std::unique_ptr<const char[]> data_; ///< Immutable string data
        mutable std::optional<std::uint64_t> hash_{std::nullopt}; ///< Cached hash value
        mutable std::once_flag hash_flag_; ///< Ensures thread-safe lazy initialization

        /**
         * @brief Initializes the string from a C-string, with optional trimming.
         * @param input_str A null-terminated C-string.
         * @param input_len len If known else -1
         */
        void init_from_string(const char *input_str, std::uint64_t input_len = static_cast<std::uint64_t>(-1)) {
#if defined(JH_IMMUTABLE_STR_AUTO_TRIM) && JH_IMMUTABLE_STR_AUTO_TRIM
            init_from_string_trim(input_str, input_len);
#else
            init_from_string_no_trim(input_str, input_len);
#endif
        }

        void init_from_string_trim(const char *input_str, std::uint64_t input_len);

        void init_from_string_no_trim(const char *input_str, std::uint64_t input_len);

    };

    /**
     * @brief Shared pointer alias for `immutable_str`.
     *
     * @details
     * This is the **recommended way to distribute immutable strings** efficiently.
     * Using `std::shared_ptr<immutable_str>` avoids unnecessary deep copies of string data.
     */
    using atomic_str_ptr = std::shared_ptr<immutable_str>;
    using weak_str_ptr [[maybe_unused]] = std::weak_ptr<immutable_str>;

    /**
     * @brief Concept for immutable string compatible types.
     *
     * @details
     * This concept is satisfied by:
     * - `atomic_str_ptr` (and its cv/ref-qualified variants)
     * - `const char*`
     * - string literals (e.g. `"hello"`, decayed to `const char*`)
     *
     * It is used to constrain templates (e.g., custom hash/equality) so that they
     * only accept types safely comparable to `immutable_str` content.
     *
     * @tparam U Candidate type to check.
     */
    template<typename U>
    concept is_immutable_str =
    std::same_as<std::remove_cvref_t<U>, atomic_str_ptr> ||
    std::same_as<std::decay_t<U>, const char *>;

    /**
     * @brief Custom hash function for `atomic_str_ptr`.
     *
     * @details
     * The default `std::shared_ptr` hashing function compares the raw pointer itself,
     * which is not useful for comparing `immutable_str` content. This custom function
     * ensures the hash is computed based on the immutable string content instead.
     * - Supports hashing for **both** `shared_ptr<immutable_str>` and `const char*`.
     * - Allows **direct lookups using string literals** in hash-based containers, without requiring a temporary `immutable_str`.
     * - **Auto-trims whitespace** (if `immutable_str::auto_trim` is enabled) before computing the hash.
     * - **Optimized for hash containers** to ensure efficient key lookup.
     * @note If `immutable_str::auto_trim` is enabled, leading and trailing spaces are **ignored** in hash calculations.
     *
     * @tparam U The type of the input value (either `atomic_str_ptr` or `const char*`).
     * @param value The value to hash.
     * @return The computed hash of the string content.
     */
    struct atomic_str_hash {
        using is_transparent = void; ///< Enables `find(const char*)` in hash-based containers.
        template<typename U>
        requires is_immutable_str<U>
        std::uint64_t operator()(const U &value) const noexcept {
            if constexpr (std::same_as<std::remove_cvref_t<U>, atomic_str_ptr>) {
                return value ? value->hash() : 0;
            } else {
                if (value == nullptr) {
                    return 0;
                }
                if constexpr (immutable_str::auto_trim) {
                    const std::uint64_t len = std::strlen(value); // Get `const char*` length
                    std::uint64_t leading = 0, trailing = len;
                    while (leading < len && detail::is_space_ascii(static_cast<unsigned char>(value[leading]))) {
                        ++leading;
                    }
                    while (trailing > leading &&
                           detail::is_space_ascii(static_cast<unsigned char>(value[trailing - 1]))) {
                        --trailing;
                    }
                    return std::hash<std::string_view>{}(std::string_view{value + leading, trailing - leading});
                }
                return std::hash<std::string_view>{}(std::string_view{value});  // NOLINT if !auto_trim
            }
        }
    };

    /**
     * @brief Custom equality function for `atomic_str_ptr`.
     *
     * @details
     * The default `std::shared_ptr` equality operator only checks if two shared pointers
     * refer to the same memory address. This custom function allows content-based comparison
     * of `immutable_str` instances while keeping pointer safety intact.
     * - Supports hashing for **both** `shared_ptr<immutable_str>` and `const char*`.
     * - Allows **direct lookups using string literals** in hash-based containers, without requiring a temporary `immutable_str`.
     * - **Auto-trims whitespace** (if `immutable_str::auto_trim` is enabled) before computing the hash.
     * - **Optimized for hash containers** to ensure efficient key lookup.
     * @note If `immutable_str::auto_trim` is enabled, leading and trailing spaces are **ignored** in hash calculations.
     *
     * @tparam U The type of the left-hand side value (either `atomic_str_ptr` or `const char*`).
     * @tparam V The type of the right-hand side value (either `atomic_str_ptr` or `const char*`).
     * @param lhs The left-hand side value to compare.
     * @param rhs The right-hand side value to compare.
     * @return `true` if the strings are equal. `false` if either value is `nullptr` or the strings are not equal.
     */
    struct atomic_str_eq {
        using is_transparent = void; ///< Enables `find(const char*)` in hash-based containers.

        template<typename U, typename V>
        requires (is_immutable_str<U> && is_immutable_str<V>)
        bool operator()(const U &lhs, const V &rhs) const noexcept {
            if constexpr (std::same_as<std::remove_cvref_t<U>, atomic_str_ptr> &&
                          std::same_as<std::remove_cvref_t<V>, atomic_str_ptr>) {
                return lhs && rhs && *lhs == *rhs;
            } else {
                if (lhs == nullptr || rhs == nullptr) return false; // nullptr are considered as not eq
                std::string_view lhs_view, rhs_view;

                if constexpr (std::same_as<std::remove_cvref_t<U>, atomic_str_ptr> &&
                              std::same_as<std::decay_t<V>, const char *>) {
                    lhs_view = lhs->view();
                    if constexpr (immutable_str::auto_trim) {
                        const auto &[leading, size_] = trim(rhs);
                        rhs_view = std::string_view(rhs + leading, size_);
                    } else {
                        rhs_view = std::string_view(rhs);  // NOLINT if !auto_trim
                    }
                } else {
                    rhs_view = rhs->view();
                    if constexpr (immutable_str::auto_trim) {
                        const auto &[leading, size_] = trim(lhs);
                        lhs_view = std::string_view(lhs + leading, size_);
                    } else {
                        lhs_view = std::string_view(lhs);  // NOLINT if !auto_trim
                    }
                }
                return lhs_view == rhs_view;
            }
        }

    private:
        static std::pair<uint64_t, uint64_t> trim(const char *str) noexcept {
            std::uint64_t leading = 0, trailing = std::strlen(str);

            while (leading < trailing && detail::is_space_ascii(static_cast<unsigned char>(str[leading]))) {
                ++leading;
            }
            while (trailing > leading && detail::is_space_ascii(static_cast<unsigned char>(str[trailing - 1]))) {
                --trailing;
            }
            return {leading, trailing - leading};
        }
    };

    template<typename T>
    [[maybe_unused]] atomic_str_ptr make_atomic(T str) = delete;

    /**
     * @brief Creates a shared pointer to an `immutable_str`.
     *
     * @param str A null-terminated C-string.
     * @return A shared pointer to the `immutable_str`.
     */
    inline atomic_str_ptr make_atomic(const char *str) {
        return std::make_shared<immutable_str>(str);
    }

    /**
     * @brief Creates a shared pointer to an `immutable_str` from a `std::string_view` with a mutex.
     *
     * @param sv A `std::string_view` representing the string data.
     * @param mtx A reference to the `std::mutex` that protects the lifetime of the base-struct containing `sv`.
     *
     * @throws std::logic_error If `sv` contains embedded null (`\0`) characters.
     *
     * @warning The caller must ensure that `mtx` is the correct mutex protecting `sv`.
     *          If an unrelated mutex is provided, undefined behavior may occur.
     *
     * @return A shared pointer to an `immutable_str`, ensuring safe access.
     *
     * @note This function is useful when dealing with `std::string_view` obtained from temporary or mutable sources
     *       that require explicit locking to guarantee a valid string lifetime.
     */
    inline atomic_str_ptr safe_from(std::string_view sv, std::mutex &mtx) {
        return std::make_shared<immutable_str>(sv, mtx);
    }

} // namespace jh


#include "jh/marcos/header_begin.h"

namespace jh {
#if JH_INTERNAL_SHOULD_DEFINE

    JH_INLINE immutable_str::immutable_str(const char *str) {
        init_from_string(str);
    }

    JH_INLINE immutable_str::immutable_str(const std::string_view sv, std::mutex &mtx) {
        std::lock_guard lock(mtx);
        if (std::strlen(sv.data()) != sv.size()) {
            throw std::logic_error(
                    "jh::immutable_str does not support string views containing embedded null characters.");
        }
        init_from_string(sv.data(), sv.size());
    }

    JH_INLINE const char *immutable_str::c_str() const noexcept {
        return data_.get();
    }

    JH_INLINE std::string immutable_str::str() const {
        return {data_.get(), size_};
    }

    JH_INLINE std::string_view immutable_str::view() const noexcept {
        return {data_.get(), size_};
    }

    JH_INLINE pod::string_view immutable_str::pod_view() const noexcept {
        return {this->c_str(), this->size()};
    }

    JH_INLINE uint64_t immutable_str::size() const noexcept {
        return size_;
    }

    JH_INLINE bool immutable_str::operator==(const immutable_str &other) const noexcept {
        return std::strcmp(data_.get(), other.data_.get()) == 0;
    }

    JH_INLINE std::uint64_t immutable_str::hash() const noexcept {
        std::call_once(hash_flag_, [this] {
            hash_.emplace(std::hash<std::string_view>{}(std::string_view(data_.get(), size_)));
        });
        return hash_.value();
    }

    JH_INLINE void immutable_str::init_from_string_trim(const char *input_str,
                                                        std::uint64_t input_len) {
        if (!input_str) [[unlikely]] {
            // Initialize an empty string if input is null
            size_ = 0;
            auto data_array_ = std::make_unique<char[]>(1);
            data_array_[0] = '\0';
            data_ = std::move(data_array_);
            return;
        }

        const char *start = input_str;
        if (input_len == static_cast<std::size_t>(-1)) {
            input_len = std::strlen(input_str);  // fallback only if not known
        }

        const char *end = input_str + input_len - 1;

        // If auto_trim is enabled, remove leading and trailing whitespace

        while (*start && detail::is_space_ascii(*start)) ++start;

        // If the input contains only whitespace, treat it as an empty string
        if (*start == '\0') [[unlikely]] {
            size_ = 0;
            auto data_array_ = std::make_unique<char[]>(1);
            data_array_[0] = '\0';
            data_ = std::move(data_array_);
            return;
        }

        while (end > start && detail::is_space_ascii(*end)) --end;


        // Compute the final string size
        size_ = end - start + 1;

        // Allocate memory and copy the string
        auto data_array_ = std::make_unique<char[]>(size_ + 1);
        std::memcpy(data_array_.get(), start, size_);
        data_array_[size_] = '\0';
        data_ = std::move(data_array_);
    }

    JH_INLINE void immutable_str::init_from_string_no_trim(const char *input_str,
                                                           std::uint64_t input_len) {
        if (!input_str) [[unlikely]] {
            // Initialize an empty string if input is null
            size_ = 0;
            auto data_array_ = std::make_unique<char[]>(1);
            data_array_[0] = '\0';
            data_ = std::move(data_array_);
            return;
        }

        const char *start = input_str;
        if (input_len == static_cast<std::size_t>(-1)) {
            input_len = std::strlen(input_str);  // fallback only if not known
        }

        const char *end = input_str + input_len - 1;

        // Compute the final string size
        size_ = end - start + 1;

        // Allocate memory and copy the string
        auto data_array_ = std::make_unique<char[]>(size_ + 1);
        std::memcpy(data_array_.get(), start, size_);
        data_array_[size_] = '\0';
        data_ = std::move(data_array_);
    }

    [[maybe_unused]] JH_INLINE bool immutable_str::is_static_built(){
#ifdef JH_IS_STATIC_BUILD
        return true;
#else
        return false;
#endif // JH_IS_STATIC_BUILD
    }

#endif // JH_INTERNAL_SHOULD_DEFINE
}

#include "jh/marcos/header_end.h"