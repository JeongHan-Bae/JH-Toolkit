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
 * @file flat_pool.h
 *
 * @brief Key-based, contiguous, GC-like interning pool for copyable or movable objects.
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 *
 * <h3>Overview</h3>
 * <p>
 * <code>jh::conc::flat_pool</code> is a concurrent object interning container that deduplicates
 * objects using an explicit external <b>key</b> and stores them inside a contiguous memory pool.
 * Each unique key corresponds to at most one active slot at any time, and acquisitions return
 * lightweight reference-counted handles (<code>flat_pool::ptr</code>) bound to stable indices.
 * </p>
 *
 * <p>
 * Unlike pointer-based interning containers, <code>flat_pool</code> does <b>not</b> rely on
 * <code>std::shared_ptr</code> for ownership, synchronization, or lifetime control. All concurrency
 * guarantees are enforced exclusively through the pool's internal locking strategy. As a result,
 * the behavior of the pool is independent of platform-specific <code>shared_ptr</code>
 * implementations (including Windows-specific locking behavior).
 * </p>
 *
 * <h3>Key-Based Identity Model</h3>
 * <p>
 * Object identity is defined entirely by an external <code>Key</code> type. The key must be:
 * </p>
 * <ul>
 *   <li>lightweight to construct,</li>
 *   <li>cheap to hash and compare,</li>
 *   <li>capable of representing object identity independently of object storage.</li>
 * </ul>
 *
 * <p>
 * Lookup and deduplication are performed solely through the key. The stored object itself is
 * never inspected for equality. This allows the pool to perform lookups <em>before</em>
 * construction and avoids provisional object creation.
 * </p>
 *
 * <p>
 * Keys are accepted in cv/ref-qualified form during acquisition. These equivalent key
 * representations are used only for identification and are not retained unless insertion
 * succeeds.
 * </p>
 *
 * <h3>Value Construction Model</h3>
 * <p>
 * For map-like pools (<code>Value != jh::typed::monostate</code>), values are constructed using
 * a <code>std::tuple</code> of arguments supplied at acquisition time.
 * </p>
 *
 * <ul>
 *   <li>The argument tuple is treated as <b>initialization-only</b> data.</li>
 *   <li>The argument tuple is not a complete data-tuple, but rather a left-value-tuple or perfectly-forwarded-tuple
 *       bound via <code>std::tie</code> or <code>std::forward_as_tuple</code>.</li>
 *   <li>If an equivalent key already exists, the tuple is discarded without constructing a value.</li>
 *   <li>Value construction occurs exactly once per unique key.</li>
 * </ul>
 *
 * <p>
 * Value creation is customizable via the public extension point
 * <code>jh::conc::extension::value_factory&lt;Value&gt;</code>, enabling user-defined construction
 * strategies without modifying or subclassing the pool.
 * </p>
 *
 * <h3>GC-like Lifetime Semantics</h3>
 * <p>
 * <code>flat_pool</code> deliberately adopts a <b>GC-like</b> lifetime model:
 * </p>
 *
 * <ul>
 *   <li>Reference counting represents <em>liveness</em>, not destruction.</li>
 *   <li>When a slot's reference count drops to zero, the slot becomes reusable.</li>
 *   <li>Objects are <b>not</b> destroyed immediately when they become unused.</li>
 * </ul>
 *
 * <p>
 * Instead of invoking destructors eagerly, the pool prefers slot reuse through assignment.
 * This avoids repeated destructor / constructor cycles and significantly reduces allocation
 * pressure for objects whose initialization may involve expensive memory allocation or
 * resource setup.
 * </p>
 *
 * <p>
 * Because destruction is deferred and non-deterministic, <code>flat_pool</code> is not suitable
 * for objects whose correctness depends on immediate destruction when references are released.
 * </p>
 *
 * <p>
 * In a manner of speaking, this design can be viewed as transforming <code>shared_ptr</code>, which
 * performs allocation individually, into a bunch of smart pointers that allocate keys, values, index tables,
 * and control blocks all in contiguous memory managed by the pool.
 * </p>
 *
 * <h3>Concurrency Model</h3>
 * <p>
 * All synchronization is handled internally by the pool using shared and exclusive locks.
 * No external atomic or smart-pointer-level synchronization is relied upon.
 * </p>
 *
 * <ul>
 *   <li>Lookup operations acquire shared locks only.</li>
 *   <li>Insertion, release, and resizing acquire exclusive locks.</li>
 *   <li>Reference counts are maintained using atomics.</li>
 * </ul>
 *
 * <p>
 * The pool stores objects in contiguous memory. To protect against vector reallocation during
 * concurrent access, dereferencing a handle in multithreaded contexts requires holding a
 * <code>no_reallocate_guard</code>.
 * </p>
 *
 * <h3>Comparison with <code>pointer_pool</code></h3>
 * <p>
 * <code>flat_pool</code> and <code>pointer_pool</code> address complementary problem domains:
 * </p>
 *
 * <ul>
 *   <li>
 *     <b><code>flat_pool</code></b>:
 *     <ul>
 *       <li>Key-driven identity</li>
 *       <li>Contiguous storage</li>
 *       <li>Slot reuse via assignment</li>
 *       <li>GC-like deferred destruction</li>
 *       <li>Entirely pool-controlled synchronization</li>
 *     </ul>
 *   </li>
 *   <li>
 *     <b><code>pointer_pool</code></b>:
 *     <ul>
 *       <li>Pointer-driven identity (Generally, comparisons of internal objects are proxied using
 *           <code>jh::weak_ptr_hash</code> and <code>jh::weak_ptr_eq</code>.)
 *           <br>
 *           On Windows, the native implementation of <code>shared_ptr</code> can cause greater jitter than POSIX.
 *       </li>
 *       <li>Heap-allocated, immovable objects</li>
 *       <li>Immediate destruction via <code>shared_ptr</code></li>
 *       <li>Weak-observed lifetime</li>
 *     </ul>
 *   </li>
 * </ul>
 *
 * <p>
 * If your objects:
 * </p>
 * <ul>
 *   <li>are copyable or movable,</li>
 *   <li>can be identified by a lightweight external key,</li>
 *   <li>benefit from slot reuse and delayed destruction,</li>
 * </ul>
 * <p>
 * then <code>flat_pool</code> is the preferred structure.
 * </p>
 *
 * <p>
 * If your objects:
 * </p>
 * <ul>
 *   <li>must reside at a stable address,</li>
 *   <li>cannot be copied or moved,</li>
 *   <li>or must release heavy resources exactly when the last reference disappears,</li>
 * </ul>
 * <p>
 * then a pointer-based pool should be used instead.
 * </p>
 *
 * @version <pre>1.4.x</pre>
 * @date <pre>2025</pre>
 */


