/**
 * @copyright
 * Copyright 2025 JeongHan-Bae &lt;mastropseudo\@gmail.com&gt;
 * <br>
 * Licensed under the Apache License, Version 2.0 (the "License"); <br>
 * you may not use this file except in compliance with the License.<br>
 * You may obtain a copy of the License at<br>
 * <br>
 *     http://www.apache.org/licenses/LICENSE-2.0<br>
 * <br>
 * Unless required by applicable law or agreed to in writing, software<br>
 * distributed under the License is distributed on an "AS IS" BASIS,<br>
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.<br>
 * See the License for the specific language governing permissions and<br>
 * limitations under the License.<br>
 * <br>
 * Full license: <a href="https://github.com/JeongHan-Bae/JH-Toolkit?tab=Apache-2.0-1-ov-file#readme">GitHub</a>
 */
/**
 * @file string_view.h
 * @brief POD-safe <code>string_view</code> with full <code>constexpr</code> semantics and consteval/runtime dual-path optimization.
 *
 * This header defines a lightweight, read-only, non-owning string view
 * specifically designed for <b>POD-compatible</b> use cases, offering
 * <code>constexpr</code> evaluation and optimized runtime behavior.
 *
 * <h3>Highlights:</h3>
 * <ul>
 *   <li><b>POD layout</b>: strictly <code>{ const char*, uint64_t }</code></li>
 *   <li><b>All operations constexpr</b>: usable in both compile-time (<code>consteval</code>) and runtime contexts</li>
 *   <li><b>Dual-path design</b>:
 *       <ul>
 *         <li>Constexpr-safe logic for compile-time evaluation</li>
 *         <li>Accelerated runtime path using <code>memcmp</code> / <code>memcpy</code></li>
 *       </ul>
 *   </li>
 *   <li><b>Deep comparison</b> via <code>memcmp</code>, not pointer identity</li>
 *   <li><b>Constexpr hash</b> supported &mdash; usable in <code>consteval</code> expressions</li>
 *   <li>Designed as a safe view type for <code>immutable_str</code> and POD containers</li>
 * </ul>
 *
 * <h3>Usage Notes:</h3>
 * <ul>
 *   <li>Provides zero-overhead interop with <code>std::string_view</code></li>
 *   <li>Supports compile-time construction via <code>from_literal()</code> or <code>"..."_psv</code></li>
 *   <li>No dynamic allocation, no exceptions, fully constexpr evaluable</li>
 *   <li>Enables compile-time hashing (e.g., <code>jh::meta::t_str</code>)</li>
 * </ul>
 */

#pragma once

#include <algorithm>
#include <compare>
#include <cstdint>
#include <cstring>      // for memcmp, memcpy
#include <string_view>  // for std::string_view interoperability
#include <type_traits>
#include <cstddef>

#include "jh/metax/char.h"
#include "jh/pods/pod_like.h"
#include "jh/detail/base64_common.h"
#include "jh/metax/hash.h"

namespace jh::pod {

    /**
     * @brief Read-only string view with POD layout.
     *
     * Holds a raw pointer and a length (not null-terminated).
     * Provides slicing, comparison, and constexpr hashing, all
     * while remaining fully POD (<code>const char* + uint64_t</code>).
     *
     * <h4>Key differences from std::string_view:</h4>
     * <ul>
     *   <li>Always POD layout (aggregate struct, no constructors)</li>
     *   <li><code>operator==</code> performs <b>deep comparison</b> (memcmp)</li>
     *   <li><code>hash()</code> is <b>constexpr-safe</b> and usable in consteval contexts</li>
     *   <li>No exceptions, no allocator awareness</li>
     * </ul>
     *
     * <h4>Initialization semantics:</h4>
     * <ul>
     *   <li>This type is an aggregate; initialization normally requires both
     *       a <code>const char*</code> and an explicit length.</li>
     *   <li>If only a <code>const char*</code> is provided (without <code>len</code>),
     *       then <code>len</code> defaults to <code>0</code>, producing an empty view.</li>
     *   <li>A special helper <code>from_literal()</code> exists for string literals:
     *       it deduces the array size at compile time and creates a view of length <code>N-1</code>
     *       (excluding the null terminator).</li>
     *   <li>At runtime, you must always provide both pointer and size explicitly
     *       &mdash; this class never computes length automatically.</li>
     * </ul>
     *
     * <h4>Usage Model:</h4>
     * <ul>
     *   <li>Typically used as a safe view for <code>immutable_str</code></li>
     *   <li>Can represent string literals or arena-allocated strings</li>
     *   <li>Compile-time hashing supports <code>jh::meta::t_str</code></li>
     * </ul>
     */
    struct string_view final {
        const char *data;       ///< Pointer to string data (not null-terminated)
        std::uint64_t len;      ///< Number of valid bytes in the view

