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
 * @file range_adaptor.h (ranges)
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 * @brief STL-compatible adapter for duck-typed sequences and iterators.
 *
 * @details
 * This header provides the bridge between <code>jh::concepts::sequence</code>
 * and <code>std::ranges::range</code>.
 * It introduces two key components:
 * <ul>
 *   <li><code>jh::ranges::detail::completed_iterator</code> &mdash; a behavioral
 *       iterator adaptor that completes any duck-typed iterator to full
 *       STL iterator semantics.</li>
 *   <li><code>jh::ranges::range_adaptor</code> &mdash; a lightweight view class
 *       that exposes a sequence as a <code>std::ranges::range</code>,
 *       using <code>completed_iterator</code> for <code>begin()</code>
 *       and directly forwarding <code>end()</code>.</li>
 * </ul>
 *
 * Together, they allow any object satisfying
 * <code>jh::concepts::sequence</code> to be seamlessly used in
 * <code>std::ranges</code> algorithms, even if it defines no iterator typedefs
 * or category tags.
 */
#pragma once

#include "jh/conceptual/iterator.h"
#include "jh/conceptual/range_traits.h"
#include <ranges>


namespace jh::ranges {
    namespace detail {
        /**
         * @brief Behavior-complete adaptor for any duck-typed iterator.
         *
         * @details
         * Inherits directly from the underlying iterator type and augments it
         * with full STL-compliant iterator behavior:
         * <ul>
         *   <li>Provides a guaranteed <code>difference_type</code> (defaults to <code>std::ptrdiff_t</code> if undeducible).</li>
         *   <li>Supplies all standard comparison and arithmetic operators where supported.</li>
         *   <li>Ensures compatibility with a corresponding sentinel type.</li>
         * </ul>
         *
         * Used internally by <code>jh::ranges::range_adaptor</code> to normalize
         * custom iterators for range-based operations.
         *
         * @tparam Inner   The underlying iterator type.
         * @tparam Sentinel The matching sentinel type (defaults to <code>Inner</code>).
         */
        template<typename Inner, typename Sentinel = Inner> requires jh::concepts::input_iterator<Inner, Sentinel>
        struct completed_iterator : public Inner {

            // ---- Input Basics ----
            using inner_type [[maybe_unused]] = Inner;
            using sentinel_type [[maybe_unused]] = Sentinel;

            using value_type = jh::concepts::iterator_value_t<Inner>;
            using reference = jh::concepts::iterator_reference_t<Inner>;
            using rvalue_reference [[maybe_unused]] = jh::concepts::iterator_rvalue_reference_t<Inner>;
            using difference_type = std::conditional_t<
                    std::is_void_v<jh::concepts::iterator_difference_t<Inner>>,
                    std::ptrdiff_t,
                    jh::concepts::iterator_difference_t<Inner>
            >;

            using iterator_concept =
                    decltype([] {
                        if constexpr (jh::concepts::random_access_iterator<Inner>)
                            return std::type_identity<std::random_access_iterator_tag>{};
                        else if constexpr (jh::concepts::bidirectional_iterator<Inner>)
                            return std::type_identity<std::bidirectional_iterator_tag>{};
                        else if constexpr (jh::concepts::forward_iterator<Inner>)
                            return std::type_identity<std::forward_iterator_tag>{};
                        else
                            return std::type_identity<std::input_iterator_tag>{};
                    }())::type;

            using iterator_category [[maybe_unused]] = iterator_concept;

            completed_iterator() = default;

            constexpr explicit completed_iterator(const Inner &it) : Inner(it) {}

            constexpr explicit completed_iterator(Inner &&it) noexcept(std::is_nothrow_move_constructible_v<Inner>)
                    : Inner(std::move(it)) {}

            // should always have it++
            completed_iterator &operator++() {
                Inner::operator++();
                return *this;
            }

            // only expose ++it if exists
            constexpr auto operator++(int)
            noexcept(noexcept(std::declval<Inner &>()++)) requires requires(Inner &it) { it++; } {
                auto tmp = *this;
                ++(*this);
                return tmp;
            }

