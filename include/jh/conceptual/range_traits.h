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
 * @file range_traits.h (conceptual)
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 * @brief Generic range-related conceptual utilities and traits.
 *
 * @details
 * This header serves as the <b>general conceptual aggregation layer</b> for
 * range-related constructs that are not tied to any specific subdomain such as
 * <code>views</code>, <code>containers</code>, or <code>sequences</code>.
 *
 * <p>
 * Concepts and traits defined here represent <em>foundational</em> or
 * <em>cross-cutting</em> abstractions used throughout the JH range system,
 * especially for:
 * </p>
 * <ul>
 *   <li>adaptor and visitor validation,</li>
 *   <li>unified value category forwarding,</li>
 *   <li>internal range storage policy deduction.</li>
 * </ul>
 *
 * <h3>Included Components</h3>
 * <ul>
 *   <li><code>vis_function_for&lt;F, R&gt;</code> — validates a range–callable
 *       pair for <em>visual traversal semantics</em>.</li>
 *   <li><code>range_storage_traits&lt;R, UseRefWrapper&gt;</code> — defines
 *       how a range is held inside wrapper-based views.</li>
 * </ul>
 *
 * <h3>Purpose</h3>
 * <p>
 * <code>jh::concepts::range_traits</code> defines cross-sectional utilities
 * required by other modules such as <code>jh::ranges_ext</code> and
 * <code>jh::views</code>, consolidating range-level mechanics that do not
 * belong to a dedicated conceptual category.
 * </p>
 *
 * <h3>Design Policy</h3>
 * <ul>
 *   <li>Acts as the conceptual catch-all for <b>range-adjacent</b> utilities.</li>
 *   <li>Maintains semantic separation from specific layers like
 *       <code>sequence</code> or <code>iterator</code>.</li>
 *   <li>Defines traits and concepts reusable across <b>range adaptors</b>,
 *       <b>transformation views</b>, and <b>range storage logic</b>.</li>
 * </ul>
 *
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */


#pragma once

#include <iterator>
#include <ranges>
#include <functional>
#include <type_traits>

namespace jh::concepts {

    /**
     * @brief Concept describing the <em>visual relation</em> between a range and a callable.
     *
     * @details
     * <code>vis_function_for&lt;F, R&gt;</code> formalizes a <b>visual visitation contract</b>:
     * both the range and the callable must satisfy conditions for a
     * <em>non-consuming, non-mutating traversal</em> with a meaningful result.
     *
     * <h4>Contract Requirements</h4>
     * <ul>
     *   <li>The range <code>R</code> supports <b>non-consuming iteration</b> —
     *       calling <code>begin()</code> and <code>end()</code> does not alter its state.</li>
     *   <li>Elements of <code>R</code> can be dereferenced as input iterators.</li>
     *   <li>The callable <code>F</code> can be safely invoked on each element:
     *       <code>std::invoke(f, *begin(r))</code> must be well-formed.</li>
     *   <li>The invocation result is <b>non-void</b>, allowing the result
     *       to participate in further pipeline or transform chaining.</li>
     *   <li>Neither the range nor its elements are modified by the operation —
     *       the relation is purely observational.</li>
     * </ul>
     *
     * <p>
     * In other words, <code>vis_function_for</code> ensures that a callable and a
     * range can participate together in a <em>visual transformation pipeline</em>
     * such as those implemented by <code>jh::ranges::vis_transform_view</code>.
     * </p>
     *
     * @tparam F Callable type to be applied visually to the range.
     * @tparam R Range type providing non-consuming elements.
     */
    template<typename F, typename R>
    concept vis_function_for = requires(const std::remove_cvref_t<R> &r, const F &f) {
        { std::begin(r) } -> std::input_iterator;
        { *std::begin(r) };
        { std::invoke(f, *std::begin(r)) };
        requires (!std::is_void_v<decltype(std::invoke(f, *std::begin(r)))>);
    };

    /**
     * @brief Trait defining how a range (or equivalent) is held inside a view or wrapper.
     *
     * @details
     * <p>
     * <code>range_storage_traits&lt;R&gt;</code> defines the storage model for
     * wrapper-based view types that internally hold another range, view, or
     * sequence — for example:
     * <code>jh::ranges::range_wrapper</code> and
     * <code>jh::ranges::vis_transform_view</code>.
     * </p>
     *
     * <h4>Policy Overview</h4>
     * <ul>
     *   <li><b>Rvalues / temporaries</b> — stored by value.</li>
     *   <li><b>Lvalues</b> — stored as references, optionally wrapped in
     *       <code>std::reference_wrapper</code> for safety.</li>
     *   <li><b>UseRefWrapper = true</b> — enforces safe reference semantics,
     *       ensuring non-dangling access in deferred pipelines.</li>
     *   <li><b>UseRefWrapper = false</b> — uses direct references for
     *       lightweight, local usage.</li>
     * </ul>
     *
     * <p>
     * This trait allows range-holding views to remain agnostic to the
     * lifetime and category of their sources, achieving consistent forwarding
     * semantics across the entire <code>jh::ranges</code> ecosystem.
     * </p>
     *
     * @tparam R Source range type to be stored.
     * @tparam UseRefWrapper Whether lvalues should be stored as
     *         <code>std::reference_wrapper</code>.
     */
    template<typename R, bool UseRefWrapper = false>
    struct range_storage_traits {
        /// @brief Range/View/Sequence type with cv/ref qualifiers removed.
        using raw_t [[maybe_unused]] = std::remove_cvref_t<R>;

        /// @brief Indicates whether the source is an lvalue reference.
        static constexpr bool is_lvalue = std::is_lvalue_reference_v<R>;

        /// @brief Type used internally to hold the source, based on reference category and policy.
        using stored_t = std::conditional_t<
                is_lvalue,
                std::conditional_t<UseRefWrapper,
                        std::reference_wrapper<std::remove_reference_t<R>>,
                        R>,
                std::remove_cvref_t<R>
        >;

        /**
         * @brief Wraps a range/view/sequence according to the defined storage policy.
         * @param v The source range to wrap.
         * @return Stored representation (value or reference) suitable for internal retention.
         */
        static constexpr auto wrap(R &&v) noexcept(
        std::is_nothrow_constructible_v<stored_t, R &&>
        ) {
            if constexpr (is_lvalue && UseRefWrapper) {
                if constexpr (std::is_const_v<std::remove_reference_t<R>>)
                    return std::cref(v);
                else
                    return std::ref(v);
            } else if constexpr (is_lvalue)
                return v;
            else
                return std::move(v);
        }

        /**
         * @brief Retrieves a reference to the underlying source.
         * @param v Stored instance.
         * @return Constant / Mutable reference to the source.
         */
        static constexpr decltype(auto) get(stored_t &v) noexcept {
            if constexpr (is_lvalue && UseRefWrapper)
                return v.get();
            else
                return (v);
        }

        /**
         * @brief Retrieves a reference to the underlying source.
         * @param v Stored instance.
         * @return Constant / Mutable reference to the source.
         */
        static constexpr decltype(auto) get(const stored_t &v)
        noexcept requires (!std::is_reference_v<stored_t>) {
            if constexpr (is_lvalue && UseRefWrapper)
                return v.get();
            else
                return (v);
        }
    };
} // namespace jh::concepts
