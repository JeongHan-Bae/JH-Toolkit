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
 * @file vis_transform_view.h
 * @brief Implementation of the <code>jh::ranges::vis_transform_view</code> class &mdash;
 *        a non-consuming, observation-oriented transformation view.
 * @author
 *   JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 *
 * <p>
 * The <code>jh::ranges::vis_transform_view</code> represents the underlying mechanism
 * used by <code>jh::ranges::views::vis_transform</code>.
 * It applies a callable over a range <em>without consuming</em> the source elements,
 * preserving reentrancy and the integrity of the underlying data.
 * </p>
 *
 * <h3>Core semantics</h3>
 * <ul>
 *   <li>Transforms elements through an observational projection (the callable
 *       must return a non-void value).</li>
 *   <li>Does not mutate or consume the source range &mdash; iteration is repeatable.</li>
 *   <li>Implements <code>std::ranges::view_interface</code> for composability.</li>
 *   <li>Acts as a lightweight, fully constexpr-compatible wrapper.</li>
 * </ul>
 *
 * <p>
 * This class template is instantiated only when
 * <code>jh::concepts::vis_function_for&lt;F, R&gt;</code> is satisfied.
 * That concept ensures both that the callable <code>F</code> can be safely
 * applied to <code>R</code> and that the transformation does not alter
 * or terminate the stream.
 * </p>
 *
 * <p>
 * In contrast to <code>std::views::transform</code>, which models a potentially
 * consumptive view, <code>vis_transform_view</code> guarantees the output
 * remains <b>non-consuming</b> and fully <b>reentrant</b>.
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
     * Unlike <code>std::ranges::transform_view</code>, it does not consume the range.
     * The resulting iteration can be repeated safely, provided the base range
     * itself is reentrant.
     * </p>
     */
    template<std::ranges::range R, typename F> requires jh::concepts::vis_function_for<F, R>
    class vis_transform_view : public std::ranges::view_interface<vis_transform_view<R, F>> {
    public:
        [[no_unique_address]] F func_;
        using traits = jh::concepts::range_storage_traits<R, false>;

        [[no_unique_address]] typename traits::stored_t base_;

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

            constexpr iterator(const vis_transform_view *p, base_iterator it)
                    : current_(it), parent_(p) {}

            /**
             * @brief Apply the transformation function on dereference.
             */
            constexpr decltype(auto) operator*() const
            noexcept(noexcept(std::invoke(std::declval<const F &>(), *std::declval<base_iterator &>()))) {
                return std::invoke(parent_->func_, *current_);
            }

            constexpr iterator &operator++() {
                ++current_;
                return *this;
            }

            constexpr iterator operator++(int)
            noexcept(noexcept(std::declval<base_iterator &>()++)) requires requires(base_iterator it) { it++; } {
                auto tmp = *this;
                ++current_;
                return tmp;
            }

            friend constexpr bool operator==(const iterator &a, const iterator &b)
            noexcept(noexcept(a.current_ == b.current_)) requires requires(const base_iterator &x,
                                                                           const base_iterator &y) { x == y; } {
                return a.current_ == b.current_;
            }

            friend constexpr bool operator!=(const iterator &a, const iterator &b)
            noexcept(noexcept(!(a == b))) requires requires(const base_iterator &x, const base_iterator &y) {
                x == y;
            } {
                return !(a == b);
            }

            constexpr iterator &operator--()
            noexcept(noexcept(--std::declval<base_iterator &>())) requires requires(base_iterator it) { --it; } {
                --current_;
                return *this;
            }

            constexpr iterator operator--(int)
            noexcept(noexcept(std::declval<base_iterator &>()--)) requires requires(base_iterator it) { it--; } {
                auto tmp = *this;
                --current_;
                return tmp;
            }

            constexpr iterator &operator+=(difference_type n)
            noexcept(noexcept(std::declval<base_iterator &>() += n)) requires requires(base_iterator it,
                                                                                       difference_type n) { it += n; } {
                current_ += n;
                return *this;
            }

            constexpr iterator &operator-=(difference_type n)
            noexcept(noexcept(std::declval<base_iterator &>() -= n)) requires requires(base_iterator it,
                                                                                       difference_type n) { it -= n; } {
                current_ -= n;
                return *this;
            }

            constexpr iterator operator+(difference_type n) const
            noexcept(noexcept(std::declval<const base_iterator &>() + n)) requires requires(const base_iterator it,
                                                                                            difference_type n) {
                it + n;
            } {
                return iterator(parent_, current_ + n);
            }

            constexpr iterator operator-(difference_type n) const
            noexcept(noexcept(std::declval<const base_iterator &>() - n)) requires requires(const base_iterator it,
                                                                                            difference_type n) {
                it - n;
            } {
                return iterator(parent_, current_ - n);
            }

            friend constexpr difference_type operator-(const iterator &a, const iterator &b)
            noexcept(noexcept(a.current_ - b.current_)) requires requires(const base_iterator &x,
                                                                          const base_iterator &y) { x - y; } {
                return a.current_ - b.current_;
            }

            constexpr decltype(auto) operator[](difference_type n) const
            noexcept(noexcept(std::declval<const base_iterator &>()[n])) requires requires(const base_iterator it,
                                                                                           difference_type n) { it[n]; } {
                return std::invoke(parent_->func_, current_[n]);
            }

            friend constexpr bool operator<(const iterator &a, const iterator &b)
            noexcept(noexcept(a.current_ < b.current_)) requires requires(const base_iterator &x,
                                                                          const base_iterator &y) { x < y; } {
                return a.current_ < b.current_;
            }

            friend constexpr bool operator>(const iterator &a, const iterator &b)
            noexcept(noexcept(a.current_ > b.current_)) requires requires(const base_iterator &x,
                                                                          const base_iterator &y) { x > y; } {
                return a.current_ > b.current_;
            }

            friend constexpr bool operator<=(const iterator &a, const iterator &b)
            noexcept(noexcept(a.current_ <= b.current_)) requires requires(const base_iterator &x,
                                                                           const base_iterator &y) { x <= y; } {
                return a.current_ <= b.current_;
            }

            friend constexpr bool operator>=(const iterator &a, const iterator &b)
            noexcept(noexcept(a.current_ >= b.current_)) requires requires(const base_iterator &x,
                                                                           const base_iterator &y) { x >= y; } {
                return a.current_ >= b.current_;
            }

            // --- n + iterator ---
            friend constexpr iterator operator+(difference_type n, const iterator &it)
            noexcept(noexcept(n + std::declval<const base_iterator &>())) requires requires(const base_iterator &i,
                                                                                            difference_type n) {
                n + i;
            } {
                return iterator(it.parent_, n + it.current_);
            }

            // --- sentinel cmp ---
            template<typename Sentinel>
            friend constexpr bool operator==(const iterator &it, const Sentinel &sent)
            noexcept(noexcept(it.current_ == sent)) {
                return it.current_ == sent;
            }

            template<typename Sentinel>
            friend constexpr bool operator==(const Sentinel &sent, const iterator &it)
            noexcept(noexcept(it == sent)) {
                return it == sent;
            }

            template<typename Sentinel>
            friend constexpr bool operator!=(const iterator &it, const Sentinel &sent)
            noexcept(noexcept(!(it == sent))) {
                return !(it == sent);
            }

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

        constexpr auto begin() {
            return std::as_const(*this).begin();
        }

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
    template<typename R, typename F>
    [[maybe_unused]] inline constexpr bool enable_borrowed_range<
            jh::ranges::vis_transform_view<R, F>> =
            std::is_lvalue_reference_v<R> || std::ranges::borrowed_range<R>;
} // namespace std::ranges