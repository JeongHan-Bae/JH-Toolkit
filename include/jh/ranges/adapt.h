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
 * @file adapt.h (ranges)
 * @brief Range adaptor promoting <code>jh::concepts::sequence</code> to <code>std::ranges::range</code>.
 * @author
 *   JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 *
 * <p>
 * The <code>jh::ranges::adapt</code> adaptor provides a unified interface to
 * convert any <code>jh::concepts::sequence</code>-compatible object into a
 * valid <code>std::ranges::viewable_range</code>.
 * </p>
 *
 * <p>
 * It is a pipeline-friendly wrapper around <code>jh::to_range()</code>,
 * providing both direct and pipe usage forms:
 * </p>
 *
 * @code
 * using namespace jh::ranges;
 *
 * auto r1 = adapt(seq);      // direct
 * auto r2 = seq | adapt();   // pipe
 * @endcode
 *
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include "jh/conceptual/sequence.h"

namespace jh::ranges {

    namespace detail {

        /**
         * @brief Closure enabling pipe syntax for <tt>adapt</tt>.
         *
         * @details
         * Provides the core callable that applies <code>jh::to_range()</code>
         * lazily over a given sequence.
         *
         * Example:
         * @code
         * auto view = seq | jh::ranges::adapt();
         * @endcode
         */
        struct adapt_closure {

            /**
             * @brief Converts a sequence to a range using <code>jh::to_range()</code>.
             *
             * @tparam Seq The sequence type, must satisfy <code>jh::concepts::sequence</code>.
             * @param seq The sequence to be adapted.
             * @return A <code>std::ranges::viewable_range</code> object.
             */
            template <jh::concepts::sequence Seq>
            constexpr auto operator()(Seq&& seq) const {
                return jh::to_range(std::forward<Seq>(seq));
            }

            /**
             * @brief Enables pipe syntax <tt>sequence | adapt()</tt>.
             *
             * @tparam Seq Sequence type on the left-hand side.
             * @param lhs The sequence being adapted.
             * @param rhs The closure instance.
             * @return The adapted range produced by <code>jh::to_range(lhs)</code>.
             */
            template <jh::concepts::sequence Seq>
            friend constexpr auto operator|(Seq&& lhs, const adapt_closure& rhs) {
                return rhs(std::forward<Seq>(lhs));
            }
        };

        /**
         * @brief Callable implementing the <tt>adapt</tt> adaptor.
         *
         * @details
         * Supports both direct and pipe forms:
         * <ul>
         *   <li><b>Direct:</b> <tt>adapt(seq)</tt></li>
         *   <li><b>Pipe:</b> <tt>seq | adapt()</tt></li>
         * </ul>
         *
         * Internally forwards to <code>jh::to_range()</code>,
         * preserving const-correctness and perfect forwarding.
         */
        struct adapt_fn {

            /**
             * @brief Direct invocation of the adaptor.
             *
             * @tparam Seq Sequence type modeling <code>jh::concepts::sequence</code>.
             * @param seq Input sequence.
             * @return A <code>std::ranges::viewable_range</code>.
             */
            template <jh::concepts::sequence Seq>
            constexpr auto operator()(Seq&& seq) const {
                return jh::to_range(std::forward<Seq>(seq));
            }

            /**
             * @brief Produces a closure for pipe-based composition.
             *
             * @return A <code>detail::adapt_closure</code> object.
             */
            constexpr auto operator()() const noexcept {
                return adapt_closure{};
            }
        };

    } // namespace detail

    /**
     * @brief The user-facing <tt>adapt</tt> adaptor.
     *
     * @details
     * Provides both direct and pipeline conversion of any
     * <code>jh::concepts::sequence</code> into a viewable range.
     *
     * <ul>
     *   <li><tt>auto r = jh::ranges::adapt(seq);</tt></li>
     *   <li><tt>auto r = seq | jh::ranges::adapt();</tt></li>
     * </ul>
     *
     * Acts as a bridge between <code>jh::concepts::sequence</code>
     * and the C++23 <code>std::ranges</code> ecosystem.
     *
     * @note
     * Some types already model <code>std::ranges::range</code> but are
     * <b>non-copyable</b> or <b>non-movable</b>. Such ranges cannot satisfy
     * <code>std::ranges::viewable_range</code>, and thus cannot be used
     * directly with <code>std::views::*</code> adaptors.
     *
     * Passing <b>non-copyable</b> or <b>non-movable</b> <tt>ranges</tt>
     * or any <b>non-standard</b> <tt>sequences</tt> through <code>adapt</code> (or equivalently
     * <code>jh::to_range()</code>) constructs a safe proxy —
     * typically a <code>std::ranges::subrange</code> or
     * <code>jh::ranges::range_wrapper</code> — that <b>restores
     * viewable_range compatibility</b>.
     *
     * Once adapted, these ranges can participate freely in
     * <code>std::views</code> pipelines or <code>jh::ranges::views</code>
     * adaptors.
     *
     * @see jh::to_range
     * @see jh::concepts::sequence
     */
    inline constexpr detail::adapt_fn adapt{};

} // namespace jh::ranges
