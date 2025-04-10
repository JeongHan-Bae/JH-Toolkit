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
 * @file runtime_arr.h
 * @author JeongHan-Bae <mastropseudo@gmail.com>
 * @brief A fixed-size, read-stable runtime array with optional allocator and POD-level optimization.
 *
 * @details
 * `runtime_arr<T, Alloc>` is a minimal, explicitly bounded container designed for high-performance
 * algorithms and data pipelines where dynamic growth is **undesirable or disallowed**.
 *
 * It offers:
 * - Fast initialization, with optional uninitialized construction for POD types
 * - Value-range-safe access (via `at()`, or unsafe fast version via `operator[]`)
 * - Iteration via STL-style pointers and C++20 `view_interface`
 * - Move-only semantics, with opt-in allocator support and efficient `.reset_all()` for zeroing
 *
 * ## Core Behavior
 * - `runtime_arr<T>` behaves like a fixed-capacity, flat buffer with no `resize()` or `insert()` APIs
 * - It does not support element removal, capacity extension, or iterator invalidation semantics
 * - This restriction allows compilers to perform aggressive loop and alias analysis
 *
 * ## Specializations
 * - `runtime_arr<bool>` provides a bit-packed array using `uint64_t[]` as the internal storage
 *   - Fully indexable
 *   - Supports logical access via `bit_ref`, `set()`, `test()`, `unset()`
 *   - Exposes `raw_data()` and `raw_word_count()` for direct bit-level interoperability
 *
 * ## Use Cases
 * - Intermediate buffers in algorithms (sorting, filtering, dynamic programming)
 * - Per-frame storage in simulation pipelines
 * - Temporary space for zero-initialized POD blocks (e.g., bitmap masks, lookup tables)
 * - Efficient serialization and compression routines that demand control over memory layout
 *
 * ## Comparison vs std::vector
 * | Feature                  | std::vector<T>   | runtime_arr<T>             |
 * |--------------------------|------------------|----------------------------|
 * | Dynamic resize           | ✅                | ❌                          |
 * | Default initialization   | ✅                | Optional (`uninitialized`) |
 * | POD memset shortcut      | ❌                | ✅ (`reset_all()`)          |
 * | Allocator override       | ✅                | ✅                          |
 * | Range view compatibility | ✅ (via adaptors) | ✅ (native view_interface)  |
 * | Move into/from vector    | ✅                | ✅                          |
 *
 * ## Notes
 * - POD types benefit most from this structure
 * - For safety-critical code or complex ownership, prefer `std::vector<T>`
 * - For performance-critical code with known sizes, prefer `runtime_arr<T>`
 *
 * ## Version
 * @version 1.3.x
 * @date 2025
 */


#pragma once

#include <cstdint>
#include <vector>
#include <ranges>
#include <stdexcept>
#include <cstring>
#include <functional>
#include "iterator.h"
#include "pod.h"

namespace jh {
    /**
     * @brief A fixed-size, allocator-aware, move-only runtime array.
     *
     * @tparam T The element type stored in the array
     * @tparam Alloc Optional allocator type (defaults to std::monostate)
     *
     * @details
     * The default allocator is `std::monostate`, meaning internal memory is managed via `new[]` and `delete[]`,
     * (which makes this class usable without explicitly passing allocators, but also means no memory pooling, no alignment control).
     * This is suitable for **small POD types** and **moderate-sized data structures** used in intermediate computation.
     * This structure models a non-resizable flat container intended for fast initialization,
     * predictable memory behavior, and algorithmic processing.
     *
     * @note
     * For large total size (e.g., > 1MB) or non-pod types, prefer supplying a custom allocator,
     * such as `std::allocator<T>` or pool allocators, to avoid heap fragmentation and allocator overhead.
     * This container is not meant to replace `std::vector<T>`, but to serve as a **bounded, fast-reset, move-only**
     * alternative in performance-critical contexts.
     */
    template<typename T, typename Alloc = std::monostate>
    struct runtime_arr : std::ranges::view_interface<runtime_arr<T, Alloc> > {
    private:
        std::uint64_t size_{0};

        using deleter_t = std::function<void(T *)>;
        std::unique_ptr<T[], deleter_t> data_{nullptr, make_deleter()}; // Tie deleter to allocator

        static void default_deleter(T *p) { delete[] p; } // NOLINT

