/**
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
 */

/**
 * @file pair.h (utils)
 * @author JeongHan-Bae <mastropseudo@gmail.com>
 * @brief Provides utilities for constructing pair-like objects with either value or reference semantics.
 *
 * @details
 * This header defines a hybrid `make_pair` utility for composing pair-like objects:
 * - If both inputs are POD-like, returns `jh::pod::pair<T1, T2>` (by value)
 * - Otherwise returns `jh::utils::ref_pair<T1, T2>` (pair of references)
 *
 * Design goal:
 * - Enable efficient and expressive structural bindings via `const auto& [a, b]`
 * - Avoid unnecessary copies for complex types
 * - Preserve exact semantics of original references for non-trivial types
 *
 * This utility is intended to support view pipelines such as `zip()` or `enumerate()`
 * where lazily created pair-like values are destructured by the user.
 *
 * The returned type is intentionally transparent: user code should treat it like
 * any `pair` and avoid templating over it.
 *
 * ❗️Important usage note:
 * - You are strongly discouraged from pattern-matching the result of `make_pair`/`make_pair_cp` via template specialization or type traits.
 * - This utility is designed **specifically** for automatic packing/unpacking (typically via structured binding).
 *   Use `const auto& [a, b] = make_pair(...);` — this is the intended idiom.
 *
 * - If you need to implement templated logic or persistent storage based on the pair-like result,
 *   you should explicitly use `jh::pod::pair`, `jh::utils::ref_pair`, or `jh::utils::val_pair` as appropriate for your use case.
 *
 * Note:
 * `make_val_pair()` uses template argument deduction.
 * It does NOT perform implicit conversion to target types.
 * You must explicitly construct complex types like `std::string` or `std::vector<int>` when needed.
 *
 * Additional guidance:
 *
 * - `jh::pair<T1, T2>` is not a class, but a type-dispatching alias.
 *   It resolves to one of three concrete types, depending on POD-ness and `Ref`.
 *
 * - Default behavior assumes values are to be owned. If you need references,
 *   use `make_pair()` for structure binding.
 *
 * - Use `make_pair_cp()` only when you know copying is necessary to avoid dangling references.
 *
 * - If you know exactly which pair type you want, use the corresponding class directly.
 *
 * - Avoid using `make_val_pair(...)` with types like `"string literal"` unless you explicitly
 *   wrap them (e.g., `std::string("literal")`) to ensure correct deduction.
 */


#pragma once

#include "../pods/pair.h" // jh::pod::pod_like, jh::pod::pair
#include <type_traits>    // std::decay_t, std::forward, etc.

namespace jh::utils {
    /**
     * @brief A lightweight pair of references.
     *
     * Used internally by `make_pair` to represent non-POD pair-like structures
     * without incurring value copies.
     *
     * Not intended for long-term storage or ownership
     *
     * This type supports transparent structure binding via `const auto& [a, b]`.
     * Not intended for storage or long-lived ownership.
     *
     * @tparam T1 First element type (as reference)
     * @tparam T2 Second element type (as reference)
     */
    template<typename T1, typename T2>
    struct ref_pair final {
        T1 &first;
        T2 &second;

        constexpr ref_pair(T1 &a, T2 &b) : first(a), second(b) {
        }
        // Prevent construction from rvalue references
        ref_pair(T1&&, T2&&) = delete;

        constexpr ref_pair(const ref_pair &) = default;

        constexpr bool operator==(const ref_pair &other) const = default;
    };

    template<typename T1, typename T2>
    ref_pair(T1 &, T2 &) -> ref_pair<T1, T2>;

    template <std::size_t I, typename T1, typename T2>
    constexpr decltype(auto) get(const ref_pair<T1, T2>& p) {
        if constexpr (I == 0) return p.first;
        else static_assert(I == 1, "ref_pair only has two elements");
        return p.second;
    }

    /**
     * @brief A value-owning, perfectly forwarding pair.
     *
     * This struct provides a minimal, predictable alternative to `std::pair`,
     * optimized for explicit structural composition and perfect forwarding.
     *
     * Intended as a simpler alternative to std::pair
     *
     * Unlike `std::pair`, this type avoids unnecessary constructors or overhead
     * associated with allocator-aware or tuple integration.
     *
     * @tparam T1 First element type
     * @tparam T2 Second element type
     */
    template<typename T1, typename T2>
    struct val_pair final {
        static_assert(std::is_copy_constructible_v<T1> || std::is_move_constructible_v<T1>,
                      "val_pair<T1, T2>: T1 must be move- or copy-constructible");
        static_assert(std::is_copy_constructible_v<T2> || std::is_move_constructible_v<T2>,
                      "val_pair<T1, T2>: T2 must be move- or copy-constructible");

        T1 first;
        T2 second;

        constexpr val_pair() = default;

        template<typename U1, typename U2>
        constexpr val_pair(U1 &&a, U2 &&b)
            noexcept(std::is_nothrow_constructible_v<T1, U1 &&> &&
                     std::is_nothrow_constructible_v<T2, U2 &&>)
            requires std::is_constructible_v<T1, U1 &&> &&
                     std::is_constructible_v<T2, U2 &&>
            : first(std::forward<U1>(a)), second(std::forward<U2>(b)) {
        }

        constexpr val_pair(const val_pair &) = default;

        constexpr val_pair(val_pair &&) = default;

        constexpr bool operator==(const val_pair &) const = default;
    };

