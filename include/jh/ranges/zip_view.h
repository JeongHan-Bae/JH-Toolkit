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
 * @file zip_view.h
 * @brief Internal implementation for <code>jh::ranges::zip_view</code>.
 *
 * <p>
 * Defines the implementation of <code>jh::ranges::zip_view</code> &mdash;
 * a lightweight, C++20-compatible view that aggregates multiple ranges
 * into synchronized tuple-like element proxies.
 * </p>
 *
 * <p>
 * This header is part of the <strong>ranges</strong> module and provides the
 * view used by <code>jh::ranges::views::zip</code> and
 * <code>jh::ranges::views::enumerate</code>.
 * </p>
 *
 * <p>
 * <strong>Standard integration:</strong><br/>
 * When the
 * standard implementation is available, <code>jh::ranges::zip_view</code>
 * transparently aliases it through feature detection.
 * Otherwise, this header defines a portable fallback implementation
 * for synchronized traversal of input views. The fallback preserves
 * forward iteration when every component is a forward range; it does not
 * provide the standard zip view's bidirectional or random-access operations.
 * </p>
 *
 * <p>
 * The fallback implementation includes:
 * <ul>
 *   <li><code>zip_iterator</code> &mdash; synchronized iterator tuple</li>
 *   <li><code>zip_sentinel</code> &mdash; range termination marker</li>
 *   <li><code>zip_reference_proxy</code> &mdash; reference aggregator for structured bindings</li>
 * </ul>
 * Together, these provide the C++20 fallback's input and forward traversal.
 * </p>
 *
 * @see jh::ranges::views::zip
 * @see jh::ranges::views::enumerate
 */

#pragma once

#include <tuple>
#include <ranges>
#include <utility>
#include <type_traits>
#include <functional>
#include "jh/metax/adl_apply.h"

/// Feature detection for <code>std::ranges::zip_view</code> availability.
#if defined(__cpp_lib_ranges_zip) && __cpp_lib_ranges_zip >= 202110L
#define JH_HAS_STD_ZIP_VIEW 1
#elif defined(__cplusplus) && __cplusplus > 202302L
#define JH_HAS_STD_ZIP_VIEW 1
#else
#define JH_HAS_STD_ZIP_VIEW 0
#endif

namespace jh::ranges {

    /// Use the standard <code>std::ranges::zip_view</code> when available.
#if JH_HAS_STD_ZIP_VIEW
    using std::ranges::zip_view;
#else

    namespace detail {

        /**
         * @brief Applies a callable to each element of a <code>std::tuple</code>.
         * @tparam F Callable type.
         * @tparam Tuple Input tuple type.
         * @tparam I Index sequence (automatically deduced).
         * @param f The callable to apply to each element.
         * @param t The tuple to transform.
         * @return A new <code>std::tuple</code> containing the transformed elements.
         */
        template<typename F, typename Tuple, std::size_t... I>
        constexpr auto tuple_transform_impl(F &&f, Tuple &&t, std::index_sequence<I...>) {
            return std::tuple{std::invoke(f, std::get<I>(std::forward<Tuple>(t)))...};
        }

        /**
         * @brief Wraps an element depending on its value category.
         *
         * @tparam T Element type.
         * @param value The element to wrap.
         * @return If <code>T</code> is an lvalue reference, returns a
         *         <code>std::reference_wrapper&lt;T&gt;</code>; otherwise returns the value itself.
         */
        template<typename T>
        constexpr auto wrap_element(T &&value) {
            if constexpr (std::is_lvalue_reference_v<T>) {
                return std::ref(value);
            } else {
                return std::forward<T>(value);
            }
        }
    } // namespace detail

