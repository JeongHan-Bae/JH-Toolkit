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
 * @file collect.h (ranges)
 * @brief Eager materialization adaptor for range pipelines — explicit termination of lazy views.
 * @author
 *   JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 *
 * <p>
 * The <code>jh::ranges::collect</code> adaptor provides a <b>controlled and explicit</b>
 * way to <em>materialize</em> a range pipeline into a concrete container <code>C</code>.
 * </p>
 *
 * <p>
 * Conceptually, <code>collect</code> represents the <b>eager half</b> of the proposed
 * C++23 <code>std::ranges::to</code>. While the standard function fuses materialization
 * and container adaptation into a single call, <code>jh::ranges::collect</code>
 * separates these responsibilities:
 * </p>
 * <ul>
 *   <li><b><code>collect&lt;C&gt;</code></b> – performs materialization, producing a fully
 *       realized container instance of type <code>C</code>.</li>
 *   <li><b><code>to&lt;C&gt;</code></b> – performs container adaptation only when
 *       the container and range are already <em>closable</em>
 *       (i.e., directly constructible via <code>closable_container_for</code>).</li>
 * </ul>
 *
 * <p>
 * This separation allows <b>fine-grained control</b> over lazy evaluation boundaries,
 * ensuring that intermediate <code>view</code> layers (such as
 * <code>transform_view</code> or <code>flatten_view</code>) can be explicitly
 * finalized into concrete storage before further adaptation.
 * </p>
 *
 * <h3>Behavior overview</h3>
 * <ul>
 *   <li>If <code>C</code> and the input range <code>R</code> satisfy
 *       <code>closable_container_for&lt;C, R&gt;</code>, <code>collect</code>
 *       delegates directly to <code>jh::ranges::to_adaptor&lt;C&gt;</code>.</li>
 *   <li>Otherwise, it performs a generic insertion or emplace iteration
 *       based on <code>collectable_status</code> deduction.</li>
 *   <li>If <code>C</code> supports <code>reserve()</code> and <code>R</code>
 *       is a <code>sized_range</code>, capacity is reserved automatically.</li>
 * </ul>
 *
 * <h3>Usage</h3>
 * @code
 * using namespace jh::ranges;
 *
 * std::vector&lt;int&gt; v = {1, 2, 3};
 *
 * // Direct form:
 * auto s = collect&lt;std::set&lt;int&gt;&gt;(v);
 *
 * // Pipe form:
 * auto dq = v | collect&lt;std::deque&lt;int&gt;&gt;();
 *
 * // Combine with views:
 * auto r = input
 *        | jh::ranges::views::zip(other)
 *        | jh::ranges::views::enumerate(100)
 *        | jh::ranges::views::flatten()
 *        | jh::ranges::collect&lt;std::vector&lt;std::tuple&lt;int, char, char&gt;&gt;&gt;()
 *        | jh::ranges::to&lt;std::deque&lt;std::tuple&lt;int, char, char&gt;&gt;&gt;();
 * @endcode
 *
 * <h3>Design rationale</h3>
 * <ul>
 *   <li><b>Explicit materialization point:</b> marks where the lazy evaluation chain ends.</li>
 *   <li><b>Composable pipeline:</b> integrates seamlessly into the range adaptor model.</li>
 *   <li><b>Closability awareness:</b> reuses direct constructors when possible for efficiency.</li>
 *   <li><b>Container independence:</b> works for both standard and custom containers that
 *       satisfy <code>collectable_container_for</code>.</li>
 * </ul>
 *
 * @note
 * Unlike <code>jh::ranges::to</code>, which requires a fully <em>closable</em> container–range pair,
 * <code>collect</code> is more permissive and can operate on any <em>collectable</em> range,
 * materializing even partially defined or proxy-based views (such as <code>transform_view</code>)
 * that would otherwise be non-closable.
 *
 * @see jh::ranges::to
 * @see jh::concepts::collectable_container_for
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */


#pragma once

#include <ranges>
#include "jh/conceptual/collectable_container.h"
#include "jh/ranges/to.h"


namespace jh::ranges {

