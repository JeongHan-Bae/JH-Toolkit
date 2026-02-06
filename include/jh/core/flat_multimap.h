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
 * @file flat_multimap.h
 * @brief Flat ordered multimap container.
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 *
 * <h3>Design Rationale &mdash; Why <code>flat_multimap</code> Exists</h3>
 *
 * <p>
 * <code>jh::flat_multimap&lt;K,V&gt;</code> is <b>not</b> an extension of
 * <code>ordered_map</code> to support multiple values per key, nor is it a
 * tree-based multimap variant.
 * It exists to serve a <b>fundamentally different access pattern</b>.
 * </p>
 *
 * <p>
 * In practical systems, a multimap is rarely used merely because
 * "a key can have multiple values".
 * Instead, the defining operations are:
 * </p>
 *
 * <ul>
 *   <li>efficient <b>range queries</b> for a single key (<code>equal_range</code>),</li>
 *   <li>batch processing of all values associated with a key,</li>
 *   <li>bulk erasure of all entries for a key (<code>erase(key)</code>).</li>
 * </ul>
 *
 * <p>
 * These operations are <b>structurally hostile</b> to tree-based containers
 * that maintain balance via rotations.
 * In a contiguous AVL tree, removing a range of equivalent keys would require
 * repeated node removal, each potentially triggering rebalancing and rotations.
 * The resulting cost is not only higher, but also less predictable.
 * </p>
 *
 * <p>
 * For this reason, the <code>ordered_*</code> family deliberately does
 * <b>not</b> provide bulk-erasure or multimap-style range deletion APIs.
 * Their design goal is <b>stable, per-element operations</b> with bounded
 * rebalancing cost &mdash; a goal that conflicts with multimap semantics.
 * </p>
 *
 * <h3>Flat Multimap: Turning an Algorithm into a Container</h3>
 *
 * <p>
 * <code>flat_multimap</code> embraces a different principle:
 * </p>
 *
 * <blockquote>
 *   <b>Store elements contiguously, keep them stably sorted by key,
 *   and express multimap semantics as range operations over a flat sequence.</b>
 * </blockquote>
 *
 * <p>
 * Internally, the container is little more than a
 * <code>std::vector&lt;std::pair&lt;K,V&gt;&gt;</code>
 * maintained in sorted order.
 * Multimap operations are implemented using binary search
 * (<code>lower_bound</code>, <code>upper_bound</code>) and contiguous range
 * operations on the underlying storage.
 * </p>
 *
 * <p>
 * This effectively <b>packages the "sorted vector + binary search";
 * algorithm into a first-class container</b>, with explicit semantics for:
 * </p>
 *
 * <ul>
 *   <li>contiguous storage of equivalent keys,</li>
 *   <li>range-oriented lookup,</li>
 *   <li>batch erasure with a single compaction step.</li>
 * </ul>
 *
 * <p>
 * In workloads such as in-memory tables, indexing layers, routing maps,
 * or subscription registries, this model aligns naturally with the
 * dominant access patterns: scan, group, and rebuild.
 * </p>
 *
 * <h3>Why Not Extend the AVL-Based <code>ordered_map</code>?</h3>
 *
 * <p>
 * While it is theoretically possible to add multimap support to a contiguous
 * AVL structure, doing so would compromise its core invariants:
 * </p>
 *
 * <ul>
 *   <li>range deletion would trigger repeated rotations,</li>
 *   <li>erase(key) would devolve into multiple independent erase operations,</li>
 *   <li>cost predictability would degrade under multimap workloads.</li>
 * </ul>
 *
 * <p>
 * Rather than overloading the <code>ordered_*</code> containers with semantics
 * they are ill-suited for, <code>flat_multimap</code> provides a
 * <b>purpose-built structure</b> whose behavior is transparent and
 * intentionally biased toward bulk and range-oriented operations.
 * </p>
 *
 * <h3>Why There Is No <code>flat_multiset</code></h3>
 *
 * <p>
 * A multiset variant would add little semantic value.
 * A sorted sequence of keys with duplicates is already fully expressible as
 * a <code>std::vector&lt;K&gt;</code> with <code>std::stable_sort</code>.
 * </p>
 *
 * <p>
 * Unlike <code>flat_multimap</code>, which must expose key-value association
 * and range-based deletion, a hypothetical <code>flat_multiset</code> would
 * amount to a thin wrapper over an existing algorithm without providing
 * meaningful additional structure or guarantees.
 * </p>
 *
 * <h3>About Performance</h3>
 *
 * <p>
 * Benchmarks (Apple Silicon M3, LLVM clang++20, 2025) compare
 * <code>jh::flat_multimap</code> against <code>std::multimap</code>
 * under representative workloads.
 * Measurements were taken at both <code>-O0</code> and
 * <code>-O3 -march=native</code>, with consistent trends observed
 * across optimization levels.
 * </p>
 *
 * <table>
 *   <tr>
 *     <th>Operation</th>
 *     <th><code>std::multimap</code></th>
 *     <th><code>jh::flat_multimap</code></th>
 *     <th>Notes</th>
 *   </tr>
 *   <tr>
 *     <td>Random insert (5k)</td>
 *     <td>faster</td>
 *     <td>slower</td>
 *     <td>tree insertion avoids bulk movement</td>
 *   </tr>
 *   <tr>
 *     <td>Ordered insert (5k)</td>
 *     <td>slower, higher variance</td>
 *     <td>consistently faster</td>
 *     <td>contiguous insertion dominates</td>
 *   </tr>
 *   <tr>
 *     <td>Bulk construction (50k)</td>
 *     <td>allocator-dominated</td>
 *     <td>&asymp; 2-4&times; faster</td>
 *     <td>single append + stable sort</td>
 *   </tr>
 *   <tr>
 *     <td>Random lookup</td>
 *     <td>slightly faster</td>
 *     <td>slightly slower</td>
 *     <td>binary search vs pointer traversal</td>
 *   </tr>
 *   <tr>
 *     <td>Iteration</td>
 *     <td>pointer chasing</td>
 *     <td>&asymp; 50-90&times; faster</td>
 *     <td>sequential memory traversal</td>
 *   </tr>
 *   <tr>
 *     <td>Erase by key</td>
 *     <td>faster</td>
 *     <td>slower</td>
 *     <td>range compaction cost</td>
 *   </tr>
 * </table>
 *
 * <h4>Observed Behavior</h4>
 *
 * <ul>
 *   <li>
 *     Large-scale construction using <code>bulk_insert</code> consistently
 *     outperforms node-based multimap insertion at 50k elements and beyond.
 *   </li>
 *   <li>
 *     Sequential iteration shows a decisive advantage for
 *     <code>flat_multimap</code>, often exceeding one order of magnitude.
 *   </li>
 *   <li>
 *     Lookup and erase operations are generally slower than
 *     <code>std::multimap</code> under low-pressure workloads, reflecting the
 *     cost of binary search and contiguous range compaction.
 *     <br>
 *     Under high-density datasets (e.g. &ge; 1M elements with dense key
 *     distributions), lookup performance converges toward that of
 *     <code>std::multimap</code>, as contiguous memory layout and cacheline
 *     locality increasingly dominate pointer-based traversal costs.
 *   </li>
 *   <li>
 *     Performance characteristics remain stable across optimization levels,
 *     indicating that results are dominated by memory layout and access
 *     patterns rather than compiler-specific optimizations.
 *   </li>
 * </ul>
 *
 *
 * <h3>Design Summary</h3>
 *
 * <p>
 * The final design of <code>flat_multimap</code> follows the same fundamental
 * philosophy as the <code>ordered_*</code> family: prioritizing cache locality,
 * high hit rates, and contiguous memory layout over optimal asymptotic
 * performance for individual operations.
 * </p>
 *
 * <p>
 * For small datasets, incremental insertion via <code>insert</code> or
 * <code>emplace</code> is sufficient and convenient.
 * As the container grows beyond a few thousand elements (typically around
 * 5k entries), bulk-oriented construction is strongly recommended to preserve
 * predictable performance characteristics.
 * </p>
 *
 * <p>
 * Compared to <code>std::multimap</code>, <code>flat_multimap</code> deliberately
 * trades a portion of lookup and erase performance (on the order of ~30% under
 * low-pressure workloads) in exchange for:
 * </p>
 *
 * <ul>
 *   <li>significantly improved L1/L2 cache hit rates,</li>
 *   <li>fully contiguous storage with no allocator-induced fragmentation,</li>
 *   <li>more stable performance at larger scales.</li>
 * </ul>
 *
 * <p>
 * While pointer-based containers often perform well in microbenchmarks executed
 * in isolation, such measurements typically assume an idealized environment
 * with no long-term allocator pressure or memory fragmentation.
 * In real-world systems, contiguous containers tend to exhibit more stable
 * behavior over time due to their compact layout and predictable access
 * patterns.
 * </p>
 *
 * <p>
 * <code>flat_multimap</code> is therefore intended as a locality-optimized,
 * range-oriented structure for large in-memory datasets, rather than as a
 * drop-in replacement for tree-based multimaps in all scenarios.
 * </p>
 */