    /**
     * @brief Applies a callable to each element of a <code>std::tuple</code>.
     *
     * <p>
     * This helper function iterates over all elements of a tuple and applies the
     * provided callable <code>f</code> to each element, returning a new
     * <code>std::tuple</code> containing the transformed results.
     * </p>
     *
     * <p>
     * It is implemented using <code>std::index_sequence</code> expansion and
     * perfect forwarding. This utility serves as a C++20-compatible equivalent
     * to <code>std::apply</code> for element-wise transformation.
     * </p>
     *
     * @tparam F Callable type &mdash; must be invocable with each element of <code>Tuple</code>.
     * @tparam Tuple The tuple-like type to be transformed.
     * @param f The callable to apply to each element.
     * @param t The tuple whose elements are to be transformed.
     * @return A new <code>std::tuple</code> containing <code>f(get&lt;i&gt;(t))</code> for each element.
     *
     * @note
     * This function preserves value category:
     * <ul>
     *   <li>Lvalue elements are forwarded as references.</li>
     *   <li>Rvalue elements are moved when possible.</li>
     * </ul>
     */
    template<typename F, typename Tuple>
    constexpr auto tuple_transform(F &&f, Tuple &&t) {
        constexpr std::size_t N = std::tuple_size_v<std::remove_reference_t<Tuple>>;
        return detail::tuple_transform_impl(std::forward<F>(f), std::forward<Tuple>(t),
                                            std::make_index_sequence<N>{});
    }

    template<typename... Es>
    struct zip_reference_proxy;

    template<typename T>
    struct is_zip_proxy final : std::false_type {};

    template<typename... Ts>
    struct is_zip_proxy<class zip_reference_proxy<Ts...>> final : std::true_type {};

    template<typename T>
    inline constexpr bool is_zip_proxy_v = is_zip_proxy<T>::value;

    /// @brief Derives the value type stored by a zipped reference element.
    template<typename T>
    struct zip_proxy_value_tuple final {
        using type = std::unwrap_reference_t<std::remove_cvref_t<T>>;
    };

    /// @brief Derives a tuple of value types from nested zipped reference elements.
    template<typename... Ts>
    struct zip_proxy_value_tuple<class zip_reference_proxy<Ts...>> final {
        using type = std::tuple<
                typename zip_proxy_value_tuple<std::remove_cvref_t<Ts>>::type...
        >;
    };

    template<typename T>
    using zip_proxy_value_tuple_t = typename zip_proxy_value_tuple<T>::type;

    /**
     * @brief Aggregates element references for a single tuple in <code>jh::ranges::zip_view</code>.
     *
     * <p>
     * <code>zip_reference_proxy</code> acts as the dereference result of
     * <code>zip_iterator</code>, holding a tuple of element references (or
     * values) from the underlying ranges.
     * </p>
     *
     * <p>
     * This proxy provides structured binding support by exposing a member
     * <code>get&lt;I&gt;()</code> and a corresponding free function
     * <code>get&lt;I&gt;(const zip_reference_proxy&amp;)</code>,
     * making it a tuple-like object accessible through ADL and
     * <code>std::tuple_size</code>.
     * </p>
     *
     * <p>
     * When dereferencing <code>zip_iterator</code>, each element of the
     * aggregated tuple may be either:
     * <ul>
     *   <li>a direct reference, if the source element is an lvalue;</li>
     *   <li>a <code>std::reference_wrapper</code>, if wrapped through
     *       <code>detail::wrap_element()</code>;</li>
     *   <li>a value, if the range yields temporaries (e.g. <code>std::views::iota</code>).</li>
     * </ul>
     * The proxy abstracts over these distinctions so that element access
     * through <code>get&lt;I&gt;()</code> preserves the stored reference or value.
     * </p>
     *
     * @tparam Es The element types of the aggregated tuple. Each may be a
     *         reference, <code>std::reference_wrapper</code>, or value type.
     *
     * @see jh::ranges::zip_iterator
     * @see jh::ranges::zip_view
     */
    template<typename... Es>
    struct zip_reference_proxy final {
        std::tuple<Es...> elems;

