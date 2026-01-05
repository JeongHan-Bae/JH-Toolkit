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
 * @file pointer_pool.h
 *
 * @brief Pointer-based interning for non-copyable, non-movable, structurally immutable objects.
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 *
 * <h3>Overview</h3>
 * <p>
 * <code>jh::conc::pointer_pool</code> is a weak-observed pointer interning container designed for objects
 * whose identity is defined intrinsically by the object itself and which cannot be copied, moved,
 * or represented by an external key. These objects are stored and deduplicated through
 * <code>std::shared_ptr</code> instances, while the pool maintains only <code>std::weak_ptr</code>
 * references for lookup and reuse.
 * </p>
 *
 * <p>
 * The pool specializes in handling objects that must reside at a stable address for their entire lifetime,
 * and therefore cannot be placed inside contiguous storage or trivial containers. It enables
 * pointer-stable sharing without imposing ownership, allocator customization, or intrusive hooks.
 * </p>
 *
 * <h4>Design Philosophy</h4>
 * <p>
 * The essential purpose of <code>pointer_pool</code> is to support types that are impossible to intern
 * through traditional contiguous or key-indexed structures. These types may be non-copyable,
 * non-movable, or may express equality only through their full object state rather than an external key.
 * As a result, they cannot participate in compact storage models and must rely on pointer identity
 * for stable lifetime management.
 * </p>
 *
 * <p>
 * Because the pool only ever stores separate heap-allocated shared objects, fragmentation is unavoidable.
 * For this reason, the pool deliberately avoids allocator customization. Regardless of allocator choice,
 * large numbers of pointer-sized allocations inevitably produce fragmentation, and avoiding it is neither
 * practical nor a design goal for this container.
 * </p>
 *
 * <h4>Lookup Model</h4>
 * <p>
 * The pool does not provide a <code>find()</code> operation. This is a fundamental design decision.
 * Equality and hashing depend on the object itself, so a candidate object must already exist before
 * the pool can determine whether an equivalent instance is present. Therefore, every acquisition follows
 * the sequence:
 * </p>
 *
 * <ol>
 *   <li>A candidate object is constructed (temporarily).</li>
 *   <li>The pool performs a hash-based lookup using this constructed object.</li>
 *   <li>If an equivalent object already exists, that instance is returned and the candidate is discarded.</li>
 *   <li>If no match exists, the candidate becomes the canonical instance stored inside the pool.</li>
 * </ol>
 *
 * <p>
 * This model ensures that objects can be deduplicated even when they offer no external key. However,
 * it also means that construction must always occur before lookup. As such, the theoretical access
 * cost is a combination of <code>O(1)</code> hash addressing and whatever cost is required to build
 * a provisional object.
 * </p>
 *
 * <h4>Recommended Object Pattern</h4>
 * <p>
 * Because provisional construction may occur frequently, objects used with <code>pointer_pool</code>
 * should support low-cost identity construction. Heavy initialization should be deferred until
 * after the object becomes the accepted canonical instance. A common pattern is:
 * </p>
 *
 * <ul>
 *   <li>Construct identity-defining fields first (used in hashing and equality).</li>
 *   <li>Perform heavy or mutable initialization lazily, often guarded by <code>std::once_flag</code>.</li>
 * </ul>
 *
 * <p>
 * This approach allows the pool to discard temporary instances cheaply while ensuring that full
 * initialization happens only for the accepted canonical object.
 * </p>
 *
 * <h4>Lifetime and Ownership Model</h4>
 * <p>
 * The pool never owns any object. All objects are owned exclusively by <code>std::shared_ptr</code>
 * instances returned to the user. The pool only observes these objects via <code>std::weak_ptr</code>.
 * </p>
 *
 * <p>
 * Because of this design:
 * </p>
 *
 * <ul>
 *   <li>The destruction order between the pool and the objects is irrelevant.</li>
 *   <li>Objects remain valid even if the pool is destroyed first.</li>
 *   <li>Expired entries are removed opportunistically during insertion or via explicit cleanup calls.</li>
 * </ul>
 *
 * <h4>Cleanup and Resizing</h4>
 * <p>
 * Cleanup is performed on a best-effort basis. The pool removes expired weak entries only during
 * insertion, expansion, or explicit calls to <code>cleanup()</code> and <code>cleanup_shrink()</code>.
 * </p>
 *
 * <p>
 * Resizing and shrinking are adaptive:
 * </p>
 *
 * <ul>
 *   <li>Before expanding, the pool attempts cleanup.</li>
 *   <li>If the set remains above a high-watermark threshold, capacity grows.</li>
 *   <li>If cleanup reveals vacancy below a low-watermark threshold, capacity may shrink.</li>
 * </ul>
 *
 * <p>
 * This design avoids rehash jitter and minimizes allocation disturbances during periods of
 * high-frequency reuse.
 * </p>
 *
 * <h4>Intended Use Cases</h4>
 * <p>
 * The pool is intended for objects that:
 * </p>
 *
 * <ul>
 *   <li>cannot be copied or moved,</li>
 *   <li>cannot be expressed through an external key,</li>
 *   <li>must rely on full-object equality for deduplication,</li>
 *   <li>require stable pointer identity throughout their lifetime.</li>
 * </ul>
 *
 * <h4>Comparison with <code>flat_pool</code> (<code>resource_pool</code>)</h4>
 * <p>
 * <code>pointer_pool</code> differs fundamentally from <code>flat_pool</code> and its user-layer
 * extension <code>resource_pool</code>. While <code>pointer_pool</code> is designed for immovable,
 * non-copyable objects that cannot be represented by an external key, <code>flat_pool</code> supports
 * contiguous memory layout, slot reuse, and key-driven lookup. It offers <code>find()</code> operations
 * that return a null pointer on miss and can store either keys alone or key-value pairs. Lookup is based
 * on full-hash binary search with <code>O(log N)</code> complexity.
 * </p>
 *
 * <p>
 * Although <code>flat_pool</code> has asymptotically higher lookup complexity than the
 * <code>O(1)</code> expectation of hash probing, this does not imply inferior performance. Binary search
 * over contiguous memory is extremely cache-friendly, and modern CPUs can predict the comparison pattern
 * effectively due to stable branching behavior. Even without branch prediction, the number of steps is
 * small and bounded. By contrast, hash-table probing involves irregular memory access and higher
 * constant-time factors despite its theoretical <code>O(1)</code> model. As a result, at small and medium
 * scales, <code>flat_pool</code> lookup performance can match or exceed that of <code>pointer_pool</code>.
 * In reality, excessive fragmentation and pointer addressing that may fall into L3 are unacceptable in
 * practical applications, <code>pointer_pool</code> will <b>never</b> be used in large-scale storage,
 * so it can be simply assumed that <code>flat_pool</code> is faster and can store larger amounts of data.
 * </p>
 *
 * <p>
 * <code>flat_pool</code> requires objects to be copyable or movable and can integrate allocator
 * customization. <code>pointer_pool</code> remains the preferred structure for objects that cannot be
 * relocated or keyed externally and must be deduplicated solely through their own equality semantics.
 * </p>
 *
 * @version <pre>1.4.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include <atomic>           // NOLINT for std::atomic
