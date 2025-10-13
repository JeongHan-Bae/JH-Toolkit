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
 * @file type_name.h (macros)
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 * @brief Compile-time type name extraction using compiler-specific macros.
 *
 * This header provides <code>jh::macro::type_name&lt;T&gt;()</code>, which extracts
 * the unmangled type name of <code>T</code> from <code>__PRETTY_FUNCTION__</code>
 * (Clang/GCC) without requiring RTTI.
 *
 * <h3>Design Notes:</h3>
 * <ul>
 *   <li>Implemented with compiler-dependent "macro magic".</li>
 *   <li>No ABI or RTTI dependency — works even with <code>-fno-rtti</code>.</li>
 *   <li>Only intended for <b>debugging, logging, and diagnostics</b>.</li>
 *   <li><b>Do not</b> rely on this in production logic:
 *       <ul>
 *         <li>Output is <tt>compiler-</tt> and <tt>version-</tt>specific.</li>
 *         <li>Format may change between toolchains.</li>
 *         <li>No guarantees of stability across builds.</li>
 *       </ul>
 *   </li>
 * </ul>
 *
 * <h3>Usage Example:</h3>
 * @code
 *   std::cout << jh::macro::type_name&lt;int&gt;();            // "int"
 *   std::cout << jh::macro::type_name&lt;array&lt;int, 4&gt;&gt;();  // "jh::pod::array&lt;int, 4&gt;"
 * @endcode
 *
 * @note Falls back to <code>"unknown"</code> if compiler is unsupported.
 * @note Not suitable for serialization or program logic.
 *
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */


#pragma once
#include <string_view>
#include "jh/macros/platform.h"

namespace jh::macro {
    /**
     * @brief Extract the human-readable type name of <code>T</code> at compile time.
     *
     * Uses compiler-specific macros (Clang/GCC <code>__PRETTY_FUNCTION__</code>)
     * to obtain the unmangled type name of a template parameter.
     *
     * <h4>Key Properties:</h4>
     * <ul>
     *   <li>No RTTI dependency (works with <code>-fno-rtti</code>).</li>
     *   <li>Result is compiler- and version-specific.</li>
     *   <li>Evaluated entirely at compile time (constexpr).</li>
     * </ul>
     *
     * @warning
     * <ul>
     *   <li>Do <b>not</b> use this for serialization, persistence, or ABI logic.</li>
     *   <li>Intended solely for debugging, logging, and developer diagnostics.</li>
     *   <li>Format is <b>not stable</b> across compilers or versions.</li>
     * </ul>
     *
     * @tparam T The type to inspect.
     * @return <code>std::string_view</code> with the extracted type name,
     *         or <code>"unknown"</code> if extraction fails.
     */
    template<typename T>
    constexpr std::string_view type_name() {
#if IS_GCC || IS_CLANG
        constexpr std::string_view func = __PRETTY_FUNCTION__;
        // GCC:   constexpr std::string_view type_name() [with T = int]
        // Clang: std::string_view type_name() [T = int]

        constexpr std::string_view key = "T = ";
        const auto start = func.find(key);
        if (start == std::string_view::npos) {
            return "unknown";
        }

        const auto from = start + key.size();
        const auto end = func.rfind(']');
        if (end == std::string_view::npos || end <= from) {
            return "unknown";
        }

        return func.substr(from, end - from);
#else
        return "unknown";
#endif
    }
}
