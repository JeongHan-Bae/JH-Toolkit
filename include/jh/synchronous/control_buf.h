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
 * @file control_buf.h
 * @brief Fixed-capacity control block container for non-copyable, non-movable types.
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 *
 * <h3>Overview</h3>
 * <p>
 * <code>jh::sync::control_buf&lt;T&gt;</code> is a special-purpose, heap-backed container designed to store
 * control-related types (such as <code>std::mutex</code> or <code>std::atomic</code>) that are:
 * </p>
 *
 * <ul>
 *   <li>Default-constructible</li>
 *   <li>Non-copyable and non-movable</li>
 *   <li>Unsuitable for use with <code>std::vector</code>, <code>std::deque</code>, etc.</li>
 * </ul>
 *
 * <p>
 * Unlike STL containers, this type <b>intentionally disables element-wise relocation</b> such as
 * move or copy during reallocation. This is critical for storing control-type objects whose
 * addresses must remain stable during their lifetime &mdash; for instance, synchronization primitives
 * like <code>std::mutex</code>, <code>std::atomic</code>, or file descriptors.
 * </p>
 *
 * <p>
 * This constraint is formalized by rejecting types that satisfy
 * <code>jh::concepts::is_contiguous_reallocable</code>, which checks whether a type supports
 * move- or copy-based relocation under contiguous growth strategies.
 * </p>
 *
 * <p>
 * Internally, <code>control_buf</code> uses a <b>block-allocated growth model</b>, where memory is
 * allocated in fixed-size blocks. These blocks are never relocated, and each element is constructed
 * in-place, preserving pointer stability.
 * </p>
 *
 * <h3>Core Features</h3>
 * <ul>
 *   <li>Block-based allocation: memory grows by allocating fixed-sized blocks on demand.</li>
 *   <li><b>Block size:</b> each block contains <code>JH_FIXED_VEC_BLOCK_SIZE</code> elements
 *       (default: 64). This can be configured via a preprocessor macro.</li>
 *   <li>Each element is <b>default-constructed in-place</b>; no relocation, no reordering.</li>
 *   <li><b>Strict type constraints</b>: relocation-disabled types only (e.g. <code>std::mutex</code>).</li>
 *   <li>Dynamic growth supported at block level (like <code>std::deque</code>), but never at element level.</li>
 *   <li>No iteration interfaces (e.g., range-for); access is by index only.</li>
 *   <li>Allocator-aware with safe RAII block lifetime management.</li>
 * </ul>
 *
 * <h3>Key Properties</h3>
 * <ul>
 *   <li>Only supports <code>emplace_back()</code> to append default-constructed elements.</li>
 *   <li>Does not support resizing via insert/erase; only <code>resize()</code> or <code>clear()</code>.</li>
 *   <li>Provides indexed access but not STL-compatible iteration or range-for loops.</li>
 *   <li>Respects allocator customization through standard <code>std::allocator_traits</code>.</li>
 * </ul>
 *
 * @note <b>This is not a general-purpose container.</b> It should only be used for types explicitly disallowed
 * from standard containers due to allocator or relocation requirements (e.g., <code>std::mutex</code>, <code>std::atomic</code>).
 *
 * @see jh::concepts::is_contiguous_reallocable
 *
 * @version <pre>1.4.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include <memory>
#include <vector>
#include <type_traits>
#include "jh/conceptual/container_traits.h"

#ifndef JH_FIXED_VEC_BLOCK_SIZE
#define JH_FIXED_VEC_BLOCK_SIZE 64
#endif


namespace jh::sync {
    /**
     * @brief Fixed-capacity, block-allocated container for control-only types (e.g., mutexes, atomics).
     * @tparam T Type to store (must be default-constructible and non-reallocable).
     * @tparam Alloc Optional allocator type (defaults to <code>std::allocator&lt;T&gt;</code>).
     *
     * <h4>Design Constraints</h4>
     * <ul>
     *   <li><b>T must be default-constructible.</b></li>
     *   <li><b>T must NOT be copyable or movable.</b></li>
     *   <li><code>control_buf</code> is not intended for data storage or iteration.</li>
     * </ul>
     */
    template<typename T, typename Alloc = std::allocator<T>> requires
    (std::is_default_constructible_v<T>
     && !jh::concepts::is_contiguous_reallocable<T>)
    class control_buf final {
    public:
        /// @brief Number of elements per allocation block.
        static constexpr std::size_t BLOCK_SIZE = JH_FIXED_VEC_BLOCK_SIZE;

