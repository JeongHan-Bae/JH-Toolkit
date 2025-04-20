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
 * @file pool.h
 * @author JeongHan-Bae <mastropseudo@gmail.com>
 *
 * @brief A weak_ptr-based object pool with automatic template deduction.
 *
 * @details
 * `jh::pool<T>` is a **specialized derived struct of `sim_pool<T, Hash, Eq>`** that provides automatic
 * template deduction for compatible types. If `T` implements a `hash()` function and `operator==`,
 * `jh::pool<T>` will infer the appropriate hash and equality functions, eliminating the need for
 * manual template specialization. This enables **content-based pooling** where semantically identical
 * objects are automatically managed as a single instance.
 *
* ## Key Features
 * - **Automatic Hash and Equality Deduction**:
 *   - Uses `T::hash()` for hashing instead of `std::hash<T>` to improve efficiency.
 *   - Uses `operator==` for content-based equality comparison.
 * - **Efficient Object Pooling**:
 *   - Ensures semantically identical objects are stored only once.
 *   - Automatically removes expired weak pointers to maintain efficiency.
 * - **Seamless Integration with `sim_pool<T>`**:
 *   - Automatically selects `weak_ptr_hash<T>` and `weak_ptr_eq<T>`, eliminating the need
 *     to manually specify hashing and equality policies.
 * - **Supports Argument-Based Construction**:
 *   - `T` **must** be constructible with arguments passed to `acquire()`.
 *   - Enables the creation and pooling of `T` instances via `pool<T>::acquire(Args&&...)`.
 *
 * @version 1.2.x
 * @date 2025
 */

#pragma once

#include <concepts>         // NOLINT for concepts
#include <cstdint>          // for std::uint64_t
#include <utility>          // for std::ignore
#include "sim_pool.h"

namespace jh {
    /**
     * @brief Concept to check if a type `T` provides a `hash()` function.
     *
     * @details
     * The `has_hash` concept ensures that a type `T` implements a method:
     * ```
     * [[nodiscard]] std::uint64_t hash() const noexcept;
     * ```
     * This allows objects to compute their hash **only once** if being immutable, improving efficiency
     * compared to rehashing mutable objects.
     *
     * @tparam T The type to be checked.
     */
    template<typename T>
    concept has_hash = requires(T obj)
    {
        { obj.hash() } -> std::convertible_to<std::uint64_t>;
    };

    /**
     * @brief Concept to check if a type `T` provides an equality comparison operator.
     *
     * @details
     * The `has_equal` concept ensures that a type `T` implements:
     * ```
     * bool operator==(const T&) const;
     * ```
     * This is required for checking object equivalence inside `sim_pool`.
     *
     * @tparam T The type to be checked.
     */
    template<typename T>
    concept has_equal = requires(T obj)
    {
        { obj == obj } -> std::convertible_to<bool>;
    };

    /**
     * @brief Custom hash function for `std::weak_ptr<T>`.
     *
     * @details
     * - If the `weak_ptr` is **expired**, it hashes to `0`.
     * - If the `weak_ptr` is **valid**, it locks and uses `T::hash()`.
     *
     * @tparam T The type of the managed object.
     */
    template<typename T>
        requires has_hash<T>
    struct weak_ptr_hash {
        std::size_t operator()(const std::weak_ptr<T> &ptr) const noexcept {
            if (const auto sp = ptr.lock()) {
                // Pre-touch hash once to ensure consistent hash during insert()
                std::ignore = sp->hash(); // ensures atomic hash() before insert
                return sp->hash();
            }
            return 0;
        }
    };

    /**
     * @brief Custom equality function for `std::weak_ptr<T>`.
     *
     * @details
     * - If either `weak_ptr` is expired, they are **not equal**.
     * - If both are valid, it locks and compares using `operator==`.
     *
     * @tparam T The type of the managed object.
     */
    template<typename T>
        requires has_equal<T>
    struct weak_ptr_eq {
        bool operator()(const std::weak_ptr<T> &lhs, const std::weak_ptr<T> &rhs) const noexcept {
            const auto sp1 = lhs.lock();
            const auto sp2 = rhs.lock();
            if (!sp1 || !sp2) return false;
            return *sp1 == *sp2;
        }
    };

    /**
     * @brief A specialized `sim_pool<T>` that automatically derives hash and equality functions.
     *
     * @details
     * This specialization of `sim_pool<T>` automatically uses:
     * - `weak_ptr_hash<T>` as the hash function.
     * - `weak_ptr_eq<T>` as the equality function.
     * .
     * This eliminates the need for manually specifying the `Hash` and `Eq` template parameters.
     * Requirements for `T`:
     * - Must provide a `hash()` function returning `std::uint64_t`.
     * - Must provide `operator==` for content-based comparison.
     *
     * @tparam T The type of object being pooled.
     */
    template<typename T>
        requires (has_hash<T> && has_equal<T>) // Define jh::pool<T> only for types with hash() and operator==
    struct pool final : sim_pool<T, weak_ptr_hash<T>, weak_ptr_eq<T> > {
        using sim_pool<T, weak_ptr_hash<T>, weak_ptr_eq<T> >::sim_pool; ///< Inherit constructors from `sim_pool`
    };
} // namespace jh
