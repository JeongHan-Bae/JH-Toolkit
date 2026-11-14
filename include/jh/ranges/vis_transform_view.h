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
 * @file vis_transform_view.h
 * @brief Implementation of the <code>jh::ranges::vis_transform_view</code> class &mdash;
 *        a non-consuming, observation-oriented transformation view.
 * @author
 *   JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 *
 * <p>
 * The <code>jh::ranges::vis_transform_view</code> represents the underlying mechanism
 * used by <code>jh::ranges::views::vis_transform</code>.
 * It applies a callable over a range under an observational contract.
 * Repeatability requires a reentrant source and a callable that leaves the
 * observed state unchanged.
 * </p>
 *
 * <h3>Core semantics</h3>
 * <ul>
 *   <li>Transforms elements through an observational projection (the callable
 *       must return a non-void value).</li>
 *   <li>Supports repeatable observation when the source and callable uphold
 *       the contract.</li>
 *   <li>Implements <code>std::ranges::view_interface</code> for composability.</li>
 *   <li>Acts as a lightweight, fully constexpr-compatible wrapper.</li>
 * </ul>
 *
 * <p>
 * This class template is instantiated only when
 * <code>jh::concepts::vis_function_for&lt;F, R&gt;</code> is satisfied.
 * That concept checks const-accessible iteration, const invocation, and a
 * non-void result. It cannot establish runtime purity or repeatability.
 * </p>
 *
 * <p>
 * The standard transform view does not promise reentrant transformed
 * observations. This view makes that JH contract explicit for callers.
 * </p>
 *
 * @see jh::ranges::views::vis_transform
 * @see jh::concepts::vis_function_for
 * @see std::views::transform
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */


#pragma once

#include "jh/conceptual/range_traits.h"
#include <utility>

namespace jh::ranges {

    /**
     * @brief A non-consuming transform view preserving reentrancy.
     *
     * @tparam R The underlying range type.
     * @tparam F The callable transformation type.
     *
     * <p>
     * This view wraps a base range and lazily applies a callable to each element.
     * The resulting observations can be repeated when the base range is
     * reentrant and the callable preserves observed state.
     * Iterators retain a pointer to this view, so the view must outlive them.
     * </p>
     */
    template<std::ranges::range R, typename F> requires jh::concepts::vis_function_for<F, R>
    class vis_transform_view final : public std::ranges::view_interface<vis_transform_view<R, F>> {
    public:
        using traits = jh::concepts::range_storage_traits<R, false>;

        [[no_unique_address]] typename traits::stored_t base_;
        [[no_unique_address]] F func_;

        /**
         * @brief Construct a <code>vis_transform_view</code> over a base range and callable.
         *
         * @param base The input range.
         * @param func The transformation function.
         */
        vis_transform_view(R &&base, F func)
                : base_(traits::wrap(std::forward<R>(base))), func_(std::move(func)) {}

        /**
         * @brief Iterator type implementing the transformation projection.
         *
         * <p>
         * Each dereference applies the stored function to the corresponding
         * element of the base iterator.
         * Iterator category and operations mirror those of the base iterator.
         * </p>
         */
        struct iterator {
            using Base = const std::remove_reference_t<R>;
            using base_iterator = std::ranges::iterator_t<Base>;

            base_iterator current_;
            const vis_transform_view *parent_ = nullptr;

            using iterator_concept = decltype([] {
                if constexpr (std::ranges::random_access_range<Base>)
                    return std::type_identity<std::random_access_iterator_tag>{};
                else if constexpr (std::ranges::bidirectional_range<Base>)
                    return std::type_identity<std::bidirectional_iterator_tag>{};
                else if constexpr (std::ranges::forward_range<Base>)
                    return std::type_identity<std::forward_iterator_tag>{};
                else
                    return std::type_identity<std::input_iterator_tag>{};
            }())::type;

            using iterator_category [[maybe_unused]] = iterator_concept;

            using difference_type = std::ranges::range_difference_t<Base>;
            using value_type = std::remove_cvref_t<
                    std::invoke_result_t<const F &, std::ranges::range_reference_t<Base>>>;

            iterator() = default;

