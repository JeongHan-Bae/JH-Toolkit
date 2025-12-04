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
 * @file to.h (ranges)
 * @brief Container adaptation adaptor &mdash; constructs a target container <code>C</code>
 *        directly from a compatible range <code>R</code>.
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 *
 * <p>
 * The <code>jh::ranges::to</code> adaptor provides a high-efficiency way to
 * <b>directly construct</b> a container <code>C</code> from a range <code>R</code>,
 * as long as they form a valid <code>closable_container_for</code> pair.
 * </p>
 *
 * <p>
 * Conceptually, this corresponds to the <b>closable half</b> of the proposed
 * C++23 <code>std::ranges::to</code>. Unlike <code>jh::ranges::collect</code>,
 * which can always materialize a range via insertion or emplace iteration,
 * <code>to</code> requires that the container and range are <em>structurally compatible</em>
 * and can be constructed directly.
 * </p>
 *
 * <h3>Behavior overview</h3>
 * <ul>
 *   <li>If <code>C</code> can be directly constructed from <code>[begin(r), end(r)]</code>,
 *       the adaptor uses that constructor.</li>
 *   <li>If <code>C</code> supports move iterators, vector bridging, or underlying
 *       container adaptation, these strategies are automatically detected and applied.</li>
 *   <li>Constructor arguments (if any) are forwarded via <code>Args...</code>.</li>
 * </ul>
 *
 * <h3>Usage</h3>
 * @code
 * using namespace jh::ranges;
 *
 * std::vector&lt;int&gt; v = {1, 2, 3, 4};
 *
 * std::pmr::monotonic_buffer_resource pool;
 *
 * // Direct construction &mdash; requires closable_container_for&lt;std::pmr::vector&lt;int&gt;, std::vector&lt;int&gt;&gt;
 * auto pmr_vec = to&lt;std::pmr::vector&lt;int&gt;&gt;(
 *     v,
 *     std::pmr::polymorphic_allocator&lt;int&gt;(&pool)
 * );
 *
 * // Equivalent pipe form
 * auto pmr_vec2 = v
 *   | to&lt;std::pmr::vector&lt;int&gt;&gt;(std::pmr::polymorphic_allocator&lt;int&gt;(&pool));
 *
 * // When the source range is not directly closable, normalize first:
 * auto pmr_vec3 = std::views::iota(0, 5)
 *   | std::views::transform([](int x) { return x * x; })
 *   | collect&lt;std::vector&lt;int&gt;&gt;()
 *   | to&lt;std::pmr::vector&lt;int&gt;&gt;(std::pmr::polymorphic_allocator&lt;int&gt;(&pool));
 * @endcode
 *
 * <h3>Design rationale</h3>
 * <ul>
 *   <li><b>Closability-first design:</b> Only works if <code>C</code> and <code>R</code>
 *       form a valid <code>closable_container_for</code> relation.</li>
 *   <li><b>Optimal constructor dispatch:</b> Prefers native constructors, falls back to
 *       intermediate <code>std::vector</code> bridges if needed.</li>
 *   <li><b>Full pipe compatibility:</b> Works seamlessly in range pipelines
 *       with or without additional constructor parameters.</li>
 * </ul>
 * 
 * <h3>Optimization semantics: <code>collect + to</code></h3>
 * <p>
 * Under optimized compilation (C++20 and later), the combination
 * <code>collect + to</code> behaves as a <em>semantic two-stage pipeline</em> but is
 * compiled down to a near-optimal one-stage construction.  
 * Although <code>collect</code> materializes an intermediate container (usually
 * <code>std::vector&lt;T&gt;</code>), the compiler's <b>RVO</b> and <b>move-propagation</b>
 * rules ensure that no redundant copies occur for right-value ranges.
 * </p>
 *
 * <table>
 *   <tr>
 *     <th>Stage</th>
 *     <th>Responsibility</th>
 *     <th>Typical Behavior</th>
 *     <th>Optimization Result</th>
 *   </tr>
 *   <tr>
 *     <td><nobr><b>collect&lt;V&gt;()</b></nobr></td>
 *     <td>Terminates lazy views and produces a normalized, value-semantic
 *         container (e.g. <code>std::vector&lt;pair&lt;K,V&gt;&gt;</code>).</td>
 *     <td>Constructs a temporary container on the caller's stack.</td>
 *     <td>RVO creates the container directly in the caller frame; no copy or
 *         move is required.</td>
 *   </tr>
 *   <tr>
 *     <td><nobr><b>to&lt;C&gt;()</b></nobr></td>
 *     <td>Adapts the collected data into the final container <code>C</code>
 *         via its constructor (<code>[begin, end)</code>, move-iterators,
 *         or adapter dispatch).</td>
 *     <td>Passes iterators of the temporary container.</td>
 *     <td>Because the source is a right-value, the target container moves each
 *         element; no deep copy occurs.</td>
 *   </tr>
 *   <tr>
 *     <td><b>Overall pipeline</b></td>
 *     <td>Two semantic stages: materialization + construction.</td>
 *     <td>Intermediate vector exists logically but not physically duplicated.</td>
 *     <td>Equivalent runtime cost to <code>C(std::make_move_iterator(...))</code>;
 *         no extra allocations, deterministic lifetime.</td>
 *   </tr>
 * </table>
 *
 * <h4>Why this design is semantically superior</h4>
 * <ul>
 *   <li><b>Explicit materialization point:</b> The exact moment a lazy range
 *       becomes concrete is visible and controllable.</li>
 *   <li><b>Allocator and resource safety:</b> Construction parameters are isolated
 *       in <code>to</code>, preventing accidental cross-allocator moves.</li>
 *   <li><b>Predictable evaluation order:</b> No hidden eager materialization as in
 *       <code>std::ranges::to</code>.</li>
 *   <li><b>Copy-safe yet move-efficient:</b> Even though
 *       <code>return C(begin(r), end(r))</code> is specified as a copy operation,
 *       compilers perform per-element <code>move</code> for right-value containers,
 *       achieving optimal performance without losing semantic clarity.</li>
 *   <li><b>Transparent lifetime management:</b> The intermediate vector's scope is
 *       clear, making debugging and reasoning about resource ownership trivial.</li>
 * </ul>
 *
 * <p>
 * In practice, <code>collect + to</code> achieves the same performance as a
 * monolithic <code>std::ranges::to</code> call while providing stronger guarantees
 * of safety, clarity, and composability within a range pipeline.
 * </p>
 * 
 * @see jh::ranges::collect
 * @see jh::concepts::closable_container_for
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include <ranges>
#include "jh/metax/adl_apply.h"
#include "jh/conceptual/closable_container.h"


