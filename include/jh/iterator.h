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
 * @file iterator.h
 * @author JeongHan-Bae <mastropseudo@gmail.com>
 * @brief Forward declaration and concept definitions for iterators.
 *
 * @details
 * This header file provides:
 * - A **forward declaration** of the `iterator<>` template for use in `generator<>` and other containers.
 * - **Concepts for iterator validation**, compatible with both `std::` iterators and `jh::` iterators.
 *
 * ## Key Features
 * - Defines **concepts** (`input_iterator`, `output_iterator`, etc.) to validate iterators.
 * - Supports **both standard and custom iterators**.
 * - Prevents unnecessary inclusion of full iterator implementations.
 * - Ensures **compatibility with STL-style iterator traits**.
 *
 * @version 1.3.x
 * @date 2025
 */

#pragma once

#include <iterator>

namespace jh {
    /**
     * @brief Forward declaration of the `iterator` class template.
     * @details
     * This allows generator and other container types to reference `iterator<>`
     * without including the full definition.
     * - The actual implementation of `iterator<>` is specialized elsewhere for different types.
     *
     * @tparam X The type for which an iterator will be defined.
     */
    template<typename X>
    struct iterator;

    /**
     * @brief Checks if a type `T` has an associated `value_type`.
     */
    template<typename T, typename = void>
    struct has_value_type : std::false_type {
    };

    template<typename T>
    struct has_value_type<T, std::void_t<typename T::value_type> > : std::true_type {
    };

    template<typename T>
    [[maybe_unused]] inline constexpr bool has_value_type_v = has_value_type<T>::value;

    /**
     * @brief Concept to check if a type `I` is a valid input iterator.
     * @details
     * Works for both `std` and `jh` iterators.
     * - Must support **dereferencing (`*iter`)**, **increment (`++iter`, `iter++`)**,
     *   and **comparison (`==`, `!=`)**.
     *
     * @tparam I The type to check.
     */
    template<typename I>
    concept input_iterator = requires(I it)
    {
        typename std::iterator_traits<I>::value_type;
        { *it } -> std::convertible_to<typename std::iterator_traits<I>::value_type>;
        { ++it } -> std::same_as<I &>;
        { it++ } -> std::same_as<I>;
        { it == it } -> std::convertible_to<bool>;
        { it != it } -> std::convertible_to<bool>;
    };

    /**
     * @brief Concept to check if a type `I` is a valid output iterator.
     * @tparam I The iterator type.
     * @tparam T The type of values that can be assigned to `*iter`.
     */
    template<typename I, typename T>
    concept output_iterator = requires(I it, T value)
    {
        *it = value;
        ++it;
        it++;
    };

    /**
     * @brief Concept to check if a type `I` is a valid forward iterator.
     * @tparam I The iterator type.
     */
    template<typename I>
    concept forward_iterator = input_iterator<I> && requires(I it)
    {
        { it++ } -> std::same_as<I>;
        { *it } -> std::same_as<typename std::iterator_traits<I>::reference>;
    };

    /**
     * @brief Concept to check if a type `I` is a valid bidirectional iterator.
     * @tparam I The iterator type.
     */
    template<typename I>
    concept bidirectional_iterator = forward_iterator<I> && requires(I it)
    {
        --it;
        it--;
    };

    /**
     * @brief Concept to check if a type `I` is a valid random-access iterator.
     * @tparam I The iterator type.
     */
    template<typename I>
    concept random_access_iterator = bidirectional_iterator<I> && requires(I it, std::ptrdiff_t n)
    {
        { it + n } -> std::same_as<I>;
        { it - n } -> std::same_as<I>;
        { it[n] } -> std::same_as<typename std::iterator_traits<I>::reference>;
        { it < it } -> std::convertible_to<bool>;
        { it > it } -> std::convertible_to<bool>;
        { it <= it } -> std::convertible_to<bool>;
        { it >= it } -> std::convertible_to<bool>;
    };


    /**
     * @brief Concept to check if a type `T` is a valid iterator.
     * @details
     * This concept works for **both `std::` and `jh::` iterators**.
     * - Allows checking if a given type behaves like an iterator.
     *
     * @tparam T The type to check.
     */
    template<typename T>
    concept is_iterator = requires(T it)
    {
        typename std::iterator_traits<T>::value_type;
        { ++it } -> std::same_as<T &>;
        { it++ } -> std::same_as<T>;
        { *it };
    };

    namespace detail {
        // Primary template: default to empty
        template<typename T, typename = void, typename = void>
        struct iterator_resolver;

        // Case 1: has jh::iterator<T>::type
        template<typename T>
        struct iterator_resolver<
            T,
            std::void_t<typename iterator<T>::type>, // if jh::iterator<T>::type exists
            std::enable_if_t<true>
        > {
            using type = typename iterator<T>::type;
        };

        // Case 2: fallback - no jh::iterator<T>::type, but has .begin()
        template<typename T>
        struct iterator_resolver<
            T,
            std::void_t<decltype(std::declval<T &>().begin())>,
            std::enable_if_t<!requires { typename iterator<T>::type; }>
        > {
            using type = decltype(std::declval<T &>().begin());
        };

        // Fallback 3: pointer or array case
        template<typename T>
            requires std::is_pointer_v<T>
        struct iterator_resolver<T, void, void> {
            using type = T;
        };
    }

    /**
     * @brief Extracts the iterator type from a container, supporting both STL and jh containers.
     *
     * @tparam Container The container type
     * @note This works even if `jh::iterator<Container>` is not specialized.
     */

    /**
     * @brief Extracts the iterator type of some container.
     *
     * `iterator_t<Container>` is a type alias used to deduce the iterator type of a given container `Container`.
     * It is designed for **custom containers** that are compatible with `jh::iterator<>`.
     *
     * @tparam Container The container type whose iterator type needs to be deduced.
     *
     * @details
     * - The iterator type is determined via `jh::iterator<Container>::type`.
     * - `iterator_t<Container>` may be `jh::iterator<Container>`, such as for `jh::generator`.
     * - For certain linear structures, `iterator_t<Container>` may resolve to `T*`, for example,
     *   `jh::runtime_arr<T>` (planned for release in v1.3+) uses `T*` as its iterator type.
     * .
     * ## Usage Scenarios:
     * - **Generic programming**: Allows obtaining the iterator type without explicitly specifying `Container::iterator`.
     *   - For continuous linear structures, `T*` can be used as `jh::iterator<Container<T>>::type`.
     * - **Concept-based programming**: Can be used in `concept` constraints for **SFINAE** and **template metaprogramming**.
     * - Supports **STL-style containers** and **custom containers**, provided they are adapted to `jh::iterator<>`.
     */
    template<typename Container>
    using iterator_t = typename detail::iterator_resolver<Container>::type;
} // namespace jh
