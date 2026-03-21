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
 * @file flatten.h
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 * @brief View adaptor for flattening tuple-like elements in a range.
 *
 * <p>
 * The <code>jh::ranges::views::flatten</code> adaptor produces a lazy view that
 * inspects each element of a range and, if it models
 * <code>jh::concepts::tuple_like</code>, wraps it in a
 * <code>jh::meta::flatten_proxy</code>. Non-tuple-like elements are forwarded
 * unchanged.
 * </p>
 *
 * <h3>Behavior</h3>
 * <ul>
 *   <li>Tuple-like elements are recursively flattened into
 *       <code>jh::meta::flatten_proxy</code> objects.</li>
 *   <li>Non-tuple-like elements are passed through as-is.</li>
 *   <li>The transformation is applied lazily; no element is copied or expanded eagerly.</li>
 * </ul>
 *
 * <h3>Implementation Notes</h3>
 * <ul>
 *   <li><code>flatten</code> delegates to <code>jh::ranges::views::transform</code>.</li>
 *   <li><code>jh::ranges::views::transform</code> internally decides whether to
 *       construct a non-consuming or consuming transformation view depending on
 *       the behavior of the projection function.</li>
 *   <li>Because <code>flatten</code> is a pure observation, its output preserves
 *       the consumption property of the underlying range:
 *       if the input is non-consuming, the output remains non-consuming;
 *       if the input is single-pass, the result is also single-pass.</li>
 * </ul>
 *
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include <ranges>
#include "jh/metax/flatten_proxy.h"
#include "jh/ranges/views/transform.h"

namespace jh::ranges::views {
    namespace detail {

        /**
         * @brief Closure enabling pipe syntax for <code>flatten</code>.
         *
         * <p>
         * Provides the callable interface for <code>jh::ranges::views::flatten</code>.
         * It applies a lazy flattening transformation that wraps tuple-like elements
         * into <code>jh::meta::flatten_proxy</code> objects while leaving other elements unchanged.
         * </p>
         *
         * <p>
         * The transformation is purely observational. Data is neither consumed nor copied,
         * and the resulting view preserves the consumption property of the input range.
         * </p>
         */
        struct flatten_closure final {

            /**
             * @brief Applies the flatten adaptor directly to a range.
             *
             * @tparam R A type satisfying <code>std::ranges::range</code>.
             * @param r  The range whose elements are to be flattened.
             * @return   A transformation view that flattens tuple-like elements lazily.
             *
             * <p><b>Semantics:</b></p>
             * <ul>
             *   <li>Tuple-like elements are wrapped into <code>jh::meta::flatten_proxy</code>.</li>
             *   <li>Non-tuple-like elements are passed through unchanged.</li>
             *   <li>The adaptor invokes <code>jh::ranges::views::transform</code>,
             *       which automatically selects an appropriate internal implementation
             *       according to the projection's observational nature.</li>
             *   <li>As <code>flatten</code> is purely observational, no ownership or
             *       lifetime semantics are altered.</li>
             * </ul>
             */
            template<std::ranges::range R>
            constexpr auto operator()(R &&r) const {
                auto transform_fn = [](auto &&elem) {
                    using T = std::remove_cvref_t<decltype(elem)>;
                    if constexpr (jh::concepts::tuple_like<T>)
                        return jh::meta::flatten_proxy{std::forward<decltype(elem)>(elem)};
                    else
                        return std::forward<decltype(elem)>(elem);
                };

                return jh::ranges::views::transform(
                        std::views::all(std::forward<R>(r)),
                        transform_fn
                );
            }

            /**
             * @brief Enables pipe syntax for <code>flatten</code>.
             *
             * @tparam R A type satisfying <code>std::ranges::range</code>.
             * @param lhs The range on the left-hand side of the pipe.
             * @param rhs The closure instance representing <code>flatten</code>.
             * @return   The flattened transformation view.
             *
             * <p>
             * Allows expressions of the form:
             * </p>
             * @code
             * auto v = some_range | jh::ranges::views::flatten();
             * @endcode
             * which is equivalent to:
             * @code
             * auto v = jh::ranges::views::flatten(some_range);
             * @endcode
             */
            template<std::ranges::range R>
            friend constexpr auto operator|(R &&lhs, const flatten_closure &rhs) {
                return rhs(std::forward<R>(lhs));
            }
        };

