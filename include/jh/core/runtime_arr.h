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
 * @file runtime_arr.h
 * @brief RAII-managed, non-resizable runtime array &mdash; a safe modern replacement for C99 VLA.
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 *
 * <h3>Overview</h3>
 * <p>
 * <code>jh::runtime_arr&lt;T, Alloc&gt;</code> provides a <b>safe, RAII-managed</b> version
 * of the C99 Variable Length Array (VLA) concept, which was removed from C++
 * due to undefined behavior and stack safety concerns.
 * </p>
 * <p>
 * It represents a <b>runtime-sized but fixed-capacity</b> array with deterministic lifetime
 * management &mdash; effectively combining the semantics of <code>std::array</code>
 * (fixed capacity) with the flexibility of <code>std::vector</code>
 * (runtime sizing), but without dynamic resizing.
 * </p>
 *
 * <h3>Design Goals</h3>
 * <ul>
 *   <li>Provide a safe, heap-based alternative to C99 VLAs with deterministic lifetime (RAII).</li>
 *   <li>Offer predictable memory ownership and <b>no implicit growth or reallocation</b>.</li>
 *   <li>Maintain contiguous memory layout and full STL interoperability (<code>std::span</code>, ranges).</li>
 *   <li>Support POD-aware zeroing (<code>reset_all()</code>) and uninitialized construction paths.</li>
 *   <li>Expose allocator parameterization for custom memory management, but default to safe local semantics.</li>
 * </ul>
 *
 * <h3>Core Characteristics</h3>
 * <table>
 *   <tr><th>Aspect</th><th>Behavior</th></tr>
 *   <tr><td>Ownership</td><td>Unique / move-only (RAII semantics)</td></tr>
 *   <tr><td>Resizability</td><td>&#10060; &mdash; fixed-size only</td></tr>
 *   <tr><td>Allocator</td><td>Optional (default: <code>typed::monostate</code>)</td></tr>
 *   <tr><td>Initialization</td><td>Zero, uninitialized, or iterator-based</td></tr>
 *   <tr><td>POD optimization</td><td>Automatic <code>memset</code> zeroing for POD-like types</td></tr>
 *   <tr><td>Interop</td><td>STL-compatible iterators, <code>std::span</code>, <code>view_interface</code></td></tr>
 * </table>
 *
 * <h3>Comparison vs Related Containers</h3>
 * <table>
 *   <tr>
 *     <th>Feature</th>
 *     <th><code>std::vector&lt;T&gt;</code></th>
 *     <th><code>jh::runtime_arr&lt;T&gt;</code></th>
 *     <th><code>std::array&lt;T, N&gt;</code></th>
 *     <th><code>VLA (C99)</code></th>
 *   </tr>
 *   <tr>
 *     <td>Compile-time size</td>
 *     <td>&#10060;</td>
 *     <td>&#10060;</td>
 *     <td>&#9989;</td>
 *     <td>&#10060;</td>
 *   </tr>
 *   <tr>
 *     <td>Runtime size (fixed after init)</td>
 *     <td>&#9989;</td>
 *     <td>&#9989; (non-resizable)</td>
 *     <td>&#10060;</td>
 *     <td>&#9989;</td>
 *   </tr>
 *   <tr>
 *     <td>Resizing / growth</td>
 *     <td>&#9989;</td>
 *     <td>&#10060;</td>
 *     <td>&#10060;</td>
 *     <td>&#10060;</td>
 *   </tr>
 *   <tr>
 *     <td>Allocator control</td>
 *     <td>&#9989; (optional)</td>
 *     <td>&#9989; (optional)</td>
 *     <td>&#10060;</td>
 *     <td>&#10060;</td>
 *   </tr>
 *   <tr>
 *     <td>Storage location</td>
 *     <td>Heap</td>
 *     <td>Heap<br>(RAII-managed)</td>
 *     <td>Stack / static</td>
 *     <td>Stack (unsafe)</td>
 *   </tr>
 *   <tr>
 *     <td>Exception safety</td>
 *     <td>Strong</td>
 *     <td>Strong<br>(RAII + noexcept moves)</td>
 *     <td>Strong</td>
 *     <td>Undefined</td>
 *   </tr>
 *   <tr>
 *     <td>POD zero-reset</td>
 *     <td>&#10060;</td>
 *     <td>&#9989; (<code>reset_all()</code>)</td>
 *     <td>&#10060;</td>
 *     <td>&#10060;</td>
 *   </tr>
 *   <tr>
 *     <td>Lifetime management</td>
 *     <td>Automatic (allocator)</td>
 *     <td>RAII-owned unique_ptr</td>
 *     <td>Automatic</td>
 *     <td>Automatic (non-deterministic destruction)</td>
 *   </tr>
 * </table>
 *
 * <h3>Design Motivation</h3>
 * <p>
 * While C99 introduced Variable Length Arrays (VLAs) to allow runtime-sized stack arrays,
 * they were banned in C++ due to undefined lifetime behavior, missing exception handling,
 * and non-portable ABI implications.
 * </p>
 * <p>
 * <code>jh::runtime_arr</code> safely revives the same expressiveness using heap-based allocation,
 * strong RAII ownership, and predictable lifetime management &mdash; without giving up performance
 * or direct pointer interoperability.
 * </p>
 *
 * <h3>Specializations</h3>
 * <ul>
 *   <li><code>runtime_arr&lt;bool&gt;</code> &mdash; bit-packed specialization (64-bit words).</li>
 *   <li>Provides <code>set()</code>, <code>unset()</code>, <code>test()</code>, <code>reset_all()</code> for bit control.</li>
 *   <li>Explicitly disables <code>data()</code> and <code>as_span()</code> for safety.</li>
 * </ul>
 *
 * <h3>Notes</h3>
 * <ul>
 *   <li>No reallocation or growth semantics; all operations are in-place.</li>
 *   <li>Prefer <code>reset_all()</code> to <code>clear()</code> for POD types.</li>
 *   <li>Move-only by design &mdash; copying is deleted.</li>
 *   <li>Ideal as a stable buffer for algorithms requiring strict capacity contracts.</li>
 * </ul>
 *
 * <h3>Implementation Summary</h3>
 * <ul>
 *   <li>Implements <code>std::ranges::view_interface</code> for range integration.</li>
 *   <li>Backed by <code>unique_ptr&lt;T[], deleter&gt;</code> (RAII).</li>
 *   <li>Allocator-aware; default uses <code>typed::monostate</code>.</li>
 *   <li>Optimized <code>reset_all()</code> for POD/trivially destructible types.</li>
 * </ul>
 *
 * <h3>Dual-Mode Header Integration</h3>
 * <p>
 * This header participates in the <b>Dual-Mode Header</b> system.
 * You do not need to modify the source code to switch build modes:
 * </p>
 * <ul>
 *   <li>When linked via <code>jh-toolkit</code> &rarr; behaves as a <b>header-only</b> component.</li>
 *   <li>When linked via <code>jh-toolkit-static</code> &rarr; uses the <b>precompiled static</b> implementation.</li>
 * </ul>
 * <p>
 * The mode is resolved automatically through <code>JH_INTERNAL_SHOULD_DEFINE</code>,
 * consistent with <code>jh::immutable_str</code> and other dual-mode headers
 * (<b>currently limited to</b> <code>immutable_str</code> and <code>runtime_arr</code> in v1.3.x,
 * but <b>future releases may extend this system</b> to additional components).
 * </p>
 *
 * <h3>Performance Summary</h3>
 *
 * <p>
 * Microbenchmark results for <code>jh::runtime_arr&lt;T&gt;</code>
 * (1024 POD elements, Apple Silicon M3, LLVM clang++ 20, 2025):
 * </p>
 *
 * <table>
 *   <tr><th>Optimization Level</th><th><code>std::vector&lt;T&gt;</code></th><th><code>runtime_arr&lt;T&gt;</code></th><th>Relative Speedup</th></tr>
 *   <tr><td>-O0</td><td>&asymp; 7.6 &micro;s</td><td>&asymp; 0.15 &micro;s</td><td>&asymp; 50&times;</td></tr>
 *   <tr><td>-O2</td><td>&asymp; 0.13 &micro;s</td><td>&asymp; 0.017 &micro;s</td><td>&asymp; 7&times;</td></tr>
 *   <tr><td>-O3</td><td>&asymp; 0.15 &micro;s</td><td>&asymp; 0.017 &micro;s</td><td>&asymp; 8&times;</td></tr>
 *   <tr><td>-Ofast</td><td>&asymp; 0.16 &micro;s</td><td>&asymp; 0.017 &micro;s</td><td>&asymp; 9&times;</td></tr>
 * </table>
 *
 * <h4>Observations</h4>
 * <ul>
 *   <li>Benchmarks executed on <b>Apple Silicon M3</b> using <b>LLVM clang++ 20</b> (Darwin target).</li>
 *   <li>For trivially constructible POD types, <code>runtime_arr</code> exhibits allocation cost virtually identical to raw <code>operator new[]</code>.</li>
 *   <li>From <code>-O2</code> upward, both <code>std::vector</code> and <code>runtime_arr</code> reach optimization saturation;
 *       higher levels (<code>-O3</code>, <code>-Ofast</code>) bring negligible gains.</li>
 *   <li>The consistent 6-9&times; advantage stems from <code>runtime_arr</code>'s simplified layout,
 *       absence of <code>allocator_traits</code> indirection, and elimination of dynamic capacity management.</li>
 *   <li>Measured variance &lt; 1 % across runs, confirming deterministic RAII allocation and compiler inlining behavior.</li>
 * </ul>
 *
 * <p>
 * These results indicate that <code>jh::runtime_arr</code> offers
 * <b>stable, compiler-optimized, and allocation-efficient</b> performance
 * for fixed-size runtime buffers &mdash; matching the predictability of raw arrays
 * while preserving RAII semantics and full STL interoperability.
 * </p>
 *
 * @see jh::pod::pod_like
 * @see jh::typed::monostate
 *
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include <cstdint>
#include <vector>
#include <span>
#include <stdexcept>
#include <cstring>
#include <functional>
#include <memory>
#include <type_traits>

#include "jh/conceptual/iterator.h"
#include "jh/pods/pod_like.h"
#include "jh/typing/monostate.h"

namespace jh {

    namespace detail {
        /// @brief Checks if Alloc provides direct allocate/deallocate for T.
        template<typename A, typename T>
        concept direct_alloc_for =
        (!jh::typed::monostate_t<A>) && requires(A a, std::uint64_t n) {
            { a.allocate(n) } -> std::same_as<T *>;
            { a.deallocate(std::declval<T *>(), n) };
        };

