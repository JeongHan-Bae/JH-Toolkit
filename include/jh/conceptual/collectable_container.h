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
 * @file collectable_container.h (conceptual)
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 *
 * @brief Compile-time deduction of <em>collectable containers</em> &mdash; determining
 *        whether and how a container <code>C</code> can accept elements of a range
 *        <code>R</code> through incremental, value-preserving insertion.
 *
 * <p>
 * This header defines the <b>collectable container model</b>, which forms the
 * foundation of <code>jh::ranges::collect</code>.
 * It classifies all containers that can <em>materialize</em> data from a range
 * using well-defined insertion semantics, possibly including tuple-like
 * structural unpacking.
 * </p>
 *
 * <h3>Purpose</h3>
 * <p>
 * A container is <em>collectable</em> from a range when it can be
 * <b>incrementally populated</b> with the elements of that range, without requiring
 * any external constructor arguments.
 * This mechanism powers <code>jh::ranges::collect</code>, which performs
 * <b>data materialization and normalization</b> &mdash; converting a lazy or proxy-based
 * pipeline into a concrete, value-semantic container.
 * </p>
 *
 * <p>
 * In contrast, <code>jh::ranges::to</code> performs <b>container adaptation</b>:
 * it builds the final target container, possibly with allocators, hashers, or
 * custom constructor parameters.
 * Therefore:
 * </p>
 * <ul>
 *   <li><code>collect</code> has <b>no extra arguments</b>; it only materializes
 *       and normalizes data.</li>
 *   <li><code>to</code> may take constructor arguments and performs the final
 *       adaptation step.</li>
 *   <li>The combination <code>collect + to</code> forms a complete
 *       <em>materialization &rarr; adaptation</em> pipeline.</li>
 * </ul>
 *
 * <h3>Classification</h3>
 * <p>
 * The <code>collectable_status</code> enumeration defines all supported behaviors:
 * </p>
 * <ul>
 *   <li><b><code>closable</code></b> &mdash; the container is directly constructible via
 *       <code>jh::ranges::to</code>.</li>
 *   <li><b><code>emplace_back_direct</code></b> &mdash; appends elements using
 *       <code>emplace_back()</code>.</li>
 *   <li><b><code>push_back_direct</code></b> &mdash; appends using
 *       <code>push_back()</code>.</li>
 *   <li><b><code>emplace_direct</code></b> &mdash; inserts at logical end using
 *       <code>emplace()</code>.</li>
 *   <li><b><code>insert_direct</code></b> &mdash; inserts at logical end using
 *       <code>insert()</code>.</li>
 *   <li><b><code>emplace_back_unpack</code></b> &mdash; performs tuple-like structural
 *       unpacking into <code>emplace_back()</code>.</li>
 *   <li><b><code>emplace_unpack</code></b> &mdash; performs tuple-like structural
 *       unpacking into <code>emplace()</code>.</li>
 * </ul>
 *
 * <h3>Design rationale</h3>
 * <ul>
 *   <li><b>Aligned semantics:</b>
 *       The four direct forms (<code>emplace_back</code>, <code>push_back</code>,
 *       <code>emplace</code>, <code>insert</code>) exactly correspond to the
 *       element-wise insertion semantics defined for <code>std::ranges::to</code>,
 *       ensuring identical behavior in the incremental case.</li>
 *   <li><b>Tuple-aware extension:</b>
 *       The unpacking forms extend the capability beyond the standard proposal:
 *       <code>collect</code> can destructure tuple-like elements (with
 *       <code>std::tuple_size</code>, <code>std::tuple_element</code>, and ADL
 *       <code>get()</code>) and reconstruct real value objects directly in-place.</li>
 *   <li><b>Non-destructive semantics:</b>
 *       No operation may move-from or invalidate the source range; insertion is
 *       performed purely by reference traversal.</li>
 * </ul>
 *
 * <h3>Unpack constraints</h3>
 * <p>
 * Unpacking requires the element type to be an <b>explicitly tuple-like</b> type,
 * providing all of:
 * </p>
 * <ul>
 *   <li><code>std::tuple_size&lt;T&gt;</code></li>
 *   <li><code>std::tuple_element&lt;I, T&gt;</code></li>
 *   <li>ADL-visible <code>get&lt;I&gt;(T)</code></li>
 * </ul>
 * <p>
 * Arbitrary aggregates or user-defined proxies are not implicitly unpacked,
 * ensuring deterministic behavior and preserving the element’s semantic identity.
 * </p>
 *
 * <h3>Relationship with other modules</h3>
 * <ul>
 *   <li><b><code>jh::ranges::collect</code></b> &mdash; uses this classification to
 *       materialize non-closable ranges into stable containers.</li>
 *   <li><b><code>jh::ranges::to</code></b> &mdash; skips this classification entirely,
 *       performing direct container construction when <code>closable</code>.</li>
 *   <li><b><code>jh::concepts::closable_container_for</code></b> &mdash; represents the
 *       complementary “construction-level” concept for <code>to</code>.</li>
 * </ul>
 *
 * <h3>Semantic guarantee</h3>
 * <p>
 * The <code>collectable_container_for</code> concept ensures that
 * <code>collect</code> can <b>safely and deterministically</b> construct a
 * container <code>C</code> from a range <code>R</code> via incremental insertion,
 * matching the observable semantics of <code>std::ranges::to</code> while
 * supporting richer tuple-unpacking behaviors.
 * </p>
 *
 * @note
 * <p>
 * <code>collect</code> is a <b>materializer</b>, not an adaptor:
 * it normalizes and realizes data into value semantics, but never applies
 * construction arguments.
 * Configuration belongs exclusively to <code>to</code>, allowing
 * <code>collect + to</code> to form a clean, composable, two-stage pipeline.
 * </p>
 *
 * @see jh::ranges::collect
 * @see jh::ranges::to
 * @see jh::concepts::closable_container_for
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include <cstdint>
#include "jh/conceptual/closable_container.h"
#include "jh/conceptual/tuple_like.h"


