// ⚠️ [EXPERIMENTAL] This module is internal and not part of the public API.
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
 * @file pod_stack.h
 * @author JeongHan-Bae <mastropseudo@gmail.com>
 * @brief A high-performance, LIFO-only stack for POD (Plain Old Data) types.
 *
 * @details
 * `pod_stack<T, BLOCK_SIZE>` is a top-optimized, memory-stable stack designed for algorithms
 * that operate on trivially structured data, such as simulation frames, DFS nodes, or runtime scopes.
 *
 * Internally, it manages memory as chained blocks of raw aligned storage, allowing for:
 * - Placement-new construction
 * - Zero per-element destruction
 * - Fast, cache-friendly access to the stack top
 *
 * The design is tailored specifically for:
 * - POD-only types (no virtuals, no destructors, no hidden state)
 * - Reuse in loop-heavy or stack-heavy algorithms
 * - Minimal control surface (no iterators, no introspection)
 *
 * ## Type Requirements
 * - `T` must satisfy `jh::pod::pod_like<T>` (see `pod.h`)
 * - `BLOCK_SIZE` must be ≥ 256 and a power of two
 *
 * ## A Fun Fact 🐲
 * `pod_stack` is like **椒图 (Jiaotu, or 초도 *Chodo* in our Joseonjok pronunciation)**,
 * a mythical guardian beast known for guarding gates and **controlling entry and exit**.
 *
 * Just like Jiaotu, `pod_stack` **only cares about the top**:
 * - You can `push`, `pop`, `top`, and `clear`, but nothing else.
 * - You can't peek inside, iterate, or inspect history.
 * - It is a **gatekeeper**, optimized for LIFO flows with tight memory control.
 *
 * ## Key Use Cases
 * - DFS / BFS simulation
 * - Recursion flattening
 * - Context stack in interpreters or rule engines
 * - Per-frame algorithmic state
 *
 * This stack does not aim to be general-purpose. It aims to be fast.
 *
 * ## Version
 * @version 1.3.x
 * @date 2025
 */


#pragma once

#include <cstdint>
#include <cassert>
#include <optional>
#include <type_traits> // NOLINT
#include "jh/pods/pod_like.h"

namespace jh {
    /**
     * @brief Stack structure for POD-only types, with block-based reuse and zero abstraction overhead.
     *
     * @tparam T A POD-like type (trivial, standard layout, no destructor or virtuals)
     * @tparam BLOCK_SIZE Number of elements per block (≥256, power of 2)
     *
     * This structure is optimized for stack-style memory access and reuse.
     * It guarantees:
     * - O(1) push/pop/top
     * - Manual control over memory blocks
     * - No hidden cost from construction or destruction
     *
     * Not suitable for non-trivial types or dynamic polymorphism.
     */
    template<typename T, std::uint32_t BLOCK_SIZE = 2048>
        requires (pod::pod_like<T> && (BLOCK_SIZE >= 256 && (BLOCK_SIZE & BLOCK_SIZE - 1) == 0))
    struct pod_stack {
    private:
        struct node {
            alignas(T) std::byte raw_data_[sizeof(T) * BLOCK_SIZE]{};
            T *data_ = reinterpret_cast<T *>(raw_data_);
            std::uint32_t size_ = 0;
            node *prev = nullptr;
            std::unique_ptr<node> next = nullptr;

            [[nodiscard]] bool full() const { return size_ == BLOCK_SIZE; }
            [[nodiscard]] bool empty() const { return size_ == 0; }
        };


        std::unique_ptr<node> root_ = nullptr;
        node *head_ = nullptr;
        std::uint64_t size_ = 0;

        /**
         * @brief Ensures the current block has space for at least one more element.
         *
         * If the current block is full, a new one is allocated and linked.
         * This is called internally by all push/emplace routines.
         */
        void _ensure_capacity() {
            if (!root_) [[unlikely]] {
                root_ = std::make_unique<node>();
                head_ = root_.get();
            } else if (head_->full()) [[unlikely]] {
                if (!head_->next) {
                    auto new_node = std::make_unique<node>();
                    new_node->prev = head_;
                    head_->next = std::move(new_node);
                }
                head_ = head_->next.get();
            }
        }

