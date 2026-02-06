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
 * <br>
 * Full license: <a href="https://github.com/JeongHan-Bae/JH-Toolkit?tab=Apache-2.0-1-ov-file#readme">GitHub</a>
 */
/**
 * @file monostate.h
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 * @brief Trivial placeholder type <code>monostate</code> and its traits.
 *
 * Provides <code>jh::typed::monostate</code>, a strict POD type equivalent in
 * spirit to <code>std::monostate</code>, but lightweight and header-only
 * (no dependency on <code>&lt;variant&gt;</code> or other STL headers).
 *
 * <h3>Components:</h3>
 * <ul>
 *   <li><code>monostate</code> &mdash; trivial empty type with equality operators.</li>
 *   <li><code>is_monostate&lt;T&gt;</code> &mdash; type trait for detection.</li>
 *   <li><code>monostate_t&lt;T&gt;</code> &mdash; concept form for SFINAE/constraints.</li>
 * </ul>
 *
 * <h3>Design Goals:</h3>
 * <ul>
 *   <li>POD compliance: trivially copyable, trivially constructible, standard layout.</li>
 *   <li>Minimal dependency footprint, no heavy STL headers required.</li>
 *   <li>Use as a safe placeholder in POD containers (e.g. unused <code>tuple</code> slots).</li>
 * </ul>
 *
 * <h3>Notes:</h3>
 * <ul>
 *   <li>Represents only "no value"; not interchangeable with <code>nullopt</code> or nullable types.</li>
 *   <li>Has no runtime state, equality is always <code>true</code>.</li>
 * </ul>
 *
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once
#include <type_traits>

namespace jh::typed {

    /// @brief Trivial empty type representing "no value".
    struct monostate final {
        constexpr bool operator==(monostate) const noexcept { return true; }
        constexpr bool operator!=(monostate) const noexcept { return false; }
    };

    static_assert(std::is_trivially_copyable_v<monostate>);
    static_assert(std::is_trivially_constructible_v<monostate>);
    static_assert(std::is_standard_layout_v<monostate>);

    /// @brief Type trait: checks whether T is <code>monostate</code>.
    template<typename T>
    struct is_monostate final : std::false_type {};

    template<>
    struct is_monostate<monostate> final : std::true_type {};

    /// @brief Concept: satisfied only if T is <code>monostate</code>.
    template<typename T>
    concept monostate_t = is_monostate<T>::value;
} // namespace jh::typed
