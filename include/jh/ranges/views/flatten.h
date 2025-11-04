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
 * @file flatten.h (ranges/views)
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 * @brief View adaptor for flattening tuple-like elements in a range.
 *
 * @details
 *   The <code>jh::ranges::views::flatten</code> adaptor produces a lazy view that
 *   traverses a range and flattens its <tt>tuple-like</tt> elements into
 *   <code>jh::meta::flatten_proxy</code> objects.
 *   Non–tuple-like elements are passed through unchanged.
 *
 *   Each element of the resulting view is therefore either
 *   - a <code>jh::meta::flatten_proxy</code> (when the source element models
 *     <code>jh::concepts::tuple_like</code>), or
 *   - the original element itself (when it does not).
 *
 * <h3>Behavior</h3>
 * <ul>
 *   <li>If an element is <code>tuple_like</code>, it is recursively flattened into
 *       a <code>jh::meta::flatten_proxy</code>.</li>
 *   <li>If an element is not <code>tuple_like</code>, it is left untouched
 *       (no-op).</li>
 *   <li>The resulting range is implemented as a
 *       <code>std::ranges::transform_view</code> applying the flattening projection
 *       lazily.</li>
 * </ul>
 *
 * <h3>Design Notes</h3>
 * <ul>
 *   <li>Flattening occurs lazily via <code>transform_view</code> projection.</li>
 *   <li><code>flatten_proxy</code> instances are compatible with structured bindings
 *       and tuple-based algorithms.</li>
 *   <li>Perfect forwarding and <code>constexpr</code> evaluation are preserved.</li>
 *   <li>Non–tuple-like elements incur zero overhead (no additional wrapping).</li>
 * </ul>
 *
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include <ranges>
#include "jh/meta/flatten_proxy.h"

namespace jh::ranges::views {
    namespace detail {

        /**
         * @brief Closure enabling pipe syntax for <tt>flatten</tt> adaptor.
         *
         * @details
         * Provides the core callable implementation for
         * <code>jh::ranges::views::flatten</code>. It applies a lazy transformation
         * over a given range, recursively flattening tuple-like elements into
         * <code>jh::meta::flatten_proxy</code> objects.
         *
         * <p><b>Behavior summary:</b></p>
         * <ul>
         *   <li>If the element type models <code>jh::concepts::tuple_like</code>,
         *       it is replaced by a <code>jh::meta::flatten_proxy</code>.</li>
         *   <li>Otherwise, the element is forwarded unchanged.</li>
         *   <li>The resulting range is represented as a
         *       <code>std::ranges::transform_view</code>.</li>
         * </ul>
         */
        struct flatten_closure {

            /**
             * @brief Applies flattening transformation to a given range.
             *
             * @tparam R The range type, must satisfy <code>std::ranges::range</code>.
             * @param r The input range to flatten.
             * @return A <tt>std::ranges::transform_view</tt> that lazily flattens tuple-like elements.
             *
             * @details
             * The transformation projects each element as follows:
             * <ul>
             *   <li>If <code>T</code> models <code>jh::concepts::tuple_like</code>,
             *       returns <code>jh::meta::flatten_proxy&lt;T&gt;</code>.</li>
             *   <li>Otherwise, returns <code>T</code> directly.</li>
             * </ul>
             *
             * @note This operation is <b>lazy</b>; no data is copied or eagerly expanded.
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

                return std::ranges::transform_view(std::views::all(std::forward<R>(r)), transform_fn);
            }

            /**
             * @brief Enables <tt>range | flatten_closure</tt> syntax.
             *
             * @tparam R The range type on the left-hand side of the pipe.
             * @param lhs The range to be flattened.
             * @param rhs The flatten closure instance.
             * @return A <tt>std::ranges::transform_view</tt> flattening <tt>lhs</tt>.
             *
             * @details
             * This overload allows expressions like:
             * @code
             * auto v = seq | jh::ranges::views::flatten();
             * @endcode
             * which is equivalent to <code>flatten()(seq)</code>.
             */
            template<std::ranges::range R>
            friend constexpr auto operator|(R &&lhs, const flatten_closure &rhs) {
                return rhs(std::forward<R>(lhs));
            }
        };