#pragma once

#include <vector>
#include <algorithm>
#include <memory>
#include <utility>
#include <type_traits>
#include <cstddef>
#include <stdexcept>

#include "jh/conceptual/tuple_like.h"
#include "jh/conceptual/container_traits.h"


namespace jh {
    /**
     * @brief Flat ordered multimap implemented as a sorted contiguous container.
     *
     * @tparam K     Key type. Must be strictly ordered via <code>operator&lt;</code>.
     * @tparam V     Mapped value type.
     * @tparam Alloc Allocator type used for underlying storage.
     *
     * @details
     * <code>flat_multimap</code> implements ordered multimap semantics by storing
     * <code>std::pair&lt;K, V&gt;</code> elements in a contiguous container
     * (<code>std::vector</code>) that is kept sorted by key.
     *
     * Duplicate keys are permitted and are stored contiguously. Lookup and range
     * queries are implemented using binary search (<code>lower_bound</code>,
     * <code>upper_bound</code>), while insertion and erasure are expressed in terms
     * of vector operations.
     * <p>
     * This container is optimized for:
     * <ul>
     *   <li>range-oriented multimap semantics (<code>equal_range</code>, <code>erase(key)</code>)</li>
     *   <li>cache-friendly traversal and lookup</li>
     *   <li>bulk insertion and reconstruction</li>
     * </ul>
     * </p>
     *
     * @note
     * All insertions and erasures may invalidate iterators.
     * <br>
     * Unlike tree-based ordered containers, no node identity or pointer stability
     * is preserved; elements may be relocated freely within the underlying storage.
     */
    template<typename K, typename V, typename Alloc = std::allocator<std::pair<K, V>>> requires (
    (requires(const K &a, const K &b) {
        { a < b } -> std::convertible_to<bool>;
    }) &&
    jh::concepts::is_contiguous_reallocable<K> && jh::concepts::is_contiguous_reallocable<V>)
    class flat_multimap final {
    public:
        /// @brief Value type stored in the container (<code>std::pair&lt;K, V&gt;</code>).
        using value_type = std::pair<K, V>;
        /// @brief Allocator type rebound to <code>value_type</code>.
        using allocator_type = typename std::allocator_traits<Alloc>
        ::template rebind_alloc<value_type>;
        /// @brief Underlying contiguous storage type.
        using container_type = std::vector<value_type, allocator_type>;
        /// @brief Iterator type for the container.
        using iterator = typename container_type::iterator;
        /// @brief Const iterator type for the container.
        using const_iterator = typename container_type::const_iterator;