        using value_type = char;                                 ///< Character type.
        using size_type = std::uint64_t;                         ///< Size type (64-bit).
        using difference_type = std::ptrdiff_t;                  ///< Difference type.
        using reference = value_type &;                          ///< Reference to character.
        using const_reference = const value_type &;              ///< Const reference to character.
        using pointer = value_type *;                            ///< Pointer to character.
        using const_pointer = const value_type *;                ///< Const pointer to character.

        /**
         * @brief Construct a <code>string_view</code> from a string literal.
         *
         * @tparam N Size of the string literal including the null terminator.
         * @param lit Reference to the string literal (must be null-terminated).
         * @return A <code>string_view</code> pointing to the literal characters
         *         with <code>size() == N - 1</code>.
         *
         * <h4>Semantics:</h4>
         * <ul>
         *   <li><code>N</code> always counts the null terminator.</li>
         *   <li>The resulting view excludes the null terminator, so length is <code>N - 1</code>.</li>
         *   <li>Empty string literal <code>""</code> is valid (<code>N == 1</code>, view <tt>length = 0</tt>).</li>
         * </ul>
         *
         * @note This overload guarantees constexpr evaluation and can be
         *       used in <code>consteval</code> contexts.
         */
        template<std::size_t N>
        requires (N > 0)
        [[nodiscard]] static constexpr string_view from_literal(const char (&lit)[N]) noexcept {
            return {lit, static_cast<std::uint64_t>(N - 1)};
        }

        /// @brief Index access (no bounds checking).
        constexpr const_reference operator[](const std::uint64_t index) const noexcept {
            return data[index];
        }

        /// @brief Pointer to the beginning of data.
        [[nodiscard]] constexpr const_pointer begin() const noexcept { return data; }

        /**
         * @brief Pointer to the end of data (<code>data + len</code>).
         *
         * @note This is not null-terminated. Use <code>len</code> for bounds.
         */
        [[nodiscard]] constexpr const_pointer end() const noexcept { return data + len; }

        /// @brief View length in bytes.
        [[nodiscard]] constexpr size_type size() const noexcept { return len; }

        /// @brief Whether the view is empty (<code>len == 0</code>).
        [[nodiscard]] constexpr bool empty() const noexcept { return len == 0; }

        /**
         * @brief Compare two views for byte-wise equality.
         *
         * Performs a <strong>deep comparison</strong> of contents using <code>memcmp</code>,
         * rather than checking pointer identity.
         *
         * @param rhs Another <code>string_view</code> to compare.
         * @return <code>true</code> if contents are equal, <code>false</code> otherwise.
         */
        constexpr bool operator==(const string_view &rhs) const noexcept {
            if (len != rhs.len)
                return false;

            if (std::is_constant_evaluated()) {
                for (std::uint64_t i = 0; i < len; ++i)
                    if (data[i] != rhs.data[i])
                        return false;
                return true;
            } else {
                return std::memcmp(data, rhs.data, len) == 0;
            }
        }

        /// @brief Sentinel value representing "no position" or "until the end".
        static constexpr auto npos = static_cast<std::uint64_t>(-1);

        /**
         * @brief Returns a substring starting at <code>offset</code>, for <code>length</code> bytes.
         *
         * <ul>
         *   <li>If <code>length == jh::pod::string_view::npos</code>, the view extends to the end.</li>
         *   <li>If <code>length == 0</code>, the result is an empty view.</li>
         *   <li>If <code>offset > len</code>, returns an empty view.</li>
         * </ul>
         *
         * @param offset Starting byte index (0-based).
         * @param @param length Number of bytes. Use <code>jh::pod::string_view::npos</code>
              to read until the end of the view.
         * @return A new <code>string_view</code> into the specified subrange.
         *
         * @note
         * This behavior intentionally mirrors the semantics of
         * <code>std::string_view::substr</code>, where
         * <code>npos</code> represents "read until the end".
         */
        [[nodiscard]] constexpr string_view
        sub(std::uint64_t offset, std::uint64_t length = npos) const noexcept {

            if (offset >= len)
                return {nullptr, 0};

            const std::uint64_t remaining = len - offset;

            const std::uint64_t real_len =
                    (length == npos || length > remaining)
                    ? remaining
                    : length;

            return {data + offset, real_len};
        }

