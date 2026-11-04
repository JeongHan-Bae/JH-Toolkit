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
 * @file shared_ptr.h
 * @brief Atomic storage for a shared ownership handle.
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 */

#pragma once

#include <atomic>
#include <memory>
#include <utility>

#if !defined(JH_ATOMIC_SMART_PTR_FORCE_FALLBACK) && defined(__cpp_lib_atomic_shared_ptr) && __cpp_lib_atomic_shared_ptr >= 201711L
#define JH_HAS_ATOMIC_SMART_PTR 1
#else
#define JH_HAS_ATOMIC_SMART_PTR 0
#endif

namespace jh::atomic {
    /**
     * @brief Atomically stores a shared pointer; load a snapshot before accessing its object.
     * @tparam T Managed object type.
     * @note All translation units sharing this type must select the same implementation path.
     */
    template<typename T>
    class shared_ptr final {
    public:
        using value_type = std::shared_ptr<T>;
#if JH_HAS_ATOMIC_SMART_PTR
        static constexpr bool is_always_lock_free = std::atomic<value_type>::is_always_lock_free;
#else
        static constexpr bool is_always_lock_free = false;
#endif

        /// @brief Construct an empty atomic slot.
        shared_ptr() noexcept = default;

        /// @brief Construct an empty atomic slot from null.
        shared_ptr(std::nullptr_t) noexcept: shared_ptr() {}

        /// @brief Construct a slot containing the supplied handle.
        shared_ptr(value_type desired) noexcept: value_(std::move(desired)) {}

        shared_ptr(const shared_ptr &) = delete;

        shared_ptr &operator=(const shared_ptr &) = delete;

        shared_ptr(shared_ptr &&) = delete;

        shared_ptr &operator=(shared_ptr &&) = delete;

        /// @brief Test whether operations on this slot are lock-free.
        [[nodiscard]] bool is_lock_free() const noexcept {
#if JH_HAS_ATOMIC_SMART_PTR
            return value_.is_lock_free();
#else
            return std::atomic_is_lock_free(&value_);
#endif
        }

        /// @brief Acquire an owning snapshot using a valid load order.
        value_type load(std::memory_order order = std::memory_order_seq_cst) const noexcept {
#if JH_HAS_ATOMIC_SMART_PTR
            return value_.load(order);
#else
            return std::atomic_load_explicit(&value_, order);
#endif
        }

        /// @brief Read an owning snapshot with sequential consistency.
        operator value_type() const noexcept { return load(); }

        /// @brief Publish a handle using a valid store order.
        void store(value_type desired, std::memory_order order = std::memory_order_seq_cst) noexcept {
#if JH_HAS_ATOMIC_SMART_PTR
            value_.store(std::move(desired), order);
#else
            std::atomic_store_explicit(&value_, std::move(desired), order);
#endif
        }

        /// @brief Publish a handle and return this slot.
        shared_ptr &operator=(value_type desired) noexcept {
            store(std::move(desired));
            return *this;
        }

        /// @brief Clear the slot.
        shared_ptr &operator=(std::nullptr_t) noexcept {
            store(nullptr);
            return *this;
        }

        /// @brief Replace the slot and return its previous owning handle.
        [[nodiscard]] value_type
        exchange(value_type desired, std::memory_order order = std::memory_order_seq_cst) noexcept {
#if JH_HAS_ATOMIC_SMART_PTR
            return value_.exchange(std::move(desired), order);
#else
            return std::atomic_exchange_explicit(&value_, std::move(desired), order);
#endif
        }

        /// @brief Attempt a weak replacement, updating expected on failure.
        bool compare_exchange_weak(value_type &expected, value_type desired,
                                   std::memory_order success, std::memory_order failure) noexcept {
#if JH_HAS_ATOMIC_SMART_PTR
            return value_.compare_exchange_weak(expected, std::move(desired), success, failure);
#else
            return std::atomic_compare_exchange_weak_explicit(&value_, &expected, std::move(desired), success, failure);
#endif
        }

        /// @brief Attempt a weak replacement using the standard derived failure order.
        bool compare_exchange_weak(value_type &expected, value_type desired,
                                   std::memory_order order = std::memory_order_seq_cst) noexcept {
#if JH_HAS_ATOMIC_SMART_PTR
            return value_.compare_exchange_weak(expected, std::move(desired), order);
#else
            return compare_exchange_weak(expected, std::move(desired), order, failure_order(order));
#endif
        }

        /// @brief Attempt a strong replacement, updating expected on failure.
        bool compare_exchange_strong(value_type &expected, value_type desired,
                                     std::memory_order success, std::memory_order failure) noexcept {
#if JH_HAS_ATOMIC_SMART_PTR
            return value_.compare_exchange_strong(expected, std::move(desired), success, failure);
#else
            return std::atomic_compare_exchange_strong_explicit(&value_, &expected, std::move(desired), success,
                                                                failure);
#endif
        }

        /// @brief Attempt a strong replacement using the standard derived failure order.
        bool compare_exchange_strong(value_type &expected, value_type desired,
                                     std::memory_order order = std::memory_order_seq_cst) noexcept {
#if JH_HAS_ATOMIC_SMART_PTR
            return value_.compare_exchange_strong(expected, std::move(desired), order);
#else
            return compare_exchange_strong(expected, std::move(desired), order, failure_order(order));
#endif
        }

    private:
        static constexpr std::memory_order failure_order(std::memory_order order) noexcept {
            return order == std::memory_order_acq_rel ? std::memory_order_acquire :
                   order == std::memory_order_release ? std::memory_order_relaxed : order;
        }

#if JH_HAS_ATOMIC_SMART_PTR
        std::atomic<value_type> value_;
#else
        value_type value_;
#endif
    };
}
