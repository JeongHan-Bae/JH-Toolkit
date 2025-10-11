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
 * @file sequence.h
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 * @brief Concept and utilities for immutable sequence detection in C++20.
 *
 * @details
 * The <code>jh::sequence</code> module defines a lightweight C++20 concept
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
 *   <li>Integrate naturally with <code>jh::iterator_t</code> and iterator concepts.</li>
 * </ul>
 *
 * <h3>Key Components</h3>
 * <ul>
 *   <li><code>jh::sequence</code> — Concept ensuring immutable iteration.</li>
 *   <li><code>jh::sequence_value_type&lt;T&gt;</code> — Extracts element type via <code>iterator_t</code>.</li>
 *   <li><code>jh::is_sequence&lt;T&gt;</code> — Compile-time boolean for detection.</li>
 *   <li><code>jh::to_range()</code> — Converts any sequence into a standard <code>std::ranges::subrange</code>.</li>
 * </ul>
 *
 * <h3>Design Notes</h3>
 * <ul>
 *   <li>Type deduction uses <code>jh::iterator_t&lt;T&gt;</code>, providing
 *       a unified meta-interface for STL and custom containers.</li>
 *   <li>All qualifiers (<code>const</code>, <code>volatile</code>, references)
 *       are removed before deduction to ensure consistent behavior.</li>
 *   <li>Concept validation is purely structural — no explicit typedefs are required.</li>
 *   <li>Serves as a foundational layer for <code>jh::range</code> and coroutine containers.</li>
 * </ul>
 *
 * @version <pre>1.3.x</pre>
 * @date <pre>2025<pre>
 */

#pragma once

#include <iterator>    // for "iterator_tag"s
#include <ranges>
#include <type_traits>
#include "jh/iterator.h"

