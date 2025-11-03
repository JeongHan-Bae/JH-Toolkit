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
 * @file pool.h
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 * @brief Duck-typed adapter of <code>jh::sim_pool</code> — content-based pooling for immutable types.
 *
 * <h3>Overview</h3>
 * <p>
 * <code>jh::pool&lt;T&gt;</code> provides an automatic, duck-typed wrapper over
 * <code>jh::sim_pool&lt;T, Hash, Eq&gt;</code>. It infers hashing and equality
 * policies for any type <code>T</code> that exposes a valid <code>hash()</code>
 * (directly or via <code>jh::hash&lt;T&gt;</code>) and <code>operator==</code>.
 * This enables transparent deduplication of shared, immutable objects without
 * manually specifying <code>Hash</code> or <code>Eq</code> functors.
 * </p>
 *
 * <h3>Automatic Type Deduction</h3>
 * <p>
 * For any <code>T</code> satisfying:
 * </p>
 * <ul>
 *   <li><code>jh::concepts::extended_hashable&lt;T&gt;</code></li>
 *   <li><code>jh::has_equal&lt;T&gt;</code></li>
 * </ul>
 * <p>
 * the specialization
 * <code>jh::pool&lt;T&gt;</code> internally binds
 * <code>jh::weak_ptr_hash&lt;T&gt;</code> and
 * <code>jh::weak_ptr_eq&lt;T&gt;</code>,
 * providing consistent content-based identity semantics for pooled objects.
 * </p>
 *
 * <h3>Supported Hash Mechanisms</h3>
 * <p>
 * From version <b>1.3.5</b> onward, <code>jh::pool</code> supports full
 * <em>duck-typed hash deduction</em> through <code>jh::hash&lt;T&gt;</code>.
 * Hashes are resolved in the following order:
 * </p>
 * <pre><tt>
 * std::hash&lt;T&gt;{ }(v)
 *   > hash(v)
 *   > v.hash()
 * </tt></pre>
 * <p>
 * This makes <code>jh::pool</code> compatible with standard, ADL, and
 * member-based hashing without requiring explicit specialization.
 * </p>
 *
 * <h3>Requirements</h3>
 * <ul>
 *   <li><b>Immutability</b> — fields affecting equality and hashing must remain constant.</li>
 *   <li><b>Equality</b> — <code>T::operator==</code> must define logical, content-based comparison.</li>
 *   <li><b>Hashability</b> — must satisfy <code>jh::concepts::extended_hashable&lt;T&gt;</code>.</li>
 * </ul>
 *
 * <h3>Behavior</h3>
 * <ul>
 *   <li>Weak reference tracking: pooled objects are observed, not owned.</li>
 *   <li>Atomic construct-then-insert semantics for safe concurrent insertion.</li>
 *   <li>Automatic cleanup of expired entries on access or trigger points.</li>
 *   <li>Adaptive resizing with load thresholds (0.875 / 0.25).</li>
 *   <li>Full thread safety under shared mutex protection.</li>
 * </ul>
 *
 * <h3>Usage</h3>
 * <p>
 * Use <code>jh::pool&lt;T&gt;</code> when:
 * </p>
 * <ul>
 *   <li><code>T</code> is immutable or structurally immutable.</li>
 *   <li><code>T</code> defines <code>hash()</code> or is compatible with
 *       <code>std::hash</code> / <code>hash(const T&)</code>.</li>
 *   <li><code>T</code> defines <code>operator==</code> with content semantics.</li>
 * </ul>
 *
 * <p>
 * For types that do not satisfy these conditions, use
 * <code>jh::sim_pool&lt;T, CustomHash, CustomEq&gt;</code> directly.
 * </p>
 *
 * @version <pre>1.3.5+</pre>
 * @date <pre>2025</pre>
 *
 * @see jh::sim_pool
 * @see jh::hash
 * @see jh::weak_ptr_hash
 * @see jh::weak_ptr_eq
 */

#pragma once

#include <concepts>         // NOLINT for concepts
#include <cstdint>          // for std::uint64_t
#include <utility>          // for std::ignore
#include "jh/sim_pool.h"
#include "jh/conceptual/hashable.h"

namespace jh {

    /**
     * @brief Compile-time concept verifying that a type supports equality comparison.
     *
     * <p><strong>Purpose:</strong></p>
     * <p>
     * Confirms that a type <strong>T</strong> provides a logical equality operator
     * for comparing object content rather than address identity.
     * Equality must reflect semantic equivalence and remain stable across the
     * lifetime of immutable objects.
     * </p>
     *
     * <p><strong>Definition:</strong></p>
     * <pre><code>bool operator==(const T&amp;, const T&amp;) noexcept;</code></pre>
     *
     * <p>
     * This concept is required by <code>jh::pool</code> and
     * <code>jh::sim_pool</code> to ensure that deduplication merges
     * logically identical instances.
     * </p>
     *
     * @tparam T The candidate type to be checked.
     *
     * @note
     * <ul>
     *   <li>The concept uses <code>const T&amp;</code> parameters, allowing
     *       support for immovable or non-copyable types such as
     *       <code>immutable_str</code>.</li>
     *   <li>The check covers both member and non-member <code>operator==</code>
     *       (via ADL lookup) and requires the result to be convertible to
     *       <code>bool</code>.</li>
     *   <li>Like all <code>requires</code> expressions, this check does not
     *       instantiate code or construct objects; it only verifies that the
     *       comparison expression is well-formed.</li>
     * </ul>
     */
    template<typename T>
    concept has_equal = requires(const T& lhs, const T& rhs)
    {
        { lhs == rhs } -> std::convertible_to<bool>;
    };