namespace jh::concepts::detail {

    /**
     * @brief Classification of how a container <code>C</code> can collect elements from a range <code>R</code>.
     */
    enum class collectable_status : std::uint8_t {
        none = 0,
        closable,            ///< constructible via <code>jh::ranges::to</code>
        emplace_back_direct, ///< uses <code>emplace_back()</code>
        push_back_direct,    ///< uses <code>push_back()</code>
        emplace_direct,      ///< uses <code>emplace()</code>
        insert_direct,       ///< uses <code>insert()</code>
        emplace_back_unpack, ///< tuple-unpack into <code>emplace_back()</code>
        emplace_unpack       ///< tuple-unpack into <code>emplace()</code>
    };

    /// @brief Checks if <code>emplace_back</code> can accept tuple-like unpacked elements.
    template<class C, class T, std::size_t... I>
    consteval bool test_emplace_back_unpackable(std::index_sequence<I...>) {
        using Tup = std::remove_cvref_t<T>;
        return requires {
            std::declval<C &>().emplace_back(
                    std::declval<std::remove_reference_t<std::tuple_element_t<I, Tup>>>()...);
        };
    }

    /// @brief Detects tuple-like unpackability into <code>emplace_back()</code>.
    template<class C, class T>
    consteval bool is_emplace_back_unpackable() {
        using Tup = std::remove_cvref_t<T>;
        if constexpr (!jh::concepts::tuple_like<Tup>)
            return false;
        else
            return test_emplace_back_unpackable<C, T>(
                    std::make_index_sequence<std::tuple_size_v<Tup>>{});
    }

    template<class C, class T>
    concept emplace_back_unpackable = is_emplace_back_unpackable<C, T>();

    /// @brief Checks if <code>emplace</code> can accept tuple-like unpacked elements.
    template<class C, class T, std::size_t... I>
    consteval bool test_emplace_unpackable(std::index_sequence<I...>) {
        using Tup = std::remove_cvref_t<T>;
        return requires {
            std::declval<C &>().emplace(
                    std::declval<std::remove_reference_t<std::tuple_element_t<I, Tup>>>()...);
        };
    }

    /// @brief Detects tuple-like unpackability into <code>emplace()</code>.
    template<class C, class T>
    consteval bool is_emplace_unpackable() {
        using Tup = std::remove_cvref_t<T>;
        if constexpr (!jh::concepts::tuple_like<Tup>)
            return false;
        else
            return test_emplace_unpackable<C, T>(
                    std::make_index_sequence<std::tuple_size_v<Tup>>{});
    }

    template<class C, class T>
    concept emplace_unpackable = is_emplace_unpackable<C, T>();