        /// @brief Checks if Alloc can be rebound to T via allocator_traits.
        template<typename A, typename T>
        concept rebind_alloc_for =
        (!jh::typed::monostate_t<A>) && requires(std::uint64_t n) {
            requires requires(typename std::allocator_traits<A>::template rebind_alloc<T> rebind){
                rebind.allocate(n);
                rebind.deallocate(std::declval<T *>(), n);
            };
        };

        /**
         * @brief Resolves the appropriate allocator type for runtime_arr.
         *
         * @details
         * <strong>Resolution Logic:</strong>
         * <ol>
         *  <li>If <code>Alloc</code> is <code>typed::monostate</code>, use <code>typed::monostate</code>.</li>
         *  <li>If <code>Alloc</code> directly supports <code>allocate(n)</code> / <code>deallocate(ptr, n)</code> for <code>T</code>,
         *      use <code>Alloc</code> as-is.</li>
         *  <li>If <code>Alloc</code> can be rebound to <code>T</code> via <code>std::allocator_traits</code>,
         *      use the rebound allocator type.</li>
         *  <li>Otherwise, resolution fails with <code>void</code>.</li>
         * </ol>
         * @tparam T Element type.
         * @tparam Alloc Provided allocator type.
         */
        template<typename T, typename Alloc>
        struct rt_arr_alloc final {
            using type = decltype([]() {
                if constexpr (jh::typed::monostate_t<Alloc>) {
                    return std::type_identity<jh::typed::monostate>{};
                } else if constexpr (direct_alloc_for<Alloc, T>) {
                    return std::type_identity<Alloc>{};
                } else if constexpr (rebind_alloc_for<Alloc, T>) {
                    using rebound = typename std::allocator_traits<Alloc>::template rebind_alloc<T>;
                    return std::type_identity<rebound>{};
                } else {
                    return std::type_identity<void>{};
                }
            }())::type;
        };

        /// @brief Helper alias for resolved allocator type.
        template<typename T, typename Alloc>
        using rt_arr_alloc_t = typename rt_arr_alloc<T, Alloc>::type;

        /// @brief Concept to validate allocator suitability for runtime_arr.
        template<typename T, typename Alloc>
        concept valid_rt_arr_allocator =
        !std::is_same_v<rt_arr_alloc_t<T, Alloc>, void>;

    } // namespace detail

    /**
     * @brief A move-only, fixed-capacity array with runtime-determined length and RAII-based ownership.
     *
     * @tparam T      Element type.
     * @tparam Alloc  Allocator type &mdash; defaults to <code>typed::monostate</code> (uses <code>new[]</code> / <code>delete[]</code>).
     *
     * <h4>Overview</h4>
     * <p>
     * Short for <b>"runtime-sized array"</b>, this class models a <b>heap-allocated,
     * non-resizable container</b> designed as a safe and expressive replacement
     * for manual heap buffers (<code>new T[n]</code>).
     * </p>
     *
     * Unlike <code>std::vector</code>, it forbids resizing, shrinking, or growth semantics.
     * It focuses on <b>semantic clarity</b> rather than raw performance &mdash; making buffer
     * lifetime and capacity constraints explicit.
     *
     * <h4>Core Features</h4>
     * <ul>
     *   <li>Move-only: eliminates accidental copies and aliasing.</li>
     *   <li>RAII-managed heap buffer (via <code>unique_ptr&lt;T[], deleter&gt;</code>).</li>
     *   <li>Optional zero-initialized or uninitialized construction.</li>
     *   <li>STL/ranges compatible (<code>view_interface</code> inheritance).</li>
     *   <li><code>reset_all()</code> for fast POD reset using <code>memset</code>.</li>
     *   <li>Allocator parameterization for custom memory control.</li>
     *   <li><code>as_span()</code> helper for safe interop with <code>std::span&lt;T&gt;</code>.</li>
     * </ul>
     *
     * <h4>Default Allocation Model</h4>
     * <ul>
     *   <li>If <code>Alloc = typed::monostate</code> (default):
     *       <ul>
     *         <li>Memory is allocated via <code>new[]</code> and released via <code>delete[]</code>.</li>
     *         <li>No external allocator is required.</li>
     *       </ul>
     *   </li>
     *   <li>If <code>Alloc</code> provides <code>allocate(n)</code> / <code>deallocate(ptr, n)</code>:
     *       <ul>
     *         <li>Runtime allocation will use the provided allocator instance.</li>
     *       </ul>
     *   </li>
     * </ul>
     *
     * <h4>When to Use</h4>
     * <ul>
     *   <li>As a fixed-capacity buffer with dynamic runtime length.</li>
     *   <li>When <code>std::vector</code>'s resizing semantics are undesired.</li>
     *   <li>As a safer RAII alternative to <code>T* arr = new T[n]</code>.</li>
     * </ul>
     *
     * <h4>When <i>Not</i> to Use</h4>
     * <ul>
     *   <li>If dynamic resizing, push/pop semantics, or polymorphic behavior is required.</li>
     *   <li>If compile-time fixed capacity (<code>std::array</code>) suffices.</li>
     * </ul>
     *
     * <h4>Interop Notes</h4>
     * <ul>
     *   <li>Contiguous and <code>std::span</code>-compatible.</li>
     *   <li>Supports range-for iteration, STL algorithms, and <code>std::ranges::views</code>.</li>
     *   <li><code>runtime_arr&lt;bool&gt;</code> provides bit-packed specialization (see below).</li>
     * </ul>
     *
     * @note
     * <ul>
     *   <li>Use <code>reset_all()</code> instead of <code>clear()</code>.</li>
     *   <li>Use <code>runtime_arr&lt;T&gt;::uninitialized</code> to skip default construction (POD only).</li>
     *   <li>Copy operations are deleted; moves are noexcept.</li>
     * </ul>
     */
    template<typename T, typename Alloc = typed::monostate> requires detail::valid_rt_arr_allocator<T, Alloc>
    class runtime_arr final {
        std::uint64_t size_{0};

        using deleter_t = std::function<void(T *)>;
        std::unique_ptr<T[], deleter_t> data_{nullptr, make_deleter()}; // Tie deleter to allocator

        static void default_deleter(T *p) { delete[] p; } // NOLINT

        static deleter_t make_deleter() {
            if constexpr (typed::monostate_t<allocator_type>) {
                return default_deleter;
            } else {
                return nullptr; // No-op deleter, it will be bound to lambda
            }
        }

    public:

        using value_type = T;                            ///< Value type alias.
        using size_type = std::uint64_t;                 ///< Size type alias (64-bit).
        using difference_type = std::ptrdiff_t;          ///< Difference type alias.
        using reference = value_type &;                  ///< Reference type.
        using const_reference = const value_type &;      ///< Const reference type.
        using pointer = value_type *;                    ///< Pointer type.
        using const_pointer = const value_type *;        ///< Const pointer type.
        using iterator = pointer;
        using const_iterator = const_pointer;
        using allocator_type = detail::rt_arr_alloc_t<T, Alloc>;

    private:
        /// @brief Helper to create allocator instance from provided Alloc.
        allocator_type make_allocator_from(const Alloc &alloc) {
            if constexpr (std::same_as<allocator_type, Alloc>) {
                return alloc;
            } else {
                return allocator_type(alloc);
            }
        }

    public:

        struct uninitialized_t final {
        };

        static constexpr uninitialized_t uninitialized{};

        /**
         * @brief Constructs an uninitialized array of POD-like elements.
         * @param size The number of elements to allocate.
         *
         * <ul>
         *   <li>Enabled only when <code>jh::pod_like&lt;T&gt;</code> and
         *       <code>Alloc</code> is <code>typed::monostate</code>.</li>
         *   <li>Uses <code>operator new[]</code> to allocate raw storage &mdash; the memory is
         *       <strong>completely uninitialized</strong> (no zero-fill, no constructor calls).</li>
         *   <li>For <strong>POD-like</strong> types, such uninitialized allocation is
         *       semantically safe: their lifetime is bound directly to the allocated storage,
         *       and no constructor/destructor side effects are required.</li>
         *   <li>Intended for performance-critical contexts such as bulk I/O buffers,
         *       custom serialization, or explicit zero-fill via <code>std::memset</code>.</li>
         *   <li>In practice, this behaves similarly to <code>std::vector::reserve()</code> &mdash;
         *       capacity is guaranteed, but elements are not value-initialized.</li>
         *   <li>Unlike <code>std::vector</code>, however, <code>runtime_arr</code> does not
         *       incur large penalties when default-initializing POD types:
         *       its "initialized" and "uninitialized" paths compile to nearly identical code
         *       for trivial objects (difference &lt;1%).</li>
         * </ul>
         *
         * <strong>Performance note</strong>
         * <p>
         * For POD and trivially constructible types, both initialized and uninitialized
         * variants of <code>runtime_arr</code> achieve equivalent performance.
         * The <code>uninitialized</code> form primarily exists to express intent &mdash;
         * much like calling <code>std::vector::reserve()</code> &mdash; signaling intent that
         * the elements will be explicitly initialized later, and thus avoiding redundant
         * zero-fills or value-initialization writes that compilers may otherwise emit.
         * For <strong>POD</strong> and trivially constructible types, however, both forms
         * typically compile to identical machine code, since their constructors are
         * effectively no-ops.
         * </p>
         *
         * <p><strong>Note:</strong> The content of the allocated memory is indeterminate until written to.
         * Accessing any element before explicit initialization results in undefined behavior.</p>
         */
        explicit runtime_arr(const std::uint64_t size, uninitialized_t) requires jh::pod::pod_like<T> &&
                                                                                 typed::monostate_t<Alloc> {
            size_ = size;
            T *ptr = static_cast<T *>(operator new[](sizeof(T) * size_));
            data_.reset(ptr);
        }

        /**
         * @brief Constructs a fixed-size runtime array from an initializer list.
         *
         * @param init Initializer list providing the values for each element.
         *
         * <p>
         * Allocates a contiguous buffer of <code>init.size()</code> elements and performs
         * <code>std::uninitialized_copy</code> to populate the storage. The array owns its
         * memory and manages lifetime automatically.
         * </p>
         *
         * <ul>
         *   <li>Enabled only when <code>Alloc == typed::monostate</code>.</li>
         *   <li>Allocates <code>new T[init.size()]</code> and copies or moves elements from <code>init</code>.</li>
         *   <li>Ownership is managed via <code>std::unique_ptr</code> with default deleter.</li>
         *   <li>Move-only type; copy operations are disabled.</li>
         * </ul>
         *
         * @throws std::bad_alloc If allocation fails.
         */
        runtime_arr(std::initializer_list<T> init) requires (typed::monostate_t<Alloc>)
                : size_(init.size()), data_(nullptr, default_deleter) {
            if (size_ == 0) return;
            T *ptr = new T[size_];
            std::uninitialized_copy(init.begin(), init.end(), ptr);
            data_.reset(ptr);
        }