        /**
         * @brief Callable function object implementing <code>flatten</code>.
         *
         * <p>
         * Provides both direct and pipe-based interfaces for applying flattening transformation.
         * Each element of the range is inspected; tuple-like values are flattened,
         * and all others are forwarded unmodified.
         * </p>
         *
         * <p>
         * This adaptor preserves the consumption semantics of the source range:
         * if the input range is non-consuming, the resulting view is reentrant;
         * if the input is single-pass, the resulting view is also single-pass.
         * </p>
         */
        struct flatten_fn final {
            /**
             * @brief Applies <code>flatten</code> directly to a range.
             *
             * @tparam R Any <code>std::ranges::viewable_range</code>.
             * @param r  The range to flatten.
             * @return   A lazy transformation view flattening tuple-like elements.
             *
             * <p>
             * Equivalent to <code>flatten_closure{}(r)</code>.
             * </p>
             *
             * <p>
             * The transformation delegates to <code>jh::ranges::views::transform</code>,
             * ensuring correct propagation of the input's reentrancy.
             * </p>
             */
            template<std::ranges::viewable_range R>
            constexpr auto operator()(R &&r) const {
                return flatten_closure{}(std::forward<R>(r));
            }

            /**
             * @brief Produces a closure object for pipe syntax.
             *
             * @return A <code>flatten_closure</code> that can be used in pipelines.
             *
             * <p>
             * This overload enables the form:
             * </p>
             * @code
             * auto flattened = range | jh::ranges::views::flatten();
             * @endcode
             */
            constexpr auto operator()() const noexcept {
                return flatten_closure{};
            }
        };

    } // namespace detail

    /**
     * @brief User-facing <code>flatten</code> adaptor.
     *
     * <p>
     * Provides a unified interface for flattening tuple-like elements within a range.
     * The adaptor is lazy and purely observational: tuple-like elements are wrapped
     * into <code>jh::meta::flatten_proxy</code> objects, while other elements are
     * forwarded unchanged.
     * </p>
     *
     * Supports both <b>direct</b> and <b>pipe</b> usage forms:
     * <ul>
     *   <li><b>Direct form:</b> <code>auto v = jh::ranges::views::flatten(range);</code></li>
     *   <li><b>Pipe form:</b> <code>auto v = range | jh::ranges::views::flatten();</code></li>
     * </ul>
     *
     * <p><b>Behavior:</b></p>
     * <ul>
     *   <li>Each element is inspected lazily; if it models
     *       <code>jh::concepts::tuple_like</code>, it is wrapped into
     *       <code>jh::meta::flatten_proxy</code>.</li>
     *   <li>Non-tuple-like elements are left untouched.</li>
     *   <li>The adaptor delegates to <code>jh::ranges::views::transform</code>,
     *       which automatically preserves the consumption behavior of the input range:
     *       <ul>
     *         <li>If the source is non-consuming, the result remains reentrant.</li>
     *         <li>If the source is consuming, the result remains single-pass.</li>
     *       </ul>
     *   </li>
     *   <li>No ownership or lifetime semantics are altered.</li>
     * </ul>
     *
     * @note
     * The flatten adaptor determines flattenability purely by duck typing:
     * any object that supports structured binding is recursively expanded.
     * <br>
     * The following are recognized as part of the framework's standard set
     * of tuple-like types to prevent accidental misuse:
     * <ul>
     *   <li><code>std::pair</code>, <code>std::tuple</code>, <code>std::array</code></li>
     *   <li><code>jh::pod::pair</code>, <code>jh::pod::tuple</code>, <code>jh::pod::array</code></li>
     *   <li><code>jh::ranges::zip_view</code> element proxies
     *       (<code>jh::ranges::zip_reference_proxy</code>)</li>
     * </ul>
     * <br>
     * User-defined POD or aggregate types that do not declare structured binding
     * are never flattened and are treated as atomic values. Flattening applies
     * only to types that explicitly declare structured binding.
     * <br>
     * Because such a declaration requires explicit <code>std</code>
     * specialization, it cannot occur accidentally. Therefore,
     * <strong>declaring structured binding is considered explicit permission
     * for recursive deconstruction</strong>, consistent with
     * <code>jh::concepts::tuple_like</code>.
     *
     * @see jh::meta::flatten_proxy
     * @see jh::ranges::views::transform
     * @see jh::concepts::tuple_like
     */
    inline constexpr detail::flatten_fn flatten{};

} // namespace jh::ranges::views
