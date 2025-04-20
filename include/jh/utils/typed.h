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
 * @file typed.h (utils)
 * @author JeongHan-Bae <mastropseudo@gmail.com>
 * @brief Lightweight monostate type and related traits.
 *
 * Provides `jh::typed::monostate`, a trivial type used in place of `std::monostate`
 * without depending on `std::variant`. Includes basic type traits and concept support.
 */


#pragma once
#include <type_traits>

namespace jh::typed {
    struct monostate final {
        constexpr bool operator==(monostate) const noexcept { return true; }
        constexpr bool operator!=(monostate) const noexcept { return false; }
    };

    static_assert(std::is_trivially_copyable_v<monostate>);
    static_assert(std::is_trivially_constructible_v<monostate>);
    static_assert(std::is_standard_layout_v<monostate>);

    // Type trait for detection
    template<typename T> // NOLINT for general T
    struct is_monostate : std::false_type {
    };

    template<>
    struct is_monostate<monostate> : std::true_type {
    };

    // Concept for use in templates
    template<typename T>
    concept monostate_t = is_monostate<T>::value;
}
