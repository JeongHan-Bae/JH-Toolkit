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
 * @file tools.h (pods)
 * @brief POD helper utilities based on macros and placeholder types.
 *
 * Unlike core POD containers (e.g., <code>array</code>, <code>pair</code>, <code>optional</code>),
 * this header provides <b>tools</b> for enforcing and constructing POD types, mostly
 * via macro magic or placeholder types.
 *
 * <h3>Contents:</h3>
 * <ul>
 *   <li><code>JH_POD_STRUCT</code>: Macro to declare a POD struct with <code>operator==</code>
 *       and automatic <code>pod_like</code> validation.</li>
 *   <li><code>JH_ASSERT_POD_LIKE</code>: Compile-time assertion that a user-defined type
 *       satisfies <code>pod_like</code> constraints.</li>
 *   <li><code>jh::pod::tuple</code>: A transitional POD-compatible fixed-size tuple,
 *       using <code>typed::monostate</code> as placeholder for unused slots.</li>
 * </ul>
 *
 * <h3>Design Notes:</h3>
 * <ul>
 *   <li>These are <b>helper facilities</b> — not general-purpose containers.</li>
 *   <li><code>tuple</code> is marked <code>[[deprecated]]</code> and provided mainly for
 *       migration or bridging with generic algorithms.</li>
 *   <li>Recommended practice: prefer <b>explicit POD structs</b> with named fields.</li>
 * </ul>
 */

#pragma once

#include "pod_like.h"
#include "jh/utils/typed.h"

#ifdef JH_POD_STRUCT
// User manually defined the macro before us — this is a hard conflict
static_assert(false,
    "Conflict: JH_POD_STRUCT macro already defined. "
    "Please remove your macro or rename it. "
    "This macro is reserved for jh::pod_struct usage and includes essential validation.");
#else

/**
 * @brief Declares a strict POD struct with <code>operator==</code> and enforces POD constraints at compile time.
 *
 * @param NAME The name of the struct to define.
 * @param ...  The member declarations of the struct.
 *
 * <h3>Example:</h3>
 * @code
 * JH_POD_STRUCT(MyPair,
 *     int x;
 *     int y;
 * );
 * @endcode
 *
 * <h3>Guarantees:</h3>
 * <ul>
 *   <li>The struct is treated as a <code>pod_like</code> type.</li>
 *   <li><code>operator==</code> is auto-generated (<code>= default</code>).</li>
 *   <li>If the type violates POD requirements (e.g., contains <code>std::string</code> or <code>unique_ptr</code>),
 *       it triggers a compile-time error.</li>
 * </ul>
 *
 * @note Highly recommended over manual struct declarations when using
 *       <code>jh::pod_stack</code> or other POD-only containers.
 */
#define JH_POD_STRUCT(NAME, ...)                                  \
    struct NAME {                                                 \
        __VA_ARGS__                                               \
        constexpr bool operator==(const NAME&) const = default;   \
    };                                                            \
    static_assert(::jh::pod::pod_like<NAME>,                      \
                  #NAME " must be trivially copyable, "           \
                  "standard layout, and contain only POD-compatible members.")

#endif

/**
 * @brief Compile-time assertion to ensure a type is POD-like, according to <code>jh::pod::pod_like</code>.
 *
 * Use this macro if you're writing your own struct (not using <code>JH_POD_STRUCT</code>)
 * but still want to ensure compatibility with POD-only containers (like <code>pod_stack</code>).
 *
 * <h3>Example:</h3>
 * @code
 * struct MyManualPod {
 *     int x;
 *     float y;
 * };
 *
 * JH_ASSERT_POD_LIKE(MyManualPod);
 * @endcode
 *
 * <h3>Guarantees:</h3>
 * This will emit a compile-time error if the type is not:
 * <ul>
 *   <li>trivially copyable</li>
 *   <li>trivially constructible/destructible</li>
 *   <li>standard layout</li>
 * </ul>
 */
#define JH_ASSERT_POD_LIKE(TYPE)             \
    static_assert(::jh::pod::pod_like<TYPE>, \
                  #TYPE " must satisfy jh::pod_like: trivial, standard layout, POD-compatible type.")
