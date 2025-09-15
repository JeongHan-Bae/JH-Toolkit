/**
 * Copyright 2025 JeongHan-Bae <mastropseudo@gmail.com>
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
 */


/**
 * @file sim_pool.h
 * @author JeongHan-Bae <mastropseudo@gmail.com>
 * @brief A generic weak_ptr-based object pool for modern C++. (Base pool in namespace jh)
 *
 * @details
 * `sim_pool` stands for **"simple pool"**, designed to provide an efficient, thread-safe, and content-aware
 * object pooling mechanism. It is particularly useful for scenarios where objects are frequently created and destroyed.
 * It ensures that objects are **automatically pooled** and **efficiently managed**, avoiding duplicate instances
 * while also automatically cleaning up expired references.
 *
 * ## Design Rationale
 * - Uses **weak_ptr-based object pooling** to avoid unnecessary memory allocation.
 * - Supports **content-based lookup** via custom **hashing and equality policies** (CRUCIAL for content-based
 *   deduplication, std::hash should not be used as it only hashes the address).
 * - Implements **dynamic expansion & contraction** of storage based on object lifetime.
 * - Provides **thread-safety** via `std::shared_mutex` and `std::atomic<std::uint64_t>`.
 *
 * ## Key Features
 * - **Automatic object pooling**: Prevents redundant `shared_ptr` allocations.
 * - **Dynamic memory management**: Expands when full, shrinks when mostly empty.
 * - **Thread-safe implementation**: Safe for concurrent access.
 * - **Flexible lookup strategy**: Supports custom `hash` and `eq` for object identity comparison.
 *
 * ## Notes
 * - `sim_pool` is designed as a base-class for pooling in jh-toolkit. Enabling specific content-based lookup.
 * - If a struct or class (obj) has already realized `hash` and `operator==` functions, it can be automatically pooled
 *   by jh::pool<obj>, (include `<jh/pool.h>` instead of `<jh/sim_pool.h>` and refer to pool.h for more details.)
 *
 * ## Extending `sim_pool`
 * Developers using `sim_pool` need to provide custom hash (`Hash`) and equality (`Eq`) functions.
 * - `Hash` should generate a **content-based** hash to allow deduplication of semantically identical objects.
 * - `Eq` should compare objects based on **content**, not memory addresses.
 * - Expired weak pointers (`expired() == true`) are **always treated as distinct** to ensure proper cleanup.
 * - Objects will **remain in the pool** until `expand_and_cleanup()` is triggered.
 * - If `Eq` treats expired objects as equal, they **WILL NOT be removed** automatically.
 *
 * **For automatic pooling of types that already implement `operator==` and `T::hash()`**,
 * consider using `jh::pool<T>` by including `<jh/pool.h>`.
 *
 * @version 1.3.x
 * @date 2025
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
     * @brief A weak_ptr-based object pool with dynamic memory management.
     *
     * @tparam T The type of object stored in the pool (`std::shared_ptr<T>` is required).
     * @tparam Hash Custom hash function for object lookup.
     *             The purpose of `Hash` is to allow **content-based lookup** rather than pointer-based lookup.
     *             By default, `std::hash<std::shared_ptr<T>>` only hashes the raw pointer itself,
     *             but in `sim_pool`, we use a custom function to **hash the actual content** of `T`
     *             to enable meaningful deduplication.
     * @tparam Eq Custom equality function for object comparison.
     *            - `Eq` should treat `expired()` `weak_ptr` as **distinct**, meaning expired objects will remain
     *              in the pool until `expand_and_cleanup()` is triggered. Cleanup happens **only at specific intervals**.
     *            - If `Eq` treats `expired()` `weak_ptr` as equal, **they will NOT be removed automatically** from the pool.
     *              This behavior **may be unexpected** to users. Instead, `expand_and_cleanup()` is still required
     *              to remove them at expansion points.
     */
    template<typename T, typename Hash, typename Eq>
    struct sim_pool {
        static std::uint64_t constexpr MIN_RESERVED_SIZE = 16; ///< The minimum reserved size for the pool.

        /**
         * @brief Constructs a `sim_pool` with an initial reserved size.
         *
         * @param reserve_size The initial number of elements the pool should reserve (default: 16).
         */
        explicit sim_pool(const std::uint64_t reserve_size = MIN_RESERVED_SIZE)
            : reserved_size_(reserve_size) {
            pool_.reserve(reserved_size_.load());
        }

        sim_pool(const sim_pool &) = delete;

        sim_pool &operator=(const sim_pool &) = delete;

        sim_pool(sim_pool &&other) noexcept {
            std::unique_lock write_lock(other.pool_mutex_);
            pool_ = std::move(other.pool_);
            reserved_size_.store(other.reserved_size_.load());
        }

        sim_pool &operator=(sim_pool &&other) noexcept {
            if (this != &other) {
                std::lock_guard lock_this(pool_mutex_);
                std::lock_guard lock_other(other.pool_mutex_);
                pool_ = std::move(other.pool_);
                reserved_size_.store(other.reserved_size_.load());
            }
            return *this;
        }

        /**
         * @brief Prevents `acquire()` from being used on `const sim_pool`.
         *
         * @tparam Args Constructor arguments for `T`.
         */
        template<typename... Args>
        [[maybe_unused]] std::shared_ptr<T> acquire(Args &&... args) const = delete;

        /**
         * @brief Retrieves an object from the pool, or creates a new one if none exists.
         *
         * @details
         * If an equivalent object already exists in the pool (as determined by `Eq`),
         * this function returns a `shared_ptr` to the existing instance.
         * Otherwise, it creates a new instance, adds it to the pool, and returns it.
         *
         * @tparam Args Constructor arguments for `T`.
         * @return A `shared_ptr<T>` containing the requested object.
         */
        template<typename... Args>
        std::shared_ptr<T> acquire(Args &&... args) {
            auto new_obj = std::make_shared<T>(std::forward<Args>(args)...);
            return get_or_insert(new_obj);
        }

        /**
         * @brief Cleans up expired weak pointers from the pool.
         *
         * @details
         * Removes all `weak_ptr`s from the pool that have expired (i.e., their `shared_ptr` is destroyed).
         * This helps maintain an efficient pool size and prevents unnecessary memory usage.
         */
        void cleanup() {
            std::lock_guard<std::shared_mutex> lock(pool_mutex_);
            cleanup_nolock();
        }

        /**
         * @brief Cleans up expired weak pointers and shrinks the reserved size if necessary.
         *
         * @details
         * - This function first removes expired `weak_ptr`s from the pool, just like `cleanup()`.
         * - After cleanup, it checks if the current pool size is **less than 1/4** of the reserved size:
         *   - If true, it **shrinks the reserved size by half** (but never below `MIN_RESERVED_SIZE`).
         *   - This ensures the pool does not hold excessive reserved memory when underutilized.
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
         * @brief Clears all objects from the pool.
         *
         * @details
         * - This removes all elements from the pool and resets `reserved_size_` to its min value.
         * - Useful for fully resetting the pool to its initial state.
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
         * @brief Inserts a `shared_ptr` into the pool if it does not already exist.
         *
         * @details
         * - First, the function checks if an equivalent object is already in the pool.
         * - If found, it returns a `shared_ptr` to the existing instance.
         * - If not found, it inserts the new object into the pool and returns it.
         * - Expired `weak_ptr`s are **automatically removed** from the pool.
         *
         * @param obj The `shared_ptr<T>` to insert.
         * @return A `shared_ptr<T>` containing the existing or newly inserted object.
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

        // Internal cleanup without locking
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
         * @brief Expands or shrinks the pool's reserved size based on usage.
         *
         * @details
         * - **Expansion**:
         *   - If the pool size reaches the reserved limit, or
         *   - If the pool size after cleanup still exceeds `HIGH_WATERMARK_RATIO * reserved_size_`
         *     (to prevent frequent re-expansion shortly after cleanup),
         *   then the reserved size is doubled (`*2`).
         *
         * - **Contraction**:
         *   - If the pool size falls below `LOW_WATERMARK_RATIO * reserved_size_`,
         *     the reserved size is halved (`/2`).
         *
         * - The reserved size will never shrink below `MIN_RESERVED_SIZE`.
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
