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
 * @file ordered_map.h
 * @brief Contiguous AVL-based ordered container with fragmentation-free semantics.
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 *
 * <h3>Overview</h3>
 * <p>
 * <code>jh::ordered_set&lt;K&gt;</code> and <code>jh::ordered_map&lt;K,V&gt;</code>
 * implement a <b>contiguous AVL tree</b> stored inside a <code>std::vector</code>
 * (or PMR-aware vector), eliminating pointer chasing and reducing allocator
 * fragmentation. Nodes are referred to by <b>indices</b> instead of pointers,
 * enabling stable, cache-friendly traversal and compact memory layout.
 * </p>
 *
 * <p>
 * These containers are <b>not intended as a full replacement</b> for
 * <code>std::set</code> or <code>std::map</code>, but serve as a
 * <b>predictable, fragmentation-free, and locality-optimized</b> alternative
 * for workloads where:
 * </p>
 * <ul>
 *   <li>memory fragmentation must be controlled,</li>
 *   <li>allocator churn is expensive or unstable,</li>
 *   <li>iterators need predictable traversal cost,</li>
 *   <li>large-scale workloads favor cache locality,</li>
 *   <li>PMR-based pooling is desired for <tt>O(1)</tt> mass-clear operations.</li>
 * </ul>
 *
 * <h3>Design Goals</h3>
 * <ul>
 *   <li>Minimize fragmentation by storing all nodes in a contiguous vector.</li>
 *   <li>Provide stable, predictable erasure via compactification.</li>
 *   <li>Offer STL-like API semantics (find, lower_bound, iteration, erase).</li>
 *   <li>Exploit cache locality by avoiding pointer-heavy RB-tree structures.</li>
 *   <li>Ensure deterministic <code>clear()</code> behavior under PMR.</li>
 * </ul>
 *
 * <h3>Internal Storage Model</h3>
 * <p>
 * Nodes are stored as <b>AVL nodes</b> in a contiguous vector:
 * </p>
 *
 * @code
 * struct node {
 *     store_t stored_; // std::pair&lt;K, V&gt; for map or K for set
 *     size_t parent, left, right;
 *     uint16_t height;
 * };
 * @endcode
 *
 * <ul>
 *   <li>No node is heap-allocated individually.</li>
 *   <li>Index references remain valid except for the erased node itself.</li>
 *   <li>Erase compacts the last node into the erased node's slot.</li>
 *   <li>AVL rotation works on indices instead of pointers.</li>
 * </ul>
 *
 * <h3>Comparison vs <code>std::set</code> / <code>std::map</code></h3>
 *
 * <table>
 *   <tr>
 *     <th>Aspect</th><th><code>std::set</code> <code>std::map</code></th><th><code>jh::ordered_set</code> <code>ordered_map</code></th>
 *   </tr>
 *   <tr>
 *     <td>Node layout</td><td>pointer-linked RB-tree</td><td>contiguous AVL (vector)</td>
 *   </tr>
 *   <tr>
 *     <td>Fragmentation</td><td>high (many small allocations)</td><td>minimal (1 vector buffer)</td>
 *   </tr>
 *   <tr>
 *     <td>Iterator stability</td><td>stable</td><td>stable except erased node</td>
 *   </tr>
 *   <tr>
 *     <td>Erase cost</td><td><tt>O(log N)</tt></td><td><tt>O(log N)</tt> + compactification</td>
 *   </tr>
 *   <tr>
 *     <td>Traversal locality</td><td>poor (pointer chasing)</td><td>excellent</td>
 *   </tr>
 *   <tr>
 *     <td>PMR <code>clear()</code></td><td>deep node destruction</td><td><tt>O(1)</tt>, vector reset</td>
 *   </tr>
 *   <tr>
 *     <td>For >5k elements</td><td>stable but noisy</td><td>predictable, low jitter</td>
 *   </tr>
 * </table>
 *
 * <h3>Rationale &mdash; Why It Exists</h3>
 * <p>
 * Modern allocators suffer from fragmentation under workloads that frequently
 * construct and destroy many tree nodes (e.g., dynamic indexing, routing tables,
 * message subscription graphs). The standard containers rely on one allocation
 * per node, causing:
 * </p>
 *
 * <ul>
 *   <li>allocator churn,</li>
 *   <li>TLB pressure and cache misses,</li>
 *   <li>unpredictable latency spikes,</li>
 *   <li>free-list poisoning in long-running systems.</li>
 * </ul>
 *
 * <p>
 * The contiguous AVL model eliminates these issues by placing all nodes into
 * a single dynamic buffer. Erasing a node does not free any memory; it simply
 * moves the last node into the removed slot. Combined with a monotonic-buffer
 * resource, <code>clear()</code> becomes nearly <tt>O(1)</tt>.
 * </p>
 *
 * <h3>About Performance</h3>
 *
 * <p>
 * Benchmarks (Apple M3, LLVM clang++20, 2025) show stable behavior across
 * 5 000-1 000 000 elements:
 * </p>
 *
 * <table>
 *   <tr>
 *     <th>Operation</th>
 *     <th><code>std::set/map</code></th>
 *     <th><code>jh::ordered_set/map</code></th>
 *     <th>Notes</th>
 *   </tr>
 *   <tr>
 *     <td>Random insert</td>
 *     <td>fast start, large jitter</td>
 *     <td>&asymp; 10-40% overhead but small jitter</td>
 *     <td>AVL maintenance but contiguous memory</td>
 *   </tr>
 *   <tr>
 *     <td>Ordered insert</td>
 *     <td>degenerates (right-heavy RB)</td>
 *     <td>consistently faster</td>
 *     <td>vector locality dominates</td>
 *   </tr>
 *   <tr>
 *     <td>Random lookup</td>
 *     <td>stable</td>
 *     <td>comparable or slightly faster</td>
 *     <td>branchless traversal & locality</td>
 *   </tr>
 *   <tr>
 *     <td>Iteration</td>
 *     <td>pointer chasing</td>
 *     <td>&asymp; 15-30% faster</td>
 *     <td>sequential memory</td>
 *   </tr>
 *   <tr>
 *     <td>Erase</td>
 *     <td>stable</td>
 *     <td>slightly slower worst-case</td>
 *     <td>compacting cost</td>
 *   </tr>
 *   <tr>
 *     <td>Clear (PMR)</td>
 *     <td><tt>O(N)</tt> destruct</td>
 *     <td><tt>O(1)</tt></td>
 *     <td>pool discard</td>
 *   </tr>
 * </table>
 *
 * <h4>Observed Behavior in Large Datasets</h4>
 * <ul>
 *   <li>For 100k-1M string keys, performance gap tightens to within ~10%.</li>
 *   <li>For fully ordered input, <code>ordered_set</code> often surpasses std::set.</li>
 *   <li>Lookup variance is consistently lower due to contiguous cachelines.</li>
 *   <li>Iteration is measurably faster at all scales.</li>
 * </ul>
 *
 * <h3>Memory & Fragmentation Notes</h3>
 * <ul>
 *   <li>No per-node allocation &rarr; extremely low fragmentation.</li>
 *   <li><code>erase()</code> never frees memory.</li>
 *   <li><code>clear()</code> under PMR is almost zero-cost.</li>
 *   <li>Ideal for systems where pointers must not be invalidated by allocators.</li>
 *   <li>Much more stable than pointer-based trees under long uptimes.</li>
 * </ul>
 *
 * <h3>Limitations</h3>
 * <ul>
 *   <li>Iterators are invalidated by <code>erase()</code> except the returned one.</li>
 *   <li>Does not provide node-hint insertion APIs (unlike <code>std::map::insert(hint)</code>).</li>
 *   <li>Erase requires compactification (copy/move of last node).</li>
 *   <li>Not designed for persistent node references.</li>
 * </ul>
 *
 * <h3>Use Cases</h3>
 * <ul>
 *   <li>Memory-fragmentation-sensitive systems (game engines, GUI trees, routing).</li>
 *   <li>Real-time components requiring predictable latency.</li>
 *   <li>Systems with massive clear/repoulate cycles using PMR.</li>
 *   <li>Large ordered indexes requiring sequential iteration.</li>
 * </ul>
 *
 * <h3>Complexity Summary</h3>
 * <ul>
 *   <li>Insert: <tt>O(log N)</tt></li>
 *   <li>Erase: <tt>O(log N)</tt> + <tt>O(1)</tt> compact</li>
 *   <li>Find: <tt>O(log N)</tt></li>
 *   <li>Traversal: <tt>O(N)</tt>, cache-friendly</li>
 * </ul>
 *
 * @version <pre>1.4.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include <concepts>
#include <cstddef>
#include <utility>
#include <cstdint>
#include <iostream>
#include <type_traits>
#include <iterator>
#include <stdexcept>
#include <ranges>
#include <vector>

#include "jh/typing/monostate.h"
#include "jh/conceptual/tuple_like.h"

/**
 * @brief Internal AVL-tree implementation namespace for
 * <code>jh::ordered_set</code> and <code>jh::ordered_map</code>.
 *
 * This namespace contains the low-level, index-based AVL tree
 * machinery used to implement the contiguous ordered containers.
 * It is an internal implementation detail and is not intended
 * for direct use.
 *
 * @see jh::ordered_set
 * @see jh::ordered_map
 */
namespace jh::avl {

    /**
     * @brief Node element for the contiguous AVL tree.
     *
     * @tparam K key type (strictly ordered)
     * @tparam V mapped value type
     *
     * @details
     * Represents a single AVL node stored inside a contiguous vector.
     * Links are expressed as indices rather than pointers. Each node
     * contains a key-value pair, parent/left/right indices, and an
     * explicit height field used for balancing. No per-node allocation
     * occurs; nodes are relocated only during erase compactification.
     */
    template<typename K, typename V> requires ((requires(const K &a, const K &b) {
        { a < b } -> std::convertible_to<bool>;
    }) && std::copy_constructible<K> && std::copy_constructible<V>)
    struct avl_node {
        /// @brief Key type stored by this node.
        using key_type = K;

        /// @brief Mapped value type stored by this node.
        using value_type = V;

        /// @brief Internal storage type (key/value pair).
        using store_t = std::pair<K, V>;

        /// @brief Container for the key/value payload.
        store_t stored_;

        /// @brief Returns key (const reference).
        [[nodiscard]] const key_type &key() const {
            return stored_.first;
        }

        /// @brief Returns mapped value (reference).
        value_type &value() {
            return stored_.second;
        }

        /// @brief Returns full stored pair (reference).
        store_t &stored() {
            return stored_;
        }

        /// @brief Returns full stored pair (const reference).
        [[nodiscard]] const store_t &stored() const {
            return stored_;
        }

        /// @brief Index of parent node, or -1 for root.
        [[maybe_unused]] std::size_t parent;

        /// @brief Index of left child, or -1 if none.
        std::size_t left;

        /// @brief Index of right child, or -1 if none.
        std::size_t right;

        /// @brief Height of this node in the AVL tree.
        std::uint16_t height;

        /// @brief Default constructor (value-initializes storage and clears links).
        avl_node() requires std::default_initializable<std::pair<K, V>>
                : stored_(),
                  parent(static_cast<std::size_t>(-1)),
                  left(static_cast<std::size_t>(-1)),
                  right(static_cast<std::size_t>(-1)),
                  height(0) {}

        /**
         * @brief Construct a node by forwarding a key and a mapped value.
         *
         * @tparam KK  A cv/ref-qualified form of <code>K</code>.
         * @tparam VV  A cv/ref-qualified form of <code>V</code>.
         *
         * @param k    Key forwarded into the stored key.
         * @param v    Mapped value forwarded into the stored value.
         * @param parent_index  Parent index of this node.
         * @param left_index    Left-child index of this node.
         * @param right_index   Right-child index of this node.
         * @param h             Initial height of the node.
         */
        template<typename KK, typename VV>
        requires std::is_same_v<std::remove_cvref_t<KK>, K> &&
                 std::is_same_v<std::remove_cvref_t<VV>, V>
        [[maybe_unused]] avl_node(KK &&k, VV &&v,
                                  std::size_t parent_index = static_cast<std::size_t>(-1),
                                  std::size_t left_index = static_cast<std::size_t>(-1),
                                  std::size_t right_index = static_cast<std::size_t>(-1),
                                  std::uint16_t h = 1)
                : stored_(std::forward<KK>(k), std::forward<VV>(v)),
                  parent(parent_index),
                  left(left_index),
                  right(right_index),
                  height(h) {}

        /// @brief Copy constructor.
        avl_node(const avl_node &) = default;

        /// @brief Copy assignment.
        avl_node &operator=(const avl_node &) = default;

        /// @brief Move constructor.
        avl_node(avl_node &&) = default;

        /// @brief Move assignment.
        avl_node &operator=(avl_node &&) = default;