namespace jh {
    /**
     * @brief Concept that checks whether a type behaves as an immutable sequence.
     *
     * @details
     * A type <code>T</code> satisfies <code>jh::sequence</code> if:
     * <ol>
     *   <li>It provides <code>begin()</code> and <code>end()</code> returning valid iterators.</li>
     *   <li>The iterators satisfy <code>jh::input_iterator</code>.</li>
     *   <li>Its <code>begin()</code> and <code>end()</code> can be compared for equality.</li>
     * </ol>
     *
     * This concept is intentionally minimal — it does not enforce mutability,
     * range-based compatibility, or category guarantees beyond input-iterable semantics.
     *
     * @tparam T The candidate type.
     */
    template<typename T>
    concept sequence = requires(const T& t)
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
         * <code>jh::iterator_t&lt;T&gt;</code> for unified iterator deduction.
         */
        template<typename T>
        struct sequence_value_type_impl {
            using raw_type = std::remove_cvref_t<T>;
            using iterator_type = jh::iterator_t<raw_type>;
            using type = typename jh::iterator_value_t<iterator_type>;
        };
    }

    /**
     * @brief Extracts the element type of a sequence.
     * @details
     * Resolves the <code>value_type</code> through <code>jh::iterator_t&lt;T&gt;</code>.
     * Cleanses cv-ref qualifiers for consistent deduction.
     *
     * @tparam T The sequence type.
     */
    template<typename T>
    using sequence_value_type = typename detail::sequence_value_type_impl<T>::type;

    /**
     * @brief Compile-time check for sequence compliance.
     * @details
     * Equivalent to <code>jh::sequence&lt;T&gt;</code>, provided as a
     * <strong>boolean constant</strong> for generic metaprogramming.
     *
     * @tparam T The type to check.
     */
    template<typename T>
    constexpr bool is_sequence = sequence<T>;
    namespace detail{

        template<typename Inner, typename Sentinel = Inner>
        requires jh::input_iterator<Inner>
        struct completed_iterator : public Inner {

            // ---- Input Basics ----
            using inner_type = Inner;
            using sentinel_type = Sentinel;

            using value_type        = jh::iterator_value_t<Inner>;
            using reference         = jh::iterator_reference_t<Inner>;
            using rvalue_reference  = jh::iterator_rvalue_reference_t<Inner>;
            using difference_type   = std::ptrdiff_t;

            using iterator_category =
                    std::conditional_t<
                            jh::random_access_iterator<Inner>, std::random_access_iterator_tag,
                            std::conditional_t<
                                    jh::bidirectional_iterator<Inner>, std::bidirectional_iterator_tag,
                                    std::conditional_t<
                                            jh::forward_iterator<Inner>, std::forward_iterator_tag,
                                             std::input_iterator_tag>>>;

            completed_iterator() = default;
            constexpr explicit completed_iterator(const Inner& it) : Inner(it) {}
            constexpr explicit completed_iterator(Inner&& it) noexcept(std::is_nothrow_move_constructible_v<Inner>)
                    : Inner(std::move(it)) {}

            // should always have it++
            completed_iterator& operator++() {
                Inner::operator++();
                return *this;
            }

            // only expose ++it if exists
            constexpr auto operator++(int)
            noexcept(noexcept(std::declval<Inner&>()++))
            requires requires(Inner& it) { it++; }
            {
                auto tmp = *this;
                ++(*this);
                return tmp;
            }

            constexpr decltype(auto) operator*() noexcept(noexcept(*std::declval<Inner&>()))
            requires requires(Inner& it) { *it; }
            {
                return *static_cast<Inner&>(*this);
            }

            constexpr decltype(auto) operator*() const noexcept(noexcept(*std::declval<const Inner&>()))
            requires requires(const Inner& it) { *it; }
            {
                return *static_cast<const Inner&>(*this);
            }

            // compare with sentinel
            friend constexpr bool operator==(const completed_iterator& a, const Sentinel& b)
            noexcept(noexcept(static_cast<const Inner&>(a) == b)) {
                return static_cast<const Inner&>(a) == b;
            }
            friend constexpr bool operator==(const Sentinel& a, const completed_iterator& b)
            noexcept(noexcept(a == static_cast<const Inner&>(b))) {
                return a == static_cast<const Inner&>(b);
            }
            friend constexpr bool operator!=(const completed_iterator& a, const Sentinel& b)
            noexcept(noexcept(!(a == b))) {
                return !(a == b);
            }
            friend constexpr bool operator!=(const Sentinel& a, const completed_iterator& b)
            noexcept(noexcept(!(a == b))) {
                return !(a == b);
            }
            // move
            constexpr completed_iterator(completed_iterator&&) noexcept(std::is_nothrow_move_constructible_v<Inner>) = default;
            constexpr completed_iterator& operator=(completed_iterator&&) noexcept(std::is_nothrow_move_assignable_v<Inner>) = default;

            // ---- Forward Addition ----

            // copy
            constexpr completed_iterator(const completed_iterator&) requires std::copy_constructible<Inner> = default;
            constexpr completed_iterator& operator=(const completed_iterator&) requires std::copyable<Inner> = default;

            // ---- Bidirectional Addition ----
            constexpr completed_iterator& operator--()
            noexcept(noexcept(--std::declval<Inner&>()))
            requires requires(Inner& it) { --it; }
            {
                Inner::operator--();
                return *this;
            }

            constexpr completed_iterator operator--(int)
            noexcept(noexcept(std::declval<Inner&>()--))
            requires requires(Inner& it) { it--; }
            {
                auto tmp = *this;
                --(*this);
                return tmp;
            }
            // ---- Random Access Addition ----

            // forward N
            constexpr completed_iterator& operator+=(difference_type n)
            noexcept(noexcept(std::declval<Inner&>() += n))
            requires requires(Inner& it, difference_type n) { it += n; }
            {
                Inner::operator+=(n);
                return *this;
            }

            // backwards N
            constexpr completed_iterator& operator-=(difference_type n)
            noexcept(noexcept(std::declval<Inner&>() -= n))
            requires requires(Inner& it, difference_type n) { it -= n; }
            {
                Inner::operator-=(n);
                return *this;
            }

            // return new ite (forward N)
            constexpr completed_iterator operator+(difference_type n) const
            noexcept(noexcept(std::declval<const Inner&>() + n))
            requires requires(const Inner& it, difference_type n) { it + n; }
            {
                return completed_iterator(static_cast<const Inner&>(*this) + n);
            }
            // return new ite (backwards N)
            constexpr completed_iterator operator-(difference_type n) const
            noexcept(noexcept(std::declval<const Inner&>() - n))
            requires requires(const Inner& it, difference_type n) { it - n; }
            {
                return completed_iterator(static_cast<const Inner&>(*this) - n);
            }

            // difference
            friend constexpr difference_type operator-(const completed_iterator& a,
                                                       const completed_iterator& b)
            noexcept(noexcept(static_cast<const Inner&>(a) - static_cast<const Inner&>(b)))
            requires requires(const Inner& x, const Inner& y) { x - y; }
            {
                return static_cast<const Inner&>(a) - static_cast<const Inner&>(b);
            }

            // index access
            constexpr decltype(auto) operator[](difference_type n) const
            noexcept(noexcept(std::declval<const Inner&>()[n]))
            requires requires(const Inner& it, difference_type n) { it[n]; }
            {
                return static_cast<const Inner&>(*this)[n];
            }

            // compare operators
            friend constexpr bool operator<(const completed_iterator& a, const completed_iterator& b)
            noexcept(noexcept(static_cast<const Inner&>(a) < static_cast<const Inner&>(b)))
            requires requires(const Inner& x, const Inner& y) { x < y; }
            {
                return static_cast<const Inner&>(a) < static_cast<const Inner&>(b);
            }

            friend constexpr bool operator>(const completed_iterator& a, const completed_iterator& b)
            noexcept(noexcept(static_cast<const Inner&>(a) > static_cast<const Inner&>(b)))
            requires requires(const Inner& x, const Inner& y) { x > y; }
            {
                return static_cast<const Inner&>(a) > static_cast<const Inner&>(b);
            }

            friend constexpr bool operator<=(const completed_iterator& a, const completed_iterator& b)
            noexcept(noexcept(static_cast<const Inner&>(a) <= static_cast<const Inner&>(b)))
            requires requires(const Inner& x, const Inner& y) { x <= y; }
            {
                return static_cast<const Inner&>(a) <= static_cast<const Inner&>(b);
            }

            friend constexpr bool operator>=(const completed_iterator& a, const completed_iterator& b)
            noexcept(noexcept(static_cast<const Inner&>(a) >= static_cast<const Inner&>(b)))
            requires requires(const Inner& x, const Inner& y) { x >= y; }
            {
                return static_cast<const Inner&>(a) >= static_cast<const Inner&>(b);
            }

            // n + ite
            friend constexpr completed_iterator operator+(difference_type n, const completed_iterator& it)
            noexcept(noexcept(n + static_cast<const Inner&>(it)))
            requires requires(const Inner& i, difference_type n) { n + i; }
            {
                return completed_iterator(n + static_cast<const Inner&>(it));
            }
            // ---- Sentinel difference support ----

            // completed_iterator - Sentinel
            friend constexpr difference_type operator-(const completed_iterator& a, const Sentinel& b)
            noexcept(noexcept(static_cast<const Inner&>(a) - b))
            requires requires(const Inner& x, const Sentinel& y) { x - y; }
            {
                return static_cast<const Inner&>(a) - b;
            }

            // Sentinel - completed_iterator
            friend constexpr difference_type operator-(const Sentinel& a, const completed_iterator& b)
            noexcept(noexcept(a - static_cast<const Inner&>(b)))
            requires requires(const Sentinel& x, const Inner& y) { x - y; }
            {
                return a - static_cast<const Inner&>(b);
            }
        };
    }

    template<typename Seq>
    class range_wrapper: public std::ranges::view_interface<range_wrapper<Seq>> {
        using Stored = std::conditional_t<
                std::is_reference_v<Seq>,
                std::reference_wrapper<std::remove_reference_t<Seq>>,
                Seq>;
        Stored seq_;
    public:

        using inner_iterator = decltype(std::declval<Seq&>().begin());
        using sentinel = decltype(std::declval<Seq&>().end());
        using iterator = detail::completed_iterator<inner_iterator, sentinel>;

        explicit range_wrapper(Seq&& s)
                : seq_(wrap(std::forward<Seq>(s))) {}

        auto begin() noexcept(noexcept(get().begin())) { return iterator(get().begin()); }
        auto end() noexcept(noexcept(get().end())) { return get().end(); }

    private:
        static auto wrap(auto&& v) {
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

    template<typename Seq>
    range_wrapper(Seq&&) -> range_wrapper<Seq>;

    namespace detail {
        /**
         * @brief Converts a sequence into a range-compatible form.
         * @details
         * If the sequence already satisfies <code>std::ranges::range</code>,
         * this operation is <b>idempotent</b> and returns it unchanged.
         * Otherwise, it wraps the sequence as a <code>std::ranges::subrange</code>.
         *
         * @tparam Seq The sequence type satisfying <code>jh::sequence</code>.
         * @param s The input sequence.
         * @return Either the same range (if already a range) or a <code>std::ranges::subrange</code>.
         */
        template<sequence Seq>
        struct to_range_traits {
            static decltype(auto) convert(Seq&& s) {  // auto -> decltype(auto)
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
                    }
                    else if constexpr (std::is_lvalue_reference_v<Seq>) {
                        // copyable left value return by ref
                        return (s);
                    } else {
                        // right value, forward
                        return std::forward<Seq>(s);
                    }
                }  else {
                    if constexpr (std::is_lvalue_reference_v<Seq>) {
                        // by ref
                        return range_wrapper<Seq>(s);
                    } else {
                        // if a value or a right value provided, move it
                        return range_wrapper<S>(std::move(s));
                    }
                }
            }
        };
    }

    /**
     * @brief Converts a sequence into a standard range object.
     * @details
     * Idempotent:
     * if the input already models <code>std::ranges::range</code>,
     * it is perfectly forwarded and no transformation occurs;
     * otherwise, it is wrapped into a <code>std::ranges::subrange</code>.
     *
     * <p>
     * The function accepts both mutable and immutable sequences.
     * A type only needs to provide <code>const</code> iteration
     * to satisfy <code>jh::sequence</code>.
     * If the sequence supports only <code>const</code> iteration,
     * the resulting range will naturally be <code>const</code>-qualified.
     * </p>
     *
     * @tparam Seq The sequence type satisfying <code>jh::sequence</code>.
     * @param s The input sequence.
     * @return Either the perfectly forwarded input or a <code>std::ranges::subrange</code>.
     */
    template<sequence Seq>
    decltype(auto) to_range(Seq&& s) {
        return detail::to_range_traits<Seq>::convert(std::forward<Seq>(s));
    }


    template <typename Seq>
    using sequence_difference_t =
            std::ranges::range_difference_t<decltype(jh::to_range(std::declval<Seq&>()))>;

} // namespace jh
