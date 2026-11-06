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
 * @file observe_pool.h
 * @brief Content-based pool with automatically selected object hash and equality.
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 *
 * <code>observe_pool</code> uses <code>pointer_pool</code>'s own handle and
 * delegates object hashing to <code>jh::hash&lt;T&gt;</code>. The supplied
 * <code>Mutex</code> protects the index, comparisons, and handle reference
 * counts. Objects remain alive while a handle owns them, including after the
 * pool is cleared or destroyed.
 */

#pragma once

#include <functional>
#include <shared_mutex>
#include "jh/concurrent/pointer_pool.h"
#include "jh/conceptual/hashable.h"

namespace jh {
    /**
     * @brief Deduplicates objects using their content hash and equality.
     * @tparam T Immutable or structurally immutable object type.
     * @tparam Mutex RW mutex used for the pool index and handle lifetime.
     *
     * <code>T</code> must be hashable through <code>jh::hash&lt;T&gt;</code> and
     * comparable through <code>operator==</code>. The default mutex is
     * <code>std::shared_mutex</code>; callers can select another mutex whose
     * runtime synchronization behavior meets their platform requirements.
     */
    template<typename T, typename Mutex = std::shared_mutex>
    using observe_pool = conc::pointer_pool<T, jh::hash<T>, std::equal_to<T>, Mutex>;
} // namespace jh