        /// @brief Destructor.
        ~avl_node() = default;
    };

    /**
     * @brief Node element for set-style contiguous AVL trees.
     *
     * @tparam K key type (strictly ordered)
     *
     * @details
     * Specialization used by ordered-set semantics. Stores only the key
     * and maintains identical AVL linkage fields (parent/left/right/height).
     * Provides no mapped value; the node's identity is its key. Layout and
     * balancing behavior remain consistent with the general node template.
     */
    template<typename K> requires ((requires(const K &a, const K &b) {
        { a < b } -> std::convertible_to<bool>;
    }) && std::copy_constructible<K>)
    struct avl_node<K, jh::typed::monostate> {
        /// @brief Key type stored by this node.
        using key_type = K;

        /// @brief Dummy mapped type to preserve template uniformity.
        using value_type = jh::typed::monostate;

        /// @brief Internal storage type (key only).
        using store_t = K;

        /// @brief Key payload.
        store_t stored_;

        /// @brief Returns key (const reference).
        [[nodiscard]] const key_type &key() const {
            return stored_;
        }

        /// @brief Returns stored key (const reference).
        [[nodiscard]] const store_t &stored() const {
            return stored_;
        }

        /// @brief Index of parent node, or -1 for root.
        [[maybe_unused]] std::size_t parent;

        /// @brief Index of left child, or -1 if none.
        std::size_t left;

        /// @brief Index of right child, or -1 if none.
        std::size_t right;

        /// @brief Height of this node in the AVL tree.
        std::uint16_t height;

        /// @brief Default constructor (value-initializes key and clears links).
        avl_node() requires std::default_initializable<K>
                : stored_(),
                  parent(static_cast<std::size_t>(-1)),
                  left(static_cast<std::size_t>(-1)),
                  right(static_cast<std::size_t>(-1)),
                  height(0) {}

        /**
         * @brief Construct a set-style node by forwarding only the key.
         *
         * @tparam KK  A cv/ref-qualified form of <code>K</code>.
         * @tparam VV  Must be a cv/ref-qualified <code>monostate</code> (placeholder).
         *
         * @param k    Key forwarded into the stored key.
         * @param parent_index  Parent index of this node.
         * @param left_index    Left-child index of this node.
         * @param right_index   Right-child index of this node.
         * @param h             Initial height of the node.
         */
        template<typename KK, typename VV>
        requires std::is_same_v<std::remove_cvref_t<KK>, K> &&
                 std::is_same_v<std::remove_cvref_t<VV>, jh::typed::monostate>
        [[maybe_unused]] avl_node(KK &&k, VV &&,
                                  std::size_t parent_index = static_cast<std::size_t>(-1),
                                  std::size_t left_index = static_cast<std::size_t>(-1),
                                  std::size_t right_index = static_cast<std::size_t>(-1),
                                  std::uint16_t h = 1)
                : stored_(std::forward<KK>(k)),
                  parent(parent_index),
                  left(left_index),
                  right(right_index),
                  height(h) {}

        /// @brief Copy constructor.
        avl_node(const avl_node &) = default;

        /// @brief Copy assignment.
        avl_node &operator=(const avl_node &) = default;

        /// @brief Move constructor.
        avl_node(avl_node &&) = default;

        /// @brief Move assignment.
        avl_node &operator=(avl_node &&) = default;

        /// @brief Destructor.
        ~avl_node() = default;
    };

    namespace detail {

        /// @brief Selects canonical value type: <code>K</code> for set, <code>std::pair&lt;const K, V&gt;</code>for map.
        template<typename K, typename V>
        using value_t =
                std::conditional_t<
                        jh::typed::monostate_t<std::remove_cvref_t<V>>,
                        std::remove_cvref_t<K>,
                        std::pair<
                                const std::remove_cvref_t<K>,
                                std::remove_cvref_t<V>
                        >
                >;

        /// @brief Canonical node type after removing cv/ref from <code>K</code> and <code>V</code>.
        template<typename K, typename V>
        using node_t = avl_node<std::remove_cvref_t<K>, std::remove_cvref_t<V>>;

        /// @brief Underlying storage type (<code>K</code> or <code>std::pair&lt;K, V&gt;</code>) extracted from node_t.
        template<typename K, typename V>
        using base_t = node_t<K, V>::store_t;

        /// @brief Allocator rebound to the canonical node type.
        template<typename K, typename V, typename Alloc>
        using node_alloc_type =
                typename std::allocator_traits<Alloc>
                ::template rebind_alloc<node_t<K, V>>;

        /// @brief Vector of canonical node types with corresponding allocator.
        template<typename K, typename V, typename Alloc>
        using node_vector_type =
                std::vector<node_t<K, V>, node_alloc_type<K, V, Alloc>>;

        /// @brief Reference type: const for set, mutable for map.
        template<typename K, typename V>
        using reference_t =
                std::conditional_t<
                        jh::typed::monostate_t<V>,
                        const base_t<K, V> &,
                        base_t<K, V> &
                >;

        /// @brief Pointer type: const for set, mutable for map.
        template<typename K, typename V>
        using pointer_t =
                std::conditional_t<
                        jh::typed::monostate_t<V>,
                        const base_t<K, V> *,
                        base_t<K, V> *
                >;

        /// @brief Concept ensuring two node types share the same canonical form.
        template<typename K, typename V, typename K_, typename V_>
        concept compatible_node_type =
        std::same_as<
                detail::node_t<K, V>,
                detail::node_t<K_, V_>
        >;

    } // namespace detail

    /**
     * @brief Contiguous-array AVL tree used by <code>jh::ordered_map</code> and <code>jh::ordered_set</code>.
     *
     * <h3>Overview</h3>
     * <p>
     * <code>jh::avl::tree_map&lt;K, V, Alloc&gt;</code> is the underlying container powering
     * <code>jh::ordered_map</code> and <code>jh::ordered_set</code>. It implements a
     * <b>contiguous-array AVL tree</b> in which all nodes are stored inside a single
     * dynamic buffer (typically <code>std::vector</code> or a PMR-aware equivalent).
     * Node linkage uses <b>indices</b> instead of pointers, enabling relocatable,
     * fragmentation-free storage with excellent cache locality.
     * </p>
     *
     * <h3>Purpose &amp; Design Philosophy</h3>
     * <p>
     * This structure aims to provide an STL-like ordered associative container with
     * performance and predictability guarantees that are difficult to achieve using
     * the traditional node-based red-black tree employed by <code>std::map</code> and
     * <code>std::set</code>.
     * It is not intended as a drop-in replacement, but rather a complementary tool
     * focused on:
     * </p>
     *
     * <ul>
     *   <li><b>Engineering stability</b> in long-running systems</li>
     *   <li><b>Zero fragmentation</b> through contiguous storage</li>
     *   <li><b>Predictable latency</b> with no per-node allocations</li>
     *   <li><b>High traversal speed</b> due to cache-friendly layout</li>
     *   <li><b><tt>O(1)</tt> <code>clear()</code></b> behavior, especially with PMR resources</li>
     *   <li><b>Optional <tt>O(N)</tt> construction</b> from strictly sorted, unique input</li>
     * </ul>
     *
     * <p>
     * Unlike the standard library's node-based trees, <code>tree_map</code>
     * behaves partially like a <code>std::vector</code>: it exposes
     * <code>reserve()</code>, <code>shrink_to_fit()</code>, and provides a
     * <code>clear()</code> operation that simply resets the vector and does not
     * deallocate individual nodes. This makes large-scale clearing and repopulation
     * extremely efficient and stable under PMR allocators.
     * </p>
     *
     * <h3>Performance Notes</h3>
     * <p>
     * This contiguous-array AVL tree has distinct performance behavior depending
     * on whether the workload is <b>insertion-heavy</b> or <b>access-heavy</b>.
     * </p>
     *
     * <h4>Insertion / Construction Cost</h4>
     * <p>
     * When constructing the tree via repeated <code>insert()</code> (i.e. the
     * equivalent of default-constructing and inserting N elements), AVL
     * rebalancing introduces a measurable overhead at small scales. For data
     * sets around 10&nbsp;k, construction is typically
     * <b>&asymp; 1.3-1.6&times;</b> the cost of <code>std::map</code>.
     * As N grows, this overhead rapidly diminishes due to contiguous storage and
     * negligible per-rotation cost; by 500&nbsp;k elements the difference is
     * only <b>~5-10%</b>, and by 1&nbsp;million elements insertion cost becomes
     * effectively comparable to <code>std::map</code>.
     * </p>
     *
     * <p>
     * For strictly sorted and unique input, <code>from_sorted()</code> bypasses
     * all rotations and achieves <b>near O(N)</b> construction, outperforming
     * any repeated-insert approach (including the standard library).
     * </p>
     *
     * <h4>Lookup / Traversal Cost</h4>
     * <p>
     * Access-related operations&mdash;<code>find()</code>, in-order traversal,
     * iteration&mdash;benefit strongly from contiguous memory layout and the smaller
     * height of AVL trees. Beyond ~5&nbsp;k elements, these operations are
     * consistently faster than <code>std::map</code>, and remain faster at every
     * larger scale tested. Across the 5&nbsp;k-1&nbsp;M range, typical speedups
     * fall in the <b>&asymp; 15-30% faster</b> range due to improved cache locality and
     * stable successor cost.
     * </p>
     *
     * <p>
     * Thus, although insertion may introduce some AVL-specific overhead at small
     * scales, access performance dominates for large workloads, making the
     * contiguous AVL structure an advantageous choice for systems where
     * traversal, lookup, or iteration cost is critical.
     * </p>
     *
     * <h3><tt>O(N)</tt> Construction</h3>
     * <p>
     * For strictly sorted, strictly unique input ranges,
     * <code>tree_map::from_sorted()</code> constructs a <b>perfectly balanced AVL
     * tree</b> in near-linear time with no rotations and no repeated comparisons.
     * This offers a fast, predictable path for bulk construction workflows.
     * </p>
     *
     * <h3>Iteration Cost</h3>
     * <p>
     * The tree produced by <code>from_sorted()</code> is a <b>perfectly balanced AVL</b>
     * laid out in a contiguous array. This structure has a remarkable property:
     * the average cost of advancing an in-order iterator (<code>operator++</code>)
     * is extremely close to a constant.
     * </p>
     *
     * <p>
     * For a perfectly balanced binary search tree with <i>N</i> nodes, the expected
     * number of pointer hops performed by <code>operator++</code> converges to:
     * </p>
     *
     * <pre><tt>
     *     E[successor steps] &rarr; 2.0  as N &rarr; &infin;
     * </tt></pre>
     *
     * <p>
     * This comes from the fact that more than half of all nodes are leaves, and
     * their successor is simply the parent. Only a vanishingly small fraction of
     * nodes require walking down a right subtree and then following several left
     * edges. Measured average successor cost <tt>[by python3.11]</tt> was:
     * </p>
     *
     * <table>
     *   <tr><th>N</th><th>avg successor steps</th></tr>
     *   <tr><td>10</td><td>1.70</td></tr>
     *   <tr><td>100</td><td>1.93</td></tr>
     *   <tr><td>1 000</td><td>1.99</td></tr>
     *   <tr><td>10 000</td><td>1.9987</td></tr>
     *   <tr><td>100 000</td><td>1.9998</td></tr>
     *   <tr><td>1 000 000</td><td>2.0000</td></tr>
     * </table>
     *
     * <p>
     * Therefore, in-order traversal runs in <b>strictly linear</b> time, with a very
     * small constant factor, and does not grow with the height of the tree.
     * </p>
     *
     * @tparam K    Key type (must be strictly ordered via <code>operator&lt;</code>)
     * @tparam V    Value type (use <code>jh::typed::monostate</code> for set semantics)
     * @tparam Alloc  Allocator for node storage; defaults to vector-compatible allocator
     */
    template<typename K, typename V,
            typename Alloc = std::allocator<detail::value_t<K, V>>>
    class tree_map final {
    private:
        /// @brief Allocator rebound to canonical node type.
        using alloc_type = detail::node_alloc_type<K, V, Alloc>;

        /// @brief Canonical node type for this map.
        using node_type = detail::node_t<K, V>;

    public:
        /// @brief Semantic allocator type as exposed to users, distinct from the internal node allocator.
        using allocator_type [[maybe_unused]] = typename std::allocator_traits<Alloc>
        ::template rebind_alloc<detail::value_t<K, V>>;

        /// @brief Vector storage type for all nodes.
        using vector_type = detail::node_vector_type<K, V, Alloc>;

    private:
        /// @brief Contiguous node pool storing the entire AVL tree.
        vector_type nodes_;

        /// @brief Root node index, or -1 if the tree is empty.
        std::size_t root = static_cast<std::size_t>(-1);

    public:
        /// @brief Allow cross-instantiation friendship for different K,V,Alloc parameterizations.
        template<typename, typename, typename>
        friend
        class tree_map;

