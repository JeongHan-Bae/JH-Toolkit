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
 * @file pair.h (utils)
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 * @brief Hybrid pair-like utilities with automatic POD/reference dispatching.
 *
 * <h3>Overview</h3>
 * <p>
 * This header defines <code>jh::pair&lt;T1, T2, Ref = false&gt;</code>, an
 * <b>automatic allocator</b> that selects the most suitable pair-like type at compile time:
 * <ul>
 *   <li><code>jh::pod::pair&lt;A, B&gt;</code> — for POD-like types (by value)</li>
 *   <li><code>jh::utils::val_pair&lt;A, B&gt;</code> — for non-POD types (by value)</li>
 *   <li><code>jh::utils::ref_pair&lt;T1, T2&gt;</code> — for reference pairs (non-owning)</li>
 * </ul>
 * </p>
 *
 * <h3>Automatic Type Resolution</h3>
 * <p>
 * <code>jh::pair</code> automatically determines whether to store elements
 * by value or by reference based on:
 * <ul>
 *   <li>Compile-time POD detection (<code>jh::pod::pod_like&lt;T&gt;</code>)</li>
 *   <li>Explicit reference mode (<code>Ref = true</code>)</li>
 * </ul>
 * This mechanism ensures layout safety and ABI stability without using RTTI
 * or runtime polymorphism.
 * </p>
 *
 * <h3>Design Goals</h3>
 * <ul>
 *   <li>Support seamless structured binding (<code>const auto& [a, b]</code>).</li>
 *   <li>Eliminate unnecessary copies for non-trivial types.</li>
 *   <li>Guarantee POD semantics when possible for low-level operations.</li>
 *   <li>Provide compile-time dispatch instead of runtime virtual dispatch.</li>
 * </ul>
 *
 * <h3>Usage Guidelines</h3>
 * <ul>
 *   <li><code>make_pair(a, b)</code> — create pair with reference semantics (fast, non-owning).</li>
 *   <li><code>make_pair_cp(a, b)</code> — create pair with guaranteed copy semantics.</li>
 *   <li><code>make_val_pair(a, b)</code> — create pair with move or value semantics.</li>
 *   <li>Treat <code>jh::pair</code> as an opaque type suitable for structured binding.</li>
 *   <li>Do <b>not</b> pattern-match or specialize its concrete type.</li>
 * </ul>
 *
 * <h4>Type Deduction Notes</h4>
 * <p>
 * <code>make_val_pair()</code> performs perfect forwarding but never coerces types.
 * For example, string literals deduce as <code>const char*</code>,
 * not <code>std::string</code>; use explicit construction when needed.
 * </p>
 *
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include "jh/pods/pair.h" // jh::pod::pod_like, jh::pod::pair
#include <type_traits>    // std::decay_t, std::forward, etc.

namespace jh::utils {
    /**
     * @brief Lightweight reference-based pair structure.
     *
     * <p>
     * <code>jh::utils::ref_pair&lt;T1, T2&gt;</code> represents a non-owning pair
     * holding two references. It is primarily used internally by
     * <code>jh::make_pair()</code> for non-POD types to avoid value copies.
     * </p>
     * <ul>
     *   <li>Zero-copy: stores only references, no ownership.</li>
     *   <li>Supports transparent structured binding via <code>const auto&amp; [a, b]</code>.</li>
     *   <li>Cannot bind temporaries — construction from rvalues is deleted.</li>
     *   <li>Suitable for short-lived pipelines like <code>zip()</code> or <code>enumerate()</code>.</li>
     * </ul>
     *
     * @note The referenced objects must outlive the pair instance.
     * @note Used automatically by <code>jh::make_pair()</code> when non-POD types are detected.
     *
     * @tparam T1 First element type (as reference)
     * @tparam T2 Second element type (as reference)
     */
    template<typename T1, typename T2>
    struct ref_pair final {
        /// @brief Reference to the first element.
        T1 &first;

        /// @brief Reference to the second element.
        T2 &second;

        /// @brief Alias for the first element type.
        using first_type = T1;

        /// @brief Alias for the second element type.
        using second_type = T2;

        /// @brief Construct a reference pair from two existing lvalues.
        constexpr ref_pair(T1 &a, T2 &b) : first(a), second(b) {}

        /// @brief Deleted constructor to prevent binding to temporaries.
        ref_pair(T1&&, T2&&) = delete;

        /// @brief Default copy constructor (shallow reference copy).
        constexpr ref_pair(const ref_pair &) = default;

        /// @brief Default equality operator.
        constexpr bool operator==(const ref_pair &other) const = default;
    };

    template<typename T1, typename T2>
    ref_pair(T1 &, T2 &) -> ref_pair<T1, T2>;