    /**
     * @brief Compute the <code>collectable_status</code> of a container–range pair.
     *
     * @details
     * This <em>constexpr deduction routine</em> examines, in order of priority,
     * how a container <code>C</code> can accept elements from a range <code>R</code>:
     * <ol>
     *   <li>If <code>C</code> is <code>closable_container_for&lt;C, R&gt;</code>, it is <b>closable</b>.</li>
     *   <li>Otherwise, if <code>C</code> supports <code>emplace_back()</code>,
     *       <code>push_back()</code>, <code>emplace()</code>, or <code>insert()</code>,
     *       the corresponding status is selected.</li>
     *   <li>If the range element is tuple-like, and <code>C</code> supports
     *       tuple-unpacked insertion, <code>emplace_back_unpack</code> or
     *       <code>emplace_unpack</code> are used.</li>
     *   <li>If no valid construction path is found, the result is <code>none</code>.</li>
     * </ol>
     *
     * @tparam C container type to evaluate
     * @tparam R range type providing input elements
     * @return enumerated <code>collectable_status</code> indicating available insertion strategy
     */
    template<typename C, typename R>
    consteval collectable_status compute_collectable_status() {
        using Cv = jh::concepts::container_value_t<C>;
        using Rv = std::ranges::range_reference_t<R>;

        // 1. prior: closable
        if constexpr (jh::concepts::closable_container_for<C, R>)
            return collectable_status::closable;

        // 2. emplace_back_direct
        if constexpr (std::default_initializable<C> && requires {{ Cv(std::declval<Rv>()) } -> std::same_as<Cv>; }
                      &&
                      requires([[maybe_unused]] C &c, [[maybe_unused]] Rv &&v) { c.emplace_back(std::forward<Rv>(v)); })
            return collectable_status::emplace_back_direct;

        // 3, push_back_direct
        if constexpr (std::default_initializable<C> && requires {{ Cv(std::declval<Rv>()) } -> std::same_as<Cv>; }
                      && requires([[maybe_unused]] C &c, [[maybe_unused]] Rv &&v) { c.push_back(std::forward<Rv>(v)); })
            return collectable_status::push_back_direct;

        // 4. emplace_direct
        if constexpr (std::default_initializable<C> && requires {{ Cv(std::declval<Rv>()) } -> std::same_as<Cv>; }
                      && requires([[maybe_unused]] C &c, [[maybe_unused]] Rv &&v) { c.emplace(std::forward<Rv>(v)); })
            return collectable_status::emplace_direct;

        // 5. insert_direct
        if constexpr (std::default_initializable<C> && requires {{ Cv(std::declval<Rv>()) } -> std::same_as<Cv>; }
                      && requires([[maybe_unused]] C &c, [[maybe_unused]] Rv &&v) { c.insert(std::forward<Rv>(v)); })
            return collectable_status::insert_direct;

        // 6. emplace_back_unpack
        if constexpr (emplace_back_unpackable<C, Rv>)
            return collectable_status::emplace_back_unpack;

        // 7. emplace_unpack
        if constexpr (emplace_unpackable<C, Rv>)
            return collectable_status::emplace_unpack;

        return collectable_status::none;
    }

    /**
     * @brief Internal trait evaluating collectability of a container–range pair.
     *
     * @tparam C container type
     * @tparam R range type
     */
    template<typename C, typename R>
    struct collectable_container_for_impl {
        static constexpr collectable_status status = compute_collectable_status<C, R>();
        static constexpr bool value = status != collectable_status::none;
    };

} // namespace jh::concepts::detail

namespace jh::concepts {

    /**
     * @brief Concept verifying that a container <code>C</code> can collect elements
     *        from a range <code>R</code> via incremental insertion.
     *
     * @details
     * This concept represents the compile-time contract that a container type
     * <code>C</code> supports an insertion form compatible with the semantics of
     * <code>jh::ranges::collect</code>.
     * Unlike <code>closable_container_for</code>, which checks for complete
     * constructibility, this concept checks for valid <em>incremental collection</em>
     * semantics (e.g. <code>push_back</code>, <code>emplace</code>, etc.).
     *
     * <h4>Requirements</h4>
     * <ul>
     *   <li><code>R</code> must model <code>std::ranges::input_range</code>.</li>
     *   <li><code>detail::compute_collectable_status&lt;C, R&gt;</code> must not return <code>none</code>.</li>
     * </ul>
     *
     * @tparam C container type to check
     * @tparam R range type providing input
     */
    template<typename C, typename R>
    concept collectable_container_for =
    std::ranges::input_range<R> &&
    detail::collectable_container_for_impl<C, R>::value;
} // namespace jh::concepts
