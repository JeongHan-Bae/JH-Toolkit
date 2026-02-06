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
 * @file container_traits.h
 * @brief Unified deduction model for container element types.
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 *
 * <p>
 * This header defines the trait <code>jh::concepts::container_value_t</code>,
 * a unified and extensible mechanism for deducing the <em>value type</em> of
 * arbitrary container-like types.
 * It harmonizes three deduction strategies &mdash; user registration, declared
 * <code>value_type</code>, and iterator-based inference &mdash; under a deterministic
 * priority system.
 * </p>
 *
 * <h3>Purpose</h3>
 * <p>
 * Many standard and custom containers expose different or ambiguous value-type
 * information. This trait provides a <b>canonical, conflict-resolving</b> method
 * to obtain a single consistent element type used across the entire
 * <code>jh::concepts</code> subsystem (e.g. in
 * <code>closable_container_for</code> and <code>collectable_container_for</code>).
 * </p>
 *
 * <h3>Deduction priority</h3>
 * <ol>
 *   <li><b>User override:</b>
 *       <code>jh::container_deduction&lt;C&gt;::value_type</code> &mdash; explicit registration
 *       always takes precedence.</li>
 *   <li><b>Declared type:</b>
 *       <code>C::value_type</code> &mdash; used if no override is present.</li>
 *   <li><b>Iterator-based deduction:</b>
 *       extracted via <code>iterator_t&lt;C&gt;</code> and
 *       <code>iterator_value_t&lt;iterator_t&lt;C&gt;&gt;</code>.</li>
 *   <li><b>Conflict resolution:</b>
 *       if both declared and deduced forms exist and share a common reference,
 *       the declared form wins.</li>
 *   <li><b>Fallback:</b> <code>void</code> &mdash; deduction failure.</li>
 * </ol>
 *
 * <h3>User customization point</h3>
 * <p>
 * Custom containers can specialize <code>jh::container_deduction&lt;C&gt;</code>
 * to explicitly define a <code>value_type</code>.
 * This mechanism overrides all automatic deduction and provides a
 * stable interface for third-party containers that do not follow standard
 * iterator or value conventions.
 * </p>
 *
 * <h3>Design rationale</h3>
 * <ul>
 *   <li>Fully constexpr and SFINAE-safe; compatible with incomplete types.</li>
 *   <li>Consistent with <code>std::ranges</code> conventions while allowing
 *       user extension without ADL or traits injection.</li>
 *   <li>Provides uniform type deduction across <code>collect</code>,
 *       <code>to</code>, and all range-concept meta-utilities.</li>
 * </ul>
 *
 * @see jh::concepts::iterator_t
 * @see jh::concepts::sequence_t
 * @see jh::concepts::closable_container_for
 * @see jh::concepts::collectable_container_for
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include <ranges>
#include <type_traits>
#include "jh/conceptual/iterator.h"
#include "jh/conceptual/sequence.h"

namespace jh {

    /**
     * @brief User customization point for container element deduction.
     *
     * This template allows users to explicitly register the value type
     * of custom or non-standard containers that cannot be automatically
     * deduced by the generic detection logic.
     *
     * When specialized, this helper provides a consistent <code>value_type</code>
     * that will be used in preference to any automatic deduction.
     * The specialization must define:
     * @code
     * template&lt;&gt;
     * struct jh::container_deduction&lt;YourContainer&gt; {
     *     using value_type = YourElementType;
     * };
     * @endcode
     *
     * Registration should be used in the following cases:
     * <ul>
     * <li>The container does not define <code>value_type</code> internally.</li>
     * <li>The container's <code>value_type</code> does not match the element
     *     type deduced from its iterator behavior.</li>
     * </ul>
     *
     * In either case, a registered specialization of this template takes
     * precedence over all deduction mechanisms and resolves conflicts
     * between declared and deduced types.
     */
    template<typename C>
    struct container_deduction; ///< intentionally undefined, detected via SFINAE

} // namespace jh


namespace jh::concepts::detail {

    /// @brief Detector for <code>C::value_type</code> declaration.
    template<typename C, typename = void>
    struct declared_value {
        using type = void;
    };

