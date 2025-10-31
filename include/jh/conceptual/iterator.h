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
 * @file iterator.h (conceptual)
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 * @brief Forward declaration and <strong>duck-typed</strong> iterator concept definitions.
 *
 * @details
 * This header defines the unified iterator interface and concept suite
 * for the JH container framework.
 * Unlike traditional STL iterators that rely on static typedefs such as
 * <code>iterator_category</code> or <code>difference_type</code>,
 * this design applies <strong>behavioral duck-typing</strong> —
 * a type is recognized as an iterator if it <em>behaves like one</em>.
 *
 * <h3>Overview</h3>
 * <ul>
 *   <li>Provides a forward declaration of <code>jh::iterator&lt;&gt;</code> for cross-module compatibility.</li>
 *   <li>Defines behavior-based iterator concepts (<code>input_iterator</code>, <code>output_iterator</code>, etc.).</li>
 *   <li>Supports STL iterators, pointers, arrays, and user-defined duck-typed iterators.</li>
 * </ul>
 *
 * <h3>Design Principles</h3>
 * <ol>
 *   <li>
 *     <strong>Behavioral Duck Typing</strong>
 *     <p>
 *     Iterators are recognized by behavior — valid expressions like
 *     <code>*it</code>, <code>++it</code>, <code>it++</code> — not by typedefs.
 *     Any type usable in <code>for(auto&& x : container)</code> qualifies,
 *     covering both <code>for(auto& x : ...)</code> and <code>for(auto x : ...)</code>.
 *     </p>
 *   </li>
 *
 *   <li>
 *     <strong>Unified Deduction Model</strong>
 *     <p>
 *     <code>iterator_t&lt;T&gt;</code> resolves iterators by fallback:
 *     <code>jh::iterator&lt;T&gt;::type</code> →
 *     <code>T.begin()</code> →
 *     array decay →
 *     raw pointer.
 *     </p>
 *   </li>
 *
 *   <li>
 *     <strong>STL Compatibility</strong>
 *     <p>
 *     Works with STL iterators, pointers, arrays, and custom duck-typed types,
 *     with no dependency on <code>&lt;iterator&gt;</code> or
 *     <code>&lt;ranges&gt;</code>.
 *     </p>
 *   </li>
 *
 *   <li>
 *     <strong>Minimal Dependencies</strong>
 *     <p>
 *     Built only on <code>&lt;concepts&gt;</code>, <code>&lt;type_traits&gt;</code>,
 *     and <code>&lt;utility&gt;</code>. Traits and categories are deduced via
 *     valid expressions instead of <code>std::iterator_traits</code>.
 *     </p>
 *   </li>
 *
 *   <li>
 *     <strong>Range Bridging</strong>
 *     <p>
 *     Reuses the same duck-typing model for <code>jh::concepts::sequence</code> and
 *     <code>to_range()</code>, enabling STL-style iteration for any
 *     non-destructive range-like type.
 *     </p>
 *   </li>
 * </ol>
 *
 * <h3>Concept Summary</h3>
 * <table>
 *   <tr><th>Concept</th><th>Behavior Checked</th><th>Primary Use</th></tr>
 *
 *   <tr><td><code>is_iterator</code></td>
 *       <td>Basic iteration (<code>*it</code>, <code>++it</code>)</td>
 *       <td>Type detection</td></tr>
 *
 *   <tr><td><code>indirectly_readable</code></td>
 *       <td>Dereference validity and value/reference consistency</td>
 *       <td>Readable element access</td></tr>
 *
 *   <tr><td><code>indirectly_writable&lt;Out, T&gt;</code></td>
 *       <td>Supports assignment through <code>*it = value</code></td>
 *       <td>Writable element access</td></tr>
 *
 *   <tr><td><code>sentinel_for&lt;S, I&gt;</code></td>
 *       <td>Equality/inequality comparability between iterator and sentinel</td>
 *       <td>Range boundary detection</td></tr>
 *
 *   <tr><td><code>input_iterator</code></td>
 *       <td>Readable, comparable iteration; refines <code>is_iterator</code> and <code>indirectly_readable</code></td>
 *       <td>Single-pass sequential read</td></tr>
 *
 *   <tr><td><code>output_iterator</code></td>
 *       <td>Writable via <code>*it = value</code>; refines <code>is_iterator</code> and <code>indirectly_writable</code></td>
 *       <td>Single-pass sequential write</td></tr>
 *
 *   <tr><td><code>forward_iterator</code></td>
 *       <td>Refines <code>input_iterator</code>; multi-pass, copyable, and
 *           <strong>self-sentinel</strong> (<code>sentinel_for&lt;I, I&gt;</code>)</td>
 *       <td>Stable, reentrant traversal</td></tr>
 *
 *   <tr><td><code>bidirectional_iterator</code></td>
 *       <td>Refines <code>forward_iterator</code>; supports reverse movement
 *           (<code>--it</code>, <code>it--</code>)</td>
 *       <td>Reversible traversal</td></tr>
 *
 *   <tr><td><code>random_access_iterator</code></td>
 *       <td>Refines <code>bidirectional_iterator</code>; supports arithmetic and indexing
 *           (<code>it + n</code>, <code>it[n]</code>)</td>
 *       <td>Contiguous or offset access</td></tr>
 *
 *   <tr><td><code>iterator_t&lt;T&gt;</code></td>
 *       <td>Unified iterator deduction via specialization, <code>begin()</code>, pointer, or array decay</td>
 *       <td>Meta-type for resolving iterator type</td></tr>
 * </table>
 *
 * <h3>Purpose</h3>
 * <p>
 * These definitions unify STL and custom iterator semantics under a single
 * duck-typed system.
 * A type is recognized as an iterator by <strong>behavioral conformance</strong>,
 * not by static signature or inheritance.
 * </p>
 *
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include <concepts>
#include <type_traits>
#include <utility>