#pragma once

#include <compare>
#include <vector>
#include <shared_mutex>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <memory>
#include <mutex>
#include <algorithm>
#include <type_traits>
#include <stdexcept>

#include "jh/typing/monostate.h"
#include "jh/core/ordered_map.h"
#include "jh/conceptual/hashable.h"
#include "jh/synchronous/control_buf.h"

namespace jh::conc {
    namespace detail {
        /// @brief Selects canonical value type: <code>K</code> for set, <code>std::pair&lt;K, V&gt;</code>for map.
        template<typename K, typename V>
        using value_t =
                std::conditional_t<
                        jh::typed::monostate_t<std::remove_cvref_t<V>>,
                        std::remove_cvref_t<K>,
                        std::pair<
                                std::remove_cvref_t<K>,
                                std::remove_cvref_t<V>>
                >;
    }

    namespace extension {
        /**
         * @brief Default value construction policy for flat_pool.
         *
         * <p>
         * <code>value_factory&lt;V&gt;</code> defines how values of type <code>V</code> are constructed
         * when a new entry is inserted into a map-like <code>flat_pool</code>.
         * </p>
         *
         * <p>
         * The default implementation forwards all arguments directly to
         * <code>V</code>'s constructor.
         * </p>
         *
         * <p>
         * This template serves as a public extension point. Users may specialize
         * <code>value_factory</code> for custom value types to override construction
         * behavior without modifying <code>flat_pool</code> itself.
         * </p>
         *
         * @tparam V Value type to construct.
         */
        template<typename V>
        struct value_factory final {
            template<typename... Args>
            static V make(Args &&... args) {
                return V(std::forward<Args>(args)...);
            }
        };

        /**
         * @brief Value construction policy specialization for <code>std::shared_ptr</code>.
         *
         * <p>
         * This specialization constructs values using <code>std::make_shared</code>
         * instead of directly invoking the <code>std::shared_ptr</code> constructor.
         * </p>
         *
         * <p>
         * Using <code>std::make_shared</code> ensures a single allocation for the
         * control block and the managed object, improving allocation efficiency
         * and cache locality.
         * </p>
         *
         * @tparam T Managed object type.
         */
        template<typename T>
        struct value_factory<std::shared_ptr<T>> final {
            template<typename... Args>
            static std::shared_ptr<T> make(Args &&... args) {
                return std::make_shared<T>(std::forward<Args>(args)...);
            }
        };

        /**
         * @brief Value construction policy specialization for <code>std::unique_ptr</code>.
         *
         * <p>
         * This specialization constructs values using <code>std::make_unique</code>,
         * ensuring exception safety and clear ownership semantics.
         * </p>
         *
         * <p>
         * The managed object is allocated on the heap and ownership is transferred
         * directly to the returned <code>std::unique_ptr</code>.
         * </p>
         *
         * @tparam T Managed object type.
         */
        template<typename T>
        struct value_factory<std::unique_ptr<T>> final {
            template<typename... Args>
            static std::unique_ptr<T> make(Args &&... args) {
                return std::make_unique<T>(std::forward<Args>(args)...);
            }
        };
    } // namespace extension