            constexpr decltype(auto)
            operator*() noexcept(noexcept(*std::declval<Inner &>())) requires requires(Inner &it) { *it; } {
                return *static_cast<Inner &>(*this);
            }

            constexpr decltype(auto)
            operator*() const noexcept(noexcept(*std::declval<const Inner &>())) requires requires(
                    const Inner &it) { *it; } {
                return *static_cast<const Inner &>(*this);
            }

            // compare with sentinel
            friend constexpr bool operator==(const completed_iterator &a, const Sentinel &b)
            noexcept(noexcept(static_cast<const Inner &>(a) == b)) {
                return static_cast<const Inner &>(a) == b;
            }

            friend constexpr bool operator==(const Sentinel &a, const completed_iterator &b)
            noexcept(noexcept(a == static_cast<const Inner &>(b))) {
                return a == static_cast<const Inner &>(b);
            }

            friend constexpr bool operator!=(const completed_iterator &a, const Sentinel &b)
            noexcept(noexcept(!(a == b))) // NOLINT
            {
                return !(a == b); // NOLINT
            }

            friend constexpr bool operator!=(const Sentinel &a, const completed_iterator &b)
            noexcept(noexcept(!(a == b)))  // NOLINT
            {
                return !(a == b); // NOLINT
            }

            // ---- Self comparison (iterator vs iterator) ----
            friend constexpr bool operator==(const completed_iterator &a, const completed_iterator &b)
            noexcept(noexcept(static_cast<const Inner &>(a) == static_cast<const Inner &>(b))) requires requires(
                    const Inner &x, const Inner &y) { x == y; } {
                return static_cast<const Inner &>(a) == static_cast<const Inner &>(b);
            }

            friend constexpr bool operator!=(const completed_iterator &a, const completed_iterator &b)
            noexcept(noexcept(!(a == b))) // NOLINT
            requires requires(const Inner &x, const Inner &y) { x == y; } {
                return !(a == b); // NOLINT
            }

            // move
            constexpr completed_iterator(
                    completed_iterator &&) noexcept(std::is_nothrow_move_constructible_v<Inner>) = default;

            constexpr completed_iterator &
            operator=(completed_iterator &&) noexcept(std::is_nothrow_move_assignable_v<Inner>) = default;

            // ---- Forward Addition ----

            // copy
            constexpr completed_iterator(const completed_iterator &) requires std::copy_constructible<Inner> = default;

            constexpr completed_iterator &operator=(const completed_iterator &) requires std::copyable<Inner> = default;

            // ---- Bidirectional Addition ----
            constexpr completed_iterator &operator--()
            noexcept(noexcept(--std::declval<Inner &>())) requires requires(Inner &it) { --it; } {
                Inner::operator--();
                return *this;
            }

            constexpr completed_iterator operator--(int)
            noexcept(noexcept(std::declval<Inner &>()--)) requires requires(Inner &it) { it--; } {
                auto tmp = *this;
                --(*this);
                return tmp;
            }
            // ---- Random Access Addition ----

            // forward N
            constexpr completed_iterator &operator+=(difference_type n)
            noexcept(noexcept(std::declval<Inner &>() += n)) requires requires(Inner &it,
                                                                               difference_type n)
            { it += n; } {
                Inner::operator+=(n);
                return *this;
            }

            // backwards N
            constexpr completed_iterator &operator-=(difference_type n)
            noexcept(noexcept(std::declval<Inner &>() -= n)) requires requires(Inner &it,
                                                                               difference_type n)
            { it -= n; } {
                Inner::operator-=(n);
                return *this;
            }

            // return new ite (forward N)
            constexpr completed_iterator operator+(difference_type n) const
            noexcept(noexcept(std::declval<const Inner &>() + n)) requires requires(const Inner &it, difference_type n)
            {
                it + n;
            } {
                return completed_iterator(static_cast<const Inner &>(*this) + n);
            }

