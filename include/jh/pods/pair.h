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
 * @file pair.h (pods)
 * @author JeongHan-Bae <mastropseudo@gmail.com>
 * @brief Implementation of `jh::pod::pair<T1, T2>`.
 */

#pragma once

#include "pod_like.h"

namespace jh::pod {
    /**
     * @brief POD-compatible alternative to `std::pair`, optimized for raw memory layout and performance.
     *
     * @tparam T1 The type of the first element. Must satisfy `jh::pod::pod_like`.
     * @tparam T2 The type of the second element. Must satisfy `jh::pod::pod_like`.
     *
     * This struct provides a minimal and strict replacement for `std::pair` that:
     * - Guarantees POD layout and properties (`trivial`, `standard layout`, etc.);
     * - Enables safe use in `pod_stack<T>` and similar raw memory containers;
     * - Can be safely copied via `memcpy`, mapped from binary blobs, or stored in raw buffers;
     * - Supports equality comparison via `operator==`.
     *
     * @note
     * This is recommended as a drop-in replacement for `std::pair` in performance-critical code.
     * Prefer using `struct` with named members if the pair is semantically meaningful beyond generic use.
     */
    template<pod_like T1, pod_like T2>
    struct pair final{
        T1 first;
        T2 second;

        using first_type [[maybe_unused]] = T1;
        using second_type [[maybe_unused]] = T2;

        constexpr bool operator==(const pair &) const = default;
    };
}