        /**
         * @brief Default constructor creating an empty tree with default allocator.
         */
        tree_map() noexcept(noexcept(alloc_type()))
                : nodes_(alloc_type()), root(static_cast<std::size_t>(-1)) {}

        /**
         * @brief Construct an empty tree with a specified allocator.
         *
         * @param alloc allocator used for node storage
         */
        explicit tree_map(const Alloc &alloc)
                : nodes_(alloc_type(alloc)), root(static_cast<std::size_t>(-1)) {}

        /**
         * @brief Copy-construct from another tree whose canonical key/value
         *        types match this tree and whose allocator type is identical.
         *
         * @tparam K_ Original (possibly cv/ref qualified) key type of the source tree.
         * @tparam V_ Original (possibly cv/ref qualified) value type of the source tree.
         * @tparam Alloc_ Allocator type of the source tree.
         *
         * @param other The source tree to copy from.
         *
         * @details
         * Participates only when the canonical forms of <K,V> and <K_,V_> produce
         * the same internal node type. Thus, trees with different template
         * arguments may still be interoperable if their canonical types coincide.
         * Because the allocator types also match, the internal storage is copied
         * directly without rebinding.
         */
        template<typename K_, typename V_, typename Alloc_>
        requires detail::compatible_node_type<K, V, K_, V_> &&
                 std::same_as<Alloc, Alloc_>
        [[maybe_unused]] explicit tree_map(const tree_map<K_, V_, Alloc_> &other)
                : nodes_(other.nodes_),
                  root(other.root) {}

        /**
         * @brief Copy-construct from another tree whose canonical key/value
         *        types match this tree, using a rebound default allocator.
         *
         * @tparam K_ Original (possibly cv/ref qualified) key type of the source tree.
         * @tparam V_ Original (possibly cv/ref qualified) value type of the source tree.
         * @tparam Alloc_ Allocator type of the source tree.
         *
         * @param other The source tree to copy from.
         *
         * @details
         * Enabled only when the canonical forms of <K,V> and <K_,V_> match.
         * Because allocator types differ, the node buffer is copied into storage
         * owned by a default-constructed allocator rebound to this tree's node type.
         */
        template<typename K_, typename V_, typename Alloc_>
        requires detail::compatible_node_type<K, V, K_, V_> &&
                 (!std::same_as<Alloc, Alloc_>)
        [[maybe_unused]] explicit tree_map(const tree_map<K_, V_, Alloc_> &other)
                : nodes_(other.nodes_.begin(), other.nodes_.end(), alloc_type{}),
                  root(other.root) {}

        /**
         * @brief Copy-construct from another tree whose canonical key/value
         *        types match this tree, using a user-supplied allocator.
         *
         * @tparam K_ Original (possibly cv/ref qualified) key type of the source tree.
         * @tparam V_ Original (possibly cv/ref qualified) value type of the source tree.
         * @tparam Alloc_ Allocator type of the source tree.
         *
         * @param other The source tree to copy from.
         * @param alloc Allocator to be used for the new tree.
         *
         * @details
         * Enabled when canonical <K,V> and <K_,V_> denote the same node type.
         * The provided allocator determines the new node buffer; node contents
         * are copied from the source tree.
         */
        template<typename K_, typename V_, typename Alloc_>
        requires detail::compatible_node_type<K, V, K_, V_>
        [[maybe_unused]] tree_map(const tree_map<K_, V_, Alloc_> &other, const Alloc &alloc)
                : nodes_(other.nodes_.begin(), other.nodes_.end(), alloc_type(alloc)),
                  root(other.root) {}

        /**
         * @brief Move-construct from another tree whose canonical key/value
         *        types match this tree and whose allocator type is identical.
         *
         * @tparam K_   Source key type (possibly cv/ref qualified).
         * @tparam V_   Source value type (possibly cv/ref qualified).
         * @tparam Alloc_ Allocator type of the source tree.
         *
         * @param other The source tree to move from.
         *
         * @details
         * Enabled only when the canonical node types of <K,V> and <K_,V_> are
         * identical and both trees use the same allocator type.
         *
         * The underlying storage is transferred directly, producing a constant-time
         * move. After the operation, @p other remains in a valid but unspecified
         * state, consistent with the standard library's container move semantics.
         */
        template<typename K_, typename V_, typename Alloc_>
        requires detail::compatible_node_type<K, V, K_, V_> &&
                 std::same_as<Alloc, Alloc_>
        [[maybe_unused]] explicit tree_map(tree_map<K_, V_, Alloc_> &&other) noexcept
                : nodes_(std::move(other.nodes_)),
                  root(other.root) {
            other.root = static_cast<std::size_t>(-1);
        }

        /**
         * @brief Move-construct from another tree whose canonical key/value
         *        types match this tree, using a rebound default allocator.
         *
         * @tparam K_ Source key type (possibly cv/ref qualified).
         * @tparam V_ Source value type (possibly cv/ref qualified).
         * @tparam Alloc_ Allocator type of the source tree.
         *
         * @param other The source tree to move from.
         *
         * @details
         * Enabled when canonical forms of <K,V> and <K_,V_> match but the allocator
         * types differ. Node contents are moved element-wise into storage owned by a
         * default-constructed allocator rebound to this tree's node type.
         *
         * After the operation, @p other remains valid but unspecified. Its internal
         * structure is not guaranteed (it may be partially moved-from), and callers
         * should clear or reassign it before further use.
         */
        template<typename K_, typename V_, typename Alloc_>
        requires detail::compatible_node_type<K, V, K_, V_> &&
                 (!std::same_as<Alloc, Alloc_>)
        [[maybe_unused]] explicit tree_map(tree_map<K_, V_, Alloc_> &&other)
                : nodes_(
                std::make_move_iterator(other.nodes_.begin()),
                std::make_move_iterator(other.nodes_.end()),
                alloc_type{}
        ),
                  root(other.root) {
            other.root = static_cast<std::size_t>(-1);
        }

        /**
         * @brief Move-construct from another tree whose canonical key/value
         *        types match this tree, using a user-supplied allocator.
         *
         * @tparam K_ Source key type (possibly cv/ref qualified).
         * @tparam V_ Source value type (possibly cv/ref qualified).
         * @tparam Alloc_ Allocator type of the source tree.
         *
         * @param other The source tree to move from.
         * @param alloc Allocator to be used for this tree.
         *
         * @details
         * Enabled when canonical node types match. Elements are moved into storage
         * controlled by the supplied allocator. The source tree is left in a valid
         * but unspecified state afterwards, consistent with standard container
         * move semantics. Callers must clear or overwrite @p other if reuse is
         * desired.
         */
        template<typename K_, typename V_, typename Alloc_>
        requires detail::compatible_node_type<K, V, K_, V_>
        [[maybe_unused]] tree_map(tree_map<K_, V_, Alloc_> &&other, const Alloc &alloc)
                : nodes_(
                std::make_move_iterator(other.nodes_.begin()),
                std::make_move_iterator(other.nodes_.end()),
                alloc_type(alloc)
        ),
                  root(other.root) {
            other.root = static_cast<std::size_t>(-1);
        }

        /// @brief Public value type (<code>K</code> for set, <code>std::pair&lt;const K, V&gt;</code> for map).
        using value_type = detail::value_t<K, V>;

        /**
         * @brief Bidirectional iterator providing in-order traversal.
         *
         * @details
         * Iterator validity follows standard associative-container semantics:
         *
         * <ul>
         *   <li>
         *     <code>erase(pos)</code> returns an iterator referring to the
         *     element that follows the erased one in sorted order. If no such
         *     element exists, the returned iterator is <code>end()</code>.
         *   </li>
         *
         *   <li>
         *     The returned iterator is the only one whose continued use is
         *     well-defined. All other iterators obtained before the erase
         *     operation must be considered invalid.
         *   </li>
         *
         *   <li>
         *     Insertions and erasures may relocate internal nodes to maintain
         *     contiguous storage. Therefore no iterator stability guarantees
         *     exist except for the iterator returned by <code>erase()</code>.
         *   </li>
         * </ul>
         *
         * Dereferencing a valid iterator yields the stored element. Increment
         * and decrement perform in-order successor/predecessor traversal with
         * amortized constant cost.
         */
        struct iterator final {
            /// @brief Owning tree type.
            using base_type = tree_map;

            /// @brief Canonical node type used internally.
            using node_type = detail::node_t<K, V>;

            /// @brief Stored value type (<code>K</code> for set, <code>std::pair&lt;const K,V&gt; for map).
            using stored_type [[maybe_unused]] = detail::base_t<K, V>;

            /// @brief Iterator concept tag (bidirectional).
            using iterator_concept = std::bidirectional_iterator_tag;

            /// @brief Iterator category for legacy algorithms.
            using iterator_category [[maybe_unused]] = iterator_concept;

            /// @brief Value type exposed by dereferencing.
            using value_type = detail::base_t<K, V>;

            /// @brief Reference type: const for set, mutable for map.
            using reference = detail::reference_t<K, V>;

            /// @brief Pointer type: const for set, mutable for map.
            using pointer = detail::pointer_t<K, V>;

            /// @brief Difference type used for iterator arithmetic.
            using difference_type = std::int64_t;

            /// @brief Owning tree instance (never nullptr except for default-constructed iterator).
            base_type *tree = nullptr;

            /// @brief Index of the referenced node, or <code>-1</code> for <code>end()</code>.
            std::size_t idx = static_cast<std::size_t>(-1);

            /**
             * @brief Default-construct an iterator in a singular state.
             *
             * @details
             * A default-constructed iterator does not refer to any tree and is
             * therefore singular. It must not be compared, incremented,
             * decremented, or dereferenced. The only valid operations on a
             * singular iterator are assignment and destruction.
             *
             * Note that this state is distinct from <code>end()</code>: an
             * end iterator is associated with a specific container and can be
             * decremented to obtain the last element, whereas a singular
             * iterator cannot participate in any traversal.
             */
            iterator() = default;

            /**
             * @brief Construct an iterator referring to a specific node index.
             *
             * @param t   Pointer to the owning tree.
             * @param i   Node index, or <code>-1</code> to represent <code>end()</code>.
             *
             * @details
             * This constructor is primarily used internally when producing
             * <code>begin()</code>, <code>end()</code>, and traversal results.
             * Users normally do not instantiate iterators directly.
             */
            iterator(base_type *t, std::size_t i) : tree(t), idx(i) {}

            /**
             * @brief Dereference the iterator and obtain the referenced value.
             *
             * @return Reference to the value stored at the current position.
             *
             * @details
             * Undefined behavior occurs if the iterator does not refer to a
             * valid element (e.g., it equals <code>end()</code>).
             */
            reference operator*() {
                return tree->nodes_[idx].stored();
            }

            /**
             * @brief Dereference the iterator and obtain a pointer to the value.
             *
             * @return Pointer to the referenced element.
             *
             * @details
             * Undefined behavior if the iterator equals <code>end()</code>.
             */
            pointer operator->() {
                return &tree->nodes_[idx].stored();
            }

            /**
             * @brief Const-qualified dereference.
             *
             * @return Const reference to the stored value.
             *
             * @details
             * Undefined behavior if the iterator equals <code>end()</code>.
             */
            reference operator*() const {
                return tree->nodes_[idx].stored();
            }

            /**
             * @brief Const-qualified pointer dereference.
             *
             * @return Pointer to the constant stored value.
             *
             * @details
             * Undefined behavior if the iterator equals <code>end()</code>.
             */
            pointer operator->() const {
                return &tree->nodes_[idx].stored();
            }

            /**
             * @brief Obtain the key associated with the current node.
             *
             * @return Constant reference to the key of the referenced element.
             *
             * @details
             * Provided as a convenience for callers that require direct access
             * to the ordering key. Undefined behavior if the iterator equals
             * <code>end()</code>.
             */
            [[nodiscard]] const node_type::key_type &key() const { return tree->nodes_[idx].key(); }

            /**
             * @brief Advance to the in-order successor.
             *
             * @details
             * Performs an in-order successor step. The behavior follows
             * standard bidirectional-iterator rules:
             *
             * <ul>
             *   <li>
             *     If the iterator is <code>end()</code>, incrementing leaves it
             *     unchanged.
             *   </li>
             *   <li>
             *     Otherwise, the iterator moves to the next element in sorted
             *     key order.
             *   </li>
             *   <li>
             *     Undefined behavior results if the iterator is in a singular
             *     state (e.g., default-constructed).
             *   </li>
             * </ul>
             *
             * @return Reference to <code>*this</code>.
             */
            iterator &operator++() {
                // end()++ &rarr; stay end()
                if (idx == static_cast<std::size_t>(-1))
                    return *this;

                auto &tree_nodes_ = tree->nodes_;
                std::size_t cur = idx;

                // case 1: has right subtree
                if (tree_nodes_[cur].right != static_cast<std::size_t>(-1)) {
                    cur = tree_nodes_[cur].right;
                    while (tree_nodes_[cur].left != static_cast<std::size_t>(-1))
                        cur = tree_nodes_[cur].left;
                    idx = cur;
                    return *this;
                }

                // case 2: go up until we exit right-side path
                std::size_t parent = tree_nodes_[cur].parent;
                while (parent != static_cast<std::size_t>(-1) && tree_nodes_[parent].right == cur) {
                    cur = parent;
                    parent = tree_nodes_[cur].parent;
                }

                // parent may become -1 -> end()
                idx = parent;
                return *this;
            }

