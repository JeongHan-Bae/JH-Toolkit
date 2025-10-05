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
 * @file pod_like.h (pods)
 * @brief Definition of the <code>pod_like</code> concept.
 */

#pragma once

#include <type_traits>

namespace jh::pod {
    /**
     * @brief Concept for types that are safe to treat as plain old data (POD).
     *
     * Requirements:
     * <ul>
     *   <li>Trivially copyable</li>
     *   <li>Trivially constructible</li>
     *   <li>Trivially destructible</li>
     *   <li>Standard layout</li>
     * </ul>
     *
     * Used as a constraint in all POD containers (<code>pod::array</code>, <code>pod::optional</code>, etc.).
     */
    template<typename T>
    concept pod_like = std::is_trivially_copyable_v<T> &&
                       std::is_trivially_constructible_v<T> &&
                       std::is_trivially_destructible_v<T> &&
                       std::is_standard_layout_v<T>;
}