    /**
     * @brief Content-based hash functor for <code>std::weak_ptr&lt;T&gt;</code>.
     *
     * <h3>Behavior</h3>
     * <ul>
     *   <li>If the pointer is expired, returns <tt>0</tt>.</li>
     *   <li>If valid, locks and applies the unified <code>jh::hash&lt;T&gt;</code> functor
     *       to the underlying object.</li>
     *   <li>Ensures consistent results during concurrent insertion into a
     *       <code>jh::pool</code> by performing a single well-defined hash access
     *       per locked instance.</li>
     * </ul>
     *
     * <h3>Purpose</h3>
     * <p>
     * Enables <code>jh::pool</code> and <code>jh::sim_pool</code> to hash
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
     * This allows any type declaring a valid hash mechanism — standard,
     * ADL-based, or member-based — to participate in pooling without the need
     * for custom specialization.
     * </p>
     *
     * @tparam T The managed type, which must satisfy
     *           <code>jh::concepts::extended_hashable</code>.
     *
     * @see jh::concepts::extended_hashable
     * @see jh::hash
     */
    template<typename T>
    requires jh::concepts::extended_hashable<T>
    struct weak_ptr_hash {
        std::size_t operator()(const std::weak_ptr<T>& ptr) const noexcept {
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
     * @tparam T The managed type, which must satisfy <code>has_equal</code>.
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
     * @brief Duck-typed specialization of <code>jh::sim_pool</code> with automatic
     *        hash and equality inference.
     *
     * <h3>Purpose</h3>
     * <p>
     * Provides a simplified user interface for pooling immutable or
     * structurally immutable objects. When a type <strong>T</strong> satisfies
     * both <code>jh::concepts::extended_hashable</code> and
     * <code>has_equal</code>, <code>jh::pool&lt;T&gt;</code> automatically
     * applies <code>jh::weak_ptr_hash&lt;T&gt;</code> and
     * <code>jh::weak_ptr_eq&lt;T&gt;</code> as internal policies.
     * </p>
     *
     * <h3>Behavior</h3>
     * <ul>
     *   <li>Deduplicates shared instances based on logical equivalence.</li>
     *   <li>Observes object lifetimes through <code>std::weak_ptr</code>.</li>
     *   <li>Preserves all concurrency, cleanup, and resizing semantics from
     *       <code>jh::sim_pool</code>.</li>
     * </ul>
     *
     * <h3>Type Requirements</h3>
     * <ul>
     *   <li>The type must be <strong>logically immutable</strong> — all fields
     *       affecting equality and hashing remain constant during the object’s
     *       lifetime.</li>
     *   <li>It must satisfy <code>jh::concepts::extended_hashable</code>,
     *       meaning it provides one of:
     *       <ul>
     *         <li><code>std::hash&lt;T&gt;</code> specialization</li>
     *         <li>ADL-discoverable <code>hash(const T&amp;)</code> free function</li>
     *         <li>Member function <code>T::hash()</code></li>
     *       </ul>
     *   </li>
     *   <li>It must support <code>operator==</code> for content-based equality.</li>
     * </ul>
     *
     * <h3>Version Note (since 1.3.5)</h3>
     * <p>
     * From <b>1.3.5</b> onward, <code>jh::pool</code> supports full
     * <em>duck-typed hash deduction</em> through <code>jh::hash&lt;T&gt;</code>,
     * enabling seamless use with standard, ADL, or member-based hash semantics.
     * No explicit <code>hash()</code> method is required as long as one of the
     * supported mechanisms is available.
     * </p>
     *
     * @note
     *
     * <p>
     * <code>jh::hash&lt;T&gt;</code> is <b>not</b> a registration point.
     * It performs only <em>behavioral deduction</em> based on available
     * hash semantics. The system intentionally does <b>not</b> recognize or
     * check for user specializations of <code>jh::hash&lt;T&gt;</code>.
     * </p>
     *
     * <p>
     * For better portability and interoperability with the standard library or
     * third-party frameworks, it is <b>recommended</b> to register your hash
     * via one of the following standard mechanisms instead:
     * </p>
     * <ul>
     *   <li>Specialize <code>std::hash&lt;T&gt;</code> — the canonical
     *       registration form recognized by all standard containers.</li>
     *   <li>Provide an <b>ADL-discoverable</b> free function
     *       <code>size_t hash(const T&amp;)</code>.</li>
     *   <li>Implement a member function <code>size_t T::hash() const</code>.</li>
     * </ul>
     *
     * <p>
     * <b>Design rationale:</b> this deliberate restriction ensures that
     * <code>jh::hash&lt;T&gt;</code> remains a <em>deduction layer</em> rather
     * than a registration layer, preventing fragmented extension points across
     * libraries. Only the three canonical mechanisms above are considered part
     * of the official detection chain.
     * </p>
     *
     * @tparam T The pooled object type satisfying both
     *           <code>jh::concepts::extended_hashable</code> and
     *           <code>has_equal</code>.
     *
     * @see jh::concepts::extended_hashable
     * @see jh::weak_ptr_hash
     * @see jh::weak_ptr_eq
     * @see jh::hash
     */
    template<typename T>
        requires (jh::concepts::extended_hashable<T> && has_equal<T>) // Define jh::pool<T> only for types with hash() and operator==
    class pool final : public sim_pool<T, weak_ptr_hash<T>, weak_ptr_eq<T> > {
        using sim_pool<T, weak_ptr_hash<T>, weak_ptr_eq<T> >::sim_pool; ///< Inherit constructors from `sim_pool`
    };
} // namespace jh
