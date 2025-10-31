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
 * @file zip.h (ranges/views)
 * @brief Extended <code>zip</code> view adaptor compatible with <code>jh::concepts::sequence</code>.
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 *
 * <p>
 * This header provides <code>jh::ranges::views::zip</code> — a <b>conceptually aligned but generalized</b>
 * version of the C++23 <code>std::views::zip</code> adaptor.
 * It extends the applicability of <code>zip</code> to custom <code>jh::concepts::sequence</code>
 * types, which may not be standard ranges but still define explicit iteration semantics
 * (non-consuming begin/end pairs).
 * </p>
 *
 * <p>
 * The resulting view type is <code>jh::ranges::zip_view</code>,
 * which models both <code>std::ranges::view</code> and <code>jh::concepts::sequence</code>.
 * This dual compatibility allows seamless interoperability between STL-style range adaptors
 * and <code>jh</code>-specific abstractions.
 * </p>
 *
 * <p><b>Behavior overview:</b></p>
 * <ul>
 *   <li><b>Direct call:</b><br/>
 *       <code>jh::ranges::views::zip(a, b, c)</code> constructs and returns
 *       a <code>jh::ranges::zip_view</code> directly, just like C++23 <code>std::views::zip</code>.
 *   </li>
 *   <li><b>Pipe call (standard form):</b><br/>
 *       <code>a | jh::ranges::views::zip(b)</code> is supported,
 *       producing a <code>zip_view&lt;a,b&gt;</code>. This exactly matches
 *       the standard C++23 behavior — only <em>one</em> right-hand sequence is allowed.
 *   </li>
 *   <li><b>Pipe call (extended form):</b><br/>
 *       The companion adaptor <code>jh::ranges::views::zip_pipe</code>
 *       enables piping with multiple right-hand sequences, e.g.:
 *       <code>a | jh::ranges::views::zip_pipe(b, c, d)</code>.
 *       This is a <b>semantic extension</b> of the standard pipeline form,
 *       generalized for <code>jh::concepts::sequence</code> sources.
 *       It preserves full compositional compatibility while providing
 *       broader applicability.
 *   </li>
 * </ul>
 *
 * <p><b>Summary of design goals:</b></p>
 * <ul>
 *   <li>Extend <code>zip</code> to all <code>jh::concepts::sequence</code>-compatible types.</li>
 *   <li>Preserve the C++23 behavioral contract for single-argument pipelines.</li>
 *   <li>Offer a clean, opt-in extension (<code>zip_pipe</code>) for multi-argument pipeline usage.</li>
 * </ul>
 *
 * <p><b>Typical usage:</b></p>
 * @code
 * using namespace jh::ranges::views;
 *
 * // Standard usage (same as C++23):
 * for (auto [x, y] : zip(a, b)) { ... }
 * for (auto [x, y] : a | zip(b)) { ... }
 *
 * // Extended multi-sequence pipe:
 * for (auto [x, y, z] : a | zip_pipe(b, c)) { ... }
 * @endcode
 *
 * <p><b>About <code>jh::concepts::sequence</code> and compatibility:</b></p>
 * <p>
 * The <code>jh::concepts::sequence</code> abstraction is designed to enable
 * <b>third-party or non-standard containers</b> to participate naturally
 * in the range adaptor ecosystem.
 * It recognizes any type that supports <em>non-consuming iteration</em> —
 * that is, a const-accessible <code>begin()</code> and a deducible <code>end()</code>
 * (including static sentinels). Such types can be safely iterated multiple times
 * without altering or consuming their internal state, and are thus considered
 * valid <code>sequence</code>s.
 * </p>
 *
 * <p>
 * Unlike a typical "range wrapper" system, this layer is <b>active and semantic</b>:
 * containers that model <code>std::ranges::range</code> and are <b>movable or copyable</b>
 * are forwarded directly. Non-movable and non-copyable ranges
 * (e.g. <code>jh::runtime_arr&lt;T&gt;</code>) are proxied by
 * <code>std::ranges::subrange</code> to ensure safe reference semantics.
 * For other sequence-like types that do not formally model <code>std::ranges::range</code>,
 * <code>jh::to_range()</code> constructs a <code>jh::ranges::range_wrapper</code>,
 * which actively completes missing iterator traits and category tags through
 * <code>jh::ranges::detail::completed_iterator</code>.
 * </p>
 *
 * <p>
 * As a result, any valid <code>sequence</code> — even a type lacking standard iterator
 * typedefs — can be <b>semantically promoted</b> into a fully compliant
 * <code>std::ranges::range</code> via <code>jh::to_range(seq)</code>.
 * All <code>jh::ranges::views</code> adaptors, including <code>zip</code> and
 * <code>zip_pipe</code>, perform this proxying step automatically, ensuring that
 * user-defined sequence-like containers can directly participate in both
 * standard and extended range pipelines.
 * </p>
 *
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include <ranges>
#include <cstdint>
#include "jh/conceptual/sequence.h"
#include "jh/ranges/zip_view.h"