        /// @brief Type of allocator used for element construction.
        using allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<T>;

        /// @brief Element type alias.
        using value_type [[maybe_unused]] = T;

    private:
        using traits = std::allocator_traits<allocator_type>;

        /// @brief Custom deleter that destroys and deallocates entire blocks.
        struct block_deleter final {
            mutable allocator_type alloc;

            void operator()(T *p) const noexcept {
                if (p) {
                    for (size_t i = 0; i < BLOCK_SIZE; ++i)
                        traits::destroy(alloc, p + i);
                    traits::deallocate(alloc, p, BLOCK_SIZE);
                }
            }
        };

        /// @brief Unique pointer to a block of T with bound deleter.
        using block_ptr = std::unique_ptr<T[], block_deleter>;

        allocator_type alloc;
        std::vector<block_ptr> blocks;
        std::size_t total = 0;

    public:

        /**
         * @brief Constructs an empty control buffer.
         *
         * @details
         * No memory is allocated and no elements are constructed.
         * The container starts with zero size and zero blocks.
         */
        control_buf() = default;

        /**
         * @brief Constructs an empty control buffer with a custom allocator.
         *
         * @param alloc Allocator instance used for all future block allocations.
         *
         * @details
         * The allocator is stored internally and copied into each block deleter.
         * No memory allocation occurs at construction time.
         */
        explicit control_buf(const Alloc &alloc) : alloc(alloc) {}

        /**
         * @brief Constructs a control buffer with exactly <code>n</code> default-constructed elements.
         *
         * @param n Number of elements to construct.
         *
         * @details
         * Elements are constructed incrementally using the block-based growth strategy.
         * Each element is default-constructed in-place and never relocated.
         */
        [[maybe_unused]] explicit control_buf(std::size_t n)
                : total(0) {
            construct_n(n);
        }

        /**
         * @brief Constructs a control buffer with <code>n</code> elements using a custom allocator.
         *
         * @param n Number of elements to construct.
         * @param alloc Allocator used for block allocation and element construction.
         */
        [[maybe_unused]] control_buf(std::size_t n, const Alloc &alloc)
                : alloc(alloc), total(0) {
            construct_n(n);
        }

        /**
         * @brief Copy-constructs the buffer topology from another instance.
         *
         * @param other Source buffer.
         *
         * @details
         * This is a <b>topological copy</b>:
         * <ul>
         *   <li>The resulting buffer has the same <code>size()</code> as @p other.</li>
         *   <li>Elements are <b>default-constructed</b>, not copied.</li>
         *   <li>No element-wise copy or move is performed.</li>
         * </ul>
         *
         * This behavior is intentional and required for control-type objects
         * whose state must not be duplicated.
         */
        control_buf(const control_buf &other)
                : alloc(other.alloc), total(0) {
            construct_n(other.total);
        }

        /**
         * @brief Copy-assigns the buffer topology from another instance.
         *
         * @param other Source buffer.
         * @return Reference to this buffer.
         *
         * @details
         * Equivalent to:
         * <ol>
         *   <li>Destroying all existing blocks.</li>
         *   <li>Copying the allocator.</li>
         *   <li>Default-constructing <code>other.size()</code> elements.</li>
         * </ol>
         *
         * Element contents are never copied.
         */
        control_buf &operator=(const control_buf &other) {
            if (this != &other) {
                clear();
                alloc = other.alloc;
                construct_n(other.total);
            }
            return *this;
        }

        /**
         * @brief Move-constructs a control buffer from another instance.
         *
         * @param other Source buffer to move from.
         *
         * @details
         * Transfers ownership of all internal blocks and allocator state from @p other
         * to the newly constructed buffer.
         * <ul>
         *   <li>No elements are copied, moved, or relocated.</li>
         *   <li>Only block ownership and allocator state are transferred.</li>
         *   <li>Block-level address stability is preserved.</li>
         * </ul>
         *
         * @note
         * After the move, @p other is left in a <b>valid but unspecified state</b>,
         * consistent with standard C++ container move semantics.
         * The moved-from object is safe to destroy.
         * If reuse is required, the caller must explicitly call <code>clear()</code>.
         */
        control_buf(control_buf &&other) noexcept
                : alloc(std::move(other.alloc)),
                  blocks(std::move(other.blocks)),
                  total(other.total) {
            other.total = 0;
        }

