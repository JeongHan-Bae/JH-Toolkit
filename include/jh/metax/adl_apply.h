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
 * @file adl_apply.h
 * @brief ADL-enabled universal tuple application utility &mdash; extends <code>std::apply</code>
 *        to arbitrary tuple-like structures, including user-defined proxies and view elements.
 * @author
 *   JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 *
 * <p>
 * The <code>jh::meta::adl_apply</code> function is a generalized alternative to
 * <code>std::apply</code> that allows invocation of callables on both standard
 * and user-defined <em>tuple-like</em> objects.
 * It performs unqualified lookup for <code>get&lt;I&gt;</code> to enable
 * Argument-Dependent Lookup (ADL), thereby extending support beyond the
 * standard tuple family.
 * </p>
 *
 * <h3>Motivation</h3>
 * <p>
 * The C++ standard restricts <code>std::apply</code> to built-in tuple-like
 * types (<code>std::tuple</code>, <code>std::pair</code>, <code>std::array</code>),
 * because <code>std::apply</code> directly calls <code>std::get&lt;I&gt;</code>.
 * However:
 * </p>
 * <ul>
 *   <li>User-defined <code>get&lt;I&gt;</code> functions cannot reside in <code>std</code> namespace;</li>
 *   <li><code>std::get</code> cannot be specialized for non-standard types;</li>
 *   <li>C++ nevertheless permits defining <code>tuple_size</code> and
 *       <code>tuple_element</code> for custom structures;</li>
 *   <li>Thus, a large class of <em>tuple-like proxies</em> (e.g., <code>jh::ranges::zip_reference_view</code>
 *       supporting <code>jh::ranges::zip_view</code>) as well as a lot of third-party defined tuple-like proxies
 *       cannot work with <code>std::apply</code>.</li>
 * </ul>
 *
 * <p>
 * <code>jh::meta::adl_apply</code> bridges this semantic gap by employing
 * unqualified <code>get&lt;I&gt;</code> calls, so both standard and ADL-visible
 * overloads participate in lookup. It effectively becomes a
 * <b>universal apply</b> for all tuple-like types.
 * </p>
 *
 * <h3>Supported types</h3>
 * <ul>
 *   <li><b>Standard tuple-likes:</b> <code>std::tuple</code>, <code>std::pair</code>, <code>std::array</code>.</li>
 *   <li><b>User-defined tuple-likes:</b> types modeling <code>jh::concepts::tuple_like</code>
 *       (i.e., define <code>tuple_size</code> / <code>tuple_element</code> and expose
 *       ADL-visible <code>get&lt;I&gt;</code>).</li>
 *   <li><b>Proxy-based ranges:</b> view elements such as
 *       <code>jh::ranges::zip_reference_view</code>.</li>
 * </ul>
 *
 * <h3>Usage example</h3>
 * @code
 * namespace demo {
 *   struct proxy { int i; double d; };
 *
 *   // Define ADL-visible get&lt;I&gt; overloads
 *   template&lt;std::size_t I&gt;
 *   decltype(auto) get(proxy &amp;p) noexcept {
 *     if constexpr (I == 0) return (p.i);
 *     else return (p.d);
 *   }
 * }
 *
 * namespace std {
 *   template&lt;&gt;
 *   struct std::tuple_size&lt;proxy&gt; : std::integral_constant&lt;size_t, 2&gt; {};
 *   template&lt;size_t I&gt;
 *   struct std::tuple_element&lt;I, proxy&gt; {
 *     using type = std::conditional_t&lt;I == 0, int, double&gt;;
 *   };
 * }
 *
 * demo::proxy p{1, 3.14};
 *
 * // Works for both std and ADL get&lt;I&gt;
 * jh::meta::adl_apply([](auto &amp;&amp;x, auto &amp;&amp;y) {
 *   std::cout &lt;&lt; x &lt;&lt; ", " &lt;&lt; y &lt;&lt; '&bsol;n';
 * }, p);
 * @endcode
 *
 * <h3>Behavior summary</h3>
 * <ul>
 *   <li>Performs unqualified lookup for <code>get&lt;I&gt;</code> &mdash; enabling ADL discovery.</li>
 *   <li>Expands index sequence from <code>std::tuple_size_v&lt;T&gt;</code>.</li>
 *   <li>Perfect-forwards both callable and tuple object.</li>
 *   <li>Propagates <code>noexcept</code> and <code>constexpr</code> guarantees.</li>
 * </ul>
 *
 * <h3>Integration example (with collect)</h3>
 * @code
 * auto result = range
 *   | jh::ranges::views::enumerate()
 *   | jh::ranges::collect&lt;std::vector&lt;std::pair&lt;size_t, std::string&gt;&gt;&gt;();
 * @endcode
 *
 * <p>
 * In this example, <code>enumerate()</code> yields a tuple-like proxy containing
 * an index and reference. <code>collect</code> internally employs
 * <code>jh::meta::adl_apply</code> to unpack the proxy into actual
 * <code>std::pair&lt;size_t, std::string&gt;</code> values via <code>emplace_back()</code>.
 * </p>
 *
 * <h3>Design rationale</h3>
 * <ul>
 *   <li><b>Language conformance:</b> avoids illegal specialization of <code>std::get</code>.</li>
 *   <li><b>ADL openness:</b> honors user-defined <code>get&lt;I&gt;</code> functions.</li>
 *   <li><b>Compile-time safety:</b> constrained by <code>jh::concepts::tuple_like</code>.</li>
 *   <li><b>Zero-overhead abstraction:</b> identical assembly as <code>std::apply</code> for STL tuples.</li>
 * </ul>
 *
 * @see jh::concepts::tuple_like
 * @see std::apply
 *
 * @version <pre>1.3.x</pre>
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
     * @brief Internal implementation helper for <code>jh::meta::adl_apply</code>.
     * @details
     * Expands a tuple-like object <code>t</code> into individual elements and
     * invokes <code>f</code> with them via unqualified <code>get&lt;I&gt;</code> lookup.
     *
     * <p>
     * Unlike <code>std::apply</code>, this implementation deliberately omits
     * <code>using std::get;</code> to preserve Argument-Dependent Lookup (ADL).
     * Bringing <code>std::get</code> into scope would suppress ADL resolution
     * and force all <code>get&lt;I&gt;</code> calls to bind to <code>std::get</code>,
     * breaking compatibility with user-defined tuple-like types that rely on
     * ADL-visible <code>get&lt;I&gt;</code> overloads.
     * </p>
     *
     * <p>
     * This choice allows both standard tuple-likes
     * (<code>std::tuple</code>, <code>std::pair</code>, <code>std::array</code>)
     * and user-defined tuple-like structures to be expanded uniformly without
     * violating the one-definition rule for <code>std::get</code>.
     * </p>
     *
     * <p>
     * In summary, <b><code>using std::get</code> is incorrect here</b> because it
     * prevents ADL participation, effectively disabling lookup of valid
     * user-defined <code>get&lt;I&gt;</code> functions.
     * </p>
     *
     * @tparam F Callable type.
     * @tparam T Tuple-like object type satisfying <code>jh::concepts::tuple_like</code>.
     * @tparam I... Compile-time indices derived from <code>std::tuple_size_v&lt;T&gt;</code>.
     * @param f Callable to be invoked with unpacked elements.
     * @param t Tuple-like object to unpack.
     * @return The result of invoking <code>f</code> with the elements of <code>t</code>.
     */
    template<class F, class T, size_t... I>
    constexpr decltype(auto) adl_apply_impl(F&& f, T&& t, std::index_sequence<I...>) {
        return std::invoke(static_cast<F&&>(f),
                           get<I>(static_cast<T&&>(t))...);
    }

    /**
     * @brief ADL-enabled universal apply for tuple-like objects.
     * @details
     * Invokes a callable object <code>f</code> with the unpacked elements of a
     * tuple-like <code>t</code>, performing unqualified lookup for
     * <code>get&lt;I&gt;</code> so that both standard and user-defined tuple-likes
     * are supported.
     *
     * <p>
     * This function is conceptually equivalent to <code>std::apply</code>, but
     * designed for broader compatibility: it requires only that the type model
     * <code>jh::concepts::tuple_like</code>, not that <code>std::get</code> be
     * specialized.
     * Standard library tuples continue to resolve <code>std::get</code> normally,
     * while user-defined types participate via ADL.
     * </p>
     *
     * @tparam F Callable type.
     * @tparam T Tuple-like type satisfying <code>jh::concepts::tuple_like</code>.
     * @param f Callable object to invoke.
     * @param t Tuple-like object whose elements are unpacked and forwarded.
     * @return The result of <code>std::invoke(f, get&lt;I&gt;(t)...)</code>.
     *
     * @note
     * ADL lookup ensures correctness for user-defined tuple-like proxies and
     * prevents illegal specialization of <code>std::get</code>.
     * <br>
     * This approach is fully conforming and generates identical code to
     * <code>std::apply</code> for standard tuple types.
     */
    template<class F, jh::concepts::tuple_like T>
    constexpr decltype(auto) adl_apply(F&& f, T&& t)
    noexcept(noexcept(adl_apply_impl(std::forward<F>(f), std::forward<T>(t),
                                     std::make_index_sequence<
                                             std::tuple_size_v<std::remove_cvref_t<T>>>{})))
    {
        return adl_apply_impl(std::forward<F>(f), std::forward<T>(t),
                              std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<T>>>{});
    }

} // namespace jh::meta