namespace jh {
    /**
     * @brief Forward declaration of the <code>iterator</code> class template.
     *
     * @details
     * <code>jh::iterator&lt;Container&gt;</code> serves as a <strong>universal iterator declaration point</strong>
     * within the JH container ecosystem.  
     * It provides both a forward-declaration facility and an integration point
     * for <code>jh::concepts::iterator_t</code> deduction.
     *
     * <h4>Forward Declaration and Local Binding</h4>
     * Enables user-defined containers to declare their iterator type
     * without depending on a complete iterator definition.
     *
     * @code
     * template&lt;typename... Args&gt;
     * class my_container {
     * public:
     *     using iterator = jh::iterator&lt;my_container&gt;;
     *     // ...
     * };
     * @endcode
     *
     * This ensures that other headers can safely reference <code>iterator&lt;&gt;</code> 
     * without circular inclusion or incomplete-type errors.
     *
     * <h4>Integration with <code>jh::concepts::iterator_t</code> Deduction</h4>
     * <code>jh::iterator&lt;&gt;</code> also acts as the <strong>deduction entry point</strong> for
     * <code>jh::concepts::iterator_t&lt;Container&gt;</code>, which automatically resolves
     * a container's iterator type.
     * If a specialization of <code>jh::iterator&lt;Container&gt;</code> exists and
     * defines a nested <code>type</code>, <code>iterator_t&lt;Container&gt;</code>
     * resolves to that <code>type</code>.
     *
     * @code
     * namespace jh {
     *     template&lt;typename... Args&gt;
     *     struct iterator&lt;my_container&lt;Args...&gt;&gt; {
     *         using iterator_category = ...;
     *         using value_type = ...;
     *         using type = iterator;
     *         using difference_type = std::ptrdiff_t;
     *         using pointer = value_type *;
     *         using reference = value_type &;
     *         // iterator implementation ...
     *     };
     * }
     *
     * using it_t = jh::concepts::iterator_t&lt;my_container&lt;int, float&gt;&gt;;  // resolves to iterator::type
     * @endcode
     *
     * These two examples together illustrate a <strong>complete and valid</strong> pattern
     * for defining a container's iterator externally (unlike STL's inner-class convention),
     * while remaining fully compatible with <code>jh::concepts::iterator_t</code> deduction.
     *  
     * It is also legal to define the iterator internally as a nested type —
     * this header does not enforce external specialization.
     *  
     * <p>
     * The <code>jh::generator&lt;T, U&gt;</code> class no longer declares an external
     * <code>jh::iterator&lt;generator&lt;T, U&gt;&gt;</code> specialization; it now implements its
     * iterator internally. This header simply provides a uniform mechanism for
     * automatic deduction via <code>jh::concepts::iterator_t&lt;Container&gt;</code>.
     * </p>
     *
     * <h4>Design Summary</h4>
     * <ul>
     *   <li>Provides a safe <strong>forward-declaration mechanism</strong> for containers.</li>
     *   <li>Acts as a <strong>type-deduction path</strong> for external generic utilities
     *       such as <code>jh::concepts::iterator_t&lt;&gt;</code>, <code>jh::sequence</code>,
     *       and related meta-based concepts.</li>
     * </ul>
     *
     * <h4>Notes</h4>
     * <ul>
     *   <li><code>jh::iterator&lt;&gt;</code> always takes exactly <strong>one</strong> template parameter —
     *       the container type.</li>
     *   <li>It should <strong>not</strong> be directly instantiated.</li>
     *   <li>When no specialization exists, <code>jh::concepts::iterator_t&lt;Container&gt;</code>
     *       falls back to <code>decltype(Container::begin())</code> or a pointer-based iterator.</li>
     * </ul>
     *
     * @tparam Container The type (usually a container) for which an iterator specialization will be defined.
     */
    template<typename Container>
    struct iterator;
} // namespace jh

