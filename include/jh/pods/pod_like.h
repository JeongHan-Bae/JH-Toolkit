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
 * @brief Definition of the <code>pod_like</code> concept
 * and its cv-free constraint variant (<code>cv_free_pod_like</code>).
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

    /**
     * @brief Concept for POD-like types that are free of <code>const</code> or <code>volatile</code> qualification.
     *
     * <h4>Definition</h4>
     * Equivalent to <code>pod_like&lt;T&gt;</code>, but adds the requirement that
     * <code>T</code> itself must not be <code>const</code>-qualified nor
     * <code>volatile</code>-qualified.
     *
     * <h4>Motivation</h4>
     * In certain templates—such as <code>pod::pair&lt;T1, T2&gt;</code>—using
     * <code>const</code>-qualified inner types (e.g., <code>pair&lt;const int, int&gt;</code>)
     * would violate standard layout or trivially-copyable constraints, rendering the
     * resulting type non-POD.
     *
     * This concept ensures that only unqualified POD-like types are used in such contexts,
     * while still allowing <code>const pod::pair&lt;...&gt;</code> to remain valid.
     *
     * <h4>Example</h4>
     * @code
     * static_assert(cv_free_pod_like&lt;int&gt;);
     * static_assert(!cv_free_pod_like&lt;const int&gt;);
     * static_assert(cv_free_pod_like&lt;pod::pair&lt;int, int&gt;&gt;);
     * @endcode
     */
    template<typename T>
    concept cv_free_pod_like =
    pod_like<T> &&
    !std::is_const_v<T> &&
    !std::is_volatile_v<T>;
}