    /**
     * @brief Hash-ordered, contiguous resource interning pool.
     *
     * <h4>Conceptual Model</h4>
     * <p>
     * <code>flat_pool</code> interns objects by mapping keys to stable integer indices
     * inside a contiguous storage vector. Each unique key corresponds to at most one
     * active slot at any time.
     * </p>
     *
     * <p>
     * The pool maintains a sorted index of <code>(hash, index)</code> pairs, allowing
     * logarithmic lookup by hash followed by linear resolution of hash collisions.
     * This design preserves the full entropy of the hash value and avoids bucket-based
     * aliasing.
     * </p>
     *
     * <h4>Key–Value Semantics</h4>
     * <p>
     * The pool may operate in two modes:
     * </p>
     * <ul>
     *   <li><b>Set-like:</b> When <code>Value</code> is <code>jh::typed::monostate</code>,
     *       only keys are stored.</li>
     *   <li><b>Map-like:</b> Otherwise, the pool stores <code>(Key, Value)</code> pairs,
     *       where the value is constructed only upon first insertion.</li>
     * </ul>
     *
     * <h4>Construction and Deduplication</h4>
     * <p>
     * Acquisition follows a two-phase lookup strategy:
     * </p>
     * <ol>
     *   <li>Shared-lock lookup to detect an existing entry.</li>
     *   <li>Exclusive-lock recheck followed by insertion if absent.</li>
     * </ol>
     *
     * <p>
     * For map-like pools, value construction is deferred until the key is confirmed
     * to be absent, ensuring that repeated acquisitions do not incur unnecessary
     * construction cost.
     * </p>
     *
     * <h4>Lifetime Management</h4>
     * <p>
     * Each slot maintains an atomic reference count. When the count reaches zero,
     * the slot is marked as free and may be reused by subsequent insertions.
     * </p>
     *
     * <p>
     * Slots are not immediately destroyed or removed from storage. Instead,
     * they participate in a free-slot reuse mechanism that minimizes memory churn.
     * </p>
     *
     * <h4>Concurrency Guarantees</h4>
     * <ul>
     *   <li>Lookup operations acquire only shared locks.</li>
     *   <li>Insertion and release require exclusive access.</li>
     *   <li>Reference counting is performed atomically.</li>
     * </ul>
     *
     * <h4>Reallocation Safety</h4>
     * <p>
     * Although indices remain stable, vector reallocation may invalidate references
     * or pointers to stored objects. To address this, the pool provides a
     * <code>no_reallocate_guard</code> mechanism that prevents reallocation while
     * dereferencing pooled objects in concurrent environments.
     * </p>
     *
     * @tparam Key   Key type defining object identity.
     * @tparam Value Optional associated value type (<code>jh::typed::monostate</code> for key-only).
     * @tparam Hash  Hash functor used to compute full-width hash values
     *               (<code>jh::hash</code> for auto hash-derivation).
     * @tparam Alloc Allocator type for contiguous storage.
     *
     * @note
     * <p>
     * Unlike <code>pointer_pool</code> or its user-facing interface <code>observe_pool</code>,
     * <code>flat_pool</code>(<code>resource_pool</code>) does not permit moves.
     * </p>
     * <p>
     * As indicated by their user-facing interface names, <code>pointer_pool</code>(<code>observe_pool</code>)
     * does not hold objects: it merely observes them.
     * The worst-case scenario after a move is deduplication failure, but the system remains operational.
     * <br>
     * <code>flat_pool</code>(<code>resource_pool</code>), however, fully owns objects.
     * Once moves are permitted, the <code>flat_pool::ptr</code> handle becomes dangling, hence the move semantics
     * are disabled to ensure safety.
     * </p>
     */
    template<typename Key,
            typename Value = jh::typed::monostate,
            typename Hash = jh::hash<Key>,
            typename Alloc = std::allocator<detail::value_t<Key, Value>>> requires
    ((requires(const Key &k) {
        { Hash{}(k) } -> std::convertible_to<size_t>;
    }) &&
     (requires(const Key &a, const Key &b) {
         { a == b } -> std::convertible_to<bool>;
     }) &&
     jh::concepts::is_contiguous_reallocable<Key>
     && (jh::typed::monostate_t<Value> || jh::concepts::is_contiguous_reallocable<Value>))
    class flat_pool final {
    public:
        /// @brief Stored element type (<code>Key</code> or <code>std::pair&ltKey, Value&gt;</code>)
        using value_type = detail::value_t<Key, Value>;

        /// @brief Allocator rebound for value <code>storage_</code> only
        using allocator_type =
                typename std::allocator_traits<Alloc>::template rebind_alloc<detail::value_t<Key, Value>>;

    private:
        /**
         * @brief Internal index key for hash-ordered entry storage.
         *
         * Used by the ordered entry set to map full hash values to storage indices.
         * A sentinel key <code>{hash, 0}</code> enables <code>lower_bound</code>-based
         * lookup of all entries sharing the same hash.
         */
        struct entry_key final {
            std::size_t hash;
            [[maybe_unused]] std::size_t index;

            std::strong_ordering operator<=>(const entry_key &other) const = default;
        };

        std::vector<value_type, allocator_type> storage_;
        jh::sync::control_buf<std::atomic<std::uint64_t>> refcounts_;
        std::vector<uint8_t> occupation_;
        std::size_t first_candidate_ = static_cast<std::size_t>(-1);
        jh::ordered_set<entry_key> entries_;
        // Contiguous ordered set (vector-backed AVL-style structure) used as a hash-ordered index

        mutable std::shared_mutex entry_mtx_;
        mutable std::shared_mutex pool_mtx_;

        /**
         * @brief Non-copyable, non-movable guard preventing pool reallocation during access.
         *
         * Internal RAII guard that holds a shared lock to ensure objects remain
         * safe to dereference by preventing concurrent reallocation.
         *
         * This type is private, cannot be copied or moved, and cannot escape
         * its intended scope or be stored in containers.
         */
        struct no_reallocate_guard final {
            explicit no_reallocate_guard(std::shared_mutex &m) : m_(&m) {
                m_->lock_shared();
            }

            ~no_reallocate_guard() {
                m_->unlock_shared();
            }

            no_reallocate_guard(const no_reallocate_guard &) = delete;

            no_reallocate_guard &operator=(const no_reallocate_guard &) = delete;

            no_reallocate_guard(no_reallocate_guard &&) = delete;

            no_reallocate_guard &operator=(no_reallocate_guard &&) = delete;

        private:
            std::shared_mutex *m_;
        };

        /// @brief Creates a no-reallocation-guard for use by <code>flat_pool::ptr::guard()</code>
        no_reallocate_guard make_no_reallocate_guard() const {
            return no_reallocate_guard{pool_mtx_};
        }

        /**
         * @brief Finds the storage index for a key without modifying reference counts.
         *
         * @details
         * Performs a hash-ordered lookup using <code>lower_bound({hash, 0})</code>
         * to locate the contiguous range of entries sharing the same hash, then
         * resolves collisions by key comparison.
         *
         * This function assumes required synchronization is handled by the caller
         * and does not adjust reference counts.
         *
         * @return The storage index if found; otherwise <code>size_t(-1)</code>.
         */
        std::size_t find_idx_no_lock(const Key &key) {
            std::size_t h = Hash{}(key);
            auto it = entries_.lower_bound(entry_key{h, 0});

            for (; it != entries_.end() && it->hash == h; ++it) {
                size_t idx = it->index;
                if constexpr (jh::typed::monostate_t<Value>) {
                    if (storage_[idx] == key) {
                        return idx;
                    }
                } else {
                    if (storage_[idx].first == key) {
                        return idx;
                    }
                }
            }
            return static_cast<std::size_t>(-1);
        }