        /**
         * @brief Move-assigns a control buffer from another buffer.
         *
         * @param other The source buffer to move from.
         * @return Reference to this buffer after assignment.
         *
         * @details
         * Clears current contents and transfers ownership of all internal blocks and allocator state
         * from @p other to this buffer.
         * <ul>
         *   <li>All previously allocated memory is destroyed before the move.</li>
         *   <li>Element memory is not copied; blocks are moved as raw pointers.</li>
         *   <li>The moved-from buffer is left in a valid but unspecified state.</li>
         * </ul>
         *
         * @note As with move construction, reusing the moved-from buffer requires an
         *       explicit call to <code>clear()</code>.
         */
        control_buf &operator=(control_buf &&other) noexcept {
            if (this != &other) {
                clear();
                alloc = std::move(other.alloc);
                blocks = std::move(other.blocks);
                total = other.total;
                other.total = 0;
            }
            return *this;
        }

        /**
         * @brief Returns the current number of constructed elements.
         *
         * @return Number of elements currently held in the buffer.
         *
         * @note This reflects the logical size (i.e., how many elements were constructed via emplace or resize),
         *       not the total memory capacity.
         */
        [[nodiscard]] std::size_t size() const noexcept { return total; }

        /**
         * @brief Returns the total number of elements that can be stored without further allocation.
         *
         * @return The maximum number of elements that can be emplaced before triggering a new block allocation.
         *
         * @note Capacity is managed in blocks of fixed size (@c BLOCK_SIZE) and is calculated as:
         *       <code>blocks.capacity() * BLOCK_SIZE</code>.
         */
        [[nodiscard]] std::size_t capacity() const noexcept {
            return blocks.capacity() * BLOCK_SIZE;
        }

        /**
         * @brief Appends a default-constructed element to the end of the buffer.
         *
         * @return Reference to the newly constructed element.
         *
         * @details
         * This method constructs a new element in-place at the logical end of the buffer.
         * If the current block is full, a new block is allocated and all its elements are default-constructed.
         * <ul>
         *   <li>Elements are constructed using <code>allocator_traits::construct</code>.</li>
         *   <li>New memory is allocated in chunks of <code>BLOCK_SIZE</code> elements.</li>
         *   <li>Each block is allocated only once and never reallocated or moved.</li>
         *   <li>Element addresses remain stable for the lifetime of the container.</li>
         * </ul>
         *
         * This function's interface is intentionally consistent with
         * <code>std::vector&lt;T&gt;::emplace_back()</code> and
         * <code>std::deque&lt;T&gt;::emplace_back()</code> (default-construction form),
         * even though this container does not support in-place argument forwarding.
         * <ul>
         *   <li>This design choice lowers the learning curve for users familiar with STL containers.</li>
         *   <li>It enables generic code to use <code>emplace_back()</code> in the same way across both
         *       control and data containers.</li>
         *   <li>It allows working with types like <code>std::mutex</code> and <code>std::atomic</code>,
         *       which cannot be stored in <code>std::vector</code> due to relocation or copying constraints,
         *       but still benefit from a consistent push-back interface.</li>
         * </ul>
         *
         * @note This function always performs default-construction only.
         *       Argument forwarding is not supported.
         */
        T &emplace_back() {
            std::size_t offset = total % BLOCK_SIZE;

            if (offset == 0) {
                T *raw = traits::allocate(alloc, BLOCK_SIZE);
                block_ptr blk(raw, block_deleter{alloc});
                for (size_t i = 0; i < BLOCK_SIZE; ++i)
                    traits::construct(alloc, raw + i);
                blocks.emplace_back(std::move(blk));
            }

            T &obj = (*this)[total];
            total++;
            return obj;
        }

