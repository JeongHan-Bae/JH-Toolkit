/**
* Copyright 2025 JeongHan-Bae <mastropseudo@gmail.com>
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
 */

/**
 * @file zip.h
 * @author JeongHan-Bae <mastropseudo@gmail.com>
 * @brief Provides a lazy zip view over two jh::sequence-compatible ranges.
 *
 * @details
 * This header defines `jh::views::zip`, a utility that lazily combines two sequences
 * into a view of pair-like values. The resulting view allows parallel iteration over
 * both sequences, producing `(a[i], b[i])` on each step.
 *
 * The returned elements automatically adapt based on the value types:
 * - If both element types are POD-like, a `jh::pod::pair<T1, T2>` is returned.
 * - If either element is non-POD, a `jh::utils::ref_pair<T1&, T2&>` is returned.
 *
 * This allows safe and efficient structural binding without unintended copies.
 *
 * Characteristics:
 * - Compatible with any types satisfying the `jh::sequence` concept.
 * - The resulting zipped view is limited to the shorter of the two sequences.
 * - Internally tracks index and accesses elements via `.begin()[i]`.
 * - Evaluates values lazily at access time.
 */

#pragma once


#include "../utils/pair.h"
#include "../sequence.h" // jh::sequence

namespace jh::views {
    /**
     * @brief Lazily zips two sequences into a pair-like range.
     *
     * @tparam R1 Type of the first sequence (must satisfy jh::sequence)
     * @tparam R2 Type of the second sequence (must satisfy jh::sequence)
     * @param a First sequence
     * @param b Second sequence
     * @return A view of pair-like values zipped from both sequences.
     *
     * @note The size of the zipped view is the minimum of both sequence lengths.
     * @note Each element is evaluated lazily at access time.
     */
    template<sequence R1, sequence R2>
    class zip_view : public std::ranges::view_interface<zip_view<R1, R2> > {
    public:
        zip_view(const R1 &a, const R2 &b)
            : a_(a), b_(b),
              size_(std::min(static_cast<uint64_t>(std::ranges::size(a)),
                             static_cast<uint64_t>(std::ranges::size(b)))) {
        }

        class iterator {
        public:
            using index_t = std::uint64_t;

            iterator(const R1 &a, const R2 &b, index_t i)
                : a_(a), b_(b), i_(i) {
            }

            auto operator*() const {
                auto &&lhs = a_.begin()[i_];

                if constexpr (auto &&rhs = b_.begin()[i_];
                    jh::pod::pod_like<std::decay_t<decltype(lhs)> > &&
                    jh::pod::pod_like<std::decay_t<decltype(rhs)> >) {
                    // POD → safe to copy
                    return jh::pod::pair<std::decay_t<decltype(lhs)>, std::decay_t<decltype(rhs)> >{
                        lhs, rhs
                    };
                } else {
                    // Non-POD → Ref Pair
                    return utils::ref_pair{lhs, rhs};
                }
            }

            iterator &operator++() {
                ++i_;
                return *this;
            }

            bool operator!=(const iterator &other) const {
                return i_ != other.i_;
            }

        private:
            const R1 &a_;
            const R2 &b_;
            index_t i_;
        };

        auto begin() const { return iterator{a_, b_, 0}; }
        auto end() const { return iterator{a_, b_, size_}; }

    private:
        const R1 &a_;
        const R2 &b_;
        std::uint64_t size_;
    };

    /**
     * @brief Creates a zip view over two sequences.
     *
     * @tparam R1 Type of the first sequence (must satisfy jh::sequence)
     * @tparam R2 Type of the second sequence (must satisfy jh::sequence)
     * @param a First sequence
     * @param b Second sequence
     * @return A zip view over the two sequences.
     */
    template<sequence R1, sequence R2>
    auto zip(const R1 &a, const R2 &b) {
        return zip_view<R1, R2>{a, b};
    }
}
