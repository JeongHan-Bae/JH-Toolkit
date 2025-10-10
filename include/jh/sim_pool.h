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
 * @file sim_pool.h
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 * @brief Smart Immutable-objects Managing Pool — lightweight, non-intrusive pooling for shared immutable objects.
 *
 * <h3>Overview</h3>
 * <p>
 * <code>jh::sim_pool</code> (short for <b>Smart Immutable-objects Managing Pool</b>,
 * pronounced similar to <em>"simple"</em>) is a <b>weak_ptr-observed</b> pool that manages
 * shared instances of immutable or structurally immutable objects.
 * It deduplicates logically equivalent objects while ensuring that
 * externally held <code>std::shared_ptr</code> instances remain fully valid
 * even after the pool itself is destroyed.
 * </p>
 *
 * <p>
 * Typical use cases include:
 * <ul>
 *   <li><b>Shared data</b> — e.g. <code>jh::immutable_str</code>, a read-only string type safely shareable across threads.</li>
 *   <li><b>Handle-like or resource objects</b> — where the identity-defining fields are immutable
 *       (e.g. texture handles, GPU resources, database tokens), while internal state may remain mutable.
 *       The user is responsible for synchronizing mutable access.</li>
 * </ul>
 * </p>
 *
 * <h4>Design Rationale</h4>
 * <p>
 * Unlike conventional object pools that register destructors or manage ownership,
 * <code>sim_pool</code> treats the pool as a <b>pure observer</b> of shared ownership.
 * All objects are owned by <code>std::shared_ptr</code>, and the pool only maintains
 * <code>std::weak_ptr</code> references for lookup and deduplication.
 * </p>
 *
 * <h4>Why <code>std::weak_ptr</code></h4>
 * <ul>
 *   <li>The pool never owns its elements — destruction order is irrelevant.</li>
 *   <li>If the pool is destroyed first, live <code>shared_ptr</code> objects outside remain valid and functional.</li>
 *   <li>Shared instances are guaranteed to be unique: any logically equivalent object
 *       constructed later will resolve to the same shared instance through the pool.</li>
 *   <li>Insertion and replacement are atomic and race-safe under shared mutex protection.</li>
 * </ul>
 *
 * <h4>Behavioral Flow</h4>
 * <ol>
 *   <li>A new object is tentatively constructed (with forwarded arguments).</li>
 *   <li>The pool lock is acquired only when attempting insertion.</li>
 *   <li>If a logically equivalent instance already exists, it is reused —
 *       the newly constructed temporary object is immediately discarded.</li>
 *   <li>If not found, the new object is inserted and its <code>shared_ptr</code> returned.</li>
 * </ol>
 *
 * <p>
 * This approach minimizes long-term locking and avoids lifetime coupling,
 * allowing construction of even non-copyable, non-movable types
 * such as <code>immutable_str</code>.
 * </p>
 *
 * <p>
 * <b>Best Practice:</b>  
 * Because <code>sim_pool</code> adopts a <b>construct-first, lock-then-insert</b> strategy
 * — rather than holding the lock throughout construction —
 * objects should support <b>low-cost provisional construction</b>.
 * That is, temporary instances may be created and discarded if an equivalent
 * object already exists in the pool.
 * </p>
 *
 * <p>
 * For <em>structurally immutable</em> objects (e.g., handles or resource wrappers whose
 * identity is fixed but internal data may be initialized later),
 * a recommended pattern is <b>lazy initialization</b>:
 * <ul>
 *   <li>Construct only the immutable identity fields first — the parts used in hashing and equality.</li>
 *   <li>Defer any heavy or mutable setup until first use, guarded by <code>std::once_flag</code> or similar.</li>
 * </ul>
 * This model aligns with the pool's design choice: since insertion is a single
 * hash-based lookup under a short lock, objects must be safely discardable
 * without incurring significant cost if already present.
 * </p>
 * 
 * <h4>Cleanup Model</h4>
 * <ul>
 *   <li><b>Attempt-based cleanup</b> — expired entries are removed automatically only
 *       when insertion or expansion triggers capacity checks, or when
 *       <code>cleanup()</code> / <code>cleanup_shrink()</code> are explicitly invoked.</li>
 *
 *   <li><b>Non-aggressive reclamation</b> — the pool deliberately avoids immediate or
 *       continuous shrinkage to prevent allocation jitter during high-frequency reuse.
 *       Cleanup is opportunistic and event-driven, never periodic.</li>
 *
 *   <li><b>Adaptive resizing</b> — during expansion attempts, the pool first performs
 *       cleanup and then decides whether to resize:
 *       <ul>
 *         <li>If the live entry count still exceeds the high-watermark threshold,
 *             capacity is doubled.</li>
 *         <li>If cleanup reveals significant vacancy (below the low-watermark threshold),
 *             the capacity may be reduced — even though triggered by expansion logic.</li>
 *       </ul>
 *       External manual calls to <code>cleanup_shrink()</code> are also supported
 *       when predictable memory release is desired.</li>
 * </ul>
 *
 * <h4>Immutability Requirement</h4>
 * <p>
 * Objects stored in <code>sim_pool</code> must be immutable — or at least
 * <b>partially immutable</b> such that all fields affecting hashing and equality
 * remain constant throughout their lifetime.
 * </p>
 *
 * <h4>Comparison with <code>jh::pool</code></h4>
 * <ul>
 *   <li><code>sim_pool</code> requires explicit registration of <code>Hash</code> and <code>Eq</code> functors.</li>
 *   <li><code>jh::pool&lt;T&gt;</code> builds upon <code>sim_pool</code>,
 *       automatically deducing hash/equality semantics for duck-typed types
 *       exposing <code>hash()</code> and <code>operator==()</code>.</li>
 *   <li><code>sim_pool</code> is the generic foundation — flexible, type-agnostic,
 *       and minimal in dependency.</li>
 * </ul>
 *
 * <h4>Key Advantages</h4>
 * <ul>
 *   <li>Stable sharing — live <code>shared_ptr</code>s outlive the pool safely.</li>
 *   <li>Zero registration overhead — no intrusive hooks or custom deleters.</li>
 *   <li>Behavior-triggered cleanup — avoids timing-based management threads.</li>
 *   <li>Thread-safe — concurrent reads and atomic insertion under shared mutex.</li>
 *   <li>Ideal for high-frequency interning or immutable resource sharing.</li>
 * </ul>
 *
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include <atomic>           // NOLINT for std::atomic
#include <cstdint>          // for std::uint64_t
#include <algorithm>        // for std::copy_if
#include <vector>           // for std::vector
#include <unordered_set>    // for std::unordered_set
#include <mutex>            // For std::shared_mutex
#include <shared_mutex>