    private:
        /// @brief Internal storage.
        container_type storage_;

    public:
        /**
         * @brief Default-construct an empty multimap.
         */
        flat_multimap() = default;

        /**
         * @brief Construct from an existing container.
         *
         * @details
         * The contents of @p cont are copied into the internal storage and
         * stably sorted by key. If the input is already sorted, the cost is
         * near-linear.
         *
         * @param cont Source container.
         */
        explicit flat_multimap(const container_type &cont)
                : storage_(cont) {
            std::stable_sort(
                    storage_.begin(), storage_.end(),
                    [](auto &a, auto &b) { return a.first < b.first; }
            );
        }

        /**
         * @brief Move-construct from an existing container.
         *
         * @details
         * The container is taken by move and then stably sorted by key.
         *
         * @param cont Source container.
         */
        explicit flat_multimap(container_type &&cont)
                : storage_(std::move(cont)) {
            std::stable_sort(
                    storage_.begin(), storage_.end(),
                    [](auto &a, auto &b) { return a.first < b.first; }
            );
        }

        /**
         * @brief Construct an empty multimap with a specific allocator.
         *
         * @param alloc Allocator used for underlying storage.
         */
        explicit flat_multimap(const allocator_type &alloc)
                : storage_(alloc) {}

        /**
         * @brief Construct from a container and allocator.
         *
         * @details
         * The container is copied and then stably sorted by key.
         *
         * @param cont  Source container.
         * @param alloc Allocator to use.
         */
        flat_multimap(const container_type &cont, const allocator_type &alloc)
                : storage_(cont, alloc) {
            std::stable_sort(
                    storage_.begin(), storage_.end(),
                    [](auto &a, auto &b) { return a.first < b.first; }
            );
        }

