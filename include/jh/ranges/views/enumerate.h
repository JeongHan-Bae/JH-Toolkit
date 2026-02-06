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
 * @file enumerate.h
 * @brief <code>jh::ranges::views::enumerate</code> &mdash; sequence-aware enumerate adaptor.
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 * @see jh::ranges::views::zip
 *
 * <p>
 * This adaptor provides a Python-like <code>enumerate</code> view,
 * implemented via <code>jh::ranges::views::zip</code> and <code>std::views::iota</code>.
 * It pairs each element of a sequence with an increasing index.
 * </p>
 *
 * <p><b>Usage</b> (same form as <code>zip</code>):</p>
 * @code
 * for (auto [i, v] : enumerate(seq)) { ... }       // start from 0
 * for (auto [i, v] : enumerate(seq, 10)) { ... }   // start from 10
 * for (auto [i, v] : seq | enumerate(5)) { ... }   // pipe form
 * @endcode
 *
 * <p><b>About index type (<code>diff_t</code>):</b></p>
 * <ul>
 *   <li>Deduced via <code>jh::concepts::sequence_difference_t&lt;std::remove_cvref_t&lt;Seq&gt;&gt;</code>.</li>
 *   <li>If the associated iterator defines a valid <code>difference_type</code>,
 *       or supports subtraction (<code>it - it</code>), that type is used
 *       (they must be consistent if both exist).</li>
 *   <li>If neither is available, the type falls back to <code>std::ptrdiff_t</code>.</li>
 *   <li>Conflicting iterator traits (mismatched <code>difference_type</code> and subtraction result)
 *       invalidate the type &mdash; such a type does not model <code>jh::concepts::sequence</code>.</li>
 *   <li><code>start</code> is <code>static_cast</code> to <code>diff_t</code>; invalid conversions are ill-formed.</li>
 * </ul>
 *
 * @note Behavior, composition, and pipeline rules are identical to <code>zip</code>.
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */
#pragma once

#include "jh/ranges/views/zip.h"

namespace jh::ranges::views {

    namespace detail {

        /**
         * @brief Closure type enabling pipe syntax for <tt>enumerate</tt>.
         *
         * @tparam Diff The integer difference type used for indexing.
         */
        template<typename Diff>
        struct [[maybe_unused]] enumerate_closure final {
            Diff start;

            template<jh::concepts::sequence Seq>
            constexpr auto operator()(Seq &&seq) const {
                using diff_t = jh::concepts::sequence_difference_t<std::remove_cvref_t<Seq>>;
                return jh::ranges::views::zip(
                        std::views::iota(static_cast<diff_t>(start)),
                        std::forward<Seq>(seq)
                );
            }

            template<jh::concepts::sequence Seq>
            friend constexpr auto operator|(Seq &&lhs, const enumerate_closure &rhs) {
                return rhs(std::forward<Seq>(lhs));
            }
        };

        struct enumerate_fn final {
        private:
            /// @brief real implementation, based on deduced index type
            template<jh::concepts::sequence Seq>
            static constexpr auto impl(Seq &&seq,
                                       jh::concepts::sequence_difference_t<std::remove_cvref_t<Seq>> start) {
                using diff_t = jh::concepts::sequence_difference_t<std::remove_cvref_t<Seq>>;
                return jh::ranges::views::zip(
                        std::views::iota(static_cast<diff_t>(start)),
                        std::forward<Seq>(seq)
                );
            }

        public:
            /**
             * @brief Direct call form.
             * Automatically deduces the index type from the given sequence.
             *
             * @tparam Seq The sequence type.
             * @param seq The sequence to enumerate.
             * @param start The starting index (any integral type, implicitly convertible).
             * @return A <tt>zip_view</tt> combining an index iota view with the sequence.
             */
            template<jh::concepts::sequence Seq, typename Int>
            constexpr auto operator()(Seq &&seq, Int start = 0) const {
                using diff_t = jh::concepts::sequence_difference_t<std::remove_cvref_t<Seq>>;
                return impl(std::forward<Seq>(seq), static_cast<diff_t>(start));
            }

            /**
             * @brief Pipe form factory.
             *
             * Produces a closure capturing the starting index.
             */
            template<typename Int = std::ptrdiff_t>
            constexpr auto operator()(Int start = 0) const {
                return enumerate_closure<Int>{start};
            }
        };

    } // namespace detail

    /**
     * @brief The user-facing <tt>enumerate</tt> adaptor.
     *
     * @details
     * Provides a unified interface for enumerating a sequence with indices,
     * implemented as a <code>jh::ranges::zip_view</code> combining an index
     * <code>std::views::iota</code> with the given sequence.
     * Supports both direct and pipe syntax:
     * <ul>
     *   <li><b>Direct:</b> <code>auto e = jh::ranges::views::enumerate(seq);</code></li>
     *   <li><b>Pipe:</b> <code>auto e = seq | jh::ranges::views::enumerate();</code></li>
     * </ul>
     *
     * The starting index can be optionally specified; defaults to <code>0</code> if omitted:
     * <ul>
     *   <li><code>enumerate(seq, start)</code></li>
     *   <li><code>seq | enumerate(start)</code></li>
     * </ul>
     *
     * @see jh::ranges::views::zip
     */
    inline constexpr detail::enumerate_fn enumerate{};

} // namespace jh::ranges::views