    private:
        static constexpr decltype(auto) unwrap_ref(auto&& x) {
            using X = std::remove_cvref_t<decltype(x)>;
            if constexpr (requires { x.get(); }) {
                return x.get();
            } else if constexpr (is_zip_proxy_v<X>) {
                return static_cast<zip_proxy_value_tuple_t<X>>(std::forward<decltype(x)>(x));
            } else {
                return std::forward<decltype(x)>(x);
            }
        }

    public:
        /// @brief Copies the referenced elements into a value tuple.
        constexpr operator zip_proxy_value_tuple_t<zip_reference_proxy>() // NOLINT
                const & {
            return jh::meta::adl_apply([](auto const&... e) {
                return zip_proxy_value_tuple_t<zip_reference_proxy>{ unwrap_ref(e)... };
            }, elems);
        }

        /// @brief Moves the referenced elements into a value tuple.
        constexpr operator zip_proxy_value_tuple_t<zip_reference_proxy>() // NOLINT
                && {
            return jh::meta::adl_apply([](auto&&... e) {
                return zip_proxy_value_tuple_t<zip_reference_proxy>{ unwrap_ref(std::forward<decltype(e)>(e))... };
            }, std::move(elems));
        }

        /// @brief Returns the indexed element stored by this lvalue proxy.
        template <std::size_t N>
        constexpr decltype(auto) get() const & noexcept {
            return std::get<N>(elems);
        }

        /// @brief Returns the indexed element stored by this rvalue proxy.
        template <std::size_t N>
        constexpr decltype(auto) get() && noexcept {
            return std::get<N>(std::move(elems));
        }
    };

    /**
     * @brief Retrieves the <code>I</code>-th element from a <code>zip_reference_proxy</code>.
     *
     * <p>
     * This free function provides tuple-like access to elements within a
     * <code>zip_reference_proxy</code>, enabling structured bindings and
     * compatibility with ADL-based tuple access and <code>std::tuple_element</code>.
     * </p>
     *
     * <p>
     * Internally, this simply forwards to
     * <code>p.get&lt;I&gt;()</code>, preserving reference semantics and
     * preserving the proxy's stored element access.
     * </p>
     *
     * @tparam I The index of the element to retrieve.
     * @tparam Elems Parameter pack of stored element types.
     * @param p The <code>zip_reference_proxy</code> instance to access.
     * @return Access to the <code>I</code>-th stored proxy element.
     *
     * @see jh::ranges::zip_reference_proxy
     * @see jh::meta::adl_apply
     */
    template<std::size_t I, typename... Elems>
    constexpr decltype(auto) get(const zip_reference_proxy<Elems...> &p) noexcept {
        return p.template get<I>();
    }

    /**
     * @brief Sentinel type for <code>jh::ranges::zip_view</code>.
     *
     * <p>
     * <code>zip_sentinel</code> represents the logical end of a
     * <code>jh::ranges::zip_view</code>. It holds a tuple of end iterators
     * (<code>Sentinels...</code>) corresponding to the underlying ranges
     * being zipped together.
     * </p>
     *
     * <p>
     * During iteration, <code>zip_iterator</code> compares itself against
     * a <code>zip_sentinel</code> to determine termination. Iteration stops
     * when <strong>any</strong> of the underlying iterators equals its
     * corresponding sentinel &mdash; ensuring short-circuit termination on the
     * shortest range.
     * </p>
     *
     * @tparam Sentinels The sentinel types associated with the zipped ranges.
     *
     * @see jh::ranges::zip_iterator
     * @see jh::ranges::zip_view
     */
    template<typename... Sentinels>
    struct zip_sentinel final {
        /// Tuple of underlying range end iterators.
        std::tuple<Sentinels...> ends;
    };

