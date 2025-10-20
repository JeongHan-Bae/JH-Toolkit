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
 * @file range_wrapper.h (ranges)
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 * @brief STL-compatible adapter for duck-typed sequences and iterators.
 *
 * @details
 * This header provides the bridge between <code>jh::concepts::sequence</code>
 * and <code>std::ranges::range</code>.
 * It introduces two key components:
 * <ul>
 *   <li><code>jh::ranges::detail::completed_iterator</code> — a behavioral
 *       iterator adaptor that completes any duck-typed iterator to full
 *       STL iterator semantics.</li>
 *   <li><code>jh::ranges::range_wrapper</code> — a lightweight view class
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
         * Used internally by <code>jh::ranges::range_wrapper</code> to normalize
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

            using iterator_category [[maybe_unused]] =
                    std::conditional_t<
                            jh::concepts::random_access_iterator<Inner>, std::random_access_iterator_tag,
                            std::conditional_t<
                                    jh::concepts::bidirectional_iterator<Inner>, std::bidirectional_iterator_tag,
                                    std::conditional_t<
                                            jh::concepts::forward_iterator<Inner>, std::forward_iterator_tag,
                                            std::input_iterator_tag>>>;

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
            noexcept(noexcept(std::declval<Inner &>()++))requires requires(Inner &it) { it++; } {
                auto tmp = *this;
                ++(*this);
                return tmp;
            }

            constexpr decltype(auto)
            operator*() noexcept(noexcept(*std::declval<Inner &>()))requires requires(Inner &it) { *it; } {
                return *static_cast<Inner &>(*this);
            }

            constexpr decltype(auto)
            operator*() const noexcept(noexcept(*std::declval<const Inner &>()))requires requires(
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
            noexcept(noexcept(!(a == b))) {
                return !(a == b);
            }

            friend constexpr bool operator!=(const Sentinel &a, const completed_iterator &b)
            noexcept(noexcept(!(a == b))) {
                return !(a == b);
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
            noexcept(noexcept(--std::declval<Inner &>()))requires requires(Inner &it) { --it; } {
                Inner::operator--();
                return *this;
            }

            constexpr completed_iterator operator--(int)
            noexcept(noexcept(std::declval<Inner &>()--))requires requires(Inner &it) { it--; } {
                auto tmp = *this;
                --(*this);
                return tmp;
            }
            // ---- Random Access Addition ----

            // forward N
            constexpr completed_iterator &operator+=(difference_type n)
            noexcept(noexcept(std::declval<Inner &>() += n))requires requires(Inner &it,
                                                                              difference_type n) { it += n; } {
                Inner::operator+=(n);
                return *this;
            }

            // backwards N
            constexpr completed_iterator &operator-=(difference_type n)
            noexcept(noexcept(std::declval<Inner &>() -= n))requires requires(Inner &it,
                                                                              difference_type n) { it -= n; } {
                Inner::operator-=(n);
                return *this;
            }

            // return new ite (forward N)
            constexpr completed_iterator operator+(difference_type n) const
            noexcept(noexcept(std::declval<const Inner &>() + n))requires requires(const Inner &it, difference_type n) {
                it + n;
            } {
                return completed_iterator(static_cast<const Inner &>(*this) + n);
            }

            // return new ite (backwards N)
            constexpr completed_iterator operator-(difference_type n) const
            noexcept(noexcept(std::declval<const Inner &>() - n))requires requires(const Inner &it, difference_type n) {
                it - n;
            } {
                return completed_iterator(static_cast<const Inner &>(*this) - n);
            }

            // difference
            friend constexpr difference_type operator-(const completed_iterator &a,
                                                       const completed_iterator &b)
            noexcept(noexcept(static_cast<const Inner &>(a) - static_cast<const Inner &>(b)))requires requires(
                    const Inner &x,
                    const Inner &y
            ) { x - y; } {
                return static_cast<const Inner &>(a) - static_cast<const Inner &>(b);
            }

            // index access
            constexpr decltype(auto) operator[](difference_type n) const
            noexcept(noexcept(std::declval<const Inner &>()[n]))requires requires(const Inner &it,
                                                                                  difference_type n) { it[n]; } {
                return static_cast<const Inner &>(*this)[n];
            }

            // compare operators
            friend constexpr bool operator<(const completed_iterator &a, const completed_iterator &b)
            noexcept(noexcept(static_cast<const Inner &>(a) < static_cast<const Inner &>(b)))requires requires(
                    const Inner &x,
                    const Inner &y
            ) { x < y; } {
                return static_cast<const Inner &>(a) < static_cast<const Inner &>(b);
            }

            friend constexpr bool operator>(const completed_iterator &a, const completed_iterator &b)
            noexcept(noexcept(static_cast<const Inner &>(a) > static_cast<const Inner &>(b)))requires requires(
                    const Inner &x,
                    const Inner &y
            ) { x > y; } {
                return static_cast<const Inner &>(a) > static_cast<const Inner &>(b);
            }

            friend constexpr bool operator<=(const completed_iterator &a, const completed_iterator &b)
            noexcept(noexcept(static_cast<const Inner &>(a) <= static_cast<const Inner &>(b)))requires requires(
                    const Inner &x,
                    const Inner &y
            ) { x <= y; } {
                return static_cast<const Inner &>(a) <= static_cast<const Inner &>(b);
            }

            friend constexpr bool operator>=(const completed_iterator &a, const completed_iterator &b)
            noexcept(noexcept(static_cast<const Inner &>(a) >= static_cast<const Inner &>(b)))requires requires(
                    const Inner &x,
                    const Inner &y
            ) { x >= y; } {
                return static_cast<const Inner &>(a) >= static_cast<const Inner &>(b);
            }

            // n + ite
            friend constexpr completed_iterator operator+(difference_type n, const completed_iterator &it)
            noexcept(noexcept(n + static_cast<const Inner &>(it)))requires requires(const Inner &i, difference_type n) {
                n + i;
            } {
                return completed_iterator(n + static_cast<const Inner &>(it));
            }
            // ---- Sentinel difference support ----

            // completed_iterator - Sentinel
            friend constexpr difference_type operator-(const completed_iterator &a, const Sentinel &b)
            noexcept(noexcept(static_cast<const Inner &>(a) - b))requires requires(
                    const Inner &x,
                    const Sentinel &y
            ) { x - y; } {
                return static_cast<const Inner &>(a) - b;
            }

            // Sentinel - completed_iterator
            friend constexpr difference_type operator-(const Sentinel &a, const completed_iterator &b)
            noexcept(noexcept(a - static_cast<const Inner &>(b)))requires requires(
                    const Sentinel &x,
                    const Inner &y
            ) { x - y; } {
                return a - static_cast<const Inner &>(b);
            }
        };
    }

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
     */
    template<typename Seq>
    class range_wrapper : public std::ranges::view_interface<range_wrapper<Seq>> {
        using Stored = std::conditional_t<
                std::is_reference_v<Seq>,
                std::reference_wrapper<std::remove_reference_t<Seq>>,
                Seq>;
        Stored seq_;
    public:

        using inner_iterator = decltype(std::declval<Seq &>().begin());
        using sentinel = decltype(std::declval<Seq &>().end());
        using iterator = detail::completed_iterator<inner_iterator, sentinel>;

        explicit range_wrapper(Seq &&s)
                : seq_(wrap(std::forward<Seq>(s))) {}

        auto begin() noexcept(noexcept(get().begin())) { return iterator(get().begin()); }

        auto end() noexcept(noexcept(get().end())) { return get().end(); }

    private:
        static auto wrap(auto &&v) {
            if constexpr (std::is_reference_v<decltype(v)>)
                return std::ref(v);
            else
                return std::forward<decltype(v)>(v);
        }

        constexpr decltype(auto) get() noexcept {
            if constexpr (std::is_reference_v<Seq>)
                return seq_.get();
            else
                return (seq_);
        }

        constexpr decltype(auto) get() const noexcept {
            if constexpr (std::is_reference_v<Seq>)
                return seq_.get();
            else
                return (seq_);
        }
    };
}