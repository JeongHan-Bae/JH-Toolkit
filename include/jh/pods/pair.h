/**
 * \verbatim
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
 * \endverbatim
 */
/**
 * @file pair.h (pods)
 * @brief Implementation of <code>jh::pod::pair&lt;T1, T2&gt;</code>.
 */

#pragma once

#include "pod_like.h"

namespace jh::pod {
    /**
     * @brief POD-compatible aggregate of two values, equivalent in layout to a plain struct.
     *
     * @tparam T1 First element type. Must satisfy <code>pod_like</code>.
     * @tparam T2 Second element type. Must satisfy <code>pod_like</code>.
     *
     * This type provides the simplest form of a pair:
     * <ul>
     *   <li>Two inline members: <code>first</code> and <code>second</code></li>
     *   <li>Strictly POD — trivial, standard layout, memcpy-safe</li>
     *   <li>Equality comparison via <code>operator==</code></li>
     * </ul>
     *
     * <h4>Notes</h4>
     * <ul>
     *   <li>Intended as the POD-only building block for pair-like objects</li>
     *   <li>Optimized for raw containers (<code>pod::array</code>, <code>pod::tuple</code>, etc.)</li>
     *   <li>For generic pair creation, prefer <code>jh::utils::make_pair</code>, which will
     *       automatically select <code>pod::pair</code> when both arguments are POD</li>
     */
    template<pod_like T1, pod_like T2>
    struct pair final{
        T1 first;   ///< First element.
        T2 second;  ///< Second element.

        using first_type = T1;   ///< Alias for first element type.
        using second_type = T2;  ///< Alias for second element type.

        /// @brief Member-wise equality comparison.
        constexpr bool operator==(const pair &) const = default;
    };
}
