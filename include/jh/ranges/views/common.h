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
 * @file common.h
 * @brief A unified <code>common</code> view adaptor for both <code>std::ranges::range</code> and
 *        <code>jh::concepts::sequence</code> types.
 * @author
 * JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 *
 * <p>
 * This header provides <code>jh::ranges::views::common</code> &mdash; a lightweight and compatible
 * wrapper around the standard <code>std::views::common</code> adaptor (C++20).
 * It extends the adaptor to support any type modeling <code>jh::concepts::sequence</code>,
 * enabling third-party or user-defined sequence types to interoperate naturally
 * with the standard range pipeline.
 * </p>
 *
 * <p>
 * The adaptor normalizes iterator/sentinel types. Standard ranges outside
 * the Sequence concept are passed to <code>std::views::common</code>.
 * For types satisfying <code>jh::concepts::sequence</code>, it performs a promotion through
 * <code>jh::to_range()</code> to obtain a valid range representation.
 * Non-copyable or non-movable sequences are automatically proxied via
 * <code>std::ranges::subrange</code> when required.
 * </p>
 *
 * <p><b>Behavior summary:</b></p>
 * <ul>
 *   <li>If the source models <code>jh::concepts::sequence</code>, it is converted
 *       using <code>jh::to_range()</code>. A <code>common_view</code> is added only
 *       when the converted view has a distinct sentinel type.</li>
 *   <li>If the source models only <code>std::ranges::range</code>, the adaptor
 *       uses <code>std::views::common()</code> where available and a shared
 *       single-pass cursor when that standard path cannot be constructed.</li>
 *   <li>An already-common converted view is returned without another
 *       <code>common_view</code> layer.</li>
 * </ul>
 *
 * <p><b>Usage examples:</b></p>
 * @code
 * using namespace jh::ranges::views;
 *
 * // 1. Standard range usage
 * auto v1 = std::views::iota(0, 5) | common();
 * // equivalent to std::views::common(iota(...))
 *
 * // 2. A user-defined type modeling jh::concepts::sequence
 * MySequence seq = ...;        // satisfies jh::concepts::sequence
 * auto v2 = seq | common();
 * // promoted through jh::to_range() and std::views::common()
 *
 * // 3. A non-copyable but reentrant range
 * jh::runtime_arr&lt;int&gt; arr{1, 2, 3};
 * auto v3 = arr | common();
 * // already a common_range; produces a subrange proxy if needed
 *
 * // 4. Composed pipeline example
 * auto combined =
 *     ids | enumerate(100)
 *         | zip_pipe(names)
 *         | flatten()
 *         | common();
 * @endcode
 *
 * <p><b>Notes:</b></p>
 * <ul>
 *   <li>A view borrowing an lvalue source requires that source to outlive it.
 *       An rvalue source may be owned by the resulting view.</li>
 *   <li>If the input already models <code>std::ranges::common_range</code>,
 *       <code>common()</code> performs no semantic change; it may return
 *       a <code>subrange</code> proxy for non-copyable or non-movable sources.</li>
 *   <li>A single-pass fallback cursor allocates state on <code>begin()</code>
 *       and keeps iterator copies at the same input position.</li>
 * </ul>
 *
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include <ranges>
#include <memory>
#include "jh/conceptual/sequence.h"

namespace jh::ranges::views {

    namespace detail {

        /**
         * @brief Makes a single-pass input view common through a shared cursor.
         * @tparam V The stored input view type.
         *
         * Iterator copies share the current position. Each call to
         * <code>begin()</code> allocates cursor state and obtains a new iterator
         * from the base view. The base view must outlive its iterators.
         */
        template<std::ranges::view V>
            requires std::ranges::input_range<V>
        class shared_common_view final : public std::ranges::view_interface<shared_common_view<V>> {
            V base_;

            struct state final {
                std::ranges::iterator_t<V> current;
                std::ranges::sentinel_t<V> last;

                state(std::ranges::iterator_t<V> first, std::ranges::sentinel_t<V> end)
                    : current(std::move(first)), last(std::move(end)) {}
            };

        public:
            /// @brief Constructs the wrapper from a standard input view.
            explicit shared_common_view(V base) : base_(std::move(base)) {}

