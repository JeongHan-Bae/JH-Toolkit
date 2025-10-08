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
 * @file iterator.h
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
 *     <strong>Behavioral Validation (Duck Typing)</strong>
 *     <p>
 *     No static typedefs are required.
 *     Iterators are detected purely by the validity and semantics of expressions
 *     such as <code>*it</code>, <code>++it</code>, and <code>it++</code>.
 *     Any type that works in a range-based for loop (<tt>for(auto &amp;x : container)</tt>)
 *     qualifies as an iterator.
 *     </p>
 *   </li>
 *
 *   <li>
 *     <strong>Unified Deduction Model</strong>
 *     <p>
 *     The alias <code>jh::iterator_t&lt;T&gt;</code> deduces an iterator type through a layered fallback:
 *     </p>
 *     <ul>
 *       <li><code>jh::iterator&lt;T&gt;::type</code> (if defined and valid)</li>
 *       <li><code>decltype(std::declval&lt;T&amp;&gt;().begin())</code></li>
 *       <li>Array decay to pointer (<code>T[N]</code>, <code>T[]</code> → <code>T*</code>)</li>
 *       <li>Raw pointer passthrough (<code>T*</code>)</li>
 *     </ul>
 *   </li>
 *
 *   <li>
 *     <strong>Compatibility</strong>
 *     <p>
 *     Compatible with STL iterators, raw pointers, and containers exposing
 *     <code>.begin()</code> / <code>.end()</code>.
 *     Works seamlessly with both fixed-size and incomplete arrays.
 *     </p>
 *   </li>
 *
 *   <li>
 *     <strong>Lightweight Dependencies</strong>
 *     <p>
 *     Depends only on <code>&lt;iterator&gt;</code>; no reliance on
 *     <code>&lt;ranges&gt;</code> or <code>&lt;concepts&gt;</code>.
 *     Safe for forward-declaration and meta-only use.
 *     </p>
 *   </li>
 * </ol>
 *
 * <h3>Concept Summary</h3>
 * <table>
 *   <tr><th>Concept</th><th>Behavior Checked</th><th>Primary Use</th></tr>
 *   <tr><td><code>is_iterator</code></td><td>Basic iteration (<code>*it</code>, <code>++it</code>)</td><td>Type detection</td></tr>
 *   <tr><td><code>input_iterator</code></td><td>Readable and comparable iteration</td><td>Sequential read</td></tr>
 *   <tr><td><code>output_iterator</code></td><td>Writable via <code>*it = value</code></td><td>Sequential write</td></tr>
 *   <tr><td><code>forward_iterator</code></td><td>Stable dereference after increment</td><td>Multi-pass read</td></tr>
 *   <tr><td><code>bidirectional_iterator</code></td><td>Supports <code>--it</code> and <code>it--</code></td><td>Reversible traversal</td></tr>
 *   <tr><td><code>random_access_iterator</code></td><td>Arithmetic and indexing operations</td><td>Contiguous access</td></tr>
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

#include <iterator>
#include <concepts>

namespace jh {
    /**
     * @brief Forward declaration of the <code>iterator</code> class template.
     *
     * @details
     * <code>jh::iterator&lt;X&gt;</code> serves as a <strong>universal iterator declaration point</strong>
     * within the JH container ecosystem.  
     * It provides both a forward-declaration facility and an integration point
     * for <code>jh::iterator_t</code> deduction.
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
     * <h4>Integration with <code>jh::iterator_t</code> Deduction</h4>
     * <code>jh::iterator&lt;&gt;</code> also acts as the <strong>deduction entry point</strong> for
     * <code>jh::iterator_t&lt;Container&gt;</code>, which automatically resolves
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
     * using it_t = jh::iterator_t&lt;my_container&lt;int, float&gt;&gt;;  // resolves to iterator::type
     * @endcode
     *
     * These two examples together illustrate a <strong>complete and valid</strong> pattern
     * for defining a container's iterator externally (unlike STL's inner-class convention),
     * while remaining fully compatible with <code>jh::iterator_t</code> deduction.
     *  
     * It is also legal to define the iterator internally as a nested type —
     * this header does not enforce external specialization.
     *  
     * <p>
     * The <code>jh::generator&lt;T, U&gt;</code> class no longer declares an external
     * <code>jh::iterator&lt;generator&lt;T, U&gt;&gt;</code> specialization; it now implements its
     * iterator internally. This header simply provides a uniform mechanism for
     * automatic deduction via <code>jh::iterator_t&lt;Container&gt;</code>.
     * </p>
     *
     * <h4>Design Summary</h4>
     * <ul>
     *   <li>Provides a safe <strong>forward-declaration mechanism</strong> for containers.</li>
     *   <li>Acts as a <strong>type-deduction path</strong> for external generic utilities
     *       such as <code>jh::iterator_t&lt;&gt;</code>, <code>jh::sequence</code>,
     *       and related meta-based concepts.</li>
     * </ul>
     *
     * <h4>Notes</h4>
     * <ul>
     *   <li><code>jh::iterator&lt;&gt;</code> always takes exactly <strong>one</strong> template parameter —
     *       the container type.</li>
     *   <li>It should <strong>not</strong> be directly instantiated.</li>
     *   <li>When no specialization exists, <code>jh::iterator_t&lt;Container&gt;</code>
     *       falls back to <code>decltype(Container::begin())</code> or a pointer-based iterator.</li>
     * </ul>
     *
     * @tparam X The type (usually a container) for which an iterator specialization will be defined.
     */
    template<typename X>
    struct iterator;