            // return new ite (backwards N)
            constexpr completed_iterator operator-(difference_type n) const
            noexcept(noexcept(std::declval<const Inner &>() - n)) requires requires(const Inner &it,
                                                                                    difference_type n) {
                it - n;
            } {
                return completed_iterator(static_cast<const Inner &>(*this) - n);
            }

            // difference
            friend constexpr difference_type operator-(const completed_iterator &a,
                                                       const completed_iterator &b)
            noexcept(noexcept(static_cast<const Inner &>(a) - static_cast<const Inner &>(b))) requires requires(
                    const Inner &x,
                    const Inner &y
            ) { x - y; } {
                return static_cast<const Inner &>(a) - static_cast<const Inner &>(b);
            }

            // index access
            constexpr decltype(auto) operator[](difference_type n) const
            noexcept(noexcept(std::declval<const Inner &>()[n])) requires requires(const Inner &it,
                                                                                   difference_type n) { it[n]; } {
                return static_cast<const Inner &>(*this)[n];
            }

            // compare operators
            friend constexpr bool operator<(const completed_iterator &a, const completed_iterator &b)
            noexcept(noexcept(static_cast<const Inner &>(a) < static_cast<const Inner &>(b))) requires requires(
                    const Inner &x,
                    const Inner &y
            ) { x < y; } {
                return static_cast<const Inner &>(a) < static_cast<const Inner &>(b);
            }

            friend constexpr bool operator>(const completed_iterator &a, const completed_iterator &b)
            noexcept(noexcept(static_cast<const Inner &>(a) > static_cast<const Inner &>(b))) requires requires(
                    const Inner &x,
                    const Inner &y
            ) { x > y; } {
                return static_cast<const Inner &>(a) > static_cast<const Inner &>(b);
            }

            friend constexpr bool operator<=(const completed_iterator &a, const completed_iterator &b)
            noexcept(noexcept(static_cast<const Inner &>(a) <= static_cast<const Inner &>(b))) requires requires(
                    const Inner &x,
                    const Inner &y
            ) { x <= y; } {
                return static_cast<const Inner &>(a) <= static_cast<const Inner &>(b);
            }

            friend constexpr bool operator>=(const completed_iterator &a, const completed_iterator &b)
            noexcept(noexcept(static_cast<const Inner &>(a) >= static_cast<const Inner &>(b))) requires requires(
                    const Inner &x,
                    const Inner &y
            ) { x >= y; } {
                return static_cast<const Inner &>(a) >= static_cast<const Inner &>(b);
            }

            // n + ite
            friend constexpr completed_iterator operator+(difference_type n, const completed_iterator &it)
            noexcept(noexcept(n + static_cast<const Inner &>(it))) requires requires(const Inner &i,
                                                                                     difference_type n)
            {
                n + i;
            } {
                return completed_iterator(n + static_cast<const Inner &>(it));
            }
            // ---- Sentinel difference support ----

            // completed_iterator - Sentinel
            friend constexpr difference_type operator-(const completed_iterator &a, const Sentinel &b)
            noexcept(noexcept(static_cast<const Inner &>(a) - b)) requires requires(
                    const Inner &x,
                    const Sentinel &y
            ) { x - y; } {
                return static_cast<const Inner &>(a) - b;
            }

            // Sentinel - completed_iterator
            friend constexpr difference_type operator-(const Sentinel &a, const completed_iterator &b)
            noexcept(noexcept(a - static_cast<const Inner &>(b))) requires requires(
                    const Sentinel &x,
                    const Inner &y
            ) { x - y; } {
                return a - static_cast<const Inner &>(b);
            }
        };
    } // namespace detail