        static deleter_t make_deleter() {
            if constexpr (std::is_same_v<Alloc, std::monostate>) {
                return default_deleter;
            } else {
                return nullptr; // No-op deleter, it will be bound to lambda
            }
        }

        template<typename A>
        static constexpr bool is_valid_allocator =
                std::is_same_v<A, std::monostate> || requires(A a, std::uint64_t n)
                {
                    { a.allocate(n) } -> std::same_as<T *>;
                    { a.deallocate(std::declval<T *>(), n) };
                };

    public:
        using iterator = T *;
        using const_iterator = const T *;

        struct uninitialized_t {
        };

        static constexpr uninitialized_t uninitialized{};

        /**
         * @brief Constructs an uninitialized array of POD-like elements.
         * @param size The number of elements to allocate
         */
        explicit runtime_arr(const std::uint64_t size, uninitialized_t)
            requires jh::pod::pod_like<T> && std::is_same_v<Alloc, std::monostate> {
            size_ = size;
            T *ptr = static_cast<T *>(operator new[](sizeof(T) * size_));
            data_.reset(ptr);
        }

        /**
         * @brief Constructs a zero-initialized array using default allocation strategy.
         * @param size Number of elements
         */
        explicit runtime_arr(std::uint64_t size)
            requires (is_valid_allocator<Alloc>)
            : size_(size) {
            if constexpr (std::is_same_v<Alloc, std::monostate>) {
                T *ptr = new T[size_];
                data_.reset(ptr); // Use default_deleter
            } else {
                Alloc alloc{};
                T *ptr = alloc.allocate(size_);
                data_ = std::unique_ptr<T[], deleter_t>(
                    ptr,
                    [alloc, size](T *p) mutable {
                        alloc.deallocate(p, size);
                    }
                ); // Bind lambda
            }
        }

        /**
         * @brief Constructs using a provided allocator instance.
         * @param size Number of elements
         * @param alloc Allocator instance
         */
        explicit runtime_arr(std::uint64_t size, Alloc alloc)
            requires (!std::is_same_v<Alloc, std::monostate>)
            : size_(size) {
            T *ptr = alloc.allocate(size_);
            data_ = std::unique_ptr<T[], deleter_t>(
                ptr,
                [alloc, size](T *p) mutable {
                    alloc.deallocate(p, size);
                }
            );
        }

        /**
         * @brief Moves a std::vector<T> into runtime_arr (only when Alloc is monostate).
         * @param vec Rvalue reference to std::vector<T>
         */
        explicit runtime_arr(std::vector<T> &&vec)
            requires (std::is_same_v<Alloc, std::monostate>)
            : size_(vec.size()), data_(nullptr, default_deleter) {
            if (!vec.empty()) {
                T *ptr = new T[size_];
                std::move(vec.begin(), vec.end(), ptr);
                data_.reset(ptr);
            }
        }

        /**
         * @brief Constructs from input iterator range (only when Alloc is monostate).
         * @param first Beginning of input range
         * @param last End of input range
         */
        template<typename InputIt>
        runtime_arr(InputIt first, InputIt last)
            requires (std::is_same_v<Alloc, std::monostate> &&
                      jh::input_iterator<InputIt> &&
                      std::is_copy_constructible_v<T>)
            : data_(nullptr, default_deleter) {
            const auto dist = std::distance(first, last);
            if (dist < 0) throw std::invalid_argument("Invalid iterator range");
            size_ = static_cast<std::uint64_t>(dist);

            T *ptr = new T[size_];
            std::copy(first, last, ptr);
            data_.reset(ptr);
        }

        /// @brief Returns pointer to first element.
        T *begin() noexcept { return data_.get(); }

        /// @brief Returns pointer past the last element.
        T *end() noexcept { return data_.get() + size_; }

        /// @brief Returns const pointer to first element.
        const T *begin() const noexcept { return data_.get(); }

        /// @brief Returns const pointer past the last element.
        const T *end() const noexcept { return data_.get() + size_; }

        /// @brief Returns const pointer to first element.
        const T *cbegin() const noexcept { return data_.get(); }

        /// @brief Returns const pointer past the last element.
        const T *cend() const noexcept { return data_.get() + size_; }

        /**
         * @brief Access element at index (unchecked).
         * @param index Element index
         * @return Reference to element
         */
        T &operator[](std::uint64_t index) { return data_[index]; }
        const T &operator[](std::uint64_t index) const { return data_[index]; }

