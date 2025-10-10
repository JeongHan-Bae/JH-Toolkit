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
 * @brief Duck-typed extension of <code>jh::sim_pool</code> — automatic hash and equality inference for immutable objects.
 *
 * <h3>Overview</h3>
 * <p>
 * <code>jh::pool&lt;T&gt;</code> is a lightweight, duck-typed wrapper around
 * <code>jh::sim_pool&lt;T, Hash, Eq&gt;</code>, designed for types that expose
 * both a <code>hash()</code> method and an equality operator.
 * It automatically infers hashing and equality behavior, eliminating the need
 * to manually register <code>Hash</code> and <code>Eq</code> functors.
 * </p>
 *
 * <p>
 * This specialization enables transparent, content-based deduplication of
 * shared immutable or <em>structurally immutable</em> objects while preserving
 * all behavioral guarantees and thread-safety mechanisms of <code>jh::sim_pool</code>.
 * </p>
 *
 * <h4>Automatic Type Deduction</h4>
 * <p>
 * For any type <code>T</code> that satisfies both:
 * </p>
 * <ul>
 *   <li><code>std::uint64_t hash() const noexcept</code></li>
 *   <li><code>bool operator==(const T&amp;) const noexcept</code></li>
 * </ul>
 * <p>
 * <code>jh::pool&lt;T&gt;</code> automatically applies
 * <code>jh::weak_ptr_hash&lt;T&gt;</code> and
 * <code>jh::weak_ptr_eq&lt;T&gt;</code> internally,
 * enabling seamless pooling of semantically identical instances without
 * requiring external hash or equality registration.
 * </p>
 *
 * <h4>Type Requirements</h4>
 * <ul>
 *   <li>Objects must be <strong>immutable</strong> or at least
 *       <strong>structurally immutable</strong> — fields affecting hashing
 *       and equality must remain constant throughout their lifetime.</li>
 *   <li><code>T::hash()</code> must produce a consistent hash for logically identical values.</li>
 *   <li><code>T::operator==()</code> must define stable, content-based equality semantics.</li>
 * </ul>
 *
 * <h4>Non-hashable Types</h4>
 * <p>
 * Types that do not implement <code>hash()</code> may still use
 * <code>jh::sim_pool</code> directly by providing a custom hash functor.
 * In such cases, <code>jh::weak_ptr_eq&lt;T&gt;</code> can be reused for
 * equality comparison, since it forwards comparison to
 * the underlying type's <code>operator==</code>.
 * </p>
 *
 * <p>
 * This allows non-hashable but equality-comparable types to participate in
 * pooling without modifying their class definition.
 * The resulting behavior remains fully compatible with the semantics of
 * <code>jh::pool</code>.
 * </p>
 *
 * <h4>Behavior and Semantics</h4>
 * <p>
 * All lifecycle, synchronization, and resizing behaviors are identical to
 * <code>jh::sim_pool</code>:
 * </p>
 * <ul>
 *   <li><strong>Weak reference observation</strong> — the pool tracks shared instances without owning them.</li>
 *   <li><strong>Construct-first, lock-then-insert</strong> — insertion occurs atomically after tentative construction.</li>
 *   <li><strong>Attempt-based cleanup</strong> — expired entries are reclaimed on behavioral triggers or explicit cleanup.</li>
 *   <li><strong>Adaptive resizing</strong> — managed by high/low watermarks
 *       (<tt>0.875</tt> / <tt>0.25</tt>) to balance reuse and avoid jitter.</li>
 *   <li><strong>Thread-safe access</strong> — concurrent read access and atomic insertions under shared mutex protection.</li>
 * </ul>
 *
 * <h4>Relationship to <code>jh::sim_pool</code></h4>
 * <ul>
 *   <li><code>jh::pool&lt;T&gt;</code> is a duck-typed convenience adapter.</li>
 *   <li>Internally inherits
 *       <code>jh::sim_pool&lt;T, weak_ptr_hash&lt;T&gt;, weak_ptr_eq&lt;T&gt;&gt;</code>.</li>
 *   <li>All mechanisms of <code>jh::sim_pool</code> — cleanup, deduplication,
 *       resizing, and thread safety — are fully preserved.</li>
 *   <li>Recommended as the primary interface for immutable or
 *       structurally immutable types such as <code>jh::immutable_str</code>.</li>
 * </ul>
 *
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include <concepts>         // NOLINT for concepts
#include <cstdint>          // for std::uint64_t
#include <utility>          // for std::ignore
#include "jh/sim_pool.h"