namespace jh::concepts {
    /**
     * @brief Trait to detect whether a type declares a <code>value_type</code>.
     *
     * @details
     * <code>jh::concepts::has_value_type<&lt;T&gt;</code> is a lightweight compile-time detector
     * that checks if a type <code>T</code> provides a nested member
     * <code>typename T::value_type</code>.
     * It is commonly used when adapting both STL-style containers and custom
     * JH containers to generic algorithms that rely on <code>value_type</code>
     * for type deduction.
     *
     * <h4>Behavior</h4>
     * <ul>
     *   <li>Evaluates to <code>std::true_type</code> if <code>T::value_type</code> is valid.</li>
     *   <li>Evaluates to <code>std::false_type</code> otherwise.</li>
     * </ul>
     *
     * <h4>Helper Constant</h4>
     * <p>
     * The convenience variable template <code>has_value_type_v&lt;T&gt;</code> provides a
     * direct <code>bool</code> constant expression equivalent to
     * <code>has_value_type&lt;T&gt;::value</code>.
     * </p>
     *
     * <h4>Usage Example</h4>
     * @code
     * static_assert(jh::concepts::has_value_type<_v&lt;std::vector&lt;int&gt;&gt;);     // true
     * static_assert(!jh::concepts::has_value_type<_v&lt;int&gt;);                 // false
     * @endcode
     *
     * <h4>Design Notes</h4>
     * <ul>
     *   <li>Implemented using <code>std::void_t</code> SFINAE pattern.</li>
     *   <li>Does not instantiate <code>T</code>; safe for incomplete types.</li>
     *   <li>Serves as a low-level building block for iterator and sequence traits.</li>
     * </ul>
     *
     * @tparam T The type being tested for a nested <code>value_type</code>.
     * @see jh::concepts::iterator_t, jh::concepts::sequence_value_t
     */
    template<typename T, typename = void>
    struct has_value_type : std::false_type {
    };

    template<typename T>
    struct has_value_type<T, std::void_t<typename T::value_type> > : std::true_type {
    };

    /**
     * @brief Convenience constant for <code>has_value_type</code>.
     * @details
     * Equivalent to <code>has_value_type&lt;T&gt;::value</code>,
     * provided for readability in template metaprogramming.
     */
    template<typename T>
    inline constexpr bool has_value_type_v = has_value_type<T>::value;

    namespace detail {

        template<typename I, typename = void>
        struct iterator_value_impl {
            using value_type = void;
        };