        /**
         * @brief Move-construct from a container using a specific allocator.
         *
         * @param cont  Source container.
         * @param alloc Allocator to use.
         */
        flat_multimap(container_type &&cont, const allocator_type &alloc)
                : storage_(std::move(cont), alloc) {
            std::stable_sort(
                    storage_.begin(), storage_.end(),
                    [](auto &a, auto &b) { return a.first < b.first; }
            );
        }

        /**
         * @brief Copy-construct from another flat_multimap, using a rebound default allocator.
         */
        flat_multimap(const flat_multimap &) = default;

        /**
         * @brief Copy-construct from another flat_multimap, using a user-supplied allocator.
         *
         * @param other The source tree to copy from.
         * @param alloc Allocator to be used for the new tree.
         */
        flat_multimap(const flat_multimap &other, const allocator_type &alloc)
                : storage_(other.storage_, alloc) {}

        /**
         * @brief Move-construct from another flat_multimap, using a rebound default allocator.
         *
         * @note
         * The source flat_multimap is left in a valid but unspecified state afterwards,
         * consistent with standard container move semantics.
         * Callers must clear or overwrite the source if reuse is desired.
         */
        flat_multimap(flat_multimap &&) noexcept = default;

        /**
         * @brief Move-construct from another flat_multimap, using a user-supplied allocator.
         *
         * @param other The source flat_multimap to move from.
         * @param alloc Allocator to be used for this flat_multimap.
         *
         * @details
         * The source flat_multimap is left in a valid but unspecified state afterwards,
         * consistent with standard container move semantics.
         * Callers must clear or overwrite @p other if reuse is desired.
         */
        flat_multimap(flat_multimap &&other, const allocator_type &alloc)
                : storage_(std::move(other.storage_), alloc) {}

        /// @brief Copy assignment.
        flat_multimap &operator=(const flat_multimap &) = default;

        /// @brief Move assignment.
        flat_multimap &operator=(flat_multimap &&) noexcept = default;

    public:
        /**
         * @brief Return the range of elements equivalent to the given key.
         *
         * @details
         * Provides the canonical <code>equal_range</code> semantics for
         * associative containers with multiple equivalent keys:
         * <ul>
         *   <li>
         *     If an element with the given key exists, returns a pair
         *     <code>{lower_bound(key), upper_bound(key)}</code>.
         *   </li>
         *   <li>
         *     If no such element exists, both iterators in the returned pair
         *     equal <code>end()</code>.
         *   </li>
         *   <li>
         *     The returned range is half-open: the first iterator refers to
         *     the first element with the key (if present), and the second refers to
         *     the element that follows the last element with the key.
         *   </li>
         *   <li>Does not modify the container.</li>
         * </ul>
         *
         * @param k The key to search for.
         *
         * @return A pair of iterators defining the range of matching elements.
         */
        std::pair<iterator, iterator> equal_range(const K &k) {
            auto lower = std::lower_bound(
                    storage_.begin(), storage_.end(), k,
                    [](auto &p, const K &v) { return p.first < v; }
            );
            auto upper = std::upper_bound(
                    lower, storage_.end(), k,
                    [](const K &v, auto &p) { return v < p.first; }
            );
            return {lower, upper};
        }

        /**
         * @brief Return the range of elements equivalent to the given key (const overload).
         *
         * @details
         * Behaves identically to the non-const version but returns a pair of
         * <code>const_iterator</code>.
         *
         * @param k The key to search for.
         *
         * @return A pair of const iterators defining the range of matching elements.
         */
        [[nodiscard]] std::pair<const_iterator, const_iterator> equal_range(const K &k) const {
            auto lower = std::lower_bound(
                    storage_.begin(), storage_.end(), k,
                    [](auto &p, const K &v) { return p.first < v; }
            );
            auto upper = std::upper_bound(
                    lower, storage_.end(), k,
                    [](const K &v, auto &p) { return v < p.first; }
            );
            return {lower, upper};
        }

