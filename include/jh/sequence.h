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
 * @file sequence.h
 * @author JeongHan-Bae <mastropseudo@gmail.com>
 * @brief A lightweight concept for defining immutable sequences in C++20.
 *
 * @details
 * This module introduces `jh::sequence`, a C++20 concept that ensures a type
 * supports **immutable iteration** via `begin()` and `end()`. It provides a
 * uniform way to identify sequence-like containers at compile time.
 *
 * ## Key Features
 * - Defines `jh::sequence` for immutable iteration.
 * - Provides `sequence_value_type<T>` to extract element type.
 * - Offers `is_sequence<T>` for compile-time checking.
 *
 * @version 1.0.x
 * @date 2025
 */

#pragma once

#include <iterator>

namespace jh {
    /**
     * @brief Concept to check if a type is a sequence (immutable iterable).
     * @tparam T The type to check.
     */
    template<typename T>
    concept sequence = requires(const T t)
    {
        // ✅ Force to use `const T`
        { t.begin() } -> std::input_iterator;
        { t.end() } -> std::input_iterator;
    };

    /**
     * @brief Internal helper to extract value_type safely.
     * - Removes `const`, `volatile`, and reference qualifiers before accessing `begin()`.
     */
    template<typename T>
    struct sequence_value_type_impl {
        using raw_type = std::remove_cvref_t<T>;
        using iterator_type = decltype(std::declval<const raw_type>().begin());
        using type = typename std::iterator_traits<iterator_type>::value_type;
    };

    /**
     * @brief Utility to extract value_type from a sequence.
     * @tparam T The sequence type.
     */
    template<typename T>
    using sequence_value_type = typename sequence_value_type_impl<T>::type;

    /**
     * @brief Compile-time boolean to check if a type is a sequence.
     * @tparam T The type to check.
     */
    template<typename T>
    constexpr bool is_sequence = sequence<T>;
} // namespace jh
