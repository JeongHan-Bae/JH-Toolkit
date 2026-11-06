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
 * @file pointer_pool.h
 * @brief Pointer-stable interning with reference-counted handles and a configurable mutex.
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 *
 * Candidates are constructed in independent heap nodes, so <code>T</code>
 * need not be movable. The pool stores cached hashes and owning node pointers
 * in sorted, reusable vector slots. A sorted table tracks every empty slot.
 * Candidate hashes are computed before locking;
 * matching objects are compared under the selected <code>Mutex</code>.
 *
 * Returned handles use the same mutex for reference counts. The final handle
 * release destroys its node; the index keeps a reusable empty slot. Handles
 * remain valid after the pool is cleared or destroyed.
 */

#pragma once

#include <cstddef>          // for std::size_t
#include <algorithm>
#include <vector>
#include <mutex>            // For lock guards
#include <memory>
#include <shared_mutex>
#include <utility>
#include <limits>
#include <iterator>

#include "jh/synchronous/strong_lock.h"


namespace jh::conc {

    /**
     * @brief Content-based pool for heap-allocated, pointer-stable objects.
     * @tparam T Object type; it may be non-copyable and non-movable.
     * @tparam Hash Callable taking <code>const T&amp;</code> and returning a hash.
     * @tparam Eq Callable comparing two <code>const T&amp;</code> objects.
     * @tparam Mutex RW mutex used for the index, comparisons, and handle counts.
     *
     * The pool constructs a candidate before lookup. It computes the hash
     * once, searches the index, and compares only entries with that hash.
     * The returned <code>ptr</code> owns a reference to the canonical
     * object. Hash and equality must be stable while the object is observed.
     * Callbacks run under the pool mutex during equality comparison and must
     * not reenter the same pool.
     */
    template<typename T, typename Hash, typename Eq, typename Mutex = std::shared_mutex>
    requires(jh::concepts::rw_mutex_like<Mutex> &&
             requires(const T &value) {
                 { Hash{}(value) } -> std::convertible_to<std::size_t>;
             } &&
             requires(const T &lhs, const T &rhs) {
                 { Eq{}(lhs, rhs) } -> std::convertible_to<bool>;
             })
    class pointer_pool final {
    public:
        static constexpr std::size_t MIN_RESERVED_SIZE = 16;

    private:
        struct node {
            template<typename... Args>
            explicit node(Args &&... args) : value(std::forward<Args>(args)...) {}

            T value;
            std::size_t hash = 0;
            std::size_t references = 0;
        };

        struct slot {
            std::size_t hash = std::numeric_limits<std::size_t>::max();
            std::unique_ptr<node> value;
        };

        struct state {
            explicit state(const std::size_t reserve_size)
                : slots(std::max(reserve_size, std::size_t{1})) {
                free_slots.reserve(slots.size());
                for (std::size_t index = 0; index < slots.size(); ++index)
                    free_slots.push_back(index);
            }

            Mutex mutex;
            std::vector<slot> slots;
            std::vector<std::size_t> free_slots;
            std::size_t live = 0;
            std::size_t references = 1; // Pool ownership plus one reference per handle.
        };

        static void rebuild(state *owner, std::size_t capacity) {
            std::vector<slot> next(capacity);
            std::vector<std::size_t> free_slots;
            free_slots.reserve(capacity);
            if (owner->live == 0) {
                for (std::size_t index = 0; index < capacity; ++index)
                    free_slots.push_back(index);
                owner->slots.swap(next);
                owner->free_slots.swap(free_slots);
                return;
            }
            std::size_t position = 0;
            std::size_t inserted = 0;
            std::size_t preceding_hash = 0;
            for (auto &old : owner->slots) {
                if (!old.value) continue;
                const std::size_t target = (++inserted * capacity) / (owner->live + 1);
                while (position < target) {
                    next[position].hash = preceding_hash;
                    free_slots.push_back(position++);
                }
                next[position].hash = old.hash;
                next[position++].value = std::move(old.value);
                preceding_hash = old.hash;
            }
            while (position < capacity) {
                next[position].hash = preceding_hash;
                free_slots.push_back(position++);
            }
            owner->slots.swap(next);
            owner->free_slots.swap(free_slots);
        }

        static auto matching_range(state *owner, std::size_t hash) {
            auto first = std::lower_bound(owner->slots.begin(), owner->slots.end(), hash,
                    [](const slot &entry, std::size_t key) { return entry.hash < key; });
            auto last = std::upper_bound(first, owner->slots.end(), hash,
                    [](std::size_t key, const slot &entry) { return key < entry.hash; });
            return std::pair{first, last};
        }

        static void retain(state *owner, node *value) {
            jh::sync::posix_smtx_unique_lock lock(owner->mutex);
            ++value->references;
            ++owner->references;
        }

