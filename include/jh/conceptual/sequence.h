/**
 * \verbatim
 * Copyright 2025 JeongHan-Bae &lt;mastropseudo@gmail.com&gt;
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
 * @file sequence.h (conceptual)
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 * @brief Concept and utilities for immutable sequence detection in C++20.
 *
 * @details
 * The <code>jh::concepts::sequence</code> module defines a lightweight C++20 concept
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
 *   <li>Integrate naturally with <code>jh::concepts::iterator_t</code> and iterator concepts.</li>
 * </ul>
 *
 * <h3>Key Components</h3>
 * <ul>
 *   <li><code>jh::concepts::sequence</code> &mdash; Concept ensuring immutable iteration.</li>
 *   <li><code>jh::concepts::sequence_value_t&lt;T&gt;</code> &mdash; Extracts element type via <code>iterator_t</code>.</li>
 *   <li><code>jh::concepts::is_sequence&lt;T&gt;</code> &mdash; Compile-time boolean for detection.</li>
 *   <li>
 *     <code>jh::to_range()</code> &mdash;
 *     Converts any <code>sequence</code> into a <em>range-compatible</em> object.
 *     If the input already models <code>std::ranges::range</code> and is safely
 *     movable or copyable, it is forwarded unchanged.
 *     Otherwise &mdash; for non-movable, non-copyable lvalues &mdash; it automatically
 *     produces a <code>std::ranges::subrange</code> to preserve reference validity.
 *     For non-range sequences, it wraps them in a
 *     <code>jh::ranges::range_adaptor</code>.
 *   </li>
 * </ul>
 *
 * <h3>Design Notes</h3>
 * <ul>
 *   <li>Type deduction uses <code>jh::concepts::iterator_t&lt;T&gt;</code>, providing
 *       a unified meta-interface for STL and custom containers.</li>
 *   <li>All qualifiers (<code>const</code>, <code>volatile</code>, references)
 *       are removed before deduction to ensure consistent behavior.</li>
 *   <li>Concept validation is purely structural &mdash; no explicit typedefs are required.</li>
 *   <li>Serves as the foundational layer for the <code>jh::range</code> system and
 *       range interoperability across user-defined sequence types.</li>
 * </ul>
 *
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include <iterator>    // for "iterator_tag"s
#include <ranges>
#include <type_traits>
#include "iterator.h"

namespace jh::concepts {

    /**
     * @brief Concept that checks whether a type provides at least const (non-destructive) iteration.
     *
     * @details
     * <p>
     * A type <code>T</code> satisfies <code>jh::concepts::sequence</code> if:
     * </p>
     * <ol>
     *   <li>It provides <code>begin()</code> and <code>end()</code> that can be called on a <code>const T&amp;</code>.</li>
     *   <li>The returned iterators satisfy <code>jh::input_iterator</code>.</li>
     *   <li><code>begin()</code> and <code>end()</code> are comparable for equality and inequality.</li>
     * </ol>
     *
     * <p>
     * The concept requires that iteration be <strong>non-destructive</strong> &mdash;
     * traversing the sequence must not modify or consume its internal state.
     * </p>
     *
     * <p>
     * The type must support at least <em>const iteration</em> (i.e. traversal through
     * <code>const T&amp;</code>), but may also provide mutable iteration (<code>T&amp;</code>).
     * In other words, a <code>sequence</code> represents any type that can be iterated
     * safely multiple times without state consumption, regardless of whether its
     * elements themselves are mutable.
     * </p>
     *
     * @tparam T The candidate type to test for sequence behavior.
     */
    template<typename T>
    concept sequence = requires(const T &t)
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
         * <code>jh::concepts::iterator_t&lt;T&gt;</code> for unified iterator deduction.
         */
        template<typename T>
        struct sequence_value_type_impl {
            using raw_type = std::remove_cvref_t<T>;
            using iterator_type = jh::concepts::iterator_t<raw_type>;
            using type = typename jh::concepts::iterator_value_t<iterator_type>;
        };
    } // namespace detail

    /**
     * @brief Extracts the element type of a sequence.
     * @details
     * Resolves the <code>value_type</code> through <code>jh::concepts::iterator_t&lt;T&gt;</code>.
     * Cleanses cv-ref qualifiers for consistent deduction.
     *
     * @tparam T The sequence type.
     */
    template<typename T>
    using sequence_value_t = typename detail::sequence_value_type_impl<T>::type;

    /**
     * @brief Compile-time check for sequence compliance.
     * @details
     * Equivalent to <code>jh::concepts::sequence&lt;T&gt;</code>, provided as a
     * <strong>boolean constant</strong> for generic metaprogramming.
     *
     * @tparam T The type to check.
     */
    template<typename T>
    constexpr bool is_sequence = sequence<T>;
} // namespace jh::concepts

namespace jh::ranges {
    template<typename Seq>
    class range_adaptor;

    template<typename Seq>
    range_adaptor(Seq &&) -> range_adaptor<Seq>;
} // namespace jh::ranges