            /**
             * @brief Post-increment: return the previous iterator value and
             *        advance to the in-order successor.
             *
             * @details
             * Equivalent to creating a copy, performing <code>++(*this)</code>,
             * and returning the saved copy. Follows the same semantic rules as
             * <code>operator++()</code>.
             *
             * @return A copy of the iterator before incrementing.
             */
            iterator operator++(int) {
                iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            /**
             * @brief Move to the in-order predecessor.
             *
             * @details
             * Performs an in-order predecessor step. Bidirectional-iterator
             * semantics apply:
             *
             * <ul>
             *   <li>
             *     If the iterator is <code>end()</code>, decrementing moves it
             *     to the last element (the greatest key), or leaves it unchanged
             *     if the container is empty.
             *   </li>
             *
             *   <li>
             *     If the iterator refers to the first element
             *     (<code>begin()</code>), decrementing moves it to
             *     <code>end()</code>.
             *   </li>
             *
             *   <li>
             *     Otherwise, the iterator moves to the previous element in
             *     sorted key order.
             *   </li>
             *
             *   <li>
             *     Undefined behavior results if the iterator is singular
             *     (e.g., default-constructed).
             *   </li>
             * </ul>
             *
             * @return Reference to <code>*this</code>.
             */
            iterator &operator--() {
                auto &tree_nodes_ = tree->nodes_;

                // end()-- &rarr; go to rightmost
                if (idx == static_cast<std::size_t>(-1)) {
                    std::size_t cur = tree->root;
                    if (cur == static_cast<std::size_t>(-1))
                        return *this;
                    while (tree_nodes_[cur].right != static_cast<std::size_t>(-1))
                        cur = tree_nodes_[cur].right;
                    idx = cur;
                    return *this;
                }

                std::size_t cur = idx;

                // case 1: has left subtree
                if (tree_nodes_[cur].left != static_cast<std::size_t>(-1)) {
                    cur = tree_nodes_[cur].left;
                    while (tree_nodes_[cur].right != static_cast<std::size_t>(-1))
                        cur = tree_nodes_[cur].right;
                    idx = cur;
                    return *this;
                }

                // case 2: go up until we exit left-side path
                std::size_t parent = tree_nodes_[cur].parent;
                while (parent != static_cast<std::size_t>(-1) && tree_nodes_[parent].left == cur) {
                    cur = parent;
                    parent = tree_nodes_[cur].parent;
                }

                // parent may become -1 -> end()
                idx = parent;
                return *this;
            }

            /**
             * @brief Post-decrement: return the previous iterator value and
             *        move to the in-order predecessor.
             *
             * @details
             * Equivalent to creating a copy, performing <code>--(*this)</code>,
             * and returning the saved copy. Follows the same semantic rules as
             * <code>operator--()</code>.
             *
             * @return A copy of the iterator before decrementing.
             */
            iterator operator--(int) {
                iterator tmp = *this;
                --(*this);
                return tmp;
            }

            /**
             * @brief Compare two iterators for equality.
             *
             * @details
             * Two iterators are equal only if they refer to the same container
             * and the same logical position. Singular iterators compare equal
             * only if both are singular.
             *
             * @return <code>true</code> if both iterators are equal.
             */
            bool operator==(const iterator &other) const {
                return tree == other.tree && idx == other.idx;
            }

            /**
             * @brief Compare two iterators for inequality.
             *
             * @details
             * Equivalent to <code>!(a == b)</code>.
             *
             * @return <code>true</code> if the iterators are not equal.
             */
            bool operator!=(const iterator &other) const {
                return !(*this == other); // NOLINT
            }
        };

        /**
         * @brief Const bidirectional iterator for in-order traversal.
         *
         * @details
         * Unlike <code>iterator</code>, this type provides a strictly read-only
         * view of the underlying elements. All dereference operations yield
         * <code>const</code> access, and mutation through the iterator is not
         * permitted. This mirrors the behavior of standard associative-container
         * <code>const_iterator</code>.
         *
         * Conversion from <code>iterator</code> to <code>const_iterator</code>
         * is supported, but not the reverse. Aside from const-qualification,
         * traversal semantics (increment, decrement, ordering) match those of
         * <code>iterator</code>.
         */
        struct const_iterator final {
            /// @brief Owning tree type.
            using base_type = tree_map;

            /// @brief Canonical node type used internally.
            using node_type = detail::node_t<K, V>;

            /// @brief Stored value type (key for set, pair<const K,V> for map).
            using stored_type = detail::base_t<K, V>;

            /// @brief Iterator concept tag (bidirectional).
            using iterator_concept = std::bidirectional_iterator_tag;

            /// @brief Iterator category for legacy algorithms.
            using iterator_category [[maybe_unused]] = iterator_concept;

            /// @brief Value type produced when dereferenced.
            using value_type = stored_type;

            /// @brief Reference type for dereferencing.
            using reference = const stored_type &;

            /// @brief Pointer type for dereferencing.
            using pointer = const stored_type *;

            /// @brief Difference type used for iterator arithmetic.
            using difference_type = std::int64_t;

            /// @brief Associated tree instance; <code>nullptr</code> indicates a singular iterator.
            const base_type *tree = nullptr;

            /// @brief Index of the referenced node, or <code>-1</code> for <code>end()</code>.
            std::size_t idx = static_cast<std::size_t>(-1);

            /**
             * @brief Construct a singular iterator.
             *
             * @details
             * A default-constructed const_iterator does not refer to any tree and
             * is therefore singular. It must not be incremented, decremented, or
             * dereferenced. Only assignment or destruction is permitted.
             *
             * This state is distinct from <code>end()</code>: an end iterator is
             * bound to a specific container and supports valid bidirectional
             * traversal (e.g., <code>--end()</code>), while a singular iterator does not.
             */
            const_iterator() = default;

            /**
             * @brief Construct an iterator referring to the given node index.
             *
             * @param t   Pointer to the owning tree.
             * @param i   Node index, or <code>-1</code> to represent <code>end()</code>.
             *
             * @details
             * This constructor is typically used internally to form begin/end
             * iterators. Users generally obtain iterators through member functions.
             */
            const_iterator(const base_type *t, std::size_t i) : tree(t), idx(i) {}

            /**
             * @brief Construct a const_iterator from a non-const iterator.
             *
             * @param it The iterator to convert.
             *
             * @details
             * Enables implicit promotion from iterator to const_iterator, matching
             * standard iterator behavior for associative containers.
             */
            explicit const_iterator(const iterator &it) : tree(it.tree), idx(it.idx) {}

            /**
             * @brief Dereference the iterator.
             *
             * @return Reference to the element.
             *
             * @details
             * Undefined behavior if the iterator is singular or equals <code>end()</code>.
             */
            reference operator*() const {
                return tree->nodes_[idx].stored();
            }

            /**
             * @brief Access the referenced element via pointer.
             *
             * @return Pointer to the element.
             *
             * @details
             * Undefined behavior if the iterator is singular or equals <code>end()</code>.
             */
            pointer operator->() const {
                return &tree->nodes_[idx].stored();
            }

            /**
             * @brief Obtain the key of the referenced element.
             *
             * @return Constant reference to the key.
             *
             * @details
             * Undefined behavior if the iterator is singular or equals <code>end()</code>.
             */
            [[nodiscard]] const node_type::key_type &key() const { return tree->nodes_[idx].key(); }

            /**
             * @brief Advance to the in-order successor.
             *
             * @details
             * Bidirectional-iterator semantics apply:
             *
             * <ul>
             *   <li>
             *     If the iterator is <code>end()</code>, incrementing leaves it unchanged.
             *   </li>
             *   <li>
             *     Otherwise moves to the next element in sorted order.
             *   </li>
             *   <li>
             *     Undefined behavior if the iterator is singular.
             *   </li>
             * </ul>
             *
             * @return Reference to <code>*this</code>.
             */
            const_iterator &operator++() {
                iterator tmp(const_cast<tree_map *>(tree), idx);
                ++tmp;
                tree = tmp.tree;
                idx = tmp.idx;
                return *this;
            }

            /**
             * @brief Post-increment.
             *
             * @details
             * Returns the prior iterator value, then advances using
             * <code>operator++()</code>.
             *
             * @return Copy of the iterator before incrementing.
             */
            const_iterator operator++(int) {
                auto r = *this;
                ++(*this);
                return r;
            }

            /**
             * @brief Move to the in-order predecessor.
             *
             * @details
             * Bidirectional-iterator semantics apply:
             *
             * <ul>
             *   <li>
             *     If the iterator is <code>end()</code>, decrementing moves it to
             *     the last element, or leaves it unchanged if the container is empty.
             *   </li>
             *   <li>
             *     If the iterator refers to <code>begin()</code>, decrementing
             *     moves it to <code>end()</code>.
             *   </li>
             *   <li>
             *     Otherwise moves to the previous element in sorted order.
             *   </li>
             *   <li>
             *     Undefined behavior if the iterator is singular.
             *   </li>
             * </ul>
             *
             * @return Reference to <code>*this</code>.
             */
            const_iterator &operator--() {
                iterator tmp(const_cast<tree_map *>(tree), idx);
                --tmp;
                tree = tmp.tree;
                idx = tmp.idx;
                return *this;
            }

            /**
             * @brief Post-decrement.
             *
             * @details
             * Returns the prior iterator value, then decrements using
             * <code>operator--()</code>.
             *
             * @return Copy of the iterator before decrementing.
             */
            const_iterator operator--(int) {
                auto r = *this;
                --(*this);
                return r;
            }

            /**
             * @brief Compare two const_iterators for equality.
             *
             * @param other The iterator to compare with.
             *
             * @details
             * Two const_iterators are equal only if they refer to the same
             * container and the same logical position.
             */
            bool operator==(const const_iterator &other) const {
                return tree == other.tree && idx == other.idx;
            }

            /**
             * @brief Compare two const_iterators for inequality.
             *
             * @details
             * Equivalent to <code>!(a == b)</code>.
             *
             * @return <code>true</code> if the iterators are not equal.
             */
            bool operator!=(const const_iterator &other) const {
                return !(*this == other); // NOLINT
            }
        };

        /**
         * @brief Reverse iterator type.
         *
         * @details
         * Traverses elements in descending key order.
         * Provides the same mutability as <code>iterator</code>.
         */
        using reverse_iterator = std::reverse_iterator<iterator>;

        /**
         * @brief Const reverse iterator type.
         *
         * @details
         * Traverses elements in descending key order.
         * All element access is read-only.
         */
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        /**
         * @brief Return an iterator to the smallest element.
         *
         * @details
         * If the container is empty, returns <code>end()</code>.
         */
        iterator begin() {
            if (root == static_cast<std::size_t>(-1))
                return iterator(this, static_cast<std::size_t>(-1));

            std::size_t cur = root;
            while (nodes_[cur].left != static_cast<std::size_t>(-1))
                cur = nodes_[cur].left;

            return iterator(this, cur);
        }

        /**
         * @brief Return the past-the-end iterator.
         *
         * @details
         * The returned iterator compares equal to all other
         * past-the-end iterators for the same container.
         */
        iterator end() {
            return iterator(this, static_cast<std::size_t>(-1));
        }

        /**
         * @brief Return a const iterator to the smallest element.
         *
         * @details
         * Identical to the non-const overload but produces a
         * <code>const_iterator</code>. Returns <code>end()</code>
         * if the container is empty.
         */
        [[nodiscard]] const_iterator begin() const {
            if (root == static_cast<std::size_t>(-1))
                return const_iterator(this, static_cast<std::size_t>(-1));

            std::size_t cur = root;
            while (nodes_[cur].left != static_cast<std::size_t>(-1))
                cur = nodes_[cur].left;

            return const_iterator(this, cur);
        }

        /**
         * @brief Return the const past-the-end iterator.
         */
        [[nodiscard]] const_iterator end() const {
            return const_iterator(this, static_cast<std::size_t>(-1));
        }

        /**
         * @brief Return a const iterator to the first element.
         *
         * @details
         * Equivalent to <code>begin() const</code>.
         */
        [[maybe_unused]] [[nodiscard]] const_iterator cbegin() const { return begin(); }

