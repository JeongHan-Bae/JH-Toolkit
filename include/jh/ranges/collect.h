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
 * @brief Eager materialization adaptor &mdash; explicitly terminates a lazy range pipeline
 *        and realizes it into a concrete container <code>C</code>.
 * @author
 *   JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 *
 * <p>
 * The <code>jh::ranges::collect</code> adaptor provides an <b>explicit</b> and
 * <b>controlled</b> way to materialize a lazy range into a concrete container.
 * It represents the <b>eager half</b> of <code>std::ranges::to</code>: while
 * <code>to</code> performs direct construction of a <em>closable</em> container,
 * <code>collect</code> enforces <b>explicit evaluation</b> of any lazy or
 * proxy-based range and yields a stable, value-semantic container.
 * </p>
 *
 * <h3>Behavior overview</h3>
 * <ul>
 *   <li>If <code>C</code> and <code>R</code> satisfy
 *       <code>closable_container_for&lt;C, R&gt;</code>, it delegates directly to
 *       <code>jh::ranges::to_adaptor&lt;C&gt;</code>.</li>
 *   <li>Otherwise, it performs element-wise insertion according to
 *       <code>collectable_status</code> deduction, supporting the four canonical
 *       insertion forms aligned with the proposed <code>std::ranges::to</code>:
 *       <ul>
 *         <li><b><code>emplace_back()</code></b></li>
 *         <li><b><code>push_back()</code></b></li>
 *         <li><b><code>emplace()</code></b></li>
 *         <li><b><code>insert()</code></b></li>
 *       </ul>
 *       These cover virtually all standard and third-party container families.</li>
 *   <li>If <code>C</code> provides <code>reserve()</code> and <code>R</code> models
 *       <code>sized_range</code>, capacity is automatically preallocated.</li>
 * </ul>
 *
 * <h3>Tuple unpacking and reconstruction</h3>
 * <p>
 * When none of the direct or implicit construction paths apply, and the range's
 * element type is <em>tuple-like</em>, <code>collect</code> attempts a fallback
 * reconstruction step: it unpacks the tuple-like element via
 * <code>jh::meta::adl_apply</code> into <code>emplace_back()</code> or
 * <code>emplace()</code> calls.
 * This mechanism is unique to <code>collect</code> &mdash; the standard
 * <code>std::ranges::to</code> does not perform such unpacking.
 * </p>
 *
 * @code
 * auto result = input_strings
 *   | jh::ranges::views::enumerate()
 *   | jh::ranges::collect&lt;std::vector&lt;std::pair&lt;size_t, std::string&gt;&gt;&gt;()
 *   | jh::ranges::to&lt;std::pmr::unordered_map&lt;size_t, std::string&gt;&gt;(
 *         0,
 *         std::hash&lt;size_t&gt;{},
 *         std::equal_to&lt;size_t&gt;{},
 *         alloc
 *     );
 * @endcode
 *
 * <p>
 * Here, <code>enumerate()</code> (implemented via jh::ranges::zip_view and std::iota) yields a
 * tuple-like proxy (zip_reference_proxy) combining the index and
 * string reference; <code>collect</code> detects that it cannot insert the
 * proxy directly, unpacks it, and reconstructs real
 * <code>std::pair&lt;size_t, std::string&gt;</code> objects.
 * </p>
 *
 * <h3>Semantic role</h3>
 * <p>
 * <code>collect</code> defines the <b>explicit evaluation boundary</b> within a
 * lazy pipeline &mdash; it marks where deferred computations stop and data becomes
 * concrete.
 * This is crucial when interacting with <code>std::views::transform</code> or
 * other lazy adaptors that convert a range into a transient, consumptive stream.
 * By forcing evaluation, <code>collect</code> ensures that the data is
 * materialized and safe from dangling or deferred access.
 * </p>
 *
 * <ul>
 *   <li><b>Explicit materialization:</b> forces evaluation of lazy pipelines.</li>
 *   <li><b>Type normalization:</b> resolves proxy and reference wrappers into
 *       value objects.</li>
 *   <li><b>Tuple fallback:</b> reconstructs unpacked objects when direct
 *       construction is not viable.</li>
 * </ul>
 *
 * <h3>Argument policy</h3>
 * <p>
 * <code>collect</code> does <b>not</b> accept additional constructor arguments.
 * It performs data normalization only &mdash; all container-specific configuration
 * (e.g. allocators, hashers, comparators) belongs to <code>jh::ranges::to</code>.
 * </p>
 *
 * @code
 * // Correct pipeline:
 * auto result = lazy_view
 *   | jh::ranges::collect&lt;std::vector&lt;Key, Val&gt;&gt;()
 *   | jh::ranges::to&lt;std::pmr::unordered_map&lt;Key, Val&gt;&gt;(
 *         0,
 *         std::hash&lt;Key&gt;{},
 *         std::equal_to&lt;Key&gt;{},
 *         alloc
 *     );
 * @endcode
 *
 * <p>
 * Do <b>not</b> use <code>std::move()</code> between <code>collect</code> and
 * <code>to</code> &mdash; it provides no benefit.
 * The two adaptors are designed to compose directly in a pipeline;
 * move construction is handled automatically via RVO/NRVO.
 * See the <code>to</code> adaptor documentation for detailed move semantics.
 * </p>
 *
 * <h3>Direct completion</h3>
 * <p>
 * If your target container is a standard type that does not require extra
 * constructor parameters (e.g. <code>std::vector</code>,
 * <code>std::set</code>, <code>std::unordered_map</code>), you can use
 * <code>collect&lt;C&gt;</code> directly as the final stage.
 * Performance will be identical to <code>to</code> because, when possible,
 * <code>collect</code> automatically dispatches to
 * <code>to_adaptor&lt;C&gt;</code> internally.
 * </p>
 *
 * <h3>Design rationale</h3>
 * <ul>
 *   <li><b>Evaluation control:</b> explicitly terminates lazy or consumptive pipelines.</li>
 *   <li><b>Unified insertion model:</b> supports four canonical insertion forms
 *       and extends them with tuple-based reconstruction.</li>
 *   <li><b>Predictable semantics:</b> forbids extra arguments, ensuring
 *       unambiguous materialization.</li>
 *   <li><b>Composable design:</b> integrates seamlessly with
 *       <code>jh::ranges::to</code> for final adaptation.</li>
 * </ul>
 *
 * <h4>Relation to <code>jh::ranges::to</code></h4>
 * <p>
 * <code>collect</code> focuses on <b>materialization</b> &mdash; forcing a lazy range
 * into stable storage.
 * <code>to</code> focuses on <b>adaptation</b> &mdash; constructing the final container,
 * possibly with configuration parameters.
 * </p>
 *
 * <ol>
 *   <li><code>collect&lt;V&gt;()</code> &mdash; eagerly realize and normalize data.</li>
 *   <li><code>to&lt;C&gt;(...)</code> &mdash; adapt and construct the final container.</li>
 * </ol>
 *
 * <p>
 * Together they form a deterministic two-phase pipeline, separating lazy
 * evaluation from container adaptation for clarity, safety, and composability.
 * </p>
 *
 * @note
 * <code>collect</code> is more permissive than <code>to</code>: it accepts any
 * range that supports minimal insertion semantics, and provides additional
 * tuple-unpacking fallback paths.
 * However, because it forbids extra arguments, configuration such as allocators
 * or policies must be handled by <code>to</code>.
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
#include "jh/metax/adl_apply.h"


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
     *   <li><b>insert()</b> &mdash; for associative containers (e.g. <code>std::set</code>).</li>
     *   <li><b>emplace()</b> &mdash; for emplace-enabled containers (e.g. <code>std::unordered_set</code>).</li>
     *   <li><b>emplace_back()</b> &mdash; for sequence containers (e.g. <code>std::vector</code>, <code>std::deque</code>).</li>
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
            // prefer direct closable construction
            return jh::ranges::to_adaptor<C>(std::forward<R>(r));
        } else {
            C c{}; // guaranteed NRVO, only need default constructor

            // reserve if available
            if constexpr (requires(C &cont, std::size_t n) { cont.reserve(n); } &&
            std::ranges::sized_range<R>) {
                c.reserve(std::ranges::size(r));
            }

            for (auto &&e: r) {
                if constexpr (status == jh::concepts::detail::collectable_status::emplace_back_direct) {
                    c.emplace_back(std::forward<decltype(e)>(e));
                } else if constexpr (status == jh::concepts::detail::collectable_status::push_back_direct) {
                    c.push_back(std::forward<decltype(e)>(e));
                } else if constexpr (status == jh::concepts::detail::collectable_status::emplace_direct) {
                    c.emplace(std::forward<decltype(e)>(e));
                } else if constexpr (status == jh::concepts::detail::collectable_status::insert_direct) {
                    c.insert(std::forward<decltype(e)>(e));
                } else if constexpr (status == jh::concepts::detail::collectable_status::emplace_back_unpack) {
                    // tuple-like element, unpack into emplace_back
                    jh::meta::adl_apply(
                            [&](auto &&... args) {
                                c.emplace_back(std::forward<decltype(args)>(args)...);
                            },
                            std::forward<decltype(e)>(e));
                } else if constexpr (status == jh::concepts::detail::collectable_status::emplace_unpack) {
                    // tuple-like element, unpack into emplace
                    jh::meta::adl_apply(
                            [&](auto &&... args) {
                                c.emplace(std::forward<decltype(args)>(args)...);
                            },
                            std::forward<decltype(e)>(e));
                }
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
     *   <li><b>Direct form:</b> <code>auto v = jh::ranges::collect&lt;std::vector&lt;int&gt;&gt;(input);</code></li>
     *   <li><b>Pipe form:</b> <code>auto v = range | jh::ranges::collect&lt;std::vector&lt;int&gt;&gt;();</code></li>
     * </ul>
     *
     * @note
     * <p><b>Recommendation:</b></p> If your goal is to explicitly materialize a lazy pipeline,
     * <code>std::vector</code> is usually the best target container &mdash; it provides
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