            /// @brief Creates an iterator bound to this view and a base iterator.
            constexpr iterator(const vis_transform_view *p, base_iterator it)
                    : current_(it), parent_(p) {}

            /**
             * @brief Apply the transformation function on dereference.
             */
            constexpr decltype(auto) operator*() const
            noexcept(noexcept(std::invoke(std::declval<const F &>(), *std::declval<base_iterator &>()))) {
                return std::invoke(parent_->func_, *current_);
            }

            /// @brief Advances the underlying iterator and returns this iterator.
            constexpr iterator &operator++() {
                ++current_;
                return *this;
            }

            /// @brief Advances the underlying iterator and returns the prior iterator.
            constexpr iterator operator++(int)
            noexcept(noexcept(std::declval<base_iterator &>()++)) requires requires(base_iterator it) { it++; } {
                auto tmp = *this;
                ++current_;
                return tmp;
            }

            /// @brief Compares underlying iterator positions for equality.
            friend constexpr bool operator==(const iterator &a, const iterator &b)
            noexcept(noexcept(a.current_ == b.current_)) requires requires(const base_iterator &x,
                                                                           const base_iterator &y) { x == y; } {
                return a.current_ == b.current_;
            }

            /// @brief Compares underlying iterator positions for inequality.
            friend constexpr bool operator!=(const iterator &a, const iterator &b)
            noexcept(noexcept(!(a == b))) requires requires(const base_iterator &x, const base_iterator &y) {
                x == y;
            } {
                return !(a == b);
            }

            /// @brief Moves the underlying iterator to the previous element.
            constexpr iterator &operator--()
            noexcept(noexcept(--std::declval<base_iterator &>())) requires requires(base_iterator it) { --it; } {
                --current_;
                return *this;
            }

            /// @brief Moves to the previous element and returns the prior iterator.
            constexpr iterator operator--(int)
            noexcept(noexcept(std::declval<base_iterator &>()--)) requires requires(base_iterator it) { it--; } {
                auto tmp = *this;
                --current_;
                return tmp;
            }

            /// @brief Advances the underlying iterator by the given offset.
            constexpr iterator &operator+=(difference_type n)
            noexcept(noexcept(std::declval<base_iterator &>() += n)) requires requires(base_iterator it,
                                                                                       difference_type n) { it += n; } {
                current_ += n;
                return *this;
            }

            /// @brief Moves the underlying iterator backward by the given offset.
            constexpr iterator &operator-=(difference_type n)
            noexcept(noexcept(std::declval<base_iterator &>() -= n)) requires requires(base_iterator it,
                                                                                       difference_type n) { it -= n; } {
                current_ -= n;
                return *this;
            }

            /// @brief Returns an iterator advanced by the given offset.
            constexpr iterator operator+(difference_type n) const
            noexcept(noexcept(std::declval<const base_iterator &>() + n)) requires requires(const base_iterator it,
                                                                                            difference_type n) {
                it + n;
            } {
                return iterator(parent_, current_ + n);
            }

            /// @brief Returns an iterator moved backward by the given offset.
            constexpr iterator operator-(difference_type n) const
            noexcept(noexcept(std::declval<const base_iterator &>() - n)) requires requires(const base_iterator it,
                                                                                            difference_type n) {
                it - n;
            } {
                return iterator(parent_, current_ - n);
            }

            /// @brief Returns the distance between two underlying iterators.
            friend constexpr difference_type operator-(const iterator &a, const iterator &b)
            noexcept(noexcept(a.current_ - b.current_)) requires requires(const base_iterator &x,
                                                                          const base_iterator &y) { x - y; } {
                return a.current_ - b.current_;
            }

            /// @brief Applies the view transform to the element at the given offset.
            constexpr decltype(auto) operator[](difference_type n) const
            noexcept(noexcept(std::declval<const base_iterator &>()[n])) requires requires(const base_iterator it,
                                                                                           difference_type n) { it[n]; } {
                return std::invoke(parent_->func_, current_[n]);
            }

            /// @brief Checks whether one underlying iterator precedes another.
            friend constexpr bool operator<(const iterator &a, const iterator &b)
            noexcept(noexcept(a.current_ < b.current_)) requires requires(const base_iterator &x,
                                                                          const base_iterator &y) { x < y; } {
                return a.current_ < b.current_;
            }