#include <cstdint>          // for std::uint64_t
#include <algorithm>        // for std::copy_if
#include <vector>           // for std::vector
#include <unordered_set>    // for std::unordered_set
#include <mutex>            // For std::shared_mutex
#include <memory>
#include <shared_mutex>


namespace jh::conc {

    /**
     * @brief Weak pointer–observed pool for immutable or structurally immutable objects.
     *
     * <h4>Core Behavior</h4>
     * <ol>
     *   <li>Objects are always constructed first using the forwarded arguments.</li>
     *   <li>The pool lock is acquired only when attempting insertion.</li>
     *   <li>If an equivalent object already exists, that instance is returned and the temporary is discarded.</li>
     *   <li>If no match exists, the temporary becomes the canonical instance stored in the pool.</li>
     * </ol>
     * <p>
     * This construct-first, lock-then-insert pattern is required because lookup is defined by the
     * object's own equality; no external key or pre-hash structure can be used.
     * </p>
     *
     * <h4>Design Characteristics</h4>
     * <ul>
     *   <li><b>Non-intrusive:</b> The pool never owns objects; it records only <code>std::weak_ptr</code>
     *       while ownership remains external.</li>
     *   <li><b>Deferred cleanup:</b> Expired entries are removed only during insertion, capacity checks,
     *       or explicit cleanup calls.</li>
     *   <li><b>Adaptive capacity:</b> The container may grow or shrink depending on occupancy thresholds
     *       evaluated during insertion.</li>
     *   <li><b>Thread-safe:</b> Lookups and insertions coordinate through <code>std::shared_mutex</code>.</li>
     *   <li><b>Discard-friendly:</b> Temporary objects are cheap to abandon when a matching instance exists.</li>
     * </ul>
     *
     * <h4>Usage Notes</h4>
     * <ul>
     *   <li>Best suited for immutable or structurally immutable types whose identity is fully determined
     *       at construction.</li>
     *   <li>For heavier objects, prefer a two-phase initialization pattern: construct only identity fields
     *       immediately and defer expensive setup until the object becomes the accepted instance.</li>
     *   <li>Fields contributing to equality and hashing must remain constant while managed by the pool.</li>
     * </ul>
     *
     * <h4>Concurrency and Safety</h4>
     * <ul>
     *   <li>Concurrent calls to <code>acquire()</code> are safe.</li>
     *   <li>Insertion and deduplication are atomic under exclusive locking.</li>
     *   <li>Externally held <code>std::shared_ptr</code> objects remain valid even if the pool is cleared
     *       or destroyed.</li>
     * </ul>
     *
     * @note
     * <p>
     * Hash and equality functors need only reflect object-level identity. When using
     * <code>jh::observe_pool</code>, these are automatically derived from <code>std::hash&lt;T&gt;()</code> or
     * adl <code>hash(t)</code> or <code>t.hash()</code>, and <code>operator==()</code> to ensure consistent behavior.
     * </p>
     *
     * @warning
     * <p>
     * On Windows environments based on the Universal CRT (including MinGW variants),
     * <code>std::shared_ptr</code> and <code>std::weak_ptr</code> may exhibit incorrect reference-count
     * synchronization under high concurrency. As a result, <code>weak_ptr::lock()</code> may succeed
     * against an object whose underlying <code>shared_ptr</code> has already been destroyed, leading to
     * invalid access or crashes even under otherwise correct usage. Additionally, insertion of
     * <code>std::weak_ptr</code> into <code>std::unordered_*</code> containers on these platforms incurs
     * significant jitter.</p>
     * <p>
     * Due to these platform-specific defects, high-pressure concurrent use of
     * <code>pointer_pool</code> is not recommended on Windows UCRT-based toolchains.</p>
     */
    template<typename T, typename Hash, typename Eq>
    requires(
        requires(const std::weak_ptr<T>& t) {
            { Hash{}(t) } -> std::convertible_to<size_t>;
        } &&
        requires(const std::weak_ptr<T>& a, const std::weak_ptr<T>& b) {
            { Eq{}(a, b) } -> std::convertible_to<bool>;
        })
    class pointer_pool final {
    public:

        /**
         * @brief The minimum reserved capacity for the pool.
         *
         * Defines the lower bound of the adaptive capacity management system.
         * The pool will never shrink below this threshold even when mostly empty,
         * ensuring predictable memory usage and avoiding excessive reallocation.
         *
         * @note
         * This value is also used as the default reserve size when constructing a new pool.
         */
        static std::uint64_t constexpr MIN_RESERVED_SIZE = 16;

        /**
         * @brief Constructs a pool with an initial reserved capacity.
         *
         * @param reserve_size The initial capacity to reserve for the internal hash set.
         *                     Defaults to <code>MIN_RESERVED_SIZE</code> (<tt>16</tt>).
         *
         * Initializes the pool's internal storage and establishes the adaptive
         * resizing baseline. This constructor performs no object construction;
         * it only reserves memory for the underlying <code>std::unordered_set</code>
         * that stores weak references.
         *
         * @note
         * The reserved size determines the initial hash set capacity and defines
         * the minimum capacity threshold for future adaptive resizing.
         * The pool will never shrink below <code>MIN_RESERVED_SIZE</code>
         * (<tt>16</tt>), ensuring predictable allocation behavior and avoiding
         * frequent reallocation during low-load periods.
         */
        explicit pointer_pool(std::uint64_t reserve_size = MIN_RESERVED_SIZE)
                : capacity_(reserve_size) {
            pool_.reserve(capacity_.load());
        }