    /**
     * @brief Lightweight adapter that exposes any sequence as a standard range.
     *
     * @details
     * Wraps a duck-typed sequence so that it models <code>std::ranges::range</code>.
     * <ul>
     *   <li><code>begin()</code> returns a <code>completed_iterator</code> wrapping the sequence's iterator.</li>
     *   <li><code>end()</code> is forwarded directly from the underlying sequence.</li>
     * </ul>
     *
     * Enables generic algorithms and range-based for loops to work with any
     * object satisfying <code>jh::concepts::sequence</code>, regardless of whether
     * it defines STL-compatible iterator traits.
     *
     * @tparam Seq Sequence-like type satisfying <code>jh::concepts::sequence</code>.
     *
     * @note
     * If the sequence's <code>begin()</code> and <code>end()</code> types are identical,
     * the adaptor transparently returns a <code>completed_iterator</code> for both.
     * This allows the resulting view to model <code>std::ranges::common_range</code>,
     * enabling tighter interoperability with standard range algorithms.
     */
    template<typename Seq>
    class range_adaptor : public std::ranges::view_interface<range_adaptor<Seq>> {
        using traits = jh::concepts::range_storage_traits<Seq, true>;
        typename traits::stored_t seq_;
    public:

        using inner_iterator = decltype(std::declval<Seq &>().begin());
        using sentinel = decltype(std::declval<Seq &>().end());
        using iterator = detail::completed_iterator<inner_iterator, sentinel>;

        explicit range_adaptor(Seq &&s)
                : seq_(traits::wrap(std::forward<Seq>(s))) {}

        auto begin() noexcept(noexcept(get().begin())) { return iterator(get().begin()); }

        auto end() noexcept(noexcept(get().end())) {
            if constexpr (std::same_as<inner_iterator, sentinel>)
                return iterator(get().end());
            else
                return get().end();
        }

    private:

        constexpr decltype(auto) get() noexcept { return traits::get(seq_); }

        constexpr decltype(auto) get() const noexcept { return traits::get(seq_); }

    };
} // namespace jh::ranges


namespace std::ranges {
    /**
     * @brief Legal specialization of <code>std::ranges::enable_borrowed_range</code>.
     *
     * <p>
     * This specialization is <b>explicitly permitted</b> by the C++ standard.
     * User code may provide template specializations for certain customization points
     * declared under the <code>std::ranges</code> namespace, including
     * <code>std::ranges::enable_borrowed_range</code>, to indicate that a custom range
     * type models the borrowed-range property.
     * </p>
     *
     * <h4>About Clang-Tidy false positives</h4>
     * <p>
     * Static analyzers such as <b>Clang-Tidy</b> may emit a warning:
     * <em>"Modification of 'std' namespace can result in undefined behavior"</em>.
     * This warning is triggered because Clang-Tidy heuristically treats
     * <b>any</b> declaration inside a user-written <code>namespace std</code> block
     * as a modification of the standard namespace, without recognizing that
     * <em>nested namespaces</em> like <code>std::ranges</code> define a separate,
     * standards-sanctioned customization domain.
     * </p>
     *
     * <p>
     * Concretely:
     * </p>
     * <ul>
     *   <li>Specializations such as <code>std::tuple_element&lt;&gt;</code> are recognized
     *       by Clang-Tidy as explicitly allowed and thus do <b>not</b> trigger the warning.</li>
     *   <li>Specializations inside nested subnamespaces (e.g.
     *       <code>std::ranges::enable_borrowed_range&lt;T&gt;</code>) are equally legal
     *       under the standard, but Clang-Tidy does not check the whitelist at that depth
     *       and therefore incorrectly flags them as potential UB.</li>
     * </ul>
     *
     * <p>
     * This is a <b>false positive</b> &mdash; the specialization is <em>fully standard-compliant</em>
     * and does <b>not</b> constitute undefined behavior.
     * </p>
     *
     * @see <a href="https://en.cppreference.com/w/cpp/ranges/borrowed_range.html">
     * std::ranges::borrowed_range</a>
     */
    template<typename SeqType>
    [[maybe_unused]] [[maybe_unused]] inline constexpr bool enable_borrowed_range<jh::ranges::range_adaptor<SeqType>> =
            std::is_lvalue_reference_v<SeqType>;
} // namespace std::ranges