        /**
         * @brief Callable function object implementing <tt>flatten</tt>.
         *
         * @details
         * Supports both direct invocation and pipe-based composition.
         * <ul>
         *   <li><b>Direct form:</b> <code>flatten(range)</code></li>
         *   <li><b>Pipe form:</b> <code>range | flatten()</code></li>
         * </ul>
         *
         * The adaptor produces a lazy transformation view
         * that flattens tuple-like elements while leaving other
         * elements untouched.
         */
        struct flatten_fn {
            /**
             * @brief Direct form of the <tt>flatten</tt> adaptor.
             *
             * @tparam R The range type to be flattened.
             * @param r The input range.
             * @return A <tt>std::ranges::transform_view</tt> applying flattening lazily.
             *
             * @details
             * Invokes <code>flatten_closure{}(r)</code>, forwarding all arguments
             * and preserving <code>constexpr</code> semantics.
             */
            template<std::ranges::viewable_range R>
            constexpr auto operator()(R &&r) const {
                return flatten_closure{}(std::forward<R>(r));
            }

            /**
             * @brief Pipe form factory for the <tt>flatten</tt> adaptor.
             *
             * @return A <tt>flatten_closure</tt> enabling composition via the pipe operator.
             *
             * @details
             * This overload allows the adaptor to be used in a pipeline expression:
             * @code
             * auto flat = seq | jh::ranges::views::flatten();
             * @endcode
             */
            constexpr auto operator()() const noexcept {
                return flatten_closure{};
            }
        };

    } // namespace detail

    /**
     * @brief The user-facing <tt>flatten</tt> adaptor.
     *
     * @details
     * Provides a unified interface for flattening tuple-like elements within a range.
     * Each element is inspected lazily, and tuple-like elements are wrapped into
     * <code>jh::meta::flatten_proxy</code> objects.
     *
     * Supports both <b>direct</b> and <b>pipe</b> usage forms:
     * <ul>
     *   <li><b>Direct form:</b> <tt>auto v = jh::ranges::views::flatten(range);</tt></li>
     *   <li><b>Pipe form:</b> <tt>auto v = range | jh::ranges::views::flatten();</tt></li>
     * </ul>
     *
     * <p><b>Behavior:</b></p>
     * <ul>
     *   <li>If an element models <code>jh::concepts::tuple_like</code>,
     *       it is flattened into a <code>jh::meta::flatten_proxy</code>.</li>
     *   <li>Otherwise, the element is passed through unchanged.</li>
     *   <li>The transformation is lazy, implemented via
     *       <code>std::ranges::transform_view</code>.</li>
     *   <li>The resulting element type is <code>jh::meta::flatten_proxy</code>,
     *       which can be implicitly converted to a standard or structured
     *       <code>tuple</code> type for binding or tuple algorithms.</li>
     * </ul>
     *
     * @note
     * <p>
     * The flatten adaptor enforces structural flattening detection via structured
     * bindings. If you are using custom POD types or user-defined array wrappers
     * that are not intended to behave like tuples, they remain unflattened.
     * However, if you use standard tuple-like containers such as
     * <code>std::array</code>, <code>std::tuple</code>, <code>std::pair</code>, or their pod
     * equivalents:
     * <code>jh::pod::array</code>, <code>jh::pod::tuple</code>, <code>jh::pod::pair</code>,
     * these will be automatically flattened.
     * </p>
     * <p>
     * Be cautious when iterating over associative containers such as
     * <code>std::map</code> or <code>unordered_map</code>:
     * their iteration elements are <code>std::pair&lt;const Key, T&gt;</code>,
     * which will also be recognized as tuple-like and thus flattened into a tuple.
     * Only use <code>flatten</code> if you explicitly want such pair elements to
     * participate in flattening.
     * </p>
     *
     * @see jh::meta::flatten_proxy
     */
    inline constexpr detail::flatten_fn flatten{};

} // namespace jh::ranges::views