        /**
         * @brief Deleted copy constructor.
         *
         * Copying is disabled because two pools observing the same set of
         * immutable objects have no meaningful deduplication relationship.
         * A duplicated observer would only increase contention and break the
         * one-pool-per-type principle for resource or handle management.
         *
         * @note
         * For structurally immutable resource types, only one pool should exist
         * in the entire program. For simple data deduplication (e.g. shared strings),
         * threads can directly share <code>std::reference_wrapper</code> or
         * <code>shared_ptr</code> instead of copying the pool.
         */
        pointer_pool(const pointer_pool &) = delete;

        /**
         * @brief Deleted copy assignment operator.
         *
         * Copy assignment is not supported for the same reasons as copy construction:
         * duplicating the pool would create two independent observers of the same
         * logical object space, defeating deduplication semantics.
         */
        pointer_pool &operator=(const pointer_pool &) = delete;

        /**
         * @brief Move constructor.
         *
         * Transfers internal weak references and capacity state from another pool.
         * The source pool is locked during transfer and cleared afterward to ensure
         * a valid empty state.
         *
         * @note
         * Moving a pool transfers observation authority, not ownership of objects.
         * Since the pool tracks objects via <code>weak_ptr</code>, live instances
         * remain valid and unaffected. After a move, entries may temporarily exist
         * in both the old and new pool, but this is acceptable for deduplication use.
         */
        pointer_pool(pointer_pool &&other) noexcept {
            std::unique_lock write_lock(other.pool_mutex_);
            pool_ = std::move(other.pool_);
            capacity_.store(other.capacity_.load());
            other.pool_.clear();  // Ensure valid empty state after move.
        }

        /**
         * @brief Move assignment operator.
         *
         * Moves the weak reference set and capacity state from another pool.
         * Both pools are locked during transfer to ensure atomicity.
         *
         * @note
         * During move assignment, all existing weak references in the current pool
         * are released and replaced by those from the source pool.
         * This only removes the pool's <em>observation</em> of those objects &mdash;
         * their actual lifetimes remain intact because ownership is held by
         * external <code>std::shared_ptr</code> instances.
         *
         * Moving represents transfer of observation scope. Since deduplication
         * is tolerant to transient duplicates, the existence of similar entries
         * in both pools after move is not a correctness issue.
         * The moved-from pool is cleared and remains valid for later reuse.
         *
         * @warning
         * For structurally immutable resource or handle pools, move assignment
         * is not semantically appropriate, as such pools are expected to be unique
         * within their management domain.
         */
        pointer_pool &operator=(pointer_pool &&other) noexcept {
            if (this != &other) {
                std::lock_guard lock_this(pool_mutex_);
                std::lock_guard lock_other(other.pool_mutex_);
                pool_ = std::move(other.pool_);
                capacity_.store(other.capacity_.load());
                other.pool_.clear();  // Ensure safe use after move.
            }
            return *this;
        }

        /**
         * @brief Deleted <code>acquire()</code> for <code>const pointer_pool</code>.
         *
         * Prevents acquiring or inserting objects through a constant pool reference.
         * Since acquisition may modify the internal pool state, it is not permitted
         * on <code>const</code> instances.
         *
         * @tparam Args Constructor argument types for <code>T</code>.
         *
         * @note
         * This overload exists purely to provide a compile-time diagnostic when
         * <code>acquire()</code> is accidentally called on a constant pool reference.
         * Immutable access to existing shared objects should be performed through
         * previously returned <code>std::shared_ptr</code> instances.
         */
        template<typename... Args>
        [[maybe_unused]] std::shared_ptr<T> acquire(Args &&... args) const = delete;

