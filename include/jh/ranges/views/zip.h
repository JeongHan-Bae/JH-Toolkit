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
 * @file zip.h
 * @brief Extended <code>zip</code> view adaptor compatible with <code>jh::concepts::sequence</code>.
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 *
 * <p>
 * This header provides the JH-Toolkit's sequence-aware
 * <code>jh::ranges::views::zip</code> adaptor. It converts
 * <code>jh::concepts::sequence</code> inputs to standard ranges before
 * constructing <code>jh::ranges::zip_view</code>.
 * </p>
 *
 * <p>
 * The resulting <code>jh::ranges::zip_view</code> models
 * <code>std::ranges::view</code>. Its traversal category depends on the
 * converted inputs and the selected Zip view implementation.
 * </p>
 *
 * <p><b>Behavior overview:</b></p>
 * <ul>
 *   <li><b>Direct call:</b><br/>
 *       <code>jh::ranges::views::zip(a, b, c)</code> constructs and returns
 *       a <code>jh::ranges::zip_view</code> directly.
 *   </li>
 *   <li><b>Single-sequence pipe call:</b><br/>
 *       <code>a | jh::ranges::views::zip(b)</code> is supported,
 *       producing a Zip view after <code>zip(b)</code> captures <code>b</code>
 *       in a closure.
 *   </li>
 *   <li><b>Pipe call (extended form):</b><br/>
 *       The companion adaptor <code>jh::ranges::views::zip_pipe</code>
 *       enables piping with multiple right-hand sequences, e.g.:
 *       <code>a | jh::ranges::views::zip_pipe(b, c, d)</code>.
 *       This closure captures multiple right-hand sequences for a later
 *       pipe call.
 *   </li>
 * </ul>
 *
 * <p><b>Summary of design goals:</b></p>
 * <ul>
 *   <li>Extend <code>zip</code> to all <code>jh::concepts::sequence</code>-compatible types.</li>
 *   <li>Support a single-sequence pipe call through a closure.</li>
 *   <li>Offer a clean, opt-in extension (<code>zip_pipe</code>) for multi-argument pipeline usage.</li>
 * </ul>
 *
 * <p><b>Typical usage:</b></p>
 * @code
 * using namespace jh::ranges::views;
 *
 * // Direct and single-sequence pipe calls:
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
 * It checks for const-accessible iteration and a comparable end sentinel.
 * Implementations of the sequence interface must uphold the non-consuming
 * iteration contract.
 * </p>
 *
 * <p>
 * Containers that model <code>std::ranges::range</code> and are <b>movable or copyable</b>
 * are forwarded directly. Non-movable and non-copyable ranges
 * (e.g. <code>jh::runtime_arr&lt;T&gt;</code>) are proxied by
 * <code>std::ranges::subrange</code> to ensure safe reference semantics.
 * For other sequence-like types that do not formally model <code>std::ranges::range</code>,
 * <code>jh::to_range()</code> constructs a <code>jh::ranges::range_adaptor</code>,
 * which actively completes missing iterator traits and category tags through
 * <code>jh::ranges::detail::completed_iterator</code>.
 * </p>
 *
 * <p>
 * As a result, supported <code>sequence</code> inputs &mdash; even types lacking standard iterator
 * typedefs &mdash; can be <b>semantically promoted</b> into a fully compliant
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
#include "jh/metax/adl_apply.h"
#include "jh/ranges/zip_view.h"

namespace jh::ranges::views {

    namespace detail {

        /**
         * @brief Closure type used to enable pipe syntax for <code>zip</code>.
         *
         * @details
         * This structure stores a tuple of pre-converted <tt>view</tt> objects
         * (via <code>std::views::all</code>), allowing a later sequence
         * to be "piped" into the zip adaptor using the <code>|</code> operator.
         *
         * <p>For example:</p>
         * @code
         * auto zipped = some_sequence | jh::ranges::views::zip_pipe(other_seq, third_seq);
         * @endcode
         *
         * @tparam Views The types of the captured view objects.
         */
        template <typename... Views>
        struct [[maybe_unused]] zip_closure final {
            /**
             * @brief The tuple storing the captured view objects.
             */
            std::tuple<Views...> views;

        private:
            template<typename Self, jh::concepts::sequence R>
            static constexpr auto apply(Self&& self, R&& r) {
                return jh::meta::adl_apply(
                        [&](auto&&... v) {
                            return jh::ranges::zip_view{
                                    std::views::all(jh::to_range(std::forward<R>(r))),
                                    std::forward<decltype(v)>(v)...
                            };
                        },
                        std::forward<Self>(self).views
                );
            }

        public:

            /**
             * @brief Applies the stored views to a new range.
             *
             * @tparam R Type of the left-hand side range or sequence.
             * @param r The range to be combined with the stored views.
             * @return A <code>jh::ranges::zip_view</code> combining <code>r</code> with all captured views.
             *
             * @details
             * This function is invoked when the closure is called directly or
             * when the pipe operator <code>|</code> is used. It constructs a
             * <code>jh::ranges::zip_view</code> containing all views.
             */
            template <jh::concepts::sequence R>
            constexpr auto operator()(R&& r) const &
                requires (std::copy_constructible<Views> && ...) {
                return apply(*this, std::forward<R>(r));
            }