        template<typename I> requires has_value_type_v<I>
        struct iterator_value_impl<I> {
            using value_type = typename I::value_type;
            using ref_type = decltype(*std::declval<I &>());

            static_assert(std::convertible_to<ref_type, value_type>,
                          "iterator's operator*() must be convertible to its value_type");

            using type = value_type;
        };

        template<typename I>
        struct iterator_value_impl<I, std::void_t<decltype(*std::declval<I &>())>> {
            using type = std::remove_cvref_t<decltype(*std::declval<I &>())>;
        };

        template<typename T>
        struct iterator_value_impl<T *, void> {
            using type = std::remove_cv_t<T>;
        };

        template<typename I, typename = void>
        struct iterator_reference_impl {
            using type = void;
        };

        template<typename I> requires requires { typename I::reference; }
        struct iterator_reference_impl<I> {
            using reference_type = typename I::reference;
            using deref_type = decltype(*std::declval<I &>());

            static_assert(
                    std::convertible_to<deref_type, reference_type> ||
                    std::convertible_to<reference_type, deref_type>,
                    "iterator::reference must be consistent with decltype(*it)"
            );

            using type = reference_type;
        };

        template<typename I>
        struct iterator_reference_impl<I, std::void_t<decltype(*std::declval<I &>())>> {
            using type = decltype(*std::declval<I &>());
        };

        template<typename T>
        struct iterator_reference_impl<T *, void> {
            using type = T &;
        };

        template<typename I>
        constexpr decltype(auto) adl_iter_move(I &&it)
        noexcept(false) {
            if constexpr (requires { iter_move(std::forward<I>(it)); }) {
                return iter_move(std::forward<I>(it));
            } else if constexpr (requires { std::forward<I>(it).iter_move(); }) {
                return std::forward<I>(it).iter_move();
            } else if constexpr (requires { *std::forward<I>(it); }) {
                if constexpr (std::is_lvalue_reference_v<decltype(*std::forward<I>(it))>)
                    return std::move(*std::forward<I>(it));
                else
                    return *std::forward<I>(it);
            } else {
                static_assert(sizeof(I) == 0, "adl_iter_move: iterator type not readable");
            }
        }

        template<typename I, typename = void>
        struct iterator_difference_impl {
            using type = void;
        };

        template<typename I>
        struct iterator_difference_impl<I, std::void_t<decltype(std::declval<const I &>() -
                                                                std::declval<const I &>())>> {
            using type = decltype(std::declval<const I &>() - std::declval<const I &>());
        };
    } // namespace detail

    /// @brief Deduces the value type of a duck-typed iterator.
    template<typename I>
    using iterator_value_t = typename detail::iterator_value_impl<I>::type;

    /// @brief Deduces the reference type of a duck-typed iterator.
    template<typename I>
    using iterator_reference_t = typename detail::iterator_reference_impl<I>::type;

    /// @brief Deduces the rvalue reference type of a duck-typed iterator.
    template<typename I>
    using iterator_rvalue_reference_t = decltype(detail::adl_iter_move(std::declval<I &>()));

    /// @brief Deduces the difference type of a duck-typed iterator.
    template<typename I>
    using iterator_difference_t = typename detail::iterator_difference_impl<I>::type;

    /**
     * @brief Concept for types that can be read indirectly via dereference.
     * @tparam I Iterator-like type to be checked.
     */
    template<typename I>
    concept indirectly_readable =requires(I &it) {
        typename jh::concepts::iterator_value_t<I>;
        typename jh::concepts::iterator_reference_t<I>;
        typename jh::concepts::iterator_rvalue_reference_t<I>;
        requires (!std::same_as<jh::concepts::iterator_value_t<I>, void>);
        requires (!std::same_as<jh::concepts::iterator_reference_t<I>, void>);
        requires (!std::same_as<jh::concepts::iterator_rvalue_reference_t<I>, void>);
        { *it } -> std::convertible_to<std::remove_cvref_t<jh::concepts::iterator_reference_t<I>>>;
    } && requires(I &&it) {
        detail::adl_iter_move(std::forward<I>(it));
    } &&
                                 std::convertible_to<std::remove_cvref_t<jh::concepts::iterator_reference_t<I>>, jh::concepts::iterator_value_t<I>> &&
                                 std::convertible_to<std::remove_cvref_t<jh::concepts::iterator_rvalue_reference_t<I>>, jh::concepts::iterator_value_t<I>>;