        /**
         * @brief Lexical comparison (similar to <code>strcmp()</code>).
         * @return <code>&lt;0</code> if <tt>this &lt; rhs</tt>,
         *         <code>0</code> if <tt>equal</tt>,
         *         <code>&gt;0</code> if <tt>this &gt; rhs</tt>.
         */
        [[nodiscard]] constexpr int compare(const string_view &rhs) const noexcept {
            const std::uint64_t min_len = len < rhs.len ? len : rhs.len;

            if (std::is_constant_evaluated()) {
                // constexpr path: manual loop
                for (std::uint64_t i = 0; i < min_len; i++) {
                    if (data[i] < rhs.data[i]) return -1;
                    if (data[i] > rhs.data[i]) return 1;
                }
            } else {
                // runtime path: use memcmp
                if (int cmp = std::memcmp(data, rhs.data, min_len); cmp != 0) {
                    return cmp;
                }
            }

            return static_cast<int>(len) - static_cast<int>(rhs.len);
        }

        /// @brief Check whether this view starts with the given <code>prefix</code>.
        [[nodiscard]] constexpr bool starts_with(const string_view &prefix) const noexcept {
            if (prefix.len > len)
                return false;

            if (std::is_constant_evaluated()) {
                // constexpr path
                for (std::uint64_t i = 0; i < prefix.len; ++i)
                    if (data[i] != prefix.data[i])
                        return false;
                return true;
            } else {
                // runtime path
                return std::memcmp(data, prefix.data, prefix.len) == 0;
            }
        }

        /// @brief Check whether this view ends with the given <code>suffix</code>.
        [[nodiscard]] constexpr bool ends_with(const string_view &suffix) const noexcept {
            if (suffix.len > len)
                return false;

            const std::uint64_t offset = len - suffix.len;

            if (std::is_constant_evaluated()) {
                // constexpr path
                for (std::uint64_t i = 0; i < suffix.len; ++i)
                    if (data[offset + i] != suffix.data[i])
                        return false;
                return true;
            } else {
                // runtime path
                return std::memcmp(data + offset, suffix.data, suffix.len) == 0;
            }
        }

        /**
         * @brief Returns the index of the first occurrence of a character.
         *
         * @param ch Target character to search for.
         * @return Offset index if found, or <code>-1</code> (as <code>uint64_t</code>) if not found.
         */
        [[nodiscard]] constexpr std::uint64_t find(const char ch) const noexcept {
            for (std::uint64_t i = 0; i < len; ++i)
                if (data[i] == ch) return i;
            return static_cast<std::uint64_t>(-1); // not found
        }

        /**
         * @brief Hash the view content using a selectable non-cryptographic algorithm.
         *
         * Provides stable 64-bit hashing over the view contents.
         *
         * @param hash_method Algorithm to use for hashing (default: <code>fnv1a64</code>).
         * @return 64-bit hash of the view data, or <code>-1</code> if <code>data == nullptr</code>.
         *
         * @note
         * <ul>
         *   <li>This is <strong>not cryptographic</strong>; do not use it for security-sensitive logic.</li>
         *   <li>If <code>data</code> is null, the return value is <code>-1</code> (sentinel).</li>
         *   <li>Hashing is based only on contents and length, not on pointer identity.</li>
         *   <li>
         *     Unlike <code>bytes_view::hash</code>, this function is <strong>valid in consteval contexts</strong>.
         *     <ul>
         *       <li><code>bytes_view</code> relies on <code>reinterpret_cast</code>, so it cannot be evaluated at compile time.</li>
         *       <li><code>string_view</code> operates directly on characters, so compile-time hashing of string literals
         *           is both <strong>well-defined</strong> and <strong>semantically meaningful</strong>.</li>
         *       <li>This design enables features such as <code>jh::meta::t_str</code> to compute hashes fully at compile time.</li>
         *     </ul></li>
         * </ul>
         */
        [[nodiscard]] constexpr std::uint64_t
        hash(jh::meta::c_hash hash_method = jh::meta::c_hash::fnv1a64) const noexcept {
            if (!data) return static_cast<std::uint64_t>(-1);
            return meta::hash(hash_method, data, len);
        }

