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
 * @file data_sink.h
 * @author JeongHan-Bae <mastropseudo@gmail.com>
 * @brief High-performance append-only data structure for sequential storage and retrieval.
 *
 * @details
 * `data_sink` is a specialized, high-performance container designed for fast, sequential
 * data storage with a focus on **low overhead**, **cache efficiency**, and **predictable memory usage**.
 * It is optimized for **append-only** workloads where elements are written in bulk and later retrieved.
 *
 * Unlike `std::deque`, `data_sink` provides **contiguous block allocation** with user-configurable
 * block sizes (`BLOCK_SIZE`), ensuring minimal memory fragmentation while maintaining high-speed
 * write performance. It **does not support removal operations**, making it ideal for logging,
 * batch processing, and large-scale data buffering.
 *
 * `data_sink` is optimized for **raw fundamental types** (integers, floating-point numbers, and `bool`),
 * **raw pointers**, and **unique pointers (`std::unique_ptr<T>`)**.
 * Complex types such as `std::string` or user-defined structs are **not supported**
 * unless wrapped in `std::unique_ptr<T>`.
 *
 * ## Block Size Considerations (`BLOCK_SIZE`):
 * - **Larger `BLOCK_SIZE`** (e.g., 8192) improves sequential write performance by reducing dynamic allocations.
 * - **Smaller `BLOCK_SIZE`** minimizes memory waste but may lead to more frequent memory allocations.
 *
 * @note Since `T` is restricted to **1, 2, 4, or 8 bytes** in size (or a pointer),
 * memory alignment is optimal for CPU cache efficiency.
 *
 * @note `data_sink` is **strictly append-only**:
 * - **No mutation**: Once added, elements **cannot be modified**,
 *   except for **batch transformations** via `inplace_map()`.
 * - **No removal**: `data_sink` does **not** support `pop()`, `erase()`, or `remove()`.
 * - **Read-only iteration**: All iterators provide `const` access.
 * - **For `T = std::unique_ptr<U>`**:
 *   - `emplace_back(args...)` constructs `U` **inside** `data_sink`.
 *   - **Cannot `move` `std::unique_ptr<U>` out** (iterators provide `const` access).
 *   - Access stored objects using `*it` or `it->member`.
 * - **Best Use Cases**:
 *   - High-throughput, append-only data storage
 *   - Log buffering & event streaming
 *   - Large-scale batch processing & analytics
 *
 * ## Key Features
 * - **Fast sequential writes** with **contiguous block storage**.
 * - **Fixed block size (`BLOCK_SIZE`)**, reducing dynamic allocations.
 * - **FIFO iteration** with minimal overhead.
 * - **`clear()` and `clear_reserve()` ** for efficient memory management.
 * - **Inline mapping** with `inplace_map()` for bulk data transformation.
 *   - `inplace_map()` supports sequential bulk transformations.
 *   - Future versions may introduce parallel execution (`std::execution::par`).
 * - **Optimized cache locality** via aligned memory allocation.
 * - Does *NOT* support multithread safety as most stl containers. Refer to `data_buffer` (example in docs/data_sink.md)
 *   for a simple realization of multi-thread wrapping.
 *
 * ## Design Considerations
 * - **No pop operations**: For efficiency and alignment reasons, `data_sink` **only supports insertion and iteration**.
 * - **Single-threaded optimization**: Not thread-safe by design, but can be wrapped with locks if needed.
 * - **Write-only buffer**: Designed for high-throughput **append-only** scenarios.
 * - **Memory reuse**: `clear_reserve()` allows reusing previously allocated blocks to reduce heap allocations.
 * - **Bulk insert optimization**: Instead of using `std::vector` with `reserve()`, use `bulk_append()` for efficient batch insertion.
 * - **No Random Access**: random access is NOT and will NOT be provided to ensure speed.
 * - **CPU Friendly**: `data_sink` is optimized for CPU cache locality and sequential memory access.
 *   - Uses contiguous memory blocks to reduce cache misses.
 *   - Prefetching and memory alignment to optimize SIMD operations.
 *   - No unnecessary branching, improving branch prediction accuracy.
 *
 * ## A Fun Fact 🐉
 * `data_sink` is like **貔貅 (Pixiu, or 비휴 Bihyu in our Joseonjok pronunciation)**,
 * a legendary Chinese mythical creature known as a **treasure sink** — it can only take in wealth but never let it out.
 * Likewise, `data_sink` **only supports inserting elements, never removing them**,
 * ensuring **maximum performance** and **minimal fragmentation**.
 *
 * Just like how Pixiu will NEVER give back its treasures, `data_sink` will NEVER allow any type of `pop()` or **partial modification**.
 * If you need to modify elements, consider using `std::vector`, pre-allocating with `reserve()`, and using an iterator to fill in values.
 *
 * @usecases
 * - High-throughput logging system
 * - Batch processing buffer
 * - Custom radix sort bucket storage
 * - Large-scale data buffering for streaming applications
 * - High-performance, cache-friendly append-only structure
 * - **Cache-efficient computing buffer for numerical simulations**
 *
 *
 * @version 1.3.x
 * @date 2025
 */


