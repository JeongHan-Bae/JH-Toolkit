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
 * @file t_str.h (metax)
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 * @brief A C++20 compile-time string wrapper enabling string literals as non-type template parameters (NTTP).
 *
 * @details
 * <code>t_str&lt;N&gt;</code> is a <strong>compile-time string container</strong> that enables
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
 * With <code>t_str&lt;N&gt;</code> and the <code>TStr</code> alias, this indirection is no longer needed:
 * string literals can be passed directly as template parameters and validated at compile time.
 *
 * <h3>Key Advantages</h3>
 * <ul>
 *   <li><strong>Direct NTTP binding</strong>: use <code>t_str</code> to inject string literals directly into templates.</li>
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
#include "jh/pods/array.h"
#include "jh/pods/string_view.h"
#include "jh/metax/hash.h"
#include "jh/detail/base64_common.h"

namespace jh::meta {
    namespace detail {
        template<std::uint16_t N>
        concept t_str_size_legal = (N <= jh::pod::max_pod_array_bytes);

        template<std::uint16_t N, std::uint16_t M>
        concept t_str_concat_legal = ((N - 1) + (M - 1) + 1 <= jh::pod::max_pod_array_bytes);
    } // namespace detail

    /**
     * @brief Compile-time string wrapper for use as a non-type template parameter (NTTP).
     *
     * @tparam N The size of the string literal including null terminator.
     *
     * @details
     * <code>t_str&lt;N&gt;</code> enables string literals to be bound directly as
     * <strong>non-type template parameters (NTTP)</strong> in C++20.
     * It provides constexpr construction, validation, transformation,
     * concatenation, and hashing of string literals with <b>zero runtime overhead</b>.
     */
    template<std::uint16_t N> requires detail::t_str_size_legal<N>
    struct t_str final {

        /**
         * @brief Friendship is required for implementing <code>operator+</code>.
         *
         * @tparam M Size of the other string literal.
         * @details
         * Different <code>t_str&lt;M&gt;</code> instances must access
         * each other's internal storage to perform constexpr concatenation.
         */
        template<std::uint16_t M> requires detail::t_str_size_legal<M>
        friend
        struct t_str;

        /// @brief Fixed-size storage for the compile-time string (null-terminated).
        const jh::pod::array<char, N> storage;

        using c_hash = jh::meta::c_hash;

        /// @brief build from underlying buffer
        constexpr explicit t_str(const jh::pod::array<char, N> &arr) noexcept
                : storage(arr) {}
    private:

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
        constexpr t_str(const char(&lit)[N]) noexcept: storage(make_array(lit)) {} // NOLINT

        /**
         * @brief Construct from a <code>char8_t</code>-based string literal (<code>u8""</code>).
         *
         * @details
         * This constructor provides compatibility for platforms or codebases
         * where <code>u8""</code> string literals yield <code>const char8_t[]</code>.
         * Each element is converted to <code>char</code> for uniform storage.
         *
         * This is a non-standard compatibility feature &mdash; since in most modern
         * platforms, <code>""</code> literals are already UTF-8 encoded.
         * The intent is only to allow <code>u8""</code> literals to be used
         * seamlessly as NTTP, consistent with regular string literals.
         */
        constexpr t_str(const char8_t(&lit)[N]) noexcept: storage(make_array(lit)) {} // NOLINT

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
         *          <li><code>c_hash::murmur64</code> - constexpr-safe MurmurHash variant (seedless)</li>
         *          <li><code>c_hash::xxhash64</code> - constexpr xxHash64 variant (seedless)</li>
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
         * @details Expands two <code>t_str</code> storages into a new one
         * using index sequences. Called by <code>operator+</code>.
         */
        template<std::uint16_t M, std::size_t... I, std::size_t... J>
        requires detail::t_str_concat_legal<N, M>
        [[nodiscard]] constexpr t_str<(N - 1) + (M - 1) + 1>
        concat_impl(const t_str<M> &other,
                    std::index_sequence<I...>, std::index_sequence<J...>) const noexcept {
            constexpr std::uint16_t NewSize = (N - 1) + (M - 1) + 1;
            jh::pod::array<char, NewSize> arr{{storage[I]..., other.storage[J]...}};
            return t_str<NewSize>(arr);
        }