namespace jh::ranges::views {

    namespace detail {

        /**
         * @brief Closure type used to enable pipe syntax for <tt>zip</tt>.
         *
         * @details
         * This structure stores a tuple of pre-converted <tt>view</tt> objects
         * (via <tt>std::views::all</tt>), allowing a later sequence
         * to be "piped" into the zip adaptor using the <tt>|</tt> operator.
         *
         * <p>For example:</p>
         * @code
         * auto zipped = some_sequence | jh::ranges::views::zip(other_seq, third_seq);
         * @endcode
         *
         * @tparam Views The types of the captured view objects.
         */
        template <typename... Views>
        struct [[maybe_unused]] zip_closure {
            /**
             * @brief The tuple storing the captured view objects.
             */
            std::tuple<Views...> views;

            /**
             * @brief Applies the stored views to a new range.
             *
             * @tparam R Type of the left-hand side range or sequence.
             * @param r The range to be combined with the stored views.
             * @return A <tt>jh::ranges::zip_view</tt> combining <tt>r</tt> with all captured views.
             *
             * @details
             * This function is invoked when the closure is called directly or
             * when the pipe operator <tt>|</tt> is used. It constructs a
             * <tt>jh::ranges::zip_view</tt> containing all views.
             */
            template <jh::concepts::sequence R>
            constexpr auto operator()([[maybe_unused]] R&& r) const {
                return std::apply(
                        [&](auto&&... v) {
                            return jh::ranges::zip_view{
                                    std::views::all(jh::to_range(r)),
                                    std::forward<decltype(v)>(v)...
                            };
                        },
                        views
                );
            }

            /**
             * @brief Enables <tt>range | zip_closure</tt> syntax.
             *
             * @tparam R The range type used as the left-hand side operand.
             * @param lhs The left-hand side range.
             * @param rhs The right-hand side <tt>zip_closure</tt>.
             * @return A <tt>jh::ranges::zip_view</tt> that zips <tt>lhs</tt> with all captured views.
             *
             * @details
             * This overload allows expressions like:
             * <pre>
             * auto zipped = seq1 | jh::ranges::views::zip(seq2, seq3);
             * </pre>
             */
            template <jh::concepts::sequence R>
            friend constexpr auto operator|(R&& lhs, const zip_closure& rhs) {
                return rhs(std::forward<R>(lhs));
            }
        };

        /**
         * @brief Function object implementing the <tt>zip</tt> view adaptor.
         *
         * @details
         * This callable object supports two forms of usage:
         * <ul>
         *   <li><b>Direct call form:</b> <tt>zip(a, b, c)</tt> returns a zip_view combining all arguments.</li>
         *   <li><b>Pipe form:</b> <tt>seq | zip(other_seq)</tt> zips <tt>seq</tt> with the captured view.</li>
         * </ul>
         *
         * <p>Internally, the single-argument version produces a <tt>zip_closure</tt>
         * object that stores the captured views.</p>
         */
        struct zip_fn {
            /**
             * @brief Zips multiple sequences directly.
             *
             * @tparam Seq Parameter pack of sequence types.
             * @param seqs The sequences to be zipped.
             * @return A <tt>jh::ranges::zip_view</tt> containing all input ranges.
             *
             * @details
             * Each sequence is converted to a view using <tt>jh::to_range()</tt>
             * and <tt>std::views::all()</tt>, ensuring compatibility with
             * various custom sequence types such as <tt>jh::pod::array</tt>.
             *
             * <p>Example:</p>
             * <pre>
             * auto zipped = jh::ranges::views::zip(a, b, c);
             * </pre>
             */
            template <jh::concepts::sequence... Seq>
            constexpr auto operator()(Seq&&... seqs) const {
                return jh::ranges::zip_view{
                        std::views::all(jh::to_range(seqs))...
                };
            }

