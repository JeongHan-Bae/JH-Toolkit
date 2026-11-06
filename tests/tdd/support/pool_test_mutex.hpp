/**
 * @file pool_test_mutex.hpp
 * @brief Read/write mutex composed from a mutex and reader count for pool tests.
 */

#pragma once

#include <atomic>
#include <cstddef>
#include <condition_variable>
#include <mutex>

#include "jh/conceptual/mutex_like.h"

namespace test {
    class pool_test_mutex final {
        std::mutex entrance_;
        std::mutex readers_mutex_;
        std::condition_variable readers_done_;
        std::atomic<std::size_t> readers_{0};

    public:
        void lock() {
            std::unique_lock entrance(entrance_);
            std::unique_lock wait(readers_mutex_);
            readers_done_.wait(wait, [&] {
                return readers_.load(std::memory_order_acquire) == 0;
            });
            entrance.release();
        }

        bool try_lock() {
            if (!entrance_.try_lock()) return false;
            if (readers_.load(std::memory_order_acquire) == 0) return true;
            entrance_.unlock();
            return false;
        }

        void unlock() noexcept {
            entrance_.unlock();
        }

        void lock_shared() {
            std::lock_guard entrance(entrance_);
            readers_.fetch_add(1, std::memory_order_acq_rel);
        }

        bool try_lock_shared() {
            if (!entrance_.try_lock()) return false;
            readers_.fetch_add(1, std::memory_order_acq_rel);
            entrance_.unlock();
            return true;
        }

        void unlock_shared() {
            if (readers_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                std::lock_guard wait(readers_mutex_);
                readers_done_.notify_one();
            }
        }
    };

    static_assert(jh::concepts::rw_mutex_like<pool_test_mutex>);
}
