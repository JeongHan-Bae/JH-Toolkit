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
 * @file optional.h (pods)
 * @author JeongHan-Bae <mastropseudo@gmail.com>
 * @brief POD-safe optional<T> replacement with raw layout and zero overhead.
 *
 * A strict, trivially copyable alternative to `std::optional<T>`,
 * designed for raw memory containers, `pod_stack`, and SIMD-optimized use.
 *
 * - POD-only: `trivially_copyable`, `standard_layout`
 * - Requires `.store()` for assignment
 * - `.has()` / `.empty()` must be used before accessing `.ref()`
 *
 * ABI: `sizeof(optional<T>) == sizeof(T) + 1` (may pad to alignof(T))
 */


#pragma once

#include "pod_like.h"
#include <new>
#include <cstring>
#include <memory>

namespace jh::pod {
    /**
     * @brief POD-compatible optional value wrapper.
     *
     * Stores a raw value of type `T` with a boolean presence flag.
     * Does not call constructors; memory is copied via `memcpy()`.
     *
     * @tparam T Any `pod_like` type.
     */
    template<pod_like T>
    struct alignas(alignof(T)) optional final {
        std::byte storage[sizeof(T)];
        bool has_value;

        /// @brief Default constructor (empty state). Value must be assigned manually.
        constexpr optional() noexcept = default;

        /**
         * @brief Stores a value via `memcpy`.
         * @param value Source value to copy into storage.
         */
        void store(const T &value) noexcept {
            std::memcpy(storage, std::addressof(value), sizeof(T));
            has_value = true;
        }

        /// @brief Clears the value (marks as empty).
        void clear() noexcept {
            has_value = false;
        }

        /**
         * @brief Unsafe pointer access to stored value.
         * @return `T*` pointer to raw value (must check `.has()` before use).
         */
        T *get() noexcept {
            return std::launder(reinterpret_cast<T *>(&storage));
        }

        /**
         * @brief Unsafe const pointer access.
         * @return `const T*` pointer to raw value.
         */
        [[nodiscard]] const T *get() const noexcept {
            return std::launder(reinterpret_cast<const T *>(&storage));
        }

        /// @brief Whether a value is present.
        [[nodiscard]] bool has() const noexcept { return has_value; }

        /// @brief Whether the optional is empty.
        [[nodiscard]] bool empty() const noexcept { return !has_value; }

        /**
         * @brief Returns a reference to the stored value.
         * @return Reference to `T`. Undefined if `.has()` is false.
         */
        [[nodiscard]] T &ref() noexcept { return *get(); }

        /**
         * @brief Returns a const reference to the stored value.
         */
        [[nodiscard]] const T &ref() const noexcept { return *get(); }
    };

    /**
     * @brief Constructs an `optional<T>` with a value.
     *
     * Creates an empty optional, then stores a value via `optional<T>::store(...)`.
     *
     * @param value Value to copy into optional.
     * @return A filled `optional<T>` with `.has() == true`.
     */
    template<pod_like T>
    [[nodiscard]] constexpr optional<T> make_optional(const T &value) noexcept {
        optional<T> o;
        o.store(value);
        return o;
    }
} // namespace jh::pod