        static void release(state *owner, node *value) {
            std::unique_ptr<node> retired;
            bool destroy_state = false;
            {
                jh::sync::posix_smtx_unique_lock lock(owner->mutex);
                if (--value->references == 0) {
                    auto [first, last] = matching_range(owner, value->hash);
                    for (auto it = first; it != last; ++it) {
                        if (it->value.get() == value) {
                            const std::size_t index = static_cast<std::size_t>(it - owner->slots.begin());
                            auto vacant = std::lower_bound(owner->free_slots.begin(),
                                    owner->free_slots.end(), index);
                            owner->free_slots.insert(vacant, index);
                            retired = std::move(it->value);
                            --owner->live;
                            break;
                        }
                    }
                }
                destroy_state = --owner->references == 0;
            }
            retired.reset();
            if (destroy_state) delete owner;
        }

        static void close(state *owner) {
            bool destroy_state;
            {
                jh::sync::posix_smtx_unique_lock lock(owner->mutex);
                destroy_state = --owner->references == 0;
            }
            if (destroy_state) delete owner;
        }

    public:
        /**
         * @brief Reference-counted handle whose lifetime uses the pool's mutex type.
         *
         * Handles remain valid after the pool is moved, cleared, or destroyed.
         * Concurrent access to the same handle object requires external synchronization.
         */
        class ptr final {
            friend class pointer_pool;
            state *owner_ = nullptr;
            node *value_ = nullptr;

            ptr(state *owner, node *value) noexcept : owner_(owner), value_(value) {}

        public:
            /// @brief Constructs an empty handle.
            ptr() noexcept = default;
            /// @brief Constructs an empty handle from nullptr.
            ptr(std::nullptr_t) noexcept {}

            /// @brief Copies a handle and retains its object under the configured mutex.
            ptr(const ptr &other) : owner_(other.owner_), value_(other.value_) {
                if (value_) retain(owner_, value_);
            }

            /// @brief Transfers a handle without changing its reference count.
            ptr(ptr &&other) noexcept : owner_(other.owner_), value_(other.value_) {
                other.owner_ = nullptr;
                other.value_ = nullptr;
            }

            /// @brief Releases the held reference, if any.
            ~ptr() { reset(); }

            /// @brief Copies another handle.
            ptr &operator=(const ptr &other) {
                if (this != &other) {
                    ptr copy(other);
                    swap(copy);
                }
                return *this;
            }

            /// @brief Transfers another handle.
            ptr &operator=(ptr &&other) noexcept {
                if (this != &other) {
                    reset();
                    owner_ = other.owner_;
                    value_ = other.value_;
                    other.owner_ = nullptr;
                    other.value_ = nullptr;
                }
                return *this;
            }

            /// @brief Releases the current object and becomes empty.
            void reset() {
                if (!value_) return;
                state *owner = owner_;
                node *value = value_;
                owner_ = nullptr;
                value_ = nullptr;
                release(owner, value);
            }

            /// @brief Exchanges two handles without changing reference counts.
            void swap(ptr &other) noexcept {
                std::swap(owner_, other.owner_);
                std::swap(value_, other.value_);
            }

            /// @brief Returns the owned object pointer, or nullptr.
            [[nodiscard]] T *get() const noexcept { return value_ ? &value_->value : nullptr; }
            /// @brief Accesses the owned object.
            [[nodiscard]] T &operator*() const noexcept { return *get(); }
            /// @brief Accesses the owned object's members.
            [[nodiscard]] T *operator->() const noexcept { return get(); }
            /// @brief Reports whether this handle owns an object.
            explicit operator bool() const noexcept { return value_ != nullptr; }

            /// @brief Returns the number of handles to this object.
            [[nodiscard]] std::size_t use_count() const {
                if (!value_) return 0;
                jh::sync::posix_smtx_shared_lock lock(owner_->mutex);
                return value_->references;
            }

            /// @brief Compares object identity.
            friend bool operator==(const ptr &lhs, const ptr &rhs) noexcept {
                return lhs.value_ == rhs.value_;
            }
            /// @brief Compares a handle with nullptr.
            friend bool operator==(const ptr &value, std::nullptr_t) noexcept {
                return !value;
            }
        };

        /**
         * @brief Constructs a pool with the requested number of index slots.
         * @param reserve_size Initial slot count, clamped to at least one.
         */
        explicit pointer_pool(const std::size_t reserve_size = MIN_RESERVED_SIZE)
            : state_(new state(reserve_size)) {}

        /// @brief Releases the pool's index while existing handles remain valid.
        ~pointer_pool() { close(state_); }

        /// @brief Pool copying is disabled.
        pointer_pool(const pointer_pool &) = delete;
        /// @brief Pool copy assignment is disabled.
        pointer_pool &operator=(const pointer_pool &) = delete;

        /// @brief Transfers the index and leaves the source pool empty.
        pointer_pool(pointer_pool &&other) : state_(new state(MIN_RESERVED_SIZE)) {
            jh::sync::posix_smtx_unique_lock lock(other.state_mutex_);
            std::swap(state_, other.state_);
        }