#pragma once

#include <memory>      // NOLINT
#include <type_traits>
#include <cstdint>
#include <functional>
#include <algorithm>
#include <ranges>      // NOLINT
#include <utility>     // NOLINT
#include "iterator.h"

namespace jh::data_sink_restrictions {
    /**
     * @brief Checks whether `U` is a `std::unique_ptr<T>`.
     * @tparam U Type to check.
     */
    template<typename U>
    struct is_unique_ptr : std::false_type {
        using type = U;
    };

    /// @brief Specialization to detect `std::unique_ptr<Reg>` without specialized deleter.
    template<typename U>
    struct is_unique_ptr<std::unique_ptr<U> > : std::true_type {
        using type = U;
    };

    /**
     * @brief Checks if `T` is a fundamental data type allowed in `data_sink`.
     * @tparam T The type to check.
     *
     * Allowed:
     * - Integer types: `int8_t`, `uint8_t`, `int16_t`, `uint16_t`, `int32_t`, `uint32_t`, `int64_t`, `uint64_t`
     * - Floating-point types: `float`, `double`
     * - Boolean type: `bool`
     *
     * @note This restriction ensures **efficient memory alignment** and **optimal cache performance**.
     */
    template<typename T>
    concept is_fundamental_type = (std::is_integral_v<T> || std::is_floating_point_v<T>) &&
                                  (sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8);

    /**
     * @brief Checks if `T` is a valid pointer type (only supports `sizeof(T*) == 8`).
     * @tparam T The type to check.
     *
     * Allowed:
     * - Only raw pointers where `sizeof(T*) == 8`
     *
     * **Why is this restriction applied?**
     * - Ensures **proper memory alignment** on 64-bit architectures.
     * - Avoids accidental use of non-standard pointer sizes.
     */
    template<typename T>
    concept is_valid_pointer_type = std::is_pointer_v<T> && sizeof(T) == 8;

    /**
     * @brief Checks if `T` is a `std::unique_ptr<T>`.
     * @tparam T The type to check.
     *
     * Allowed:
     * - `std::unique_ptr<T>` (to support smart pointer-based dynamic allocation)
     */
    template<typename T>
    concept is_valid_unique_ptr = is_unique_ptr<T>::value;

    /**
     * @brief The set of all valid data types for `data_sink<T, BLOCK_SIZE>`.
     * @tparam T The type to check.
     *
     * Allowed types:
     * - **Fundamental types** (`int`, `float`, `bool`, `char`, etc.).
     * - **Pointer types** (only `sizeof(T*) == 8` is allowed). (32-bit Systems are explicitly unsupported).
     * - **Smart pointers** (`std::unique_ptr<T>`).
     *
     * **Disallowed types**:
     * - STL containers like `std::vector<T>`, `std::list<T>`, `std::map<K, V>`, etc.
     * - Custom user-defined complex classes.
     *
     * @note If you need to store complex types, wrap them in `std::unique_ptr<T>`.
     */
    template<typename T>
    concept valid_data_sink_type = is_fundamental_type<T> || is_valid_pointer_type<T> || is_valid_unique_ptr<T>;

    /**
     * @brief Ensures `BLOCK_SIZE` is at least 1024 and is a power of two.
     * @tparam BLOCK_SIZE The block size to validate.
     *
     * @details This constraint ensures that:
     * - The minimum block size is `1024` to prevent excessive memory fragmentation.
     * - `BLOCK_SIZE` must be a power of two (`BLOCK_SIZE & (BLOCK_SIZE - 1) == 0`)
     *   for fast bitwise calculations and cache efficiency.
     */
    template<std::uint32_t BLOCK_SIZE>
    concept valid_block_size = BLOCK_SIZE >= 1024 && (BLOCK_SIZE & BLOCK_SIZE - 1) == 0;
} // namespace jh::data_sink_restrictions

