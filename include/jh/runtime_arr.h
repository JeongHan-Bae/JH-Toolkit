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
 * @brief A non-resizable, strongly-semantic runtime array for fixed-length buffer logic.
 *
 * @details
 * `runtime_arr<T, Alloc>` is a move-only, fixed-size container designed for scenarios
 * where **buffer size is known at runtime** but **mutating size (e.g. via `resize()`) is forbidden**.
 *
 * It aims to provide semantic clarity, avoid misuse (such as accidental dynamic growth),
 * and act as a safer alternative to raw heap arrays when using POD or trivially constructible types.
 *
 * ## Key Properties
 * - The buffer is allocated with a runtime-determined size, fixed during lifetime.
 * - No `resize()`, `push_back()`, or any size-altering operations are available.
 * - Suitable for **performance-aware**, **memory-stable**, and **temporary data** usage patterns.
 * - Fully compatible with STL-style iteration and `std::ranges::view_interface`.
 *
 * ## Comparison vs `std::vector`
 * | Feature                        | `std::vector<T>`           | `runtime_arr<T>`                  |
 * |--------------------------------|----------------------------|-----------------------------------|
 * | Resizability                   | ✅ (`resize`, `push_back`) | ❌ (fixed-size only)              |
 * | Initialization customization   | Partial (`reserve`, etc.)  | ✅ (`uninitialized`, `reset_all`) |
 * | POD-optimized zeroing          | ❌                         | ✅ (`reset_all()` for `T = POD`)  |
 * | Allocator control              | ✅                         | ✅ (optional custom allocator)    |
 * | Access safety                  | `at()`, bounds-check opt   | ✅ (`at()` + `operator[]`)        |
 * | Iterator & range support       | ✅                         | ✅ (`view_interface`)             |
 *
 * ## Design Motivation
 * - Better **communicates intent** for fixed-capacity buffers.
 * - Avoids accidental reallocation or memory growth in inner loops.
 * - Offers reset capability without reallocating, improving memory reuse patterns.
 *
 * ## Caveats
 * - `runtime_arr` **is not faster** than `std::vector` in most compiler-optimized paths (especially with trivial types).
 * - Intended more for **semantic clarity** and **compile-time safety**, not micro-optimization.
 * - No implicit copy/clone support: move-only by design.
 *
 * ## Specialization Note
 * - `runtime_arr<bool, typed::monostate>` provides bit-packing using `uint64_t[]`, and exposes methods like:
 *   - `set(i)`, `unset(i)`, `test(i)`
 *   - `bit_ref`, `raw_data()`, and `raw_word_count()`
 *
 * ## Typical Use Cases
 * - Intermediate fixed-size buffers in sort, radix pipelines, or scratch space.
 * - Preventing accidental container growth in memory-critical paths.
 * - Enforcing allocation discipline in systems that rely on buffer contracts.
 * - Safer replacement for raw `T* arr = new T[n]` heap buffers.
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
#include "pods/pod_like.h"
#include "utils/typed.h"

namespace jh {
    /**
     * @brief A move-only, fixed-size array for runtime-determined, non-resizable storage.
     *
     * @tparam T      Element type
     * @tparam Alloc  Optional allocator (defaults to `typed::monostate`)
     *
     *  Short for "runtime-sized array" – move-only, heap-allocated, fixed-length.
     *
     * `runtime_arr<T, Alloc>` is a strongly semantic array structure meant to replace raw heap buffers
     * (like `new T[n]`) in performance-sensitive or correctness-critical code.
     *
     * Unlike `std::vector`, it forbids resizing, shrinking, or growth semantics.
     * The goal is not to outperform STL containers, but to **clarify intent**, **limit misuse**, and **make memory behavior predictable**.
     *
     * ### Core Features
     * - Move-only: no accidental copies or aliasing
     * - Optional zero/uninitialized construction
     * - Compatible with STL/ranges interfaces (via `view_interface`)
     * - `reset_all()` for fast re-zeroing of POD/trivial types
     * - Allocator customization (if needed)
     * - Provides `as_span()` helper for safe and ergonomic interop with `std::span<T>`
     *
     * ### When to Use
     * - In-place buffers for radix sort, DP tables, etc.
     * - Safer alternative to raw `T*` when lifetime and size are known
     * - Where fixed-capacity is a **hard constraint**, not a performance hint
     *
     * ### When *Not* to Use
     * - You need `push_back`, `resize`, or dynamic append
     * - You require polymorphic container behavior (e.g., erase/insert/iterator invalidation)
     *
     * ### Interop Notes
     * - `runtime_arr<T>` is contiguous and span-compatible: use `std::span(arr.data(), arr.size())`
     * - Also compatible with range-based for-loops, `std::ranges::views`, and STL algorithms
     *
     * ### Notes
     * - Use `reset_all()` instead of `clear()`
     * - Use `.data()` if interop with raw APIs is needed
     * - Use `runtime_arr<T>::uninitialized` to skip default-construction (if safe for T)
     */
    template<typename T, typename Alloc = typed::monostate>
    struct runtime_arr : std::ranges::view_interface<runtime_arr<T, Alloc> > {
    private:
        std::uint64_t size_{0};