        /**
         * @brief Return the const past-the-end iterator.
         *
         * @details
         * Equivalent to <code>end() const</code>.
         */
        [[maybe_unused]] [[nodiscard]] const_iterator cend() const { return end(); }

        /**
         * @brief Return a reverse iterator to the last element.
         *
         * @details
         * Implemented as <code>reverse_iterator(end())</code>.
         * A reverse iterator constructed from <code>end()</code> yields the
         * position obtained conceptually by <code>--end()</code>, which is the
         * element with the greatest key.
         */
        [[maybe_unused]] reverse_iterator rbegin() {
            return reverse_iterator(end());
        }

        /**
         * @brief Return the reverse past-the-end iterator.
         *
         * @details
         * Implemented as <code>reverse_iterator(begin())</code>.
         * Because <code>reverse_iterator</code> stores the base iterator and
         * defines its past-the-end position as the value conceptually equal to
         * <code>--begin()</code>, this corresponds to the reverse end.
         *
         * Notably, <code>--begin()</code> is the normal forward <code>end()</code>;
         * therefore this reverse iterator represents the position one step
         * before <code>begin()</code> in reverse traversal.
         */
        [[maybe_unused]] reverse_iterator rend() {
            return reverse_iterator(begin());
        }

        /**
         * @brief Return a const reverse iterator to the last element.
         *
         * @details
         * Same semantics as the non-const overload: constructed from
         * <code>end()</code>, representing the position produced by the
         * conceptual operation <code>--end()</code>.
         */
        [[maybe_unused]] [[nodiscard]] const_reverse_iterator rbegin() const {
            return const_reverse_iterator(end());
        }

        /**
         * @brief Return the const reverse past-the-end iterator.
         *
         * @details
         * Same semantics as the non-const overload: constructed from
         * <code>begin()</code>, representing the reverse past-the-end position
         * associated with the conceptual <code>--begin()</code>, which equals
         * the normal forward <code>end()</code>.
         */
        [[maybe_unused]] [[nodiscard]] const_reverse_iterator rend() const {
            return const_reverse_iterator(begin());
        }

        /**
         * @brief Return a const reverse iterator to the last element.
         *
         * @details
         * Convenience wrapper identical to <code>rbegin() const</code>.
         */
        [[maybe_unused]] [[nodiscard]] const_reverse_iterator crbegin() const {
            return const_reverse_iterator(end());
        }

        /**
         * @brief Return the const reverse past-the-end iterator.
         *
         * @details
         * Convenience wrapper identical to <code>rend() const</code>. Represents
         * the reverse-iterator view of <code>--begin()</code>, which equals the
         * normal forward <code>end()</code>.
         */
        [[maybe_unused]] [[nodiscard]] const_reverse_iterator crend() const {
            return const_reverse_iterator(begin());
        }

    private:

        /**
         * @brief Return the height of the node at the given index.
         *
         * @details
         * An index of <code>-1</code> is treated as an empty subtree and
         * contributes height 0. Otherwise the stored height of the node is returned.
         */
        [[nodiscard]] std::uint16_t _height(std::size_t idx) const {
            return (idx != static_cast<std::size_t>(-1)) ? nodes_[idx].height : 0;
        }

        /**
         * @brief Recompute the height of the node at the given index.
         *
         * @details
         * Sets the node's height to one plus the maximum of its children's heights.
         * Used after structural modifications that may affect subtree heights.
         */
        void _update(std::size_t idx) {
            auto &node = nodes_[idx];
            std::uint16_t lh = _height(node.left);
            std::uint16_t rh = _height(node.right);
            node.height = (lh > rh ? lh : rh) + 1;
        }

        /**
         * @brief Compute the balance factor of the node at the given index.
         *
         * @details
         * Returns <code>left_height - right_height</code>. Positive values indicate
         * left-heavy subtrees; negative values indicate right-heavy subtrees.
         */
        [[nodiscard]] int _balance_factor(std::size_t idx) const {
            const auto &node = nodes_[idx];
            return int(_height(node.left)) - int(_height(node.right));
        }

        /**
         * @brief Perform a left rotation around the node at index <code>x</code>.
         *
         * @details
         * A left rotation promotes the right child of <code>x</code> and adjusts
         * parents and subtree relationships to preserve the binary-search-tree
         * ordering. Conceptually:
         *
         * <ol>
         *   <li>The right child of <code>x</code> becomes the new root of the
         *       rotated subtree.</li>
         *   <li><code>x</code> becomes the left child of the promoted node.</li>
         *   <li>The promoted node's previous left subtree becomes the new right
         *       subtree of <code>x</code>.</li>
         *   <li>Node heights are recomputed for the affected nodes.</li>
         * </ol>
         *
         * @param x Index of the subtree root to rotate.
         * @return The index of the subtree's new root after rotation.
         */
        std::size_t _rotate_left(std::size_t x) {
            std::size_t y = nodes_[x].right;
            auto &x_node = nodes_[x];
            auto &y_node = nodes_[y];

            std::size_t old_parent = x_node.parent;
            std::size_t T2 = y_node.left;

            y_node.left = x;
            x_node.parent = y;

            x_node.right = T2;
            if (T2 != static_cast<std::size_t>(-1))
                nodes_[T2].parent = x;

            y_node.parent = old_parent;

            if (old_parent == static_cast<std::size_t>(-1)) {
                root = y;
            } else {
                auto &p = nodes_[old_parent];
                if (p.left == x)
                    p.left = y;
                else
                    p.right = y;
            }

            _update(x);
            _update(y);

            return y;
        }

        /**
         * @brief Perform a right rotation around the node at index <code>y</code>.
         *
         * @details
         * A right rotation promotes the left child of <code>y</code> and adjusts
         * parent links and subtrees while preserving binary-search-tree order.
         * Conceptually:
         *
         * <ol>
         *   <li>The left child of <code>y</code> becomes the new root of the
         *       rotated subtree.</li>
         *   <li><code>y</code> becomes the right child of the promoted node.</li>
         *   <li>The promoted node's previous right subtree becomes the new left
         *       subtree of <code>y</code>.</li>
         *   <li>Node heights are updated for the nodes involved.</li>
         * </ol>
         *
         * @param y Index of the subtree root to rotate.
         * @return The index of the subtree's new root after rotation.
         */
        std::size_t _rotate_right(std::size_t y) {
            std::size_t x = nodes_[y].left;
            auto &y_node = nodes_[y];
            auto &x_node = nodes_[x];

            std::size_t old_parent = y_node.parent;
            std::size_t T1 = x_node.right;

            x_node.right = y;
            y_node.parent = x;

            y_node.left = T1;
            if (T1 != static_cast<std::size_t>(-1))
                nodes_[T1].parent = y;

            x_node.parent = old_parent;

            if (old_parent == static_cast<std::size_t>(-1)) {
                root = x;
            } else {
                auto &p = nodes_[old_parent];
                if (p.left == y)
                    p.left = x;
                else
                    p.right = x;
            }

            _update(y);
            _update(x);

            return x;
        }

        /**
         * @brief Rebalance the subtree rooted at the given index and continue
         *        upward toward the root.
         *
         * @details
         * This function restores AVL balance starting from <code>idx</code> and
         * repeatedly moving to its parent until reaching <code>-1</code>.
         * At each step:
         *
         * <ol>
         *   <li>The height of the current node is recomputed.</li>
         *   <li>The balance factor is examined to determine whether rotation
         *       is needed.</li>
         *   <li>
         *     Depending on subtree structure, one of the following cases is
         *     applied:
         *     <ol>
         *       <li><b>LL</b>: Right rotation.</li>
         *       <li><b>LR</b>: Left rotation on the left child, then right rotation.</li>
         *       <li><b>RR</b>: Left rotation.</li>
         *       <li><b>RL</b>: Right rotation on the right child, then left rotation.</li>
         *     </ol>
         *   </li>
         *   <li>The process proceeds to the parent node and repeats.</li>
         * </ol>
         *
         * @param idx Index of the node from which rebalancing begins.
         *
         * @return Always <code>-1</code> after reaching the top of the tree.
         */
        std::size_t _rebalance(std::size_t idx) {
            while (idx != static_cast<std::size_t>(-1)) {
                _update(idx);
                int bf = _balance_factor(idx);

                if (bf > 1 && _balance_factor(nodes_[idx].left) >= 0) {
                    idx = _rotate_right(idx);
                } else if (bf > 1 && _balance_factor(nodes_[idx].left) < 0) {
                    std::size_t left = nodes_[idx].left;
                    _rotate_left(left);
                    idx = _rotate_right(idx);
                } else if (bf < -1 && _balance_factor(nodes_[idx].right) <= 0) {
                    idx = _rotate_left(idx);
                } else if (bf < -1 && _balance_factor(nodes_[idx].right) > 0) {
                    std::size_t right = nodes_[idx].right;
                    _rotate_right(right);
                    idx = _rotate_left(idx);
                }
                idx = nodes_[idx].parent;
            }
            return idx;
        }

        /**
         * @brief Core insertion routine parameterized by compile-time assignment behavior.
         *
         * @details
         * This template forms the shared implementation of all public insertion
         * interfaces, including <code>insert</code>, <code>emplace</code> and
         * <code>insert_or_assign</code>. Both the key and mapped value are accepted
         * through forwarding references, allowing perfect forwarding of user-supplied
         * arguments. The template parameters <code>KArg</code> and <code>VArg</code>
         * must reduce (via <code>std::remove_cvref_t</code>) to the canonical key and
         * value types <code>K</code> and <code>V</code>.
         *
         * The compile-time boolean <code>Assign</code> selects whether an existing
         * mapped value should be overwritten when the key already exists. This is used
         * by <code>insert_or_assign</code> in map mode. For set semantics (where the
         * value type is <code>monostate</code>), the value parameter is ignored.
         *
         * The algorithm proceeds as follows:
         *
         * <ol>
         *   <li>If the tree is empty, a new root node is constructed from the
         *       forwarded key and value arguments.</li>
         *
         *   <li>Otherwise, the tree is traversed in binary-search-tree order until
         *       either an existing matching key is found or an appropriate parent for
         *       a new node is identified.</li>
         *
         *   <li>If the key already exists:
         *     <ol>
         *       <li>For set semantics, no modification is performed.</li>
         *       <li>For map semantics, the mapped value is updated only when
         *           <code>Assign == true</code>.</li>
         *       <li>An iterator to the existing node is returned and the boolean
         *           result is <code>false</code>.</li>
         *     </ol>
         *   </li>
         *
         *   <li>If the key does not exist, a new node is constructed using perfect
         *       forwarding of the key and value, attached beneath its parent in key
         *       order.</li>
         *
         *   <li>AVL invariants are then restored by rebalancing upward from the
         *       insertion point's parent.</li>
         *
         *   <li>The returned iterator refers to the newly inserted node and the
         *       boolean result is <code>true</code>.</li>
         * </ol>
         *
         * @tparam Assign
         *         Compile-time flag determining whether an existing mapped value
         *         should be replaced when the key is already present.
         * @tparam KArg
         *         Forwarded key argument type; must reduce to <code>K</code>.
         * @tparam VArg
         *         Forwarded mapped-value argument type; must reduce to <code>V</code>.
         *
         * @param key    Key of the element to insert or locate (forwarded).
         * @param value  Mapped value associated with the element (forwarded).
         *
         * @return A pair containing:
         *         <ol>
         *           <li>An iterator to the existing or newly inserted element.</li>
         *           <li><code>true</code> if a new element was inserted,
         *               <code>false</code> otherwise.</li>
         *         </ol>
         */
        template<bool Assign = false, typename KArg, typename VArg>
        requires std::is_same_v<std::remove_cvref_t<KArg>, K> &&
                 std::is_same_v<std::remove_cvref_t<VArg>, V>
        std::pair<iterator, bool>
        _insert_impl(KArg &&key, VArg &&value) {

            if (root == static_cast<std::size_t>(-1)) {
                std::size_t idx = nodes_.size();
                nodes_.emplace_back(
                        std::forward<KArg>(key),
                        std::forward<VArg>(value),
                        static_cast<std::size_t>(-1));
                root = idx;
                return {iterator(this, idx), true};
            }
            std::size_t cur = root;
            auto parent = static_cast<std::size_t>(-1);

            while (cur != static_cast<std::size_t>(-1)) {
                parent = cur;
                auto &node = nodes_[cur];

                if (std::forward<KArg>(key) < node.key()) {
                    cur = node.left;
                } else if (node.key() < std::forward<KArg>(key)) {
                    cur = node.right;
                } else {
                    if constexpr ((!jh::typed::monostate_t<V>) && Assign) {
                        node.value() = std::forward<VArg>(value);
                    }
                    return {iterator(this, cur), false};
                }
            }

            std::size_t idx = nodes_.size();
            nodes_.emplace_back(
                    std::forward<KArg>(key),
                    std::forward<VArg>(value),
                    parent);
            auto &p_node = nodes_[parent];
            if (std::forward<KArg>(key) < p_node.key())
                p_node.left = idx;
            else
                p_node.right = idx;

            _rebalance(parent);

            return {iterator(this, idx), true};
        }