    /// @brief Specialization when container declares a member <code>value_type</code>.
    template<typename C>
    struct declared_value<C, std::void_t<typename C::value_type>> {
        using type = typename C::value_type;
    };

/// @brief Detector for iterator-based deduction.
    template<typename C, typename = void>
    struct deduced_value {
        using type = void;
    };

/// @brief Deduction specialization when iterator value type is available.
    template<typename C>
    struct deduced_value<C,
            std::void_t<jh::concepts::iterator_value_t<jh::concepts::iterator_t<C>>>> {
        using type = jh::concepts::iterator_value_t<jh::concepts::iterator_t<C>>;
    };

/// @brief Detector for user override via <code>jh::container_deduction&lt;T&gt;::value</code>.
    template<typename T, typename = void>
    struct container_deduction_resolver {
        using type = void;
    };

/// @brief Specialization when <code>jh::container_deduction&lt;T&gt;::value</code> is defined.
    template<typename T>
    struct container_deduction_resolver<
            T,
            std::void_t<typename jh::container_deduction<T>::value_type>
    > {
        using type = typename jh::container_deduction<T>::value_type;
    };

/**
 * @brief Unified deduction logic for container value type.
 *
 * Deduction priority:
 * <ol>
 * <li>User override via <code>jh::container_deduction&lt;T&gt;::value</code></li>
 * <li>Declared <code>C::value_type</code></li>
 * <li>Deduced from <code>iterator_t&lt;C&gt;</code> and <code>iterator_value_t</code></li>
 * <li>If both declared and deduced exist and have a common reference type,
 *     declared type is preferred</li>
 * <li>Otherwise, <code>void</code></li>
 * </ol>
 */
    template<typename C>
    struct container_value_type_impl {
    private:
        using override_t = typename container_deduction_resolver<C>::type;
        using declared_t = typename declared_value<C>::type;
        using deduced_t = typename deduced_value<C>::type;

    public:
        static constexpr bool has_override = !std::same_as<override_t, void>;
        static constexpr bool has_declared = !std::same_as<declared_t, void>;
        static constexpr bool has_deduced = !std::same_as<deduced_t, void>;

        using type = decltype([]() {
            if constexpr (has_override) {
                // Step 1: user override always wins
                return std::type_identity<override_t>{};
            }
                // Step 2: declared only
            else if constexpr (has_declared && !has_deduced) {
                return std::type_identity<declared_t>{};
            }
                // Step 3: deduced only
            else if constexpr (!has_declared && has_deduced) {
                return std::type_identity<deduced_t>{};
            }
                // Step 4: both exist, and share a common reference
            else if constexpr (has_declared && has_deduced &&
                               std::common_reference_with<declared_t, deduced_t>) {
                return std::type_identity<declared_t>{};
            }
                // Step 5: fallback
            else {
                return std::type_identity<void>{};
            }
        }())::type;
    };

} // namespace jh::concepts::detail

namespace jh::concepts {