namespace jh {

    /**
     * @brief Weak pointer–observed object pool for immutable or structurally immutable objects.
     *
     * @tparam T   Object type stored in the pool. Must be immutable, or at least
     *             partially immutable such that fields affecting hashing and equality
     *             remain constant during its lifetime.
     * @tparam Hash  Custom hashing functor defining content-based identity.
     * @tparam Eq    Custom equality functor for comparing two weak pointers by
     *               the underlying object's logical equality.
     *
     * @note
     * Hash and Eq must operate on object content rather than pointer addresses.
     * Expired weak pointers should be treated as distinct to allow deferred cleanup.
     * Types managed by sim_pool are required to be immutable or structurally immutable.
     *
     * <h4>Core Behavior</h4>
     * <ol>
     *   <li>Objects are constructed first (with forwarded arguments).</li>
     *   <li>The pool lock is acquired only when attempting insertion.</li>
     *   <li>If an equivalent object exists, it is reused and the temporary is discarded.</li>
     *   <li>If not found, the new object is inserted and returned.</li>
     * </ol>
     *
     * <h4>Design Characteristics</h4>
     * <ul>
     *   <li><b>Non-intrusive:</b> The pool never owns elements; it only observes
     *       <code>std::shared_ptr</code> lifetimes.</li>
     *   <li><b>Event-driven cleanup:</b> Expired entries are purged only
     *       during insertion or explicit cleanup requests.</li>
     *   <li><b>Adaptive resizing:</b> Capacity expands or contracts based on
     *       occupancy thresholds during expansion checks.</li>
     *   <li><b>Thread-safe:</b> Uses <code>std::shared_mutex</code> for concurrent
     *       read and exclusive write access.</li>
     *   <li><b>Discard-friendly:</b> Temporary constructions are cheap to discard,
     *       aligning with the construct-first, lock-then-insert model.</li>
     * </ul>
     *
     * <h4>Usage Notes</h4>
     * <ul>
     *   <li>For immutable data objects such as <code>immutable_str</code>, construct
     *       directly using <code>acquire(...)</code>.</li>
     *   <li>For handle-like or resource objects, prefer lazy initialization:
     *       build only immutable identity fields on construction, and defer heavy setup
     *       to the first use (for example guarded by <code>std::once_flag</code>).</li>
     *   <li>All fields contributing to hash and equality must remain constant
     *       while the object is managed by the pool.</li>
     * </ul>
     *
     * <h4>Concurrency and Safety</h4>
     * <ul>
     *   <li>Multiple threads may safely call <code>acquire()</code> concurrently.</li>
     *   <li>Insertion and replacement are atomic under <code>std::shared_mutex</code>.</li>
     *   <li>Externally held <code>shared_ptr</code> instances remain valid even after
     *       the pool is cleared or destroyed.</li>
     * </ul>
     *
     * @note
     * <code>jh::sim_pool</code> is the low-level foundation for <code>jh::pool</code> and similar components;
     * it provides deduplicated sharing, predictable cleanup, and minimal coupling
     * between object lifetime and pool existence.
     */
    template<typename T, typename Hash, typename Eq>
    class sim_pool {
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
        explicit sim_pool(const std::uint64_t reserve_size = MIN_RESERVED_SIZE)
            : reserved_size_(reserve_size) {
            pool_.reserve(reserved_size_.load());
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
        sim_pool(const sim_pool &) = delete;

        /**
         * @brief Deleted copy assignment operator.
         *
         * Copy assignment is not supported for the same reasons as copy construction:
         * duplicating the pool would create two independent observers of the same
         * logical object space, defeating deduplication semantics.
         */
        sim_pool &operator=(const sim_pool &) = delete;

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
        sim_pool(sim_pool &&other) noexcept {
            std::unique_lock write_lock(other.pool_mutex_);
            pool_ = std::move(other.pool_);
            reserved_size_.store(other.reserved_size_.load());
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
         * This only removes the pool's <em>observation</em> of those objects —
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
        sim_pool &operator=(sim_pool &&other) noexcept {
            if (this != &other) {
                std::lock_guard lock_this(pool_mutex_);
                std::lock_guard lock_other(other.pool_mutex_);
                pool_ = std::move(other.pool_);
                reserved_size_.store(other.reserved_size_.load());
                other.pool_.clear();  // Ensure safe use after move.
            }
            return *this;
        }

        /**
         * @brief Deleted <code>acquire()</code> for <code>const sim_pool</code>.
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
         *   <li>If a logically equivalent instance already exists, it is reused —
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
            cleanup_nolock();
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
            cleanup_nolock();

            auto current_size     = pool_.size();
            auto current_reserved = reserved_size_.load();

            const auto low_watermark =
                    static_cast<std::uint64_t>(static_cast<double>(current_reserved) * LOW_WATERMARK_RATIO);

            if (current_size <= low_watermark) {
                reserved_size_.store(std::max(current_reserved / 2, MIN_RESERVED_SIZE));
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
        [[nodiscard]] std::uint64_t reserved_size() const {
            return reserved_size_.load();
        }

        /**
         * @brief Clears all entries and resets the pool to its initial state.
         *
         * <p>
         * Removes all elements from the internal container and resets
         * <code>reserved_size_</code> to <code>MIN_RESERVED_SIZE</code>.
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
         *   <li>After clearing, <code>reserved_size()</code> is reset to
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
            reserved_size_.store(MIN_RESERVED_SIZE);
        }

    private:
        std::unordered_set<std::weak_ptr<T>, Hash, Eq> pool_; ///< Storage for weak_ptr objects.
        std::atomic<std::uint64_t> reserved_size_; ///< The dynamically managed reserved size.
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
            if (pool_.size() >= reserved_size_.load()) {
                expand_and_cleanup(); // This function is already acquiring the lock.
            }
            std::unique_lock write_lock(pool_mutex_); // Lock for pool access.

            auto [it, inserted] = pool_.insert(obj);
            if (!inserted) return it->lock();
            return obj;
        }

        /// @brief Internal cleanup without locking
        void cleanup_nolock() {
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
        static constexpr double LOW_WATERMARK_RATIO  = 0.25;  ///< Shrink if usage falls below 25%

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
         * Before any resizing, <code>cleanup_nolock()</code> is called to purge
         * expired <code>weak_ptr</code> entries. This ensures that capacity decisions
         * are based on live objects only.
         * </p>
         *
         * <p><strong>Resizing Logic</strong></p>
         * <p>
         * After cleanup, the pool evaluates usage against two thresholds:
         * <ul>
         *   <li><b>High-watermark:</b> <tt>0.875</tt> — expansion trigger.</li>
         *   <li><b>Low-watermark:</b> <tt>0.25</tt> — shrink trigger.</li>
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
         *   <li>Both expansion and shrinkage are <strong>gradual</strong> — capacity
         *       changes by doubling or halving, avoiding aggressive reallocation.</li>
         *   <li>The pool never shrinks below <code>MIN_RESERVED_SIZE</code>,
         *       maintaining a stable baseline.</li>
         *   <li>This function serves as the unified adaptive management mechanism
         *       used by both external and internal cleanup triggers.</li>
         * </ul>
         */
        void expand_and_cleanup() {
            std::lock_guard<std::shared_mutex> lock(pool_mutex_);
            cleanup_nolock();

            auto current_size     = pool_.size();
            auto current_reserved = reserved_size_.load();

            const auto high_watermark =
                    static_cast<std::uint64_t>(static_cast<double >(current_reserved) * HIGH_WATERMARK_RATIO);
            const auto low_watermark =
                    static_cast<std::uint64_t>(static_cast<double >(current_reserved) * LOW_WATERMARK_RATIO);

            if (current_size >= current_reserved || current_size >= high_watermark) {
                // Expand if size exceeds the limit or crosses the high watermark.
                reserved_size_.store(current_reserved * 2);
            } else if (current_size <= low_watermark) {
                // Shrink if size falls below the low watermark.
                reserved_size_.store(std::max(current_reserved / 2, MIN_RESERVED_SIZE));
            }
        }
    };
} // namespace jh
