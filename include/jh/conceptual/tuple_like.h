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
 * @file tuple_like.h (conceptual)
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 * @brief Structural detection of tuple-like types with proxy compatibility.
 *
 * @details
 * The <code>jh::concepts::tuple_like</code> concept generalizes the
 * standard tuple protocol to any type that provides:
 * <ul>
 *   <li>A valid <code>std::tuple_size&lt;T&gt;</code> specialization</li>
 *   <li>ADL-resolvable <code>get&lt;I&gt;(t)</code> for each element</li>
 *   <li>Element compatibility between <code>get&lt;I&gt;(t)</code> and
 *       <code>std::tuple_element&lt;I, T&gt;::type</code>, validated via
 *       <code>std::common_reference_t</code></li>
 * </ul>
 *
 * <h3>Design Intent</h3>
 * <p>
 * This concept provides a purely structural detection of tuple-like
 * behavior, without requiring inheritance or specific type traits.
 * It aligns with the semantics of structured bindings as defined by
 * the C++ standard, while supporting proxy or view types that register
 * their own <code>std::common_reference</code> specializations.
 * </p>
 *
 * <h3>Why use <code>common_reference_t</code>?</h3>
 * <p>
 * Proxy-based tuple-like types (such as <code>zip_reference_proxy</code>)
 * may not provide element types identical to their <code>tuple_element</code>
 * declarations. Instead, they rely on <code>std::common_reference_t</code>
 * registration to express semantic compatibility between proxy references
 * and value elements. By validating element compatibility through
 * <code>common_reference_t</code>, the concept gracefully accepts these
 * nested proxies while rejecting unrelated types.
 * </p>
 *
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include <tuple>
#include <utility>
#include <type_traits>
#include <concepts>

namespace jh::concepts {

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
            tuple_like_impl<std::remove_cvref_t<T>>(
    std::make_index_sequence<
    std::tuple_size_v<std::remove_cvref_t<T>>
    >{});

} // namespace jh::concepts