            /// @brief Checks whether one underlying iterator follows another.
            friend constexpr bool operator>(const iterator &a, const iterator &b)
            noexcept(noexcept(a.current_ > b.current_)) requires requires(const base_iterator &x,
                                                                          const base_iterator &y) { x > y; } {
                return a.current_ > b.current_;
            }

            /// @brief Checks whether one iterator does not follow another.
            friend constexpr bool operator<=(const iterator &a, const iterator &b)
            noexcept(noexcept(a.current_ <= b.current_)) requires requires(const base_iterator &x,
                                                                           const base_iterator &y) { x <= y; } {
                return a.current_ <= b.current_;
            }

            /// @brief Checks whether one iterator does not precede another.
            friend constexpr bool operator>=(const iterator &a, const iterator &b)
            noexcept(noexcept(a.current_ >= b.current_)) requires requires(const base_iterator &x,
                                                                           const base_iterator &y) { x >= y; } {
                return a.current_ >= b.current_;
            }

            // --- n + iterator ---
            /// @brief Returns an iterator at the given offset from another iterator.
            friend constexpr iterator operator+(difference_type n, const iterator &it)
            noexcept(noexcept(n + std::declval<const base_iterator &>())) requires requires(const base_iterator &i,
                                                                                            difference_type n) {
                n + i;
            } {
                return iterator(it.parent_, n + it.current_);
            }

            // --- sentinel cmp ---
            /// @brief Checks whether an iterator has reached a sentinel.
            template<typename Sentinel>
            friend constexpr bool operator==(const iterator &it, const Sentinel &sent)
            noexcept(noexcept(it.current_ == sent)) {
                return it.current_ == sent;
            }

            /// @brief Checks whether a sentinel has been reached by an iterator.
            template<typename Sentinel>
            friend constexpr bool operator==(const Sentinel &sent, const iterator &it)
            noexcept(noexcept(it == sent)) {
                return it == sent;
            }

            /// @brief Checks whether an iterator has not reached a sentinel.
            template<typename Sentinel>
            friend constexpr bool operator!=(const iterator &it, const Sentinel &sent)
            noexcept(noexcept(!(it == sent))) {
                return !(it == sent);
            }

            /// @brief Checks whether a sentinel has not been reached by an iterator.
            template<typename Sentinel>
            friend constexpr bool operator!=(const Sentinel &sent, const iterator &it)
            noexcept(noexcept(!(it == sent))) {
                return !(it == sent);
            }

        };

        vis_transform_view() = default;

        /**
         * @brief Obtain a constant iterator to the beginning of the transformed range.
         */
        [[nodiscard]] constexpr auto begin() const {
            return iterator(this, std::ranges::begin(base_));
        }

        /**
         * @brief Obtain a constant sentinel or iterator to the end of the range.
         */
        [[nodiscard]] constexpr auto end() const {
            if constexpr (std::ranges::common_range<const std::remove_reference_t<R>>)
                return iterator(this, std::ranges::end(base_));
            else
                return std::ranges::end(base_);
        }

        /// @brief Returns a mutable iterator at the start of the transformed view.
        constexpr auto begin() {
            return std::as_const(*this).begin();
        }

        /// @brief Returns the end iterator or sentinel of the transformed view.
        constexpr auto end() {
            return std::as_const(*this).end();
        }
    };

    /**
     * @brief Deduction guide for <code>vis_transform_view</code>.
     */
    template<typename R, typename F>
    vis_transform_view(R&&, F)
    -> vis_transform_view<std::views::all_t<R>, F>;


} // namespace jh::ranges

namespace std::ranges {
    /**
     * @brief Marks every <code>vis_transform_view</code> as non-borrowed.
     *
     * <p>
     * Its iterator stores a pointer to the view so that dereference can
     * invoke the view's callable. That pointer becomes invalid when the
     * view is destroyed. A borrowed base range therefore cannot make the
     * transformed view borrowed for any <code>R</code> or <code>F</code> in
     * this implementation.
     * </p>
     */
    template<typename R, typename F>
    inline constexpr bool enable_borrowed_range<jh::ranges::vis_transform_view<R, F>> = false;
} // namespace std::ranges