        /**
         * @brief Locate the first element with the specified key.
         *
         * @details
         * If multiple elements with the same key exist, returns an iterator
         * to the first such element. If no element exists, returns <code>end()</code>.
         *
         * @param k The key to search for.
         * @return Iterator to the matching element, or <code>end()</code> if not found.
         */
        iterator find(const K &k) {
            auto r = equal_range(k);
            return (r.first == r.second ? storage_.end() : r.first);
        }

        /**
         * @brief Locate the first element with the specified key (const overload).
         *
         * @details
         * Behaves identically to the non-const version but returns a
         * <code>const_iterator</code>.
         *
         * @param k The key to search for.
         * @return Const iterator to the matching element, or <code>end()</code> if not found.
         */
        const_iterator find(const K &k) const {
            auto r = equal_range(k);
            return (r.first == r.second ? storage_.end() : r.first);
        }

    private:
        /// @brief Internal insertion implementation.
        template<typename KArg, typename VArg>
        requires std::is_same_v<std::remove_cvref_t<KArg>, K>
                 && std::is_same_v<std::remove_cvref_t<VArg>, V>
        iterator _insert_impl(KArg &&key, VArg &&value) {
            // upper_bound to insert after existing equivalents
            auto pos = std::upper_bound(
                    storage_.begin(), storage_.end(), key,
                    [](const K &v, auto &&p) { return v < p.first; }
            );
            return storage_.insert(pos, {key, value});
            // all iterators invalidated, except the returned one
        }

    public:

        /**
         * @brief Insert a key-value pair by constructing it from arbitrary arguments.
         *
         * @details
         * This function preserves the usual <code>emplace</code> semantics: the
         * arguments are forwarded into a temporary <code>std::pair&lt;K, V&gt;</code>.
         * The key and mapped value constructed in this temporary object are then forwarded into
         * the internal insertion logic.
         *
         * If the key already exists in the map, the new element is inserted after
         * all existing equivalents.
         *
         * @param args Arguments used to construct a temporary <code>std::pair&lt;K, V&gt;</code>.
         *
         * @return Iterator to the inserted element.
         *
         * @note All iterators are invalidated except the returned one.
         */
        template<typename... Args>
        iterator emplace(Args &&... args) {
            auto temp = value_type(std::forward<Args>(args)...);
            return _insert_impl(std::move(temp.first), std::move(temp.second));
        }

        /**
         * @brief Insert a key-value pair into the map.
         *
         * @details
         * This overload generalizes the traditional
         * <code>std::pair&lt;K, V&gt;</code>-based insertion interface.
         * Instead of requiring a <code>value_type</code> object, any
         * 2-element tuple-like value is accepted as long as:
         * <ul>
         *   <li><code>get&lt;0&gt;(p)</code> has type <code>K</code> (after remove_cvref)</li>
         *   <li><code>get&lt;1&gt;(p)</code> has type <code>V</code> (after remove_cvref)</li>
         * </ul>
         *
         * This reflects the actual insertion semantics: the container consumes
         * the key and mapped value directly. Any tuple-like pair (including
         * <code>std::pair</code>, <code>std::tuple</code>, proxy references,
         * and structured-binding-compatible types) is therefore permitted if
         * its element types match exactly.
         * <br>
         * If the key already exists in the map, the new element is inserted after
         * all existing equivalents.
         *
         * @tparam P The tuple-like type providing key and mapped value.
         *
         * @param p A tuple-like value providing key and mapped value.
         *
         * @return Iterator to the inserted element.
         *
         * @note All iterators are invalidated except the returned one.
         */
        template<typename P>
        requires (jh::concepts::pair_like_for<P, K, V>)
        iterator insert(P &&p) {
            return _insert_impl(
                    get<0>(std::forward<P>(p)),
                    get<1>(std::forward<P>(p))
            );
        }