namespace jh::ranges {

    /**
     * @brief Core implementation for <code>jh::ranges::to</code> adaptor.
     * @details
     * Constructs a container <code>C</code> from range <code>R</code> when the pair
     * satisfies the <code>closable_container_for</code> concept. This includes
     * direct iterator constructors, move iterators, vector bridging, and
     * adapter-based composition (e.g. <code>std::stack&lt;std::deque&lt;T&gt;&gt;</code>).
     *
     * <p>
     * This function is selected only if the relationship between <code>C</code> and
     * <code>R</code> can be established at compile time. Otherwise, use
     * <code>jh::ranges::collect</code> as a more permissive fallback.
     * </p>
     *
     * @tparam C The target container type.
     * @tparam R The input range type.
     * @tparam Args Additional constructor argument types for <code>C</code>.
     * @param r The source range.
     * @param args Optional forwarded constructor arguments for <code>C</code>.
     * @return A fully constructed container <code>C</code>.
     *
     * @note
     * This function forms the core implementation used by both direct calls
     * and the pipe-style <code>to&lt;C&gt;()</code> adaptor.
     */
    template<typename C, std::ranges::range R, typename... Args>
    requires jh::concepts::closable_container_for<C, R, std::tuple<std::remove_cvref_t<Args>...>>
    constexpr auto to_adaptor(R &&r, Args &&... args) {
        using ArgsTuple = std::tuple<std::remove_cvref_t<Args>...>;
        using Impl = jh::concepts::detail::closable_container_for_impl<C, R, ArgsTuple>;
        constexpr auto status = Impl::status;
        using Cv = jh::concepts::container_value_t<C>;

        if constexpr (status == jh::concepts::detail::closable_status::direct_copy) {
            return C(std::ranges::begin(r), std::ranges::end(r), std::forward<Args>(args)...);
        } else if constexpr (status == jh::concepts::detail::closable_status::via_vector_whole) {
            std::vector<Cv> tmp(std::ranges::begin(r), std::ranges::end(r));
            return C(std::move(tmp), std::forward<Args>(args)...);
        } else if constexpr (status == jh::concepts::detail::closable_status::via_vector_move) {
            std::vector<Cv> tmp(std::ranges::begin(r), std::ranges::end(r));
            return C(std::make_move_iterator(tmp.begin()),
                     std::make_move_iterator(tmp.end()),
                     std::forward<Args>(args)...);
        } else if constexpr (status == jh::concepts::detail::closable_status::via_vector_copy) {
            std::vector<Cv> tmp(std::ranges::begin(r), std::ranges::end(r));
            return C(tmp.begin(), tmp.end(), std::forward<Args>(args)...);
        } else if constexpr (status == jh::concepts::detail::closable_status::adapter_via_underlying) {
            using Underlying = typename C::container_type;
            auto base = jh::ranges::to_adaptor<Underlying>(std::forward<R>(r), std::forward<Args>(args)...);
            return C(std::move(base));
        }
    }

    namespace detail {