namespace jh {

    /**
     * @brief Compile-time concept verifying that a type provides a hash function.
     *
     * <p><strong>Purpose:</strong></p>
     * <p>
     * Ensures that a type <strong>T</strong> defines a member function
     * returning a hash value convertible to <code>std::uint64_t</code>.
     * The hash function must be <strong>pure</strong> — it must always produce
     * the same value for logically equivalent instances.
     * </p>
     *
     * <p><strong>Definition:</strong></p>
     * <pre><code>std::uint64_t T::hash() const noexcept;</code></pre>
     *
     * <p>
     * This concept is required by <code>jh::pool</code> and
     * <code>jh::sim_pool</code> to enable content-based lookup and deduplication
     * instead of pointer-based identity comparison.
     * </p>
     *
     * @tparam T The candidate type to be checked.
     *
     * @note
     * <ul>
     *   <li>The check uses <code>const T&amp;</code> instead of <code>const T</code>
     *       to match standard C++ conventions and avoid the impression that
     *       <strong>T</strong> must be copy-constructible.</li>
     *   <li>Concept checking is a non-evaluated context; even if a type deletes
     *       its copy or move constructors (e.g. <code>immutable_str</code>),
     *       the check will still succeed as long as <code>obj.hash()</code>
     *       is a valid expression.</li>
     * </ul>
     */
    template<typename T>
    concept has_hash = requires(const T& obj)
    {
        { obj.hash() } -> std::convertible_to<std::uint64_t>;
    };

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
     * <p><strong>Behavior:</strong></p>
     * <ul>
     *   <li>If the pointer is expired, returns <tt>0</tt>.</li>
     *   <li>If valid, locks and calls <code>T::hash()</code> on the underlying object.</li>
     *   <li>Performs a preliminary <code>hash()</code> access to guarantee consistent results
     *       during concurrent insertion into a pool.</li>
     * </ul>
     *
     * <p><strong>Purpose:</strong></p>
     * <p>
     * Enables <code>jh::pool</code> and <code>jh::sim_pool</code> to hash weakly referenced
     * shared objects without altering ownership or extending lifetimes.
     * </p>
     *
     * @tparam T The managed type, which must satisfy <code>has_hash</code>.
     */
    template<typename T>
        requires has_hash<T>
    struct weak_ptr_hash {
        std::size_t operator()(const std::weak_ptr<T> &ptr) const noexcept {
            if (const auto sp = ptr.lock()) {
                // Pre-touch hash once to ensure consistent hash during insert()
                std::ignore = sp->hash(); // ensures atomic hash() before insert
                return sp->hash();
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
     * <p><strong>Purpose:</strong></p>
     * <p>
     * Provides a simplified user interface for pooling immutable or
     * structurally immutable objects. When a type <strong>T</strong> defines
     * both <code>hash()</code> and <code>operator==()</code>,
     * <code>jh::pool&lt;T&gt;</code> automatically applies
     * <code>jh::weak_ptr_hash&lt;T&gt;</code> and
     * <code>jh::weak_ptr_eq&lt;T&gt;</code> as internal policies.
     * </p>
     *
     * <p><strong>Behavior:</strong></p>
     * <ul>
     *   <li>Deduplicates shared instances based on logical equivalence.</li>
     *   <li>Observes object lifetimes via <code>std::weak_ptr</code>.</li>
     *   <li>Preserves all concurrency and cleanup semantics from <code>jh::sim_pool</code>.</li>
     * </ul>
     *
     * <p><strong>Type Requirements:</strong></p>
     * <ul>
     *   <li><code>T::hash()</code> must return a stable <code>std::uint64_t</code> hash.</li>
     *   <li><code>T::operator==()</code> must define logical equality semantics.</li>
     *   <li>All fields involved in hash or equality must remain constant for the object's lifetime.</li>
     * </ul>
     *
     * @tparam T The pooled object type satisfying both <code>has_hash</code> and <code>has_equal</code>.
     */
    template<typename T>
        requires (has_hash<T> && has_equal<T>) // Define jh::pool<T> only for types with hash() and operator==
    class pool final : public sim_pool<T, weak_ptr_hash<T>, weak_ptr_eq<T> > {
        using sim_pool<T, weak_ptr_hash<T>, weak_ptr_eq<T> >::sim_pool; ///< Inherit constructors from `sim_pool`
    };
} // namespace jh
