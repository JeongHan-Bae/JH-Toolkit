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
 * @file vis_transform.h
 * @brief Explicit non-consuming transform adaptor preserving reentrancy.
 * @author
 *   JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 *
 * <p>
 * The <code>jh::ranges::views::vis_transform</code> adaptor provides an
 * <b>explicit, observation-only</b> variant of <code>std::views::transform</code>.
 * It guarantees non-consuming behavior for reentrant ranges and is used internally
 * by adaptors such as <code>jh::ranges::views::flatten</code>.
 * </p>
 *
 * <p>
 * Unlike the standard adaptor, which treats every transformation as consumptive,
 * <code>vis_transform</code> ensures that its result remains reentrant and stable
 * whenever the underlying range allows it.
 * </p>
 *
 * <h3>Design rationale</h3>
 * <ul>
 *   <li><b>Non-consuming by contract:</b> The resulting view never consumes its input.</li>
 *   <li><b>Explicit intent:</b> Because this guarantees reentrancy, its use must be explicit.</li>
 *   <li><b>Interoperability:</b> Fully compatible with <code>jh::ranges::views::common()</code>
 *       and <code>jh::ranges::to</code>.</li>
 * </ul>
 *
 * <h3>Typical pipeline</h3>
 * <p>
 * A typical usage chain combining multiple adaptors and collection:
 * </p>
 * @code
 * auto result = ids
 *    | jh::ranges::views::enumerate(100)
 *    | jh::ranges::views::zip_pipe(names)
 *    | jh::ranges::views::flatten() // invokes vis_transform internally
 *    | jh::ranges::views::common()
 *    | jh::ranges::to&lt;std::vector&lt;std::tuple&lt;int, int, std::string&gt;&gt;&gt;();
 * @endcode
 *
 * <p>
 * This form requires no intermediate <code>collect()</code> adaptor &mdash; the
 * pipeline remains reentrant, and <code>to&lt;&gt;</code> performs direct construction.
 * </p>
 *
 * @see jh::ranges::vis_transform_view
 * @see jh::ranges::views::transform
 * @see jh::ranges::views::common
 * @see jh::ranges::to
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include "jh/ranges/vis_transform_view.h"

namespace jh::ranges::views {

    namespace detail {

        /**
         * @brief Closure object used for pipe syntax of <code>vis_transform</code>.
         *
         * @tparam F The callable type captured.
         * @details
         * Stores a function object to be applied lazily to elements of a range.
         * The result is always a non-consuming, observation-only view.
         */
        template<typename F>
        struct vis_transform_closure final {
            [[no_unique_address]] F func;

            /**
             * @brief Apply the captured function to a range.
             *
             * @tparam R The range type.
             * @param r The input range.
             * @return A <code>jh::ranges::vis_transform_view</code> preserving non-consuming semantics.
             */
            template<std::ranges::range R>
            requires jh::concepts::vis_function_for<F, R>
            constexpr auto operator()(R &&r) const {
                return jh::ranges::vis_transform_view{
                        std::forward<R>(r), func
                };
            }

            /**
             * @brief Enables pipe syntax <code>range | vis_transform(f)</code>.
             */
            template<std::ranges::range R>
            requires jh::concepts::vis_function_for<F, R>
            friend constexpr auto operator|(R &&lhs, const vis_transform_closure &rhs) {
                return rhs(std::forward<R>(lhs));
            }
        };

        /**
         * @brief Callable adaptor for <code>vis_transform</code>.
         *
         * @details
         * Provides both direct and pipe invocation forms for constructing
         * a <code>jh::ranges::vis_transform_view</code>. The transformation
         * is always non-consuming.
         */
        struct vis_transform_fn final {

            /**
             * @brief Direct form &mdash; immediately constructs a <code>vis_transform_view</code>.
             *
             * @tparam R Range type.
             * @tparam F Callable type.
             * @param r The source range.
             * @param f The transformation function.
             * @return A non-consuming <code>jh::ranges::vis_transform_view</code>.
             */
            template<std::ranges::range R, typename F>
            requires jh::concepts::vis_function_for<F, R>
            constexpr auto operator()(R &&r, F &&f) const {
                return jh::ranges::vis_transform_view{
                        std::forward<R>(r), std::forward<F>(f)
                };
            }

            /**
             * @brief Pipe form &mdash; captures a callable into a reusable closure.
             *
             * @tparam F Callable type.
             * @param f The function to be applied to each element.
             * @return A <code>vis_transform_closure</code> suitable for pipe syntax.
             *
             * <p>Example:</p>
             * <code>auto v = range | jh::ranges::views::vis_transform(f);</code>
             */
            template<typename F>
            constexpr auto operator()(F &&f) const {
                return vis_transform_closure<std::decay_t<F>>{
                        std::forward<F>(f)
                };
            }
        };

    } // namespace detail

    /**
     * @brief The user-facing <code>vis_transform</code> adaptor.
     *
     * <p>
     * Provides an explicit, non-consuming transform adaptor for
     * observation-only projections. Unlike <code>std::views::transform</code>,
     * this adaptor preserves reentrancy whenever possible.
     * </p>
     *
     * Supports both <b>direct</b> and <b>pipe</b> usage forms:
     * <ul>
     *   <li><b>Direct form:</b> <code>auto v = jh::ranges::views::vis_transform(r, f);</code></li>
     *   <li><b>Pipe form:</b> <code>auto v = r | jh::ranges::views::vis_transform(f);</code></li>
     * </ul>
     *
     * <p>
     * The adaptor enforces the concept <code>jh::concepts::vis_function_for&lt;F, R&gt;</code>,
     * which requires that the callable is safely invocable for every element
     * of <code>R</code> and returns a non-void result.
     * </p>
     *
     * <h4>Design semantics</h4>
     * <ul>
     *   <li>All projections are non-consuming and reentrant by design.</li>
     *   <li>Intended for explicit use in analytical or visualization pipelines.</li>
     *   <li>Integrates directly with <code>jh::ranges::views::common()</code> and
     *       <code>jh::ranges::to</code> for final materialization.</li>
     * </ul>
     *
     * @note
     * <p>
     * When combined with <code>jh::ranges::views::common()</code>, a
     * <code>vis_transform</code>-based pipeline can be materialized directly
     * into a container via <code>jh::ranges::to</code> &mdash; no intermediate
     * <code>collect()</code> is required.
     * </p>
     *
     * @see jh::ranges::vis_transform_view
     * @see jh::concepts::vis_function_for
     * @see jh::ranges::views::transform
     * @see jh::ranges::views::common
     * @see jh::ranges::to
     */
    inline constexpr detail::vis_transform_fn vis_transform{};

} // namespace jh::ranges::views