    /**
     * @brief Trait to detect whether a type declares a <code>value_type</code>.
     *
     * @details
     * <code>jh::has_value_type&lt;T&gt;</code> is a lightweight compile-time detector
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
     * static_assert(jh::has_value_type_v&lt;std::vector&lt;int&gt;&gt;);     // true
     * static_assert(!jh::has_value_type_v&lt;int&gt;);                 // false
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
     * @see jh::iterator_t, jh::sequence_value_type
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
    [[maybe_unused]] inline constexpr bool has_value_type_v = has_value_type<T>::value;

    /**
     * @brief Concept to check if a type behaves as a valid input iterator.
     *
     * @details
     * This concept verifies whether a type <code>I</code> satisfies the minimal
     * requirements of an <strong>input iterator</strong> using structural typing
     * (duck typing).
     * It does not require formal compliance with <code>std::input_iterator</code>;
     * instead, it checks the presence of standard iterator-like operations.
     *
     * <h4>Requirements</h4>
     * A type <code>I</code> is considered an input iterator if:
     * <ul>
     *   <li>It can be <strong>dereferenced</strong> (<code>*it</code> is valid).</li>
     *   <li>It can be <strong>incremented</strong> (<code>++it</code> and <code>it++</code> are valid).</li>
     *   <li>It supports <strong>comparison for equality</strong> (<code>==</code> and <code>!=</code>).</li>
     *   <li><code>std::iterator_traits&lt;I&gt;::value_type</code> is well-formed
     *       and <code>*it</code> is convertible to that type.</li>
     *   <li>It either defines <code>value_type</code> internally, or is a
     *       <strong>raw pointer</strong> (<code>T*</code> or <code>const T*</code>)
     *       for which <code>std::iterator_traits</code> is valid.</li>
     * </ul>
     *
     * <h4>Design Notes</h4>
     * <ul>
     *   <li>Allows both STL-style and user-defined iterator types.</li>
     *   <li>Accepts raw pointers as valid iterators for primitive or POD types.</li>
     *   <li>Performs only structural checks; no inheritance or trait registration is required.</li>
     * </ul>
     *
     * @tparam I The type being tested for input iterator behavior.
     */
    template<typename I>
    concept input_iterator = requires(I it)
    {
        typename std::iterator_traits<I>::value_type;
        { *it } -> std::convertible_to<typename std::iterator_traits<I>::value_type>;
        { ++it } -> std::same_as<I &>;
        { it++ } -> std::same_as<I>;
        { it == it } -> std::convertible_to<bool>;
        { it != it } -> std::convertible_to<bool>;
    };