        /**
         * @brief Check if all characters are decimal digits (0-9).
         * @note This only checks that each character is a digit.
         *       To validate if the whole string represents a number
         *       (with optional sign, decimal point, or exponent),
         *       use @c is_number() instead.
         * @return true if all characters are digits, false otherwise.
         */
        [[nodiscard]] constexpr bool is_digit() const noexcept {
            if (std::is_constant_evaluated()) {
                for (std::uint64_t i = 0; i < size(); ++i) {
                    if (!jh::meta::is_digit(data[i]))
                        return false;
                }
                return true;
            } else {
                return std::all_of(
                        begin(),
                        end(),
                        [](char c) {
                            return jh::meta::is_digit(c);
                        }
                );
            }
        }

        /**
         * @brief Check if the string represents a valid decimal number.
         *
         * @return <code>true</code> if the string is a valid number, otherwise <code>false</code>.
         *
         * @details
         * Grammar (simplified BNF):
         * <pre>
         *   [ '+' | '-' ] DIGIT+ [ '.' DIGIT+ ] [ ( 'e' | 'E' ) [ '+' | '-' ] DIGIT+ ]
         * </pre>
         *
         * Equivalent regular expression:
         * <pre>
         *   ^[+-]?[0-9]+(&bsol;.[0-9]+)?([eE][+-]?[0-9]+)?$
         * </pre>
         *
         * Rules:
         * <ul>
         *   <li>The first character may be <code>'+'</code> or <code>'-'</code>.</li>
         *   <li>At least one digit must appear before optional '.' or 'e/E'.</li>
         *   <li>If '.' appears, at least one digit must follow (either before or after '.').</li>
         *   <li>If 'e' or 'E' appears, it must be followed by an optional sign and at least one digit.</li>
         *   <li>Only decimal notation is supported (no hex, octal, binary, or locale-specific formats).</li>
         * </ul>
         */
        [[nodiscard]] constexpr bool is_number() const noexcept {
            const std::uint64_t n = size();
            if (n == 0) return false;

            std::uint64_t i = 0;
            if (data[i] == '+' || data[i] == '-') {
                ++i;
            }

            bool has_digit = false;
            bool seen_dot = false;
            bool seen_exp = false;

            for (; i < n; ++i) {
                const char c = data[i];

                /// do NOT apply [[likely]] as this is constexpr
                if (jh::meta::is_digit(c)) {
                    has_digit = true;
                    continue;
                }

                if (c == '.') {
                    if (!has_digit || seen_dot || seen_exp) return false; // must have digit before '.'
                    seen_dot = true;
                    has_digit = false; // must see digit after '.'
                    continue;
                }

                if (c == 'e' || c == 'E') {
                    if (!has_digit || seen_exp) return false; // must have digit before 'e'
                    seen_exp = true;
                    has_digit = false; // must see digit after 'e'
                    if (i + 1 < n && (data[i + 1] == '+' || data[i + 1] == '-')) {
                        ++i; // skip optional sign after e/E
                        // no leak risk, worst case reach '\0'
                    }
                    continue;
                }
                return false; // invalid character
            }
            return has_digit;
        }

        /**
         * @brief Check if all characters are alphabetic (A-Z, a-z).
         * @return true if all characters are alphabetic, false otherwise.
         */
        [[nodiscard]] constexpr bool is_alpha() const noexcept {
            if (std::is_constant_evaluated()) {
                for (std::uint64_t i = 0; i < size(); ++i) {
                    if (!jh::meta::is_alpha(data[i]))
                        return false;
                }
                return true;
            } else {
                return std::all_of(
                        begin(),
                        end(),
                        [](char c) {
                            return jh::meta::is_alpha(c);
                        }
                );
            }
        }

        /**
         * @brief Check if all characters are alphanumeric (letters or digits).
         * @return true if all characters are alphanumeric, false otherwise.
         */
        [[nodiscard]] constexpr bool is_alnum() const noexcept {
            if (std::is_constant_evaluated()) {
                for (std::uint64_t i = 0; i < size(); ++i)
                    if (!jh::meta::is_alnum(data[i]))
                        return false;
                return true;
            } else {
                return std::all_of(
                        begin(),
                        end(),
                        [](char c) {
                            return jh::meta::is_alnum(c);
                        }
                );
            }
        }