        /**
         * @brief Inserts or finds a key in set-like mode without modifying reference counts.
         *
         * @details
         * <p>
         * Uses a two-phase lookup (shared lock followed by exclusive locks) to
         * avoid ABA-style races: even if a slot is reclaimed and reused by another
         * thread between checks, the second lookup under exclusive locking
         * revalidates the state before insertion.
         * </p>
         * <p>
         * This guarantees that slot reuse cannot cause a logically distinct entry
         * to be observed as an existing one.
         * </p>
         *
         * @tparam KArg A cv/ref-qualified form of <code>Key</code>.
         *
         * @param k The key identifying the object.
         * @return The storage index of the existing or newly inserted key.
         */
        template<typename KArg>
        std::size_t emplace(KArg &&k) requires jh::typed::monostate_t<Value> {
            // shared lock the entries_ for lookup
            {
                std::shared_lock lk(entry_mtx_);
                auto attempt = find_idx_no_lock(k);
                if (attempt != static_cast<std::size_t>(-1)) return attempt;
            }
            // not found -> acquire exclusive locks
            std::unique_lock entry_lock(entry_mtx_);
            std::unique_lock storage_lock(pool_mtx_);

            // revalidate under exclusive locks to avoid race
            auto attempt = find_idx_no_lock(k);
            if (attempt != static_cast<std::size_t>(-1)) return attempt;

            // not found -> create new object
            size_t idx;
            if (first_candidate_ != static_cast<std::size_t>(-1)) {
                idx = first_candidate_;
                storage_[idx] = k;
                occupation_[idx] = 1;
                refcounts_[idx].store(0);
                update_first_candidate();
            } else {
                idx = storage_.size();
                storage_.emplace_back(k);
                refcounts_.emplace_back();
                occupation_.emplace_back(1);
                refcounts_[idx].store(0);
            }
            std::size_t h = Hash{}(k);
            // update entries_
            entries_.emplace(h, idx);

            return idx;
        }

        /**
         * @brief Inserts or finds a key–value entry in map-like mode without modifying reference counts.
         *
         * @details
         * <p>
         * Uses the same two-phase lookup strategy as the set-like overload
         * (shared lock followed by exclusive locks) to avoid ABA-style races.
         * Even if a slot is reclaimed and reused by another thread between checks,
         * the second lookup under exclusive locking revalidates the state
         * before insertion.
         * </p>
         *
         * <p>
         * Value construction is performed <em>only</em> when the key is confirmed
         * absent under exclusive locking. The provided argument tuple is ignored
         * if an equivalent key already exists.
         * </p>
         *
         * <p>
         * This guarantees that slot reuse cannot cause a logically distinct
         * key–value entry to be observed as an existing one, and that value
         * construction occurs exactly once per unique key.
         * </p>
         *
         * @tparam KArg A cv/ref-qualified form of <code>Key</code>.
         * @tparam Args Types of arguments used for value construction.
         *
         * @param k          The key identifying the entry.
         * @param args_tuple Tuple of arguments forwarded to value construction.
         *
         * @return The storage index of the existing or newly inserted entry.
         */
        template<typename KArg, typename ...Args>
        std::size_t emplace(KArg &&k, std::tuple<Args...> args_tuple)requires (!jh::typed::monostate_t<Value>) {
            // shared lock the entries_ for lookup
            {
                std::shared_lock lk(entry_mtx_);
                auto attempt = find_idx_no_lock(k);
                if (attempt != static_cast<std::size_t>(-1)) return attempt;
            }
            // not found -> acquire exclusive locks
            std::unique_lock entry_lock(entry_mtx_);
            std::unique_lock storage_lock(pool_mtx_);

            // revalidate under exclusive locks to avoid race
            auto attempt = find_idx_no_lock(k);
            if (attempt != static_cast<std::size_t>(-1)) return attempt;

            // not found -> create new object
            size_t idx;
            // assign or emplace_back, use extension::value_factory::make to construct Value
            if (first_candidate_ != static_cast<std::size_t>(-1)) {
                idx = first_candidate_;
                storage_[idx].first = k;
                storage_[idx].second = std::apply([](auto &&... unpacked) {
                    return extension::value_factory<Value>::make(
                            std::forward<decltype(unpacked)>(unpacked)...);
                }, args_tuple);
                occupation_[idx] = 1;
                refcounts_[idx].store(0);
                update_first_candidate();
            } else {
                idx = storage_.size();
                storage_.emplace_back(k,
                                      std::apply([](auto &&... unpacked) {
                                          return extension::value_factory<Value>::make(
                                                  std::forward<decltype(unpacked)>(unpacked)...);
                                      }, args_tuple));
                refcounts_.emplace_back();
                occupation_.emplace_back(1);
                refcounts_[idx].store(0);
            }
            std::size_t h = Hash{}(k);
            // update entries_
            entries_.emplace(h, idx);

            return idx;
        }

        /**
         * @brief Updates the next reusable slot hint.
         *
         * Advances <code>first_candidate_</code> to the next unoccupied slot,
         * or sets it to <code>size_t(-1)</code> if no free slot exists.
         */
        void update_first_candidate() {
            for (size_t i = first_candidate_; i < occupation_.size(); ++i) {
                if (occupation_[i] == 0) {
                    first_candidate_ = i;
                    return;
                }
            }
            first_candidate_ = static_cast<std::size_t>(-1);
        }

        /**
         * @brief Increments the reference count of a slot if it is valid.
         *
         * @param index The storage index whose reference count is to be incremented.
         * @return <code>true</code> if the slot is valid and the reference count
         *         was successfully incremented; <code>false</code> if the index
         *         is out of range or refers to an unoccupied slot.
         */
        bool add_ref(size_t index) {
            std::shared_lock lk(pool_mtx_);
            if (index >= occupation_.size() || occupation_[index] == 0)
                return false;
            refcounts_[index].fetch_add(1);
            return true;
        }

        /**
         * @brief Removes an entry from the hash-ordered index.
         *
         * @param h   The full hash value associated with the entry.
         * @param idx The storage index to remove from the index.
         */
        void remove_entry_no_lock(std::size_t h, std::size_t idx) {
            auto it = entries_.lower_bound(entry_key{h, 0});

            for (; it != entries_.end() && it->hash == h; ++it) {
                if (it->index == idx) {
                    entries_.erase(it);
                    return;
                }
            }
        }

