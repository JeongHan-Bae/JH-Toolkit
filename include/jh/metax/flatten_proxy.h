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
 * @file flatten_proxy.h
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
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
 *   <li><code>jh::meta::tuple_materialize</code> &mdash; Flattens any tuple-like object.</li>
 *   <li><code>jh::meta::flatten_proxy</code> &mdash; Lazy wrapper exposing flattened <code>get&lt;I&gt;</code> interface.</li>
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
 * @version <pre>1.4.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include "jh/conceptual/tuple_like.h"
#include <functional>
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

        /// @brief unwrap_ref &mdash; extract value from reference-like wrappers
        constexpr decltype(auto) unwrap_ref(auto &&x) {
            if constexpr (requires { x.get(); })
                return x.get();
            else
                return std::forward<decltype(x)>(x);
        }

        /// @brief flatten_one &mdash; flattens a single element
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

        /// @brief tuple_materialize_impl &mdash; implementation detail with index sequence
        template<typename Tuple, std::size_t... I>
        constexpr auto tuple_materialize_impl(const Tuple &t, std::index_sequence<I...>) {
            return std::tuple_cat(flatten_one(get<I>(t))...);
        }

        template<typename T>
        struct is_reference_wrapper : std::false_type {};

        template<typename T>
        struct is_reference_wrapper<std::reference_wrapper<T>> : std::true_type {};

        template<typename T, bool IsTuple = jh::concepts::tuple_like<std::remove_cvref_t<T>>>
        struct contains_reference_wrapper
                : is_reference_wrapper<std::remove_cv_t<std::remove_reference_t<T>>> {};

        template<typename T>
        struct contains_reference_wrapper<T, true> {
        private:
            using tuple_type = std::remove_cvref_t<T>;

            template<std::size_t... I>
            static consteval bool check(std::index_sequence<I...>) {
                return (contains_reference_wrapper<std::tuple_element_t<I, tuple_type>>::value || ...);
            }

        public:
            static constexpr bool value = check(
                std::make_index_sequence<std::tuple_size_v<tuple_type>>{}
            );
        };

        template<typename T>
        inline constexpr bool contains_reference_wrapper_v = contains_reference_wrapper<T>::value;

        template<std::size_t I, typename Tuple>
        constexpr decltype(auto) tuple_get_forwarded(Tuple &&tuple) {
            if constexpr (requires { std::forward<Tuple>(tuple).template get<I>(); }) {
                return std::forward<Tuple>(tuple).template get<I>();
            } else {
                return get<I>(std::forward<Tuple>(tuple));
            }
        }

        template<typename T>
        constexpr auto flatten_forward_one(T &&value) {
            if constexpr (jh::concepts::tuple_like<std::remove_cvref_t<T>>) {
                constexpr std::size_t N = std::tuple_size_v<std::remove_cvref_t<T>>;
                return [&]<std::size_t... I>(std::index_sequence<I...>) {
                    return std::tuple_cat(
                        flatten_forward_one(
                            tuple_get_forwarded<I>(std::forward<T>(value))
                        )...
                    );
                }(std::make_index_sequence<N>{});
            } else if constexpr (is_reference_wrapper<std::remove_cvref_t<T>>::value) {
                return std::forward_as_tuple(value.get());
            } else {
                return std::forward_as_tuple(std::forward<T>(value));
            }
        }

        template<bool CopyReferencedValues, typename TargetElement, typename Source>
        constexpr decltype(auto) flatten_materialization_arg(Source &&source) {
            if constexpr (CopyReferencedValues &&
                          !std::is_reference_v<TargetElement> &&
                          std::is_lvalue_reference_v<Source>) {
                return std::remove_cvref_t<Source>{source};
            } else {
                return std::forward<Source>(source);
            }
        }

        template<bool CopyReferencedValues, typename TargetTuple, typename SourceTuple,
                 std::size_t... I>
        constexpr TargetTuple flatten_materialize_as_impl(
                SourceTuple &&source, std::index_sequence<I...>) {
            return TargetTuple{
                flatten_materialization_arg<
                    CopyReferencedValues,
                    std::tuple_element_t<I, TargetTuple>
                >(
                    tuple_get_forwarded<I>(std::forward<SourceTuple>(source))
                )...
            };
        }

        template<bool CopyReferencedValues, typename TargetTuple, typename SourceTuple>
        constexpr TargetTuple flatten_materialize_as(SourceTuple &&source) {
            constexpr std::size_t N = std::tuple_size_v<TargetTuple>;
            return flatten_materialize_as_impl<CopyReferencedValues, TargetTuple>(
                std::forward<SourceTuple>(source), std::make_index_sequence<N>{}
            );
        }

        template<typename TargetTuple, typename SourceTuple, std::size_t... I>
        consteval bool has_dangling_rvalue_reference_impl(std::index_sequence<I...>) {
            return ((
                std::is_reference_v<std::tuple_element_t<I, TargetTuple>> &&
                std::is_rvalue_reference_v<
                    std::tuple_element_t<I, std::remove_cvref_t<SourceTuple>>
                >
            ) || ...);
        }

        template<typename TargetTuple, typename SourceTuple>
        consteval bool has_dangling_rvalue_reference() {
            if constexpr (
                std::tuple_size_v<TargetTuple> !=
                std::tuple_size_v<std::remove_cvref_t<SourceTuple>>
            ) {
                return false;
            } else {
                return has_dangling_rvalue_reference_impl<TargetTuple, SourceTuple>(
                    std::make_index_sequence<std::tuple_size_v<TargetTuple>>{}
                );
            }
        }

        template<typename Tuple>
        using flatten_proxy_lvalue_t = decltype(flatten_forward_one(std::declval<Tuple &>()));

        template<typename Tuple>
        using flatten_proxy_const_lvalue_t =
                decltype(flatten_forward_one(std::declval<const Tuple &>()));

        template<typename Tuple>
        using flatten_proxy_rvalue_t =
                decltype(flatten_forward_one(std::declval<Tuple &&>()));

        template<typename Tuple>
        using flatten_proxy_const_rvalue_t =
                decltype(flatten_forward_one(std::declval<const Tuple &&>()));

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
     * This proxy encapsulates any type that models
     * <code>jh::concepts::tuple_like</code> and exposes a
     * flattened <code>get&lt;I&gt;</code> interface compatible with
     * structured bindings and <code>std::tuple</code> introspection.
     * The accepted source types include <code>std::tuple</code>,
     * <code>std::pair</code>, <code>std::array</code>, and custom tuple-like
     * types that satisfy the concept; the source does not have to be a
     * <code>std::tuple</code>.
     *
     * @tparam Tuple Type of the tuple-like object held by the proxy. It may be
     *         a reference type for a borrowed lvalue source or an object type
     *         for an owned rvalue source. Class template argument deduction
     *         selects this storage type automatically. The type must model
     *         <code>jh::concepts::tuple_like</code>.
     *
     * <h4>Implicit Conversion</h4>
     * <p>
     * The proxy can be <b>implicitly converted</b> to a fully materialized
     * <code>std::tuple</code>. During conversion, element category follows the
     * proxy's value category: lvalue proxies expose lvalue elements, and rvalue
     * proxies expose rvalue elements. This lets a destination construct directly
     * from the source without an intermediate value tuple.
     * <ul>
     *   <li>Structured bindings see the flattened members directly</li>
     *   <li>Target <code>std::tuple</code> can hold value and reference types</li>
     *   <li>Materialized tuples cannot retain <code>std::reference_wrapper</code></li>
     *   <li>An rvalue proxy cannot produce references into its owned storage</li>
     * </ul>
     * </p>
     *
     * @code
     * int i0 = 1;
     * jh::meta::flatten_proxy p{ std::tuple{std::tie(i0), std::tuple{2, 3}} };
     * std::tuple&lt;int&, int, int&gt; t_ref = p;
     * std::tuple&lt;int, int, int&gt; t_val = p;
     * @endcode
     *
     * <h4>Ownership and Evaluation</h4>
     * <p>
     * An lvalue source is held by reference and an rvalue source is owned by
     * the proxy. Materialization constructs the destination tuple directly from
     * the flattened elements, preserving their reference category.
     * </p>
     */
    template<typename Tuple>
    requires jh::concepts::tuple_like<Tuple>
    struct flatten_proxy final {
        Tuple tuple;

        /**
         * @brief Stores an lvalue tuple-like source by reference or owns an rvalue source.
         * @param source Tuple-like object to expose through the proxy.
         */
        template<typename Source>
        requires (!std::is_same_v<std::remove_cvref_t<Source>, flatten_proxy> &&
                  jh::concepts::tuple_like<Source> &&
                  std::is_constructible_v<Tuple, Source&&>)
        constexpr explicit flatten_proxy(Source &&source)
                : tuple(std::forward<Source>(source)) {}

        /// @brief Returns flattened element I as a value.
        template<std::size_t I>
        [[nodiscard]] constexpr auto get() const noexcept {
            return std::get<I>(tuple_materialize(tuple));
        }

        /// @brief Converts a mutable lvalue proxy to a tuple when its elements permit it.
        template<typename... Ts>
        constexpr operator std::tuple<Ts...>() &
                requires (!detail::contains_reference_wrapper_v<std::tuple<Ts...>> &&
                          std::is_constructible_v<
                                  std::tuple<Ts...>, detail::flatten_proxy_lvalue_t<Tuple>
                          >) {
            return detail::flatten_materialize_as<false, std::tuple<Ts...>>(
                detail::flatten_forward_one(tuple)
            );
        }

        template<typename... Ts>
        constexpr operator std::tuple<Ts...>() &
                requires detail::contains_reference_wrapper_v<std::tuple<Ts...>> = delete;

        /// @brief Converts a const lvalue proxy to a tuple when its elements permit it.
        template<typename... Ts>
        constexpr operator std::tuple<Ts...>() const &
                requires (!detail::contains_reference_wrapper_v<std::tuple<Ts...>> &&
                          std::is_constructible_v<
                                  std::tuple<Ts...>, detail::flatten_proxy_const_lvalue_t<Tuple>
                          >) {
            return detail::flatten_materialize_as<false, std::tuple<Ts...>>(
                detail::flatten_forward_one(tuple)
            );
        }

        template<typename... Ts>
        constexpr operator std::tuple<Ts...>() const &
                requires detail::contains_reference_wrapper_v<std::tuple<Ts...>> = delete;

        /// @brief Converts an rvalue proxy to a tuple while forwarding its elements as rvalues.
        template<typename... Ts>
        constexpr operator std::tuple<Ts...>() &&
                requires (!detail::contains_reference_wrapper_v<std::tuple<Ts...>> &&
                          !detail::has_dangling_rvalue_reference<
                                  std::tuple<Ts...>, detail::flatten_proxy_rvalue_t<Tuple>
                          >() &&
                          std::is_constructible_v<
                                  std::tuple<Ts...>, detail::flatten_proxy_rvalue_t<Tuple>
                          >) {
            return detail::flatten_materialize_as<true, std::tuple<Ts...>>(
                detail::flatten_forward_one(std::forward<Tuple>(tuple))
            );
        }

        template<typename... Ts>
        constexpr operator std::tuple<Ts...>() &&
                requires (
                    detail::contains_reference_wrapper_v<std::tuple<Ts...>> ||
                    detail::has_dangling_rvalue_reference<
                        std::tuple<Ts...>, detail::flatten_proxy_rvalue_t<Tuple>
                    >()
                ) = delete;

        /// @brief Converts a const rvalue proxy to a tuple while preserving constness.
        template<typename... Ts>
        constexpr operator std::tuple<Ts...>() const &&
                requires (!detail::contains_reference_wrapper_v<std::tuple<Ts...>> &&
                          !detail::has_dangling_rvalue_reference<
                                  std::tuple<Ts...>, detail::flatten_proxy_const_rvalue_t<Tuple>
                          >() &&
                          std::is_constructible_v<
                                  std::tuple<Ts...>, detail::flatten_proxy_const_rvalue_t<Tuple>
                          >) {
            return detail::flatten_materialize_as<true, std::tuple<Ts...>>(
                detail::flatten_forward_one(std::forward<const Tuple>(tuple))
            );
        }

        template<typename... Ts>
        constexpr operator std::tuple<Ts...>() const &&
                requires (
                    detail::contains_reference_wrapper_v<std::tuple<Ts...>> ||
                    detail::has_dangling_rvalue_reference<
                        std::tuple<Ts...>, detail::flatten_proxy_const_rvalue_t<Tuple>
                    >()
                ) = delete;
    };

    /// @brief Deduces reference storage for lvalue sources and owned storage for rvalues.
    template<typename Tuple>
    requires jh::concepts::tuple_like<Tuple>
    flatten_proxy(Tuple &&) -> flatten_proxy<
            std::conditional_t<
                    std::is_lvalue_reference_v<Tuple>,
                    Tuple,
                    std::remove_cvref_t<Tuple>
            >
    >;

    template<std::size_t I, typename Tuple>
    constexpr decltype(auto) get(const flatten_proxy<Tuple> &p) noexcept {
        return p.template get<I>();
    }

} // namespace jh::meta

namespace std {

    template<typename Tuple>
    requires jh::concepts::tuple_like<Tuple>
    struct tuple_size<jh::meta::flatten_proxy<Tuple>>
            : std::tuple_size<decltype(jh::meta::detail::flatten_one(std::declval<Tuple>()))> {
    };

    template<std::size_t I, typename Tuple>
    requires jh::concepts::tuple_like<Tuple>
    struct tuple_element<I, jh::meta::flatten_proxy<Tuple>>
            : std::tuple_element<I, decltype(jh::meta::detail::flatten_one(std::declval<Tuple>()))> {
    };

} // namespace std