        /**
         * @brief Check if all characters are 7-bit ASCII.
         * @return true if all characters are in range 0-127, false otherwise.
         */
        [[nodiscard]] constexpr bool is_ascii() const noexcept {
            if (std::is_constant_evaluated()) {
                for (std::uint64_t i = 0; i < size(); ++i)
                    if (!jh::meta::is_ascii(data[i]))
                        return false;
                return true;
            } else {
                return std::all_of(
                        begin(),
                        end(),
                        [](char c) {
                            return jh::meta::is_ascii(c);
                        }
                );
            }
        }

        /**
         * @brief Check if all characters are printable 7-bit ASCII.
         * @return true if all characters are in range 32-126, false otherwise.
         *
         * @details
         * Verifies that every character lies within the printable
         * 7-bit ASCII range (decimal 32-126).
         *
         * @note
         * Printable ASCII is a strict subset of 7-bit ASCII.
         * Therefore: <code>is_printable_ascii()</code> implies <code>is_ascii()</code>
         * <br>
         * If this function returns true, calling @c is_ascii()
         * again is redundant. When used inside a @c requires clause,
         * do not combine the two checks.
         * <br>
         * This function only permits ASCII characters.
         * If the intention is to validate fully printable text
         * including multi-byte UTF-8 sequences, use @c is_legal()
         * instead.
         * @note
         * @c is_legal() performs:
         * <ul>
         *  <li>UTF-8 structural validation</li>
         *  <li>rejection of invalid UTF-8 byte combinations</li>
         *  <li>rejection of illegal ASCII control characters</li>
         * </ul>
         */
        [[nodiscard]] constexpr bool is_printable_ascii() const noexcept {
            if (std::is_constant_evaluated()) {
                for (std::uint64_t i = 0; i < size(); ++i)
                    if (!jh::meta::is_printable_ascii(data[i]))
                        return false;
                return true;
            } else {
                return std::all_of(
                        begin(),
                        end(),
                        [](char c) {
                            return jh::meta::is_printable_ascii(c);
                        }
                );
            }
        }

        /**
         * @brief Check if all characters are valid (printable ASCII or UTF-8).
         * @return true if all characters are valid, false otherwise.
         */
        [[nodiscard]] constexpr bool is_legal() const noexcept {
            std::uint64_t i = 0;
            int remaining = 0;       // how many continuation bytes still expected
            unsigned char lead = 0;  // last leading byte

            while (i < size()) {
                auto c = static_cast<unsigned char>(data[i]);
                // filter out disallowed ASCII control characters
                if (!jh::meta::is_valid_char(static_cast<char>(c))) return false;
                ///< constexpr, avoid using [[likely/unlikely]]
                if (remaining == 0) {
                    // --- leading byte ---
                    if (c <= 0x7F) {
                        // single-byte ASCII
                        i++;
                        continue;
                    } else if (c >= 0xC2 && c <= 0xDF) {
                        // 2-byte sequence
                        remaining = 1;
                        lead = c;
                    } else if (c >= 0xE0 && c <= 0xEF) {
                        // 3-byte sequence
                        remaining = 2;
                        lead = c;
                    } else if (c >= 0xF0 && c <= 0xF4) {
                        // 4-byte sequence
                        remaining = 3;
                        lead = c;
                    } else {
                        return false; // invalid leading byte
                    }
                } else {
                    // --- continuation byte ---
                    if ((c & 0xC0) != 0x80) return false;
                    // special restrictions for the first continuation
                    if (remaining == ((lead >= 0xE0 && lead <= 0xEF) ? 2 :
                                      (lead >= 0xF0 && lead <= 0xF4) ? 3 : 1)) {
                        if (lead == 0xE0 && (c < 0xA0 || c > 0xBF)) return false;
                        if (lead == 0xED && (c < 0x80 || c > 0x9F)) return false;
                        if (lead == 0xF0 && (c < 0x90 || c > 0xBF)) return false;
                        if (lead == 0xF4 && (c < 0x80 || c > 0x8F)) return false;
                    }
                    remaining--;
                }
                i++;
            }
            return remaining == 0;
        }