        /**
         * @brief Decrements the reference count of a slot and releases it if it reaches zero.
         *
         * @param index The storage index whose reference count is to be decremented.
         *
         * @details
         * When the reference count drops to zero, the slot is marked unoccupied,
         * removed from the hash-ordered index, and becomes eligible for reuse.
         * Slot reuse is tracked by updating <code>first_candidate_</code>.
         */
        void release_ref(size_t index) {
            {
                std::shared_lock lk(pool_mtx_);
                if (refcounts_[index].fetch_sub(1) > 1)
                    return;
            }
            std::unique_lock storage_lock(pool_mtx_);

            if (refcounts_[index].load() != 0) return;

            if (occupation_[index] == 1) {
                occupation_[index] = 0;
                size_t h;
                if constexpr (jh::typed::monostate_t<Value>) {
                    h = Hash{}(storage_[index]);
                } else {
                    h = Hash{}(storage_[index].first);
                }
                remove_entry_no_lock(h, index); // already locked

                if (index < first_candidate_)
                    first_candidate_ = index;
            }
        }

    public:
        /// @brief Minimum reserved size for the pool.
        static std::uint64_t constexpr MIN_RESERVED_SIZE = 16;

        /**
         * @brief Constructs a flat_pool with pre-reserved contiguous storage.
         *
         * @details
         * Initializes an empty pool and pre-reserves internal storage to reduce
         * reallocation overhead during early insertions.
         * <p>
         * The reservation applies to:
         * </p>
         * <ul>
         *   <li>the contiguous value storage,</li>
         *   <li>the reference count buffer,</li>
         *   <li>the occupation bitmap,</li>
         *   <li>the hash-ordered entry index.</li>
         * </ul>
         *
         * <p>
         * If <code>reserve_size</code> is smaller than <code>MIN_RESERVED_SIZE</code>, the minimum
         * value is used instead. This guarantees a baseline capacity suitable for
         * typical workloads and avoids pathological reallocation behavior.
         * </p>
         *
         * <p>
         * No objects are constructed during initialization. All slots are created
         * lazily upon first acquisition.
         * </p>
         *
         * @param reserve_size Initial number of slots to reserve.
         */
        explicit flat_pool(std::uint64_t reserve_size = MIN_RESERVED_SIZE) :
                storage_(), refcounts_(), occupation_(), first_candidate_(static_cast<std::size_t>(-1)), entries_() {
            if (reserve_size < MIN_RESERVED_SIZE)
                reserve_size = MIN_RESERVED_SIZE;
            storage_.reserve(reserve_size);
            refcounts_.reserve(reserve_size);
            occupation_.reserve(reserve_size);
            entries_.reserve(reserve_size);
        }

        /**
         * @brief Constructs a flat_pool using a custom allocator.
         *
         * @details
         * Initializes an empty pool with allocator-aware contiguous storage and
         * reserves the minimum required capacity.
         *
         * @param alloc Allocator used for internal value storage.
         */
        explicit flat_pool(const allocator_type &alloc)
                : storage_(alloc), refcounts_(), occupation_(), first_candidate_(static_cast<std::size_t>(-1)),
                  entries_() {
            storage_.reserve(MIN_RESERVED_SIZE);
            refcounts_.reserve(MIN_RESERVED_SIZE);
            occupation_.reserve(MIN_RESERVED_SIZE);
            entries_.reserve(MIN_RESERVED_SIZE);
        }

        /**
         * @brief Constructs a flat_pool with a custom allocator and explicit reserved capacity.
         *
         * @details
         * Initializes an empty pool using the provided allocator for contiguous
         * value storage and pre-reserves internal capacity according to
         * <code>reserve_size</code>.
         * <p>
         * The reservation applies uniformly to all internal structures, including:
         * </p>
         * <ul>
         *   <li>the contiguous value storage vector,</li>
         *   <li>the reference count buffer,</li>
         *   <li>the occupation bitmap,</li>
         *   <li>the hash-ordered entry index.</li>
         * </ul>
         *
         * <p>
         * If <code>reserve_size</code> is smaller than <code>MIN_RESERVED_SIZE</code>, the minimum
         * value is used instead. This ensures a baseline capacity and avoids
         * early reallocation under light workloads.
         * </p>
         *
         * <p>
         * No objects are constructed during initialization. All slots are created
         * lazily upon first successful acquisition.
         * </p>
         *
         * @param reserve_size Initial number of slots to reserve.
         * @param alloc        Allocator used for contiguous value storage.
         */
        [[maybe_unused]] explicit flat_pool(std::uint64_t reserve_size, const allocator_type &alloc)
                : storage_(alloc), refcounts_(), occupation_(), first_candidate_(static_cast<std::size_t>(-1)),
                  entries_() {
            if (reserve_size < MIN_RESERVED_SIZE)
                reserve_size = MIN_RESERVED_SIZE;
            storage_.reserve(reserve_size);
            refcounts_.reserve(reserve_size);
            occupation_.reserve(reserve_size);
            entries_.reserve(reserve_size);
        }

        /// @brief Copy construction is disabled to preserve handle and index validity.
        flat_pool(const flat_pool &other) = delete;

        /// @brief Copy assignment is disabled to prevent duplication of owned storage.
        flat_pool &operator=(const flat_pool &other) = delete;

        /// @brief Move construction is disabled because pooled objects are index-bound.
        flat_pool(flat_pool &&other) = delete;

        /// @brief Allocator-aware move construction is disabled for safety.
        flat_pool(flat_pool &&other, const allocator_type &alloc) = delete;

        /// @brief Move assignment is disabled to avoid dangling pool handles.
        flat_pool &operator=(flat_pool &&other) = delete;