    /**
     * @brief Concept to check if a type behaves as a valid output iterator.
     *
     * @details
     * This concept validates whether a type <code>I</code> supports
     * <strong>forwardable write-through iteration</strong> for values of type <code>T</code>.
     * It mirrors the semantics of the standard <code>std::output_iterator</code> concept,
     * but avoids dependency on <code>std::indirectly_writable</code> and related STL machinery.
     *
     * <h4>Requirements</h4>
     * A type <code>I</code> is considered an output iterator for <code>T</code> if:
     * <ul>
     *   <li>The expression <code>*i++ = std::forward&lt;T&gt;(t)</code> is valid.</li>
     *   <li>The iterator supports increment operations (<code>++it</code> and <code>it++</code>).</li>
     *   <li>The dereference target may be a proxy — it is <strong>not required</strong> to be a modifiable lvalue.</li>
     *   <li>Assignments through <code>*it</code> and <code>*std::forward&lt;I&gt;(it)</code> must both be valid.</li>
     *   <li>Assignments through <code>const_cast&lt;const std::iter_reference_t&lt;I&gt;&amp;&amp;&gt;(*it)</code> must be valid.</li>
     * </ul>
     *
     * <h4>Design Notes</h4>
     * <ul>
     *   <li>Semantically equivalent to <code>std::output_iterator</code> (C++23 §24.4.4.2).</li>
     *   <li>Omits <code>std::input_or_output_iterator</code> dependency to remain duck-typed and minimal.</li>
     *   <li>Accepts STL-style, pointer-based, or coroutine-style iterators that forward values.</li>
     * </ul>
     *
     * @tparam I The iterator-like type being validated.
     * @tparam T The value type that can be forwarded or assigned through dereference.
     */
    template<typename I, typename T>
    concept output_iterator = requires(I it, T &&value)
    {
        // Core assignability (forwarded write)
        *it = std::forward<T>(value);
        *std::forward<I>(it) = std::forward<T>(value);
        const_cast<const std::iter_reference_t<I> &&>(*it) = std::forward<T>(value);

        // Increment operations (required for iterator semantics)
        ++it;
        it++;
    };

    /**
     * @brief Concept that detects types behaving as a valid forward iterator.
     *
     * @details
     * This concept refines <code>jh::input_iterator</code> by requiring
     * copyability, equality comparability, and stable increment / dereference
     * semantics. It is a <strong>duck-typed</strong> equivalent of
     * <code>std::forward_iterator</code>, designed to validate iterator-like
     * types without relying on <code>std::iterator_category</code> tags.
     *
     * <h4>Semantic Guarantees</h4>
     * A type <code>I</code> models <code>jh::forward_iterator</code> if:
     * <ul>
     *   <li>It satisfies <code>jh::input_iterator&lt;I&gt;</code>.</li>
     *   <li>It is <strong>copyable</strong> — copies of <code>I</code> are
     *       independent, and incrementing one does not affect the other.</li>
     *   <li>It is <strong>equality comparable</strong> with itself.</li>
     *   <li>It supports both pre- and post-increment:
     *       <ul>
     *         <li><code>++it</code> returns <code>I&amp;</code> (reference to self)</li>
     *         <li><code>it++</code> yields a convertible copy of <code>I</code></li>
     *       </ul>
     *   </li>
     *   <li>Dereferencing yields a reference type consistent with
     *       <code>std::iterator_traits&lt;I&gt;::reference</code>.</li>
     * </ul>
     *
     * <h4>Design Notes</h4>
     * <ul>
     *   <li>Emulates the behavioral semantics of <code>std::forward_iterator</code>
     *       without requiring <code>iterator_category</code> tags.</li>
     *   <li>Guarantees <strong>multi-pass stability</strong>: dereferencing
     *       and incrementing can be repeated without invalidating existing copies.</li>
     *   <li>Fully compatible with STL-style iterators and user-defined types
     *       that exhibit equivalent behavior ("duck typing").</li>
     *   <li>Unlike <code>jh::input_iterator</code>, forward iterators
     *       ensure that multiple traversals produce the same results.</li>
     * </ul>
     *
     * @tparam I Iterator-like type to be validated.
     *
     * @see jh::input_iterator
     * @see std::forward_iterator
     */
    template<typename I>
    concept forward_iterator =
    input_iterator<I> &&
    std::copyable<I> &&
    requires(I it) {
        { ++it } -> std::same_as<I&>;
        { it++ } -> std::convertible_to<I>;
        { *it } -> std::same_as<typename std::iterator_traits<I>::reference>;
    };