namespace jh::concepts::detail {
    /**
     * @brief Converts a sequence into a <em>range-compatible</em> form.
     *
     * @details
     * <p>
     * This function guarantees that any valid <code>jh::concepts::sequence</code>
     * can be safely transformed into a range object usable by
     * <code>std::ranges</code> algorithms or range-based for loops.
     * In other words, <code>jh::to_range()</code> always returns a value that
     * models <code>std::ranges::range</code>.
     * </p>
     *
     * <p>
     * The conversion strategy adapts to the capabilities and value category of
     * the input sequence:
     * </p>
     *
     * <ul>
     *   <li>
     *     <strong>If the input already models</strong> <code>std::ranges::range</code>:
     *     <ul>
     *       <li>
     *         When it is movable or copyable, it is perfectly forwarded (idempotent).
     *       </li>
     *       <li>
     *         When it is a non-movable and non-copyable left value, it is converted
     *         into a <code>std::ranges::subrange</code> constructed from its
     *         <code>begin()</code> / <code>end()</code> iterators to ensure safe access.
     *       </li>
     *     </ul>
     *   </li>
     *
     *   <li>
     *     <strong>If the input does not model</strong> <code>std::ranges::range</code>:
     *     <ul>
     *       <li>
     *         Left-value sequences are wrapped by <code>jh::ranges::range_adaptor&lt;Seq&gt;</code>
     *         (by reference).
     *       </li>
     *       <li>
     *         Right-value or value sequences are wrapped by
     *         <code>jh::ranges::range_adaptor&lt;S&gt;</code> (moved).
     *       </li>
     *     </ul>
     *   </li>
     * </ul>
     *
     * <p>
     * As a result, <code>jh::to_range()</code> provides a uniform bridge between
     * duck-typed <code>sequence</code> objects and the standard range ecosystem,
     * ensuring they can always participate in range-based algorithms and
     * <code>std::ranges</code> pipelines without additional adaptation.
     * </p>
     *
     * @tparam Seq The sequence type satisfying <code>jh::concepts::sequence</code>.
     * @param s The input sequence to convert.
     * @return A guaranteed <code>std::ranges::range</code>-compatible object:
     *         either the same range, a <code>std::ranges::subrange</code>,
     *         or a <code>jh::ranges::range_adaptor</code>.
     *
     * @note This function ensures algorithm-level compatibility &mdash; any valid
     *       <code>sequence</code> passed through <code>jh::to_range()</code>
     *       becomes immediately usable in <code>std::ranges</code> algorithms.
     */
    template<sequence Seq>
    struct to_range_traits {
        static decltype(auto) convert(Seq &&s) {  // auto -> decltype(auto)
            using S = std::remove_reference_t<Seq>;

            if constexpr (std::ranges::range<S>) {
                // non-copyable non-movable
                if constexpr ((!std::is_copy_constructible_v<S> &&
                               !std::is_move_constructible_v<S>) ||
                              (!std::is_copy_constructible_v<const S> &&
                               !std::is_move_constructible_v<const S>)) {
                    // copy iterators for subrange
                    return std::ranges::subrange(std::ranges::begin(s),
                                                 std::ranges::end(s));
                } else if constexpr (std::is_lvalue_reference_v<Seq>) {
                    // copyable left value return by ref
                    return (s);
                } else {
                    // right value, forward
                    return std::forward<Seq>(s);
                }
            } else {
                if constexpr (std::is_lvalue_reference_v<Seq>) {
                    // by ref
                    return jh::ranges::range_adaptor<Seq>(s);
                } else {
                    // if a value or a right value provided, move it
                    return jh::ranges::range_adaptor<S>(std::move(s));
                }
            }
        }
    };
} // namespace jh::concepts::detail

namespace jh {
    /**
     * @brief Converts a sequence into a usable <code>std::ranges::range</code> object.
     *
     * @details
     * <p>
     * This is the user-facing interface for range conversion.
     * It guarantees that any valid <code>jh::concepts::sequence</code> can be
     * transformed into an object that models <code>std::ranges::range</code>,
     * regardless of whether the original type is movable, copyable,
     * or a simple sequence-like container.
     * </p>
     *
     * <p>
     * The conversion is <b>idempotent</b> &mdash; if the input already models
     * <code>std::ranges::range</code> and is safely movable or copyable,
     * it is forwarded unchanged.
     * Otherwise, the function wraps or adapts it internally to ensure
     * range compatibility and reference safety.
     * </p>
     *
     * @tparam Seq A type satisfying <code>jh::concepts::sequence</code>.
     * @param s The input sequence to convert.
     * @return An object guaranteed to model <code>std::ranges::range</code>,
     *         ready for direct use in range-based for loops or
     *         <code>std::ranges</code> algorithms.
     */
    template<concepts::sequence Seq>
    decltype(auto) to_range(Seq &&s) {
        return concepts::detail::to_range_traits<Seq>::convert(std::forward<Seq>(s));
    }
} // namespace jh

namespace jh::concepts {
    /**
     * @brief Deduce the <code>difference_type</code> used by a sequence after range adaptation.
     *
     * @details
     * <p>
     * This alias evaluates the <code>difference_type</code> that would be observed
     * by STL algorithms after converting a <code>jh::concepts::sequence</code>
     * into a valid <code>std::ranges::range</code> via <code>jh::to_range()</code>.
     * </p>
     *
     * <p>
     * In effect, it represents the type actually used by standard range-based
     * algorithms for distance and offset calculations. Since every legal range
     * must define a valid difference type, this alias is always well-formed.
     * </p>
     *
     * <p>
     * If the original <code>sequence</code> does not provide a deducible
     * difference type, the internal <code>range_adaptor</code> automatically
     * falls back to <code>std::ptrdiff_t</code> to ensure STL algorithm
     * compatibility.
     * </p>
     *
     * @tparam Seq A type satisfying <code>jh::concepts::sequence</code>.
     */
    template<sequence Seq>
    using sequence_difference_t =
            std::ranges::range_difference_t<decltype(jh::to_range(std::declval<Seq &>()))>;

} // namespace jh::concepts

#include "jh/ranges/range_adaptor.h"