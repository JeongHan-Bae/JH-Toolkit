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
 * @file transform.h
 * @brief Unified transform adaptor that dispatches between <code>std::views::transform</code>
 *        and <code>jh::ranges::views::vis_transform</code>.
 * @author
 *   JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 *
 * <p>
 * The <code>jh::ranges::views::transform</code> adaptor performs a compile-time
 * semantic dispatch between two transformation behaviors:
 * <ul>
 *   <li><code>jh::ranges::views::vis_transform</code> &mdash; when the pair <code>&lt;R, F&gt;</code>
 *       satisfies <code>jh::concepts::vis_function_for</code>, meaning the range
 *       is non-consuming and the function is a pure observation (returns non-void).</li>
 *   <li><code>std::views::transform</code> &mdash; otherwise, for standard consumptive
 *       transformations that may alter or terminate the underlying stream.</li>
 * </ul>
 * </p>
 *
 * <h3>Design rationale</h3>
 * <ul>
 *   <li><b>Problem:</b> <code>std::views::transform</code> treats every projection
 *       as consumptive, even when it merely observes elements.
 *       This breaks reentrancy and forces intermediate buffering in non-consuming pipelines.</li>
 *   <li><b>Solution:</b> Defer dispatch until both <code>R</code> and <code>F</code> are known,
 *       so the adaptor can route to <code>vis_transform</code> only when the combination
 *       preserves observation semantics. This ensures correctness and efficiency.</li>
 * </ul>
 *
 * Supports both <b>direct</b> and <b>pipe</b> usage forms:
 * <ul>
 *   <li><b>Direct form:</b> <code>auto v = jh::ranges::views::transform(r, f);</code></li>
 *   <li><b>Pipe form:</b> <code>auto v = r | jh::ranges::views::transform(f);</code></li>
 * </ul>
 *
 * @note
 * <p>
 * Dispatch occurs <em>inside</em> the call operator, because determining
 * whether <code>vis_transform</code> semantics apply requires both
 * the range and the callable type.
 * This ensures that only non-consuming, pure-observation combinations
 * are elevated to <code>vis_transform</code>, while all others fall back
 * to the standard <code>std::views::transform</code>.
 * </p>
 *
 * @see jh::concepts::vis_function_for
 * @see jh::ranges::views::vis_transform
 * @see std::views::transform
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include "jh/ranges/views/vis_transform.h"

namespace jh::ranges::views {

    namespace detail {

        /**
         * @brief Closure object enabling pipe syntax for <code>transform</code>.
         *
         * @tparam F The callable type captured.
         * @details
         * Stores the callable and delays dispatch until a range is provided.
         * The combination <code>&lt;R, F&gt;</code> is inspected in
         * <code>operator()</code> to determine whether the transformation
         * is non-consuming (<code>vis_transform</code>) or consumptive
         * (<code>std::views::transform</code>).
         */
        template<typename F>
        struct transform_closure final {
            [[no_unique_address]] F func;

            /**
             * @brief Apply the captured callable to a range with semantic dispatch.
             *
             * @tparam R Range type.
             * @param r The input range.
             * @return Either a <code>jh::ranges::vis_transform_view</code> (for non-consuming)
             *         or a <code>std::ranges::transform_view</code> (for consumptive behavior).
             */
            template<std::ranges::range R>
            constexpr auto operator()(R &&r) const {
                if constexpr (jh::concepts::vis_function_for<F, R>)
                    return jh::ranges::views::vis_transform(std::forward<R>(r), func);
                else
                    return std::ranges::views::transform(std::forward<R>(r), func);
            }

            /**
             * @brief Enables pipe syntax <code>range | transform(f)</code>.
             */
            template<std::ranges::range R>
            friend constexpr auto operator|(R &&lhs, const transform_closure &rhs) {
                return rhs(std::forward<R>(lhs));
            }
        };

        /**
         * @brief Callable adaptor for <code>jh::ranges::views::transform</code>.
         *
         * @details
         * Provides both direct and pipe forms.
         * Dispatch is determined for each <code>&lt;R, F&gt;</code> pair at the point of call:
         * <ul>
         *   <li>If the range is non-consuming and the callable is observational,
         *       uses <code>vis_transform</code>.</li>
         *   <li>Otherwise, uses <code>std::views::transform</code>.</li>
         * </ul>
         */
        struct transform_fn final {

            /**
             * @brief Direct form &mdash; immediately constructs the selected transform view.
             *
             * @tparam R Range type.
             * @tparam F Callable type.
             * @param r Source range.
             * @param f Function applied to each element.
             * @return A transform view &mdash; either non-consuming or consumptive depending on semantics.
             */
            template<std::ranges::range R, typename F>
            constexpr auto operator()(R &&r, F &&f) const {
                if constexpr (jh::concepts::vis_function_for<F, R>)
                    return jh::ranges::views::vis_transform(std::forward<R>(r), std::forward<F>(f));
                else
                    return std::ranges::views::transform(std::forward<R>(r), std::forward<F>(f));
            }

            /**
             * @brief Pipe form &mdash; captures a callable into a reusable closure.
             *
             * @tparam F The callable type.
             * @param f The function to apply lazily.
             * @return A <code>transform_closure</code> for pipe composition.
             */
            template<typename F>
            constexpr auto operator()(F &&f) const {
                return transform_closure<std::decay_t<F>>{std::forward<F>(f)};
            }
        };

    } // namespace detail

    /**
     * @brief The unified <code>transform</code> adaptor entry point.
     *
     * <p>
     * Provides an adaptive interface for transformation within range pipelines.
     * The adaptor performs a deferred semantic inspection of the pair <code>&lt;R, F&gt;</code>
     * to determine whether to use the non-consuming or consumptive transformation path.
     * </p>
     *
     * Supports both <b>direct</b> and <b>pipe</b> usage forms:
     * <ul>
     *   <li><b>Direct form:</b> <code>auto v = jh::ranges::views::transform(r, f);</code></li>
     *   <li><b>Pipe form:</b> <code>auto v = r | jh::ranges::views::transform(f);</code></li>
     * </ul>
     *
     * @note
     * <p>
     * The dispatch is performed per combination of range and callable.
     * Non-consuming ranges with purely observational projections automatically
     * gain <code>vis_transform</code> semantics and remain reentrant.
     * All others use <code>std::views::transform</code> to preserve correct consumption semantics.
     * </p>
     */
    inline constexpr detail::transform_fn transform{};

} // namespace jh::ranges::views