namespace jh {
    /**
     * @brief A high-performance append-only data structure for sequential storage and retrieval.
     * @tparam T The element type, must be one of the following:
     *   - [Integer types]: `int8_t`, `uint8_t`, `int16_t`, `uint16_t`, `int32_t`, `uint32_t`, `int64_t`, `uint64_t`
     *   - [Floating-point types]: `float`, `double`
     *   - [Boolean type]: `bool`
     *   - [Raw pointers (`U*`)] (only if `sizeof(U*) == 8`)
     *   - [Unique pointers] (`std::unique_ptr<U>`)
     *
     * @tparam BLOCK_SIZE The number of elements per allocated block. Must be a power of two and at least 1024.
     *
     * @details
     * `data_sink<T, BLOCK_SIZE>` is an **append-only** data container optimized for:
     * - **Low overhead memory management**.
     * - **Cache-friendly single-thread block storage**.
     * - **Bulk insertions with minimal dynamic allocations**.
     *
     *
     * @note If you need to partially modify elements after insertion, consider copying to `std::vector<T>`.
     *
     * @note `data_sink<T = std::unique_ptr<U>>` **strictly manages U inside the container**:
     * - `emplace_back(args...)` **directly constructs U** inside `data_sink`.
     * - **Cannot `move` `std::unique_ptr<U>` out** (iterators provide **const** access).
     * - Use `*it` or `it->member` to access stored objects.
     * - **Best practice**: Store lightweight **custom structures (`struct MyData {...};`)**.
     */
    template<typename T, std::uint32_t BLOCK_SIZE = 8192>
        requires data_sink_restrictions::valid_data_sink_type<T> &&
                 data_sink_restrictions::valid_block_size<BLOCK_SIZE>
    struct data_sink : std::ranges::view_interface<data_sink<T, BLOCK_SIZE> > {
    private:
        /// @brief Node specialization for fundamental types and raw pointers.
        template<typename Reg>
        struct node_regular final {
            alignas(alignof(Reg)) Reg data_[BLOCK_SIZE]; ///< Optimized aligned storage.
            std::uint32_t size_ = 0;
            std::unique_ptr<node_regular> next;

            explicit node_regular() : next(nullptr) {
            }

            [[nodiscard]] bool full() const { return size_ == BLOCK_SIZE; }

            template<typename... Args>
            void emplace(Args &&... args) {
                new(&this->data_[this->size_]) Reg(std::forward<Args>(args)...);
                ++this->size_;
            }
        };

        /// @brief Node specialization for `std::unique_ptr<Inner>`.
        template<typename Inner>
        struct node_unique_ptr final {
            std::unique_ptr<Inner> data_[BLOCK_SIZE]; ///< No explicit alignment needed.
            std::uint32_t size_ = 0;
            std::unique_ptr<node_unique_ptr> next;

            explicit node_unique_ptr() : next(nullptr) {
            }

            [[nodiscard]] bool full() const { return size_ == BLOCK_SIZE; }

            template<typename... Args>
            void emplace(Args &&... args) {
                data_[size_].reset(new Inner(std::forward<Args>(args)...));
                ++this->size_;
            }
        };

        /// **Selects the correct `node` type based on `Reg`**
        using node = std::conditional_t<
            data_sink_restrictions::is_unique_ptr<T>::value, // Check if Reg is `std::unique_ptr<Inner>`
            node_unique_ptr<typename data_sink_restrictions::is_unique_ptr<T>::type>, // Select unique_ptr node
            node_regular<T> // Otherwise, select regular node
        >;

        std::unique_ptr<node> head_; ///< Pointer to the first block.
        node *tail_; ///< Pointer to the last block. (should not use alignas() or might lead to UB.)
        std::uint64_t size_; ///< Number of elements stored.

    public:
        /**
         * @brief Constructs an empty `data_sink`.
         */
        explicit data_sink() : head_(nullptr), tail_(nullptr), size_(0) {
        }

        /**
         * @brief Checks whether the container is empty.
         * @return `true` if empty, otherwise `false`.
         */
        [[nodiscard]] bool empty() const {
            return size_ == 0;
        }

        /**
         * @brief Appends an element to the container.
         *
         * @tparam Args Types of arguments for constructing `T` If `T` is not a unique_ptr type,
         * Else types of arguments for constructing `U` if T = std::unique_ptr<U>.
         * @param args Arguments forwarded to the constructor.
         */
        template<typename... Args>
        void emplace_back(Args &&... args) {
            if (!head_) [[unlikely]] {
                // Instead of checking empty, check if head is nullptr in case last called clear_reserve
                head_ = std::make_unique<node>();
                tail_ = head_.get();
            } else if (const std::uint32_t tail_size = tail_->size_; tail_size == BLOCK_SIZE) [[unlikely]] {
                if (!tail_->next) {
                    tail_->next = std::make_unique<node>();
                } else {
                    tail_->next->size_ = 0;
                }
                tail_ = tail_->next.get();
            }
            tail_->emplace(std::forward<Args>(args)...);
            ++size_;
        }

