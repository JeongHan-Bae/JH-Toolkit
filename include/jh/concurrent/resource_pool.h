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
 * @file observe_pool.h
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 * @brief User-facing aliases for <code>jh::conc::flat_pool</code> with deduced hashing.
 *
 * <p>
 * <code>jh::resource_pool</code> provides a simplified, user-facing entry point
 * to <code>jh::conc::flat_pool</code> by fixing the hash policy to
 * <code>jh::hash&lt;Key&gt;</code> and exposing only the most commonly varied
 * parameters: key, value, and allocator.
 * </p>
 *
 * <p>
 * Unlike <code>flat_pool</code>, which allows arbitrary hash functor substitution,
 * <code>resource_pool</code> assumes that <code>jh::hash&lt;Key&gt;</code> is a valid
 * hashing strategy for the given key type. This removes the need for explicit hash
 * specification while still preserving extensibility via allocator replacement.
 * </p>
 *
 * <p>
 * The <code>resource_pool_set</code> variant further specializes this pattern by
 * fixing the value type to <code>jh::typed::monostate</code>, providing a set-like
 * abstraction over keys with pool-managed storage and concurrency semantics.
 * </p>
 *
 * <p>
 * The pool manages object lifetimes directly and owns all stored elements.
 * Logical identity is defined exclusively by the external key, independent of
 * object address or construction history. Internally, elements are stored in
 * contiguous memory and relocated as needed, enabling fragmentation-free
 * storage and cache-efficient access under concurrent workloads.
 * </p>
 *
 * <p>
 * These aliases do not introduce additional constraints, ownership semantics, or
 * behavioral indirection. All concurrency control, lifetime management, and
 * resizing behavior is defined exclusively by <code>jh::conc::flat_pool</code>.
 * </p>
 *
 * <p>
 * As user-facing facilities under the <code>jh</code> namespace, they are available via:
 * </p>
 * <ul>
 *   <li><code>&lt;jh/pool&gt;</code></li>
 *   <li><code>&lt;jh/concurrency&gt;</code></li>
 * </ul>
 *
 * @version <pre>1.4.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include "jh/concurrent/flat_pool.h"

namespace jh {
    /**
     * @brief Convenience alias of <code>jh::conc::flat_pool</code> with
     *        behaviorally deduced hashing.
     *
     * <p>
     * <code>jh::resource_pool&lt;Key, Value, Alloc&gt;</code> is a direct alias of
     * <code>jh::conc::flat_pool</code> that fixes the hash functor to
     * <code>jh::hash&lt;Key&gt;</code>. No additional abstraction layer or duck-typing
     * mechanism is introduced.
     * </p>
     *
     * <p>
     * The alias relies on <code>jh::hash&lt;Key&gt;</code> to successfully resolve a
     * hashing strategy for <code>Key</code>, following its resolution order:
     * <code>std::hash</code>, ADL-discovered free <code>hash(key)</code>, or
     * <code>key.hash()</code>.
     * </p>
     *
     * <p>
     * This design preserves the full behavior and constraints of
     * <code>jh::conc::flat_pool</code> while reducing template verbosity for the
     * common case where no custom hash functor is required.
     * </p>
     *
     * @tparam Key
     * The key type defining logical identity. <code>Key</code> must satisfy
     * <code>jh::concepts::extended_hashable</code> and
     * <code>jh::concepts::is_contiguous_reallocable</code>.
     *
     * @tparam Value
     * The value type stored in the pool. Must either be
     * <code>jh::typed::monostate</code> or satisfy
     * <code>jh::concepts::is_contiguous_reallocable</code>.
     *
     * @tparam Alloc
     * Allocator used for internal storage. Defaults to an allocator compatible with
     * the underlying <code>flat_pool</code> value representation.
     *
     * @note
     * <h5>Design intent:</h5>
     * <p>
     * <code>jh::resource_pool</code> exists solely to reduce the cognitive and syntactic
     * cost of using <code>flat_pool</code> in the common case.
     * </p><p>
     * When a nonstandard hashing strategy is required, users should instantiate
     * <code>jh::conc::flat_pool</code> directly with an explicit hash functor.
     * </p>
     */
    template<typename Key,
            typename Value = jh::typed::monostate,
            typename Alloc = std::allocator<jh::conc::detail::value_t<Key, Value>>>
    using resource_pool = jh::conc::flat_pool<Key, Value, jh::hash<Key>, Alloc>;

    /**
     * @brief Set-style specialization of <code>jh::resource_pool</code>.
     *
     * <p>
     * <code>jh::resource_pool_set&lt;Key&gt;</code> is an alias of
     * <code>jh::conc::flat_pool</code> with <code>Value</code> fixed to
     * <code>jh::typed::monostate</code>. It represents a concurrent pool of unique
     * keys with no associated payload.
     * </p>
     */
    template<typename Key,
            typename Alloc = std::allocator<Key>>
    using resource_pool_set = jh::conc::flat_pool<Key, jh::typed::monostate, jh::hash<Key>, Alloc>;

} // namespace jh