        /**
         * @brief Check if the string is a valid hexadecimal sequence.
         * @details Length must be even, and all characters must be hex digits.
         * @return true if valid hex string, false otherwise.
         */
        [[nodiscard]] constexpr bool is_hex() const noexcept {
            if (size() % 2 != 0)
                return false;

            if (std::is_constant_evaluated()) {
                for (std::uint64_t i = 0; i < size(); ++i)
                    if (!jh::meta::is_hex_char(data[i]))
                        return false;
                return true;
            } else {
                return std::all_of(
                        begin(),
                        end(),
                        [](char c) {
                            return jh::meta::is_hex_char(c);
                        }
                );
            }
        }

        /**
         * @brief Check if the string is valid Base64.
         * @details Length must be a multiple of 4, padding ('=') allowed at the end.
         * @return true if valid Base64, false otherwise.
         */
        [[nodiscard]] constexpr bool is_base64() const noexcept {
            return jh::detail::base64_common::is_base64(data, size());
        }

        /**
         * @brief Check if the string is valid Base64URL.
         * @details '=' padding is optional. If present, length must be a multiple of 4.
         * @return true if valid Base64URL, false otherwise.
         */
        [[nodiscard]] constexpr bool is_base64url() const noexcept {
            return jh::detail::base64_common::is_base64url(data, size());
        }

        /**
         * @brief Copy the view into a C-style null-terminated buffer.
         *
         * @warning This is not POD-safe. Intended for debugging or interop only.
         *
         * @param buffer Output character buffer.
         * @param max_len Maximum bytes to write (including null terminator).
         */
        void copy_to(char *buffer, const std::uint64_t max_len) const noexcept {
            const std::uint64_t n = len < max_len - 1 ? len : max_len - 1;
            std::memcpy(buffer, data, n);
            buffer[n] = '\0';
        }

        /**
         * @brief Returns the semantic length of the UTF-8 string.
         *
         * Counts the number of Unicode code points represented in this view,
         * rather than the number of raw bytes. This function assumes that the
         * underlying data is valid UTF-8.
         *
         * <h4>Definition:</h4>
         * <ul>
         *   <li>A new code point is identified by a byte that is <b>not</b>
         *       a UTF-8 continuation byte (<code>10xxxxxx</code>).</li>
         *   <li>Continuation bytes are excluded from the count.</li>
         * </ul>
         *
         * <h4>Evaluation Model:</h4>
         * <ul>
         *   <li>In constant-evaluated contexts, a fully <code>constexpr</code>
         *       UTF-8 scan is performed.</li>
         *   <li>At runtime, the implementation may delegate to optimized
         *       standard library algorithms.</li>
         * </ul>
         *
         * <h4>Important Notes:</h4>
         * <ul>
         *   <li>This function counts <b>Unicode code points</b>, not grapheme clusters.</li>
         *   <li>Multi-code-point sequences (e.g. emoji ZWJ sequences or
         *       combining characters) are counted individually.</li>
         *   <li>No UTF-8 validation is performed.</li>
         * </ul>
         *
         * @return Number of Unicode code points in the view,
         *         or <code>0</code> if the view is empty.
         *
         * @note The computation of grapheme clusters will never be provided,
         *       as it is evident that in software development, this is a front-end requirement
         *       rather than a back-end one, and the systems upon which grapheme clusters depend
         *       are excessively cumbersome.
         */
        [[nodiscard]] constexpr std::uint64_t semantic_len() const noexcept {
            if (!data || len == 0)
                return 0;

            if (std::is_constant_evaluated()) {
                // constexpr path: count non-continuation bytes
                std::uint64_t count = 0;
                for (std::uint64_t i = 0; i < len; ++i) {
                    const auto c = static_cast<unsigned char>(data[i]);
                    if ((c & 0b11000000) != 0b10000000)
                        ++count;
                }
                return count;
            } else {
                // runtime path: use std::count_if for efficiency
                return static_cast<std::uint64_t>(
                        std::count_if(
                                data,
                                data + len,
                                [](unsigned char c) {
                                    return (c & 0b11000000) != 0b10000000;
                                }
                        )
                );
            }
        }