        /**
         * @brief Access element at index (with bounds checking).
         * @param index Element index
         * @return Reference to the element
         * @throws std::out_of_range if index is out of bounds
         */
        T &at(std::uint64_t index) {
            if (index >= size_) throw std::out_of_range("at(): index out of bounds");
            return data_[index];
        }

        /**
         * @brief Access element at index (with bounds checking).
         * @param index Element index
         * @return Constant reference to the element
         * @throws std::out_of_range if index is out of bounds
         */
        const T &at(std::uint64_t index) const {
            if (index >= size_) throw std::out_of_range("at(): index out of bounds");
            return data_[index];
        }


        /**
         * @brief Sets the value at given index using constructor arguments.
         * @param i Index to write to
         * @param args Arguments to construct T
         */
        template<typename... Args>
        void set(std::uint64_t i, Args &&... args) {
            if (i >= size_) throw std::out_of_range("set(): index out of bounds");
            data_[i] = T(std::forward<Args>(args)...);
        }


        /**
         * @brief Resets all elements to default state.
         *  - For POD-like types, uses `memset(0)` for max speed.
         *  - For trivially destructible but non-POD types, uses placement-new to reinitialize.
         *  - For complex types, falls back to `T{}` assignment (requiring destructor + constructor).
         * @note
         * This avoids undefined behavior by not using placement-new on non-trivial types.
         * Types must be default-constructible.
         */
        template<typename U = T>
            requires (std::is_default_constructible_v<U>)
        void reset_all() {
            if constexpr (pod::pod_like<T>) {
                std::memset(data_.get(), 0, size_ * sizeof(T));
            } else if constexpr (std::is_trivially_destructible_v<T>) {
                for (std::uint64_t i = 0; i < size_; ++i)
                    new(data_ + i) T{};
            } else {
                for (std::uint64_t i = 0; i < size_; ++i)
                    data_[i] = T{};
            }
        }

        /// @brief Returns number of elements in the array.
        [[nodiscard]] std::uint64_t size() const noexcept { return size_; }

        /// @brief Checks whether the array is empty.
        [[nodiscard]] bool empty() const noexcept { return size_ == 0; }

        /// @brief Raw pointer access.
        T *data() noexcept { return data_.get(); }

        /// @brief Raw const pointer access.
        const T *data() const noexcept { return data_.get(); }

        /**
         * @brief Move constructor.
         */
        runtime_arr(runtime_arr &&other) noexcept
            : size_(other.size_), data_(std::move(other.data_)) {
            other.size_ = 0;
            other.data_.reset();
        }

        /**
         * @brief Move assignment.
         */
        runtime_arr &operator=(runtime_arr &&other) noexcept {
            if (this != &other) {
                size_ = other.size_;
                data_ = std::move(other.data_);
                other.size_ = 0;
                other.data_.reset();
            }
            return *this;
        }

        /**
         * @brief Converts the buffer into a std::vector<T> via move.
         *
         * - For POD-like types (`jh::pod_like<T>`), performs a raw `memcpy` for maximum speed.
         * - For all other types, uses `std::make_move_iterator` for element-wise move construction.
         *
         * @note This operation consumes the array: resets size to 0 and releases internal memory.
         */
        explicit operator std::vector<T>() && {
            if constexpr (pod::pod_like<T>) {
                std::vector<T> vec(size_);
                std::memcpy(vec.data(), data_.get(), size_ * sizeof(T));
                size_ = 0;
                data_.reset();
                return vec;
            }
            std::vector<T> vec(std::make_move_iterator(begin()),
                               std::make_move_iterator(end()));
            size_ = 0;
            data_.reset();
            return vec;
        }

        /// @brief Deleted copy
        runtime_arr(const runtime_arr &) = delete;

        /// @brief Deleted copy assignment
        runtime_arr &operator=(const runtime_arr &) = delete;
    };

    /// @brief Iterator traits helper for runtime_arr<T>; enables custom traits resolution.
    template<typename T, typename Alloc>
    struct iterator<runtime_arr<T, Alloc> > {
        using type = typename runtime_arr<T, Alloc>::iterator;
    };

