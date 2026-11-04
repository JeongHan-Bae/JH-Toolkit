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
 * @file weak_ptr.h
 * @brief Atomic storage for a weak ownership handle.
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 */

#pragma once

#include "jh/atomic/shared_ptr.h"
#include <mutex>

namespace jh::atomic {
    /**
     * @brief Atomically stores a weak pointer; load a snapshot before calling lock.
     * @tparam T Managed object type.
     */
    template<typename T>
    class weak_ptr final {
    public:
        using value_type = std::weak_ptr<T>;
#if JH_HAS_ATOMIC_SMART_PTR
        static constexpr bool is_always_lock_free = std::atomic<value_type>::is_always_lock_free;
#else
        static constexpr bool is_always_lock_free = false;
#endif

        /// @brief Construct an empty slot.
        weak_ptr() noexcept = default;

        /// @brief Construct a slot with a weak handle.
        weak_ptr(value_type desired) noexcept: value_(std::move(desired)) {}

        /// @brief Construct a slot observing a shared handle.
        weak_ptr(std::shared_ptr<T> desired) noexcept: weak_ptr(value_type(std::move(desired))) {}

        weak_ptr(const weak_ptr &) = delete;

        weak_ptr &operator=(const weak_ptr &) = delete;

        weak_ptr(weak_ptr &&) = delete;

        weak_ptr &operator=(weak_ptr &&) = delete;

        /// @brief Test whether operations on this slot are lock-free.
        bool is_lock_free() const noexcept {
#if JH_HAS_ATOMIC_SMART_PTR
            return value_.is_lock_free();
#else
            return false;
#endif
        }

        /// @brief Read a weak snapshot using a valid load order.
        value_type load(std::memory_order order = std::memory_order_seq_cst) const noexcept {
#if JH_HAS_ATOMIC_SMART_PTR
            return value_.load(order);
#else
            (void) order;
            std::lock_guard<std::mutex> lock(mutex_);
            return value_;
#endif
        }

        /// @brief Read a weak snapshot with sequential consistency.
        operator value_type() const noexcept { return load(); }

        /// @brief Publish a weak handle using a valid store order.
        void store(value_type desired, std::memory_order order = std::memory_order_seq_cst) noexcept {
#if JH_HAS_ATOMIC_SMART_PTR
            value_.store(std::move(desired), order);
#else
            (void) order;
            std::lock_guard<std::mutex> lock(mutex_);
            value_ = std::move(desired);
#endif
        }

        /// @brief Publish a weak handle and return this slot.
        weak_ptr &operator=(value_type desired) noexcept {
            store(std::move(desired));
            return *this;
        }

        /// @brief Replace the slot and return its previous weak handle.
        value_type exchange(value_type desired, std::memory_order order = std::memory_order_seq_cst) noexcept {
#if JH_HAS_ATOMIC_SMART_PTR
            return value_.exchange(std::move(desired), order);
#else
            (void) order;
            std::lock_guard<std::mutex> lock(mutex_);
            value_.swap(desired);
            return desired;
#endif
        }

    private:
#if JH_HAS_ATOMIC_SMART_PTR
        std::atomic<value_type> value_;
#else
        mutable std::mutex mutex_;
        value_type value_;
#endif
    };
}