        /**
         * @brief Reference-counted handle to a pooled object.
         *
         * <p>
         * <code>flat_pool::ptr</code> is a lightweight RAII handle that represents a
         * reference to a slot within a <code>flat_pool</code>.
         * </p>
         *
         * <p>
         * Copying a <code>ptr</code> increments the underlying slot's reference count.
         * Destruction or reset decrements it. When the count reaches zero, the slot
         * becomes eligible for reuse.
         * </p>
         *
         * <h5>Dereferencing and Safety</h5>
         * <p>
         * Dereferencing yields a reference or pointer to the underlying stored object.
         * In multithreaded contexts where the pool may be resized concurrently,
         * users must acquire a <code>no_reallocate_guard</code> before dereferencing
         * to prevent vector reallocation.
         * </p>
         *
         * <h5>Null Semantics</h5>
         * <p>
         * A default-constructed or explicitly reset <code>ptr</code> represents a null
         * handle and compares equal to <code>nullptr</code>.
         * </p>
         *
         * @note
         * In fact, <code>flat_pool::ptr</code> behaves exactly like <code>shared_ptr</code>
         * (i.e., copy/move construction/assignment shares the handle, and the object is logically
         * dead when the count is 0).
         * <br>
         * Even if the slot is not reused, the object remains unreachable once it is dead.
         * <br>
         * <code>flat_pool::ptr</code>, as a pool pointer, simply provides an additional way to retrieve objects
         * from the pool using <code>find</code> and <code>acquire</code>,
         * as well as lazy (GC-like) object destruction.
         */
        struct ptr final {
        private:
            /// @brief Pointer to the owning pool.
            flat_pool *pool_ = nullptr;
            /// @brief Storage index within the pool.
            size_t index_ = static_cast<std::size_t>(-1);
        public:
            /**
             * @brief Default-constructs a null handle.
             */
            ptr() = default;

            /**
             * @brief Constructs a handle referencing a pool slot.
             *
             * @param p Pointer to the owning pool.
             * @param i Storage index within the pool.
             */
            ptr(flat_pool *p, size_t i) : pool_(p), index_(i) {
                if (pool_) {
                    pool_->add_ref(index_);
                }
            }

            /**
             * @brief Constructs a null handle.
             */
            explicit ptr(std::nullptr_t) : pool_(nullptr), index_(static_cast<std::size_t>(-1)) {}

            /**
             * @brief Copy-constructs a handle.
             *
             * @details
             * The new handle references the same slot and increments
             * the associated reference count.
             *
             * @param o Source handle.
             */
            ptr(const ptr &o) {
                pool_ = o.pool_;
                index_ = o.index_;
                if (pool_) pool_->add_ref(index_);
            }

            /**
             * @brief Copy-assigns a handle.
             *
             * @details
             * Releases the currently held reference (if any), then acquires
             * a reference to the slot held by the source handle.
             *
             * @param o Source handle.
             * @return Reference to this handle.
             */
            ptr &operator=(const ptr &o) {
                if (this != &o) {
                    reset();
                    pool_ = o.pool_;
                    index_ = o.index_;
                    if (pool_) pool_->add_ref(index_);
                }
                return *this;
            }

            /**
             * @brief Move-constructs a handle.
             *
             * @details
             * Transfers the reference without modifying the reference count.
             * The source handle is reset to null.
             *
             * @param o Source handle.
             */
            ptr(ptr &&o) noexcept: pool_(o.pool_), index_(o.index_) {
                o.pool_ = nullptr;
                o.index_ = static_cast<std::size_t>(-1);
            }

            /**
             * @brief Move-assigns a handle.
             *
             * @details
             * Releases the currently held reference (if any) and takes ownership
             * of the reference held by the source handle. The source handle is
             * reset to null.
             *
             * @param o Source handle.
             * @return Reference to this handle.
             */
            ptr &operator=(ptr &&o) noexcept {
                if (this != &o) {
                    reset();
                    pool_ = o.pool_;
                    index_ = o.index_;
                    o.pool_ = nullptr;
                    o.index_ = static_cast<std::size_t>(-1);
                }
                return *this;
            }

            /**
             * @brief Releases the reference held by this handle.
             *
             * @details
             * If the reference count drops to zero, the slot is marked as unoccupied,
             * but the stored object is <b>not</b> destroyed at this point.
             *
             * <p>
             * This design avoids immediate destructor invocation, which may be
             * expensive. In many cases, reassigning an existing object is cheaper
             * than destroying and reconstructing it.
             * </p>
             *
             * <p>
             * The underlying object is only destroyed when the slot is reused or
             * forcibly reclaimed during <code>resize_pool()</code>, following a
             * GC-like deferred reclamation strategy.
             * </p>
             */
            ~ptr() { reset(); }

            /**
             * @brief Releases the reference held by this handle and resets it to null.
             *
             * @details
             * This function follows STL smart pointer conventions: the handle is
             * explicitly reset to a null state rather than being manually destroyed.
             *
             * Decrements the reference count of the associated slot. If the count
             * reaches zero, the slot is marked as unoccupied but the stored object
             * is not immediately destroyed.
             */
            void reset() {
                if (pool_) {
                    pool_->release_ref(index_);
                    pool_ = nullptr;
                    index_ = static_cast<std::size_t>(-1);
                }
            }

            /**
             * @brief Dereferences the handle to access the stored object.
             *
             * @details
             * <p>
             * In single-threaded usage, this operator may be used directly.
             * </p>
             * <p>
             * In multithreaded contexts, if more than one thread may operate on the pool
             * and any operation could trigger reallocation (including insertion or
             * <code>resize_pool()</code>), the caller must hold a guard obtained via
             * <code>auto g = p.guard();</code> for the duration of the access.
             * </p>
             * Failing to do so may result in a dangling reference even though the
             * pointer itself remains logically valid.
             */
            value_type &operator*() {
                if (!pool_) {
                    throw std::runtime_error("jh::conc::flat_pool::ptr: dereferencing nullptr");
                }
                return pool_->storage_[index_];
            }