    /**
     * @brief Iterator type for <code>jh::ranges::zip_view</code>.
     *
     * <p>
     * <code>zip_iterator</code> traverses multiple input ranges in lockstep,
     * dereferencing into a <code>zip_reference_proxy</code> that aggregates
     * references or values from the underlying ranges.
     * </p>
     *
     * <p>
     * Each iterator maintains a tuple of underlying iterators
     * (<code>Iters...</code>) and advances all of them together.
     * Comparison against a corresponding <code>zip_sentinel</code> terminates
     * iteration when <strong>any</strong> of the underlying iterators reaches
     * its end, ensuring short-circuit termination on the shortest range.
     * </p>
     *
     * <p>
     * Dereferencing (<code>operator*</code>) applies <code>tuple_transform</code>
     * to dereference each underlying iterator, wrapping lvalue results with
     * <code>std::reference_wrapper</code> when necessary via
     * <code>detail::wrap_element()</code>.
     * </p>
     *
     * @tparam Iters The iterator types for the zipped ranges.
     *
     * @note
     * This type is intentionally defined outside of <code>zip_view</code>
     * to maintain independent template deduction and suppress
     * <tt>clang-tidy</tt> "unused type" warnings via
     * <code>[[maybe_unused]]</code>.
     *
     * @see jh::ranges::zip_reference_proxy
     * @see jh::ranges::zip_sentinel
     * @see jh::ranges::zip_view
     */
    template<typename... Iters>
    struct [[maybe_unused]] zip_iterator final {
        std::tuple<Iters...> iters;
        bool terminal_ = false;

        using difference_type   = std::ptrdiff_t;
        using value_type        = std::tuple<std::remove_cvref_t<std::iter_value_t<Iters>>...>;
        using reference         = zip_reference_proxy<std::iter_reference_t<Iters>...>;
        using iterator_concept [[maybe_unused]] = std::conditional_t<(std::forward_iterator<Iters> && ...),
                                                  std::forward_iterator_tag,
                                                  std::input_iterator_tag>;

        /// @brief Combines the current elements of all underlying iterators into a proxy.
        constexpr reference operator*() const {
            auto packed = tuple_transform([](auto &it) -> decltype(auto) {
                return detail::wrap_element(*it);
            }, iters);
            return jh::meta::adl_apply([](auto &&... args) {
                return reference{std::tuple{std::forward<decltype(args)>(args)...}};
            }, packed);
        }

        /// @brief Advances every underlying iterator by one element.
        constexpr zip_iterator &operator++() {
            jh::meta::adl_apply([](auto &... it) { (++it, ...); }, iters);
            return *this;
        }

        /// @brief Advances a single-pass zip iterator without returning its old state.
        constexpr void operator++(int) requires (!(std::forward_iterator<Iters> && ...)) {
            ++(*this);
        }

        /// @brief Advances a forward zip iterator and returns its old state.
        constexpr zip_iterator operator++(int) requires (std::forward_iterator<Iters> && ...) {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        /// @brief Compares zipped positions, using any exhausted component at the end.
        constexpr bool operator==(const zip_iterator &other) const
            requires (std::equality_comparable<Iters> && ...) {
            return [&]<std::size_t... I>(std::index_sequence<I...>) {
                if (terminal_ || other.terminal_) {
                    return ((std::get<I>(iters) == std::get<I>(other.iters)) || ...);
                }
                return ((std::get<I>(iters) == std::get<I>(other.iters)) && ...);
            }(std::index_sequence_for<Iters...>{});
        }

        /// @brief Reports whether two zipped iterators differ.
        constexpr bool operator!=(const zip_iterator &other) const
            requires (std::equality_comparable<Iters> && ...) {
            return !(*this == other); // NOLINT
        }

        /// @brief Checks whether any component has reached its corresponding sentinel.
        template<typename... Sentinels>
        constexpr bool operator==(const zip_sentinel<Sentinels...> &s) const {
            return [&]<std::size_t... I>(std::index_sequence<I...>) {
                return ((std::get<I>(iters) == std::get<I>(s.ends)) || ...);
            }(std::index_sequence_for<Iters...>{});
        }

        /// @brief Checks whether every component remains before its sentinel.
        template<typename... Sentinels>
        constexpr bool operator!=(const zip_sentinel<Sentinels...> &s) const {
            return !(*this == s); // NOLINT
        }
    };