        /**
         * @brief Retrieves an object from the pool, or creates a new one if none exists.
         *
         * Constructs a temporary instance of <code>T</code> using the forwarded arguments,
         * then attempts to insert it into the pool.
         * If a logically equivalent instance already exists (as determined by <code>Eq</code>),
         * it is reused and the newly created temporary object is discarded.
         * Otherwise, the new instance is inserted and returned.
         *
         * @tparam Args Constructor argument types for <code>T</code>.
         * @param args Arguments forwarded to <code>T</code>'s constructor.
         * @return A <code>std::shared_ptr&lt;T&gt;</code> representing the pooled object.
         *
         * <p><b>Acquisition Flow:</b></p>
         * <ol>
         *   <li>A new object is tentatively constructed using the forwarded arguments.</li>
         *   <li>The pool lock is acquired only during insertion and lookup.</li>
         *   <li>If a logically equivalent instance already exists, it is reused &mdash;
         *       the temporary object is immediately discarded.</li>
         *   <li>If not found, the new object is inserted and its <code>shared_ptr</code> returned.</li>
         * </ol>
         *
         * @note
         * The pool employs a construct-first, lock-then-insert model.
         * This avoids holding the pool lock during object construction,
         * enabling support for non-copyable or non-movable types.
         * Temporary objects may be discarded if an equivalent instance already exists,
         * so types should support lightweight provisional construction
         * (e.g. lazy initialization of heavy internal resources).
         */
        template<typename... Args>
        std::shared_ptr<T> acquire(Args &&... args) {
            auto new_obj = std::make_shared<T>(std::forward<Args>(args)...);
            return get_or_insert(new_obj);
        }

        /**
         * @brief Removes expired weak references from the pool.
         *
         * Scans the internal container and erases all <code>weak_ptr</code> entries
         * that have expired (that is, their corresponding <code>shared_ptr</code> instances
         * have been destroyed).
         * <p>
         * This operation reclaims hash table slots and prevents unbounded growth
         * when many pooled objects are released.
         * </p>
         *
         * @note
         * This function is safe to call at any time and is intended for
         * <b>manual maintenance</b>.
         * Automatic cleanup also occurs opportunistically during insertion or
         * expansion when capacity thresholds are reached.
         */
        void cleanup() {
            std::lock_guard<std::shared_mutex> lock(pool_mutex_);
            cleanup_no_lock();
        }

        /**
         * @brief Removes expired entries and conditionally shrinks the reserved capacity.
         *
         * Performs the same expired-entry cleanup as <code>cleanup()</code>,
         * then evaluates the current usage ratio to determine whether
         * capacity should be reduced.
         *
         * <p>
         * If the number of active entries falls below <tt>0.25</tt> of the current
         * reserved size (the low-watermark ratio), the reserved capacity is reduced
         * to one half of its previous value.
         * The pool will never shrink below <code>MIN_RESERVED_SIZE</code>.
         * </p>
         *
         * @note
         * <ul>
         *   <li>Both manual and automatic shrinkage follow the same rule:
         *       capacity is reduced by half instead of being minimized to fit
         *       the current usage exactly.</li>
         *   <li>This conservative policy prevents oscillation between expansion
         *       and contraction when workload size fluctuates,
         *       reducing allocation jitter.</li>
         *   <li>Since a previously expanded pool indicates historically higher load,
         *       shrinking only halfway preserves readiness for future reuse
         *       without significant memory overhead.</li>
         * </ul>
         *
         * This function is intended for <b>manual maintenance</b> when predictable
         * memory release is desired. It complements the automatic,
         * event-driven resizing that may also perform half shrinkage
         * during expansion checks.
         */
        void cleanup_shrink() {
            std::lock_guard<std::shared_mutex> lock(pool_mutex_);
            cleanup_no_lock();

            auto current_size = pool_.size();
            auto current_reserved = capacity_.load();

            const auto low_watermark =
                    static_cast<std::uint64_t>(static_cast<double>(current_reserved) * LOW_WATERMARK_RATIO);

            if (current_size <= low_watermark) {
                capacity_.store(std::max(current_reserved / 2, MIN_RESERVED_SIZE));
            }
        }