    /**
     * @brief Concept for types that support indirect write operations through dereference.
     * @tparam Out Output iterator-like type to be tested.
     * @tparam T Value type assignable through the iterator.
     */
    template<typename Out, typename T>
    concept indirectly_writable =requires(Out &&o, T &&t) {
        *o = std::forward<T>(t);
        *std::forward<Out>(o) = std::forward<T>(t);
        const_cast<const jh::concepts::iterator_reference_t<Out> &&>(*o) = std::forward<T>(t);
        const_cast<const jh::concepts::iterator_reference_t<Out> &&>(*std::forward<Out>(o)) = std::forward<T>(t);
    };

    /**
     * @brief Concept for detecting iterator-like types based on behavior.
     * @details
     * <ul>
     *   <li>
     *     Supports basic iteration operations:
     *     <code>*it</code>, <code>++it</code>, and <code>it++</code>.
     *   </li>
     *   <li>
     *     If <code>difference_type</code> is defined, it must be a
     *     <tt>signed integral</tt> type.
     *   </li>
     *   <li>
     *     If both <code>difference_type</code> and <code>operator-</code> are defined,
     *     the result of <code>a - b</code> must be convertible to
     *     <code>difference_type</code>.
     *   </li>
     *   <li>
     *     <code>difference_type</code> may be omitted, but its definition must not
     *     conflict with iterator arithmetic semantics.
     *   </li>
     * </ul>
     * @tparam I Type to be tested for iterator-like behavior.
     * @note Conceptually equivalent to <code>std::input_or_output_iterator</code>
     *       but implemented through pure behavioral (duck-typed) validation.
     */
    template<typename I>
    concept is_iterator =requires(I it) {
        *it;
        { ++it } -> std::same_as<I &>;
        it++;
    } &&
                         (
                                 // if difference_type defined
                                 (!requires { typename I::difference_type; }) ||
                                 (requires { typename I::difference_type; } &&
                                  !std::same_as<typename I::difference_type, void> &&
                                  std::signed_integral<typename I::difference_type>
                                 )
                         ) &&
                         (
                                 // match only if both (difference_type) and (operator-) exist
                                 (!requires { typename I::difference_type; } || !requires(I a, I b) { a - b; }) ||
                                 (requires(I a, I b) {
                                     typename I::difference_type;
                                     { a - b } -> std::convertible_to<typename I::difference_type>;
                                 }
                                 )
                         );

    /**
     * @brief Concept for detecting sentinel-iterator compatibility.
     * @details
     * <ul>
     *   <li>
     *     Checks only mutual equality and inequality between <code>S</code> and <code>I</code>
     *     (<code>==</code> and <code>!=</code>), ensuring both directions are valid and
     *     convertible to <code>bool</code>.
     *   </li>
     *   <li>
     *     Unlike the STL's <code>std::sentinel_for</code>, this concept is more permissive:
     *     it does not require specific construction, iterator traits, or range category
     *     relationships.
     *   </li>
     *   <li>
     *     Intended for lightweight duck-typed iteration boundaries where full STL
     *     conformance is unnecessary.
     *   </li>
     * </ul>
     * @tparam S Sentinel type to compare against an iterator.
     * @tparam I Iterator-like type to be checked for compatibility with <code>S</code>.
     */
    template<typename S, typename I>
    concept sentinel_for =requires(const S &s, const I &i) {
        { i == s } -> std::convertible_to<bool>;
        { s == i } -> std::convertible_to<bool>;
        { i != s } -> std::convertible_to<bool>;
        { s != i } -> std::convertible_to<bool>;
    };