            /**
             * @brief Applies a temporary closure by moving its captured views.
             * @tparam R Left-hand sequence type.
             * @param r The sequence to prepend.
             * @return A Zip view owning any captured rvalue views.
             */
            template <jh::concepts::sequence R>
            constexpr auto operator()(R&& r) && {
                return apply(std::move(*this), std::forward<R>(r));
            }

            /**
             * @brief Enables <code>range | zip_closure</code> syntax.
             *
             * @tparam R The range type used as the left-hand side operand.
             * @param lhs The left-hand side range.
             * @param rhs The right-hand side <code>zip_closure</code>.
             * @return A <code>jh::ranges::zip_view</code> that zips <code>lhs</code> with all captured views.
             *
             * @details
             * This overload allows expressions like:
             * <pre><code>
             * auto zipped = seq1 | jh::ranges::views::zip_pipe(seq2, seq3);
             * </code></pre>
             */
            template <jh::concepts::sequence R>
            friend constexpr auto operator|(R&& lhs, const zip_closure& rhs)
                requires (std::copy_constructible<Views> && ...) {
                return rhs(std::forward<R>(lhs));
            }

            /**
             * @brief Pipes a sequence into a temporary Zip closure.
             * @tparam R Left-hand sequence type.
             * @param lhs The sequence to prepend.
             * @param rhs A temporary closure whose captured views are moved.
             * @return A Zip view containing the converted sequences.
             */
            template <jh::concepts::sequence R>
            friend constexpr auto operator|(R&& lhs, zip_closure&& rhs) {
                return std::move(rhs)(std::forward<R>(lhs));
            }
        };

        /**
         * @brief Function object implementing the <code>zip</code> view adaptor.
         *
         * @details
         * This callable object supports two forms of usage:
         * <ul>
         *   <li><b>Direct call form:</b> <code>zip(a, b, c)</code> returns a zip_view combining all arguments.</li>
         *   <li><b>Pipe form:</b> <code>seq | zip(other_seq)</code> zips <code>seq</code> with the captured view.</li>
         * </ul>
         *
         * <p>Internally, the single-argument version produces a <code>zip_closure</code>
             * object that stores the captured views. Lvalue sources are
             * borrowed; rvalue sources are owned by their converted views.</p>
         */
        struct zip_fn final {
            /**
             * @brief Zips multiple sequences directly.
             *
             * @tparam Seq Parameter pack of sequence types.
             * @param seqs The sequences to be zipped.
             * @return A <code>jh::ranges::zip_view</code> containing all input ranges.
             *
             * @details
             * Each sequence is converted to a view using <code>jh::to_range()</code>
             * and <code>std::views::all()</code>, ensuring compatibility with
             * various custom sequence types.
             *
             * <p>Example:</p>
             * <pre><code>
             * auto zipped = jh::ranges::views::zip(a, b, c);
             * </code></pre>
             */
            template <jh::concepts::sequence... Seq>
            constexpr auto operator()(Seq&&... seqs) const {
                return jh::ranges::zip_view{
                        std::views::all(jh::to_range(std::forward<Seq>(seqs)))...
                };
            }

            /**
             * @brief Creates a <code>zip_closure</code> capturing a single sequence.
             *
             * @tparam Seq Sequence type to capture.
             * @param seq The sequence to capture into the closure.
             * @return A <code>zip_closure</code> that can be used with pipe syntax.
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
                using View = decltype(std::views::all(jh::to_range(std::forward<Seq>(seq))));
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
         * This adaptor returns a closure that captures the right-hand
         * sequences for a later pipe call.
         */
        struct zip_pipe_fn final {
            template <jh::concepts::sequence... Seq>
            constexpr auto operator()(Seq&&... seqs) const {
                return zip_closure<decltype(std::views::all(jh::to_range(std::forward<Seq>(seqs))))...>{
                        std::tuple{ std::views::all(jh::to_range(std::forward<Seq>(seqs)))... }
                };
            }
        };

    } // namespace detail

    /**
     * @brief The user-facing <code>zip</code> adaptor.
     *
     * @details
     * Provides a unified interface for zipping multiple sequences into a
     * <code>jh::ranges::zip_view</code>. Supports both direct and pipe syntax:
     * <ul>
     *   <li><b>Direct:</b> <code>auto z = jh::ranges::views::zip(a, b);</code></li>
     *   <li><b>Pipe:</b> <code>auto z = a | jh::ranges::views::zip(b);</code></li>
     * </ul>
     */
    inline constexpr detail::zip_fn zip{};

    /**
     * @brief Extended multi-argument zip adaptor for pipe syntax.
     *
     * <strong>Supports:</strong>
     * <ul>
     *    <li>Pipe with multiple arguments: <code>a | zip_pipe(b, c, d)</code></li>
     *    <li>Always returns a closure.</li>
     * </ul>
     * @note
     * <code>zip_pipe</code> always produces a closure, enabling multi-sequence
     * pipe syntax:
     * @code
     * auto z1 = a | jh::ranges::views::zip(b);
     * auto z2 = a | jh::ranges::views::zip_pipe(b,c);
     * @endcode
     *
     * <code>zip_pipe</code> captures multiple right-hand sequences and builds
     * the view when a left-hand sequence is supplied.
     */
    inline constexpr detail::zip_pipe_fn zip_pipe{};

} // namespace jh::ranges::views