        /**
         * @brief Provides unchecked access to the element at index <code>i</code>.
         *
         * @param i Zero-based index of the element to access.
         * @return Reference to the element at the specified index.
         *
         * @details
         * This function does not perform bounds checking.
         * It assumes the index is valid (i.e., less than <code>size()</code>).
         * <ul>
         *   <li>Equivalent in usage to <code>std::vector::operator[]</code>.</li>
         *   <li>Allows direct indexed access for integration with generic code.</li>
         *   <li>Performs internal block lookup via division and modulus on <code>BLOCK_SIZE</code>.</li>
         * </ul>
         *
         * @note Use <code>at(i)</code> if you need bounds checking.
         */
        T &operator[](std::size_t i) noexcept {
            return blocks[i / BLOCK_SIZE].get()[i % BLOCK_SIZE];
        }

        /**
         * @brief Provides unchecked, read-only access to the element at index <code>i</code>.
         *
         * @param i Zero-based index of the element to access.
         * @return Const reference to the element at the specified index.
         *
         * @details
         * This overload offers const access to the same element as the non-const version.
         * <ul>
         *   <li>No bounds checking is performed.</li>
         *   <li>Safe to call as long as <code>i</code> is within <tt>[0, size())</tt>.</li>
         *   <li>Useful in const contexts and read-only algorithms.</li>
         * </ul>
         *
         * @note Use <code>at(i)</code> for bounds-checked access.
         */
        const T &operator[](std::size_t i) const noexcept {
            return blocks[i / BLOCK_SIZE].get()[i % BLOCK_SIZE];
        }

        /**
         * @brief Returns a reference to the element at index <code>i</code>, with bounds checking.
         *
         * @param i Zero-based index of the element to access.
         * @return Reference to the element at index <code>i</code>.
         * @throws std::out_of_range If <code>i</code> is greater than or equal to <code>size()</code>.
         *
         * @details
         * This function performs runtime bounds checking before accessing the element.
         * If the index is invalid, an exception is thrown.
         * <ul>
         *   <li>Semantically equivalent to <code>std::vector::at()</code>.</li>
         *   <li>Access is internally delegated to <code>operator[]</code> after validation.</li>
         *   <li>Provides a safer alternative to unchecked access via <code>operator[]</code>.</li>
         * </ul>
         *
         * @note Prefer this method over <code>operator[]</code> when index validity is uncertain.
         */
        T &at(std::size_t i) {
            if (i >= total)
                throw std::out_of_range("jh::sync::control_buf::at(): index out of bounds");
            return operator[](i);
        }

        /**
         * @brief Returns a const reference to the element at index <code>i</code>, with bounds checking.
         *
         * @param i Zero-based index of the element to access.
         * @return Const reference to the element at index <code>i</code>.
         * @throws std::out_of_range If <code>i</code> is greater than or equal to <code>size()</code>.
         *
         * @details
         * This overload provides read-only, bounds-checked access to an element.
         * <ul>
         *   <li>Returns the same result as the non-const version but guarantees const access.</li>
         *   <li>Delegates to <code>operator[]</code> after index validation.</li>
         * </ul>
         *
         * @note Use this method when accessing elements in a const context with safety guarantees.
         */
        [[nodiscard]] const T &at(std::size_t i) const {
            if (i >= total)
                throw std::out_of_range("jh::sync::control_buf::at(): index out of bounds");
            return operator[](i);
        }

        /**
         * @brief Reserves space in the internal block index for at least <code>n</code> elements.
         *
         * @param n Desired minimum number of elements the buffer should be able to hold without reallocation.
         *
         * @details
         * This function performs a preallocation on the internal <code>std::vector&lt;block_ptr&gt;</code>
         * that manages block pointers. It does not preallocate the actual element blocks themselves.
         * <ul>
         *   <li>Internally reserves <code>ceil(n / BLOCK_SIZE)</code> block pointer slots.</li>
         *   <li>Does not allocate or construct any elements or blocks.</li>
         *   <li>Useful mostly for reducing index vector reallocations during bulk appends.</li>
         *   <li>Cannot avoid per-block allocation due to the semantics of <code>control_buf</code>.</li>
         * </ul>
         *
         * @note The usefulness of <code>reserve()</code> is limited because control-type elements (e.g. mutexes)
         *       cannot be preconstructed or relocated. This function primarily exists for interface consistency
         *       and intent declaration.
         */
        void reserve(std::size_t n)
        noexcept(noexcept(std::declval<std::vector<block_ptr> &>().reserve(n))) {
            size_t need_blocks = (n + BLOCK_SIZE - 1) / BLOCK_SIZE;
            blocks.reserve(need_blocks);
        }