    /// @brief Checks whether a sentinel has been reached by a zipped iterator.
    template<typename... Iters, typename... Sentinels>
    constexpr bool operator==(const zip_sentinel<Sentinels...>& s,
                              const zip_iterator<Iters...>& it) {
        return it == s;
    }

    /// @brief Checks whether a zipped iterator remains before the sentinel.
    template<typename... Iters, typename... Sentinels>
    constexpr bool operator!=(const zip_sentinel<Sentinels...>& s,
                              const zip_iterator<Iters...>& it) {
        return !(it == s);
    }

    /**
     * @brief A C++20-compatible implementation of <code>std::ranges::zip_view</code>.
     *
     * <p>
     * The <code>zip_view</code> class provides a view that aggregates multiple
     * underlying ranges into synchronized tuples of elements, effectively
     * iterating them in parallel. Each dereference yields a
     * <code>zip_reference_proxy</code> holding references (or values) to the
     * corresponding elements from all component ranges.
     * </p>
     *
     * <p>
     * Iteration stops when <strong>any</strong> of the underlying ranges is
     * exhausted, ensuring short-circuit semantics on the shortest input range.
     * </p>
     *
     * <p>
     * The C++20 fallback supports input traversal and preserves forward
     * traversal when every component is a forward range.
     * </p>
     *
     * <p>
     * Example:
     * @code
     * std::vector&lt;int&gt; a = {1, 2, 3};
     * std::vector&lt;char&gt; b = {'A', 'B', 'C', 'D'};
     *
     * jh::ranges::zip_view zipped(a, b);
     *
     * for (const auto& [x, y] : zipped) {
     *     std::cout << x << " : " << y << std::endl;
     * }
     * @endcode
     * </p>
     *
     * @tparam Views Parameter pack of underlying view types.
     *
     * @see jh::ranges::zip_iterator
     * @see jh::ranges::zip_sentinel
     * @see jh::ranges::zip_reference_proxy
     * @see jh::ranges::views::zip
     */
    template<typename... Views>
        requires ((std::ranges::view<Views> && std::ranges::input_range<Views>) && ...)
    class zip_view final : public std::ranges::view_interface<zip_view<Views...>> {
        /// @brief Tuple of underlying view objects.
        std::tuple<Views...> bases;

    public:
        /**
         * @brief Default-constructs an empty <code>zip_view</code>.
         *
         * @details
         * All underlying views are value-initialized.
         */
        zip_view() = default;

        /**
         * @brief Constructs a <code>zip_view</code> from multiple views.
         *
         * @param vs The view objects to aggregate.
         *
         * @details
         * Each view is stored by value inside <code>zip_view</code>.
         * The lifetime of these views determines the lifetime of
         * their corresponding iterators and reference proxies.
         */
        constexpr explicit zip_view(Views... vs) : bases(std::move(vs)...) {}

        /**
         * @brief Returns an iterator to the beginning of the zipped sequence.
         *
         * @return A <code>zip_iterator</code> aggregating begin iterators
         *         of all underlying ranges.
         *
         * @details
         * This overload participates when <code>*this</code> is non-const.
         */
        constexpr auto begin() {
            return zip_iterator{
                    tuple_transform([](auto &v) { return std::ranges::begin(v); }, bases)
            };
        }

        /**
         * @brief Returns a sentinel marking the end of the zipped sequence.
         *
         * @return A <code>zip_sentinel</code> aggregating end iterators
         *         of all underlying ranges.
         *
         * @details
         * Iteration stops when <strong>any</strong> of the component iterators
         * reaches its corresponding end sentinel.
         *
         * <p>
         * The fallback implementation always uses <code>zip_sentinel</code>
         * for termination so truncation works correctly even when all
         * underlying ranges are common ranges with different lengths.
         * </p>
         */
        constexpr auto end() {
            if constexpr ((std::ranges::common_range<Views> && ...)) {
                return zip_iterator{
                        tuple_transform([](auto &v) { return std::ranges::end(v); }, bases),
                        true
                };
            } else {
                return zip_sentinel{
                        tuple_transform([](auto &v) { return std::ranges::end(v); }, bases)
                };
            }
        }

