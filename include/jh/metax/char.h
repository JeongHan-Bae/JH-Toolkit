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
 * @file char.h (metax)
 * @brief Character-semantics concept and utilities &mdash; constexpr-safe character classification
 *        and transformation for 1-byte fundamental types.
 * @author
 *   JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 *
 * <p>
 * This header defines <code>jh::meta::any_char</code>, a C++20 concept identifying
 * the core 1-byte character types (<code>char</code>, <code>signed char</code>,
 * <code>unsigned char</code>), together with a small constexpr-safe classification
 * and transformation toolkit for character operations.
 * </p>
 *
 * <h3>Overview</h3>
 * <p>
 * The <code>any_char</code> concept constrains type parameters to only
 * those built-in 1-byte integral character types directly usable for
 * raw text or binary data. This design ensures type purity, constexpr
 * safety, and clear separation from UTF-8 and byte-level abstractions.
 * </p>
 *
 * <h3>Included utilities</h3>
 * <ul>
 *   <li><code>is_alpha()</code> &mdash; check for alphabetic characters (<tt>A–Z</tt>, <tt>a–z</tt>).</li>
 *   <li><code>is_digit()</code> &mdash; check for decimal digits (<tt>0–9</tt>).</li>
 *   <li><code>is_alnum()</code> &mdash; check for alphanumeric (letter or digit).</li>
 *   <li><code>is_hex_char()</code> &mdash; check for valid hexadecimal characters (<tt>0–9</tt>, <tt>A–F</tt>, <tt>a–f</tt>).</li>
 *   <li><code>is_base64_core()</code> &mdash; check if part of the standard Base64 alphabet (<tt>A–Z</tt>, <tt>a–z</tt>, <tt>0–9</tt>, '+', '/').</li>
 *   <li><code>is_base64url_core()</code> &mdash; check if part of the Base64URL alphabet (<tt>A–Z</tt>, <tt>a–z</tt>, <tt>0–9</tt>, '-', '_').</li>
 *   <li><code>is_ascii()</code> &mdash; verify whether a character is within the 7-bit ASCII range (<tt>0–127</tt>).</li>
 *   <li><code>is_printable_ascii()</code> &mdash; check if character is a printable 7-bit ASCII symbol (<tt>32–126</tt> inclusive).</li>
 *   <li><code>is_valid_char()</code> &mdash; check that character is not a control or DEL (returns true for all non-control bytes).</li>
 *   <li><code>to_upper()</code> &mdash; convert a letter to uppercase; leave others unchanged.</li>
 *   <li><code>to_lower()</code> &mdash; convert a letter to lowercase; leave others unchanged.</li>
 *   <li><code>flip_case()</code> &mdash; invert alphabetic case.</li>
 * </ul>
 *
 * <h3>Design rationale</h3>
 * <ul>
 *   <li><b>Concept-level filtering:</b> The <code>any_char</code> constraint rejects
 *       higher-level or incompatible types such as <code>char8_t</code>, <code>std::byte</code>,
 *       and <code>bool</code>.</li>
 *   <li><b>No <code>remove_cvref</code> usage:</b>
 *       The concept explicitly expects the raw type form &mdash; it models the
 *       <em>prototype</em> of character semantics. cv-qualifiers (e.g. <code>const Char</code>)
 *       can be manually specified if needed. References are intentionally
 *       excluded because <code>Char</code> is a metaprogramming token rather than
 *       a forwarding type; templates like <code>template&lt;any_char Char&gt;</code>
 *       instantiate with copies, not references, preserving constexpr safety.</li>
 *   <li><b>Strict 1-byte validation:</b> Guarantees <code>sizeof(T) == 1</code> and
 *       avoids UB across translation units when hashing or reinterpreting raw data.</li>
 * </ul>
 *
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include <concepts>
#include <type_traits>


namespace jh::meta {

