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
 * @file common.h (ranges/views)
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
 * The adaptor normalizes iterator/sentinel categories without altering data ownership
 * semantics. For standard ranges, it behaves identically to <code>std::views::common</code>.
 * For types satisfying <code>jh::concepts::sequence</code>, it performs a promotion through
 * <code>jh::to_range()</code> to obtain a valid range representation.
 * Non-copyable or non-movable sequences are automatically proxied via
 * <code>std::ranges::subrange</code> when required.
 * </p>
 *
 * <p><b>Behavior summary:</b></p>
 * <ul>
 *   <li>If the source models <code>jh::concepts::sequence</code>, it is promoted
 *       using <code>jh::to_range()</code> and wrapped by <code>std::ranges::common_view</code>.</li>
 *   <li>If the source models only <code>std::ranges::range</code>, the adaptor
 *       forwards to <code>std::views::common()</code>.</li>
 *   <li>Existing <code>common_range</code> types remain unchanged, except when
 *       proxied for non-copyable sources.</li>
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
 *   <li>Applying <code>common()</code> to a temporary object results in a dangling view.
 *       Always ensure the source object outlives the resulting view.</li>
 *   <li>If the input already models <code>std::ranges::common_range</code>,
 *       <code>common()</code> performs no semantic change; it may return
 *       a <code>subrange</code> proxy for non-copyable or non-movable sources.</li>
 *   <li>For consuming (single-pass) ranges, the adaptor directly invokes
 *       <code>std::views::common()</code>.</li>
 * </ul>
 *
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include <ranges>
#include "jh/conceptual/sequence.h"

namespace jh::ranges::views {

    namespace detail {

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
        struct [[maybe_unused]] common_closure {
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
             *       <code>std::ranges::range</code> (consuming flow),
             *       the call directly forwards to
             *       <code>std::views::common(std::forward&lt;R&gt;(r))</code>.</li>
             * </ul>
             *
             * <p>
             * In both cases, ownership semantics are preserved; no copies or moves
             * are introduced unless required for view construction.
             * </p>
             */
            template<typename R>
            requires (jh::concepts::sequence<R> || std::ranges::range<R>)
            constexpr auto operator()(R &&r) const {
                if constexpr (std::ranges::common_range<R>) {
                    return std::forward<R>(r);
                }
                if constexpr (jh::concepts::sequence<R>)
                    return std::ranges::common_view{
                            std::views::all(jh::to_range(std::forward<R>(r)))
                    };
                else
                    return std::ranges::views::common(std::forward<R>(r));
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
        struct [[maybe_unused]] common_fn {
            /**
             * @brief Applies <code>common</code> to a given range or sequence directly.
             *
             * @tparam R Type satisfying <code>jh::concepts::sequence</code>
             *           or <code>std::ranges::range</code>.
             * @param r  The range or sequence object.
             * @return A <code>std::ranges::common_view</code> if <code>R</code>
             *         satisfies <code>jh::concepts::sequence</code>,
             *         otherwise <code>std::views::common(R)</code>.
             *
             * <p><b>Notes:</b></p>
             * <ul>
             *   <li>Sequence types are promoted or proxied through
             *       <code>jh::to_range()</code> as necessary.</li>
             *   <li>For consuming ranges, the standard
             *       <code>std::views::common</code> is applied directly.</li>
             * </ul>
             */
            template<typename R>
            requires (jh::concepts::sequence<R> || std::ranges::range<R>)
            constexpr auto operator()(R &&r) const {
                if constexpr (jh::concepts::sequence<R>)
                    return std::ranges::common_view{
                            std::views::all(jh::to_range(std::forward<R>(r)))
                    };
                else
                    return std::ranges::views::common(std::forward<R>(r));
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