        /**
         * @brief Explicit conversion to <code>std::string_view</code>.
         *
         * Provides safe, zero-overhead interoperability with the standard library.
         * This conversion preserves both pointer and length semantics without
         * affecting POD compatibility.
         *
         * <p>
         * <b>Semantics:</b>
         * </p>
         * <ul>
         *   <li>Conversion is <b>explicit</b> &mdash; requires <code>static_cast</code> or brace-init form.</li>
         *   <li>Performs no allocation or copy; simply wraps existing data.</li>
         *   <li>Pointer and size are preserved exactly (1:1 mapping).</li>
         * </ul>
         *
         * @note Explicit to avoid unintended implicit conversions in overload resolution.
         *       See also: <code>to_std()</code> for named equivalent.
         */
        explicit constexpr operator std::string_view() const noexcept {
            return {data, static_cast<std::size_t>(len)};
        }

        /**
         * @brief Named conversion helper to obtain a <code>std::string_view</code>.
         *
         * Functionally identical to <code>explicit operator std::string_view()</code>,
         * but callable in normal expressions without <code>static_cast</code>.
         *
         * <p>
         * <b>Use cases:</b>
         * </p>
         * <ul>
         *   <li>Improves readability in non-template or mixed API contexts.</li>
         *   <li>Convenient when passing to standard library functions expecting <code>std::string_view</code>.</li>
         * </ul>
         *
         * @see operator std::string_view()
         */
        [[nodiscard]] constexpr std::string_view to_std() const noexcept {
            return {data, static_cast<std::size_t>(len)};
        }

        /**
         * @brief Three-way comparison operator (spaceship operator).
         *
         * Performs a <b>lexicographical three-way comparison</b> between two
         * <code>string_view</code> instances, returning a value of type
         * <code>std::strong_ordering</code>.
         *
         * <h4>Semantics:</h4>
         * <ul>
         *   <li>Returns <code>std::strong_ordering::less</code>  if <tt>*this &lt; rhs</tt></li>
         *   <li>Returns <code>std::strong_ordering::equal</code> if <tt>*this == rhs</tt></li>
         *   <li>Returns <code>std::strong_ordering::greater</code> if <tt>*this &gt; rhs</tt></li>
         * </ul>
         *
         * The comparison is implemented in terms of <code>compare()</code>,
         * and therefore follows identical lexicographic ordering rules.
         * This ensures <b>bitwise consistency</b> between <code>compare()</code>,
         * <code>operator==</code>, and all derived relational operators.
         *
         * <h4>Properties:</h4>
         * <ul>
         *   <li>Guaranteed <b>constexpr</b> and <b>noexcept</b>.</li>
         *   <li>Implements a <b>strict total ordering</b> (same as <code>std::string_view</code>).</li>
         *   <li>Automatically enables all relational operators
         *       (<code>&lt;, &lt;=, &gt;, &gt;=</code>) via the compiler.</li>
         * </ul>
         *
         * @param rhs The right-hand side <code>string_view</code> to compare against.
         * @return <code>std::strong_ordering</code> value indicating the lexicographic relation.
         *
         * @see compare()
         * @see operator==()
         */
        constexpr std::strong_ordering operator<=>(const string_view &rhs) const noexcept {
            const int cmp = compare(rhs);
            if (cmp < 0) return std::strong_ordering::less;
            if (cmp > 0) return std::strong_ordering::greater;
            return std::strong_ordering::equal;
        }
    };
} // namespace jh::pod

static_assert(jh::pod::pod_like<jh::pod::string_view>);

/**
 * @brief Official literal helpers for <code>jh::pod</code> types.
 *
 * This namespace contains the officially provided literal utilities
 * associated with <code>jh::pod</code>, offering concise and POD-safe
 * construction of view types such as <code>jh::pod::string_view</code>.
 * <br>
 * These literals are part of the public interface and are intended to be
 * used via:
 * @code
 * using namespace jh::pod::literals;
 * @endcode
 */
namespace jh::pod::literals {

    /**
     * @brief User-defined literal for <code>jh::pod::string_view</code>.
     *
     * Converts a string literal to a lightweight, POD-safe string_view.
     *
     * Example:
     * @code
     * using namespace jh::pod::literals;
     * constexpr auto s = "hello"_psv;
     * static_assert(s.size() == 5);
     * @endcode
     *
     * This is the only fully standard, portable, and safe form.
     * The literal's storage is static by definition,
     * so the resulting view never dangles.
     */
    [[nodiscard]] constexpr jh::pod::string_view
    operator ""_psv(const char *str, std::size_t len) noexcept {
        return {str, static_cast<std::uint64_t>(len)};
    }

} // namespace jh::pod::literals