    /**
     * @brief Concept for readable, comparable single-pass iterators.
     * @details
     * <ul>
     *   <li>
     *     Refines <code>is_iterator</code> and <code>indirectly_readable</code>,
     *     ensuring the iterator supports dereference and increment operations.
     *   </li>
     *   <li>
     *     Requires compatibility with a corresponding sentinel type
     *     (<code>sentinel_for&lt;S, I&gt;</code>).
     *   </li>
     *   <li>
     *     Represents single-pass input traversal — readable but not necessarily
     *     multi-pass or writable.
     *   </li>
     *   <li>
     *     When the sentinel type <code>S</code> differs from <code>I</code>,
     *     it is recommended <em>not</em> to use this concept alone.
     *     In such cases, it implicitly verifies whether the provided
     *     <code>begin()</code> and <code>end()</code> form a valid pair,
     *     avoiding redundant standalone <code>sentinel_for</code> checks.
     *   </li>
     * </ul>
     * @tparam I Iterator-like type providing readable sequential access.
     * @tparam S Sentinel type marking the end of iteration (defaults to <code>I</code>).
     */
    template<typename I, typename S = I>
    concept input_iterator =
    is_iterator<I> &&
    indirectly_readable<I> &&
    sentinel_for<S, I> && requires(I &it) {
        { ++it } -> std::same_as<I &>;
        { it++ } -> std::convertible_to<I>;
    };

    /**
     * @brief Concept for writable single-pass iterators.
     * @details
     * <ul>
     *   <li>
     *     Refines <code>is_iterator</code> and <code>indirectly_writable</code>,
     *     ensuring the iterator can assign values through dereference.
     *   </li>
     *   <li>
     *     Represents single-pass output traversal — writable but not necessarily
     *     readable or multi-pass.
     *   </li>
     * </ul>
     * @tparam I Iterator-like type providing writable sequential access.
     * @tparam T Value type assignable through the iterator (defaults to <code>iterator_value_t&lt;I&gt;</code>).
     */
    template<typename I, typename T = jh::concepts::iterator_value_t<I>>
    concept output_iterator =
    is_iterator<I> &&
    indirectly_writable<I, T> && requires(I it, T &&t) {
        *it++ = std::forward<T>(t);
    };

    /**
     * @brief Concept for multi-pass, readable, and self-sentinel iterators.
     * @details
     * <ul>
     *   <li>
     *     Refines <code>input_iterator</code>, requiring readability and
     *     <code>std::copyable</code> to support multi-pass traversal.
     *   </li>
     *   <li>
     *     Ensures compatibility with <code>sentinel_for&lt;I, I&gt;</code>,
     *     meaning the iterator can act as its own sentinel.
     *   </li>
     *   <li>
     *     Optionally, a distinct sentinel type <code>S</code> may be provided to
     *     verify that <code>begin()</code> and <code>end()</code> form a consistent pair,
     *     ensuring correct range termination semantics.
     *   </li>
     *   <li>
     *     Guarantees consistent dereference semantics where
     *     <code>*it</code> matches <code>iterator_reference_t&lt;I&gt;</code>.
     *   </li>
     * </ul>
     * @tparam I Iterator-like type providing multi-pass traversal.
     * @tparam S Sentinel type used to verify consistency (defaults to <code>I</code>).
     */
    template<typename I, typename S = I>
    concept forward_iterator =
    input_iterator<I, S> &&
    std::copyable<I> &&
    sentinel_for<I, I> && requires(I &it) {
        { ++it } -> std::same_as<I &>;
        { it++ } -> std::same_as<I>;
        { *it } -> std::same_as<jh::concepts::iterator_reference_t<I>>;
    };

    /**
     * @brief Concept for iterators supporting bidirectional traversal.
     * @details
     * <ul>
     *   <li>
     *     Refines <code>forward_iterator</code>, adding support for reverse movement
     *     using <code>--it</code> and <code>it--</code>.
     *   </li>
     *   <li>
     *     Allows both pre-decrement and post-decrement operations, enabling traversal
     *     in both forward and backward directions.
     *   </li>
     *   <li>
     *     An optional sentinel type <code>S</code> can be provided to check
     *     range consistency between <code>begin()</code> and <code>end()</code>.
     *   </li>
     * </ul>
     * @tparam I Iterator-like type supporting bidirectional movement.
     * @tparam S Sentinel type used to verify consistency (defaults to <code>I</code>).
     */
    template<typename I, typename S = I>
    concept bidirectional_iterator =
    forward_iterator<I, S> && requires(I it) {
        { --it } -> std::same_as<I &>;
        { it-- } -> std::convertible_to<I>;
    };

