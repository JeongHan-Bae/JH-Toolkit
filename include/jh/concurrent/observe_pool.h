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
 * @brief Top-level user-facing pool for content-based interning of immutable objects.
 *
 * <p>
 * <code>jh::observe_pool&lt;T&gt;</code> is a duck-typed, user-oriented pooling facility
 * built on top of <code>jh::conc::pointer_pool</code>. It provides content-based
 * deduplication for immutable or structurally immutable objects, without requiring
 * explicit hash or equality policy specification.
 * </p>
 *
 * <p>
 * The pool observes object lifetimes via <code>std::weak_ptr</code> and never owns
 * pooled objects. Logical identity is defined by <code>T</code>'s hashing semantics
 * and <code>operator==</code>.
 * </p>
 *
 * <p>
 * <code>observe_pool</code> is defined as a direct alias of
 * <code>jh::conc::pointer_pool</code> with automatically selected
 * <code>jh::weak_ptr_hash&lt;T&gt;</code> and <code>jh::weak_ptr_eq&lt;T&gt;</code>.
 * Instantiation is valid only when the underlying type requirements are satisfied.
 * </p>
 *
 * <p>
 * As a user-facing pool under the <code>jh</code> namespace, it is available via:
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

#include <concepts>         // NOLINT for concepts
#include <cstdint>          // for std::uint64_t
#include <utility>          // for std::ignore
#include "jh/concurrent/pointer_pool.h"
#include "jh/conceptual/hashable.h"

namespace jh {

    /**
     * @brief Content-based hash functor for <code>std::weak_ptr&lt;T&gt;</code>.
     *
     * <h3>Behavior</h3>
     * <ul>
     *   <li>If the pointer is expired, returns <tt>0</tt>.</li>
     *   <li>If valid, locks and applies the unified <code>jh::hash&lt;T&gt;</code> functor
     *       to the underlying object.</li>
     *   <li>Ensures consistent results during concurrent insertion into a
     *       <code>jh::observe_pool</code> by performing a single well-defined hash access
     *       per locked instance.</li>
     * </ul>
     *
     * <h3>Purpose</h3>
     * <p>
     * Enables <code>jh::observe_pool</code> and <code>jh::conc::pointer_pool</code> to hash
     * weakly referenced shared objects by logical content, without altering
     * ownership or extending object lifetimes.
     * </p>
     *
     * <h3>Version Note (since 1.3.5)</h3>
     * <p>
     * Starting with <b>1.3.5</b>, <code>weak_ptr_hash&lt;T&gt;</code> supports
     * <em>automatic hash deduction</em> through
     * <code>jh::hash&lt;T&gt;</code>, which transparently resolves hashing
     * via the following precedence chain:
     * </p>
     * <pre><tt>
     * std::hash&lt;T&gt;{ }(v)
     *   &gt;  hash(v)
     *   &gt;  v.hash()
     * </tt></pre>
     *
     * <p>
     * This allows any type declaring a valid hash mechanism &mdash; standard,
     * ADL-based, or member-based &mdash; to participate in pooling without the need
     * for custom specialization.
     * </p>
     *
     * @tparam T The managed type, which must satisfy
     *           <code>jh::concepts::extended_hashable</code>.
     *
     * @see jh::concepts::extended_hashable
     * @see jh::hash
     */
    template<typename T> requires jh::concepts::extended_hashable<T>
    struct weak_ptr_hash final {
        std::size_t operator()(const std::weak_ptr<T> &ptr) const noexcept {
            if (const auto sp = ptr.lock()) {
                return jh::hash<T>{}(*sp);
            }
            return 0;
        }
    };

    /**
     * @brief Equality functor for <code>std::weak_ptr&lt;T&gt;</code>.
     *
     * <p><strong>Behavior:</strong></p>
     * <ul>
     *   <li>If either pointer is expired, comparison yields <tt>false</tt>.</li>
     *   <li>If both are valid, comparison is delegated to the underlying
     *       <code>T::operator==()</code>.</li>
     * </ul>
     *
     * <p><strong>Purpose:</strong></p>
     * <p>
     * Allows weak pointers to be compared by the logical content of their targets,
     * ensuring that semantically identical live objects match in pooling structures.
     * Expired entries are safely treated as distinct.
     * </p>
     *
     * @tparam T The managed type, which must satisfy equality comparing.
     */
    template<typename T> requires requires(const T &lhs, const T &rhs){
        { lhs == rhs } -> std::convertible_to<bool>;
    }
    struct weak_ptr_eq final {
        bool operator()(const std::weak_ptr<T> &lhs, const std::weak_ptr<T> &rhs) const noexcept {
            const auto sp1 = lhs.lock();
            const auto sp2 = rhs.lock();
            if (!sp1 || !sp2) return false;
            return *sp1 == *sp2;
        }
    };

    /**
     * @brief Duck-typed alias of <code>jh::conc::pointer_pool</code> for
     *        content-based pooling of immutable objects.
     *
     * <p>
     * <code>jh::observe_pool&lt;T&gt;</code> provides logical deduplication of
     * shared objects based on content hashing and equality. Objects are
     * observed via <code>std::weak_ptr</code> and are never owned by the pool.
     * </p>
     *
     * <p>
     * All concurrency, cleanup, and adaptive resizing behavior is inherited
     * directly from <code>jh::conc::pointer_pool</code>.
     * </p>
     *
     * @tparam T
     * The pooled object type. <code>T</code> must be logically immutable,
     * satisfy <code>jh::concepts::extended_hashable</code>, and support
     * content-based <code>operator==</code>.
     *
     * @note
     * <h5>Usage guidance:</h5>
     * <p>
     * <code>jh::observe_pool</code> relies on <code>std::shared_ptr</code> /
     * <code>std::weak_ptr</code> for object tracking. This inevitably introduces
     * heap fragmentation and reference-counting overhead. It is therefore intended
     * only for types that are <b>neither copyable nor movable</b>, and for workloads
     * where the total number of live objects and concurrency level remain modest.
     * Excessive object counts or high parallel pressure may lead to allocation
     * jitter and degraded performance.
     * </p><p>
     * On Windows platforms using the Universal CRT (including MinGW variants),
     * <code>std::shared_ptr</code> is not reliably thread-safe under contention.
     * Practical limits are significantly lower than on other platforms; it is
     * recommended to keep concurrency within approximately <b>4 threads</b> and
     * the total number of live pooled objects within roughly <b>2k</b>.
     * </p><p>
     * If the managed type is at least copyable or movable, prefer
     * <code>jh::resource_pool&lt;T&gt;</code>. If a stable key can be used to identify
     * objects, prefer <code>jh::resource_pool&lt;K, V&gt;</code>. When a key is available
     * but the value type is neither copyable nor movable, using
     * <code>jh::resource_pool&lt;K, std::shared_ptr&lt;V&gt;&gt;</code> is often a better
     * alternative: hashing and equality are applied only to the key, avoiding
     * expensive object-level comparisons and large-scale rehash jitter during
     * resizing.
     * </p>
     */
    template<typename T>
    using observe_pool =
            conc::pointer_pool<T, weak_ptr_hash<T>, weak_ptr_eq<T>>;

} // namespace jh