    /**
     * @brief Concept to check if a type behaves as a valid bidirectional iterator.
     *
     * @details
     * This concept extends <code>jh::forward_iterator</code> by requiring
     * that the iterator supports <strong>both forward and backward</strong>
     * movement through increment (<code>++it</code>, <code>it++</code>)
     * and decrement (<code>--it</code>, <code>it--</code>) operations.
     *
     * <h4>Requirements</h4>
     * A type <code>I</code> satisfies <code>jh::bidirectional_iterator</code> if:
     * <ul>
     *   <li>It satisfies <code>jh::forward_iterator&lt;I&gt;</code>.</li>
     *   <li>It supports pre-decrement (<code>--it</code>) and post-decrement (<code>it--</code>).</li>
     *   <li>Both increment and decrement operations return a valid, dereferenceable iterator.</li>
     * </ul>
     *
     * <h4>Design Notes</h4>
     * <ul>
     *   <li>Semantics are equivalent to <code>std::bidirectional_iterator</code> but
     *       implemented in a duck-typed form without requiring full STL compliance.</li>
     *   <li>Allows traversal in both directions while preserving iterator stability.</li>
     *   <li>Useful for containers supporting reverse traversal (e.g., double-linked lists).</li>
     * </ul>
     *
     * @tparam I The iterator-like type being validated.
     */
    template<typename I>
    concept bidirectional_iterator = forward_iterator<I> && requires(I it)
    {
        --it;
        it--;
    };

    /**
     * @brief Concept to check if a type behaves as a valid random-access iterator.
     *
     * @details
     * This concept extends <code>jh::bidirectional_iterator</code> by requiring
     * constant-time arithmetic and comparison operations, equivalent to
     * pointer-like behavior.
     *
     * <h4>Requirements</h4>
     * A type <code>I</code> satisfies <code>jh::random_access_iterator</code> if:
     * <ul>
     *   <li>It satisfies <code>jh::bidirectional_iterator&lt;I&gt;</code>.</li>
     *   <li>It supports arithmetic operations with integer offsets:
     *       <ul>
     *         <li><code>it + n</code> and <code>it - n</code> yield a valid iterator of type <code>I</code>.</li>
     *         <li><code>it[n]</code> is equivalent to <code>*(it + n)</code> and yields a reference.</li>
     *       </ul>
     *   </li>
     *   <li>It supports ordering comparisons:
     *       <ul>
     *         <li><code>it &lt; it</code>, <code>it &lt;= it</code>, <code>it &gt; it</code>, <code>it &gt;= it</code></li>
     *         <li>All return <code>bool</code>-convertible results.</li>
     *       </ul>
     *   </li>
     * </ul>
     *
     * <h4>Design Notes</h4>
     * <ul>
     *   <li>Semantics are equivalent to <code>std::random_access_iterator</code> (C++20 §24.4.4.4).</li>
     *   <li>Arithmetic guarantees constant-time offset traversal (no intermediate iteration).</li>
     *   <li>Typically implemented by pointers (<code>T*</code>) or contiguous-memory iterators.</li>
     *   <li>Implemented as a <strong>duck-typed</strong> concept to support both STL and user-defined iterators.</li>
     * </ul>
     *
     * @tparam I The iterator-like type being validated.
     */
    template<typename I>
    concept random_access_iterator = bidirectional_iterator<I> && requires(I it, std::ptrdiff_t n)
    {
        { it + n } -> std::same_as<I>;
        { it - n } -> std::same_as<I>;
        { it[n] } -> std::same_as<typename std::iterator_traits<I>::reference>;
        { it < it } -> std::convertible_to<bool>;
        { it > it } -> std::convertible_to<bool>;
        { it <= it } -> std::convertible_to<bool>;
        { it >= it } -> std::convertible_to<bool>;
    };