    /**
     * @brief Concept for iterators supporting random access operations.
     * @details
     * <ul>
     *   <li>
     *     Refines <code>bidirectional_iterator</code>, extending capabilities to
     *     constant-time positional access and arithmetic offset operations.
     *   </li>
     *   <li>
     *     Requires full comparison support:
     *     <code>&lt;</code>, <code>&gt;</code>, <code>&lt;=</code>, and <code>&gt;=</code>.
     *     Equality and inequality are already guaranteed by <code>forward_iterator</code>.
     *   </li>
     *   <li>
     *     Requires a valid <code>iterator_difference_t&lt;I&gt;</code> type for
     *     distance computation between iterators.
     *   </li>
     *   <li>
     *     Must support arithmetic and indexing:
     *     <code>i += n</code>, <code>i -= n</code>,
     *     <code>j + n</code>, <code>n + j</code>,
     *     <code>j - n</code>, and <code>j[n]</code>.
     *   </li>
     *   <li>
     *     An optional sentinel type <code>S</code> can be provided to check
     *     range consistency between <code>begin()</code> and <code>end()</code>.
     *   </li>
     * </ul>
     * @tparam I Iterator-like type supporting random access and arithmetic operations.
     * @tparam S Sentinel type used to verify consistency (defaults to <code>I</code>).
     */
    template<typename I, typename S = I>
    concept random_access_iterator =
    bidirectional_iterator<I, S> &&

    // Can be compared
    requires(const I a, const I b) {
        { a < b }  -> std::convertible_to<bool>;
        { a > b }  -> std::convertible_to<bool>;
        { a <= b } -> std::convertible_to<bool>;
        { a >= b } -> std::convertible_to<bool>;
        // (forward) has already proven that == and != exist
    } &&

    // distance calculable
    requires(const I a, const I b) {
        typename jh::concepts::iterator_difference_t<I>;
        requires (!std::same_as<jh::concepts::iterator_difference_t<I>, void>);
        // is_iterator has proven that the implementation does not conflict with the declaration.
    } &&

    // offset supported
    requires(I i, const I j, const jh::concepts::iterator_difference_t<I> n) {
        { i += n } -> std::same_as<I &>;
        { i -= n } -> std::same_as<I &>;
        { j + n }  -> std::same_as<I>;
        { n + j }  -> std::same_as<I>;
        { j - n }  -> std::same_as<I>;
        { j[n] }   -> std::same_as<jh::concepts::iterator_reference_t<I>>;
    };

    namespace detail {
        // Primary template: default to empty
        template<typename T, typename = void, typename = void>
        struct iterator_resolver;

        // Case 1: has (jh::iterator<T>::type) and it behaves like an iterator
        template<typename T>
        struct iterator_resolver<
                T,
                std::void_t<typename jh::iterator<T>::type>,
                std::enable_if_t<is_iterator<typename iterator<T>::type>>
        > {
            using type = typename iterator<T>::type;
        };


        // Case 2: fallback - has .begin() returning a valid iterator
        template<typename T>
        struct iterator_resolver<
                T,
                std::void_t<decltype(std::declval<T &>().begin())>,
                std::enable_if_t<
                        !requires { typename iterator<T>::type; } &&
                        is_iterator<decltype(std::declval<T &>().begin())>
                >
        > {
            using type = decltype(std::declval<T &>().begin());
        };

        // Case 3: pointer fallback
        template<typename ElemPtr> requires std::is_pointer_v<ElemPtr>
        struct iterator_resolver<ElemPtr, void, void> {
            using type = ElemPtr; // already pointer type, e.g. int*
        };

