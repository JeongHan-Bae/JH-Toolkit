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
 * @file tools.h (pods)
 * @author JeongHan-Bae <mastropseudo@gmail.com>
 * @brief Implementation of all POD-like tools.
 */

#pragma once

#include "pod_like.h"
#include "../utils/typed.h"

#ifdef JH_POD_STRUCT
// User manually defined the macro before us — this is a hard conflict
static_assert(false,
    "Conflict: JH_POD_STRUCT macro already defined. "
    "Please remove your macro or rename it. "
    "This macro is reserved for jh::pod_struct usage and includes essential validation.");
#else

/**
 * @brief Declares a strict POD struct with `operator==` and enforces POD constraints at compile time.
 *
 * @param NAME   The name of the struct to define.
 * @param ...    The member declarations of the struct.
 *
 * @example
 * ```
 * JH_POD_STRUCT(MyPair,
 *     int x;
 *     int y;
 * );
 * ```
 *
 * This macro guarantees:
 * - The struct is treated as a POD-like type.
 * - `operator==` is auto-generated (`= default`)
 * - If the type violates POD requirements (e.g., contains `std::string` or `unique_ptr`), it triggers a compile-time error.
 *
 * @note Highly recommended over manual struct declarations when using `jh::pod_stack` or other POD-only containers.
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
 * @brief Compile-time assertion to ensure a type is POD-like, according to `jh::pod::pod_like`.
 *
 * Use this macro if you're writing your own struct (not using `JH_POD_STRUCT`)
 * but still want to ensure compatibility with POD-only containers (like `pod_stack`).
 *
 * @example
 * ```
 * struct MyManualPod {
 *     int x;
 *     float y;
 * };
 *
 * JH_ASSERT_POD_LIKE(MyManualPod);
 * ```
 *
 * This will emit a compile-time error if the type is not:
 * - trivially copyable
 * - trivially constructible/destructible
 * - standard layout
 */
#define JH_ASSERT_POD_LIKE(TYPE)             \
    static_assert(::jh::pod::pod_like<TYPE>, \
                  #TYPE " must satisfy jh::pod_like: trivial, standard layout, POD-compatible type.")


namespace jh::pod {
    /**
     * @brief POD-compatible fixed-size tuple type, suitable for raw memory and algorithmic bridging.
     *
     * @tparam T1-T8 Up to 8 type parameters.
     *         All types must satisfy `jh::pod::pod_like`. Unused parameters must default to `typed::monostate`.
     *
     * This struct provides a tuple-like container with fixed fields `_0` to `_7`,
     * while guaranteeing POD layout and layout stability. It is intended for:
     * - Migration from `std::tuple<Ts...>` in performance-sensitive code;
     * - Use in raw memory systems (`pod_stack`, mapped buffers, etc.);
     * - Scenarios where dynamic field count is not required and POD layout is mandatory.
     *
     * @note
     * This type is API-compatible with `std::tuple` in terms of index-based access and equality,
     * but does not support variadic unpacking, structured bindings, or dynamic field count.
     *
     * @warning @unstable:
     *          Use of this type is **discouraged** in general-purpose code. Prefer defining
     *          explicit `struct` types with named members when possible. This type exists
     *          primarily to aid in `std::tuple` migration or interfacing with generic algorithms.
     */
    template<pod_like T1, pod_like T2,
        typename T3 = typed::monostate,
        typename T4 = typed::monostate,
        typename T5 = typed::monostate,
        typename T6 = typed::monostate,
        typename T7 = typed::monostate,
        typename T8 = typed::monostate>
        requires (!typed::monostate_t<T1> && !typed::monostate_t<T2>)
                 && (typed::monostate_t<T3> || pod_like<T3>)
                 && (typed::monostate_t<T4> || pod_like<T4>)
                 && (typed::monostate_t<T5> || pod_like<T5>)
                 && (typed::monostate_t<T6> || pod_like<T6>)
                 && (typed::monostate_t<T7> || pod_like<T7>)
                 && (typed::monostate_t<T8> || pod_like<T8>)
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