    public:
        /**
         * @brief Concatenate two <code>t_str</code> strings at compile time.
         *
         * @tparam M Size (including null terminator) of the right-hand operand.
         * @param other Another <code>t_str&lt;M&gt;</code> to append.
         * @return A new <code>t_str</code> whose size is <code>(N - 1) + (M - 1) + 1</code>,
         *         containing the concatenated characters and a null terminator.
         *
         * @details
         * <ul>
         *   <li>Performs constexpr-safe concatenation without dynamic allocation.</li>
         *   <li>The total size must satisfy <code>t_str_concat_legal</code> (≤ 16 KB).</li>
         *   <li>The null terminator of the left string is ignored during concatenation,
         *       and a new null terminator is appended at the end.</li>
         * </ul>
         */
        template<std::uint16_t M>
        requires detail::t_str_concat_legal<N, M>
        [[nodiscard]] constexpr auto operator+(const t_str<M> &other) const noexcept {
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
                if (!jh::meta::is_digit(storage[i])) return false;
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
            if (storage[i] == '+' || storage[i] == '-') {
                ++i;
            }

            bool has_digit = false;
            bool seen_dot = false;
            bool seen_exp = false;

            for (; i < n; ++i) {
                const char c = storage[i];

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
                if (!jh::meta::is_alpha(storage[i])) return false;
            return true;
        }

        /**
         * @brief Check if all characters are alphanumeric (letters or digits).
         * @return true if all characters are alphanumeric, false otherwise.
         */
        [[nodiscard]] constexpr bool is_alnum() const noexcept {
            for (std::uint64_t i = 0; i < size(); i++)
                if (!jh::meta::is_alnum(storage[i])) return false;
            return true;
        }

        /**
         * @brief Check if all characters are 7-bit ASCII.
         * @return true if all characters are in range 0–127, false otherwise.
         */
        [[nodiscard]] constexpr bool is_ascii() const noexcept {
            for (std::uint64_t i = 0; i < size(); i++)
                if (!jh::meta::is_ascii(storage[i])) return false;
            return true;
        }

        /**
         * @brief Check if all characters are printable 7-bit ASCII.
         * @return true if all characters are in range 32-126, false otherwise.
         */
        [[nodiscard]] constexpr bool is_printable_ascii() const noexcept {
            for (std::uint64_t i = 0; i < size(); i++)
                if (!jh::meta::is_printable_ascii(storage[i])) return false;
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
            if (size() % 2 != 0) return false;
            for (std::uint64_t i = 0; i < size(); i++)
                if (!jh::meta::is_hex_char(storage[i])) return false;
            return true;
        }

        /**
         * @brief Check if the string is valid Base64.
         * @details Length must be a multiple of 4, padding ('=') allowed at the end.
         * @return true if valid Base64, false otherwise.
         */
        [[nodiscard]] constexpr bool is_base64() const noexcept {
            return jh::detail::base64_common::is_base64(val(), size());
        }

        /**
         * @brief Check if the string is valid Base64URL.
         * @details '=' padding is optional. If present, length must be a multiple of 4.
         * @return true if valid Base64URL, false otherwise.
         */
        [[nodiscard]] constexpr bool is_base64url() const noexcept {
            return jh::detail::base64_common::is_base64url(val(), size());
        }

    private:
        /**
         * @brief Internal helper for character-wise transformation.
         *
         * @tparam F Transformation function (e.g. <code>to_upper</code>).
         * @tparam I Index sequence for unrolling characters at compile time.
         * @return A new <code>t_str</code> with all characters transformed by <code>F</code>.
         *
         * @note Used internally by <code>to_upper()</code>, <code>to_lower()</code>, and <code>flip_case()</code>.
         */
        template<char (*F)(char), std::size_t ... I>
        [[nodiscard]] constexpr auto transform_impl(std::index_sequence<I...>) const noexcept {
            constexpr std::uint16_t NewSize = N;
            const char arr[NewSize] = {F(storage[I])...};
            return t_str<NewSize>(arr);
        }

    public:
        /**
         * @brief Convert all alphabetic characters to uppercase (A–Z).
         * @return A new <code>t_str</code> with characters transformed to uppercase.
         */
        [[nodiscard]] constexpr auto to_upper() const noexcept {
            return transform_impl<jh::meta::to_upper>(std::make_index_sequence<N>{});
        }

        /**
         * @brief Convert all alphabetic characters to lowercase (a–z).
         * @return A new <code>t_str</code> with characters transformed to lowercase.
         */
        [[nodiscard]] constexpr auto to_lower() const noexcept {
            return transform_impl<jh::meta::to_lower>(std::make_index_sequence<N>{});
        }

        /**
         * @brief Toggle the case of all alphabetic characters.
         * @return A new <code>t_str</code> with each character's case flipped.
         */
        [[nodiscard]] constexpr auto flip_case() const noexcept {
            return transform_impl<jh::meta::flip_case>(std::make_index_sequence<N>{});
        }

        /**
         * @brief Equality comparison with another <code>t_str</code> of different size.
         *
         * @tparam M Size of the other <code>t_str</code>.
         * @param Unused The other string (ignored, since size mismatch short-circuits).
         * @return <code>false</code> always, because string sizes differ.
         *
         * @note This overload exists to provide a compile-time fast-path:
         *       if <code>N != M</code>, the comparison does not even check characters.
         */
        template<std::uint16_t M>
        constexpr bool operator==(const t_str<M> &) const noexcept
        requires (M != N) { return false; }

        /**
         * @brief Equality comparison with another <code>t_str</code> of the same size.
         *
         * @param other The other <code>t_str&lt;N&gt;</code>.
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
        constexpr bool operator==(const t_str &) const noexcept = default;

        /**
         * @brief Convert the string (excluding the null terminator) to a byte array.
         *
         * @return A <code>jh::pod::array&lt;std::uint8_t, N - 1&gt;</code> containing
         *         the string's raw character bytes, excluding the null terminator.
         *
         * @details
         * <ul>
         *   <li>The returned array contains exactly <code>N - 1</code> bytes &mdash;
         *       the effective string length.</li>
         *   <li>The array does <em>not</em> contain a null terminator, because it
         *       represents a binary buffer, not a C-string.</li>
         *   <li>This makes the result suitable for binary serialization,
         *       hashing, Base64 encoding, and other byte-wise operations.</li>
         *   <li>At compile time, values are assigned element-by-element.</li>
         *   <li>At runtime, <code>std::memcpy</code> is used for maximum efficiency.</li>
         * </ul>
         */
        [[nodiscard]] constexpr explicit operator jh::pod::array<std::uint8_t, N - 1>() const noexcept {
            jh::pod::array<std::uint8_t, N - 1> bytes{};
            if (std::is_constant_evaluated()) {
                for (std::uint64_t i = 0; i < N - 1; ++i)
                    bytes.data[i] = static_cast<std::uint8_t>(storage[i]);
            } else {
                std::memcpy(bytes.data, storage.data, N - 1);
            }
            return bytes;
        }
        
        /**
         * @brief Convert to an immutable byte buffer.
         *
         * @return A <code>jh::pod::array&lt;std::uint8_t, N - 1&gt;</code> containing
         *         the raw characters of the string (not null-terminated).
         *
         * @details
         * This is equivalent to the explicit byte-array conversion operator.
         */
        [[nodiscard]] constexpr jh::pod::array<std::uint8_t, N - 1> to_bytes() const noexcept {
            return jh::pod::array<std::uint8_t, N - 1>(*this);
        }

        /**
         * @brief Construct a <code>t_str&lt;N&gt;</code> from a byte buffer.
         *
         * @param bytes
         *     A <code>jh::pod::array&lt;std::uint8_t, N - 1&gt;</code> representing
         *     a binary buffer.  
         *     The buffer does <b>not</b> contain a null terminator.
         *
         * @return A new <code>t_str&lt;N&gt;</code> whose characters are taken directly
         *         from <code>bytes</code>, with a null terminator appended internally.
         *
         * @details
         * <ul>
         *   <li>This function treats <code>bytes</code> as pure binary data.</li>
         *   <li>No validation is performed &mdash; any byte value (0–255) is accepted.</li>
         *   <li>The resulting <code>t_str</code> is always null-terminated internally,
         *       because <code>t_str</code> is semantically a C-string wrapper.</li>
         *   <li>
         *     This method enables a useful pattern:
         *     writing a binary buffer as if it were a string literal,
         *     then reconstructing <code>t_str</code> from it.
         *   </li>
         *   <li>
         *     The null terminator added at the end is not part of the returned
         *     <em>bytes</em> if converted back using <code>to_bytes()</code>.
         *   </li>
         * </ul>
         */
        [[nodiscard]] static constexpr t_str from_bytes(const jh::pod::array<std::uint8_t, N - 1>& bytes) noexcept {
            jh::pod::array<char, N> arr{};
            if (std::is_constant_evaluated()) {
                for (std::uint64_t i = 0; i < N - 1; ++i)
                    arr.data[i] = static_cast<char>(bytes.data[i]);
            } else {
                std::memcpy(arr.data, bytes.data, N - 1);
            }
            return t_str(arr);
        }
    };

    /**
     * @brief Alias for <code>t_str&lt;N&gt;</code> with template argument deduction.
     *
     * @tparam N Size of the string literal including the null terminator.
     *
     * @details
     * <ul>
     *   <li>This alias allows <code>t_str</code> to be used directly
     *       as a <b>non-type template parameter</b> in C++20.</li>
     *   <li>Simplifies template declarations by avoiding the explicit
     *       <code>t_str&lt;N&gt;</code> spelling.</li>
     *   <li>Intended primarily for compile-time string literal binding in templates.</li>
     * </ul>
     */
    template<std::uint16_t N>
    using TStr [[maybe_unused]] = t_str<N>;

    /**
     * @brief Stream output operator for <code>t_str&lt;N&gt;</code>.
     *
     * @tparam N Size of the compile-time string (including null terminator).
     * @param os The output stream.
     * @param str The <code>t_str&lt;N&gt;</code> instance to print.
     * @return Reference to the output stream.
     *
     * @details
     * Writes the string's <code>std::string_view</code> to the given output stream.
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
    inline std::ostream &operator<<(std::ostream &os, const t_str<N> &str) {
        os << str.view();
        return os;
    }
} // namespace jh::meta