        /**
         * @brief Compute the in-order successor of a node index.
         *
         * @details
         * Returns the index of the next element in in-order traversal, or
         * <code>static_cast&lt;std::size_t&gt;(-1)</code> if no successor exists.
         * The behavior follows the standard binary-search-tree successor rules:
         *
         * <ol>
         *   <li>
         *     If the node has a right subtree, the successor is the leftmost
         *     node of that subtree.
         *   </li>
         *
         *   <li>
         *     Otherwise, the successor is the first ancestor for which the
         *     current node lies in its left subtree. If no such ancestor exists,
         *     the node has no successor.
         *   </li>
         * </ol>
         *
         * This helper is used internally to implement iterator increment
         * operations.
         *
         * @param idx Index of the node whose successor is sought.
         * @return The index of the successor, or <code>-1</code> if none exists.
         */
        [[nodiscard]] std::size_t _successor_index(std::size_t idx) const {
            if (idx == static_cast<std::size_t>(-1)) return static_cast<std::size_t>(-1);
            const auto &tree_nodes_ = nodes_;

            if (tree_nodes_[idx].right != static_cast<std::size_t>(-1)) {
                std::size_t cur = tree_nodes_[idx].right;
                while (tree_nodes_[cur].left != static_cast<std::size_t>(-1))
                    cur = tree_nodes_[cur].left;
                return cur;
            }
            std::size_t cur = idx;
            std::size_t parent = tree_nodes_[cur].parent;

            while (parent != static_cast<std::size_t>(-1) && tree_nodes_[parent].right == cur) {
                cur = parent;
                parent = tree_nodes_[cur].parent;
            }
            return parent; // may be -1 -> end()
        }

        /**
         * @brief Return the index of the first element whose key is not less than the given key.
         *
         * @details
         * Performs a tree-based lower-bound search:
         * <ol>
         *   <li>Traverses the tree according to key ordering.</li>
         *   <li>Tracks the leftmost node whose key satisfies
         *       <code>!(node.key() &lt; key)</code> (i.e. node.key >= key).</li>
         *   <li>If such an element exists, returns its index; otherwise returns
         *       <code>static_cast&lt;std::size_t&gt;(-1)</code> to indicate end().</li>
         * </ol>
         *
         * This function does not modify the tree and produces the internal
         * index underlying lower_bound.
         */
        [[nodiscard]] std::size_t _lower_bound_idx(const node_type::key_type &key) const {
            std::size_t cur = root;
            auto candidate = static_cast<std::size_t>(-1);
            while (cur != static_cast<std::size_t>(-1)) {
                const auto &node = nodes_[cur];
                // node.key >= key
                if (!(node.key() < key))  // NOLINT
                {
                    candidate = cur;
                    cur = node.left;
                } else {
                    cur = node.right;
                }
            }
            return candidate;   // maybe -1
        }

        /**
         * @brief Return the index of the first element whose key is greater than the given key.
         *
         * @details
         * Performs a tree-based upper-bound search:
         * <ol>
         *   <li>Traverses the tree in key order.</li>
         *   <li>Tracks the leftmost node whose key satisfies
         *       <code>key &lt; node.key()</code> (i.e. node.key > key).</li>
         *   <li>If such an element exists, returns its index; otherwise returns
         *       <code>static_cast&lt;std::size_t&gt;(-1)</code> to indicate end().</li>
         * </ol>
         *
         * This function does not modify the tree and produces the internal
         * index underlying upper_bound.
         */
        [[nodiscard]] std::size_t _upper_bound_idx(const node_type::key_type &key) const {
            std::size_t cur = root;
            auto candidate = static_cast<std::size_t>(-1);
            while (cur != static_cast<std::size_t>(-1)) {
                const auto &node = nodes_[cur];
                // node.key > key
                if (key < node.key()) {
                    candidate = cur;
                    cur = node.left;
                } else {
                    cur = node.right;
                }
            }
            return candidate;
        }

    public:

        /**
         * @brief Locate the element with the specified key.
         *
         * @details
         * Performs a standard binary-search-tree lookup. If an element with the
         * given key exists, returns an iterator referring to it. Otherwise
         * returns <code>end()</code>.
         *
         * @param key The key to search for.
         * @return Iterator to the matching element, or <code>end()</code> if not found.
         */
        [[nodiscard]] iterator find(const node_type::key_type &key) {
            std::size_t cur = root;

            while (cur != static_cast<std::size_t>(-1)) {
                const auto &node = nodes_[cur];

                if (key == node.key()) {
                    return iterator(this, cur);
                } else if (key < node.key()) {
                    cur = node.left;
                } else {
                    cur = node.right;
                }
            }
            // not found -> return end()
            return iterator(this, static_cast<std::size_t>(-1));
        }

        /**
         * @brief Locate the element with the specified key (const overload).
         *
         * @details
         * Behaves identically to the non-const version but returns a
         * <code>const_iterator</code>.
         *
         * @param key The key to search for.
         * @return Const iterator to the matching element, or <code>end()</code> if not found.
         */
        [[nodiscard]] const_iterator find(const node_type::key_type &key) const {
            std::size_t cur = root;

            while (cur != static_cast<std::size_t>(-1)) {
                const auto &node = nodes_[cur];

                if (key == node.key()) {
                    return const_iterator(this, cur);
                } else if (key < node.key()) {
                    cur = node.left;
                } else {
                    cur = node.right;
                }
            }
            return const_iterator(this, static_cast<std::size_t>(-1));
        }

        /**
         * @brief Insert a key into the set.
         *
         * @details
         * For set semantics, <code>value_type</code> is simply the key type
         * <code>K</code>. If the key does not already exist, it is inserted
         * and the returned boolean is <code>true</code>. Otherwise the existing
         * element is returned and no modification occurs.
         *
         * @param key The key to insert.
         * @return A pair consisting of:
         *         <ol>
         *           <li>An iterator to the existing or newly inserted key.</li>
         *           <li><code>true</code> if insertion occurred,
         *               <code>false</code> otherwise.</li>
         *         </ol>
         */
        std::pair<iterator, bool> insert(const value_type &key) requires(jh::typed::monostate_t<V>) {
            return _insert_impl(key, V{});
        }

        /**
         * @brief Insert a key into the set (non-const reference overload).
         *
         * @details
         * Behaves identically to the const-reference overload. Provided for
         * completeness and consistency with associative-container interfaces.
         */
        std::pair<iterator, bool> insert(value_type &key) requires(jh::typed::monostate_t<V>) {
            return _insert_impl(key, V{});
        }

        /**
         * @brief Construct a key and insert it into the set.
         *
         * @details
         * The key is constructed from the forwarded arguments and then inserted
         * following standard set semantics. If the key already exists, no
         * modification occurs. This overload exists even though the underlying
         * node type uses only the key; construction is still routed through the
         * key's constructor invoked with <code>Args...</code>.
         *
         * @param args Constructor arguments for the key.
         * @return A pair as with <code>insert</code>, indicating the inserted
         *         or existing position.
         */
        template<typename... Args>
        std::pair<iterator, bool> emplace(Args &&... args) requires(jh::typed::monostate_t<V>) {
            return _insert_impl(typename node_type::key_type(std::forward<Args>(args)...),
                                V{});
        }

        /**
         * @brief Insert a key-value pair into the map.
         *
         * @details
         * This overload generalizes the traditional
         * <code>std::pair&lt;const K, V&gt;</code>-based insertion interface.
         * Instead of requiring a <code>value_type</code> object, any
         * 2-element tuple-like value is accepted as long as:
         * <ul>
         *   <li><code>get&lt;0&gt;(p)</code> has type <code>K</code> (after remove_cvref)</li>
         *   <li><code>get&lt;1&gt;(p)</code> has type <code>V</code> (after remove_cvref)</li>
         * </ul>
         *
         * This reflects the actual insertion semantics: the container consumes
         * the key and mapped value directly rather than constructing a
         * <code>std::pair&lt;const K, V&gt;</code> object, which is generally
         * not constructible from user input. Any tuple-like pair (including
         * <code>std::pair</code>, <code>std::tuple</code>, proxy references,
         * and structured-binding-compatible types) is therefore permitted if
         * its element types match exactly.
         * <br>
         * If the key already exists, no insertion occurs and the boolean
         * return value is <code>false</code>.
         *
         * @tparam P The tuple-like type providing key and mapped value.
         *
         * @param p A tuple-like value providing key and mapped value.
         *
         * @return A pair containing:
         *         <ol>
         *           <li>An iterator to the existing or newly inserted element.</li>
         *           <li><code>true</code> if a new element was inserted,
         *               <code>false</code> otherwise.</li>
         *         </ol>
         */
        template<typename P>
        requires (jh::concepts::pair_like_for<P, K, V>)
        std::pair<iterator, bool> insert(P &&p) requires (!jh::typed::monostate_t<V>) {
            return _insert_impl(
                    get<0>(std::forward<P>(p)),
                    get<1>(std::forward<P>(p))
            );
        }

        /**
         * @brief Insert a key-value pair or assign to the mapped value.
         *
         * @details
         * This function provides the combined "insert or assign" behavior used by
         * ordered associative maps. The key and mapped value are accepted as
         * forwarding references, allowing both lvalues and rvalues to be supplied.
         * When rvalues are provided, their contents may be moved into the container.
         *
         * The behavior is:
         * <ol>
         *   <li>If no element with the same key exists, a new element is inserted
         *       using the forwarded key and value.</li>
         *   <li>If an element with the same key already exists, its mapped value is
         *       replaced by the forwarded value.</li>
         * </ol>
         *
         * This mirrors the semantics of <code>std::map::insert_or_assign</code>.
         *
         * @param k The key of the element to insert or update (forwarded).
         * @param v The mapped value to assign or insert (forwarded).
         *
         * @return A pair consisting of:
         *         <ol>
         *           <li>An iterator to the inserted or updated element.</li>
         *           <li><code>true</code> if a new element was inserted,
         *               <code>false</code> if an existing element was updated.</li>
         *         </ol>
         */
        std::pair<iterator, bool>
        insert_or_assign(node_type::key_type &&k, node_type::value_type &&v) requires (!jh::typed::monostate_t<V>) {
            return _insert_impl<true>(k, v);
        }

        /**
         * @brief Insert a key-value pair by constructing it from arbitrary arguments.
         *
         * @details
         * This function preserves the usual <code>emplace</code> semantics: the
         * arguments are forwarded into a temporary <code>std::pair&lt;K, V&gt;</code>
         * (note: <code>K</code> is used, not <code>const K</code>). The key and
         * mapped value constructed in this temporary object are then forwarded into
         * the internal insertion logic.
         *
         * This matches the behavior of standard associative containers, where
         * <code>value_type</code> is not directly constructed for emplacement
         * because <code>std::pair&lt;const K, V&gt;</code> cannot be formed from
         * arbitrary arguments. Only the key and mapped value are used.
         *
         * If the key already exists in the map, construction still occurs but
         * no insertion is performed; the returned boolean is <code>false</code>.
         *
         * @param args Arguments used to construct a temporary <code>std::pair&lt;K, V&gt;</code>.
         *
         * @return A pair consisting of:
         *         <ol>
         *           <li>An iterator to the existing or newly inserted element.</li>
         *           <li><code>true</code> if a new element was inserted,
         *               <code>false</code> otherwise.</li>
         *         </ol>
         */
        template<typename... Args>
        std::pair<iterator, bool> emplace(Args &&... args) requires(!jh::typed::monostate_t<V>) {
            auto temp = typename node_type::store_t(std::forward<Args>(args)...);
            return _insert_impl(std::move(temp.first), std::move(temp.second));
        }

        /**
         * @brief Access the mapped value associated with a key.
         *
         * @details
         * Returns a reference to the mapped value corresponding to the specified
         * key. Unlike <code>operator[]</code>, this function does not insert a new
         * element when the key is not found. Instead, it throws
         * <code>std::out_of_range</code>.
         *
         * @param key The key whose associated value is to be accessed.
         * @return A reference to the mapped value.
         *
         * @throws std::out_of_range
         *         Thrown if no element with the specified key exists.
         */
        node_type::value_type &at(const node_type::key_type &key) requires(!jh::typed::monostate_t<V>) {
            auto it = find(key);
            if (it == end())
                throw std::out_of_range("jh::ordered_map::at(): key not found");

            return it->second;
        }

