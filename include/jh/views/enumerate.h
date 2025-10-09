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
 * @file enumerate.h
 * @author JeongHan-Bae <mastropseudo@gmail.com>
 * @brief Provides a lazy enumeration view over any jh::sequence-compatible range.
 *
 * @details
 * This header defines `jh::views::enumerate`, a utility that transforms a sequence
 * into a lazily evaluated range of `(index, value)` pairs.
 *
 * The resulting view supports structured bindings and adapts its behavior based on the
 * underlying element type:
 * - If the value type is POD-like, a `jh::pod::pair<std::uint64_t, T>` is returned.
 * - If the value type is non-POD, a `jh::utils::ref_pair<const std::uint64_t, T&>` is returned.
 *
 * This ensures efficient behavior and safe reference semantics without unnecessary copying.
 *
 * Key properties:
 * - Compatible with all `jh::sequence`-satisfying containers.
 * - Requires only input-iterator semantics; no random-access assumption.
 * - Produces values lazily via custom iterator with internal index tracking.
 *
 * Intended for use in range-based for loops and composable view pipelines.
 *
 */


#pragma once

#include <ranges>
#include <cstdint>

#include "jh/utils/pair.h"
#include "jh/sequence.h"

namespace jh::views {
    /**
      * @brief Lazily enumerate a sequence, yielding (index, value) pairs.
      *
      * This view lazily pairs each index with its corresponding value from the input sequence.
      * The `enumerate()` function can be used to produce a range of index-value pairs where:
      * - For each element in the sequence, a pair of its index and the element itself is generated.
      * - The sequence is iterated element by element, regardless of whether the sequence supports random-access.
      *
      * @tparam R Type of the input sequence (must satisfy jh::sequence).
      * @param r The sequence to enumerate.
      * @return A view producing pair-like values of (index, element).
      *
      * @note The implementation supports all types of iterators, including input iterators.
      *       It does not rely on random-access iterators. Thus, this view will iterate over the sequence element by element.
      *       If the sequence supports random access, performance will still benefit from the iterator being accessed by index.
      *       Each element is evaluated lazily at access time.
      */
    template<sequence R>
    auto enumerate(const R &r) {
        auto begin_it = r.begin();
        auto end_it = r.end();

        using It = decltype(begin_it);
        using Sent = decltype(end_it);

        struct EnumerateView : std::ranges::view_interface<EnumerateView> {
            It it;
            Sent end_it;

            EnumerateView(It begin, Sent end) : it(begin), end_it(end) {
            }

            struct iterator {
                It current;
                Sent end;
                std::uint64_t index = 0;

                auto operator*() const {
                    auto &&val = *current;
                    if constexpr (jh::pod::pod_like<std::decay_t<decltype(val)> >) {
                        return jh::pod::pair<std::uint64_t, std::decay_t<decltype(val)> >{index, val};
                    } else {
                        return jh::utils::ref_pair<const std::uint64_t, decltype(val)>{index, val};
                    }
                }

                iterator & operator++() {
                    ++current;
                    ++index;
                    return *this;
                }

                bool operator!=(const iterator &other) const {
                    return current != other.current;
                }
            };

            iterator begin() const { return {it, end_it}; }
            iterator end() const { return {end_it, end_it}; }
        };

        return EnumerateView{begin_it, end_it};
    }
} // namespace jh::views