        /**
         * @brief Gets the current number of elements in the pool.
         * @return The number of stored weak_ptrs (including expired ones).
         */
        [[nodiscard]] std::uint64_t size() const {
            std::shared_lock read_lock(pool_mutex_);
            return pool_.size();
        }

        /**
         * @brief Gets the current reserved size of the pool.
         * @return The reserved size limit before expansion or contraction.
         */
        [[nodiscard]] std::uint64_t capacity() const {
            return capacity_.load();
        }

        /**
         * @brief Clears all entries and resets the pool to its initial state.
         *
         * <p>
         * Removes all elements from the internal container and resets
         * <code>capacity_</code> to <code>MIN_RESERVED_SIZE</code>.
         * This operation is functionally equivalent to <code>clear()</code>
         * on standard containers, but is <strong>thread-safe</strong> and
         * ensures consistent internal state for concurrent environments.
         * </p>
         *
         * <p>
         * Because the pool only stores <code>weak_ptr</code> references,
         * clearing it merely removes observation records and does not affect
         * the lifetime of externally held <code>shared_ptr</code> instances.
         * For immutable data types, deduplication integrity remains intact.
         * </p>
         *
         * @note
         * <ul>
         *   <li>For <strong>structurally immutable resource or handle pools</strong>,
         *       calling <code>clear()</code> is not recommended, as it abandons
         *       tracking of active handles and may cause side effects.</li>
         *   <li>After clearing, <code>capacity()</code> is reset to
         *       <code>MIN_RESERVED_SIZE</code>, fully restoring the pool to
         *       its initial baseline.</li>
         *   <li>Unlike move operations, which preserve capacity to prevent
         *       unnecessary re-expansion, <code>clear()</code> always resets
         *       the capacity to its minimum for a deterministic clean state.</li>
         * </ul>
         */
        void clear() {
            std::unique_lock write_lock(pool_mutex_);
            pool_.clear();
            capacity_.store(MIN_RESERVED_SIZE);
        }

    private:
        std::unordered_set<std::weak_ptr<T>, Hash, Eq> pool_; ///< Storage for weak_ptr objects.
        std::atomic<std::uint64_t> capacity_; ///< The dynamically managed reserved size.
        mutable std::shared_mutex pool_mutex_; ///< Ensures thread-safe access.

        /**
         * @brief Inserts a shared object into the pool or retrieves an existing equivalent one.
         *
         * Checks whether an equivalent object (as determined by <code>Eq</code>)
         * already exists in the pool. If found, returns a <code>shared_ptr</code>
         * to the existing instance; otherwise inserts the new one and returns it.
         *
         * <p>
         * This function acquires an exclusive lock for insertion. If the pool is
         * near its capacity limit, <code>expand_and_cleanup()</code> is invoked
         * beforehand to perform opportunistic cleanup or resizing.
         * </p>
         *
         * @param obj The <code>shared_ptr&lt;T&gt;</code> to insert or match.
         * @return A <code>shared_ptr&lt;T&gt;</code> to the existing or newly inserted object.
         */
        std::shared_ptr<T> get_or_insert(const std::shared_ptr<T> &obj) {
            if (pool_.size() >= capacity_.load()) {
                expand_and_cleanup(); // This function is already acquiring the lock.
            }
            std::unique_lock write_lock(pool_mutex_); // Lock for pool access.

            auto [it, inserted] = pool_.insert(obj);
            if (!inserted) return it->lock();
            return obj;
        }