    /**
     * @brief A bit-packed specialization of `runtime_arr<bool>`, backed by raw `uint64_t[]`.
     * (By using an Allocator != std::monostate, the runtime_arr will not be bit-packed but a normal array of bool).
     *
     * @details
     * This version stores `bool` values in a compact form using 64-bit words.
     * Bit-access is provided via `bit_ref` proxy, and iterators support read/write traversal.
     *  - Internally allocated with `new uint64_t[]`, without allocator customization.
     *
     * @note
     * This structure is efficient for compact bitset operations and logical flags,
     * but is **not suitable for massive-scale bitmaps** (e.g., 1e9+ bits) due to linear scan cost and allocation method.
     * - Use `raw_data()` and `raw_word_count()` to access the underlying storage directly
     * if bit-level operations (e.g., hashing, export, SIMD scan) are needed.
     */
    template<>
    struct runtime_arr<bool> {
    private:
        std::uint64_t size_{};
        std::unique_ptr<std::uint64_t[]> storage_;
        static constexpr std::uint64_t BITS = 64;

        [[nodiscard]] std::uint64_t word_count() const noexcept {
            return (size_ + BITS - 1) / BITS;
        }

    public:
        /// @brief Internal reference proxy for single bit access.
        struct bit_ref {
        private:
            std::uint64_t &word_;
            std::uint64_t mask_;

        public:
            bit_ref(std::uint64_t &word, const std::uint64_t bit)
                : word_(word), mask_(1ULL << bit) {
            }

            bit_ref &operator=(const bool val) {
                if (val) word_ |= mask_;
                else word_ &= ~mask_;
                return *this;
            }

            explicit operator bool() const {
                return (word_ & mask_) != 0;
            }
        };

        /// @brief Iterator over individual bits in the bit-packed array.
        struct bit_iterator {
        private:
            const runtime_arr *parent_;
            std::uint64_t index_;

        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type = bool;
            using difference_type = std::ptrdiff_t;
            using reference = bit_ref;
            using pointer = void;

            bit_iterator(const runtime_arr *parent, const std::uint64_t index)
                : parent_(parent), index_(index) {
            }

            bit_ref operator*() const { return (*const_cast<runtime_arr *>(parent_))[index_]; }

            bit_iterator &operator++() {
                ++index_;
                return *this;
            }

            bit_iterator operator++(int) {
                const auto tmp = *this;
                ++*this;
                return tmp;
            }

            bit_iterator &operator--() {
                --index_;
                return *this;
            }

            bit_iterator operator--(int) {
                const auto tmp = *this;
                --*this;
                return tmp;
            }

            bit_iterator &operator+=(const difference_type n) {
                index_ += n;
                return *this;
            }

            bit_iterator &operator-=(const difference_type n) {
                index_ -= n;
                return *this;
            }

            bit_iterator operator+(const difference_type n) const { return {parent_, index_ + n}; }
            bit_iterator operator-(const difference_type n) const { return {parent_, index_ - n}; }

            difference_type operator-(const bit_iterator &other) const {
                return static_cast<std::ptrdiff_t>(index_ - other.index_); // NOLINT
            }

            bool operator==(const bit_iterator &other) const { return index_ == other.index_; }
            bool operator!=(const bit_iterator &other) const { return !(*this == other); }
            bool operator<(const bit_iterator &other) const { return index_ < other.index_; }
            bool operator<=(const bit_iterator &other) const { return index_ <= other.index_; }
            bool operator>(const bit_iterator &other) const { return index_ > other.index_; }
            bool operator>=(const bit_iterator &other) const { return index_ >= other.index_; }
        };

        using iterator = bit_iterator;
        using const_iterator = bit_iterator;

        /**
         * @brief Constructs a bit-packed array with all bits zero-initialized.
         * @param size Number of bits
         */
        explicit runtime_arr(const std::uint64_t size)
            : size_(size), storage_(std::make_unique<std::uint64_t[]>(word_count())) {
            std::memset(storage_.get(), 0, word_count() * sizeof(std::uint64_t));
        }

        /**
         * @brief Constructs from a std::vector<bool>.
         * @param vec Rvalue reference to std::vector<bool>
         */
        explicit runtime_arr(std::vector<bool> &&vec)
            : runtime_arr(vec.size()) {
            for (std::uint64_t i = 0; i < size_; ++i)
                set(i, vec[i]);
        }

        /**
         * @brief Constructs from an input iterator range of bools.
         * @param first Start of range
         * @param last End of range
         * @throws std::invalid_argument if range is invalid
         */
        template<typename InputIt>
        runtime_arr(InputIt first, InputIt last)
            requires (jh::input_iterator<InputIt>) {
            const auto dist = std::distance(first, last);
            if (dist < 0) throw std::invalid_argument("Invalid iterator range");
            size_ = static_cast<std::uint64_t>(dist);
            storage_ = std::make_unique<std::uint64_t[]>(word_count());
            std::memset(storage_.get(), 0, word_count() * sizeof(std::uint64_t));

            std::uint64_t i = 0;
            for (; first != last; ++first, ++i)
                set(i, static_cast<bool>(*first));
        }