    /**
     * @brief Core implementation for <code>jh::ranges::collect</code> adaptor.
     * @details
     * Performs eager materialization of a range <code>R</code> into container <code>C</code>.
     *
     * <p>
     * If <code>C</code> and <code>R</code> satisfy
     * <code>jh::concepts::closable_container_for&lt;C, R&gt;</code>, the operation
     * directly delegates to <code>jh::ranges::to_adaptor&lt;C&gt;</code> for
     * optimal construction. Otherwise, it iterates through the range and fills
     * <code>C</code> using one of the available mechanisms:
     * </p>
     * <ul>
     *   <li><b>insert()</b> — for associative containers (e.g. <code>std::set</code>).</li>
     *   <li><b>emplace()</b> — for emplace-enabled containers (e.g. <code>std::unordered_set</code>).</li>
     *   <li><b>emplace_back()</b> — for sequence containers (e.g. <code>std::vector</code>, <code>std::deque</code>).</li>
     * </ul>
     *
     * <p>
     * If the container supports <code>reserve()</code> and the range models
     * <code>sized_range</code>, capacity is preallocated automatically.
     * </p>
     *
     * @tparam C The target container type to be constructed.
     * @tparam R The input range type to be materialized.
     * @param r The input range instance to be collected.
     * @return A fully constructed container <code>C</code> containing all elements from <code>r</code>.
     *
     * @note
     * This function forms the core of the <code>collect</code> pipeline adaptor.
     * In most user code, it is preferable to use <code>jh::ranges::collect&lt;C&gt;</code>
     * directly instead of invoking this function manually.
     */
    template<typename C, std::ranges::range R>
    requires jh::concepts::collectable_container_for<C, R>
    constexpr auto collect_adaptor(R &&r) {
        using Impl = jh::concepts::detail::collectable_container_for_impl<C, R>;
        constexpr auto status = Impl::status;

        if constexpr (status == jh::concepts::detail::collectable_status::closable) {
            // prefer closable construction
            return jh::ranges::to_adaptor<C>(std::forward<R>(r));
        } else {
            C c{}; // guaranteed NRVO, only need default constructor
            if constexpr (requires(C &cont, std::size_t n) { cont.reserve(n); } &&
            std::ranges::sized_range<R>) {
                c.reserve(std::ranges::size(r));
            }
            for (auto &&e: r) {
                if constexpr (status == jh::concepts::detail::collectable_status::insert)
                    c.insert(e);
                else if constexpr (status == jh::concepts::detail::collectable_status::emplace)
                    c.emplace(e);
                else if constexpr (status == jh::concepts::detail::collectable_status::emplace_back)
                    c.emplace_back(e);
            }
            return c;
        }
    }

    namespace detail {

        /**
         * @brief Pipe-compatible closure type for <code>jh::ranges::collect</code>.
         * @details
         * This closure enables pipe syntax usage:
         * @code
         * auto c = range | jh::ranges::collect<std::vector<int>>();
         * @endcode
         *
         * Internally forwards to <code>jh::ranges::collect_adaptor</code> for execution.
         *
         * @tparam C The target container type to be collected into.
         *
         * @see jh::ranges::collect_fn
         * @see jh::ranges::collect_adaptor
         */
        template<typename C>
        struct collect_closure {
            /**
             * @brief Invoke the closure on a given range.
             * @param r The range to be materialized into <code>C</code>.
             * @return The resulting container instance.
             */
            template<std::ranges::range R>
            constexpr auto operator()(R &&r) const {
                return jh::ranges::collect_adaptor<C>(std::forward<R>(r));
            }

            /**
             * @brief Pipe operator overload enabling <code>range | collect&lt;C&gt;()</code>.
             */
            template<std::ranges::range R>
            friend constexpr auto operator|(R &&lhs, const collect_closure &rhs) {
                return rhs(std::forward<R>(lhs));
            }
        };
    } // namespace detail

    /**
     * @brief Function object implementing <code>jh::ranges::collect</code>.
     * @details
     * Provides both direct-call and pipe-style interfaces for range materialization.
     *
     * <h4>Direct call form</h4>
     * @code
     * auto vec = jh::ranges::collect&lt;std::vector&lt;int&gt;&gt;(range);
     * @endcode
     *
     * <h4>Pipe form</h4>
     * @code
     * auto vec = range | jh::ranges::collect&lt;std::vector&lt;int&gt;&gt;();
     * @endcode
     *
     * <p>
     * When the range is <em>closable</em> to <code>C</code>, this function uses
     * <code>jh::ranges::to_adaptor&lt;C&gt;</code> internally; otherwise, it falls
     * back to insertion/emplacement iteration.
     * </p>
     *
     * @tparam C The target container type.
     * @see jh::ranges::collect_adaptor
     * @see jh::ranges::collect
     */
    template<typename C>
    struct collect_fn {
        /// @brief Eagerly materialize the provided range into container type <code>C</code>.
        template<std::ranges::range R>
        constexpr auto operator()(R &&r) const {
            return jh::ranges::collect_adaptor<C>(std::forward<R>(r));
        }

        /// @brief Return a closure object for pipe usage syntax.
        constexpr auto operator()() const {
            return detail::collect_closure<C>{};
        }
    };

    /**
     * @brief Global instance of the <code>collect</code> adaptor.
     * @details
     * This is the primary user-facing interface for range materialization.
     * It supports both direct and pipe usage styles:
     *
     * Supports both <b>direct</b> and <b>pipe</b> usage forms:
     * <ul>
     *   <li><b>Direct form:</b> <tt>auto v = jh::ranges::collect&lt;std::vector&lt;int&gt;&gt;(input);</tt></li>
     *   <li><b>Pipe form:</b> <tt>auto v = range | jh::ranges::collect&lt;std::vector&lt;int&gt;&gt;();</tt></li>
     * </ul>
     *
     * @note
     * <p><b>Recommendation:</b></p> If your goal is to explicitly materialize a lazy pipeline,
     * <code>std::vector</code> is usually the best target container — it provides
     * optimal contiguous storage and can be seamlessly passed to a subsequent
     * <code>to&lt;C&gt;</code> stage for further conversion.
     *
     * @tparam C The container type to collect into.
     * @see jh::ranges::to
     * @see jh::ranges::collect_fn
     */
    template<typename C>
    inline constexpr collect_fn<C> collect{};

} // namespace jh::ranges
