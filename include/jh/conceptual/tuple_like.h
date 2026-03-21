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
 * @file tuple_like.h
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 * @brief
 *   Validation utilities for tuple-like traits, comprising structural
 *   detection of tuple-like semantics and strict matching for pair-like
 *   binary tuple structures.
 *
 * @details
 * This header provides compile-time mechanisms that verify whether a type
 * fulfills the requirements of the tuple-like protocol used by the C++
 * standard, particularly those governing structured bindings. These
 * mechanisms do not introduce tuple-like behavior but instead validate
 * user-defined or library-defined types that claim to conform to the
 * protocol.
 *
 * <h3>Tuple-Like Validation</h3>
 * The <code>jh::concepts::tuple_like</code> concept performs structural
 * verification of tuple-like semantics. A type is considered tuple-like
 * when the following conditions are satisfied:
 * <ul>
 *   <li>A valid <code>std::tuple_size&lt;T&gt;</code> specialization exists.</li>
 *   <li>ADL <code>get&lt;I&gt;(t)</code> is available through argument-dependent lookup
 *       for each valid index.</li>
 *   <li>Each element type exposed by <code>get&lt;I&gt;</code> forms a valid
 *       <code>std::common_reference_t</code> with the corresponding
 *       <code>std::tuple_element&lt;I, T&gt;::type</code>.</li>
 * </ul>
 * These rules correspond to the semantics used by structured bindings.
 * They also allow proxy-based tuple-like types whose element references
 * differ from the declared <code>tuple_element</code> types as long as
 * their common reference relationships represent compatible semantics.
 *
 * <h3>Strict Pair-Like Matching</h3>
 * The <code>jh::concepts::pair_like_for</code> concept is a derived,
 * more restrictive form of tuple-like validation. Inspired by proposals
 * in Clang regarding the use of binary tuple-like structures as inputs
 * to associative container interfaces, it verifies that:
 * <ul>
 *   <li>The candidate type is tuple-like and has arity two.</li>
 *   <li>The first and second element types match the expected key and
 *       value types exactly, after removal of cv-ref qualifiers.</li>
 * </ul>
 * Unlike <code>tuple_like</code>, which only checks semantic
 * compatibility, <code>pair_like_for</code> enforces strict equality of
 * canonical element types. This ensures the type behaves as a pair-like
 * entity in a strong and predictable manner.
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include <tuple>
#include <utility>
#include <type_traits>
#include <concepts>

namespace jh::concepts {

    namespace detail {

        /**
         * @brief Element-level compatibility check for tuple-like structures.
         *
         * @details
         * For each index <code>I</code> in <code>[0, std::tuple_size_v&lt;U&gt;)</code>,
         * this concept ensures:
         * <ul>
         *   <li><code>std::tuple_element&lt;I, U&gt;::type</code> is well-formed</li>
         *   <li><code>get&lt;I&gt;(u)</code> is callable via ADL</li>
         *   <li>A valid <code>std::common_reference_t</code> exists between the two</li>
         * </ul>
         *
         * This check allows flexible compatibility between proxy references
         * and their element declarations.
         */
        template<typename U, std::size_t I>
        concept tuple_element_compatible =
        requires(U&& u) {
            typename std::tuple_element<I, U>::type;
            { get<I>(std::forward<U>(u)) };
            requires requires {
                typename std::common_reference_t<
                        decltype(get<I>(std::forward<U>(u))),
                        typename std::tuple_element<I, U>::type
                >;
            };
        };

        /**
         * @brief Internal fold expression validator for tuple-like structures.
         *
         * @tparam U Candidate type (cleaned by cv-ref).
         * @tparam I Parameter pack of element indices.
         */
        template<typename U, std::size_t... I>
        constexpr bool tuple_like_impl(std::index_sequence<I...>) {
            return (tuple_element_compatible<U, I> && ...);
        }
    }

    /**
     * @brief Concept recognizing <em>tuple-like</em> types.
     *
     * @details
     * A type <code>T</code> satisfies <code>tuple_like</code> if it provides a
     * valid <code>std::tuple_size&lt;T&gt;</code> specialization and all element
     * indices satisfy <code>tuple_element_compatible&lt;T, I&gt;</code>.
     *
     * This allows full recognition of both standard tuple-like types and
     * proxy-based tuple aggregates that register <code>common_reference_t</code>
     * for element interoperability.
     *
     * <h4>Examples:</h4>
     * <ul>
     *   <li><code>std::tuple&lt;...&gt;</code></li>
     *   <li><code>std::pair&lt;...&gt;</code></li>
     *   <li><code>std::array&lt;T, N&gt;</code></li>
     *   <li><code>jh::ranges::zip_reference_proxy&lt;...&gt;</code></li>
     * </ul>
     */
    template<typename T>
    concept tuple_like =
            requires { typename std::tuple_size<std::remove_cvref_t<T>>::type; } &&
            detail::tuple_like_impl<std::remove_cvref_t<T>>(
    std::make_index_sequence<
    std::tuple_size_v<std::remove_cvref_t<T>>
    >{});

    /**
     * @brief Checks whether a type <code>P</code> is a 2-element tuple-like whose element types
     *        exactly match <code>K</code> and <code>V</code> (after remove_cvref).
     *
     * @details
     * Accepts any tuple-like type with <code>tuple_size == 2</code>.
     * Both ADL <code>get&lt;0&gt;(p)</code> and <code>get&lt;1&gt;(p)</code> must yield values
     * whose <code>remove_cvref_t</code> types are exactly <code>K</code>
     * and <code>V</code>, with no implicit conversions allowed.
     *
     * @tparam P The candidate pair-like type.
     * @tparam K The expected key type.
     * @tparam V The expected value type.
     */
    template<typename P, typename K, typename V>
    concept pair_like_for =
    jh::concepts::tuple_like<P> &&
    (std::tuple_size_v<std::remove_cvref_t<P>> == 2) && requires(P &&p) {
        requires std::same_as<
                std::remove_cvref_t<K>,
                std::remove_cvref_t<decltype(get<0>(p))>
        >;

        requires std::same_as<
                std::remove_cvref_t<V>,
                std::remove_cvref_t<decltype(get<1>(p))>
        >;
    };

} // namespace jh::concepts
