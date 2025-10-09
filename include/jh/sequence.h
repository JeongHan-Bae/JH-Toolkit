/**
 * \verbatim
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
 * \endverbatim
 */
/**
 * @file sequence.h
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 * @brief Concept and utilities for immutable sequence detection in C++20.
 *
 * @details
 * The <code>jh::sequence</code> module defines a lightweight C++20 concept
 * used to detect types that provide immutable iteration through
 * <code>begin()</code> and <code>end()</code>.
 * It generalizes both STL containers and custom sequence-like objects,
 * providing consistent compile-time recognition and value type extraction.
 *
 * <h3>Design Goals:</h3>
 * <ul>
 *   <li>Provide a <strong>uniform interface</strong> for detecting iterable containers.</li>
 *   <li>Support both <strong>STL-style</strong> and <strong>duck-typed</strong> sequences.</li>
 *   <li>Rely purely on <strong>behavioral matching</strong> (no typedef or inheritance required).</li>
 *   <li>Ensure <strong>const-safe and reference-safe</strong> deduction for generic algorithms.</li>
 *   <li>Integrate naturally with <code>jh::iterator_t</code> and iterator concepts.</li>
 * </ul>
 *
 * <h3>Key Components</h3>
 * <ul>
 *   <li><code>jh::sequence</code> — Concept ensuring immutable iteration.</li>
 *   <li><code>jh::sequence_value_type&lt;T&gt;</code> — Extracts element type via <code>iterator_t</code>.</li>
 *   <li><code>jh::is_sequence&lt;T&gt;</code> — Compile-time boolean for detection.</li>
 *   <li><code>jh::to_range()</code> — Converts any sequence into a standard <code>std::ranges::subrange</code>.</li>
 * </ul>
 *
 * <h3>Design Notes</h3>
 * <ul>
 *   <li>Type deduction uses <code>jh::iterator_t&lt;T&gt;</code>, providing
 *       a unified meta-interface for STL and custom containers.</li>
 *   <li>All qualifiers (<code>const</code>, <code>volatile</code>, references)
 *       are removed before deduction to ensure consistent behavior.</li>
 *   <li>Concept validation is purely structural — no explicit typedefs are required.</li>
 *   <li>Serves as a foundational layer for <code>jh::range</code> and coroutine containers.</li>
 * </ul>
 *
 * @version <pre>1.3.x</pre>
 * @date <pre>2025<pre>
 */

#pragma once

#include <iterator>
#include "jh/iterator.h"

namespace jh {
    /**
     * @brief Concept that checks whether a type behaves as an immutable sequence.
     *
     * @details
     * A type <code>T</code> satisfies <code>jh::sequence</code> if:
     * <ol>
     *   <li>It provides <code>begin()</code> and <code>end()</code> returning valid iterators.</li>
     *   <li>The iterators satisfy <code>jh::input_iterator</code>.</li>
     *   <li>Its <code>begin()</code> and <code>end()</code> can be compared for equality.</li>
     * </ol>
     *
     * This concept is intentionally minimal — it does not enforce mutability,
     * range-based compatibility, or category guarantees beyond input-iterable semantics.
     *
     * @tparam T The candidate type.
     */
    template<typename T>
    concept sequence = requires(const T t)
    {
        { std::begin(t) } -> input_iterator;
        { std::end(t) };
        { std::begin(t) == std::end(t) } -> std::convertible_to<bool>;
        { std::begin(t) != std::end(t) } -> std::convertible_to<bool>;
    };

    namespace detail {
        /**
         * @brief Helper trait to extract <code>value_type</code> from a sequence.
         * @details
         * Performs <strong>cv-ref cleansing</strong> and leverages
         * <code>jh::iterator_t&lt;T&gt;</code> for unified iterator deduction.
         */
        template<typename T>
        struct sequence_value_type_impl {
            using raw_type = std::remove_cvref_t<T>;
            using iterator_type = jh::iterator_t<raw_type>;
            using type = typename std::iterator_traits<iterator_type>::value_type;
        };
    }

    /**
     * @brief Extracts the element type of a sequence.
     * @details
     * Resolves the <code>value_type</code> through <code>jh::iterator_t&lt;T&gt;</code>.
     * Cleanses cv-ref qualifiers for consistent deduction.
     *
     * @tparam T The sequence type.
     */
    template<typename T>
    using sequence_value_type = typename detail::sequence_value_type_impl<T>::type;

    /**
     * @brief Compile-time check for sequence compliance.
     * @details
     * Equivalent to <code>jh::sequence&lt;T&gt;</code>, provided as a
     * <strong>boolean constant</strong> for generic metaprogramming.
     *
     * @tparam T The type to check.
     */
    template<typename T>
    constexpr bool is_sequence = sequence<T>;

    namespace detail {
        /**
         * @brief Internal converter from a sequence to a <code>std::ranges::subrange</code>.
         * @tparam Seq The sequence type.
         */
        template<sequence Seq>
        struct to_range_traits {
            static auto convert(const Seq &s) {
                return std::ranges::subrange(s.begin(), s.end());
            }
        };
    }

    /**
     * @brief Converts a sequence into a standard range object.
     * @details
     * Produces a <code>std::ranges::subrange</code> built from <code>begin()</code> and <code>end()</code>.
     * Designed for use in algorithms expecting standard range-compatible objects.
     *
     * @tparam Seq The sequence type satisfying <code>jh::sequence</code>.
     * @param s The source sequence.
     * @return A <code>std::ranges::subrange</code> representing the sequence.
     */
    template<sequence Seq>
    auto to_range(const Seq &s) {
        return detail::to_range_traits<Seq>::convert(s);
    }
} // namespace jh