    public:
        /**
         * @brief Default constructor. Creates an empty `pod_stack`.
         *
         * @note No memory is allocated until the first `push()` or `emplace()` call.
         */
        pod_stack() = default;

        /**
         * @brief Checks whether the stack is empty.
         * @return `true` if the stack has no elements, `false` otherwise.
         */
        [[nodiscard]] bool empty() const { return size_ == 0; }

        /**
         * @brief Returns the number of elements currently stored in the stack.
         * @return The current size (element-count).
         */
        [[nodiscard]] std::uint64_t size() const { return size_; }

        /**
        * @brief Constructs and pushes a new element to the top of the stack using forwarded arguments.
        *
        * @tparam Args Constructor arguments for `T`
        * @param args Parameters forwarded to `T`'s constructor
        *
        * Internally use placement-new. No heap allocation occurs per element.
        */
        template<typename... Args>
        void push(Args &&... args) {
            _ensure_capacity();
            new(head_->data_ + head_->size_) T(std::forward<Args>(args)...); // placement new
            ++head_->size_;
            ++size_;
        }

        /**
        * @brief Pushes an existing object onto the stack via move.
        *
        * @param obj A fully constructed `T` instance, moved into the stack.
        *
        * Equivalent to `push(std::move(obj))`. The object is placement-moved into internal storage.
        */
        void emplace(T &&obj) {
            _ensure_capacity();
            new(head_->data_ + head_->size_) T(std::move(obj)); // placement new
            ++head_->size_;
            ++size_;
        }


        /**
         * @brief Returns a reference to the element at the top of the stack.
         *
         * @return `T&` Reference to the top element
         * @note Caller must ensure the stack is not empty (`!empty()`).
         *
         * `Top` is stable and can be safely destructured.
         */
        [[nodiscard]] T &top() {
            assert(!empty());
            return head_->data_[head_->size_ - 1];
        }


        /**
         * @brief Removes the top element from the stack without destroying it.
         *
         * No destructor is called. Only internal counters are updated.
         * The memory will be reused by the next `push()`.
         */
        void pop() {
            assert(!empty());
            --head_->size_;
            --size_;
            if (head_->empty() && head_->prev) {
                head_ = head_->prev;
            }
        }

        /**
         * @brief Removes the top element and potentially reclaims the empty memory block.
         *
         * This call will release the current node block if it becomes empty,
         * and it's not the root block.
         *
         * Use this when stack size fluctuates heavily and block reuse is not desired.
         */
        void clean_pop() {
            assert(size_ > 0);
            assert(head_ && head_->size_ > 0);

            --head_->size_;
            --size_;

            if (head_->size_ == 0 && head_->prev) {
                node *prev = head_->prev;
                prev->next.reset();
                head_ = prev;
            }
        }

        /**
         * @brief Clears the stack and resets it to a single root block.
         *
         * Does not explicitly destroy any elements. All memory is retained for reuse.
         */
        void clear() {
            if (root_) {
                root_->next.reset(); // Detach all blocks beyond root
                root_->size_ = 0;
                head_ = root_.get();
            }
            size_ = 0;
        }

        /**
         * @brief Clears all elements and optionally retains a fixed number of memory blocks.
         *
         * @param keep_blocks If specified, retains up to N blocks and clears all others.
         * If omitted, retains all blocks for future use.
         */
        void clear_reserve(const std::optional<std::uint64_t> keep_blocks = std::nullopt) {
            size_ = 0;
            if (!root_) return;

            root_->size_ = 0;
            head_ = root_.get();

            if (!keep_blocks.has_value()) return;

            node *current = root_.get();
            std::uint64_t count = 1;

            while (current->next && count < *keep_blocks) {
                current = current->next.get();
                current->size_ = 0;
                ++count;
            }

            if (current) {
                current->next.reset(); // Release all blocks beyond the threshold
            }
        }
    };
} // namespace jh