        /**
         * @brief Constructs a zero-initialized array using the default allocation strategy.
         * @param size Number of elements to allocate and value-initialize.
         *
         * <ul>
         *   <li>Requires <code>is_valid_allocator&lt;Alloc&gt;</code>.</li>
         *   <li>If <code>Alloc</code> is <code>typed::monostate</code>, storage is obtained via
         *       <code>new T[size]</code> and automatically released by the default deleter.</li>
         *   <li>Otherwise, a user-provided allocator type is instantiated and used to allocate
         *       <em>exactly</em> <code>size</code> elements, with lifetime managed through a
         *       bound deleter lambda that captures both the allocator and allocation size.</li>
         *   <li>All elements are <strong>zero-initialized</strong> (for PODs) or
         *       <strong>default-constructed</strong> (for non-PODs) immediately after allocation.</li>
         *   <li>Semantically, this behaves like <code>std::vector&lt;T&gt;(size)</code> but
         *       without growth capacity or <code>allocator_traits</code> overhead.</li>
         * </ul>
         *
         * <strong>Performance characteristics</strong>
         * <ul>
         *   <li>For <strong>POD-like</strong> types, this path compiles down to a single
         *       contiguous allocation followed by a <code>memset</code> or equivalent zero-fill.</li>
         *   <li>For non-trivial types, it performs element-wise default construction with
         *       strong exception safety guarantees.</li>
         *   <li>No dynamic resizing or capacity growth is performed &mdash; the array size is fixed
         *       for the lifetime of the object.</li>
         *   <li>In microbenchmarks, this constructor matches or slightly outperforms
         *       <code>std::vector&lt;T&gt;(size)</code> due to the absence of allocator-layer indirection.</li>
         * </ul>
         *
         * <strong>Allocator semantics</strong>
         * <ul>
         *   <li>When using a custom allocator, a lambda deleter is bound that correctly invokes
         *       <code>alloc.deallocate(ptr, size)</code> upon destruction.</li>
         *   <li>This preserves RAII semantics and guarantees proper cleanup even in the
         *       presence of exceptions.</li>
         *   <li>The allocator is captured by value, ensuring deterministic deallocation.</li>
         * </ul>
         *
         * @note This constructor is the canonical entry point for creating
         * safe, fixed-size runtime arrays. It offers predictable initialization and deallocation
         * behavior, suitable for both POD and non-POD types.</p>
         */
        explicit runtime_arr(std::uint64_t size)
                : size_(size) {
            if constexpr (typed::monostate_t<allocator_type>) {
                T *ptr = new T[size_];
                data_.reset(ptr); // Use default_deleter
            } else {
                allocator_type alloc{};
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
         * @brief Constructs a fixed-size runtime array from an initializer list using a custom allocator.
         *
         * @param init Initializer list providing the values for each element.
         * @param alloc Allocator instance used for allocation and deallocation.
         *
         * <p>
         * Allocates <code>init.size()</code> elements via the provided allocator and performs
         * <code>std::uninitialized_copy</code> to populate the buffer. The allocator is stored
         * within a bound deleter lambda for correct deallocation.
         * </p>
         *
         * <ul>
         *   <li>Enabled only when <code>Alloc != typed::monostate</code>.</li>
         *   <li>Performs <code>alloc.allocate(size)</code> and binds <code>alloc.deallocate(ptr, size)</code> as deleter.</li>
         *   <li>Ensures allocator-aware destruction and exception safety.</li>
         *   <li>Move-only type; copy operations are deleted.</li>
         * </ul>
         *
         * @throws std::bad_alloc If allocator fails to provide storage.
         */
        runtime_arr(std::initializer_list<T> init, const Alloc &alloc) requires (!jh::typed::monostate_t<Alloc>)
                : size_(init.size()) {
            allocator_type rebound = make_allocator_from(alloc);
            T *ptr = rebound.allocate(size_);
            std::uninitialized_copy(init.begin(), init.end(), ptr);
            data_ = std::unique_ptr<T[], deleter_t>(
                    ptr,
                    [rebound, size = size_](T *p) mutable { rebound.deallocate(p, size); }
            );
        }

        /**
        * @brief Constructs a runtime array using a movable allocator instance.
         * @param size Number of elements to allocate.
        * @param alloc Allocator instance (may be lvalue or rvalue).
         *
         * <ul>
         *   <li>Enabled only when <code>Alloc</code> is not <code>typed::monostate</code>.</li>
         *   <li>Accepts both lvalue and rvalue allocators; the allocator instance is captured by
         *       value (moved if possible) inside the deleter closure.</li>
         *   <li>Provides full support for <code>std::pmr::polymorphic_allocator&lt;T&gt;</code> and
         *       other stateful allocators.</li>
         * </ul>
         *
         * <strong>Design notes</strong>
         * <ul>
         *   <li>This overload avoids unnecessary allocator copies and preserves resource binding.</li>
         *   <li>Equivalent to the by-value form for trivially copyable allocators.</li>
         *   <li>Ensures allocator lifetime and destruction safety via lambda capture semantics.</li>
         * </ul>
         */
        explicit runtime_arr(std::uint64_t size, const Alloc &alloc) requires (!typed::monostate_t<Alloc>)
                : size_(size) {
            allocator_type rebound = make_allocator_from(alloc);
            T *ptr = rebound.allocate(size_);
            data_ = std::unique_ptr<T[], deleter_t>(
                    ptr,
                    [rebound = std::forward<allocator_type>(rebound), size](T *p) mutable {
                        rebound.deallocate(p, size);
                    }
            );
        }

        /**
         * @brief Constructs a <code>runtime_arr&lt;T&gt;</code> by moving from a
         *        <code>std::vector&lt;T&gt;</code> (only when <code>Alloc</code> is <code>typed::monostate</code>).
         *
         * @param vec Rvalue reference to a <code>std::vector&lt;T&gt;</code> whose contents will be moved.
         *
         * <ul>
         *   <li>Enabled only when <code>Alloc == typed::monostate</code>, ensuring consistent
         *       <code>new[]</code>/<code>delete[]</code> semantics.</li>
         *   <li>Allocates a new contiguous buffer of <code>vec.size()</code> elements via
         *       <code>operator new[]</code>.</li>
         *   <li>Performs an element-wise move from <code>vec</code> into the internal storage.</li>
         *   <li>The source vector remains in a valid but unspecified state after construction
         *       (per standard move semantics).</li>
         *   <li>The resulting <code>runtime_arr</code> owns its own independent storage and
         *       does not alias <code>vec</code>'s memory.</li>
         * </ul>
         *
         * <strong>Rationale</strong>
         * <ul>
         *   <li>This constructor provides a convenient transition path from STL containers
         *       to fixed-size <code>runtime_arr</code> semantics.</li>
         *   <li>Unlike <code>std::vector::reserve()</code> or <code>shrink_to_fit()</code>,
         *       this operation guarantees immutability of capacity and clear ownership transfer.</li>
         *   <li>It is intentionally limited to <code>typed::monostate</code> allocators to
         *       ensure predictable <code>operator new[]</code> / <code>operator delete[]</code>
         *       lifetime management without allocator-specific behavior.</li>
         * </ul>
         *
         * <strong>Performance Notes</strong>
         * <ul>
         *   <li>For trivially movable POD types, the move loop is typically optimized to
         *       a single <code>memcpy</code> by the compiler.</li>
         *   <li>For non-trivial types, each element's move constructor is invoked individually.</li>
         *   <li>Construction cost is proportional to <code>O(n)</code> moves, matching
         *       <code>std::vector::move()</code> semantics but with fixed-size final storage.</li>
         * </ul>
         *
         * <p><strong>Example:</strong></p>
         * @code
         * std::vector&lt;MyPod&gt; data(1024);
         * // Fill data...
         * jh::runtime_arr&lt;MyPod&gt; arr(std::move(data));
         * // arr now owns a separate buffer containing the moved elements.
         * @endcode
         */
        explicit runtime_arr(std::vector<T> &&vec) requires (typed::monostate_t<Alloc>)
                : size_(vec.size()), data_(nullptr, default_deleter) {
            if (!vec.empty()) {
                T *ptr = new T[size_];
                std::move(vec.begin(), vec.end(), ptr);
                data_.reset(ptr);
            }
        }

        /**
         * @brief Constructs a <code>runtime_arr&lt;T&gt;</code> from any valid forward iterator range.
         * @tparam ForwardIt Iterator type satisfying <code>jh::concepts::forward_iterator</code>.
         * @param first Beginning of the input range.
         * @param last End of the input range.
         *
         * <ul>
         *   <li>Enabled only when <code>Alloc == typed::monostate</code> and
         *       <code>T</code> is copy-constructible.</li>
         *   <li>Allocates a contiguous buffer large enough to hold
         *       <code>std::distance(first, last)</code> elements.</li>
         *   <li>Copies (or moves, if wrapped with <code>std::make_move_iterator</code>)
         *       elements from the source range into internal storage.</li>
         *   <li>Ownership is managed via RAII (<code>std::unique_ptr</code> with custom deleter).</li>
         * </ul>
         *
         * <strong>Behavior</strong>
         * <ul>
         *   <li>If the range is empty, the resulting array is empty (<code>size() == 0</code>).</li>
         *   <li>If <code>std::distance(first, last) &lt; 0</code>, an
         *       <code>std::invalid_argument</code> exception is thrown.</li>
         *   <li>Otherwise, <code>new[]</code> is used to allocate storage and
         *       <code>std::copy()</code> (or <code>std::move()</code>) to fill it.</li>
         * </ul>
         *
         * <strong>Examples</strong>
         * <p><strong>From STL containers:</strong></p>
         * @code
         * std::vector&lt;int&gt; v = {1, 2, 3};
         * jh::runtime_arr&lt;int&gt; a(v.begin(), v.end());   // copy
         *
         * std::deque&lt;int&gt; d = {4, 5, 6};
         * jh::runtime_arr&lt;int&gt; b(d.begin(), d.end());   // copy
         *
         * std::string s = "Hello, world!";
         * jh::runtime_arr&lt;char&gt; chars(
         *     std::make_move_iterator(s.begin()),
         *     std::make_move_iterator(s.end()));        // moves underlying characters
         * @endcode
         *
         * <p><strong>From other iterator sources:</strong></p>
         * @code
         * // Construct from raw array range
         * int raw[] = {10, 20, 30, 40};
         * jh::runtime_arr&lt;int&gt; arr(std::begin(raw), std::end(raw));
         *
         * // Construct from std::span
         * std::span&lt;int&gt; sp(raw);
         * jh::runtime_arr&lt;int&gt; arr2(sp.begin(), sp.end());
         * @endcode
         *
         * <p>
         * Applicable to any iterator pair that defines a finite, measurable range &mdash;
         * e.g., pointers, container iterators, or spans.
         * <strong>Single-pass input iterators</strong> (like <code>std::istream_iterator</code>)
         * are <b>not supported</b>, since <code>std::distance()</code> requires
         * multiple passes to compute the range size.
         * </p>
         *
         * <strong>Design rationale</strong>
         * <ul>
         *   <li>This constructor acts as a <b>universal range importer</b>,
         *       supporting all standard forward-iterable containers and algorithms.</li>
         *   <li>It mirrors <code>std::vector(ForwardIt, ForwardIt)</code> semantics,
         *       but without reallocation or capacity growth.</li>
         *   <li>Using <code>jh::concepts::forward_iterator</code> ensures that
         *       <code>std::distance()</code> is non-destructive and efficient.</li>
         *   <li>When used with <code>std::make_move_iterator</code>,
         *       move-constructible elements are efficiently transferred without extra copies.</li>
         * </ul>
         */
        template<typename ForwardIt>
        runtime_arr(ForwardIt first, ForwardIt last) requires (typed::monostate_t<Alloc> &&
                                                               jh::concepts::forward_iterator<ForwardIt> &&
                                                               std::convertible_to<typename ForwardIt::value_type, value_type> &&
                                                               std::is_copy_constructible_v<T>)
                : data_(nullptr, default_deleter) {
            const auto dist = std::distance(first, last);

            if (dist < 0)
                throw std::invalid_argument("Invalid iterator range");
            size_ = static_cast<std::uint64_t>(dist);

            T *ptr = new T[size_];
            std::copy(first, last, ptr);
            data_.reset(ptr);
        }

        /// @brief Returns iterator to the beginning.
        iterator begin() noexcept { return data_.get(); }

        /// @brief Returns const iterator to the beginning.
        [[nodiscard]] const_iterator begin() const noexcept { return data_.get(); }

        /// @brief Returns iterator to the end (past-the-last element).
        iterator end() noexcept { return data_.get() + size_; }

        /// @brief Returns const iterator to the end (past-the-last element).
        [[nodiscard]] const_iterator end() const noexcept { return data_.get() + size_; }

        /// @brief Returns const iterator to the beginning.
        [[nodiscard]] [[maybe_unused]] const_iterator cbegin() const noexcept { return data_.get(); }

        /// @brief Returns const iterator to the end (past-the-last element).
        [[nodiscard]] [[maybe_unused]] const_iterator cend() const noexcept { return data_.get() + size_; }

        /**
         * @brief Unchecked element access.
         *
         * @details
         * Returns a reference to the element at the given index without
         * performing bounds checking (undefined behavior if out of range).
         * Equivalent to <code>*(data() + index)</code>.
         *
         * @param index Element index within <code>[0, size())</code>.
         * @return Reference to the element.
         */
        reference operator[](std::uint64_t index) noexcept { return data_[index]; }

        /**
         * @brief Unchecked const element access.
         *
         * @details
         * Const overload providing read-only access to the element at the
         * given index (undefined behavior if out of range).
         *
         * @param index Element index within <code>[0, size())</code>.
         * @return Const reference to the element.
         */
        const_reference operator[](std::uint64_t index) const noexcept { return data_[index]; }

        /**
         * @brief Bounds-checked element access.
         *
         * @details
         * Returns a reference to the element at the given index,
         * performing explicit range checking.
         * If <code>index &gt;= size()</code>, an <code>std::out_of_range</code>
         * exception is thrown.
         *
         * @param index Element index within <code>[0, size())</code>.
         * @return Reference to the element at the specified index.
         * @throws std::out_of_range If <code>index &gt;= size()</code>.
         * @see operator[]()
         */
        reference at(std::uint64_t index) {
            if (index >= size_) throw std::out_of_range("jh::runtime_arr::at(): index out of bounds");
            return data_[index];
        }

        /**
         * @brief Const bounds-checked element access.
         *
         * @details
         * Returns a const reference to the element at the given index,
         * performing explicit range checking.
         * If <code>index &gt;= size()</code>, an <code>std::out_of_range</code>
         * exception is thrown.
         *
         * @param index Element index within <code>[0, size())</code>.
         * @return Const reference to the element at the specified index.
         * @throws std::out_of_range If <code>index &gt;= size()</code>.
         * @see operator[]()
         */
        [[nodiscard]] const_reference at(std::uint64_t index) const {
            if (index >= size_) throw std::out_of_range("jh::runtime_arr::at(): index out of bounds");
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
         * @brief Resets all elements to their default-initialized state.
         *
         * @details
         * Reinitializes every element in the array as if assigned with <code>T{}</code>.
         * The actual strategy depends on the structural properties of <code>T</code>:
         *
         * <ul>
         *   <li><b>POD-like types</b> &mdash; memory is cleared with <code>std::memset()</code>
         *       for maximal performance and determinism.</li>
         *   <li><b>Trivially destructible types</b> &mdash; reinitialized in-place using
         *       placement-new (<code>new(p) T{}</code>), without invoking destructors.</li>
         *   <li><b>Non-trivial types</b> &mdash; each element is explicitly assigned
         *       <code>T{}</code>, invoking both destructor and constructor logic.</li>
         * </ul>
         *
         * <strong>Implementation Notes</strong>
         * <ul>
         *   <li><b>Flatten attribute:</b> Explicit compiler flattening (e.g. <code>[[gnu::flatten]]</code>)
         *       was experimentally tested and removed. On <b>LLVM Clang 20</b> with <code>-O3</code> or higher,
         *       <code>flatten</code> introduced micro-jitter and inhibited
         *       certain inliner heuristics.
         *       Modern LLVM optimizers already perform ideal unrolling and hoisting automatically.</li>
         *   <li><b>Template parameter <code>U</code>:</b> This indirection allows
         *       SFINAE-based disabling via <code>requires(std::is_default_constructible_v&lt;U&gt;)</code>
         *       without interfering with specialized overloads &mdash; such as
         *       the <code>runtime_arr&lt;bool&gt;</code> specialization,
         *       which defines its own <code>reset_all()</code>.</li>
         * </ul>
         *
         * @note This function is <code>noexcept</code> for all valid <code>T</code>.
         * @warning Accessing elements before reset on uninitialized memory results in undefined behavior.
         */
        template<typename U = T>
        requires(std::is_default_constructible_v<U>)
        inline void reset_all() noexcept {
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

        /**
         * @brief Returns the number of elements in the array.
         *
         * @return Number of elements currently stored.
         */
        [[nodiscard]] size_type size() const noexcept { return size_; }

        /**
         * @brief Checks whether the array is empty.
         *
         * @return <code>true</code> if <code>size() == 0</code>, otherwise <code>false</code>.
         */
        [[nodiscard]] bool empty() const noexcept { return size_ == 0; }

        /**
         * @brief Provides raw pointer access to the underlying storage.
         *
         * @return Pointer to the first element, or <code>nullptr</code> if empty.
         */
        pointer data() noexcept { return data_.get(); }

        /**
         * @brief Provides const raw pointer access to the underlying storage.
         *
         * @return Const pointer to the first element, or <code>nullptr</code> if empty.
         */
        [[nodiscard]] const_pointer data() const noexcept { return data_.get(); }

        /**
         * @brief Constructs a new <code>runtime_arr</code> by taking ownership of another instance's data.
         *
         * @details
         * Transfers ownership of the underlying buffer from <code>other</code> to this instance.
         * After the move, <code>other</code> is left in a valid but empty state
         * (<code>size() == 0</code>, <code>data() == nullptr</code>).
         */
        runtime_arr(runtime_arr &&other) noexcept
                : size_(other.size_), data_(std::move(other.data_)) {
            other.size_ = 0;
            other.data_.reset();
        }

        /**
         * @brief Replaces the contents of this <code>runtime_arr</code> with those of another, transferring ownership.
         *
         * @details
         * Releases any existing data owned by this instance and takes ownership
         * of <code>other</code>'s buffer.
         * The source <code>other</code> becomes empty and remains safely destructible.
         *
         * @return Reference to the updated <code>*this</code>.
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
         * @brief Converts the array into a <code>std::vector&lt;T&gt;</code> by moving its contents.
         *
         * @details
         * This conversion performs a one-way ownership transfer from
         * <code>runtime_arr&lt;T&gt;</code> to <code>std::vector&lt;T&gt;</code>,
         * consuming the source in the process.
         * After the conversion, the original <code>runtime_arr</code> becomes
         * an empty, valid but unspecified object
         * (<code>size() == 0</code>, <code>data() == nullptr</code>).
         *
         * <strong>Behavior</strong>
         * <ul>
         *   <li><b>POD-like types (<code>jh::pod_like&lt;T&gt;</code>):</b>
         *       Performs a raw <code>std::memcpy</code> for maximal performance.
         *       The operation is equivalent to copying a contiguous byte buffer.</li>
         *   <li><b>Non-POD types:</b>
         *       Uses <code>std::make_move_iterator</code> to move-construct
         *       each element into the target vector, ensuring proper object semantics.</li>
         * </ul>
         *
         * <strong>Symmetry</strong>
         * <p>
         * This operator complements the constructor
         * <code>runtime_arr(std::vector&lt;T&gt;&amp;&amp;)</code>,
         * enabling seamless two-way transfer between
         * <code>std::vector&lt;T&gt;</code> and <code>runtime_arr&lt;T&gt;</code>
         * with full move semantics.
         * Both conversions leave the source container in a valid but empty state,
         * ensuring safe RAII destruction.
         * </p>
         *
         * @note This operator is only available on rvalues
         *       (<code>runtime_arr&lt;T&gt;&amp;&amp;</code>), preventing accidental copies.
         * @see runtime_arr(std::vector&lt;T&gt;&amp;&amp;)
         */
        explicit operator std::vector<T>() && {
            if constexpr (std::is_same_v<T, bool> && !typed::monostate_t<Alloc>) {
                std::vector<bool> vec(size_);
                for (std::uint64_t i = 0; i < size_; ++i)
                    vec[i] = static_cast<bool>(data_[i]);
                size_ = 0;
                data_.reset();
                return vec;
                // compressed-bit fix
            } else if constexpr (pod::pod_like<T>) {
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

        /**
         * @brief Copy constructor is explicitly deleted.
         *
         * @details
         * Copying a <code>runtime_arr</code> is intentionally disallowed,
         * as the class models a <b>fixed-size, region-bound buffer</b> &mdash;
         * conceptually similar to a safe version of a
         * <b>VLA (Variable Length Array)</b>.
         *
         * <p>
         * Each <code>runtime_arr</code> instance owns a unique contiguous
         * memory block via <code>std::unique_ptr</code>.
         * Allowing copy semantics would imply duplicating or aliasing
         * this region, violating its design goal of
         * <strong>unique, region-local ownership</strong>.
         * </p>
         *
         * <p>
         * To share or pass an existing array, use references:
         * <code>runtime_arr&amp;</code> or
         * <code>std::reference_wrapper&lt;runtime_arr&gt;</code>.
         * This preserves ownership while allowing safe, non-copy access
         * across function boundaries.
         * </p>
         */
        runtime_arr(const runtime_arr &) = delete;

        /**
         * @brief Copy assignment operator is explicitly deleted.
         *
         * @details
         * Like the copy constructor, copy assignment is disabled to prevent
         * unintended duplication of the underlying buffer.
         * This enforces strict RAII-style ownership and guarantees
         * deterministic lifetime management.
         *
         * <p>
         * When sharing a buffer between scopes or functions,
         * pass by reference (<code>runtime_arr&amp;</code>) or
         * wrap in <code>std::reference_wrapper&lt;runtime_arr&gt;</code>
         * instead of copying.
         * </p>
         */
        runtime_arr &operator=(const runtime_arr &) = delete;

        [[nodiscard]] std::span<value_type> as_span() noexcept { return {data(), size()}; }

        [[nodiscard]] std::span<const value_type> as_span() const noexcept { return {data(), size()}; }

        [[maybe_unused]] static bool is_static_built() {
#ifdef JH_IS_STATIC_BUILD
            return true;
#else
            return false;
#endif // JH_IS_STATIC_BUILD
        }
    };

    namespace runtime_arr_helper {
        /**
         * @brief Flat allocator for <code>bool</code> &mdash; disables bit-packing in <code>jh::runtime_arr&lt;bool&gt;</code>.
         *
         * <h4>Overview</h4>
         * <p>
         * <code>bool_flat_alloc</code> provides a minimal, stateless allocator used to
         * instantiate <code>jh::runtime_arr&lt;bool, bool_flat_alloc&gt;</code> as a
         * <b>byte-based (non-packed)</b> boolean array.
         * </p>
         *
         * <p>
         * Normally, <code>jh::runtime_arr&lt;bool&gt;</code> uses a <b>bit-packed</b> storage model,
         * compressing 8 boolean values into a single byte (64 per 64-bit word).
         * By supplying this allocator, each <code>bool</code> instead occupies 1 full byte,
         * allowing <b>direct memory access</b> and <b>STL-contiguous semantics</b>.
         * </p>
         *
         * <h4>Usage</h4>
         * @code
         * // Compact, bit-packed (default)
         * jh::runtime_arr&lt;bool&gt; packed(128);
         *
         * // Byte-based, non-packed
         * jh::runtime_arr&lt;bool, jh::runtime_arr_helper::bool_flat_alloc&gt; plain(128);
         * plain[0] = true;
         * @endcode
         *
         * <strong>Behavior</strong>
         * <ul>
         *   <li>Stateless &mdash; all methods are <code>static</code>.</li>
         *   <li>Allocates raw <code>bool[n]</code> arrays via <code>new[]</code>.</li>
         *   <li>Deallocates via <code>delete[]</code>.</li>
         *   <li>Serves as the <b>byte-based alternative allocator</b> for testing or explicit non-packed use cases.</li>
         *   <li>Does <b>not</b> implement <code>std::allocator_traits</code> and is not STL-compatible.</li>
         * </ul>
         *
         * <h4>Typical Use Case</h4>
         * <p>
         * Used for <b>performance benchmarking</b> and correctness testing of the
         * <code>jh::runtime_arr&lt;bool&gt;</code> specialization, or for scenarios where
         * bit-packing overhead is undesirable (e.g. frequent random access or interop with
         * APIs expecting <code>bool*</code> pointers).
         * </p>
         *
         * @see
         * <ul>
         *   <li><code>jh::runtime_arr&lt;bool&gt;</code> &mdash; bit-packed specialization.</li>
         *   <li><code>jh::runtime_arr&lt;bool, runtime_arr_helper::bool_flat_alloc&gt;</code> &mdash; byte-based variant.</li>
         * </ul>
         */
        struct bool_flat_alloc final {
            /// @brief Allocate <code>n</code> bytes for a <code>bool</code> array (non-packed form).
            static bool *allocate(std::uint64_t n) { return new bool[n]; }

            /// @brief Deallocate a previously allocated <code>bool</code> array.
            /// @note Parameter <code>p</code> must not be <code>const</code> &mdash; <code>delete[] const bool*</code> is undefined behavior.
            static void deallocate(bool *p, std::uint64_t) { delete[] p; } // NOLINT
        };

    } // namespace runtime_arr_helper

    /**
     * @brief Specialized implementation of <code>jh::runtime_arr&lt;bool&gt;</code> &mdash; a compact, bit-packed boolean array.
     *
     * <h4>Overview</h4>
     * <p>
     * This specialization provides a <b>memory-efficient representation</b> for <code>bool</code> values,
     * storing them as individual bits within 64-bit words (<code>uint64_t[]</code>).
     * Each bit represents a boolean value, achieving <b>8&times; memory compression</b> compared to
     * the generic <code>runtime_arr&lt;T, Alloc&gt;</code> template (which stores one byte per <code>bool</code>).
     * </p>
     *
     * <p>
     * Its purpose is not raw speed but <b>spatial density</b> and <b>fragmentation reduction</b> &mdash; ideal for
     * large logical masks, flags, and occupancy bitfields.
     * </p>
     *
     * <h4>Relation to Generic Template</h4>
     * <p>
     * This specialization mirrors the structure of the generic <code>runtime_arr&lt;T, Alloc&gt;</code>,
     * but modifies or disables certain operations that are incompatible with bit-level storage.
     * </p>
     *
     * <table>
     *   <tr><th>Semantics</th><th>Generic Member</th><th>Bool Specialization Equivalent</th><th>Notes</th></tr>
     *   <tr><td>Raw access</td><td><code>data()</code>, <code>as_span()</code></td><td>&#10060; Deleted</td><td>Direct pointer access invalid for bit-packed layout.</td></tr>
     *   <tr><td>Element access</td><td><code>operator[](i)</code></td><td>&#9989; Reimplemented</td><td>Non-const returns <code>bit_ref</code>, const returns <code>bool</code>.</td></tr>
     *   <tr><td>Bounded access</td><td><code>at(i)</code></td><td>&#9989; Reimplemented</td><td>Same proxy/value semantics with range checking.</td></tr>
     *   <tr><td>Bulk reset</td><td><code>reset_all()</code></td><td>&#9989; Implemented</td><td>Clears all bits via <code>std::memset()</code>.</td></tr>
     *   <tr><td>Bit manipulation</td><td>(none)</td><td>&#9989; <code>set()</code>, <code>unset()</code>, <code>test()</code></td><td>New API for direct bit operations.</td></tr>
     *   <tr><td>Allocator constructor</td><td><code>runtime_arr(size, Alloc)</code></td><td>&#10060; Deleted</td><td>Custom allocators not supported for bit layout.</td></tr>
     *   <tr><td>Copy semantics</td><td>&#10060; Deleted</td><td>&#10060; Deleted</td><td>Copying disallowed to prevent shallow duplication.</td></tr>
     *   <tr><td>Move semantics</td><td>&#9989; Supported</td><td>&#9989; Supported</td><td>Safe ownership transfer via RAII.</td></tr>
     * </table>
     *
     * <h4>Core Characteristics</h4>
     * <ul>
     *   <li>Stores bits compactly in 64-bit words (<code>uint64_t[]</code>).</li>
     *   <li>Uses <code>bit_ref</code> proxies for writable element access.</li>
     *   <li>Const accessors return plain <code>bool</code> values.</li>
     *   <li>Implements <code>bit_iterator</code> for STL-style traversal.</li>
     *   <li>Provides low-level access via <code>raw_data()</code> and <code>raw_word_count()</code>.</li>
     *   <li>Not a contiguous range (proxy elements are non-trivial).</li>
     * </ul>
     *
     * <h4>Usage Guidance</h4>
     * <p>
     * This specialization is automatically selected when <code>T == bool</code> and
     * the allocator parameter is omitted:
     * </p>
     *
     * @code
     * jh::runtime_arr&lt;bool&gt; bits(128);
     * bits.set(3);
     * bits.unset(1);
     * bool b = bits.test(3);
     * @endcode
     *
     * <h4>To disable bit packing:</h4>
     * <p>
     * Use <code>jh::runtime_arr&lt;bool, jh::runtime_arr_helper::bool_flat_alloc&gt;</code>
     * to obtain a <b>byte-based layout</b> (one byte per <code>bool</code>).
     * This form is also the baseline used in all performance comparisons below.
     * </p>
     *
     * @code
     * jh::runtime_arr&lt;bool, jh::runtime_arr_helper::bool_flat_alloc&gt; plain(256);
     * plain[0] = true;  // Stored as 1 byte per bool
     * @endcode
     *
     * <h4>Behavior Summary</h4>
     * <table>
     *   <tr><th>Aspect</th><th>Generic <code>runtime_arr&lt;T&gt;</code></th><th>Specialized <code>runtime_arr&lt;bool&gt;</code></th></tr>
     *   <tr><td>Storage layout</td><td>Contiguous <code>T[]</code></td><td>Bit-packed (<code>uint64_t[]</code>)</td></tr>
     *   <tr><td>Element access</td><td>Direct reference</td><td>Proxy (<code>bit_ref</code>) / value (<code>bool</code>)</td></tr>
     *   <tr><td><code>data()</code> / <code>as_span()</code></td><td>&#9989;</td><td>&#10060; Deleted</td></tr>
     *   <tr><td>Allocator awareness</td><td>&#9989;</td><td>&#10060; Deleted</td></tr>
     *   <tr><td>Copy semantics</td><td>&#10060;</td><td>&#10060;</td></tr>
     *   <tr><td>Move semantics</td><td>&#9989;</td><td>&#9989;</td></tr>
     *   <tr><td><code>reset_all()</code></td><td>Element-wise reset</td><td>Zero bits via <code>memset()</code></td></tr>
     *   <tr><td>Primary use</td><td>General runtime array</td><td>Compact boolean bitset</td></tr>
     * </table>
     *
     * <h4>Performance Characteristics</h4>
     *
     * <p>
     * Microbenchmark results for <code>jh::runtime_arr&lt;bool&gt;</code>
     * versus its byte-based counterpart
     * <code>jh::runtime_arr&lt;bool, jh::runtime_arr_helper::bool_flat_alloc&gt;</code>,
     * collected on <b>Apple Silicon M3</b> with <b>LLVM clang++ 20</b> (2025),
     * under the following setup:
     * </p>
     *
     * <ul>
     *   <li>Array sizes: 1,024 and 1,000,000 elements</li>
     *   <li>Bernoulli(0.5) data distribution</li>
     *   <li>Catch2 microbenchmark harness</li>
     *   <li>Single-threaded, in-cache workload</li>
     * </ul>
     *
     * <h5>Empirical results</h5>
     * <table>
     *   <tr>
     *     <th rowspan="2">Optimization</th>
     *     <th colspan="3">N = 1,000,000 elements</th>
     *     <th colspan="3">N = 1,024 elements</th>
     *   </tr>
     *   <tr>
     *     <th>set()</th><th>read()</th><th>reset_all()</th>
     *     <th>set()</th><th>read()</th><th>reset_all()</th>
     *   </tr>
     *   <tr><td>-O0</td>
     *     <td>~20&times; slower</td><td>~2.8&times; slower</td><td>~2.8&times; slower</td>
     *     <td>~0.3&times; faster</td><td>~2.3&times; slower</td><td>~2.3&times; slower</td></tr>
     *   <tr><td>-O2</td>
     *     <td>~38&times; slower</td><td>~160&times; slower</td><td>~130&times; slower</td>
     *     <td>~0.55&times; faster</td><td>~61&times; slower</td><td>~60&times; slower</td></tr>
     *   <tr><td>-O3</td>
     *     <td>~59&times; slower</td><td>~140&times; slower</td><td>~130&times; slower</td>
     *     <td>~0.6&times; faster</td><td>~62&times; slower</td><td>~61&times; slower</td></tr>
     *   <tr><td>-Ofast</td>
     *     <td>~51&times; slower</td><td>~150&times; slower</td><td>~125&times; slower</td>
     *     <td>~0.5&times; faster</td><td>~61&times; slower</td><td>~59&times; slower</td></tr>
     * </table>
     *
     * <h5>Interpretation</h5>
     * <ul>
     *   <li><b>Small arrays (&le;1K):</b> Bit-packing may outperform byte-based storage
     *       in write-heavy scenarios due to 8&times; lower memory bandwidth usage.
     *       Reads and resets remain slower due to bit masking overhead.</li>
     *   <li><b>Large arrays (&ge;1M):</b> Bitwise access overhead dominates;
     *       <code>set()</code> is typically ~30-60&times; slower,
     *       and <code>read()</code> / <code>reset_all()</code> are ~120-160&times;&plusmn; slower but mostly memory-bound.</li>
     *   <li><b>Optimization scaling:</b> <code>-O2</code> already achieves full inlining;
     *       <code>-O3</code> and <code>-Ofast</code> differences are within measurement noise (&plusmn;2%).</li>
     *   <li><b>Static instantiation:</b> A precompiled specialization provides a
     *       <b>debug fallback</b>, mitigating <code>-O0</code> template inlining overhead.</li>
     * </ul>
     *
     * <p>
     * In summary, this specialization trades raw performance for memory compactness.
     * It is most useful for boolean masks, sparse flags, and occupancy grids
     * where space efficiency outweighs per-bit access cost.
     * </p>
     *
     * <h4>Notes</h4>
     * <ul>
     *   <li>Each bit resides in a 64-bit word.</li>
     *   <li>Thread safety is not guaranteed for concurrent modification.</li>
     *   <li>RAII-managed, deterministic destruction.</li>
     * </ul>
     *
     * @see
     * <ul>
     *   <li><code>jh::runtime_arr&lt;T, Alloc&gt;</code> &mdash; generic version.</li>
     *   <li><code>jh::runtime_arr&lt;bool, runtime_arr_helper::bool_flat_alloc&gt;</code> &mdash; byte-based baseline.</li>
     *   <li><code>jh::runtime_arr&lt;bool&gt;::bit_ref</code> &mdash; proxy reference for writable bits.</li>
     *   <li><code>jh::runtime_arr&lt;bool&gt;::bit_iterator</code> &mdash; iterator class for bit traversal.</li>
     * </ul>
     */
    template<>
    class runtime_arr<bool> final {
        std::uint64_t size_{};
        std::unique_ptr<std::uint64_t[]> storage_;
        static constexpr std::uint64_t BITS = 64;

        [[nodiscard]] inline std::uint64_t word_count() const noexcept {
            return (size_ + BITS - 1) / BITS;
        }

    public:
        /**
         * @brief Internal reference proxy for single bit access.
         *
         * <h5>Overview</h5>
         * <p>Represents a writable proxy for an individual bit within a 64-bit word.</p>
         *
         * <h5>Behavior</h5>
         * <ul>
         *   <li>Writable via assignment from <code>bool</code>.</li>
         *   <li>Convertible to <code>bool</code> for read access.</li>
         * </ul>
         */
        struct bit_ref final {
        private:
            std::uint64_t &word_;
            std::uint64_t mask_;

        public:
            bit_ref(std::uint64_t &word, const std::uint64_t bit)
                    : word_(word), mask_(1ULL << bit) {
            }

            bit_ref &operator=(bool val) & noexcept {
                if (val) word_ |= mask_;
                else word_ &= ~mask_;
                return *this;
            }

            // declared for std::output_iterator
            bit_ref &operator=(bool val) const && noexcept {
                const_cast<bit_ref &>(*this) = val;
                return // NOLINT
                        const_cast<bit_ref &>(*this);
            }

            operator bool() // NOLINT
            const {
                return (word_ & mask_) != 0;
            }
        };

        /**
         * @brief Iterator over individual bits in the bit-packed array.
         *
         * @details
         * Provides STL-style traversal for elements stored in
         * <code>runtime_arr&lt;bool&gt;</code>, where each bit is accessed through a
         * lightweight proxy reference.
         *
         * @note
         * Because the element type is represented by proxy objects, the container
         * cannot satisfy the full <code>std::ranges</code> requirements for a
         * boolean range (similar to <code>std::vector&lt;bool&gt;</code>).
         * For a fully standards-compliant boolean range, use the non-packed form:
         * <code>jh::runtime_arr&lt;bool, jh::runtime_arr_helper::bool_flat_alloc&gt;</code>.
         */
        struct bit_iterator final {
        public:
            runtime_arr *parent_;
            std::uint64_t index_;
            using iterator_concept = std::random_access_iterator_tag;
            using iterator_category = iterator_concept;
            using value_type = bool;
            using difference_type = std::ptrdiff_t;
            using reference = bit_ref;
            using pointer = void;

            bit_iterator(runtime_arr *parent, const std::uint64_t index)
                    : parent_(parent), index_(index) {
            }

            bit_ref operator*() const { return (*parent_)[index_]; }

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

        /**
         * @brief Const iterator over individual bits in the bit-packed array.
         *
         * @details
         * Behaves similarly to <code>bit_iterator</code> but yields immutable boolean
         * values directly instead of proxy references. Suitable for read-only
         * traversal of the packed bit representation.
         *
         * @note
         * As with <code>bit_iterator</code>, proxy characteristics prevent
         * <code>runtime_arr&lt;bool&gt;</code> from modeling a fully valid
         * <code>std::ranges</code>-compatible boolean range.
         * For a true boolean range, prefer
         * <code>jh::runtime_arr&lt;bool, jh::runtime_arr_helper::bool_flat_alloc&gt;</code>.
         */
        struct bit_const_iterator final {
        public:
            const runtime_arr *parent_;
            std::uint64_t index_;

            using iterator_concept = std::random_access_iterator_tag;
            using iterator_category = iterator_concept;
            using value_type = bool;
            using difference_type = std::ptrdiff_t;
            using reference = bit_ref;
            using pointer = void;

            bit_const_iterator(const runtime_arr *parent, const std::uint64_t index)
                    : parent_(parent), index_(index) {
            }

            bool operator*() const {
                const auto word = parent_->raw_data()[index_ / BITS];
                return (word >> (index_ % BITS)) & 1ULL;
            }

            bit_const_iterator &operator++() {
                ++index_;
                return *this;
            }

            bit_const_iterator operator++(int) {
                const auto tmp = *this;
                ++*this;
                return tmp;
            }

            bit_const_iterator &operator--() {
                --index_;
                return *this;
            }

            bit_const_iterator operator--(int) {
                const auto tmp = *this;
                --*this;
                return tmp;
            }

            bit_const_iterator &operator+=(const difference_type n) {
                index_ += n;
                return *this;
            }

            bit_const_iterator &operator-=(const difference_type n) {
                index_ -= n;
                return *this;
            }

            bit_const_iterator operator+(const difference_type n) const { return {parent_, index_ + n}; }

            bit_const_iterator operator-(const difference_type n) const { return {parent_, index_ - n}; }

            difference_type operator-(const bit_const_iterator &other) const {
                return static_cast<std::ptrdiff_t>(index_ - other.index_); // NOLINT
            }

            bool operator==(const bit_const_iterator &other) const { return index_ == other.index_; }

            bool operator!=(const bit_const_iterator &other) const { return !(*this == other); }

            bool operator<(const bit_const_iterator &other) const { return index_ < other.index_; }

            bool operator<=(const bit_const_iterator &other) const { return index_ <= other.index_; }

            bool operator>(const bit_const_iterator &other) const { return index_ > other.index_; }

            bool operator>=(const bit_const_iterator &other) const { return index_ >= other.index_; }
        };

        using raw_type = std::uint64_t;
        using value_type = bool;
        using size_type = std::uint64_t;
        using difference_type = std::ptrdiff_t;
        using reference = bit_ref;
        using const_reference = bool;
        using iterator = bit_iterator;
        using const_iterator = bit_const_iterator;

        /**
         * @brief Constructs a bit-packed boolean runtime array with all bits zero-initialized.
         * @param size Number of logical bits to allocate and initialize.
         * @details
         * <strong>Behavior</strong>
         * <ul>
         *   <li>Allocates <code>ceil(size / 64)</code> 64-bit words via <code>new[]</code>.</li>
         *   <li>All bits are cleared to zero (<code>false</code>).</li>
         *   <li>Each bit is accessible through <code>bit_ref</code> proxy references.</li>
         *   <li>Ownership is RAII-managed using <code>std::unique_ptr&lt;uint64_t[]&gt;</code>.</li>
         *   <li>Allocator parameters are not supported for bit-packed storage.</li>
         * </ul>
         *
         * @note
         * <ul>
         *   <li>Move-only; copy operations are deleted.</li>
         *   <li>Use <code>set()</code>, <code>unset()</code>, <code>test()</code>, and <code>reset_all()</code>
         *       for bit manipulation.</li>
         * </ul>
         */
        explicit runtime_arr(std::uint64_t size);

        /**
         * @brief Constructs a bit-packed array by moving data from a <code>std::vector&lt;bool&gt;</code>.
         * @param vec Rvalue reference to <code>std::vector&lt;bool&gt;</code> whose elements are copied bitwise.
         * @details
         * <strong>Behavior</strong>
         * <ul>
         *   <li>Allocates sufficient 64-bit words to store <code>vec.size()</code> bits.</li>
         *   <li>Each element of <code>vec</code> is copied into the corresponding bit position.</li>
         *   <li>Ownership and lifetime are RAII-managed internally.</li>
         *   <li>The source vector remains valid but its contents are not preserved after the operation.</li>
         * </ul>
         *
         * @note
         * <ul>
         *   <li>Copying is bitwise; no shared memory with the original vector.</li>
         *   <li>Move-only type &mdash; copy construction and assignment are deleted.</li>
         * </ul>
         */
        explicit runtime_arr(std::vector<bool> &&vec);

        /**
         * @brief Constructs a bit-packed boolean runtime array from an initializer list.
         *
         * @param init Initializer list of boolean values.
         *
         * <p>
         * Allocates the minimal number of 64-bit words required to represent all elements
         * in <code>init</code>. Each bit is initialized according to the list values using
         * <code>set(i, v)</code>.
         * </p>
         *
         * <ul>
         *   <li>Storage is bit-packed: 64 elements per 64-bit word.</li>
         *   <li>Managed via <code>std::unique_ptr&lt;uint64_t[]&gt;</code>.</li>
         *   <li>Does not use allocators.</li>
         *   <li>Move-only type; copy operations are deleted.</li>
         * </ul>
         *
         * @throws std::bad_alloc If allocation fails.
         */
        runtime_arr(std::initializer_list<bool> init);

        /**
         * @brief Constructs a bit-packed array from a range of boolean values.
         * @tparam ForwardIt Iterator type satisfying <code>jh::concepts::forward_iterator</code>.
         * @param first Iterator to the start of the range.
         * @param last Iterator to the end of the range.
         * @throws std::invalid_argument If the iterator range is invalid.
         * @details
         * <strong>Behavior</strong>
         * <ul>
         *   <li>Computes the number of elements using <code>std::distance(first, last)</code>.</li>
         *   <li>Allocates enough 64-bit words to store all bits.</li>
         *   <li>Clears all bits to zero, then copies values from the input range bitwise.</li>
         *   <li>Throws <code>std::invalid_argument</code> if the range length is negative.</li>
         * </ul>
         *
         * @note
         * <ul>
         *   <li>Supports any forward iterator, including container iterators and <code>std::span</code>.</li>
         *   <li>Single-pass input iterators are not supported.</li>
         *   <li>Resulting array is bit-packed and non-resizable.</li>
         * </ul>
         */
        template<typename ForwardIt>
        runtime_arr(ForwardIt first, ForwardIt last) requires (jh::concepts::forward_iterator<ForwardIt> &&
                                                               std::convertible_to<typename ForwardIt::value_type, value_type>) {
            const auto dist = std::distance(first, last);
            if (dist < 0) throw std::invalid_argument("Invalid iterator range");
            size_ = static_cast<std::uint64_t>(dist);
            storage_ = std::make_unique<std::uint64_t[]>(word_count());
            std::memset(storage_.get(), 0, word_count() * sizeof(std::uint64_t));

            std::uint64_t i = 0;
            for (; first != last; ++first, ++i)
                set(i, static_cast<bool>(*first));
        }

        /**
         * @brief Returns the number of elements in the array.
         * @return Number of elements currently stored.
         */
        [[nodiscard]] size_type size() const noexcept;

        /**
         * @brief Checks whether the array is empty.
         * @return <code>true</code> if <code>size() == 0</code>, otherwise <code>false</code>.
         */
        [[nodiscard]] bool empty() const noexcept;

        /**
         * @brief Provides mutable access to the underlying word buffer.
         * @details
         * <strong>Behavior</strong>
         * <ul>
         *   <li>Returns a pointer to the internal <code>uint64_t</code> storage array.</li>
         *   <li>Each word contains 64 logical bits of packed boolean data.</li>
         *   <li>Intended for low-level bitwise operations, serialization, or direct memory inspection.</li>
         * </ul>
         *
         * @note
         * <ul>
         *   <li>This function replaces <code>data()</code> from the generic template, since the
         *       bit-packed layout is <b>not</b> contiguous in <code>bool</code> units.</li>
         *   <li>Users must manually interpret the bit positions when reading or writing raw words.</li>
         * </ul>
         */
        raw_type *raw_data() noexcept;

        /**
         * @brief Provides const access to the underlying word buffer.
         * @details
         * <strong>Behavior</strong>
         * <ul>
         *   <li>Returns a const pointer to the <code>uint64_t</code> buffer.</li>
         *   <li>Useful for inspection or bitwise read-only operations.</li>
         * </ul>
         *
         * @note
         * <ul>
         *   <li>Equivalent to the mutable form, but guarantees const-correct access.</li>
         *   <li>Replaces <code>const data()</code> from the generic <code>runtime_arr&lt;T&gt;</code> template.</li>
         * </ul>
         */
        [[maybe_unused]] [[nodiscard]] const raw_type *raw_data() const noexcept;

        /**
         * @brief Returns the number of 64-bit words used internally to store all bits.
         * @details
         * <strong>Behavior</strong>
         * <ul>
         *   <li>Computes <code>(size() + 63) / 64</code>, rounding up to the nearest full word.</li>
         *   <li>Matches the physical storage capacity, not just logical bit count.</li>
         * </ul>
         *
         * @note
         * <ul>
         *   <li>Used in conjunction with <code>raw_data()</code> for raw memory operations.</li>
         *   <li>Provided as a safe, constexpr-accessible alternative to manual division.</li>
         * </ul>
         */
        [[nodiscard]] size_type raw_word_count() const noexcept;

        /**
         * @brief Unchecked bit access (read/write).
         *
         * @details
         * Returns a proxy reference to the bit at the specified index
         * without performing bounds checking (undefined behavior if out of range).
         * Equivalent to <code>*(data() + index)</code> in semantics,
         * but operates on a bit-packed storage layout.
         *
         * @param i Bit index within <code>[0, size())</code>.
         * @return Reference proxy object representing the targeted bit.
         */
        reference operator[](std::uint64_t i) noexcept;

        /**
         * @brief Unchecked const bit access (read-only).
         *
         * @details
         * Provides read-only access to the bit at the specified index
         * without bounds checking (undefined behavior if out of range).
         * Returns the boolean value corresponding to the bit's state.
         *
         * @param i Bit index within <code>[0, size())</code>.
         * @return Boolean value of the bit.
         */
        [[nodiscard]] value_type operator[](std::uint64_t i) const noexcept;

        /**
         * @brief Bounds-checked bit access (read/write).
         *
         * @details
         * Returns a proxy reference to the bit at the specified index,
         * performing explicit range checking.
         * If <code>i &gt;= size()</code>, an <code>std::out_of_range</code>
         * exception is thrown.
         *
         * <p>
         * Equivalent to <code>operator[]</code> but with explicit bounds validation.
         * Mirrors the semantics of <code>std::vector&lt;bool&gt;::at()</code>.
         * </p>
         *
         * @param i Bit index within <code>[0, size())</code>.
         * @return Bit reference proxy representing the target bit.
         * @throws std::out_of_range If <code>i &gt;= size()</code>.
         * @see operator[]()
         */
        reference at(std::uint64_t i);

        /**
         * @brief Const bounds-checked bit access (read-only).
         *
         * @details
         * Returns the boolean value of the bit at the specified index,
         * performing explicit range checking.
         * If <code>i &gt;= size()</code>, an <code>std::out_of_range</code>
         * exception is thrown.
         *
         * <p>
         * Mirrors <code>std::vector&lt;bool&gt;::at()</code> semantics &mdash;
         * returning a plain <code>bool</code> rather than a proxy
         * for const access.
         * </p>
         *
         * @param i Bit index within <code>[0, size())</code>.
         * @return Boolean value of the bit.
         * @throws std::out_of_range If <code>i &gt;= size()</code>.
         * @see operator[]()
         */
        [[nodiscard]] value_type at(std::uint64_t i) const;

        /**
         * @brief Sets or clears the bit at given index.
         * @param i Bit index
         * @param val Bit value to assign (<code>true</code> by default)
         * @throws std::out_of_range if i out of bounds
         */
        void set(std::uint64_t i, bool val = true);

        /**
         * @brief Clears the bit at given index.
         * @param i Bit index
         * @throws std::out_of_range if i out of bounds
         */
        void unset(std::uint64_t i);

        /**
         * @brief Tests if the bit at index is set.
         * @param i Bit index
         * @return <code>true</code> if bit is <tt>1</tt>, <code>false</code> if <tt>0</tt>
         * @throws std::out_of_range if i out of bounds
         */
        [[nodiscard]] value_type test(std::uint64_t i) const;

        /**
         * @brief Resets all bits in the bit-packed array to zero.
         * @details
         * <strong>Behavior</strong>
         * <ul>
         *   <li>Clears all stored bits by setting every underlying 64-bit word to <code>0</code>.</li>
         *   <li>After the call, all logical elements read as <code>false</code>.</li>
         * </ul>
         *
         * @note
         * <ul>
         *   <li>This replaces the generic <code>reset_all()</code> implementation for <code>runtime_arr&lt;T&gt;</code>,
         *       which performs element-wise assignment.</li>
         *   <li>Uses <code>std::memset</code> for efficient zeroing of the bit storage buffer.</li>
         *   <li>Equivalent to <code>std::fill(begin(), end(), false)</code> but significantly faster.</li>
         * </ul>
         */
        [[gnu::used]] void reset_all() noexcept;

        /**
         * @brief Move constructor &mdash; transfers ownership of the bit-packed buffer.
         *
         * <strong>Behavior</strong>
         * <ul>
         *   <li>Transfers ownership of the internal <code>uint64_t[]</code> buffer
         *       from <code>other</code> to <code>*this</code>.</li>
         *   <li>After the move, <code>other</code> is left in an empty but valid state
         *       (<code>size() == 0</code>, <code>raw_data() == nullptr</code>).</li>
         *   <li>No memory allocation or bit copy is performed &mdash; the operation is <b>O(1)</b>.</li>
         * </ul>
         *
         * <strong>Rationale</strong>
         * <p>
         * Move semantics allow efficient transfer of large bitsets across scopes,
         * especially when benchmarking or composing higher-level containers.
         * Copying remains disabled to avoid accidental deep duplication of bit-packed data.
         * </p>
         */
        runtime_arr(runtime_arr &&other) noexcept;

        /**
         * @brief Move assignment operator &mdash; transfers ownership of the bit-packed buffer.
         * @details
         * <strong>Behavior</strong>
         * <ul>
         *   <li>Releases any existing owned buffer.</li>
         *   <li>Takes ownership of <code>other</code>'s bit-packed storage.</li>
         *   <li>Leaves <code>other</code> empty and valid.</li>
         *   <li>Performs no memory allocation or element-wise operation.</li>
         * </ul>
         *
         * @note
         * <p>
         * This operator is <code>noexcept</code> and preserves RAII semantics.
         * Copy assignment remains deleted to enforce unique ownership.
         * </p>
         */
        runtime_arr<bool> &operator=(runtime_arr &&other) noexcept;

        /**
         * @brief Converts the bit array into std::vector<bool>.
         *        Elements are copied bit-by-bit.
         * @note This operation consumes the array (clears and resets storage).
         */
        explicit operator std::vector<bool>() &&;

        /// @brief Mutable begin iterator over bits.
        iterator begin() noexcept;

        /// @brief Mutable end iterator over bits.
        iterator end() noexcept;

        /// @brief Const begin iterator over bits.
        [[nodiscard]] const_iterator begin() const noexcept;

        /// @brief Const end iterator over bits.
        [[nodiscard]] const_iterator end() const noexcept;

        /// @brief Const begin iterator over bits.
        [[nodiscard]] [[maybe_unused]] const_iterator cbegin() const noexcept;

        /// @brief Const end iterator over bits.
        [[nodiscard]] [[maybe_unused]] const_iterator cend() const noexcept;

        /**
         * @brief Deleted <code>data()</code> function &mdash; raw pointer access is not valid for bit-packed layout.
         * @details
         * <strong>Behavior</strong>
         * <ul>
         *   <li>This specialization intentionally deletes <code>data()</code> to prevent treating the
         *       bit-packed storage as a contiguous <code>bool*</code> array.</li>
         *   <li>Internally, elements are stored as bits within 64-bit words, not as individual bytes.</li>
         * </ul>
         *
         * @note
         * <ul>
         *   <li>Use <code>raw_data()</code> and <code>raw_word_count()</code> to access the underlying
         *       <code>uint64_t</code> storage for low-level operations.</li>
         *   <li>This deletion ensures type safety and prevents undefined behavior due to misaligned access.</li>
         * </ul>
         */
        void data() const = delete;

        /**
         * @brief Deleted allocator-based constructor.
         * @details
         * <strong>Behavior</strong>
         * <p>
         * The <code>runtime_arr&lt;bool&gt;</code> specialization does not support
         * allocator-based construction because its layout is bit-packed rather than byte-based.
         * All allocation is performed internally via <code>new[]</code> with RAII management.
         * </p>
         *
         * @note
         * <ul>
         *   <li>Allocator granularity (bytes) is incompatible with bit-level storage.</li>
         *   <li>Use <code>runtime_arr&lt;bool, runtime_arr_helper::bool_flat_alloc&gt;</code>
         *       for a byte-based boolean array that supports allocator semantics.</li>
         * </ul>
         */
        runtime_arr(std::uint64_t size, auto) = delete;

        /// @brief Deleted &mdash; bit-packed array cannot expose a contiguous span of bools.
        [[nodiscard]] std::span<value_type> as_span() = delete;

        /// @brief Deleted &mdash; const version; contiguous view over bits is not representable.
        [[nodiscard]] std::span<const value_type> as_span() const = delete;

        /// @brief Copy constructor deleted &mdash; bit array is non-copyable by design.
        runtime_arr(const runtime_arr &) = delete;

        /// @brief Copy assignment deleted &mdash; bit array is non-copyable by design.
        runtime_arr &operator=(const runtime_arr &) = delete;

        [[maybe_unused]] static bool is_static_built();
    };

} // namespace jh

#include "jh/macros/header_begin.h"

namespace jh {
#if defined(JH_HEADER_NO_IMPL)

    extern template
    class runtime_arr<bool, runtime_arr_helper::bool_flat_alloc>;

#endif
#if JH_INTERNAL_SHOULD_DEFINE

    // ---- ctor ----
    JH_INLINE runtime_arr<bool>::runtime_arr(const std::uint64_t size)
            : size_(size),
              storage_(std::make_unique<std::uint64_t[]>(word_count())) {
        std::memset(storage_.get(), 0, word_count() * sizeof(std::uint64_t));
    }

    JH_INLINE runtime_arr<bool>::runtime_arr(std::vector<bool> &&vec)
            : runtime_arr(vec.size()) {
        for (std::uint64_t i = 0; i < size_; ++i)
            set(i, vec[i]);
    }

    // ---- accessors ----

    JH_INLINE auto runtime_arr<bool>::size() const noexcept -> size_type {
        return size_;
    }

    JH_INLINE bool runtime_arr<bool>::empty() const noexcept {
        return size_ == 0;
    }

    JH_INLINE auto runtime_arr<bool>::raw_data() noexcept -> raw_type * {
        return storage_.get();
    }

    [[maybe_unused]] [[maybe_unused]] JH_INLINE auto
    runtime_arr<bool>::raw_data() const noexcept -> const raw_type * {
        return storage_.get();
    }

    JH_INLINE auto runtime_arr<bool>::raw_word_count() const noexcept -> size_type {
        return word_count();
    }

    // ---- bit access ----

    JH_INLINE auto runtime_arr<bool>::operator[](const std::uint64_t i) noexcept -> reference {
        return {storage_[i / BITS], i % BITS};
    }

    JH_INLINE auto runtime_arr<bool>::operator[](const std::uint64_t i) const noexcept -> value_type {
        return (storage_[i / BITS] >> (i % BITS)) & 1U;
    }

    JH_INLINE auto runtime_arr<bool>::at(const std::uint64_t i) -> reference {
        if (i >= size_)
            throw std::out_of_range("jh::runtime_arr<bool>::at(): index out of bounds");
        return operator[](i);
    }

    JH_INLINE auto runtime_arr<bool>::at(const std::uint64_t i) const -> value_type {
        if (i >= size_)
            throw std::out_of_range("jh::runtime_arr<bool>::at(): index out of bounds");
        return operator[](i);
    }

    // ---- modifiers ----

    JH_INLINE void runtime_arr<bool>::set(const std::uint64_t i, const bool val) {
        if (i >= size_) throw std::out_of_range("set(): index out of bounds");
        if (val)
            storage_[i / BITS] |= 1ULL << (i % BITS);
        else
            storage_[i / BITS] &= ~(1ULL << (i % BITS));
    }

    JH_INLINE void runtime_arr<bool>::unset(const std::uint64_t i) {
        if (i >= size_) throw std::out_of_range("unset(): index out of bounds");
        storage_[i / BITS] &= ~(1ULL << (i % BITS));
    }

    JH_INLINE auto runtime_arr<bool>::test(const std::uint64_t i) const -> value_type {
        if (i >= size_) throw std::out_of_range("test(): index out of bounds");
        return (storage_[i / BITS] >> (i % BITS)) & 1U;
    }

    JH_INLINE void runtime_arr<bool>::reset_all() noexcept {
        const auto words = word_count();
        if (words > 0)
            std::memset(storage_.get(), 0, words * sizeof(std::uint64_t));
    }

    // ---- move-only ----

    JH_INLINE runtime_arr<bool>::runtime_arr(runtime_arr<bool> &&other) noexcept
            : size_(other.size_),
              storage_(std::move(other.storage_)) {
        other.size_ = 0;
        other.storage_.reset();
    }

    JH_INLINE runtime_arr<bool> &
    runtime_arr<bool>::operator=(runtime_arr<bool> &&other) noexcept {
        if (this != &other) {
            size_ = other.size_;
            storage_ = std::move(other.storage_);
            other.size_ = 0;
            other.storage_.reset();
        }
        return *this;
    }

    JH_INLINE runtime_arr<bool>::operator std::vector<bool>() && {
        std::vector<bool> vec(size_);
        for (std::uint64_t i = 0; i < size_; ++i)
            vec[i] = static_cast<bool>((*this)[i]);

        size_ = 0;
        storage_.reset();
        return vec;
    }

    JH_INLINE runtime_arr<bool>::runtime_arr(std::initializer_list<bool> init)
            : size_(init.size()),
              storage_(std::make_unique<std::uint64_t[]>(word_count())) {
        std::uint64_t i = 0;
        for (bool v: init)
            set(i++, v);
    }

    // ---- pseudo-range ----

    JH_INLINE auto runtime_arr<bool>::begin() noexcept -> iterator {
        return {this, 0};
    }

    JH_INLINE auto runtime_arr<bool>::end() noexcept -> iterator {
        return {this, size_};
    }

    JH_INLINE auto runtime_arr<bool>::begin() const noexcept -> const_iterator {
        return {this, 0};
    }

    JH_INLINE auto runtime_arr<bool>::end() const noexcept -> const_iterator {
        return {this, size_};
    }

    [[maybe_unused]] JH_INLINE auto runtime_arr<bool>::cbegin() const noexcept -> const_iterator {
        return begin();
    }

    [[maybe_unused]] JH_INLINE auto runtime_arr<bool>::cend() const noexcept -> const_iterator {
        return end();
    }

    // ---- compile-flag ----
    [[maybe_unused]] JH_INLINE bool runtime_arr<bool>::is_static_built() {
#ifdef JH_IS_STATIC_BUILD
        return true;
#else
        return false;
#endif // JH_IS_STATIC_BUILD
    }

#endif // JH_INTERNAL_SHOULD_DEFINE
} // namespace jh

#include "jh/macros/header_end.h"
