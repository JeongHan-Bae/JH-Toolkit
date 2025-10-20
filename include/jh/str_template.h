/**
 * \verbatim
 * Copyright 2025 JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
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
 * \endverbatim
 */
/**
 * @file str_template.h
 * @author JeongHan-Bae <pre>&lt;mastropseudo&#64;gmail.com&gt;</pre>
 * @brief A C++20 compile-time string wrapper enabling string literals as non-type template parameters (NTTP).
 *
 * @details
 * <code>cstr&lt;N&gt;</code> is a <strong>compile-time string container</strong> that enables
 * <strong>C++20 non-type template parameters (NTTP)</strong> with string literals.
 *
 * <h3>Motivation</h3>
 * Before C++20, it was not possible to bind string literals directly as template arguments.
 * If a type needed an associated string-based metadata, developers had to:
 * <ul>
 *   <li>Define a base class with a virtual/static metadata function,</li>
 *   <li>Override the metadata in every derived class,</li>
 *   <li>Access it indirectly during template instantiation.</li>
 * </ul>
 *
 * With <code>cstr&lt;N&gt;</code> and the <code>CStr</code> alias, this indirection is no longer needed:
 * string literals can be passed directly as template parameters and validated at compile time.
 *
 * <h3>Key Advantages</h3>
 * <ul>
 *   <li><strong>Direct NTTP binding</strong>: use <code>CStr</code> to inject string literals directly into templates.</li>
 *   <li><strong>Compile-time validation</strong>: check properties such as digit, alnum, hex, base64, and ASCII legality.</li>
 *   <li><strong>Compile-time transformation</strong>: supports <code>to_upper()</code>, <code>to_lower()</code>, and <code>flip_case()</code>.</li>
 *   <li><strong>Concatenation with limits</strong>: safe constexpr concatenation with a maximum total size (default 16 KB).</li>
 *   <li><strong>Zero runtime overhead</strong>: no dynamic allocation, everything is constexpr.</li>
 * </ul>
 *
 * <h3>Design Notes</h3>
 * <ul>
 *   <li>The maximum supported string size is <b>16 KB</b>.</li>
 *   <li>All strings are null-terminated (<code>N - 1</code> is the effective length).</li>
 *   <li>Intended exclusively for <b>string literals</b> in template contexts.</li>
 *   <li><b>Not</b> a runtime replacement for <code>std::string</code> or <code>immutable_str</code>.</li>
 * </ul>
 *
 * @version <pre>1.4.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include <utility>
#include <string_view>
#include <cstdint>
#include "pods/array.h"
#include "pods/string_view.h"

namespace jh::str_template {
    /**
     * @namespace jh::str_template::detail
     * @brief Internal constexpr utilities for <code>cstr&lt;N&gt;</code>.
     *
     * @details
     * Provides compile-time character classification and transformations:
     * <ul>
     *   <li>Checks: <code>is_alpha</code>, <code>is_digit</code>, <code>is_alnum</code>, <code>is_hex_char</code>, <code>is_base64_core</code>, <code>is_base64url_core</code>.</li>
     *   <li>Transforms: <code>to_upper</code>, <code>to_lower</code>, <code>flip_case</code>.</li>
     *   <li>Validation: <code>is_valid_char</code> excludes ASCII control characters except printable.</li>
     *   <li>Constraints: <code>cstr_size_legal</code> and <code>cstr_concat_legal</code> enforce maximum size (16 KB).</li>
     * </ul>
     */
    namespace detail {
        [[nodiscard]] constexpr bool is_alpha(char c) noexcept {
            return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
        } ///< Check if character is an alphabetic letter (A–Z, a–z).

        [[nodiscard]] constexpr bool is_digit(char c) noexcept {
            return (c >= '0' && c <= '9');
        } ///< Check if character is a decimal digit (0–9).

        [[nodiscard]] constexpr bool is_alnum(char c) noexcept {
            return is_alpha(c) || is_digit(c);
        } ///< Check if character is alphanumeric (letter or digit).

        [[nodiscard]] constexpr bool is_hex_char(char c) noexcept {
            return is_digit(c) ||
                   (c >= 'a' && c <= 'f') ||
                   (c >= 'A' && c <= 'F');
        } ///< Check if character is a valid hexadecimal digit.

        [[nodiscard]] constexpr bool is_base64_core(char c) noexcept {
            return (c >= 'A' && c <= 'Z') ||
                   (c >= 'a' && c <= 'z') ||
                   (c >= '0' && c <= '9') ||
                   c == '+' || c == '/';
        } ///< Check if character belongs to standard Base64 alphabet.

        [[nodiscard]] constexpr bool is_base64url_core(char c) noexcept {
            return (c >= 'A' && c <= 'Z') ||
                   (c >= 'a' && c <= 'z') ||
                   (c >= '0' && c <= '9') ||
                   c == '-' || c == '_';
        } ///< Check if character belongs to Base64URL alphabet.

        [[nodiscard]] constexpr char to_upper(char c) noexcept {
            return is_alpha(c) ? static_cast<char>(c & ~0x20) : c;
        } ///< Convert letter to uppercase; leave others unchanged.

        [[nodiscard]] constexpr char to_lower(char c) noexcept {
            return is_alpha(c) ? static_cast<char>(c | 0x20) : c;
        } ///< Convert letter to lowercase; leave others unchanged.

        [[nodiscard]] constexpr char flip_case(char c) noexcept {
            return is_alpha(c) ? static_cast<char>(c ^ 0x20) : c;
        } ///< Flip case of alphabetic character.

        [[nodiscard]] static constexpr bool is_valid_char(char c) noexcept {
            auto uc = static_cast<unsigned char>(c);
            return !(uc < 32 || uc == 127);
        } ///< Validate ASCII: reject control chars and DEL, leave non-ASCII untouched.

        template<std::uint16_t N>
        concept cstr_size_legal = (N <= 16384);

        template<std::uint16_t N, std::uint16_t M>
        concept cstr_concat_legal = ((N - 1) + (M - 1) + 1 <= 16384);
    } // namespace detail

    /**
     * @brief Compile-time string wrapper for use as a non-type template parameter (NTTP).
     *
     * @tparam N The size of the string literal including null terminator.
     *
     * @details
     * <code>cstr&lt;N&gt;</code> enables string literals to be bound directly as
     * <strong>non-type template parameters (NTTP)</strong> in C++20.
     * It provides constexpr construction, validation, transformation,
     * concatenation, and hashing of string literals with <b>zero runtime overhead</b>.
     */
    template<std::uint16_t N> requires detail::cstr_size_legal<N>
    struct cstr {

        /**
         * @brief Friendship is required for implementing <code>operator+</code>.
         *
         * @tparam M Size of the other string literal.
         * @details
         * Different <code>cstr&lt;M&gt;</code> instances must access
         * each other's internal storage to perform constexpr concatenation.
         */
        template<std::uint16_t M> requires detail::cstr_size_legal<M>
        friend
        struct cstr;

        const jh::pod::array<char, N> storage; ///< Fixed-size storage for the compile-time string (null-terminated).

        /**
         * @brief Compile-time hash algorithm selector.
         *
         * @details
         * Alias for <code>jh::utils::hash_fn::c_hash</code>, which provides
         * constexpr-safe hashing usable in templates and static contexts.
         *
         * Supported algorithms:
         * <ul>
         *   <li><code>c_hash::fnv1a64</code> — FNV-1a 64-bit (default)</li>
         *   <li><code>c_hash::fnv1_64</code> — FNV-1 64-bit (multiply before xor)</li>
         *   <li><code>c_hash::djb2</code> — DJB2 (classic string hash)</li>
         *   <li><code>c_hash::sdbm</code> — SDBM (used in DBM/readdir)</li>
         * </ul>
         *
         * Used by <code>cstr::hash()</code> for computing static hash values
         * of compile-time strings.
         */
        using c_hash = jh::utils::hash_fn::c_hash;

    private:

        constexpr explicit cstr(const jh::pod::array<char, N> &arr) noexcept
                : storage(arr) {} ///< Private constructor from prebuilt array, used internally.

        static constexpr jh::pod::array<char, N> make_array(const char(&src)[N]) {
            jh::pod::array<char, N> arr{};
            for (std::uint64_t i = 0; i < N; i++) {
                arr.data[i] = src[i];
            }
            return arr;
        } ///< Build storage from a narrow char literal.

        static constexpr jh::pod::array<char, N> make_array(const char8_t(&src)[N]) {
            jh::pod::array<char, N> arr{};
            for (std::uint64_t i = 0; i < N; i++) {
                arr.data[i] = static_cast<char>(src[i]);
            }
            return arr;
        } ///< Build storage from a UTF-8 char8_t literal.

    public:
        /**
         * @brief Construct from a regular string literal.
         *
         * @details
         * This constructor is intentionally implicit.
         * It enables string literals to be passed directly
         * as non-type template parameters (NTTP) without requiring
         * additional wrappers.
         */
        constexpr cstr(const char(&lit)[N]) noexcept: storage(make_array(lit)) {} // NOLINT

        /**
         * @brief Construct from a <code>char8_t</code>-based string literal (<code>u8""</code>).
         *
         * @details
         * This constructor provides compatibility for platforms or codebases
         * where <code>u8""</code> string literals yield <code>const char8_t[]</code>.
         * Each element is converted to <code>char</code> for uniform storage.
         *
         * This is a non-standard compatibility feature — since in most modern
         * platforms, <code>""</code> literals are already UTF-8 encoded.
         * The intent is only to allow <code>u8""</code> literals to be used
         * seamlessly as NTTP, consistent with regular string literals.
         */
        constexpr cstr(const char8_t(&lit)[N]) noexcept: storage(make_array(lit)) {} // NOLINT

        /**
         * @brief Get a pointer to the stored string.
         * @return Pointer to the first character of the internal string (null-terminated).
         *
         * @note The returned pointer comes from <code>storage.data</code>, which is
         * a <code>const char[N]</code>. It decays to <code>const char*</code> on return.
         */
        [[nodiscard]] constexpr const char *val() const noexcept { return storage.data; }

        /**
         * @brief Get the length of the string (excluding null terminator).
         * @return Number of characters before the null terminator.
         */
        [[nodiscard]] constexpr std::uint64_t size() const noexcept { return static_cast<std::uint64_t>(N - 1); }

        /**
         * @brief Get a <code>std::string_view</code> over the stored string.
         * @return A <code>string_view</code> referencing the characters (excluding null terminator).
         */
        [[nodiscard]] constexpr std::string_view view() const noexcept {
            return {storage.data, size()};
        }

        /**
         * @brief Compute a constexpr hash of the stored string.
         *
         * @param hash_method The hash algorithm to use (default: <code>c_hash::fnv1a64</code>).
         *        Supported algorithms:
         *        <ul>
         *          <li><code>c_hash::fnv1a64</code> – FNV-1a 64-bit hash (default, fast and well-distributed).</li>
         *          <li><code>c_hash::fnv1_64</code> – FNV-1 64-bit hash (multiply before xor).</li>
         *          <li><code>c_hash::djb2</code> – DJB2 hash (classic string hash).</li>
         *          <li><code>c_hash::sdbm</code> – SDBM hash (used in readdir, DBM).</li>
         *        </ul>
         * @param include_null If true, the null terminator is included in the hash computation.
         * @return 64-bit non-cryptographic hash value.
         *
         * @note
         * All supported algorithms are constexpr-safe and suitable for compile-time use
         * (e.g., as template arguments or static IDs).
         * They are <b>not cryptographically secure</b>.
         */
        [[nodiscard]] constexpr std::uint64_t
        hash(c_hash hash_method = c_hash::fnv1a64, bool include_null = false) const noexcept {
            return jh::pod::string_view{
                    val(), size() + (include_null ? 1 : 0)
            }.hash(hash_method);
        }

    private:
        /**
         * @brief Internal helper for string concatenation.
         * @details Expands two <code>cstr</code> storages into a new one
         * using index sequences. Called by <code>operator+</code>.
         */
        template<std::uint16_t M, std::size_t... I, std::size_t... J>
        requires detail::cstr_concat_legal<N, M>
        [[nodiscard]] constexpr cstr<(N - 1) + (M - 1) + 1>
        concat_impl(const cstr<M> &other,
                    std::index_sequence<I...>, std::index_sequence<J...>) const noexcept {
            constexpr std::uint16_t NewSize = (N - 1) + (M - 1) + 1;
            jh::pod::array<char, NewSize> arr{{storage[I]..., other.storage[J]...}};
            return cstr<NewSize>(arr);
        }

    public:
        /**
         * @brief Concatenate two <code>cstr</code> strings at compile time.
         *
         * @tparam M Size (including null terminator) of the right-hand operand.
         * @param other Another <code>cstr&lt;M&gt;</code> to append.
         * @return A new <code>cstr</code> whose size is <code>(N - 1) + (M - 1) + 1</code>,
         *         containing the concatenated characters and a null terminator.
         *
         * @details
         * <ul>
         *   <li>Performs constexpr-safe concatenation without dynamic allocation.</li>
         *   <li>The total size must satisfy <code>cstr_concat_legal</code> (≤ 16 KB).</li>
         *   <li>The null terminator of the left string is ignored during concatenation,
         *       and a new null terminator is appended at the end.</li>
         * </ul>
         */
        template<std::uint16_t M>
        requires detail::cstr_concat_legal<N, M>
        [[nodiscard]] constexpr auto operator+(const cstr<M> &other) const noexcept {
            return concat_impl(other,
                               std::make_index_sequence<N - 1>{},
                               std::make_index_sequence<M>{});
        }

        /**
         * @brief Check if all characters are decimal digits (0–9).
         * @note This only checks that each character is a digit.
         *       To validate if the whole string represents a number
         *       (with optional sign, decimal point, or exponent),
         *       use @c is_number() instead.
         * @return true if all characters are digits, false otherwise.
         */
        [[nodiscard]] constexpr bool is_digit() const noexcept {
            for (std::uint64_t i = 0; i < size(); i++)
                if (!detail::is_digit(storage[i])) return false;
            return true;
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
         *   ^[+-]?[0-9]+(\\.[0-9]+)?([eE][+-]?[0-9]+)?$
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
            if (storage[i] == '+' || storage[i] == '-') {
                ++i;
            }

            bool has_digit = false;
            bool seen_dot = false;
            bool seen_exp = false;

            for (; i < n; ++i) {
                const char c = storage[i];

                /// do NOT apply [[likely]] as this is constexpr
                if (detail::is_digit(c)) {
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
                    if (i + 1 < n && (storage[i + 1] == '+' || storage[i + 1] == '-')) {
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
         * @brief Check if all characters are alphabetic (A–Z, a–z).
         * @return true if all characters are alphabetic, false otherwise.
         */
        [[nodiscard]] constexpr bool is_alpha() const noexcept {
            for (std::uint64_t i = 0; i < size(); i++)
                if (!detail::is_alpha(storage[i])) return false;
            return true;
        }

        /**
         * @brief Check if all characters are alphanumeric (letters or digits).
         * @return true if all characters are alphanumeric, false otherwise.
         */
        [[nodiscard]] constexpr bool is_alnum() const noexcept {
            for (std::uint64_t i = 0; i < size(); i++)
                if (!detail::is_alnum(storage[i])) return false;
            return true;
        }

        /**
         * @brief Check if all characters are 7-bit ASCII.
         * @return true if all characters are in range 0–127, false otherwise.
         */
        [[nodiscard]] constexpr bool is_ascii() const noexcept {
            for (std::uint64_t i = 0; i < size(); i++)
                if (static_cast<unsigned char>(storage[i]) > 127) return false;
            return true;
        }

        /**
         * @brief Check if all characters are printable 7-bit ASCII.
         * @return true if all characters are in range 32-126, false otherwise.
         */
        [[nodiscard]] constexpr bool is_printable_ascii() const noexcept {
            for (std::uint64_t i = 0; i < size(); i++) {
                auto c = static_cast<unsigned char>(storage[i]);
                if (c < 32 || c > 126) return false;
            }
            return true;
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
                auto c = static_cast<unsigned char>(storage[i]);
                // filter out disallowed ASCII control characters
                if (!detail::is_valid_char(static_cast<char>(c))) return false;
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
            if (size() % 2 != 0) return false;
            for (std::uint64_t i = 0; i < size(); i++)
                if (!detail::is_hex_char(storage[i])) return false;
            return true;
        }

        /**
         * @brief Check if the string is valid Base64.
         * @details Length must be a multiple of 4, padding ('=') allowed at the end.
         * @return true if valid Base64, false otherwise.
         */
        [[nodiscard]] constexpr bool is_base64() const noexcept {
            const std::uint64_t n = size();
            if (n == 0 || n % 4 != 0) return false;

            for (std::uint64_t i = 0; i < n - 2; i++)
                if (!detail::is_base64_core(storage[i])) return false;

            const char c3 = storage[n - 2];
            const char c4 = storage[n - 1];
            if (c3 == '=' && c4 == '=') return true;
            if (detail::is_base64_core(c3) && c4 == '=') return true;
            if (detail::is_base64_core(c3) && detail::is_base64_core(c4)) return true;
            return false;
        }

        /**
         * @brief Check if the string is valid Base64URL.
         * @details '=' padding is optional. If present, length must be a multiple of 4.
         * @return true if valid Base64URL, false otherwise.
         */
        [[nodiscard]] constexpr bool is_base64url() const noexcept {
            const std::uint64_t n = size();
            if (n == 0) return false;

            if (n % 4 == 0) {
                for (std::uint64_t i = 0; i < n - 2; i++)
                    if (!detail::is_base64url_core(storage[i])) return false;

                const char c3 = storage[n - 2];
                const char c4 = storage[n - 1];
                if (c3 == '=' && c4 == '=') return true;
                if (detail::is_base64url_core(c3) && c4 == '=') return true;
                if (detail::is_base64url_core(c3) && detail::is_base64url_core(c4)) return true;
                return false;
            }

            if (n % 4 == 1) return false;
            for (std::uint64_t i = 0; i < n; i++)
                if (!detail::is_base64url_core(storage[i])) return false;
            return true;
        }

    private:
        /**
         * @brief Internal helper for character-wise transformation.
         *
         * @tparam F Transformation function (e.g. <code>detail::to_upper</code>).
         * @tparam I Index sequence for unrolling characters at compile time.
         * @return A new <code>cstr</code> with all characters transformed by <code>F</code>.
         *
         * @note Used internally by <code>to_upper()</code>, <code>to_lower()</code>, and <code>flip_case()</code>.
         */
        template<auto F, std::size_t ... I>
        [[nodiscard]] constexpr auto transform_impl(std::index_sequence<I...>) const noexcept {
            constexpr std::uint16_t NewSize = N;
            const char arr[NewSize] = {F(storage[I])...};
            return cstr<NewSize>(arr);
        }

    public:
        /**
         * @brief Convert all alphabetic characters to uppercase (A–Z).
         * @return A new <code>cstr</code> with characters transformed to uppercase.
         */
        [[nodiscard]] constexpr auto to_upper() const noexcept {
            return transform_impl<detail::to_upper>(std::make_index_sequence<N>{});
        }

        /**
         * @brief Convert all alphabetic characters to lowercase (a–z).
         * @return A new <code>cstr</code> with characters transformed to lowercase.
         */
        [[nodiscard]] constexpr auto to_lower() const noexcept {
            return transform_impl<detail::to_lower>(std::make_index_sequence<N>{});
        }

        /**
         * @brief Toggle the case of all alphabetic characters.
         * @return A new <code>cstr</code> with each character’s case flipped.
         */
        [[nodiscard]] constexpr auto flip_case() const noexcept {
            return transform_impl<detail::flip_case>(std::make_index_sequence<N>{});
        }

        /**
         * @brief Equality comparison with another <code>cstr</code> of different size.
         *
         * @tparam M Size of the other <code>cstr</code>.
         * @param Unused The other string (ignored, since size mismatch short-circuits).
         * @return <code>false</code> always, because string sizes differ.
         *
         * @note This overload exists to provide a compile-time fast-path:
         *       if <code>N != M</code>, the comparison does not even check characters.
         */
        template<std::uint16_t M>
        constexpr bool operator==(const cstr<M> &) const noexcept
        requires (M != N) { return false; }

        /**
         * @brief Equality comparison with another <code>cstr</code> of the same size.
         *
         * @param other The other <code>cstr&lt;N&gt;</code>.
         * @return <code>true</code> if and only if all characters match (including the null terminator).
         *
         * @details
         * This operator is <code>= default</code>, meaning comparison is delegated
         * to the underlying member <code>const jh::pod::array&lt;char, N&gt; storage</code>.
         *
         *  <li>Semantically: it is equivalent to comparing all characters in the string one by one.</li>
         *  <li>Implementation-wise: since <code>storage</code> is a POD type,
         *   the compiler can optimize this into a direct <code>memcmp</code>-style comparison
         *   at compile time or runtime.</li>
         */
        constexpr bool operator==(const cstr &) const noexcept = default;

    };

    /**
     * @brief Alias for <code>cstr&lt;N&gt;</code> with template argument deduction.
     *
     * @tparam N Size of the string literal including the null terminator.
     *
     * @details
     * <ul>
     *   <li>This alias allows <code>CStr</code> to be used directly
     *       as a <b>non-type template parameter</b> in C++20.</li>
     *   <li>Simplifies template declarations by avoiding the explicit
     *       <code>cstr&lt;N&gt;</code> spelling.</li>
     *   <li>Intended primarily for compile-time string literal binding in templates.</li>
     * </ul>
     */
    template<std::uint16_t N>
    using CStr [[maybe_unused]] = cstr<N>;

    /**
     * @brief Stream output operator for <code>cstr&lt;N&gt;</code>.
     *
     * @tparam N Size of the compile-time string (including null terminator).
     * @param os The output stream.
     * @param str The <code>cstr&lt;N&gt;</code> instance to print.
     * @return Reference to the output stream.
     *
     * @details
     * Writes the string’s <code>std::string_view</code> to the given output stream.
     *  <ul>
     *    <li>This is the <b>default output representation</b>.</li>
     *    <li>It may be overridden by providing a higher-priority overload
     *        or explicitly bringing another overload into scope with <code>using</code>.</li>
     *    <li>The function is declared <code>inline</code>, which in C++17 and later
     *        does <b>not guarantee inlining</b>, but instead gives the function
     *        weak, foldable linkage semantics across translation units.</li>
     *  </ul>
     * @note The printed form is identical to the underlying string literal content
     *       (no quotes, escapes, or formatting applied).
     */
    template<std::uint16_t N>
    inline std::ostream &operator<<(std::ostream &os, const cstr<N> &str) {
        os << str.view();
        return os;
    }
} // namespace jh::str_template