    /**
     * @brief Concept representing <tt>character-semantic</tt> 1-byte integral types.
     *
     * This concept includes only the fundamental character types that the
     * implementation treats as one-byte integral entities directly usable as
     * textual or byte-sequence data.
     *
     * Specifically, it accepts:
     * <ul>
     *   <li><code>char</code></li>
     *   <li><code>signed char</code></li>
     *   <li><code>unsigned char</code></li>
     * </ul>
     *
     * These are the <tt>clean</tt> core character types without cv-qualifiers or references,
     * and are guaranteed to be exactly 1 byte in size (<code>sizeof(T) == 1</code>).
     *
     * @note
     * <code>char8_t</code> is <b>not</b> included.
     * It is a distinct built-in type introduced by <code>__cpp_char8_t</code> for UTF-8
     * character support, not considered equivalent to any form of <code>char</code>.
     * It represents <tt>UTF-8 code units</tt>, not raw bytes, and therefore requires
     * explicit conversion when hashing.
     *
     * @details
     * This constraint ensures that only raw character data (in the sense of
     * 1-byte memory representation types) participates in compile-time hashing.
     * Non-character or higher-level types such as:
     * <ul>
     *   <li><code>bool</code></li>
     *   <li><code>std::byte</code></li>
     *   <li><code>char8_t</code></li>
     * </ul>
     * must be explicitly converted via
     * <code>reinterpret_cast&lt;const char&#42;&gt;</code>.
     *
     * The intent is to enforce semantic correctness and guarantee that hashing
     * remains <tt>constexpr</tt>-safe, type-clean, and free of undefined behavior
     * across translation units.
     */
    template<typename T>
    concept any_char =
    std::same_as<T, char> ||
    std::same_as<T, signed char> ||
    std::same_as<T, unsigned char>;


    /// @brief Check if character is an alphabetic letter (A–Z, a–z).
    template<any_char Char>
    [[nodiscard]] constexpr bool is_alpha(Char c) noexcept {
        return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
    }

    /// @brief Check if character is a decimal digit (0–9).
    template<any_char Char>
    [[nodiscard]] constexpr bool is_digit(Char c) noexcept {
        return (c >= '0' && c <= '9');
    }

    /// @brief Check if character is alphanumeric (letter or digit).
    template<any_char Char>
    [[nodiscard]] constexpr bool is_alnum(Char c) noexcept {
        return is_alpha(c) || is_digit(c);
    }

    /// @brief Check if character is a valid hexadecimal digit.
    template<any_char Char>
    [[nodiscard]] constexpr bool is_hex_char(Char c) noexcept {
        return is_digit(c) ||
               (c >= 'a' && c <= 'f') ||
               (c >= 'A' && c <= 'F');
    }

    /// @brief Check if character belongs to standard Base64 alphabet.
    template<any_char Char>
    [[nodiscard]] constexpr bool is_base64_core(Char c) noexcept {
        return (c >= 'A' && c <= 'Z') ||
               (c >= 'a' && c <= 'z') ||
               (c >= '0' && c <= '9') ||
               c == '+' || c == '/';
    }

    /// @brief Check if character belongs to Base64URL alphabet.
    template<any_char Char>
    [[nodiscard]] constexpr bool is_base64url_core(Char c) noexcept {
        return (c >= 'A' && c <= 'Z') ||
               (c >= 'a' && c <= 'z') ||
               (c >= '0' && c <= '9') ||
               c == '-' || c == '_';
    }

    /// @brief Check if character is ASCII.
    template<any_char Char>
    [[nodiscard]] static constexpr bool is_ascii(Char c) noexcept {
        auto uc = static_cast<unsigned char>(c);
        return uc < 128;
    }

    /// @brief Check if character is printable 7-bit ASCII (range 32-126).
    template<any_char Char>
    [[nodiscard]] constexpr bool is_printable_ascii(Char c) noexcept {
        if (c < 32 || c > 126) return false;
        return true;
    }

    /// @brief Validate ASCII: reject control chars and DEL, leave non-ASCII untouched.
    template<any_char Char>
    [[nodiscard]] static constexpr bool is_valid_char(Char c) noexcept {
        auto uc = static_cast<unsigned char>(c);
        return !(uc < 32 || uc == 127);
    }

    /// @brief Convert letter to uppercase; leave others unchanged.
    template<any_char Char>
    [[nodiscard]] constexpr char to_upper(Char c) noexcept {
        return is_alpha(c) ? static_cast<Char>(c & ~0x20) : c;
    }

    /// @brief Convert letter to lowercase; leave others unchanged.
    template<any_char Char>
    [[nodiscard]] constexpr char to_lower(Char c) noexcept {
        return is_alpha(c) ? static_cast<Char>(c | 0x20) : c;
    }

    /// @brief Flip case of alphabetic character.
    template<any_char Char>
    [[nodiscard]] constexpr char flip_case(Char c) noexcept {
        return is_alpha(c) ? static_cast<Char>(c ^ 0x20) : c;
    }
} // namespace jh::meta