        using deleter_t = std::function<void(T *)>;
        std::unique_ptr<T[], deleter_t> data_{nullptr, make_deleter()}; // Tie deleter to allocator

        static void default_deleter(T *p) { delete[] p; } // NOLINT

        static deleter_t make_deleter() {
            if constexpr (typed::monostate_t<Alloc>) {
                return default_deleter;
            } else {
                return nullptr; // No-op deleter, it will be bound to lambda
            }
        }

        template<typename A>
        static constexpr bool is_valid_allocator =
                typed::monostate_t<A> || requires(A a, std::uint64_t n)
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
            requires jh::pod::pod_like<T> && typed::monostate_t<Alloc> {
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
            if constexpr (typed::monostate_t<Alloc>) {
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
            requires (!typed::monostate_t<Alloc>)
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
            requires (typed::monostate_t<Alloc>)
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
            requires (typed::monostate_t<Alloc> &&
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
         *
         * @note This operation is only meaningful for types that are default-constructible.
         *       For types without a valid `T()`, reset has no semantic definition and is therefore disabled.
         *
         *       The behavior varies depending on the type trait of `T`:
         *       - For POD-like types: uses `memset` for maximal efficiency.
         *       - For trivially destructible types: re-initializes via placement-new.
         *       - Otherwise: assigns `T{}` to each element (requires destructor and constructor).
         *
         * @warning This function is disabled at compile-time for types that do not satisfy `is_default_constructible<T>`.
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

        [[nodiscard]] std::span<T> as_span() noexcept { return {data(), size()}; }

        [[nodiscard]] std::span<const T> as_span() const noexcept { return {data(), size()}; }
    };

    /// @brief Iterator traits helper for runtime_arr<T>; enables custom traits resolution.
    template<typename T, typename Alloc>
    struct iterator<runtime_arr<T, Alloc> > {
        using type = typename runtime_arr<T, Alloc>::iterator;
    };

    /**
     * @brief A compact, fixed-size bit array specialized for `bool`, with bit-level access.
     *
     * This specialization stores bits in 64-bit blocks (`uint64_t[]`) and supports read/write access via `bit_ref`.
     *
     * Compared to `std::vector<bool>`:
     * - More explicit and less overloaded
     * - Does not rely on STL vector implementation tricks
     * - Exposes raw storage (`raw_data()`, `raw_word_count()`) for custom bit ops
     *
     * ### Suitable For:
     * - Compact masks and flags
     * - Custom bitmap-based algorithms
     * - Bit-parallelism or bit-serialization routines
     *
     * ### Key APIs:
     * - `set(i)`, `unset(i)`, `test(i)`
     * - `reset_all()` to zero entire array
     * - STL-style iteration over bits (via `bit_iterator`)
     *
     * ### Notable Limitations:
     * - Not allocator-customizable
     * - Not intended for massive-scale bitmap (e.g. > 1e9 bits)
     * - No `std::span/view` compatibility, does NOT inherit the std::ranges::view_interface like other templates.
     *
     * ⚠️ Do NOT reinterpret the underlying storage as `bool*`.
     * Use `bit_ref` or safe iterators instead.
     * Only the default <bool> case is affected, those with Allocators are normal.
     *
     * @note Similar to `std::bitset` in storage layout (contiguous bits), but supports runtime-sized arrays.
     *       Unlike `std::bitset`, access is via mapped proxy references (`bit_ref`), which may involve
     *       minor indirection overhead (index to word/bit mapping).
     *       Compared to `std::vector<bool>`, this class offers more explicit behavior and safer access
     *       patterns, with predictable memory structure (`uint64_t[]`) and no STL-specific optimizations.
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

        [[nodiscard]] std::span<bool> as_span() = delete;

        [[nodiscard]] std::span<const bool> as_span() const = delete;
    };

    template<>
    struct iterator<runtime_arr<bool> > {
        using type = runtime_arr<bool>::iterator;
    };
} // namespace jh