        /**
         * @brief Returns a const iterator to the beginning of the zipped sequence.
         *
         * @return A <code>zip_iterator</code> aggregating <code>begin()</code> iterators
         *         of all underlying ranges.
         *
         * @details
         * This overload participates when <code>*this</code> is const.
         */
        constexpr auto begin() const requires (std::ranges::range<const Views> && ...) {
            return zip_iterator{
                    tuple_transform([](auto const &v) { return std::ranges::begin(v); }, bases)
            };
        }

        /**
         * @brief Returns a const sentinel marking the end of the zipped sequence.
         *
         * @return A <code>zip_sentinel</code> aggregating <code>end()</code> iterators
         *         of all underlying ranges.
         *
         * @details
         * This overload participates when <code>*this</code> is const.
         */
        constexpr auto end() const requires (std::ranges::range<const Views> && ...) {
            if constexpr ((std::ranges::common_range<const Views> && ...)) {
                return zip_iterator{
                        tuple_transform([](auto const &v) { return std::ranges::end(v); }, bases),
                        true
                };
            } else {
                return zip_sentinel{
                        tuple_transform([](auto const &v) { return std::ranges::end(v); }, bases)
                };
            }
        }
    };

    /**
     * @brief Deduction guide for <code>jh::ranges::zip_view</code>.
     *
     * @tparam Rs The range types to be zipped.
     * @details
     * Allows automatic template argument deduction when constructing
     * a <code>zip_view</code> directly.
     */
    template<std::ranges::viewable_range... Rs>
    zip_view(Rs &&...) -> zip_view<std::views::all_t<Rs>...>;

#endif // fallback
} // namespace jh::ranges

#if !JH_HAS_STD_ZIP_VIEW
namespace std {
    template<typename... Elems>
    struct tuple_size<jh::ranges::zip_reference_proxy<Elems...>>
            : std::integral_constant<std::size_t, sizeof...(Elems)> {
    };

    /// @brief Exposes the standard tuple element type of a zipped reference proxy.
    template<std::size_t I, typename... Elems>
    struct tuple_element<I, jh::ranges::zip_reference_proxy<Elems...>> {
        using type = decltype(std::declval<jh::ranges::zip_reference_proxy<Elems...>>().template get<I>());
    };

    /// @brief Derives a common reference between a zipped proxy and a tuple.
    template<typename... Ts, typename... Us>
    struct [[maybe_unused]] common_reference<
            jh::ranges::zip_reference_proxy<Ts...>,
            std::tuple<Us...>
    > {
        using type = std::tuple<std::common_reference_t<
                std::unwrap_reference_t<Ts>,
                std::unwrap_reference_t<Us>>...>;
    };

    /// @brief Derives a common reference between a tuple and a zipped proxy.
    template<typename... Ts, typename... Us>
    struct [[maybe_unused]] common_reference<
            std::tuple<Ts...>,
            jh::ranges::zip_reference_proxy<Us...>
    > {
        using type = std::tuple<std::common_reference_t<
                std::unwrap_reference_t<Ts>,
                std::unwrap_reference_t<Us>>...>;
    };

    /// @brief Derives a common reference between two zipped proxies.
    template<typename... Ts, typename... Us>
    struct [[maybe_unused]] common_reference<
            jh::ranges::zip_reference_proxy<Ts...>,
            jh::ranges::zip_reference_proxy<Us...>
    > {
        using type = jh::ranges::zip_reference_proxy<std::common_reference_t<
                std::unwrap_reference_t<Ts>,
                std::unwrap_reference_t<Us>>...>;
    };