        /**
         * @brief Appends a range of elements to the container.
         *
         * @tparam R A range satisfying `std::ranges::viewable_range`.
         * @param r The range to append.
         */
        template<std::ranges::viewable_range R>
        void bulk_append(R &&r) {
            for (auto &&val: r) {
                emplace_back(std::forward<decltype(val)>(val));
            }
        }

        /**
         * @brief Provides an iterator to traverse the container.
         */
        struct iterator {
            using iterator_category = std::input_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = const T *;
            using reference = const T &;

            const node *current_node;
            std::uint32_t index;

            iterator() : current_node(nullptr), index(0) {
            }

            explicit iterator(const node *n, const std::uint32_t idx = 0) : current_node(n), index(idx) {
            }

            reference operator*() const { return current_node->data_[index]; }

            iterator &operator++() {
                ++index;
                if (index >= current_node->size_) [[unlikely]] {
                    current_node = current_node->next.get();
                    index = 0;
                }
                return *this;
            }

            iterator operator++(int) {
                iterator temp = *this;
                ++*this;
                return temp;
            }

            bool operator==(const iterator &other) const {
                return current_node == other.current_node && index == other.index;
            }

            bool operator!=(const iterator &other) const { return !(*this == other); }
        };

        /**
         * @brief Defines a constant iterator type for the container.
         */
        using const_iterator = iterator;

        /**
         * @brief Provides an iterator to the beginning of the container.
         * @return An iterator pointing to the first element.
         */
        [[nodiscard]] const_iterator begin() const { return const_iterator(head_.get(), 0); }

        /**
         * @brief Provides an iterator to the end of the container.
         * @return An iterator representing the past-the-end position.
         */
        [[nodiscard]] static const_iterator end() noexcept { return const_iterator(nullptr, 0); }

        /**
         * @brief Clears the container, releasing all memory.
         */
        void clear() {
            head_.reset();
            // Unique ptr will automatically delete everything, tail_ will be set to nullptr automatically
            size_ = 0;
        }

        /**
         * @brief Clears the container while reserving memory for reuse.
         * @param blocks The number of blocks to retain (optional).
         *  If `std::nullopt` (default), **retains all allocated blocks**.
         *
         * @details Unreserved blocks are automatically freed due to `std::unique_ptr` ownership.
         * This reduces **heap fragmentation** while maintaining **high-speed reallocation**.
         */
        void clear_reserve(const std::optional<std::uint64_t> blocks = std::nullopt) {
            size_ = 0;
            if (!head_) return;
            head_->size_ = 0;
            tail_ = head_.get();
            if (!blocks.has_value()) return; // only if a param is parsed will process
            std::uint64_t count = 0;
            // blocks == 0 will do same as clear()
            auto ptr = head_.get();
            while (ptr && count < blocks.value()) {
                ++count;
                ptr = ptr->next.get();
            }
            if (ptr) {
                ptr->next.reset();
            }
        }

        /**
         * @brief Returns the number of elements in the container.
         * @return The number of stored elements.
         */
        [[nodiscard]] std::uint64_t size() const {
            return size_;
        }

        /**
         * @brief Returns the maximum number of elements per block.
         * @return The block size.
         */
        static constexpr std::uint32_t block_capacity() {
            return BLOCK_SIZE;
        }

        /**
         * @brief Applies a transformation function to all stored elements.
         * @param transform The transformation function.
         *
         * @note `inplace_map()` **modifies all elements in-place**.
         *       Useful for bulk transformations (e.g., normalization, scaling).
         *       🚀 **Optimized for cache efficiency** due to sequential access.
         */
        void inplace_map(const std::function<void(T &)> &transform) noexcept {
            if (!head_) return;
            const std::uint64_t full_blocks = size_ / BLOCK_SIZE;
            node *current = head_.get();
            for (std::uint64_t i = 0; i < full_blocks; ++i) {
                std::for_each(current->data_, current->data_ + BLOCK_SIZE, transform);
                current = current->next.get();
            }
            if (current && current->size_ > 0) {
                std::for_each(current->data_, current->data_ + current->size_, transform);
            }
        }

        /**
         * @brief Converts the container to a `std::ranges::subrange`.
         * @return A subrange representing the stored elements.
         */
        explicit operator std::ranges::subrange<iterator>() const {
            return std::ranges::subrange<iterator>(begin(), end());
        }
    };

    template<typename T, std::uint32_t BLOCK_SIZE>
    struct iterator<data_sink<T, BLOCK_SIZE> > {
        /// @brief Provides concept recognition for iterators in `jh/iterator`.
        using type = typename data_sink<T, BLOCK_SIZE>::iterator;
    };
}