    /**
     * @brief Access an element of a <code>ref_pair</code> by index.
     *
     * <p>
     * Provides <code>std::get&lt;I&gt;</code>-style access for structured bindings
     * and tuple-like compatibility.
     * </p>
     * @tparam I Element index (0 for <code>first</code>, 1 for <code>second</code>)
     * @tparam T1 Type of the first referenced element
     * @tparam T2 Type of the second referenced element
     * @param p Reference pair to access
     * @return Reference to the corresponding element
     *
     * @note Only indices <b>0</b> and <b>1</b> are valid; other values trigger a static assertion.
     */
    template <std::size_t I, typename T1, typename T2>
    constexpr decltype(auto) get(const ref_pair<T1, T2>& p) {
        if constexpr (I == 0) return p.first;
        else static_assert(I == 1, "ref_pair only has two elements");
        return p.second;
    }

    /**
     * @brief Value-owning, perfectly forwarding pair.
     *
     * <p>
     * Minimal and predictable alternative to <code>std::pair</code>, providing
     * full structural binding and move/copy semantics without allocator or tuple overhead.
     * </p>
     * <p>
     * Designed for explicit composition and efficient transfer of ownership
     * within pipelines or view results.
     * </p>
     * @tparam T1 Type of the first element
     * @tparam T2 Type of the second element
     */
    template<typename T1, typename T2>
    struct val_pair final {
        using first_type = T1;   ///< Alias for first element type.
        using second_type = T2;  ///< Alias for second element type.

        static_assert(std::is_copy_constructible_v<T1> || std::is_move_constructible_v<T1>,
                      "val_pair<T1, T2>: T1 must be move- or copy-constructible");
        static_assert(std::is_copy_constructible_v<T2> || std::is_move_constructible_v<T2>,
                      "val_pair<T1, T2>: T2 must be move- or copy-constructible");

        T1 first;   ///< First stored element.
        T2 second;  ///< Second stored element.

        /// @brief Default constructor.
        constexpr val_pair() = default;

        /**
         * @brief Constructs both elements with perfect forwarding.
         *
         * @param a First element
         * @param b Second element
         */
        template<typename U1, typename U2>
        constexpr val_pair(U1 &&a, U2 &&b)
        noexcept(std::is_nothrow_constructible_v<T1, U1 &&> &&
                 std::is_nothrow_constructible_v<T2, U2 &&>)
        requires std::is_constructible_v<T1, U1 &&> &&
                 std::is_constructible_v<T2, U2 &&>
                : first(std::forward<U1>(a)), second(std::forward<U2>(b)) {}

        /// @brief Copy constructor.
        constexpr val_pair(const val_pair &) = default;

        /// @brief Move constructor.
        constexpr val_pair(val_pair &&) = default;

        /// @brief Equality comparison.
        constexpr bool operator==(const val_pair &) const = default;
    };

    /**
     * @brief Constructs a value-owning pair with perfect forwarding.
     *
     * <p>
     * Explicit factory for <code>val_pair&lt;A, B&gt;</code>, used when move semantics
     * are acceptable or required. Creates a value-owning structure rather than references.
     * </p>
     * <p>
     * Unlike <code>make_pair_cp()</code>, this allows construction from move-only types.
     * </p>
     * <h4>Notes:</h4>
     * <ul>
     *   <li>Deduces exact input types (e.g., string literals become <code>const char*</code>).</li>
     *   <li>Wrap literals explicitly if conversion to owning types like
     *       <code>std::string</code> is intended.</li>
     * </ul>
     *
     * @tparam T1 Type of first argument
     * @tparam T2 Type of second argument
     * @param a First argument
     * @param b Second argument
     * @return <code>val_pair&lt;A, B&gt;</code> — owning pair constructed via perfect forwarding.
     */
    template<typename T1, typename T2>
    [[maybe_unused]] [[nodiscard]] constexpr auto make_val_pair(T1 &&a, T2 &&b) {
        return val_pair<std::decay_t<T1>, std::decay_t<T2>>{
                std::forward<T1>(a), std::forward<T2>(b)
        };
    }

    /**
     * @brief Element accessor for <code>val_pair</code>.
     *
     * <p>
     * Enables structured binding and tuple-style element access via <code>std::get&lt;I&gt;</code>.
     * </p>
     *
     * @tparam I Element index (0 for <code>first</code>, 1 for <code>second</code>)
     * @tparam T1 Type of first element
     * @tparam T2 Type of second element
     * @param p The <code>val_pair</code> to access
     * @return Reference to the requested element
     */
    template <std::size_t I, typename T1, typename T2>
    constexpr decltype(auto) get(const val_pair<T1, T2>& p) noexcept {
        if constexpr (I == 0) return (p.first);
        else static_assert(I == 1, "val_pair only has two elements");
        return (p.second);
    }

} // namespace jh::utils