        /**
         * @brief Pipe-compatible closure for <code>jh::ranges::to</code>.
         * @details
         * Holds constructor arguments for the target container <code>C</code>
         * and applies them when the closure is invoked via pipe syntax.
         *
         * @tparam C The target container type.
         * @tparam Args Additional constructor argument types.
         *
         * @see jh::ranges::to_fn
         * @see jh::ranges::to_adaptor
         */
        template<typename C, typename... Args>
        struct [[maybe_unused]] to_closure {
            /// @brief Stored argument tuple for forwarding into <code>to_adaptor</code>.
            std::tuple<Args...> args;

            /**
             * @brief Invoke the closure on a given range.
             * @param r The input range to be converted to <code>C</code>.
             * @return The constructed container instance.
             */
            template<std::ranges::range R>
            constexpr auto operator()([[maybe_unused]] R &&r) const {
                return jh::meta::adl_apply([&]<typename... A>(A &&... unpacked) {
                    return jh::ranges::to_adaptor<C>(std::forward<R>(r), std::forward<A>(unpacked)...);
                }, args);
            }

            /**
             * @brief Pipe operator enabling syntax: <code>range | jh::ranges::to&lt;C&gt;(args...)</code>.
             */
            template<std::ranges::range R>
            friend constexpr auto operator|(R &&lhs, const to_closure &rhs) {
                return rhs(std::forward<R>(lhs));
            }
        };
    } // namespace detail

    /**
     * @brief Function object implementing <code>jh::ranges::to</code>.
     * @details
     * Provides both direct-call and pipe-style interfaces for constructing a
     * closable container from a compatible range.
     *
     * <h4>Direct form</h4>
     * @code
     * auto dq = jh::ranges::to&lt;std::deque&lt;int&gt;&gt;(v);
     * @endcode
     *
     * <h4>Pipe form</h4>
     * @code
     * auto dq = v | jh::ranges::to&lt;std::deque&lt;int&gt;&gt;();
     * @endcode
     *
     * <h4>With extra arguments</h4>
     * @code
     * std::pmr::monotonic_buffer_resource pool;
     * auto pmr_vec = v | jh::ranges::to&lt;std::pmr::vector&lt;int&gt;&gt;(std::pmr::polymorphic_allocator&lt;int&gt;(&pool));
     * @endcode
     *
     * <p>
     * The adaptor supports additional constructor arguments for allocator- or
     * parameterized container types and automatically deduces the correct
     * construction path.
     * </p>
     *
     * @tparam C The target container type.
     * @see jh::ranges::to_adaptor
     * @see jh::ranges::to
     */
    template<typename C>
    struct to_fn {
        /// @brief Construct container <code>C</code> directly from range <code>R</code> and optional arguments.
        template<std::ranges::range R, typename... Args>
        constexpr auto operator()(R &&r, Args &&... args) const {
            return jh::ranges::to_adaptor<C>(
                    std::forward<R>(r),
                    std::forward<Args>(args)...
            );
        }

        /// @brief Return a closure object capturing constructor arguments for pipe usage.
        template<typename... Args>
        constexpr auto operator()(Args &&... args) const requires (sizeof...(Args) == 0 ||
                                                                   !(std::ranges::range<
                                                                           std::remove_cvref_t<
                                                                                   std::tuple_element_t<0, std::tuple<Args...>>
                                                                           >
                                                                   >)) {
            return detail::to_closure<C, std::remove_cvref_t<Args>...>{
                    std::tuple{std::forward<Args>(args)...}
            };
        }
    };

    /**
     * @brief Global instance of the <code>to</code> adaptor.
     * @details
     * This is the primary user-facing entry point for constructing containers
     * from compatible ranges. It supports both direct and pipe invocation forms:
     *
     * <ul>
     *   <li><b>Direct form:</b> <code>auto s = jh::ranges::to&lt;std::set&lt;int&gt;&gt;(v);</code></li>
     *   <li><b>Pipe form:</b> <code>auto dq = v | jh::ranges::to&lt;std::deque&lt;int&gt;&gt;();</code></li>
     * </ul>
     *
     * <p>
     * The adaptor automatically distinguishes between these two forms by analyzing
     * the argument category: when the first argument is a range, it performs
     * an immediate container construction; otherwise, it returns a lightweight
     * closure object suitable for pipe composition.
     * </p>
     *
     * <p>
     * This behavior is <b>intentional and specification-safe</b> &mdash; no valid
     * standard container constructor ever accepts two independent ranges,
     * therefore this heuristic introduces no ambiguity for normal user code.
     * </p>
     *
     * @note
     * Use <code>jh::ranges::collect</code> instead if the range is not directly
     * closable to the target container or involves non-copyable proxy views.
     *
     * @tparam C The container type to construct.
     * @see jh::ranges::collect
     * @see jh::ranges::to_fn
     */
    template<typename C>
    inline constexpr to_fn<C> to{};

} // namespace jh::ranges
