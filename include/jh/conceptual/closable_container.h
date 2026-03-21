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
 * @file closable_container.h
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 * 
 * @brief Compile-time deduction of container <tt>closability</tt> &mdash; determining
 *        whether and how a container <code>C</code> can be directly constructed
 *        ("closed") from a range <code>R</code>.
 *
 * <p>
 * This header defines the <b>closable container model</b> &mdash; the conceptual basis
 * for <code>jh::ranges::to</code> and its companion <code>jh::ranges::collect</code>.
 * It classifies all possible construction paths between a container and a range,
 * forming the foundation for the <code>closable_container_for</code> concept.
 * </p>
 *
 * <h3>Purpose</h3>
 * <p>
 * In a range pipeline, a container <tt>closable</tt> from a range means that
 * it can be built directly via iterators or an intermediate vector bridge,
 * without explicit element-wise insertion. This allows the <code>to</code>
 * adaptor to perform efficient, single-step construction.
 * </p>
 *
 * <h3>Design rationale</h3>
 * <ul>
 *   <li><b>Declarative deduction:</b> All possible constructor paths are examined
 *       at compile time via <code>requires</code> expressions.</li>
 *   <li><b>Non-intrusive:</b> No specialization or overload resolution is required
 *       on the user's part.</li>
 *   <li><b>Deterministic:</b> The most specific viable construction path is chosen
 *       in a strictly ordered priority chain.</li>
 * </ul>
 *
 * <h3>Move semantics and design constraint</h3>
 * <p>
 * <b>Directly moving (consuming) the source range is semantically invalid.</b>
 * This restriction aligns with the intent of <code>std::ranges::to</code> and
 * with the design of <code>jh::ranges::to</code>:  
 * a range adaptor must never invalidate its source by stealing its state.
 * Instead, all construction paths are <tt>iterator-based</tt> and
 * <tt>non-destructive</tt>, preserving the observable validity of the source
 * range.
 * </p>
 *
 * <p>
 * In the JH framework, <code>collect</code> and <code>to</code> form a
 * <b>two-phase adaptation model</b>:
 * </p>
 * <ol>
 *   <li><b><code>collect&lt;V&gt;()</code></b> &mdash; explicitly materializes any lazy
 *       or proxy-based range into a stable, value-semantic container <code>V</code>.</li>
 *   <li><b><code>to&lt;C&gt;()</code></b> &mdash; constructs the final container
 *       <code>C</code> from that materialization.</li>
 * </ol>
 *
 * <h4>Performance note</h4>
 * <p>
 * Although <code>closable_container_for</code> never performs a direct move of
 * the source object, this design imposes <b>no runtime penalty</b>.  
 * When the source range is a prvalue (e.g. produced by <code>collect</code>),
 * modern C++ compilers guarantee through <b>RVO</b> and <b>NRVO</b> that the
 * intermediate container is constructed in-place, and element-wise initialization
 * automatically uses <b>move semantics</b> rather than copy.  
 * Therefore, <code>collect + to</code> achieves the same runtime efficiency
 * as a single monolithic move construction, while preserving precise and
 * standard-compliant semantics.
 * </p>
 *
 * <h3>Core entities</h3>
 * <ul>
 *   <li><code>jh::concepts::detail::closable_status</code> &mdash;
 *       enumerates all construction modes.</li>
 *   <li><code>jh::concepts::detail::compute_closable_status&lt;C, R&gt;</code> &mdash;
 *       evaluates and selects the appropriate mode at compile time.</li>
 *   <li><code>jh::concepts::closable_container_for&lt;C, R&gt;</code> &mdash;
 *       concept determining if a container is closable from a range.</li>
 * </ul>
 *
 * <h3>Relationship with other modules</h3>
 * <ul>
 *   <li><b><code>jh::ranges::to</code></b> &mdash; consumes closable pairs for
 *       direct construction.</li>
 *   <li><b><code>jh::ranges::collect</code></b> &mdash; produces closable,
 *       value-semantic intermediates.</li>
 * </ul>
 *
 * @note
 * The <code>closable_container_for</code> concept is <b>purely structural</b>:
 * it verifies, at compile time, that a container <em>can</em> be safely and
 * deterministically constructed from a range, without performing any move or
 * mutation of the source itself.
 *
 * @see jh::ranges::collect
 * @see jh::ranges::to
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include <cstdint>
#include <ranges>
#include <type_traits>
#include <vector>
#include "jh/conceptual/container_traits.h"

namespace jh::concepts::detail {

    /**
     * @brief Classification of how a container <code>C</code> can be constructed ("closed")
     *        from a range <code>R</code>.
     */
    enum class closable_status : std::uint8_t {
        none = 0,               ///< Not closable

        // --- Direct constructions ---
        direct_copy,            ///< <code>C(begin, end)</code>

        // --- Via vector bridge ---
        via_vector_whole,       ///< <code>C(vector(...))</code>
        via_vector_move,        ///< <code>C(make_move_iterator(vector(...).begin()), ...)</code>
        via_vector_copy,        ///< <code>C(vector(...).begin(), vector(...).end())</code>

        // --- Adapter wrapping ---
        adapter_via_underlying  ///< e.g. stack(queue) built from its <code>container_type</code>
    };

    /**
     * @brief Internal helper: apply a callable <code>F</code> to each tuple element type.
     * @tparam Tuple tuple-like type
     * @tparam F callable providing templated <code>operator&lt;&gt;</code>
     */
    template<typename Tuple, typename F, std::size_t... I>
    consteval auto tuple_apply_impl(F &&f, std::index_sequence<I...>) {
        return f.template operator()<std::tuple_element_t<I, Tuple>...>();
    }

