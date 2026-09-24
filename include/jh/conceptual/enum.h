/**
 * @copyright
 * Copyright 2025 JeongHan-Bae &lt;mastropseudo\@gmail.com&gt;
 * <br>
 * Licensed under the Apache License, Version 2.0 (the "License"); <br>
 * you may not use this file except in compliance with the License.<br>
 * You may obtain a copy of the License at<br>
 * <br>
 *     http://www.apache.org/licenses/LICENSE-2.0<br>
 * <br>
 * Unless required by applicable law or agreed to in writing, software<br>
 * distributed under the License is distributed on an "AS IS" BASIS,<br>
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.<br>
 * See the License for the specific language governing permissions and<br>
 * limitations under the License.<br>
 */
/**
 * @file enum.h
 * @brief Shared enum concepts and underlying-type alias.
 */

#pragma once

#include <type_traits>

namespace jh::concepts {
    /** @brief Accepts scoped and unscoped enumeration types. */
    template<typename T>
    concept enum_type = std::is_enum_v<T>;

    /** @brief Accepts only scoped enumerations declared with <code>enum class</code>. */
    template<typename T>
    concept scoped_enum = enum_type<T> &&
                          !std::is_convertible_v<T, std::underlying_type_t<T>>;

    /** @brief Enum-class type information shared by the conceptual layer. */
    template<scoped_enum E>
    struct enum_class final {
        using type = E;
        using underlying = std::underlying_type_t<E>;
    };

    /** @brief Underlying integer type for an enumeration. */
    template<scoped_enum E>
    using enum_underlying_t = typename enum_class<E>::underlying;
}