        /**
         * @brief Destroys all constructed elements and deallocates all blocks.
         *
         * @details
         * This function releases all memory held by the container and resets its size to zero.
         * It also clears the internal block index vector.
         * <ul>
         *   <li>All constructed elements are destroyed via allocator-aware destruction.</li>
         *   <li>All blocks are properly deallocated via their bound deleters.</li>
         *   <li>The internal block pointer vector is also cleared.</li>
         * </ul>
         *
         * @note After calling <code>clear()</code>, the container is empty and has no allocated blocks.
         */
        void clear()
        noexcept(noexcept(std::declval<std::vector<block_ptr> &>().clear())) {
            blocks.clear();
            total = 0;
        }

        /**
         * @brief Attempts to reduce the capacity of the internal block index vector to fit its size.
         *
         * @details
         * This function operates only on the internal <code>std::vector&lt;block_ptr&gt;</code> that stores
         * pointers to element blocks. It does not modify or deallocate any element storage.
         * <ul>
         *   <li>Does not affect the actual element blocks or constructed elements.</li>
         *   <li>Has no observable effect on memory usage unless the vector itself has excess capacity.</li>
         *   <li>Provided for interface symmetry with standard containers like <code>std::vector</code>.</li>
         * </ul>
         *
         * @note This function behaves similarly to <code>std::vector::shrink_to_fit()</code>,
         *       which in C++20 and beyond also has no mandated effect.
         */
        void shrink_to_fit()
        noexcept {
            blocks.shrink_to_fit();
        }

        /**
         * @brief Resizes the container to contain exactly <code>n</code> elements.
         *
         * @param n The desired number of elements after resizing.
         *
         * @details
         * This function adjusts the logical size of the container to <code>n</code>.
         * <ul>
         *   <li>If <code>n &gt; size()</code>, new elements are added via <code>emplace_back()</code>,
         *       which default-constructs new elements in-place.</li>
         *   <li>If <code>n &lt; size()</code>, blocks beyond the necessary range are destroyed and released.</li>
         *   <li>If the new size does not align exactly to <code>BLOCK_SIZE</code>,
         *       the tail block may contain partially used space that remains physically allocated
         *       but is treated as logically unused.</li>
         * </ul>
         *
         * These remaining elements are not destroyed and retain their previous state.
         * They are considered <em>logically uninitialized</em>, and the next call to <code>emplace_back()</code>
         * will continue from where <code>resize()</code> left off &mdash; it does not reinitialize the tail.
         *
         * @note This behavior is safe and expected for most control types (e.g., <code>std::atomic</code>),
         *       where default-construction does not guarantee value initialization.
         *       However, if you rely on a default-constructed value having a known state (e.g., user-defined types),
         *       you must ensure to explicitly reset such objects after <code>resize()</code>.
         *
         * @warning Do not assume that elements beyond the new logical size are reset or zeroed.
         *          Treat them as undefined and reinitialize as needed.
         */
        void resize(std::size_t n) {
            if (n >= total) {
                construct_n(n);
            } else {
                std::size_t need_blocks = (n + BLOCK_SIZE - 1) / BLOCK_SIZE;
                blocks.resize(need_blocks);
                total = n;
            }
        }

    private:
        /**
         * @brief Constructs elements up to a total count of <code>n</code>.
         *
         * @param n Target total number of constructed elements.
         *
         * @details
         * This utility function fills the container up to <code>n</code> elements,
         * using repeated calls to <code>emplace_back()</code>. It preserves
         * existing elements and only constructs new ones as needed.
         * <ul>
         *   <li>Calls <code>reserve(n)</code> to preallocate index space.</li>
         *   <li>Does not shrink or remove any existing elements.</li>
         *   <li>Used internally by constructors and <code>resize()</code>.</li>
         * </ul>
         */
        void construct_n(std::size_t n) {
            reserve(n);
            for (; total < n; ++total) {
                emplace_back();
            }
        }
    };

} // namespace jh::sync