        // Case 4: array fallback (includes fixed-size and incomplete arrays)
        template<typename ArrayType> requires std::is_array_v<ArrayType>
        struct iterator_resolver<ArrayType, void, void> {
            using type = std::remove_extent_t<ArrayType> *; // decay array into pointer
        };

    }

    /**
     * @brief Extracts the iterator type associated with a given container, pointer, or array.
     *
     * @details
     * <code>jh::concepts::iterator_t&lt;Container&gt;</code> is a unified meta-type alias that deduces
     * the appropriate iterator type for any STL-compatible or JH-style container.
     * It performs a multi-stage deduction process via
     * <code>jh::detail::iterator_resolver</code>, providing consistent iterator inference
     * across containers, raw pointers, and compile-time arrays.
     *
     * <h4>Deduction Rules</h4>
     * <ol>
     *   <li><strong>Specialized Iterator Mapping</strong><br/>
     *       If a specialization of <code>jh::iterator&lt;Container&gt;</code> exists and defines
     *       a nested <code>type</code> satisfying <code>jh::concepts::is_iterator</code>,
     *       that type is selected.<br/>
     *       Example: <code>jh::iterator&lt;jh::generator&lt;T&gt;&gt;::type</code>.</li>
     *
     *   <li><strong>Member <code>begin()</code>-based Fallback</strong><br/>
     *       If the container does not define a <code>jh::iterator&lt;&gt;</code> specialization
     *       but provides a <code>begin()</code> returning a valid iterator
     *       (validated via <code>jh::concepts::is_iterator</code>),
     *       the deduced iterator type becomes
     *       <code>decltype(std::declval&lt;T&amp;&gt;().begin())</code>.</li>
     *
     *   <li><strong>Pointer Fallback</strong><br/>
     *       If the input type is a raw pointer (<code>T*</code>),
     *       the iterator type resolves to itself (<code>T*</code>).</li>
     *
     *   <li><strong>Array Fallback</strong><br/>
     *       If the input type is any array (<code>T[N]</code> or <code>T[]</code>),
     *       it decays into a pointer type (<code>T*</code> or <code>const T*</code>).
     *       This rule guarantees consistent iterator semantics between raw arrays
     *       and standard containers, regardless of whether the array has a fixed
     *       or incomplete size.</li>
     * </ol>
     *
     * <h4>Design Rationale</h4>
     * <ul>
     *   <li>
     *     Provides a <strong>duck-typed fallback mechanism</strong> capable of detecting
     *     iterator types from any iterable entity — not limited to STL or JH containers.
     *   </li>
     *   <li>
     *     Eliminates dependence on <code>Container::iterator</code> typedefs,
     *     instead inferring iterator types directly from valid expressions.
     *   </li>
     *   <li>
     *     Bridges custom, non-standard sequence types to <code>std::ranges::range</code>
     *     through the <code>jh::concepts::sequence</code> module, ensuring smooth
     *     interoperation with standard algorithms and range utilities.
     *   </li>
     * </ul>
     *
     * <h4>Usage Examples</h4>
     * @code
     * using it1 = jh::concepts::iterator_t&lt;std::vector&lt;int&gt;&gt;;        // std::vector<int>::iterator
     * using it2 = jh::concepts::iterator_t&lt;int[10]&gt;;                 // int*  (fixed-size array)
     * using it3 = jh::concepts::iterator_t&lt;int[]&gt;;                   // int*  (incomplete array)
     * using it4 = jh::concepts::iterator_t&lt;jh::generator&lt;int&gt;&gt;;      // jh::generator<int>::iterator
     * using it5 = jh::concepts::iterator_t&lt;MyDuckContainer&lt;float&gt;&gt;;  // decltype(c.begin())
     * @endcode
     *
     * <p>
     * Both <code>int[10]</code> and <code>int[]</code> decay to <code>int*</code>,
     * ensuring <strong>uniform compatibility</strong> across arrays, pointers, and
     * any type that behaves like a range in the JH Toolkit.
     * </p>
     *
     * @tparam Container The type whose iterator type should be deduced.
     */
    template<typename Container>
    using iterator_t = typename detail::iterator_resolver<Container>::type;
} // namespace jh::concepts
