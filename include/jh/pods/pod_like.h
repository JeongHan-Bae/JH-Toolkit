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
 * @file pod_like.h (pods)
 * @author JeongHan-Bae <mastropseudo@gmail.com>
 * @brief Implementation of the `pod_like` concept.
 */


#pragma once

#include <type_traits>

namespace jh::pod {
    /**
     * @brief Concept for verifying whether a type qualifies as a "POD-like" structure.
     *
     * A POD-like type must:
     * - Be trivially copyable (no custom copy constructor/operator)
     * - Be trivially constructible (no user-defined constructor)
     * - Be trivially destructible (no destructor side effects)
     * - Have a standard layout (well-defined memory layout)
     *
     * Used to restrict containers like `pod_stack<T>` to the safest and fastest types.
     */
    template<typename T>
    concept pod_like = std::is_trivially_copyable_v<T> &&
                       std::is_trivially_constructible_v<T> &&
                       std::is_trivially_destructible_v<T> &&
                       std::is_standard_layout_v<T>;
}