            /// @brief Copyable iterator whose copies share one input position.
            class iterator final {
                std::shared_ptr<state> cursor_;

            public:
                using iterator_concept = std::input_iterator_tag;
                using value_type = std::ranges::range_value_t<V>;
                using difference_type = std::ranges::range_difference_t<V>;

                iterator() = default;

                /// @brief Attaches an iterator to shared cursor state.
                explicit iterator(std::shared_ptr<state> cursor)
                    : cursor_(std::move(cursor)) {}

                /// @brief Reads the current source element.
                decltype(auto) operator*() const { return *cursor_->current; }

                /// @brief Advances the shared input position.
                iterator& operator++() {
                    ++cursor_->current;
                    return *this;
                }

                /// @brief Advances the shared input position after a discarded result.
                void operator++(int) { ++(*this); }

                /// @brief Compares active cursors or an active cursor with the end marker.
                friend bool operator==(const iterator& lhs, const iterator& rhs) {
                    if (!lhs.cursor_) return !rhs.cursor_ || rhs.cursor_->current == rhs.cursor_->last;
                    if (!rhs.cursor_) return lhs.cursor_->current == lhs.cursor_->last;
                    if (lhs.cursor_ == rhs.cursor_) return true;
                    return lhs.cursor_->current == lhs.cursor_->last &&
                           rhs.cursor_->current == rhs.cursor_->last;
                }
            };

            /// @brief Starts a new single-pass cursor over the base view.
            iterator begin() {
                return iterator(std::make_shared<state>(std::ranges::begin(base_),
                                                        std::ranges::end(base_)));
            }

            /// @brief Returns a same-type end marker.
            iterator end() const { return iterator{}; }
        };

        /**
         * @brief Closure type enabling pipe syntax for <code>common()</code>.
         *
         * <p>
         * The closure captures no state and serves as the adaptor instance
         * that can be used in pipeline form:
         * <code>range | jh::ranges::views::common()</code>.
         * </p>
         *
         * <p>
         * The overloads of <code>operator()</code> handle both
         * <code>jh::concepts::sequence</code> and ordinary
         * <code>std::ranges::range</code> types.
         * </p>
         */
        struct [[maybe_unused]] common_closure final {
            /**
             * @brief Applies the <code>common</code> adaptor to a given range or sequence.
             *
             * @tparam R The input type, satisfying either <code>jh::concepts::sequence</code>
             *           or <code>std::ranges::range</code>.
             * @param r  The range or sequence object to normalize.
             * @return A <code>std::ranges::common_view</code> or
             *         <code>std::views::common</code> view.
             *
             * <p><b>Behavior:</b></p>
             * <ul>
             *   <li>If <code>R</code> already models <code>std::ranges::common_range</code>,
             *       it is returned directly (passed through without wrapping).</li>
             *   <li>If <code>R</code> satisfies <code>jh::concepts::sequence</code>:
             *       <ol>
             *         <li>Objects fully compatible with <code>std::views::all</code>
             *             are transparently forwarded through <code>jh::to_range()</code>.</li>
             *         <li>Types that are ranges but not copyable/movable are proxied
             *             as <code>std::ranges::subrange</code>.</li>
             *         <li>Types that are not ranges are promoted to compliant range
             *             representations via <code>jh::to_range()</code>.</li>
             *       </ol>
             *   </li>
             *   <li>Otherwise, if <code>R</code> is only a
             *       <code>std::ranges::range</code>,
             *       the call uses <code>std::views::common</code> when possible
             *       or a shared single-pass cursor otherwise.</li>
             * </ul>
             *
             * <p>
             * View construction may borrow or move its source according to
             * the source value category and the selected adaptor.
             * </p>
             */
            template<typename R>
            requires (jh::concepts::sequence<R> || std::ranges::range<R>)
            constexpr decltype(auto) operator()(R &&r) const {
                if constexpr (jh::concepts::sequence<R>) {
                    using all_t = decltype(std::views::all(jh::to_range(std::declval<R>())));

                    if constexpr (std::ranges::common_range<std::remove_cvref_t<all_t>>) {
                        return std::views::all(jh::to_range(std::forward<R>(r)));
                    } else {
                        return std::ranges::common_view{
                                std::views::all(jh::to_range(std::forward<R>(r)))
                        };
                    }
                } else {
                    if constexpr (requires { std::ranges::views::common(std::forward<R>(r)); }) {
                        return std::ranges::views::common(std::forward<R>(r));
                    } else {
                        return shared_common_view<std::views::all_t<R>>{
                                std::views::all(std::forward<R>(r))
                        };
                    }
                }
            }

