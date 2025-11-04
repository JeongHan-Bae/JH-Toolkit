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
 * @file flatten_proxy.h (meta)
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 * @brief Tuple flattening utilities and proxy wrapper for nested tuple-like types.
 *
 * @details
 * The <code>jh::meta::flatten_proxy</code> mechanism provides a compile-time
 * meta-layer for expanding and materializing arbitrarily nested
 * <code>tuple_like</code> structures into a single flattened <code>std::tuple</code>.
 *
 * <h3>Design Goals</h3>
 * <ul>
 *   <li>Provide a <b>zero-overhead</b> flattening proxy for tuple-like objects.</li>
 *   <li>Support composition of nested proxy or view types that model <code>tuple_like</code>.</li>
 *   <li>Expose a clean <code>tuple_materialize()</code> API for generic metaprogramming.</li>
 * </ul>
 *
 * <h3>Key Components</h3>
 * <ul>
 *   <li><code>jh::meta::tuple_materialize</code> — Flattens any tuple-like object.</li>
 *   <li><code>jh::meta::flatten_proxy</code> — Lazy wrapper exposing flattened <code>get&lt;I&gt;</code> interface.</li>
 * </ul>
 *
 * <h3>Design Notes</h3>
 * <ul>
 *   <li>Compatible with proxy-based tuple types such as <code>zip_reference_proxy</code>.</li>
 *   <li>All transformations are <b>constexpr</b> and <b>reference-safe</b>.</li>
 *   <li>Integrates with <code>std::tuple_size</code> / <code>std::tuple_element</code>
 *       for structured binding compatibility.</li>
 * </ul>
 *
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once
#include "jh/conceptual/tuple_like.h"
#include <tuple>
#include <utility>
#include <type_traits>

namespace jh::meta {

    /**
     * @brief Flattens a tuple-like object into a fully materialized std::tuple.
     *
     * @details
     * Recursively expands all nested <code>tuple_like</code> members within <code>T</code>
     * and produces a single-level <code>std::tuple</code> containing their underlying elements.
     *
     * @tparam Tuple The input type modeling <code>jh::concepts::tuple_like</code>.
     * @param t The tuple-like object to flatten.
     * @return A <code>std::tuple</code> with all nested contents expanded.
     */
    template<typename Tuple>
    constexpr auto tuple_materialize(const Tuple &t);

    namespace detail {

        /// @brief unwrap_ref — extract value from reference-like wrappers
        constexpr decltype(auto) unwrap_ref(auto &&x) {
            if constexpr (requires { x.get(); })
                return x.get();
            else
                return std::forward<decltype(x)>(x);
        }

        /// @brief flatten_one — flattens a single element
        template<typename T>
        constexpr auto flatten_one(T &&x) {
            using U = std::remove_cvref_t<T>;
            if constexpr (jh::concepts::tuple_like<U>) {
                return jh::meta::tuple_materialize(std::forward<T>(x));
            } else {
                return std::tuple<std::remove_cvref_t<std::unwrap_reference_t<T>>>{
                        unwrap_ref(std::forward<T>(x))
                };
            }
        }

        /// @brief tuple_materialize_impl — implementation detail with index sequence
        template<typename Tuple, std::size_t... I>
        constexpr auto tuple_materialize_impl(const Tuple &t, std::index_sequence<I...>) {
            return std::tuple_cat(flatten_one(get<I>(t))...);
        }
    } // namespace detail

    /**
     * @brief Public entry point for tuple flattening.
     */
    template<typename Tuple>
    constexpr auto tuple_materialize(const Tuple &t) {
        constexpr std::size_t N = std::tuple_size_v<std::remove_cvref_t<Tuple>>;
        return detail::tuple_materialize_impl(t, std::make_index_sequence<N>{});
    }

    /**
     * @brief Proxy wrapper that lazily exposes flattened tuple access.
     *
     * @details
     * This proxy encapsulates any tuple-like object and exposes a
     * flattened <code>get&lt;I&gt;</code> interface compatible with
     * structured bindings and <code>std::tuple</code> introspection.
     *
     * <h4>Implicit Conversion</h4>
     * <p>
     * The proxy can be <b>implicitly converted</b> to a fully materialized
     * <code>std::tuple</code>. During conversion, element category is preserved:
     * <ul>
     *   <li><code>std::reference_wrapper&lt;T&gt;</code> is transparently propagated</li>
     *   <li>Structured bindings see the flattened members directly</li>
     *   <li>Target <code>std::tuple</code> can hold value, reference, or wrapper types</li>
     * </ul>
     * </p>
     *
     * @code
     * int i0 = 1;
     * jh::meta::flatten_proxy p{ std::tuple{std::ref(i0), std::tuple{2, 3}} };
     * auto [f0, f1, f2] = p; // reference_wrapper&lt;int&gt;, int, int
     * std::tuple&lt;std::reference_wrapper&lt;int&gt;, int, int&gt; t_rw = p;
     * std::tuple&lt;int&, int, int&gt; t_ref = p;
     * std::tuple&lt;int, int, int&gt; t_val = p;
     * @endcode
     *
     * <h4>Ownership and Evaluation</h4>
     * <p>
     * All operations are <code>constexpr</code> and non-owning —
     * the underlying tuple-like object is never copied or moved unless
     * materialization is explicitly requested (e.g. via implicit conversion
     * to <code>std::tuple</code>).
     * </p>
     */
    template<typename Tuple>
    struct flatten_proxy {
        Tuple tuple;

        template<std::size_t I>
        [[nodiscard]] constexpr auto get() const noexcept {
            return std::get<I>(tuple_materialize(tuple));
        }

        template<typename... Ts>
        constexpr operator std::tuple<Ts...>() const {
            return tuple_materialize(tuple);
        }

        constexpr operator auto() const {
            return tuple_materialize(tuple);
        }
    };

    template<std::size_t I, typename Tuple>
    constexpr decltype(auto) get(const flatten_proxy<Tuple> &p) noexcept {
        return p.template get<I>();
    }

} // namespace jh::meta

namespace std {

    template<typename Tuple>
    struct tuple_size<jh::meta::flatten_proxy<Tuple>>
            : std::tuple_size<decltype(jh::meta::detail::flatten_one(std::declval<Tuple>()))> {};

    template<std::size_t I, typename Tuple>
    struct tuple_element<I, jh::meta::flatten_proxy<Tuple>>
            : std::tuple_element<I, decltype(jh::meta::detail::flatten_one(std::declval<Tuple>()))> {};

} // namespace std