namespace jh::detail {
    template<typename T>
    constexpr bool is_safe_reference_source =
            std::is_lvalue_reference_v<T> || std::is_trivially_copyable_v<std::decay_t<T> >;

    template<typename T1, typename T2, bool Ref, bool IsPod>
    struct pair_selector_impl;

    // POD branch
    template<typename T1, typename T2, bool Ref>
    struct pair_selector_impl<T1, T2, Ref, true> {
        using type = pod::pair<std::decay_t<T1>, std::decay_t<T2> >;
    };

    // Non-POD branch
    template<typename T1, typename T2, bool Ref>
    struct pair_selector_impl<T1, T2, Ref, false> {
        using type = std::conditional_t<
            Ref,
            utils::ref_pair<T1, T2>,
            utils::val_pair<std::decay_t<T1>, std::decay_t<T2> >
        >;
    };

    // Wrapper to select the correct pair type based on POD-ness
    template<typename T1, typename T2, bool Ref = false>
    struct pair_selector {
        static constexpr bool IsPod =
                pod::pod_like<std::decay_t<T1> > &&
                pod::pod_like<std::decay_t<T2> >;

        using type = typename pair_selector_impl<T1, T2, Ref, IsPod>::type;
    };
}

namespace jh {
    /**
     * @brief Automatic allocator for pair-like types.
     *
     * <h4>Description:</h4>
     * <p>
     * <code>jh::pair&lt;T1, T2, Ref&gt;</code> is a type-level allocator that automatically selects
     * the appropriate pair representation based on type traits:
     * </p>
     * <ul>
     *   <li><code>jh::pod::pair&lt;A, B&gt;</code> — when both <code>T1</code> and <code>T2</code> are POD-like</li>
     *   <li><code>jh::utils::val_pair&lt;A, B&gt;</code> — when non-POD and <code>Ref == false</code></li>
     *   <li><code>jh::utils::ref_pair&lt;T1, T2&gt;</code> — when non-POD and <code>Ref == true</code></li>
     * </ul>
     *
     * <h4>Usage Notes:</h4>
     * <ul>
     *   <li>Acts as an automatic allocator (type dispatcher), not a concrete class.</li>
     *   <li>Ideal for structural binding and <code>auto</code>-deduced return types.</li>
     *   <li>Do not use as a persistent storage type — use explicit
     *       <code>jh::pod::pair</code>, <code>jh::utils::val_pair</code>, or
     *       <code>jh::utils::ref_pair</code> for clarity.</li>
     *   <li>Default mode (<code>Ref = false</code>) produces value-owning semantics.</li>
     * </ul>
     *
     * @note Although declared as a template alias, it behaves like a dynamic type router at compile time.
     *       Users may treat <code>jh::pair&lt;...&gt;</code> as a unified API surface for all pair types.
     *
     * @see jh::utils::val_pair, jh::utils::ref_pair, jh::pod::pair
     */
    template<typename T1, typename T2, bool Ref = false>
    using pair = typename detail::pair_selector<T1, T2, Ref>::type;

    /**
     * @brief Creates a pair-like object optimized for structural binding and zero-overhead semantics.
     *
     * <h4>Description:</h4>
     * <p>
     * This function constructs a pair-like object for use in view pipelines
     * (<code>zip</code>, <code>enumerate</code>, etc.), selecting between
     * POD and reference storage automatically:
     * </p>
     * <ul>
     *   <li>If both <code>T1</code> and <code>T2</code> are POD-like, returns <code>jh::pod::pair&lt;A, B&gt;</code> by value.</li>
     *   <li>If either type is non-POD, returns <code>jh::utils::ref_pair&lt;T1, T2&gt;</code> by reference.</li>
     * </ul>
     *
     * <h4>Design Rationale:</h4>
     * <ul>
     *   <li>Aims to minimize runtime cost — POD copies are trivial, while non-POD types are referenced to avoid overhead.</li>
     *   <li>Optimized for structured binding idioms (e.g. <code>const auto&amp; [a, b]</code>).</li>
     *   <li>Behaves as a unified front-end for POD and non-POD pairing.</li>
     * </ul>
     *
     * <h4>Warnings:</h4>
     * <ul>
     *   <li>If any referenced object is temporary or non-owned, the resulting <code>ref_pair</code> will dangle.</li>
     *   <li>Use <code>make_pair_cp()</code> if you need copy semantics for safety.</li>
     * </ul>
     *
     * @tparam T1 Type of first element (deduced)
     * @tparam T2 Type of second element (deduced)
     * @param a First element
     * @param b Second element
     * @return Automatically selected:
     *         <code>jh::pod::pair&lt;A, B&gt;</code> (by value) or
     *         <code>jh::utils::ref_pair&lt;T1, T2&gt;</code> (by reference)
     *
     * @see jh::make_pair_cp, jh::utils::ref_pair, jh::pod::pair
     */
    template<typename T1, typename T2>
    [[nodiscard]] constexpr auto make_pair(T1 a, T2 b) {
        return pair<T1, T2, true>{a, b}; // POD: by value; non-POD: by reference
    }