            /**
             * @brief Enables pipe-style composition:
             *        <code>range | common()</code>.
             *
             * @tparam R A range or sequence type.
             * @param lhs Left-hand operand (range or sequence).
             * @param rhs Right-hand closure instance (the adaptor).
             * @return Result of <code>rhs(lhs)</code>.
             *
             * <p>
             * This overload ensures compatibility with the
             * <code>std::ranges::range_adaptor_closure</code> model,
             * allowing <code>common()</code> to participate naturally
             * in range pipelines.
             * </p>
             */
            template<typename R>
            requires (jh::concepts::sequence<R> || std::ranges::range<R>)
            friend constexpr auto operator|(R &&lhs, const common_closure &rhs) {
                return rhs(std::forward<R>(lhs));
            }
        };

        /**
         * @brief Function object implementing the <code>common</code> view adaptor.
         *
         * <p>
         * This callable supports both direct and pipe forms:
         * </p>
         * <ul>
         *   <li><b>Direct call:</b> <code>common(r)</code></li>
         *   <li><b>Pipe form:</b> <code>r | common()</code></li>
         * </ul>
         *
         * <p>
         * It dispatches to the appropriate normalization logic depending on
         * whether the argument satisfies <code>jh::concepts::sequence</code>
         * or only <code>std::ranges::range</code>.
         * </p>
         */
        struct [[maybe_unused]] common_fn final {
            /**
             * @brief Applies <code>common</code> to a given range or sequence directly.
             *
             * @tparam R Type satisfying <code>jh::concepts::sequence</code>
             *           or <code>std::ranges::range</code>.
             * @param r  The range or sequence object.
             * @return A common range, possibly the converted view itself.
             *
             * <p><b>Notes:</b></p>
             * <ul>
             *   <li>Sequence types are promoted or proxied through
             *       <code>jh::to_range()</code> as necessary.</li>
             *   <li>For standard ranges outside the Sequence concept, the
             *       standard adaptor is tried before the shared cursor fallback.</li>
             * </ul>
             */
            template<typename R>
            requires (jh::concepts::sequence<R> || std::ranges::range<R>)
            constexpr decltype(auto) operator()(R &&r) const {
                if constexpr (jh::concepts::sequence<R>) {
                    using all_t = decltype(std::views::all(jh::to_range(std::declval<R>())));

                    if constexpr (std::ranges::common_range<std::remove_cvref_t<all_t>>) {
                        return std::views::all(jh::to_range(std::forward<R>(r)));
                    } else {
                        return std::ranges::common_view{
                                std::views::all(jh::to_range(std::forward<R>(r)))
                        };
                    }
                } else {
                    if constexpr (requires { std::ranges::views::common(std::forward<R>(r)); }) {
                        return std::ranges::views::common(std::forward<R>(r));
                    } else {
                        return shared_common_view<std::views::all_t<R>>{
                                std::views::all(std::forward<R>(r))
                        };
                    }
                }
            }

            /**
             * @brief Returns a closure object enabling pipeline usage.
             *
             * @return A <code>common_closure</code> instance.
             *
             * <p>
             * This overload allows the adaptor to be used in the
             * pipe form <code>range | common()</code> instead of the
             * function-call form <code>common(range)</code>.
             * </p>
             */
            constexpr auto operator()() const noexcept {
                return common_closure{};
            }
        };

    } // namespace detail

    /**
     * @brief User-facing <code>common</code> adaptor instance.
     *
     * <p>
     * Provides both direct and pipeline forms:
     * </p>
     * <ul>
     *   <li><code>auto v = common(r);</code></li>
     *   <li><code>auto v = r | common();</code></li>
     * </ul>
     *
     * <p>
     * For details on behavior and semantics,
     * see <code>detail::common_fn</code>.
     * </p>
     */
    inline constexpr detail::common_fn common{};

} // namespace jh::ranges::views