        /**
         * @brief Access the mapped value associated with a key (const overload).
         *
         * @details
         * Behaves identically to the non-const overload: if the key does not
         * exist, this function throws <code>std::out_of_range</code>. No insertion
         * ever occurs.
         *
         * @param key The key whose associated value is to be accessed.
         * @return A const reference to the mapped value.
         *
         * @throws std::out_of_range
         *         Thrown if no element with the specified key exists.
         */
        [[nodiscard]] const node_type::value_type &
        at(const node_type::key_type &key) const requires(!jh::typed::monostate_t<V>) {
            auto it = find(key);
            if (it == end())
                throw std::out_of_range("jh::ordered_map::at(): key not found");

            return it->second;
        }

        /**
         * @brief Access or insert the mapped value associated with a key.
         *
         * @details
         * This operator provides the standard map-style subscript semantics.
         * If an element with the given key already exists, a reference to its
         * mapped value is returned. Otherwise, a new element is created with the
         * specified key and a value-initialized mapped value
         * (<code>V{}</code>), and a reference to that mapped value is returned.
         *
         * Unlike <code>at()</code>, this function never throws: missing keys
         * always cause insertion. For this reason, it is only available when
         * the mapped type <code>V</code> is default-initializable.
         *
         * @param key The key whose associated value is to be accessed.
         * @return A reference to the existing or newly inserted mapped value.
         */
        node_type::value_type &operator[]
                (const node_type::key_type &key) requires((!jh::typed::monostate_t<V>) &&
                                                          std::default_initializable<V>) {
            if (root == static_cast<std::size_t>(-1)) {
                std::size_t idx = nodes_.size();
                nodes_.emplace_back(key, std::remove_cvref_t<V>{}, static_cast<std::size_t>(-1));
                root = idx;
                return nodes_[idx].value();
            }

            std::size_t cur = root;
            auto parent = static_cast<std::size_t>(-1);

            while (cur != static_cast<std::size_t>(-1)) {
                parent = cur;
                auto &node = nodes_[cur];
                if (key < node.key()) {
                    cur = node.left;
                } else if (node.key() < key) {
                    cur = node.right;
                } else {
                    return node.value();
                }
            }
            std::size_t idx = nodes_.size();
            nodes_.emplace_back(key, std::remove_cvref_t<V>{}, parent);

            auto &p_node = nodes_[parent];
            if (key < p_node.key())
                p_node.left = idx;
            else
                p_node.right = idx;

            _rebalance(parent);
            return nodes_[idx].value();
        }

        /**
         * @brief Count the number of elements with the specified key.
         *
         * @details
         * Because this container stores unique keys, the result is either
         * <code>0</code> (key not present) or <code>1</code> (key present).
         *
         * @param key The key to search for.
         * @return <code>1</code> if the key exists, <code>0</code> otherwise.
         */
        [[nodiscard]] std::size_t count(const node_type::key_type &key) const {
            return find(key) == end() ? 0 : 1;
        }