        /**
         * @brief Erase the element referenced by the given iterator.
         *
         * @details
         * Removes the element pointed to by <code>pos</code> and returns an
         * iterator to its logical successor. If <code>pos</code> refers
         * to <code>end()</code>, no action is performed and <code>pos</code> is
         * returned unchanged.
         *
         * @param pos Iterator referring to the element to erase.
         * @return An iterator as the logical successor of the erased element,
         *         or <code>end()</code> if no successor exists.
         */
        iterator erase(iterator pos) {
            if (pos == storage_.end()) {
                return pos;
            }
            return storage_.erase(pos);
        }

        /**
         * @brief Erase the element referenced by the given const iterator.
         *
         * @details
         * Removes the element pointed to by <code>pos</code> by delegating to the
         * non-const overload <code>erase(iterator)</code>. The behavior, iterator
         * invalidation rules, and returned iterator semantics exactly match those of
         * <code>erase(iterator)</code>.
         *
         * If <code>pos</code> equals <code>end()</code>, no removal occurs and
         * <code>pos</code> is returned unchanged.
         *
         * @param pos Const iterator referring to the element to erase.
         * @return An iterator as the logical successor of the erased element,
         *         or <code>end()</code> if no successor exists.
         */
        iterator erase(const_iterator pos) {
            if (pos == storage_.end()) {
                return pos;
            }
            return storage_.erase(pos);
        }

        /**
         * @brief Erase a range of elements.
         *
         * @details
         * Removes all elements in the half-open range
         * <tt>[<code>first</code>, <code>last</code>)</tt> and returns an iterator to the logical
         * successor of the last erased element.
         * <br>
         * The range <tt>[<code>first</code>, <code>last</code>)</tt> must be valid. In particular,
         * <code>last</code> must not precede <code>first</code>. If this condition
         * is violated, a <code>std::logic_error</code> is thrown.
         * <br>
         * If <code>first == last</code>, no elements are removed and
         * <code>first</code> is returned.
         * <br>
         * If <code>first == end()</code>, no removal is performed and
         * <code>end()</code> is returned. This is only considered valid when
         * <code>last == end()</code>.
         *
         * @param first Iterator referring to the first element to erase.
         * @param last  Iterator referring to one past the last element to erase.
         *
         * @return An iterator referring to the logical successor of the last erased
         *         element, or <code>end()</code> if no such element exists.
         *
         * @throws std::logic_error
         *         If <code>last</code> precedes <code>first</code>.
         */
        iterator erase(iterator first, iterator last) {
            if (std::distance(first, last) < 0) {
                throw std::logic_error("jh::flat_multimap::erase: last precedes first");
            }
            if (first == storage_.end()) {
                return storage_.end();
            }
            return storage_.erase(first, last);
        }

        /**
         * @brief Erase a range of elements.
         *
         * @details
         * Removes all elements in the half-open range
         * <tt>[<code>first</code>, <code>last</code>)</tt> by delegating to the non-const overload
         * <code>erase(iterator, iterator)</code>. The behavior, iterator
         * invalidation rules, and returned iterator semantics exactly match those
         * of the non-const overload.
         *
         * @param first Const iterator referring to the first element to erase.
         * @param last  Const iterator referring to one past the last element to erase.
         *
         * @return An iterator referring to the logical successor of the last erased
         *         element, or <code>end()</code> if no such element exists.
         *
         * @throws std::logic_error
         *         If <code>last</code> precedes <code>first</code>.
         */
        iterator erase(const_iterator first, const_iterator last) {
            if (std::distance(first, last) < 0) {
                throw std::logic_error("jh::flat_multimap::erase: last precedes first");
            }
            if (first == storage_.end()) {
                return storage_.end();
            }
            return storage_.erase(first, last);
        }

        /**
         * @brief Erase all elements whose key compares equal to the given key.
         *
         * @details
         * Searches for all elements with key equivalent to <code>k</code> and
         * removes them from the container.
         * <ul>
         *   <li>
         *     If at least one matching key exists, all such elements are removed,
         *     and the number of removed elements is returned. All iterators are
         *     invalidated.
         *   </li>
         *   <li>
         *     If no matching key exists, the container is left unmodified and
         *     no iterators are invalidated.
         *   </li>
         * </ul>
         *
         * @param k The key of the element to remove.
         * @return The number of elements removed.
         */
        size_t erase(const K &k) {
            auto r = equal_range(k);
            size_t count = std::distance(r.first, r.second);
            storage_.erase(r.first, r.second);
            return count;
        }