            /**
             * @brief Member access to the stored object.
             *
             * @details
             * <p>
             * In single-threaded usage, this operator may be used directly.
             * </p>
             * <p>
             * In multithreaded contexts where concurrent operations on the pool may
             * cause reallocation (such as insertion or forced shrinking), callers
             * must acquire a guard with <code>auto g = p.guard();</code> before using
             * this operator.
             * </p>
             * The guard ensures the underlying storage remains stable for the
             * duration of the access.
             */
            value_type *operator->() {
                if (!pool_) {
                    throw std::runtime_error("jh::conc::flat_pool::ptr: dereferencing nullptr");
                }
                return &pool_->storage_[index_];
            }

            /// @brief Compares two handles for equality.
            bool operator==(const ptr &other) const = default;

            /// @brief Compares the handle against <code>nullptr</code>.
            bool operator==(std::nullptr_t) {
                return pool_ == nullptr;
            }

            /**
             * @brief Acquires a guard that prevents pool reallocation during dereference.
             *
             * @details
             * The pointer itself is stable, but the underlying contiguous storage may
             * be reallocated by other threads, which would invalidate any obtained
             * <code>T&amp;</code> or <code>T*</code>. This guard prevents such reallocation
             * while it is held.
             * <p>
             * In multithreaded contexts, the guard must be held whenever dereferencing
             * the pointer if other threads may trigger pool resizing.
             * </p>
             *
             * @return A guard witch is non-copyable, non-movable, and scope-bound,
             *         making escape or misuse impossible.
             */
            [[nodiscard]] flat_pool::no_reallocate_guard guard() const {
                return pool_->make_no_reallocate_guard();
            }
        };

        friend class flat_pool::ptr;

        /**
         * @brief Retrieves or creates a pooled object associated with a key (set-like).
         *
         * @details
         * This overload is available only when the pool operates in <b>set-like</b>
         * mode (<code>Value == jh::typed::monostate</code>).
         * </p><p>
         * If an equivalent key already exists in the pool, a handle to the existing
         * slot is returned. Otherwise, a new slot containing the key is created.
         * </p>
         * <p>
         * No value construction is involved in this mode.
         * </p>
         *
         * @tparam KArg A cv/ref-qualified form of <code>Key</code>.
         *
         * @param key The key identifying the object.
         * @return A reference-counted handle to the pooled key, or a null handle on failure.
         */
        template<typename KArg>
        ptr acquire(KArg &&key) requires(jh::typed::monostate_t<Value>) {
            size_t idx = emplace(std::forward<KArg &&>(key));
            if (idx == static_cast<std::size_t>(-1)) return ptr{nullptr};
            return ptr(this, idx);
        }

        /// @brief Deleted overload for map-like pools without value arguments.
        template<typename KArg>
        ptr acquire(KArg &&key) requires(!jh::typed::monostate_t<Value>) = delete;

        /**
         * @brief Retrieves or creates a pooled key–value entry (map-like).
         *
         * @details
         * This overload is available only when the pool operates in <b>map-like</b>
         * mode (<code>Value != jh::typed::monostate</code>).
         * <p>
         * The value construction arguments are provided as a <code>std::tuple</code>
         * and are used <em>only if</em> the key does not already exist in the pool.
         * If an equivalent key is found, the existing entry is reused and the
         * provided arguments are ignored.
         * </p>
         *
         * <p>
         * This design ensures that expensive value construction is performed
         * exactly once for each unique key, even under concurrent acquisition.
         * </p>
         *
         * <h5>Construction Semantics</h5>
         * <ul>
         *   <li>The key is used for lookup and deduplication.</li>
         *   <li>
         *     By default, the value is constructed by forwarding the provided
         *     arguments to <code>Value</code>'s constructor.
         *   </li>
         *   <li>
         *     For <code>std::shared_ptr&lt;T&gt;</code> and <code>std::unique_ptr&lt;T&gt;</code>,
         *     the default behavior forwards the arguments to
         *     <code>std::make_shared&lt;T&gt;</code> and
         *     <code>std::make_unique&lt;T&gt;</code> respectively.
         *   </li>
         *   <li>
         *     Repeated calls with the same key but different argument tuples will
         *     always return the originally constructed value.
         *   </li>
         * </ul>
         *
         * <h5>Custom Value Construction</h5>
         * <p>
         * Value construction is customizable through a public extension point:
         * <code>jh::conc::extension::value_factory&lt;Value&gt;</code>.
         * Users may specialize this template to override how values are created.
         * </p>
         *
         * <p>
         * A custom factory may be provided as follows:
         * </p>
         *
         * @code
         * namespace jh::conc::extension {
         *     template&lt;&gt;
         *     struct value_factory&lt;Foo&gt; {
         *         template&lt;typename... Args&gt;
         *         static Foo make(Args&&... args) {
         *             // user-defined construction logic
         *         }
         *     };
         * }
         * @endcode
         *
         * <p>
         * This mechanism is an <b>intentional public injection point</b> and allows
         * customization without modifying or subclassing <code>flat_pool</code>.
         * </p>
         *
         * <p>
         * Users should treat the argument tuple as <b>initialization parameters</b>,
         * not update parameters.
         * </p>
         *
         * @tparam KArg A cv/ref-qualified form of <code>Key</code>.
         * @tparam Args Types of arguments used for value construction.
         *
         * @param key        The key identifying the entry.
         * @param args_tuple Tuple of arguments forwarded to value construction.
         * @return A reference-counted handle to the pooled entry, or a null handle on failure.
         */
        template<typename KArg, typename... Args>
        ptr acquire(KArg &&key, std::tuple<Args...> args_tuple) requires (!jh::typed::monostate_t<Value>) {
            size_t idx = emplace(std::forward<KArg &&>(key), std::forward<std::tuple<Args... >>(args_tuple));
            if (idx == static_cast<std::size_t>(-1)) return ptr{nullptr};
            return ptr(this, idx);
        }

        /// @brief args acquire is deleted for set-like pools
        template<typename KArg, typename... Args>
        ptr acquire(KArg &&key, std::tuple<Args...> args_tuple) requires(jh::typed::monostate_t<Value>) = delete;