        /// @brief Transfers the index and leaves the source pool empty.
        pointer_pool &operator=(pointer_pool &&other) {
            if (this != &other) {
                state *replacement = new state(MIN_RESERVED_SIZE);
                state *old;
                {
                    jh::sync::posix_smtx_unique_lock this_lock(state_mutex_);
                    jh::sync::posix_smtx_unique_lock other_lock(other.state_mutex_);
                    old = state_;
                    state_ = other.state_;
                    other.state_ = replacement;
                }
                close(old);
            }
            return *this;
        }

        /// @brief Acquisition through a const pool is disabled.
        template<typename... Args>
        [[maybe_unused]] ptr acquire(Args &&... args) const = delete;

        /**
         * @brief Constructs a candidate and returns the canonical object handle.
         * @tparam Args Constructor argument types for T.
         * @param args Arguments used to construct the candidate.
         * @return A handle that remains valid after the pool is destroyed.
         *
         * Hashing happens before the pool lock. Equality and index changes use
         * the configured mutex. The index holds each node while handles refer to it.
         */
        template<typename... Args>
        ptr acquire(Args &&... args) {
            auto candidate = std::make_unique<node>(std::forward<Args>(args)...);
            const std::size_t hash = Hash{}(candidate->value);
            jh::sync::posix_smtx_shared_lock state_lock(state_mutex_);
            state *owner = state_;
            jh::sync::posix_smtx_unique_lock lock(owner->mutex);
            std::size_t capacity = owner->slots.size();
            auto [first, last] = matching_range(owner, hash);
            for (auto it = first; it != last; ++it) {
                if (it->value && Eq{}(it->value->value, candidate->value)) {
                    ++it->value->references;
                    ++owner->references;
                    return ptr(owner, it->value.get());
                }
            }
            node *value = candidate.get();
            value->hash = hash;
            value->references = 1;
            if (owner->free_slots.empty()) {
                capacity *= 2;
                rebuild(owner, capacity);
            }
            const std::size_t target = static_cast<std::size_t>(
                    std::upper_bound(owner->slots.begin(), owner->slots.end(), hash,
                            [](std::size_t key, const slot &entry) { return key < entry.hash; })
                    - owner->slots.begin());
            auto right_it = std::lower_bound(owner->free_slots.begin(),
                    owner->free_slots.end(), target);
            const std::size_t right = right_it == owner->free_slots.end() ? capacity : *right_it;
            const std::size_t left = right_it == owner->free_slots.begin()
                    ? capacity : *std::prev(right_it);
            const bool use_left = left != capacity &&
                    (right == capacity || target - left <= right - target);
            std::size_t destination_index;
            if (use_left) {
                owner->free_slots.erase(std::prev(right_it));
                for (std::size_t i = left; i + 1 < target; ++i)
                    owner->slots[i] = std::move(owner->slots[i + 1]);
                destination_index = target - 1;
            } else {
                owner->free_slots.erase(right_it);
                for (std::size_t i = right; i > target; --i)
                    owner->slots[i] = std::move(owner->slots[i - 1]);
                destination_index = target;
            }
            slot &destination = owner->slots[destination_index];
            destination.hash = hash;
            destination.value = std::move(candidate);
            ++owner->live;
            ++owner->references;
            return ptr(owner, value);
        }

        /// @brief Rebuilds the index without expired slots.
        void cleanup() {
            jh::sync::posix_smtx_shared_lock state_lock(state_mutex_);
            jh::sync::posix_smtx_unique_lock lock(state_->mutex);
            rebuild(state_, state_->slots.size());
        }

        /// @brief Rebuilds the index and halves capacity below one-quarter occupancy.
        void cleanup_shrink() {
            jh::sync::posix_smtx_shared_lock state_lock(state_mutex_);
            jh::sync::posix_smtx_unique_lock lock(state_->mutex);
            std::size_t capacity = state_->slots.size();
            if (capacity > MIN_RESERVED_SIZE && state_->live <= capacity / 4)
                capacity = std::max(capacity / 2, MIN_RESERVED_SIZE);
            rebuild(state_, capacity);
        }

        /// @brief Returns the number of currently observed live objects.
        [[nodiscard]] std::size_t size() const {
            jh::sync::posix_smtx_shared_lock state_lock(state_mutex_);
            jh::sync::posix_smtx_shared_lock lock(state_->mutex);
            return state_->live;
        }

        /// @brief Returns the number of index slots.
        [[nodiscard]] std::size_t capacity() const {
            jh::sync::posix_smtx_shared_lock state_lock(state_mutex_);
            jh::sync::posix_smtx_shared_lock lock(state_->mutex);
            return state_->slots.size();
        }

        /// @brief Starts a fresh index while existing handles keep their objects alive.
        void clear() {
            state *replacement = new state(MIN_RESERVED_SIZE);
            state *old;
            {
                jh::sync::posix_smtx_unique_lock lock(state_mutex_);
                old = state_;
                state_ = replacement;
            }
            close(old);
        }

    private:
        mutable Mutex state_mutex_;
        state *state_;
    };

} // namespace jh::conc
