/**
 * @file zip_view.h (ranges)
 * @brief Internal implementation for <code>jh::ranges::zip_view</code>.
 *
 * <p>
 * Defines the implementation of <code>jh::ranges::zip_view</code> —
 * a lightweight, C++20-compatible view that aggregates multiple ranges
 * into synchronized tuples of references.
 * </p>
 *
 * <p>
 * This header is part of the <strong>ranges</strong> module and serves as
 * an internal dependency of <code>jh::ranges::views::zip</code> and
 * <code>jh::ranges::views::enumerate</code>.
 * It is <strong>not</strong> intended for direct inclusion by user code.
 * </p>
 *
 * <p>
 * <strong>Semantic equivalence:</strong><br/>
 * <code>jh::ranges::zip_view</code> provides the same observable behavior
 * as <code>std::ranges::zip_view</code> (C++23 and later). When the
 * standard implementation is available, <code>jh::ranges::zip_view</code>
 * transparently aliases it through feature detection.
 * Otherwise, this header defines a portable fallback implementation
 * with identical semantics and API surface.
 * </p>
 *
 * <p>
 * This design follows the same principle as <code>boost::string_view</code>
 * — offering a pre-standard, fully compatible type that later becomes
 * a transparent bridge to its standard counterpart once available.
 * </p>
 *
 * <p>
 * The fallback implementation includes:
 * <ul>
 *   <li><code>zip_iterator</code> — synchronized iterator tuple</li>
 *   <li><code>zip_sentinel</code> — range termination marker</li>
 *   <li><code>zip_reference_proxy</code> — reference aggregator for structured bindings</li>
 * </ul>
 * Together, these emulate the complete semantics of
 * <code>std::ranges::zip_view</code> for C++20 environments.
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
    }

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
     * @tparam F Callable type — must be invocable with each element of <code>Tuple</code>.
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
     * making it behave as a tuple-like object compatible with
     * <code>std::get</code> and <code>std::tuple_size</code>.
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
     * through <code>get&lt;I&gt;()</code> always yields a reference to the
     * underlying element.
     * </p>
     *
     * @tparam Elems The element types of the aggregated tuple. Each may be a
     *         reference, <code>std::reference_wrapper</code>, or value type.
     *
     * @see jh::ranges::zip_iterator
     * @see jh::ranges::zip_view
     */
    template<typename... Elems>
    struct zip_reference_proxy {
        std::tuple<Elems...> elems;

        template<std::size_t I>
        constexpr decltype(auto) get() const noexcept {
            auto &&e = std::get<I>(elems);
            if constexpr (requires { e.get(); })
                return (e.get());
            else
                return (e);
        }

        // allow implicit conversion to tuple of unwrapped references/values
        constexpr operator std::tuple<std::remove_cvref_t<
                std::unwrap_reference_t<Elems>>...>() const {
            return std::apply([](auto&&... e) {
                return std::tuple<std::remove_cvref_t<
                        std::unwrap_reference_t<Elems>>...>{unwrap_ref(e)...};
            }, elems);
        }

    private:
        // unwrap reference_wrapper or forward the value
        static constexpr auto&& unwrap_ref(auto&& x) {
            if constexpr (requires { x.get(); })
                return x.get();
            else
                return std::forward<decltype(x)>(x);
        }
    };


    /**
     * @brief Retrieves the <code>I</code>-th element from a <code>zip_reference_proxy</code>.
     *
     * <p>
     * This free function provides tuple-like access to elements within a
     * <code>zip_reference_proxy</code>, enabling structured bindings and
     * compatibility with <code>std::get</code> and <code>std::tuple_element</code>.
     * </p>
     *
     * <p>
     * Internally, this simply forwards to
     * <code>p.get&lt;I&gt;()</code>, preserving reference semantics and
     * ensuring that both direct and wrapped elements yield proper references.
     * </p>
     *
     * @tparam I The index of the element to retrieve.
     * @tparam Elems Parameter pack of stored element types.
     * @param p The <code>zip_reference_proxy</code> instance to access.
     * @return The <code>I</code>-th element reference from the proxy.
     *
     * @see jh::ranges::zip_reference_proxy
     * @see std::get
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
     * corresponding sentinel — ensuring short-circuit termination on the
     * shortest range.
     * </p>
     *
     * @tparam Sentinels The sentinel types associated with the zipped ranges.
     *
     * @see jh::ranges::zip_iterator
     * @see jh::ranges::zip_view
     */
    template<typename... Sentinels>
    struct zip_sentinel {
        /// Tuple of underlying range end iterators.
        std::tuple<Sentinels...> ends;
    };

    /**
     * @brief Iterator type for <code>jh::ranges::zip_view</code>.
     *
     * <p>
     * <code>zip_iterator</code> traverses multiple input ranges in lockstep,
     * dereferencing into a <code>zip_reference_proxy</code> that aggregates
     * element references from each underlying range.
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
    struct [[maybe_unused]] zip_iterator {
        std::tuple<Iters...> iters;

        using difference_type   = std::ptrdiff_t;
        using value_type        = std::tuple<std::remove_cvref_t<std::iter_value_t<Iters>>...>;
        using reference         = zip_reference_proxy<std::iter_reference_t<Iters>...>;
        using iterator_category [[maybe_unused]] = std::input_iterator_tag;
        using iterator_concept [[maybe_unused]] = std::input_iterator_tag;

        constexpr reference operator*() const {
            auto packed = tuple_transform([](auto &it) -> decltype(auto) {
                return detail::wrap_element(*it);
            }, iters);
            return std::apply([](auto &&... args) {
                return reference{std::tuple{std::forward<decltype(args)>(args)...}};
            }, packed);
        }

        constexpr zip_iterator &operator++() {
            tuple_transform([](auto &it) -> auto & { ++it; return it; }, iters);
            return *this;
        }

        constexpr zip_iterator operator++(int) {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        constexpr bool operator==(const zip_iterator &other) const {
            return [&]<std::size_t... I>(std::index_sequence<I...>) {
                return ((std::get<I>(iters) == std::get<I>(other.iters)) && ...);
            }(std::index_sequence_for<Iters...>{});
        }

        constexpr bool operator!=(const zip_iterator &other) const {
            return !(*this == other);
        }

        template<typename... Sentinels>
        constexpr bool operator==(const zip_sentinel<Sentinels...> &s) const {
            return [&]<std::size_t... I>(std::index_sequence<I...>) {
                return ((std::get<I>(iters) == std::get<I>(s.ends)) || ...);
            }(std::index_sequence_for<Iters...>{});
        }

        template<typename... Sentinels>
        constexpr bool operator!=(const zip_sentinel<Sentinels...> &s) const {
            return !(*this == s);
        }
    };

    template<typename... Iters, typename... Sentinels>
    constexpr bool operator==(const zip_sentinel<Sentinels...>& s,
                              const zip_iterator<Iters...>& it) {
        return it == s;
    }

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
     * The class follows the same observable semantics as
     * <code>std::ranges::zip_view</code> (C++23), providing a transparent
     * fallback implementation for C++20 environments.
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
    template<std::ranges::view... Views>
    class zip_view : public std::ranges::view_interface<zip_view<Views...>> {
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
         */
        constexpr auto end() {
            return zip_sentinel{
                    tuple_transform([](auto &v) { return std::ranges::end(v); }, bases)
            };
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
        constexpr auto begin() const {
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
        constexpr auto end() const {
            return zip_sentinel{
                    tuple_transform([](auto const &v) { return std::ranges::end(v); }, bases)
            };
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

    template<std::size_t I, typename... Elems>
    struct tuple_element<I, jh::ranges::zip_reference_proxy<Elems...>> {
        using type = decltype(std::declval<jh::ranges::zip_reference_proxy<Elems...>>().template get<I>());
    };
}
#endif