    /**
     * @brief Constructs a value-owning pair from arbitrary types with perfect forwarding.
     *
     * This is the explicit constructor-style builder for `val_pair<A, B>`, intended for
     * use when move semantics are acceptable or desired.
     *
     * Unlike `make_pair_cp()`, this allows moves and non-copyable types.
     *
     * @warning Deduces exact input types. If you pass a string literal, it will deduce
     *          `const char*`, not `std::string`. You must explicitly wrap such inputs.
     *
     * Incorrect:
     *     jh::pair<int, std::string> p = make_val_pair(42, "Hello"); // Error Compiling (Deduces const char*)
     *
     * Correct:
     *     jh::pair<int, std::string> p = make_val_pair(42, std::string("Hello"));
     *
     * @tparam T1 First argument type
     * @tparam T2 Second argument type
     * @param a First argument
     * @param b Second argument
     * @return `val_pair<A, B>` (value-owning)
     */
    template<typename T1, typename T2>
    [[nodiscard]] constexpr auto make_val_pair(T1 &&a, T2 &&b) {
        return val_pair<std::decay_t<T1>, std::decay_t<T2> >{
            std::forward<T1>(a), std::forward<T2>(b)
        };
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
     * @brief Type alias that resolves to a pair-like type depending on the input types and mode.
     *
     * This is not a concrete class, but a type-dispatching alias that resolves to one of:
     *   - `jh::pod::pair<A, B>`     if both T1 and T2 are POD-like
     *   - `jh::utils::val_pair<A, B>` if not POD and Ref == false
     *   - `jh::utils::ref_pair<T1, T2>` if not POD and Ref == true
     *
     * By default, this alias selects a value-owning structure when appropriate.
     * For manual control, use the concrete types (`utils::val_pair`, `utils::ref_pair`, `pod::pair`) directly.
     *
     * @note This alias is suitable for structured binding and auto-return idioms,
     *       but should not be used as a generic templated pair type (due to its non-uniform identity).
     *
     * @tparam T1 First element type
     * @tparam T2 Second element type
     * @tparam Ref Whether to return a reference pair (only used if not POD)
     */
    template<typename T1, typename T2, bool Ref = false>
    using pair = typename detail::pair_selector<T1, T2, Ref>::type;


    /**
     * @brief Creates a pair-like object optimized for structural binding.
     *
     * This function is designed for use in view pipelines (`zip`, `enumerate`, etc.)
     * where performance and reference semantics are important.
     *
     * If both `T1` and `T2` are POD-like, a `pod::pair` is returned by value.
     * Otherwise, returns `utils::ref_pair<T1, T2>` — a lightweight pair of references.
     *
     * @warning If any of the referenced values are temporary or non-owned, dangling references will occur.
     *          Only use this when both `a` and `b` are owned by the caller.
     *
     * @tparam T1 Type of first value (deduced by reference)
     * @tparam T2 Type of second value
     * @param a First element
     * @param b Second element
     * @return Either `pod::pair<A, B>` (by value) or `utils::ref_pair<T1, T2>` (by reference)
     */
    template<typename T1, typename T2>
    [[nodiscard]] constexpr auto make_pair(T1 a, T2 b) {
        return pair<T1, T2, true>{a, b}; // Pair of POD or references
    }

    /**
     * @brief Constructs a copy-owning pair-like object.
     *
     * This function guarantees that both elements are copied (not moved),
     * which is critical when move construction may alter the original structure.
     *
     * If both values are POD-like, returns a `pod::pair<A, B>`.
     * Otherwise, returns a `val_pair<A, B>`, which stores both values internally.
     *
     * @note This function always performs a copy. If move semantics are desired,
     *       use `make_val_pair()` instead.
     *
     * @warning Do not use this with temporaries unless copying is explicitly intended.
     *
     * @tparam T1 First value type (copy-constructible)
     * @tparam T2 Second value type (copy-constructible)
     * @param a First value
     * @param b Second value
     * @return Resolved to `jh::pod::pair<A, B>` or `jh::utils::val_pair<A, B>`, depending on POD-ness.
     */
    template<typename T1, typename T2>
        requires (std::is_copy_constructible_v<T1> && std::is_copy_constructible_v<T2>)
    [[nodiscard]] constexpr auto make_pair_cp(T1 &a, T2 &b) {
        static_assert(detail::is_safe_reference_source<T1> && detail::is_safe_reference_source<T2>,
                      "make_pair(): Avoid binding to temporary values");
        return pair<T1, T2>{a, b}; // Pair of values, copy constructed
    }


    /**
     * @brief Concept for types that behave like a `pair` with `first` and `second` members.
     *
     * A type is considered `pair_like` if:
     *   - It exposes `.first` and `.second` members
     *   - Both members are either copy-constructible or move-constructible
     *
     * This concept is used internally to constrain view pipelines or generic destructuring logic.
     *
     * @tparam T Type to be checked
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

    /// @brief Helper alias for `pair_like` concept
    template<typename T>
    constexpr bool is_pair_like_v = pair_like<T>;


} // namespace jh

/**
 * User-facing notes:
 *
 * - `make_pair()` is optimized for structural binding with `const auto& [a, b]`.
 * - POD types are returned by value (`pod::pair`), ensuring optimal storage.
 * - Non-POD types are returned as references (`ref_pair<T1, T2>`).
 * - For owned value-pairs, use `make_pair_cp()` or `make_val_pair()`.
 * - All results are compatible with structured bindings.
 * - Do not template-match the return types — treat them as opaque `pair`-like.
 */