    template<typename... Ts, typename... Us>
    struct [[maybe_unused]] common_reference<
            jh::ranges::zip_reference_proxy<Ts...>&&,
            std::tuple<Us...>&
    > : common_reference<jh::ranges::zip_reference_proxy<Ts...>, std::tuple<Us...>> {};

    template<typename... Ts, typename... Us>
    struct [[maybe_unused]] common_reference<
            jh::ranges::zip_reference_proxy<Ts...>&,
            std::tuple<Us...>&
    > : common_reference<jh::ranges::zip_reference_proxy<Ts...>, std::tuple<Us...>> {};

    template<typename... Ts, typename... Us>
    struct [[maybe_unused]] common_reference<
            jh::ranges::zip_reference_proxy<Ts...>&&,
            std::tuple<Us...>&&
    > : common_reference<jh::ranges::zip_reference_proxy<Ts...>, std::tuple<Us...>> {};

    // tuple& vs proxy&&
    template<typename... Ts, typename... Us>
    struct [[maybe_unused]] common_reference<
            std::tuple<Ts...>&,
            jh::ranges::zip_reference_proxy<Us...>&&
    > : common_reference<
            std::tuple<Ts...>,
            jh::ranges::zip_reference_proxy<Us...>> {};

    // proxy&& vs proxy&
    template<typename... Ts, typename... Us>
    struct [[maybe_unused]] common_reference<
            jh::ranges::zip_reference_proxy<Ts...>&&,
            jh::ranges::zip_reference_proxy<Us...>&
    > : common_reference<
            jh::ranges::zip_reference_proxy<Ts...>,
            jh::ranges::zip_reference_proxy<Us...>> {};
} // namespace std

namespace std::ranges{
    /**
     * @brief Legal specialization of <code>std::ranges::enable_borrowed_range</code>.
     *
     * <p>
     * This specialization is <b>explicitly permitted</b> by the C++ standard.
     * User code may provide template specializations for certain customization points
     * declared under the <code>std::ranges</code> namespace, including
     * <code>std::ranges::enable_borrowed_range</code>, to indicate that a custom range
     * type models the borrowed-range property.
     * </p>
     *
     * <h4>About Clang-Tidy false positives</h4>
     * <p>
     * Static analyzers such as <b>Clang-Tidy</b> may emit a warning:
     * <em>"Modification of 'std' namespace can result in undefined behavior"</em>.
     * This warning is triggered because Clang-Tidy heuristically treats
     * <b>any</b> declaration inside a user-written <code>namespace std</code> block
     * as a modification of the standard namespace, without recognizing that
     * <em>nested namespaces</em> like <code>std::ranges</code> define a separate,
     * standards-sanctioned customization domain.
     * </p>
     *
     * <p>
     * Concretely:
     * </p>
     * <ul>
     *   <li>Specializations such as <code>std::tuple_element&lt;&gt;</code> are recognized
     *       by Clang-Tidy as explicitly allowed and thus do <b>not</b> trigger the warning.</li>
     *   <li>Specializations inside nested subnamespaces (e.g.
     *       <code>std::ranges::enable_borrowed_range&lt;T&gt;</code>) are equally legal
     *       under the standard, but Clang-Tidy does not check the whitelist at that depth
     *       and therefore incorrectly flags them as potential UB.</li>
     * </ul>
     *
     * <p>
     * This is a <b>false positive</b> &mdash; the specialization is <em>fully standard-compliant</em>
     * and does <b>not</b> constitute undefined behavior.
     * </p>
     *
     * @see <a href="https://en.cppreference.com/w/cpp/ranges/borrowed_range.html">
     * std::ranges::borrowed_range</a>
     */
    template <typename... Views>
    [[maybe_unused]] inline constexpr bool enable_borrowed_range<jh::ranges::zip_view<Views...>> =
            (std::ranges::borrowed_range<Views> && ...);
} // namespace std::ranges

#endif // !JH_HAS_STD_ZIP_VIEW