        /**
         * @brief Erase the element referenced by the given iterator.
         *
         * @details
         * Removes the element pointed to by <code>pos</code> and returns an
         * iterator to its logical in-order successor. If <code>pos</code> refers
         * to <code>end()</code>, no action is performed and <code>pos</code> is
         * returned unchanged.
         *
         * Contiguous-array binary trees cannot preserve iterator validity after
         * structural changes: erasing any element may relocate other nodes in the
         * underlying storage. Therefore:
         *
         * <ul>
         *   <li>
         *     Every iterator obtained prior to this call, except the one returned
         *     by the function itself, becomes invalid and must not be used.
         *   </li>
         *   <li>
         *     The returned iterator refers to the next element in sorted key
         *     order. If the erased element was the last in order, the returned
         *     iterator equals <code>end()</code>.
         *   </li>
         * </ul>
         *
         * The operation preserves AVL invariants and performs any required
         * rebalancing after removal.
         *
         * @param pos Iterator referring to the element to erase.
         * @return An iterator to the in-order successor of the erased element,
         *         or <code>end()</code> if no successor exists.
         */
        iterator erase(iterator pos) {
            std::size_t idx = pos.idx;
            if (idx == static_cast<std::size_t>(-1))
                return pos;

            std::size_t next_idx = _successor_index(idx);
            auto &node = nodes_[idx];
            auto transplant = [&](std::size_t u, std::size_t v) {
                std::size_t parent = nodes_[u].parent;

                if (parent == static_cast<std::size_t>(-1)) {
                    root = v;
                } else {
                    auto &p = nodes_[parent];
                    if (p.left == u) p.left = v;
                    else p.right = v;
                }
                if (v != static_cast<std::size_t>(-1)) {
                    nodes_[v].parent = parent;
                }
            };

            std::size_t parent_for_rebalance = node.parent;

            if (node.left == static_cast<std::size_t>(-1)) {
                transplant(idx, node.right);
            } else if (node.right == static_cast<std::size_t>(-1)) {
                transplant(idx, node.left);
            } else {
                std::size_t success = node.right;
                while (nodes_[success].left != static_cast<std::size_t>(-1))
                    success = nodes_[success].left;

                if (nodes_[success].parent != idx) {
                    transplant(success, nodes_[success].right);
                    nodes_[success].right = node.right;
                    if (node.right != static_cast<std::size_t>(-1))
                        nodes_[node.right].parent = success;
                }
                transplant(idx, success);
                nodes_[success].left = node.left;
                nodes_[node.left].parent = success;

                parent_for_rebalance = success;
            }

            std::size_t last = nodes_.size() - 1;
            if (idx != last) {
                auto last_node = nodes_[last];
                if (last_node.parent != static_cast<std::size_t>(-1)) {
                    auto &p = nodes_[last_node.parent];
                    if (p.left == last) p.left = idx;
                    else p.right = idx;
                }

                if (last_node.left != static_cast<std::size_t>(-1)) nodes_[last_node.left].parent = idx;
                if (last_node.right != static_cast<std::size_t>(-1)) nodes_[last_node.right].parent = idx;
                if (root == last)
                    root = idx;

                nodes_[idx] = std::move(last_node);
                if (parent_for_rebalance == last)
                    parent_for_rebalance = idx;
                else if (parent_for_rebalance > last)
                    parent_for_rebalance = static_cast<std::size_t>(-1);
            }

            nodes_.pop_back();

            if (parent_for_rebalance < nodes_.size()) {
                // naturally supports != static_cast<std::size_t>(-1)
                _rebalance(parent_for_rebalance);
            }

            return iterator(this, next_idx);
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
         * @return An iterator to the in-order successor of the erased element,
         *         or <code>end()</code> if no successor exists.
         */
        iterator erase(const_iterator pos) {
            return erase(iterator{this, pos.idx});
        }

        /**
         * @brief Erase the element whose key compares equal to the given key.
         *
         * @details
         * Searches for an element with the specified key and removes it if found.
         * Iterator validity follows the rules of single-element erase:
         * <ul>
         *   <li>
         *     If an element is erased, all iterators obtained prior to this call
         *     become invalid, as element removal and subsequent compaction may
         *     relocate nodes.
         *   </li>
         *   <li>
         *     If no matching key exists, the container is left unmodified and
         *     no iterators are invalidated.
         *   </li>
         * </ul>
         *
         * @param key The key of the element to remove.
         * @return <code>1</code> if an element was erased,
         *         <code>0</code> otherwise.
         */
        std::size_t erase(const node_type::key_type &key) {
            iterator it = find(key);
            if (it == end()) return 0;
            erase(it);
            return 1;
        }

        /**
         * @brief Return an iterator to the first element whose key is not less than the given key.
         *
         * @details
         * Provides the standard ordered-container lower-bound semantics:
         * <ul>
         *   <li>Returns an iterator referring to the first element whose key is
         *       greater than or equal to <code>key</code>.</li>
         *   <li>If no such element exists, returns <code>end()</code>.</li>
         *   <li>Does not modify the container.</li>
         * </ul>
         */
        iterator lower_bound(const node_type::key_type &key) {
            return iterator(this, _lower_bound_idx(key));
        }

        /**
         * @brief Const overload of lower_bound.
         *
         * @details
         * Behaves identically to the non-const version but returns a const iterator.
         */
        [[nodiscard]] const_iterator lower_bound(const node_type::key_type &key) const {
            return const_iterator(this, _lower_bound_idx(key));
        }

        /**
         * @brief Return an iterator to the first element whose key is greater than the given key.
         *
         * @details
         * Matches the usual ordered-container semantics:
         * <ul>
         *   <li>Returns an iterator referring to the first element for which
         *       <code>element.key > key</code>.</li>
         *   <li>If no such element exists, returns <code>end()</code>.</li>
         *   <li>Does not modify the container.</li>
         * </ul>
         */
        iterator upper_bound(const node_type::key_type &key) {
            return iterator(this, _upper_bound_idx(key));
        }

        /**
         * @brief Const overload of upper_bound.
         */
        [[nodiscard]] const_iterator upper_bound(const node_type::key_type &key) const {
            return const_iterator(this, _upper_bound_idx(key));
        }

        /**
         * @brief Return the range of elements equivalent to the given key.
         *
         * @details
         * Provides the canonical <code>equal_range</code> semantics for
         * associative containers with unique keys:
         * <ul>
         *   <li>
         *     If an element with the given key exists, returns a pair
         *     <code>{lower_bound(key), upper_bound(key)}</code>.
         *   </li>
         *   <li>
         *     If no such element exists, both iterators in the returned pair
         *     equal <code>lower_bound(key)</code>.
         *   </li>
         *   <li>
         *     The returned range is half-open: the first iterator refers to
         *     the element with the key (if present), and the second refers to
         *     the element that follows it.
         *   </li>
         *   <li>Does not modify the container.</li>
         * </ul>
         *
         * @param key The key to search for.
         *
         * @return A pair of iterators defining the range of matching elements.
         */
        std::pair<iterator, iterator>
        equal_range(const node_type::key_type &key) {
            iterator lb = lower_bound(key);
            if (lb == end() || !(key == lb.key())) // NOLINT
            {
                return {lb, lb};
            }
            iterator ub = lb;
            ++ub;
            return {lb, ub};
        }

        /**
         * @brief Return the range of elements equivalent to the given key (const overload).
         *
         * @details
         * Behaves identically to the non-const version but returns a pair of
         * <code>const_iterator</code>.
         *
         * @param key The key to search for.
         *
         * @return A pair of const iterators defining the range of matching elements.
         */
        [[nodiscard]] std::pair<const_iterator, const_iterator>
        equal_range(const node_type::key_type &key) const {
            const_iterator lb = lower_bound(key);
            if (lb == end() || !(key == lb.key())) // NOLINT
            {
                return {lb, lb};
            }
            const_iterator ub = lb;
            ++ub;
            return {lb, ub};
        }

        /**
         * @brief Const overload of equal_range.
         */
        [[nodiscard]] std::size_t size() const noexcept {
            return nodes_.size();
        }

        /**
         * @brief Check whether the container contains no elements.
         *
         * @details
         * Equivalent to testing whether the underlying contiguous storage is empty.
         * Does not modify the container and runs in constant time.
         *
         * @return true if the container has no elements, otherwise false.
         */
        [[nodiscard]] bool empty() const noexcept {
            return nodes_.empty();
        }

        /**
         * @brief Remove all elements from the container.
         *
         * @details
         * <p>
         * Resets the container to an empty state by clearing the underlying
         * contiguous storage and resetting the root index. The operation is
         * equivalent to clearing a vector:
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
         * tree structures such as those used by the standard ordered
         * containers, there is no need to traverse and destroy individual nodes;
         * the entire tree is discarded in one step.
         * </p>
         *
         * @note
         * All iterators are invalidated.
         */
        void clear() noexcept {
            nodes_.clear();
            root = static_cast<std::size_t>(-1);
        }

        /**
         * @brief Reserve space for at least n elements.
         *
         * @details
         * <p>
         * Requests that the underlying contiguous storage grow its capacity to
         * at least <code>n</code> elements. This operation does not change the
         * size of the container or alter any existing node indices.
         * </p>
         * <p>
         * Since iterators refer to elements by stable indices rather than
         * pointers, increasing capacity does not invalidate any iterators,
         * even if the underlying buffer is reallocated.
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
         */
        void reserve(std::size_t n) noexcept(noexcept(nodes_.reserve(n))) {
            nodes_.reserve(n);
        }

        /**
         * @brief Request that the container reduce its capacity.
         *
         * @details
         * <p>
         * Issues a non-binding request to the underlying contiguous storage to
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
        void shrink_to_fit() noexcept(noexcept(nodes_.shrink_to_fit())) {
            nodes_.shrink_to_fit();
        }

        /**
         * @brief Construct a tree by inserting elements from an input iterator range.
         *
         * @tparam It Input iterator type whose dereferenced value is accepted by <code>insert()</code>.
         *
         * @param first Iterator to the first element in the input range.
         * @param last  Iterator past the last element in the input range.
         *
         * @details
         * The tree is first initialized to an empty state via <code>clear()</code>, after
         * which all elements in the range [first, last) are inserted using
         * <code>insert()</code>.
         *
         * If @p It models <code>std::random_access_iterator</code>, the distance between
         * @p first and @p last can be computed in constant time. This allows an
         * implementation to pre-reserve exactly <code>(last - first)</code> nodes before
         * insertion, reducing reallocation overhead.
         *
         * @warning
         * Iterators must not falsely claim to be random-access. In particular,
         * a type that is fundamentally bidirectional or forward must not provide
         * random-access operations (e.g., <code>operator[]</code>, <code>operator+(int)</code>) by
         * emulating them via repeated <code>operator++</code> or <code>operator--</code> steps.
         * Such "pseudo-random-access" iterators degrade performance severely,
         * because the implementation may assume <tt>O(1)</tt> distance and indexing while
         * the actual cost is <tt>O(n)</tt>. Iterator category tags must accurately reflect
         * the iterator's capabilities.
         */
        template<std::input_iterator It>
        requires requires(tree_map &t, It i) {
            t.insert(*i);
        }
        [[maybe_unused]] tree_map(It first, It last) {
            clear();
            if constexpr (std::random_access_iterator<It>) {
                nodes_.reserve(last - first);
            }
            for (; first != last; ++first)
                insert(*first);
        }

        /**
         * @brief Construct a tree by inserting elements from an input range.
         *
         * @tparam R Input range type whose elements are accepted by <code>insert()</code>.
         *
         * @param r The input range to consume.
         *
         * @details
         * The tree is cleared and each element of @p r is inserted using
         * <code>insert()</code>.
         *
         * If @p R models <code>std::ranges::sized_range</code>, the node buffer may
         * reserve space for <code>std::size(r)</code> elements prior to insertion. For
         * random-access ranges this is typically <tt>O(1)</tt>.
         *
         * @warning
         * As with iterator-based construction, range types must not provide a
         * misleading random-access iterator category. Emulating random-access
         * semantics atop a non-random-access traversal (e.g., implementing
         * <code>operator[]</code> by repeated increment operations) causes unnecessary
         * <tt>O(n)</tt> behavior and defeats the intended performance guarantees.
         */
        template<std::ranges::input_range R>
        requires requires(tree_map &t, R r) {
            t.insert(*std::begin(r));
        }
        [[maybe_unused]] explicit tree_map(R &&r) {
            clear();
            if constexpr (std::ranges::sized_range<R>) {
                nodes_.reserve(std::size(r));
            }
            for (auto &&x: r)
                insert(x);
        }

        /**
         * @brief Construct an AVL tree from an already sorted and unique range.
         *
         * <h3>Purpose</h3>
         * <p>
         * <code>from_sorted()</code> builds a perfectly balanced contiguous AVL tree
         * directly from a sorted, strictly-unique input range. Unlike repeated
         * <code>insert()</code>, which performs <tt>O(log N)</tt> insertions with
         * rebalancing, this routine constructs the entire tree in
         * <b>near-perfect <tt>O(N)</tt></b> time.
         * </p>
         *
         * <p>
         * The input must satisfy:
         * </p>
         * <ul>
         *   <li>strictly increasing keys (already sorted),</li>
         *   <li>no duplicates,</li>
         *   <li>size known (<code>std::ranges::sized_range</code>),</li>
         *   <li>value_type convertible to <code>std::pair&lt;const K, V&gt;</code> for <code>jh::ordered_map</code>
         *       or <code>K</code> for <code>jh::ordered_set</code>.</li>
         * </ul>
         *
         * <h3>Why This Matters</h3>
         * <p>
         * Many workloads naturally produce ordered batches: log-structured indexing,
         * preprocessing pipelines, analytics results, time-sorted packets, or any
         * domain where data is accumulated in vectors. Constructing the
         * <code>jh::ordered_set/map</code> directly from this monotonic sequence avoids the
         * <b>costly per-node insertion</b> and removes the need for AVL rotations.
         * </p>
         *
         * <h3>Complexity</h3>
         * <ul>
         *   <li><b>Construction:</b> <tt>O(N)</tt> (tree shape derived directly)</li>
         *   <li><b>Iterator validity:</b> all iterators valid post-construction</li>
         *   <li><b>Recursion:</b> none (iterative layout)</li>
         * </ul>
         *
         * <h3>Performance Characteristics</h3>
         * <p>
         * Benchmarked on an Apple M3 (Clang++20) with 10 000 random or sorted
         * <code>std::string</code> keys:
         * </p>
         *
         * <table>
         *   <tr>
         *     <th>Operation</th>
         *     <th>Runtime (ns)</th>
         *     <th>Notes</th>
         *   </tr>
         *   <tr>
         *     <td><code>ordered_set.insert (random)</code></td>
         *     <td>1.4 e7 (~14 ms)</td>
         *     <td>AVL rotations + random memory access</td>
         *   </tr>
         *   <tr>
         *     <td><code>ordered_set.insert (sorted)</code></td>
         *     <td>significantly faster</td>
         *     <td>input order strongly affects performance</td>
         *   </tr>
         *   <tr>
         *     <td><code>stable_sort(10k strings)</code></td>
         *     <td>~9.6 e6 (~9.6 ms)</td>
         *     <td>merge sort detects ordered runs</td>
         *   </tr>
         *   <tr>
         *     <td><code>unique(10k strings)</code></td>
         *     <td>~1 e5 (0.1 ms)</td>
         *     <td>linear; negligible vs sorting</td>
         *   </tr>
         *   <tr>
         *     <td><code>from_sorted (10k strings)</code></td>
         *     <td>~8.6 e5 (0.86 ms)</td>
         *     <td>builds perfect AVL directly</td>
         *   </tr>
         *   <tr>
         *     <td><b>Synthesis:</b> <code>sort + unique + from_sorted</code></td>
         *     <td>~1.06 e7 (~10.6 ms)</td>
         *     <td>&lt; <code>insert</code> cost even when input is fully random</td>
         *   </tr>
         * </table>
         *
         * <h3>Interpretation</h3>
         * <ul>
         *   <li>Even with completely random input, a vector &rarr; sort &rarr; unique &rarr;
         *       <code>from_sorted</code> pipeline is faster than 10k random AVL insertions.</li>
         *   <li>For already-sorted or partially-sorted sequences, runtime becomes almost
         *       linear and outperforms both <code>ordered_set</code> and
         *       <code>std::set</code>.</li>
         *   <li><code>stable_sort</code> is strongly order-aware; partially sorted sequences
         *       reduce its cost dramatically.</li>
         *   <li><code>unique()</code> cost is negligible compared to sorting.</li>
         *   <li>Memory locality is maximized: all nodes fit in one contiguous vector.</li>
         * </ul>
         *
         * <h3>When To Use</h3>
         * <ul>
         *   <li>Bulk construction from preprocessed or batched data.</li>
         *   <li>Loading on-disk sorted indices.</li>
         *   <li>Temporal/event logs with strictly increasing timestamps.</li>
         *   <li>Any context where <code>std::set/map</code> would require many insertions.</li>
         *   <li>When memory fragmentation must be tightly controlled.</li>
         * </ul>
         *
         * <h3>Example</h3>
         * @code
         * std::vector&lt;int&gt; v = {...};
         * std::stable_sort(v.begin(), v.end());
         * sorted.erase(std::unique(v.begin(), v.end()), v.end());
         * auto s = jh::ordered_set&lt;int&gt;::from_sorted(v);
         * // s is a perfectly balanced AVL using contiguous storage
         * @endcode
         *
         * @tparam R a sized, sorted, strictly unique input range
         * @param r range whose values will be copied/moved into the resulting tree
         * @return a fully constructed, perfectly balanced <code>tree_map</code>
         *
         * @warning Input must be sorted and unique. No validation is performed.
         */
        template<std::ranges::sized_range R>
        requires ((requires(tree_map &t, R r) {
            t.insert(*std::begin(r));
        }) && std::default_initializable<K>
                  && std::default_initializable<V>)
        static tree_map from_sorted(R &&r) {
            const auto size_n = std::size(r);
            tree_map res{};
            if (!size_n) return res;
            res.reserve(size_n);
            res.nodes_.resize(size_n);
            auto &vec = res.nodes_;
            res.root = 0;

            if (size_n == 1) {
                vec[0].parent = static_cast<std::size_t>(-1);
                vec[0].left = static_cast<std::size_t>(-1);
                vec[0].right = static_cast<std::size_t>(-1);
                vec[0].height = 1;
            } else if (size_n == 2) {
                vec[0].parent = static_cast<std::size_t>(-1);
                vec[0].left = 1;
                vec[0].right = static_cast<std::size_t>(-1);
                vec[0].height = 2;

                vec[1].parent = 0;
                vec[1].left = static_cast<std::size_t>(-1);
                vec[1].right = static_cast<std::size_t>(-1);
                vec[1].height = 1;
            } else {
                std::uint16_t lvl = 1;
                auto max_height = static_cast<std::uint16_t>(0);
                auto test = size_n;
                while (test) {
                    test >>= 1;
                    ++max_height;
                }
                vec[res.root].parent = static_cast<std::size_t>(-1);
                vec[res.root].left = 1;
                vec[res.root].right = 2;
                vec[res.root].height = max_height;

                bool has_children = true;
                const std::size_t cutoff = (size_n - 1) >> 1;
                std::size_t fix_begin = (size_n & 1) ? cutoff : cutoff + 1;
                std::vector<std::size_t> fixes{};
                fixes.resize(max_height);
                std::fill(fixes.begin(), fixes.end(), static_cast<std::size_t>(-1));
                for (int st = static_cast<int>(max_height) - 2; st > 0; --st) {
                    fixes[st] = fix_begin;
                    if (!(fix_begin & 1)) break;
                    fix_begin >>= 1;
                }

                for (std::size_t i = 1; i < size_n; ++i) {
                    vec[i].parent = ((i + 1) >> 1) - 1;
                    if (has_children) {
                        if (i != cutoff) [[likely]] {
                            vec[i].left = (i << 1) + 1;
                            vec[i].right = (i << 1) + 2;
                        } else {
                            has_children = false;
                            vec[i].left = (!(size_n & 1)) ? (i << 1) + 1 : static_cast<std::size_t>(-1);
                            vec[i].right = static_cast<std::size_t>(-1);
                        }
                    } else {
                        vec[i].left = static_cast<std::size_t>(-1);
                        vec[i].right = static_cast<std::size_t>(-1);
                    }
                    if (i >= fixes[lvl]) {
                        vec[i].height = max_height - lvl - 1;
                    } else {
                        vec[i].height = max_height - lvl;
                    }

                    if (!((i + 2) & (i + 1))) {
                        ++lvl;
                    }
                }
            }

            if constexpr (jh::typed::monostate_t<V>) {
                auto ite = res.begin();
                for (const auto &v: r) {
                    vec[ite.idx].stored_ = value_type{v};
                    ++ite;
                }
            } else {
                auto ite = res.begin();
                for (auto &v: r) {
                    vec[ite.idx].stored_ = std::pair{
                            get<0>(std::forward<decltype(v)>(v)),
                            get<1>(std::forward<decltype(v)>(v))
                    };
                    ++ite;
                }
            }
            return res;
        }
    };

} // namespace jh::avl

namespace jh {

    /**
     * @brief Ordered associative set based on a contiguous-array AVL tree.
     *
     * @details
     * This alias provides a set-like container storing unique keys of type
     * <code>K</code>. The semantics match those of <code>std::set</code>:
     * keys are unique, sorted in strictly increasing order, and no mapped
     * value is stored. Internally this type is implemented using
     * <code>avl::tree_map</code> with <code>monostate</code> as its value
     * type, yielding a compact and allocation-free node layout.
     *
     * @tparam K     Key type stored in the set.
     * @tparam Alloc Allocator used for internal node storage.
     */
    template<typename K,
            typename Alloc = std::allocator<K>>
    using ordered_set =
            avl::tree_map<K, jh::typed::monostate, Alloc>;

    /**
     * @brief Ordered associative map based on a contiguous-array AVL tree.
     *
     * @details
     * This alias provides an ordered map storing unique keys of type
     * <code>K</code> and mapped values of type <code>V</code>. The semantics
     * match those of <code>std::map</code>: keys are unique, elements are
     * stored in sorted key order, and the mapped value is accessible via
     * key lookup or iterator dereferencing. Internally this type is backed
     * by <code>avl::tree_map</code>, using a contiguous storage layout
     * rather than the pointer-based red-black tree used by STL maps.
     *
     * @tparam K     Key type.
     * @tparam V     Mapped value type.
     * @tparam Alloc Allocator used for internal node storage.
     */
    template<typename K, typename V,
            typename Alloc = std::allocator<std::pair<const K, V>>>
    using ordered_map =
            avl::tree_map<K, V, Alloc>;

} // namespace jh
