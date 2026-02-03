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
 * @file shared_ptr_atomic_shim.h (detail)
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 * @brief Suppresses GCC's warnings about <code>std::shared_ptr</code> atomic operations.
 *
 * @details
 * Starting from GCC 15, libstdc++ emits warnings when the standard atomic
 * access functions for <code>std::shared_ptr</code> are used
 * (<code>atomic_load</code>, <code>atomic_store</code>,
 * <code>atomic_exchange</code>, <code>atomic_compare_exchange_* </code>).
 * <p>
 * These warnings encourage the use of the GCC-specific extension
 * <code>std::atomic&lt;std::shared_ptr&lt;T&gt;&gt;</code>, which is <b>not</b>
 * part of the ISO C++ standard and is not supported by other standard libraries.
 * </p><p>
 * The C++ standard defines atomic access to <code>std::shared_ptr</code>
 * exclusively through the free functions specified in:
 * <a href="https://timsong-cpp.github.io/cppwp/n4659/util.smartptr.shared.atomic">
 *   &sect;23.11.2.6 &mdash; shared_ptr atomic access
 * </a>
 * </p>
 *
 * @note
 * This shim exists solely to suppress GCC-15 diagnostics while preserving
 * strict ISO C++ portability. It does not introduce new semantics, does not
 * rely on GCC extensions, and does not alter the synchronization guarantees
 * defined by the standard.
 *
 * Users should treat these helpers as thin wrappers around the standard
 * atomic free functions. They are intended for use in environments where:
 * <ol>
 *   <li>GCC version 15 or newer emits warnings for the standard atomic
 *       shared_ptr operations.</li>
 *   <li>The project must remain portable across libstdc++, libc++, and the
 *       MSVC STL implementations.</li>
 *   <li>Vendor-specific types such as <code>std::atomic&lt;std::shared_ptr&lt;T&gt;&gt;</code>
 *       cannot be used.</li>
 * </ol>
 *
 * This header is intended for internal use. Public APIs should continue to
 * rely on the standard free functions specified in section 23.11.2.6 of the
 * C++ standard.
 */

#pragma once

#include <memory>
#include <atomic>
#include "jh/macros/platform.h"

namespace jh::detail::atomic {

#if IS_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

    template<typename T>
    inline std::shared_ptr<T>
    load(const std::shared_ptr<T> *p, std::memory_order mo) noexcept {
        return std::atomic_load_explicit(p, mo);
    }

    template<typename T>
    inline void
    store(std::shared_ptr<T> *p, std::shared_ptr<T> v, std::memory_order mo) noexcept {
        std::atomic_store_explicit(p, std::move(v), mo);
    }

    template<typename T>
    inline bool
    cas(std::shared_ptr<T> *p,
        std::shared_ptr<T> *expected,
        std::shared_ptr<T> desired,
        std::memory_order success,
        std::memory_order failure) noexcept {
        return std::atomic_compare_exchange_strong_explicit(
                p, expected, std::move(desired), success, failure
        );
    }

#if IS_GCC
#pragma GCC diagnostic pop
#endif

} // namespace jh::detail::atomic