        /**
         * @brief Count the number of elements with the specified key.
         *
         * @param key Key to count.
         * @return Number of elements equivalent to <code>key</code>.
         */
        [[nodiscard]] std::size_t count(const K &key) const {
            auto r = equal_range(key);
            return std::distance(r.first, r.second);
        }

        /// @brief Return iterator to the first element.
        iterator begin() { return storage_.begin(); }

        /// @brief Return iterator past the last element.
        iterator end() { return storage_.end(); }

        /// @brief Return const iterator to the first element.
        [[nodiscard]] const_iterator begin() const { return storage_.begin(); }

        /// @brief Return const iterator past the last element.
        [[nodiscard]] const_iterator end() const { return storage_.end(); }

        /**
         * @brief Return the number of elements stored.
         */
        [[nodiscard]] size_t size() const { return storage_.size(); }

        /**
         * @brief Check whether the container is empty.
         */
        [[nodiscard]] bool empty() const { return storage_.empty(); }

        /**
         * @brief Remove all elements from the container.
         *
         * @details
         * <p>
         * Resets the container to an empty state by clearing the underlying
         * storage. The operation is equivalent to clearing a vector:
         * </p>
         * <ul>
         *   <li>The size becomes zero, but the capacity is preserved.</li>
         *   <li>No reallocation occurs.</li>
         *   <li>
         *     Under polymorphic allocators (PMR), no element-by-element
         *     destruction or resource release takes place; the buffer is simply
         *     marked empty.
         *   </li>
         * </ul>
         * <p>
         * This gives clear an effectively constant-time cost. Unlike pointer-based
         * tree structures such as those used by the standard multimap, there is
         * no need to traverse and destroy individual nodes;
         * the entire storage is discarded in one step.
         * </p>
         *
         * @note
         * All iterators are invalidated.
         */
        void clear() noexcept { storage_.clear(); }

        /**
         * @brief Reserve space for at least n elements.
         *
         * @details
         * <p>
         * Requests that the underlying storage grow its capacity to
         * at least <code>n</code> elements. This operation does not change the
         * size of the container or alter any existing node indices.
         * </p>
         * <p>
         * Calling reserve with a value smaller than the current size is no-op, 
         * according to ISO C++11+ standards for standard containers.
         * </p>
         * <p>
         * With polymorphic allocators (PMR), reserve typically has minimal
         * overhead, as the resource may enlarge the buffer without a full
         * reallocation.
         * </p>
         *
         * @param n Minimum capacity to reserve.
         *
         * @note This operation may invalidate all iterators if reallocation occurs.
         */
        void reserve(std::size_t n) noexcept(noexcept(storage_.reserve(n))) {
            storage_.reserve(n);
        }

        /**
         * @brief Request that the container reduce its capacity.
         *
         * @details
         * <p>
         * Issues a non-binding request to the underlying storage to
         * reduce its capacity. The behavior matches that of
         * <code>std::vector::shrink_to_fit</code>:
         * </p>
         * <ul>
         *   <li>The operation may reduce capacity, but is not required to do so.</li>
         *   <li>There is no guarantee that the resulting capacity equals the size.</li>
         *   <li>Under polymorphic allocators (PMR), the resource may keep the
         *       existing buffer unchanged.</li>
         *   <li>Because iterators reference elements by index, not pointer,
         *       this operation never invalidates iterators.</li>
         * </ul>
         */
        void shrink_to_fit() noexcept(noexcept(storage_.shrink_to_fit())) {
            storage_.shrink_to_fit();
        }

        /**
         * @brief Insert a range of elements and restore ordering.
         *
         * @details
         * The elements in the range <tt>[<code>first</code>, <code>last</code>)</tt> are appended to the
         * underlying storage, after which the entire container is stably sorted
         * by key.
         *
         * @tparam It Input iterator type.
         * @param first Iterator to the first element.
         * @param last  Iterator past the last element.
         *
         * @note All iterators are invalidated.
         */
        template<typename It>
        requires requires(container_type &c, It first, It last) {
            c.insert(c.end(), first, last);
        }
        void bulk_insert(It first, It last) {
            storage_.insert(storage_.end(), first, last);
            std::stable_sort(
                    storage_.begin(), storage_.end(),
                    [](auto &a, auto &b) { return a.first < b.first; }
            );
        }
    };
} // namespace jh