            /**
             * @brief Creates a <tt>zip_closure</tt> capturing a single sequence.
             *
             * @tparam Seq Sequence type to capture.
             * @param seq The sequence to capture into the closure.
             * @return A <tt>zip_closure</tt> that can be used with pipe syntax.
             *
             * @details
             * This overload is used when the user writes:
             * @code
             * auto f = jh::ranges::views::zip(seq2);
             * auto result = seq1 | f; // equivalent to zip(seq1, seq2)
             * @endcode
             */
            template <jh::concepts::sequence Seq>
            constexpr auto operator()(Seq&& seq) const {
                using View = decltype(std::views::all(jh::to_range(seq)));
                return zip_closure<View>{
                        std::tuple{ std::views::all(jh::to_range(std::forward<Seq>(seq))) }
                };
            }
        };

        /**
         * @brief Extended zip adaptor supporting multiple arguments in pipe form.
         *
         * @details
         * Example:
         * @code
         * auto combined = seq1 | jh::ranges::views::zip_pipe(seq2, seq3);
         * @endcode
         *
         * Unlike the standard zip, this version always returns a closure,
         * so it can be safely used in pipelines with multiple parameters.
         */
        struct zip_pipe_fn {
            template <jh::concepts::sequence... Seq>
            constexpr auto operator()(Seq&&... seqs) const {
                return zip_closure<decltype(std::views::all(jh::to_range(seqs)))...>{
                        std::tuple{ std::views::all(jh::to_range(std::forward<Seq>(seqs)))... }
                };
            }
        };

    } // namespace detail

    /**
     * @brief The user-facing <tt>zip</tt> adaptor.
     *
     * @details
     * Provides a unified interface for zipping multiple sequences into a
     * <tt>jh::ranges::zip_view</tt>. Supports both direct and pipe syntax:
     * <ul>
     *   <li><b>Direct:</b> <tt>auto z = jh::ranges::views::zip(a, b);</tt></li>
     *   <li><b>Pipe:</b> <tt>auto z = a | jh::ranges::views::zip(b);</tt></li>
     * </ul>
     */
    inline constexpr detail::zip_fn zip{};

    /**
     * @brief Extended multi-argument zip adaptor for pipe syntax.
     *
     * <strong>Supports:</strong>
     * <ul>
     *    <li>Pipe with multiple arguments: <tt>a | zip_pipe(b, c, d)</tt></li>
     *    <li>Always returns a closure.</li>
     * </ul>
     * @note
     * The <code>zip_pipe</code> adaptor resolves the inherent ambiguity of the standard <code>zip</code> adaptor:
     * in C++23, <code>zip(a, b)</code> returns a <code>zip_view</code>, while <code>zip(b)</code> produces a closure.
     * This makes multi-argument pipeline forms (<code>a | zip(b, c)</code>) impossible.
     *
     * The <code>zip_pipe</code> variant explicitly defines a <b>closure-only</b> behavior,
     * enabling expressive and unambiguous syntax:
     * @code
     * auto z1 = a | jh::ranges::views::zip(b);        // standard form
     * auto z2 = a | jh::ranges::views::zip_pipe(b,c); // extended multi-sequence form
     * @endcode
     *
     * Conceptually, <code>zip_pipe</code> is not a new feature, but a semantic resolution:
     * it restores closure semantics for multi-argument zip operations
     * while maintaining full compatibility with the C++23 range adaptor model.
     */
    inline constexpr detail::zip_pipe_fn zip_pipe{};

} // namespace jh::ranges::views