    /**
     * @brief Concept to check if a type behaves as a general-purpose iterator.
     *
     * @details
     * This is the most relaxed iterator concept in the JH ecosystem.
     * It recognizes any type that behaves like an iterator, including:
     * <ul>
     *   <li>Standard iterators (<code>std::vector&lt;T&gt;::iterator</code> etc.)</li>
     *   <li>Raw pointers (<code>T*</code>)</li>
     *   <li>User-defined duck-typed iterators compatible with <code>std::iterator_traits</code></li>
     * </ul>
     *
     * <h4>Requirements</h4>
     * A type <code>IT</code> satisfies <code>jh::is_iterator</code> if:
     * <ul>
     *   <li><code>std::iterator_traits&lt;IT&gt;::value_type</code> is a valid type.</li>
     *   <li>Supports pre-increment (<code>++it</code>) and post-increment (<code>it++</code>).</li>
     *   <li>Dereferencing (<code>*it</code>) yields either:
     *       <ul>
     *         <li>a reference type convertible to <code>typename std::iterator_traits&lt;IT&gt;::reference</code>, or</li>
     *         <li>a value convertible to <code>typename std::iterator_traits&lt;IT&gt;::value_type</code>.</li>
     *       </ul>
     *   </li>
     * </ul>
     *
     * <h4>Design Notes</h4>
     * <ul>
     *   <li>More permissive than <code>jh::input_iterator</code>, but still enforces dereference validity.</li>
     *   <li>Suitable for template metaprogramming and SFINAE conditions requiring “iterator-like” semantics.</li>
     *   <li>Accepts both reference-returning and value-returning dereference operators.</li>
     * </ul>
     *
     * @tparam IT The candidate type being validated as an iterator.
     */
    template<typename IT>
    concept is_iterator = requires(IT it) {
        typename std::iterator_traits<IT>::value_type;
        { ++it } -> std::same_as<IT &>;
        { it++ } -> std::same_as<IT>;
    } &&
                          (requires(IT it) {
                              { *it } -> std::convertible_to<typename std::iterator_traits<IT>::reference>;
                          } || requires(IT it) {
                              { *it } -> std::convertible_to<typename std::iterator_traits<IT>::value_type>;
                          }
                          );

    namespace detail {
        // Primary template: default to empty
        template<typename T, typename = void, typename = void>
        struct iterator_resolver;

        // Case 1: has jh::iterator<T>::type and it behaves like an iterator
        template<typename T>
        struct iterator_resolver<
                T,
                std::void_t<typename iterator<T>::type>,
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
     * <code>jh::iterator_t&lt;Container&gt;</code> is a unified meta-type alias that deduces
     * the appropriate iterator type for any STL-compatible or JH-style container.
     * It performs a multi-stage deduction process via
     * <code>jh::detail::iterator_resolver</code>, providing consistent iterator inference
     * across containers, raw pointers, and compile-time arrays.
     *
     * <h4>Deduction Rules</h4>
     * <ol>
     *   <li><strong>Specialized Iterator Mapping</strong><br/>
     *       If a specialization of <code>jh::iterator&lt;Container&gt;</code> exists and defines
     *       a nested <code>type</code> satisfying <code>jh::is_iterator</code>,
     *       that type is selected.<br/>
     *       Example: <code>jh::iterator&lt;jh::generator&lt;T&gt;&gt;::type</code>.</li>
     *
     *   <li><strong>Member <code>begin()</code>-based Fallback</strong><br/>
     *       If the container does not define a <code>jh::iterator&lt;&gt;</code> specialization
     *       but provides a <code>begin()</code> returning a valid iterator
     *       (validated via <code>jh::is_iterator</code>),
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
     *   <li>Provides a <strong>duck-typed fallback mechanism</strong> for STL and JH containers alike.</li>
     *   <li>Prevents hard dependencies on <code>Container::iterator</code> typedefs.</li>
     *   <li>Accepts non-STL sequence types that expose <code>begin()</code>/<code>end()</code> pairs.</li>
     *   <li>Maintains full compatibility with <code>std::ranges</code> and <code>jh::sequence</code> concepts.</li>
     * </ul>
     *
     * <h4>Usage Examples</h4>
     * @code
     * using it1 = jh::iterator_t&lt;std::vector&lt;int&gt;&gt;;        // std::vector<int>::iterator
     * using it2 = jh::iterator_t&lt;int[10]&gt;;                 // int*  (fixed-size array)
     * using it3 = jh::iterator_t&lt;int[]&gt;;                   // int*  (incomplete array)
     * using it4 = jh::iterator_t&lt;jh::generator&lt;int&gt;&gt;;      // jh::generator<int>::iterator
     * using it5 = jh::iterator_t&lt;MyDuckContainer&lt;float&gt;&gt;;  // decltype(c.begin())
     * @endcode
     *
     * <p>
     * Both <code>int[10]</code> and <code>int[]</code> decay to <code>int*</code>,
     * ensuring <strong>uniform compatibility</strong> between raw arrays,
     * STL containers, and custom sequence-like types within the JH framework.
     * </p>
     *
     * @tparam Container The type whose iterator type should be deduced.
     */
    template<typename Container>
    using iterator_t = typename detail::iterator_resolver<Container>::type;
} // namespace jh
