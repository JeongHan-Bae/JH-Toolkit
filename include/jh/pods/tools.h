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


namespace jh::pod {
    /**
     * @brief POD-compatible fixed-size tuple type, suitable for raw memory and algorithmic bridging.
     *
     * @tparam T1 First element type. Must satisfy <code>cv_free_pod_like&lt;T1&gt;</code>.
     *             This parameter is <b>required</b> and cannot be <code>typed::monostate</code>.
     * @tparam T2 Second element type. Must satisfy <code>cv_free_pod_like&lt;T2&gt;</code>.
     *             This parameter is <b>required</b> and cannot be <code>typed::monostate</code>.
     * @tparam ... Additional optional element type placeholders (<code>T3..T8</code>, up to 8 total).
     *             Each must satisfy <code>cv_free_pod_like</code> or default to
     *             <code>jh::typed::monostate</code>. Using <code>cv</code>-qualified types will
     *             break POD layout.
     *
     * @warning Do <b>not</b> use <code>std::monostate</code> for any parameter.
     *          The tuple system only recognizes <code>jh::typed::monostate</code> for empty slots.
     *          Using <code>std::monostate</code> would bypass internal validation checks,
     *          which serves no purpose and provides no semantic meaning.
     *
     * @details
     * This struct provides a tuple-like container with fixed fields <code>_0</code> to <code>_7</code>,
     * while guaranteeing POD layout and layout stability.
     *
     * Conceptually, this type serves as a <b>migration bridge</b> from <code>std::tuple</code>
     * (a meta-level type composition system) to <b>pure POD data representations</b>.
     * It allows existing tuple-based code to transition into a POD-compatible form
     * when performance, reflection, or serialization require strictly plain data.
     *
     * <h4>Design Intent</h4>
     * <ul>
     *   <li><code>std::tuple</code> is part of a <b>complex type system</b>, not a data model.</li>
     *   <li><code>jh::pod::tuple</code> is a <b>temporary bridge</b> for migrating
     *       tuple-heavy systems toward <b>plain, POD-based structures</b>.</li>
     *   <li>If your data types use <code>std::tuple</code> internally, this bridge
     *       can help isolate and rewrite them as explicit structs.</li>
     *   <li>The final goal is to eliminate all tuple dependencies in pure data layers.</li>
     * </ul>
     *
     * <h4>Intended Use Cases</h4>
     * <ul>
     *   <li>Gradual migration from <code>std::tuple&lt;Ts...&gt;</code> to plain POD structs.</li>
     *   <li>Bridging tuple-based APIs into raw-memory containers (<code>pod_stack</code>, mapped buffers, etc.).</li>
     *   <li>Temporary compatibility in data pipelines that must remain POD-safe.</li>
     * </ul>
     *
     * @note
     * <ul>
     *   <li>This type is <b>layout-compatible</b> but not feature-compatible with <code>std::tuple</code>.</li>
     *   <li>It supports only index-based access (<code>get&lt;N&gt;()</code>) and equality comparison.</li>
     *   <li>It does <b>not</b> support variadic unpacking, structured bindings, or heterogeneous type inference.</li>
     * </ul>
     *
     * @warning
     * <b><span style="color:red">unstable</span></b>:
     * The existence of <code>jh::pod::tuple</code> is purely transitional.
     * You <b>should not</b> design new systems around it.
     * Once migration from <code>std::tuple</code> is complete,
     * define explicit POD structs with named members to preserve type clarity and safety.
     */
    template<cv_free_pod_like T1, cv_free_pod_like T2,
        typename T3 = typed::monostate,
        typename T4 = typed::monostate,
        typename T5 = typed::monostate,
        typename T6 = typed::monostate,
        typename T7 = typed::monostate,
        typename T8 = typed::monostate>
        requires (!typed::monostate_t<T1> && !typed::monostate_t<T2>)
                 && (typed::monostate_t<T3> || cv_free_pod_like<T3>)
                 && (typed::monostate_t<T4> || cv_free_pod_like<T4>)
                 && (typed::monostate_t<T5> || cv_free_pod_like<T5>)
                 && (typed::monostate_t<T6> || cv_free_pod_like<T6>)
                 && (typed::monostate_t<T7> || cv_free_pod_like<T7>)
                 && (typed::monostate_t<T8> || cv_free_pod_like<T8>)
    struct [[deprecated("pod::tuple is an unstable transitional type. Prefer explicit POD structs.")]] tuple {
        T1 _0;
        T2 _1;
        T3 _2; // no-op for empty
        T4 _3;
        T5 _4;
        T6 _5;
        T7 _6;
        T8 _7;

        template<std::uint8_t V>
            requires (V == 0 && !typed::monostate_t<T1>) // no-op for empty
        T1 &get() { return _0; }

        template<std::uint8_t V>
            requires (V == 1 && !typed::monostate_t<T1>) // no-op for empty
        T2 &get() { return _1; }

        template<std::uint8_t V>
            requires (V == 2 && !typed::monostate_t<T3>) // no-op for empty
        T3 &get() { return _2; }

        template<std::uint8_t V>
            requires (V == 3 && !typed::monostate_t<T4>) // no-op for empty
        T4 &get() { return _3; }

        template<std::uint8_t V>
            requires (V == 4 && !typed::monostate_t<T5>) // no-op for empty
        T5 &get() { return _4; }

        template<std::uint8_t V>
            requires (V == 5 && !typed::monostate_t<T6>) // no-op for empty
        T6 &get() { return _5; }

        template<std::uint8_t V>
            requires (V == 6 && !typed::monostate_t<T7>) // no-op for empty
        T7 &get() { return _6; }

        template<std::uint8_t V>
            requires (V == 7 && !typed::monostate_t<T8>) // no-op for empty
        T8 &get() { return _7; }

        constexpr bool operator==(const tuple &) const = default;
    };
}