    /**
     * @brief Convenience wrapper generating index sequence automatically.
     */
    template<typename Tuple, typename F>
    consteval auto tuple_apply(F &&f) {
        return tuple_apply_impl<Tuple>(std::forward<F>(f),
                                       std::make_index_sequence<std::tuple_size_v<Tuple>>{});
    }

    /**
     * @brief Compute the <code>closable_status</code> of a container-range pair.
     *
     * @details
     * Evaluates, at compile time, whether a container <code>C</code> can be
     * directly or indirectly constructed from a range <code>R</code>, and if so,
     * identifies the applicable construction strategy.
     *
     * This function underpins <code>closable_container_for</code> and is not
     * intended for direct user use.
     *
     * @tparam C container type
     * @tparam R range type
     * @tparam ArgsTuple optional tuple of additional constructor argument types
     * @return corresponding <code>closable_status</code> value
     */
    template<typename C, typename R, typename ArgsTuple = std::tuple<>>
    consteval closable_status compute_closable_status() {
        using Cv = jh::concepts::container_value_t<C>;
        using Rv = std::ranges::range_reference_t<R>;

        if constexpr (std::same_as<Cv, void> || std::same_as<Rv, void>)
            return closable_status::none;

        constexpr bool type_convertible =
                std::is_constructible_v<Cv, Rv> || std::is_convertible_v<Rv, Cv>;
        if constexpr (!type_convertible)
            return closable_status::none;

        return tuple_apply<ArgsTuple>([]<typename... Args>() consteval {
            // --- direct_copy ---
            if constexpr (requires([[maybe_unused]] R &&r) {
                C(std::ranges::begin(r), std::ranges::end(r),
                  std::declval<Args>()...);
            })
                return closable_status::direct_copy;

            // --- via_vector_whole ---
            if constexpr (requires([[maybe_unused]] R &&r) {
                C(std::vector<Cv>(std::ranges::begin(r), std::ranges::end(r)),
                  std::declval<Args>()...);
            })
                return closable_status::via_vector_whole;

            // --- via_vector_move ---
            if constexpr ((requires([[maybe_unused]] R &&r) {
                // Step 1: R can be materialized into std::vector<Cv>
                std::vector<Cv>(std::ranges::begin(r), std::ranges::end(r));
            } && requires([[maybe_unused]] std::vector<Cv> &&v) {
                // Step 2: C can be move-constructed from that vector's iterators
                C(std::make_move_iterator(v.begin()),
                  std::make_move_iterator(v.end()),
                  std::declval<Args>()...);
            }
            ))
                return closable_status::via_vector_move;

            // --- via_vector_copy ---
            if constexpr ((requires([[maybe_unused]] R &&r) {
                // Step 1: R can be materialized into std::vector<Cv>
                std::vector<Cv>(std::ranges::begin(r), std::ranges::end(r));
            } && requires([[maybe_unused]] std::vector<Cv> &&v) {
                // Step 2: C can be copy-constructed from that vector's iterators
                C(v.begin(), v.end(), std::declval<Args>()...);
            }
            ))
                return closable_status::via_vector_copy;

            // --- adapter_via_underlying ---
            if constexpr (requires { typename C::container_type; }) {
                using Underlying = typename C::container_type;
                constexpr auto sub = compute_closable_status<Underlying, R, ArgsTuple>();
                if constexpr (sub != closable_status::none) {
                    if constexpr (std::is_constructible_v<C, Underlying> ||
                                  std::is_constructible_v<C, Underlying &&> ||
                                  std::is_constructible_v<C, const Underlying &>)
                        return closable_status::adapter_via_underlying;
                }
            }

            return closable_status::none;
        });
    }

    /**
     * @brief Internal trait: holds the compile-time closable status of a container-range pair.
     * @tparam C container type
     * @tparam R range type
     * @tparam ArgsTuple optional constructor argument tuple
     */
    template<typename C, typename R, typename ArgsTuple = std::tuple<>>
    struct closable_container_for_impl {
        static constexpr closable_status status = compute_closable_status<C, R, ArgsTuple>();
        static constexpr bool value = status != closable_status::none;
    };

} // namespace jh::concepts::detail


namespace jh::concepts {

    /**
     * @brief Concept checking whether a container <code>C</code> can be directly
     *        constructed ("closed") from a range <code>R</code>.
     *
     * @details
     * This concept verifies, at compile time, that a container type <code>C</code>
     * can be constructed from an input range <code>R</code> either directly
     * (via iterator constructor) or indirectly (through intermediate
     * vectorization or adapter construction).
     *
     * <h4>Requirements</h4>
     * <ul>
     *   <li><code>R</code> must model <code>std::ranges::input_range</code>.</li>
     *   <li><code>detail::compute_closable_status&lt;C, R&gt;</code> must return
     *       a non-<code>none</code> value.</li>
     * </ul>
     *
     * <h4>Equivalent to</h4>
     * <code>std::ranges::input_range&lt;R&gt; &amp;&amp;
     * detail::closable_container_for_impl&lt;C, R&gt;::value</code>
     *
     * @tparam C candidate container type
     * @tparam R input range type
     * @tparam ArgsTuple optional tuple of constructor argument types
     */
    template<typename C, typename R, typename ArgsTuple = std::tuple<>>
    concept closable_container_for =
    std::ranges::input_range<R> &&
    detail::closable_container_for_impl<C, R, ArgsTuple>::value;

} // namespace jh::concepts