    /**
     * @brief Deduce the value type of a container <code>C</code>.
     *
     * @details
     * Resolution rules:
     * <ol>
     * <li>If <code>jh::container_deduction&lt;C&gt;::value_type</code> is explicitly defined,
     *     it overrides all other deduction mechanisms.</li>
     * <li>Otherwise, deduction proceeds using the following logic:
     *     <ul>
     *     <li>If only <code>C::value_type</code> exists, it is used.</li>
     *     <li>If only iterator deduction (<code>iterator_t</code> and
     *         <code>iterator_value_t</code>) is available, it is used.</li>
     *     <li>If both <code>C::value_type</code> and iterator deduction exist,
     *         they must not conflict. If they share a common reference type,
     *         the declared <code>C::value_type</code> is selected.</li>
     *     </ul>
     * </li>
     * <li>If no valid deduction is possible, the result is <tt>void</tt>.</li>
     * </ol>
     *
     * @note
     * When using a proxy reference type within an iterator, the proxy should be
     * implicitly convertible to the iterator's <code>value_type</code> to ensure correct
     * participation in generic deduction. For full interoperability, it is
     * recommended to register explicit <code>std::common_reference</code>
     * specializations as follows:
     *
     * @code
     * template&lt;&gt; struct std::common_reference&lt;ProxyT, T&gt; { using type = T; };
     * template&lt;&gt; struct std::common_reference&lt;T, ProxyT&gt; { using type = T; };
     * template&lt;&gt; struct std::common_reference&lt;ProxyT, ProxyT&gt; { using type = ProxyT; };
     * @endcode
     *
     * For completeness, derived forms should be added for reference and rvalue
     * combinations, inheriting from the base forms:
     * @code
     * template&lt;&gt; struct std::common_reference&lt;ProxyT&amp;, T&amp;&gt;
     *     : std::common_reference&lt;ProxyT, T&gt; {};
     * template&lt;&gt; struct std::common_reference&lt;ProxyT&amp;&amp;, T&amp;&amp;&gt;
     *     : std::common_reference&lt;ProxyT, T&gt; {};
     * // and T, ProxyT swapped forms, ProxyT with itself, etc.
     * @endcode
     *
     * This ensures that proxy iterators remain compatible with generic
     * range-based algorithms and container deduction mechanisms.
     *
     * @tparam C The container type to deduce from.
     * @return The deduced element type or <tt>void</tt> if deduction fails.
     */
    template<typename C>
    using container_value_t = typename detail::container_value_type_impl<C>::type;

    /**
     * @brief Concept that constrains types usable in contiguous, reallocating containers.
     *
     * @details
     * <p>
     * This concept models the requirements for a type <code>T</code> to be safely used as
     * an element type in containers that:
     * store elements contiguously (e.g. <code>std::vector</code>, <code>std::deque</code>), and
     * may perform reallocation and element relocation during growth.</li>
     * </p><p>
     * In addition, it guarantees that elements obtained from the container
     * (via <code>operator[]</code> or iterator dereference yielding <code>T&amp;</code>)
     * can be assigned from values produced externally.
     * </p>
     *
     * <h5>Semantic requirements</h5>
     * A type <code>T</code> satisfies <code>is_contiguous_reallocable</code> if and only if:
     * <ol>
     *   <li>
     *     <b>Object type:</b>
     *     <code>T</code> is an object type (<code>std::is_object_v&lt;T&gt;</code>).
     *   </li>
     *   <li>
     *     <b>External assignability:</b>
     *     <p>
     *     <code>std::assignable_from&lt;T&amp;, T&gt;</code> ensures that an assignment
     *     expression of the form
     *     <code>std::declval&lt;T&amp;&gt;() = std::declval&lt;T&gt;()</code> is well-formed.
     *     </p><p>
     *     This implies that <code>T</code> provides at least one viable assignment
     *     interface capable of assigning into an existing object from a value of
     *     the same type. The concrete form of this interface is intentionally
     *     unconstrained and may correspond to a move assignment, copy assignment,
     *     or a by-value assignment operator (e.g. copy-and-swap).
     *     </p><p>
     *     As a result, any location exposing a mutable reference to a contained
     *     element (such as <code>operator[]</code> or iterator dereference yielding
     *     <code>T&amp;</code>) is guaranteed to support assignment by the user.
     *     </p>
     *   </li>
     *   <li>
     *     <b>Reallocation support:</b>
     *     <code>T</code> is either move-constructible or copy-constructible
     *     (<code>std::is_move_constructible_v&lt;T&gt; || std::is_copy_constructible_v&lt;T&gt;</code>),
     *     guaranteeing that elements can be relocated when the container reallocates.
     *   </li>
     * </ol>
     *
     * <h5>Intended use</h5>
     * This concept is designed to express the minimal semantic contract required by
     * vector-like containers that: <br>
     * reallocate their storage, and expose mutable references to their elements.
     * <br>
     * It is suitable for constraining generic algorithms or container adapters that
     * assume both reallocation safety and external assignment into container elements.
     *
     * @tparam T Element type to be checked.
     */
    template<typename T>
    concept is_contiguous_reallocable =
    std::is_object_v<T> &&
    std::assignable_from<T &, T> &&
    (std::is_move_constructible_v<T> || std::is_copy_constructible_v<T>);

} // namespace jh::concepts