        /**
         * @brief Looks up an existing pooled object without creating a new one.
         *
         * @param key The key to search for.
         * @return A valid handle if the key exists; otherwise, a null handle.
         *
         * @note
         * Unlike <code>acquire()</code>, this function never inserts new entries.
         */
        ptr find(const Key &key) {
            std::shared_lock lk(entry_mtx_);
            auto idx = find_idx_no_lock(key);
            if (idx == static_cast<std::size_t>(-1)) return ptr{nullptr};
            return ptr(this, idx);
        }

        /**
         * @brief Checks whether the pool contains no active entries.
         *
         * <p>
         * Returns <code>true</code> if there are currently no live entries
         * registered in the pool.
         * </p>
         *
         * <p>
         * This function reflects the <b>logical emptiness</b> of the pool,
         * not its physical storage state. Internal capacity and previously
         * allocated slots may still exist even when the pool is empty.
         * </p>
         *
         * @return <code>true</code> if the pool has no active entries; otherwise <code>false</code>.
         */
        bool empty() {
            return entries_.empty();
        }

        /**
         * @brief Returns the current storage capacity of the pool.
         *
         * <p>
         * This value represents the number of slots currently allocated
         * in the underlying contiguous storage. It reflects historical
         * peak demand rather than current usage.
         * </p>
         *
         * <p>
         * A larger capacity does not imply high active usage. The pool
         * may have grown due to a temporary workload spike and later
         * released most entries without shrinking.
         * </p>
         *
         * <p>
         * Capacity is adjusted only through explicit maintenance operations
         * such as <code>resize_pool()</code>.
         * </p>
         *
         * @return The current storage capacity.
         */
        std::size_t capacity() {
            return storage_.capacity();
        }

        /**
         * @brief Returns the number of active entries in the pool.
         *
         * @details
         * This function reports the number of currently live, deduplicated
         * entries registered in the pool.
         * <br>
         * The returned value corresponds to the number of keys present
         * in the internal index, not the number of allocated slots.
         * <p>
         * In particular:
         * </p>
         * <ul>
         *   <li>Released entries that are eligible for reuse are not counted.</li>
         *   <li>Internal storage may contain inactive slots beyond this count.</li>
         * </ul>
         *
         * @return The number of active entries.
         */
        std::size_t size() {
            return entries_.size();
        }

        /**
         * @brief Returns a snapshot of pool capacity and active entry count.
         *
         * @details
         * This function acts as a <b>health observer</b> rather than a strict
         * capacity-management API. It reports the current storage capacity and
         * the number of active entries as a logically consistent pair.
         *
         * <h5>Consistency Guarantee</h5>
         * <p>
         * The returned <code>(capacity, size)</code> values are obtained under
         * a single shared lock and therefore always reflect the same internal
         * version of the pool state. Callers may rely on the two values being
         * mutually consistent and not derived from different update epochs.
         * </p>
         *
         * <h5>Hot-Path vs Cold-Path Interpretation</h5>
         * <p>
         * This metric is intended for <b>observational and heuristic use</b>.
         * It does <em>not</em> imply that the pool should be immediately shrunk
         * when utilization appears low.
         * </p>
         *
         * <ul>
         *   <li>
         *     <b>Hot paths:</b> Capacity growth reflects real demand. If the pool
         *     has expanded to a certain size, it indicates that the workload has
         *     required that capacity at some point. Shrinking on the hot path is
         *     therefore discouraged, as it may introduce allocation jitter and
         *     negate the benefit of prior expansion.
         *   </li>
         *   <li>
         *     <b>Cold paths:</b> When the pool remains underutilized for an extended
         *     period and health metrics consistently indicate low occupancy,
         *     a controlled shrink (e.g. via <code>resize_pool()</code>) may be
         *     considered to release unused memory.
         *   </li>
         * </ul>
         *
         * <h5>Slot Reuse Characteristics</h5>
         * <p>
         * The pool preferentially reuses the lowest-index free slots. As a result,
         * after a temporary surge in capacity, newly inserted entries naturally
         * migrate toward the front of the storage vector over time.
         * </p>
         *
         * <p>
         * This behavior means that tail regions tend to become empty first during
         * cooling phases, making them suitable candidates for release without
         * disrupting active entries.
         * </p>
         *
         * @return A pair consisting of:
         *         <ol>
         *           <li>the current storage capacity</li>
         *           <li>the number of active entries</li>
         *         </ol>
         */
        std::pair<std::size_t, std::size_t> occupancy_rate() {
            std::shared_lock lk(pool_mtx_);
            return {storage_.capacity(), entries_.size()};
        }

        /**
         * @brief Shrinks internal storage to fit active entries.
         *
         * <p>
         * This function scans for the highest-index active slot and reduces
         * the capacity of internal storage to the smallest power-of-two
         * sufficient to hold all active entries, subject to a minimum
         * reserved size.
         * </p>
         *
         * <p>
         * This operation acquires exclusive locks and must not be performed
         * concurrently with active dereferencing unless guarded.
         * </p>
         */
        void resize_pool() {
            std::unique_lock entry_lock(entry_mtx_);
            std::unique_lock pool_lock(pool_mtx_);

            // 1. Find last occupied slot
            auto rit = std::find_if(occupation_.rbegin(), occupation_.rend(),
                                    [](uint8_t x) { return x == 1; });
            size_t last = 0;
            if (rit != occupation_.rend()) {
                last = occupation_.size() - 1 - std::distance(occupation_.rbegin(), rit);
            }
            size_t need = last + 1;
            size_t new_cap = std::max<size_t>(MIN_RESERVED_SIZE, std::bit_ceil(need));

            if (storage_.capacity() <= new_cap) return;

            // 2. Shrink to fit
            storage_.resize(new_cap);
            storage_.shrink_to_fit();
            occupation_.resize(new_cap);
            occupation_.shrink_to_fit();
            entries_.shrink_to_fit();
            refcounts_.resize(new_cap);
            refcounts_.shrink_to_fit();
        }
    };

} // namespace jh::conc