        /// @brief Internal cleanup without locking
        void cleanup_no_lock() {
            // Step 1: Collect valid weak_ptrs using STL algorithms
            std::vector<std::weak_ptr<T>> valid_elements;
            valid_elements.reserve(pool_.size());

            std::copy_if(pool_.begin(), pool_.end(), std::back_inserter(valid_elements),
                         [](const std::weak_ptr<T> &w_ptr) { return !w_ptr.expired(); });

            // Step 2: Safely replace `pool_` inside the lock
            pool_.clear();
            pool_.insert(std::make_move_iterator(valid_elements.begin()),
                         std::make_move_iterator(valid_elements.end()));
        }

        static constexpr double HIGH_WATERMARK_RATIO = 0.875; ///< Expand if usage exceeds 87.5%
        static constexpr double LOW_WATERMARK_RATIO = 0.25;  ///< Shrink if usage falls below 25%

        /**
         * @brief Performs opportunistic cleanup and adaptive resizing.
         *
         * <p>
         * Invoked internally when the pool approaches its capacity limit.
         * This function first removes expired entries, then decides whether
         * to expand or shrink the reserved capacity based on current usage
         * ratios.
         * </p>
         *
         * <p><strong>Cleanup Phase</strong></p>
         * <p>
         * Before any resizing, <code>cleanup_no_lock()</code> is called to purge
         * expired <code>weak_ptr</code> entries. This ensures that capacity decisions
         * are based on live objects only.
         * </p>
         *
         * <p><strong>Resizing Logic</strong></p>
         * <p>
         * After cleanup, the pool evaluates usage against two thresholds:
         * <ul>
         *   <li><b>High-watermark:</b> <tt>0.875</tt> &mdash; expansion trigger.</li>
         *   <li><b>Low-watermark:</b> <tt>0.25</tt> &mdash; shrink trigger.</li>
         * </ul>
         * If the active entry count exceeds <tt>87.5%</tt> of capacity or the
         * reserved limit itself, the pool doubles its capacity.
         * If it falls below <tt>25%</tt>, the capacity is halved (but never below
         * <code>MIN_RESERVED_SIZE</code>).
         * </p>
         *
         * <p><strong>Why High-Watermark Is Below 1.0</strong></p>
         * <p>
         * The high-watermark is deliberately set below 1.0 to avoid oscillation
         * when operating near full capacity. If the threshold were exactly 1.0,
         * large pools might repeatedly reach their limit even after cleanup,
         * leading to expansion attempts on nearly every insertion.
         * </p>
         * <p>
         * Setting the threshold to <strong>87.5%</strong> provides a safety margin:
         * normal cleanup can reclaim space without immediately retriggering expansion,
         * reducing jitter and lock contention under heavy load.
         * </p>
         *
         * <p><strong>Behavior Characteristics</strong></p>
         * <ul>
         *   <li>Both expansion and shrinkage are <strong>gradual</strong> &mdash; capacity
         *       changes by doubling or halving, avoiding aggressive reallocation.</li>
         *   <li>The pool never shrinks below <code>MIN_RESERVED_SIZE</code>,
         *       maintaining a stable baseline.</li>
         *   <li>This function serves as the unified adaptive management mechanism
         *       used by both external and internal cleanup triggers.</li>
         * </ul>
         */
        void expand_and_cleanup() {
            std::lock_guard<std::shared_mutex> lock(pool_mutex_);
            cleanup_no_lock();

            auto current_size = pool_.size();
            auto current_reserved = capacity_.load();

            const auto high_watermark =
                    static_cast<std::uint64_t>(static_cast<double >(current_reserved) * HIGH_WATERMARK_RATIO);
            const auto low_watermark =
                    static_cast<std::uint64_t>(static_cast<double >(current_reserved) * LOW_WATERMARK_RATIO);

            if (current_size >= current_reserved || current_size >= high_watermark) {
                // Expand if size exceeds the limit or crosses the high watermark.
                capacity_.store(current_reserved * 2);
            } else if (current_size <= low_watermark) {
                // Shrink if size falls below the low watermark.
                capacity_.store(std::max(current_reserved / 2, MIN_RESERVED_SIZE));
            }
        }
    };

} // namespace jh::conc