        /// @brief Returns number of logical bits in the array.
        [[nodiscard]] std::uint64_t size() const noexcept { return size_; }

        /// @brief Returns whether the bit array is empty.
        [[nodiscard]] bool empty() const noexcept { return size_ == 0; }

        /// @brief Mutable access to the underlying word buffer.
        std::uint64_t *raw_data() noexcept { return storage_.get(); }

        /// @brief Const access to the underlying word buffer.
        [[nodiscard]] const std::uint64_t *raw_data() const noexcept { return storage_.get(); }

        /// @brief Returns number of `uint64_t` words used to store all bits.
        [[nodiscard]] std::uint64_t raw_word_count() const noexcept { return word_count(); }

        /**
         * @brief Access bit by index (read/write).
         * @param i Bit index
         * @return Reference proxy to the bit
         */
        bit_ref operator[](const std::uint64_t i) {
            return {storage_[i / BITS], i % BITS};
        }

        /**
         * @brief Access bit by index (read-only).
         * @param i Bit index
         * @return Boolean value of bit
         */
        bool operator[](const std::uint64_t i) const {
            return storage_[i / BITS] >> i % BITS & 1;
        }

        /**
         * @brief Sets or clears the bit at given index.
         * @param i Bit index
         * @param val Bit value to assign (true by default)
         * @throws std::out_of_range if i out of bounds
         */
        void set(const std::uint64_t i, const bool val = true) {
            if (i >= size_) throw std::out_of_range("set(): index out of bounds");
            if (val) storage_[i / BITS] |= 1ULL << i % BITS;
            else storage_[i / BITS] &= ~(1ULL << i % BITS);
        }

        /**
         * @brief Clears the bit at given index.
         * @param i Bit index
         * @throws std::out_of_range if i out of bounds
         */
        void unset(const std::uint64_t i) {
            if (i >= size_) throw std::out_of_range("unset(): index out of bounds");
            storage_[i / BITS] &= ~(1ULL << i % BITS);
        }

        /**
         * @brief Tests if the bit at index is set.
         * @param i Bit index
         * @return true if bit is 1, false if 0
         * @throws std::out_of_range if i out of bounds
         */
        [[nodiscard]] bool test(const std::uint64_t i) const {
            if (i >= size_) throw std::out_of_range("test(): index out of bounds");
            return storage_[i / BITS] >> i % BITS & 1;
        }

        /**
         * @brief Zeroes out all bits in the array.
         */
        [[gnu::used]] void reset_all() { // NOLINT not const
            std::memset(storage_.get(), 0, word_count() * sizeof(std::uint64_t));
        }

        /**
         * @brief Converts the bit array into std::vector<bool>.
         *        Elements are copied bit-by-bit.
         * @note This operation consumes the array (clears and resets storage).
         */
        explicit operator std::vector<bool>() && {
            std::vector<bool> vec(size_);
            for (std::uint64_t i = 0; i < size_; ++i)
                vec[i] = static_cast<bool>((*this)[i]);
            size_ = 0;
            storage_.reset();
            return vec;
        }

        /// @brief Mutable begin iterator over bits.
        iterator begin() noexcept { return {this, 0}; }

        /// @brief Mutable end iterator over bits.
        iterator end() noexcept { return {this, size_}; }

        /// @brief Const begin iterator over bits.
        [[nodiscard]] const_iterator begin() const noexcept { return {this, 0}; }

        /// @brief Const end iterator over bits.
        [[nodiscard]] const_iterator end() const noexcept { return {this, size_}; }

        /// @brief Const begin iterator over bits.
        [[nodiscard]] const_iterator cbegin() const noexcept { return begin(); }

        /// @brief Const end iterator over bits.
        [[nodiscard]] const_iterator cend() const noexcept { return end(); }

        /**
         * @brief forbid the general data() method from the template.
         *
         * @note Reinterpreting the bit-packed array as a bool* data buffer will not work as expected.
         */
        void data() const = delete;
    };

    template<>
    struct iterator<runtime_arr<bool> > {
        using type = runtime_arr<bool>::iterator;
    };
} // namespace jh