    /**
     * @brief Constructs a copy-owning pair-like object.
     *
     * <h4>Description:</h4>
     * <p>
     * This variant of <code>make_pair</code> always performs a <b>copy</b> of both elements,
     * ensuring that the resulting pair owns its data. It is designed for situations
     * where move operations may alter or invalidate the source values.
     * </p>
     *
     * <h4>Behavior:</h4>
     * <ul>
     *   <li>If both <code>T1</code> and <code>T2</code> are POD-like, returns <code>jh::pod::pair&lt;A, B&gt;</code> (by value).</li>
     *   <li>Otherwise returns <code>jh::utils::val_pair&lt;A, B&gt;</code>, a value-owning structure.</li>
     * </ul>
     *
     * <h4>Design Rationale:</h4>
     * <ul>
     *   <li>Guarantees copy semantics — never returns references.</li>
     *   <li>Ensures stability for pipelines where moves could break state.</li>
     *   <li>Acts as a "safe" counterpart to <code>make_pair()</code>.</li>
     * </ul>
     *
     * <h4>Warnings:</h4>
     * <ul>
     *   <li>Copying may incur overhead for non-trivial types.</li>
     *   <li>Do not use with temporaries unless explicit copying is intended.</li>
     * </ul>
     *
     * @tparam T1 First element type (must be copy-constructible)
     * @tparam T2 Second element type (must be copy-constructible)
     * @param a First element
     * @param b Second element
     * @return Automatically selected:
     *         <code>jh::pod::pair&lt;A, B&gt;</code> or
     *         <code>jh::utils::val_pair&lt;A, B&gt;</code>, depending on POD-ness
     *
     * @see jh::make_pair, jh::make_val_pair, jh::pod::pair, jh::utils::val_pair
     */
    template<typename T1, typename T2>
    requires (std::is_copy_constructible_v<T1> && std::is_copy_constructible_v<T2>)
    [[nodiscard]] constexpr auto make_pair_cp(T1 &a, T2 &b) {
        static_assert(detail::is_safe_reference_source<T1> && detail::is_safe_reference_source<T2>,
                      "make_pair_cp(): Avoid binding to temporary values");
        return pair<T1, T2>{a, b}; // Always copies; never references
    }


    /**
     * @brief Concept defining pair-like types.
     *
     * <h4>Description:</h4>
     * <p>
     * A type satisfies <code>pair_like</code> if it behaves structurally like a pair:
     * it exposes public members <code>.first</code> and <code>.second</code>, and
     * both elements are either copy-constructible or move-constructible.
     * </p>
     *
     * <h4>Usage:</h4>
     * <ul>
     *   <li>Used internally in view pipelines (e.g., <code>zip</code>, <code>enumerate</code>).</li>
     *   <li>Enables generic destructuring with structured bindings.</li>
     *   <li>Useful for concepts and SFINAE-based checks for pair-like behavior.</li>
     * </ul>
     *
     * <h4>Requirements:</h4>
     * <ul>
     *   <li><code>T</code> must have public members <code>first</code> and <code>second</code>.</li>
     *   <li>Each member must be copy- or move-constructible.</li>
     * </ul>
     *
     * @tparam T Type to be checked.
     * @see jh::utils::ref_pair, jh::utils::val_pair, jh::pod::pair
     */
    template<typename T>
    concept pair_like = requires(T t)
                        {
                            t.first;
                            t.second;
                        } &&
                        (
                                std::copy_constructible<decltype(std::declval<T>().first)> ||
                                std::move_constructible<decltype(std::declval<T>().first)>
                        ) &&
                        (
                                std::copy_constructible<decltype(std::declval<T>().second)> ||
                                std::move_constructible<decltype(std::declval<T>().second)>
                        );

    /**
     * @brief Boolean helper for <code>pair_like</code> concept.
     *
     * <h4>Usage:</h4>
     * <p>
     * Provides a convenient constant expression form of the <code>pair_like</code> concept
     * for use in static assertions and conditional expressions.
     * </p>
     *
     * @tparam T Type to test.
     * @return <code>true</code> if <code>T</code> satisfies <code>pair_like</code>.
     */
    template<typename T>
    [[maybe_unused]] constexpr bool is_pair_like_v = pair_like<T>;

} // namespace jh

