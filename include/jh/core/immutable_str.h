/**
 * \verbatim
 * Copyright 2025 JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
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
 * \endverbatim
 */
/**
 * @file immutable_str.h
 * @brief Immutable, thread-safe string with optional auto-trimming and dual-mode build support.
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 *
 * <h3>Overview</h3>
 * <p>
 * <code>jh::immutable_str</code> provides a <b>true immutable</b> string type in modern C++.
 * It guarantees <b>memory-level immutability</b> and <b>thread safety</b> — once created,
 * the string data can never be modified. This makes it ideal for concurrent environments,
 * global configuration caches, or static metadata storage.
 * </p>
 *
 * <h4>Key Characteristics</h4>
 * <ul>
 *   <li>Strict immutability at the memory level — no API allows modification.</li>
 *   <li>Thread-safe by design — multiple threads can safely share instances.</li>
 *   <li>Optional <b>automatic whitespace trimming</b> during construction.</li>
 *   <li>Compact, zero-reallocation model using <code>unique_ptr&lt;const char[]&gt;</code>.</li>
 *   <li>Transparent hashing and equality for unordered containers.</li>
 *   <li>Seamless integration with <code>std::shared_ptr&lt;immutable_str&gt;</code> for safe sharing.</li>
 * </ul>
 *
 * <h3>Motivation</h3>
 * <p>
 * In C++, <code>std::string</code> remains mutable even when declared <code>const</code>.
 * This permits unintended modification via <code>const_cast</code> or aliasing, leading to
 * race conditions and subtle data corruption. <code>jh::immutable_str</code> eliminates these
 * risks by enforcing immutability both at the type level and memory level.
 * </p>
 *
 * <h3>Comparison with <code>const std::string</code></h3>
 * <table>
 *   <tr><th>Feature</th><th><code>jh::immutable_str</code></th><th><code>const std::string</code></th></tr>
 *   <tr><td>Memory-level immutability</td><td>&#9989; True</td><td>&#10060; False</td></tr>
 *   <tr><td>Thread safety</td><td>&#9989; Safe by design</td><td>&#10060; Mutable buffer</td></tr>
 *   <tr><td>Reallocation risk</td><td>&#10060; None</td><td>&#9989; Possible</td></tr>
 *   <tr><td>Hashing</td><td>&#9989; Cached, thread-safe</td><td>&#10060; Recomputed each time</td></tr>
 *   <tr><td>Storage model</td><td>Compact (unique_ptr)</td><td>Dynamic capacity-managed</td></tr>
 * </table>
 *
 * <h3>Core Features</h3>
 * <ul>
 *   <li><b>Immutable Data:</b> Stored via <code>unique_ptr&lt;const char[]&gt;</code>, preventing mutation.</li>
 *   <li><b>Thread-Safe Hashing:</b> Lazy-evaluated via <code>std::once_flag</code> to ensure safe caching.</li>
 *   <li><b>Auto Trimming:</b> Optional compile-time whitespace removal (controlled by <code>JH_IMMUTABLE_STR_AUTO_TRIM</code>).</li>
 *   <li><b>Shared Ownership:</b> Distributed through <code>jh::atomic_str_ptr</code> (<code>shared_ptr</code> alias).</li>
 *   <li><b>Interop:</b> Compatible with <code>std::string_view</code> and C-string APIs.</li>
 *   <li><b>Custom Hash &amp; Eq:</b> Support for transparent <code>unordered_map</code> lookup via <code>const char*</code>.</li>
 * </ul>
 *
 * <h3>Dual-Mode Header Integration</h3>
 * <p>
 * From <b>v1.3.x</b>, <code>jh::immutable_str</code> supports the <b>Dual-Mode Header</b> system:
 * </p>
 * <ul>
 *   <li>Linked through <code>jh::jh-toolkit</code> → acts as a <b>header-only</b> component.</li>
 *   <li>Linked through <code>jh::jh-toolkit-static</code> → compiled as a <b>static implementation</b>
 *       for performance and deterministic linking.</li>
 *   <li>Mode controlled internally via <code>JH_INTERNAL_SHOULD_DEFINE</code>.</li>
 * </ul>
 *
 * <h4>Static Build Detection</h4>
 * <p>
 * The method <code>bool jh::immutable_str::is_static_built()</code> allows runtime mode verification:
 * </p>
 * <ul>
 *   <li>Returns <b>true</b> if built as part of <code>jh-toolkit-static</code>.</li>
 *   <li>Returns <b>false</b> when using header-only mode via <code>jh-toolkit</code>.</li>
 * </ul>
 *
 * <h3>Usage Example</h3>
 * @code
 * #include &lt;jh/immutable_str&gt;
 * #include &lt;iostream&gt;
 *
 * int main() {
 *     auto pool = jh::pool&lt;jh::immutable_str&gt;();
 *     const auto str = pool.acquire("Hello, JH Toolkit!");
 *     std::cout &lt;&lt; str-&gt;view() &lt;&lt; std::endl;
 *     return 0;
 * }
 * @endcode
 *
 * <p>
 * If the program prints the expected message, it confirms that the correct linkage
 * (header-only or static) is configured properly in your build environment.
 * </p>
 *
 * <h4>Automatic Pool Integration</h4>
 * <p>
 * This header automatically includes <code>jh/pool.h</code>, exposing the full
 * pooling behavior of <code>jh::immutable_str</code> without requiring any
 * additional include directives. Because <code>jh::pool</code> performs
 * duck-typed deduction (detecting <code>hash()</code> and <code>operator==</code>),
 * <code>immutable_str</code> instances are automatically compatible with
 * <code>jh::pool</code>.
 * </p>
 *
 * <p>
 * In addition to providing <code>hash()</code> and <code>operator==</code>,
 * types used with <code>jh::pool</code> must guarantee <b>semantic immutability</b>:
 * once an object is inserted into the pool, its <code>hash()</code> and
 * equality semantics must remain constant for its entire lifetime.
 * This ensures that pooled instances remain stable and deduplicated.
 * </p>
 *
 * <p>
 * <code>jh::immutable_str</code> naturally satisfies this requirement — its
 * memory content and hash are fixed at construction time and can never change.
 * Therefore, it represents the canonical example of a <b>pool-safe immutable type</b>.
 * </p>
 *
 * <p>
 * This means you can directly acquire shared, deduplicated immutable strings:
 * </p>
 *
 * @code
 * #include &lt;jh/immutable_str&gt;
 * #include &lt;cstdio&gt;
 *
 * int main() {
 *     jh::pool&lt;jh::immutable_str&gt; pool;  // automatically available
 *     auto a = pool.acquire("JH Toolkit");
 *     auto b = pool.acquire("JH Toolkit");
 *
 *     // both handles reference the same pooled immutable instance
 *     if (a.get() == b.get()) {
 *         std::puts("deduplicated successfully");
 *     }
 * }
 * @endcode
 *
 * <p>
 * The <code>pool.acquire()</code> call internally checks for an existing equivalent
 * object (by <code>hash()</code> and <code>operator==</code>) and reuses it if found.
 * This guarantees that semantically identical strings always share the same
 * underlying immutable buffer.
 * </p>
 *
 * <ul>
 *   <li><b>Automatic inclusion:</b> <code>jh/pool.h</code> is included by default.</li>
 *   <li><b>Value-based pooling:</b> Identical strings resolve to the same shared instance.</li>
 *   <li><b>Thread-safe:</b> Pool operations are safe since <code>immutable_str</code> itself is immutable.</li>
 *   <li><b>Duck-typed deduction:</b> <code>jh::pool</code> automatically recognizes
 *       compatible types implementing <code>hash()</code> and <code>operator==</code>.</li>
 * </ul>
 *
 * <h3>Performance Notes</h3>
 * <ul>
 *   <li>Immutable buffer — no internal reallocation or mutation.</li>
 *   <li>Constant-time string comparison and hash access after first computation.</li>
 *   <li>Optimized for concurrent, read-dominant workloads.</li>
 *   <li>Minimal memory footprint: pointer + cached hash + length field.</li>
 *   <li><b>Benchmark:</b> In controlled microbenchmarks (LLVM&#64;20, Catch2, 1024× iterations),
 *       <code>jh::immutable_str</code> shows performance essentially identical to
 *       <code>std::string</code> — sometimes slower by about <b>1%</b>,
 *       sometimes faster by up to <b>2%</b>, typically fluctuating within
 *       <b>±2%</b>. This variation is within normal measurement noise.</li>
 * </ul>
 *
 * <h3>See Also</h3>
 * <ul>
 *   <li><code>jh::pool</code> — efficient pooling system compatible with immutable_str.</li>
 *   <li><code>jh::atomic_str_ptr</code> — shared-pointer alias for efficient immutable string sharing.</li>
 * </ul>
 *
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#ifndef JH_IMMUTABLE_STR_AUTO_TRIM
#define JH_IMMUTABLE_STR_AUTO_TRIM true
#endif

#include <algorithm>        // for std::max
#include <memory>           // for std::unique_ptr, std::shared_ptr
#include <unordered_map>    // NOLINT for std::unordered_map
#include <unordered_set>    // NOLINT for std::unordered_set
#include <string>           // for std::string
#include <cstring>          // for ::strnlen
#include <string_view>      // for std::string_view
#include <cstdint>          // for std::uint64_t
#include <optional>         // for std::optional
#include <type_traits>      // for std::remove_cvref_t
#include "jh/synchronous/const_lock.h"
#include "jh/core/pool.h"
#include "jh/pods/string_view.h"

namespace jh::detail {
    constexpr bool is_space_ascii(const unsigned char ch) {
        return ch == ' ' || ch == '\t' || ch == '\n' ||
               ch == '\v' || ch == '\f' || ch == '\r';
    }
}

namespace jh {

    /**
     * @brief Immutable string with optional automatic trimming and thread-safe hash caching.
     *
     * <h4>Overview</h4>
     * <p>
     * <code>immutable_str</code> represents a <b>truly immutable</b> and <b>thread-safe</b> string object.
     * Once constructed, its internal data is fixed in memory and cannot be modified, reallocated, or replaced.
     * It is designed for efficient read-only access in concurrent systems, configuration registries,
     * and shared constant datasets.
     * </p>
     *
     * <h4>Design Goals</h4>
     * <ul>
     *   <li>Guarantee memory-level immutability with zero write access after initialization.</li>
     *   <li>Provide deterministic lifetime ownership using <code>unique_ptr&lt;const char[]&gt;</code>.</li>
     *   <li>Support concurrent reads safely without synchronization overhead.</li>
     *   <li>Enable efficient sharing via <code>std::shared_ptr&lt;immutable_str&gt;</code> (<code>atomic_str_ptr</code>).</li>
     *   <li>Offer consistent hashing and transparent equality for unordered containers.</li>
     * </ul>
     *
     * <h4>Key Features</h4>
     * <ul>
     *   <li><b>True Immutability:</b> Internal buffer is never exposed for modification.</li>
     *   <li><b>Thread-Safe Hashing:</b> Cached on first access using <code>std::once_flag</code>.</li>
     *   <li><b>Optional Auto-Trim:</b> Leading/trailing whitespace removed at construction if enabled.</li>
     *   <li><b>Memory Efficiency:</b> Minimal overhead — stores only a pointer, size, and cached hash.</li>
     *   <li><b>Transparent Lookup:</b> Works directly with <code>const char*</code> keys in hash tables.</li>
     * </ul>
     *
     * <h4>View Access</h4>
     * <p>
     * The class provides several view accessors for interoperability:
     * </p>
     * <ul>
     *   <li><code>c_str()</code> — Returns a <b>null-terminated</b> read-only C-string pointer.</li>
     *   <li><code>view()</code> — Returns a <code>std::string_view</code> to the internal data (no copy).</li>
     *   <li><code>pod_view()</code> — Returns a <code>jh::pod::string_view</code> for POD-style access.</li>
     *   <li><code>str()</code> — Returns a full <code>std::string</code> <b>copy</b> of the immutable buffer.</li>
     * </ul>
     * <p>
     * <b>Note:</b> Only <code>str()</code> performs data copying; other view functions are zero-copy.
     * </p>
     *
     * <h4>Construction Semantics</h4>
     * <ul>
     *   <li>Constructible from <code>const char*</code> or <code>std::string_view</code> (with a lock).</li>
     *   <li>Deleted copy and move semantics to preserve immutability guarantees.</li>
     *   <li>Preferred factory: <code>jh::make_atomic()</code> for shared atomic usage.</li>
     * </ul>
     *
     * <h4>Dual-Mode Header Integration</h4>
     * <ul>
     *   <li>Works under both <b>header-only</b> and <b>static library</b> modes via Dual-Mode Header system.</li>
     *   <li>Build mode internally controlled by <code>JH_INTERNAL_SHOULD_DEFINE</code>.</li>
     *   <li>Runtime detection available via <code>is_static_built()</code>.</li>
     * </ul>
     *
     * <h4>Thread Safety</h4>
     * <ul>
     *   <li>All accessors (<code>c_str()</code>, <code>view()</code>, <code>hash()</code>, etc.) are thread-safe.</li>
     *   <li>No external synchronization required after construction.</li>
     *   <li>Hash is lazily computed once, guarded by <code>std::call_once</code>.</li>
     * </ul>
     *
     * <h4>See Also</h4>
     * <ul>
     *   <li><code>jh::pool</code> — object pool compatible with <code>immutable_str</code>.</li>
     *   <li><code>jh::atomic_str_ptr</code> — alias for <code>std::shared_ptr&lt;immutable_str&gt;</code>.</li>
     * </ul>
     */
    class immutable_str final {
    public:

        /**
         * @brief Constructs an immutable string from a C-string.
         *
         * <p>
         * Creates an immutable copy of the provided <b>null-terminated C-string</b>.
         * The constructor performs an internal <code>strlen()</code> to determine
         * the source length and allocates a new immutable buffer.
         * </p>
         *
         * @param str A null-terminated C-string. May be <code>nullptr</code>, which is treated as an empty string.
         *
         * <p>
         * The caller must ensure that the input pointer remains valid and unmodified during construction.
         * This is only safe when the input is either:
         * </p>
         * <ul>
         *   <li>a string literal, or</li>
         *   <li>a thread-exclusive buffer not accessed concurrently.</li>
         * </ul>
         * Passing a pointer that can be modified by another thread results in undefined behavior.
         *
         * @note
         * <ul>
         *   <li>Marked as <code>explicit</code> to prevent unintended implicit conversions.</li>
         *   <li>Uses <code>strlen()</code> internally to determine the input length.</li>
         *   <li>Trimming behavior depends on <code>immutable_str::auto_trim</code>.</li>
         *   <li>For non-null-terminated or shared buffers, use
         *       <code>immutable_str(std::string_view, std::mutex&amp;)</code> instead.</li>
         *   <li>Designed for safe interoperation with C-style APIs (e.g., LLVM <code>extern "C"</code>).</li>
         * </ul>
         */
        explicit immutable_str(const char *str);

        /**
         * @brief Deleted constructor to prevent unintended conversions.
         *
         * <p>
         * This template overload disables single-argument construction from non-C-string types,
         * ensuring that only <code>const char*</code> inputs are accepted. This avoids
         * unsafe or ambiguous conversions that could lead to undefined behavior.
         * </p>
         *
         * @note
         * <ul>
         *   <li>Rejects non-string pointer inputs (e.g. numeric types, containers, or temporaries).</li>
         *   <li>Ensures that only <code>const char*</code> can be used for direct construction.</li>
         *   <li>For data without a null terminator or requiring lifetime protection,
         *       use <code>immutable_str(std::string_view, std::mutex&amp;)</code> instead.</li>
         * </ul>
         */
        template<typename T>
        explicit immutable_str(T) = delete;

        /**
         * @brief Constructs an immutable string from a <code>std::string_view</code> with mutex protection.
         *
         * <p>
         * Creates an immutable copy of the data referenced by <code>sv</code> while holding
         * the provided mutex. This overload is intended for cases where the source memory
         * may be transient, mutable, or shared between threads.
         * </p>
         *
         * @tparam M Any type satisfying <code>jh::concepts::mutex_like</code> —
         *         such as <code>std::mutex</code>, <code>std::shared_mutex</code>, or custom types.
         *
         * @param sv  A <code>std::string_view</code> representing the source data. It may or may not be null-terminated.
         * @param mtx A reference to a mutex protecting the lifetime of the buffer referenced by <code>sv</code>.
         *
         * @throws std::logic_error If <code>sv</code> contains embedded null (<tt>'\0'</tt>) characters.
         *
         * @note
         * <ul>
         *   <li>Performs a bounded null-character check using <code>::strnlen()</code>
         *       to verify that no embedded nulls exist within <code>sv.size()</code> bytes.</li>
         *   <li>Copies exactly <code>sv.size()</code> bytes into an internal immutable buffer, even if not null-terminated.</li>
         *   <li>The provided <code>mtx</code> <b>must</b> guard the same memory region as <code>sv.data()</code>;
         *       using an unrelated mutex leads to undefined behavior.</li>
         *   <li>Supports both exclusive and shared mutex types, via <code>jh::sync::const_lock</code>.</li>
         *   <li>Recommended for constructing immutable strings from shared or non-terminated data regions.</li>
         *   <li>
         *       <b>Optional optimization:</b><br>
         *       If you are certain that the source data is <em>not shared across threads</em>,
         *       you may explicitly use <code>jh::typed::null_mutex</code> (from <code>&lt;jh/typed&gt;</code>)
         *       as the mutex parameter. It is a zero-cost, concept-compatible dummy mutex,
         *       and all locking operations become no-ops.<br>
         *       <b>Note:</b> This must be explicitly specified; no automatic substitution is performed.
         *   </li>
         * </ul>
         */
        template <jh::concepts::mutex_like M>
        immutable_str(std::string_view sv, M &mtx) {
            jh::sync::const_lock<M> guard(mtx);  // Scope-based lock, auto-detects shared/exclusive

            if (::strnlen(sv.data(), sv.size()) != sv.size()) {
                throw std::logic_error(
                        "jh::immutable_str does not support string views containing embedded null characters.");
            }
            init_from_string(sv.data(), sv.size());
        }

        /**
         * @brief Deleted copy constructor.
         *
         * <p>
         * <code>immutable_str</code> manages its string data through
         * <code>std::unique_ptr&lt;const char[]&gt;</code>, enforcing exclusive ownership.
         * Copy construction would require duplicating the underlying data buffer,
         * which is explicitly disallowed to maintain immutability.
         * </p>
         *
         * @note
         * <ul>
         *   <li>Prevents unintended shallow copies.</li>
         *   <li>Ensures that each instance has a unique, immutable data block.</li>
         * </ul>
         */
        immutable_str(const immutable_str &) = delete;

        /**
         * @brief Deleted copy assignment operator.
         *
         * <p>
         * Copy assignment is disabled to preserve the immutable property of
         * <code>immutable_str</code>. Assigning one immutable instance to another would imply
         * replacing its internal buffer, which contradicts its design.
         * </p>
         *
         * @note
         * <ul>
         *   <li>Prevents mutation through reassignment.</li>
         *   <li>Use <code>std::shared_ptr&lt;jh::immutable_str&gt;</code> if sharing semantics are required.</li>
         * </ul>
         */
        immutable_str &operator=(const immutable_str &) = delete;

        /**
         * @brief Deleted move constructor.
         *
         * <p>
         * Unlike typical movable types, <code>immutable_str</code> forbids move semantics.
         * Moving would transfer ownership of the underlying buffer,
         * violating the immutability principle.
         * </p>
         *
         * @note
         * <ul>
         *   <li>Immutability implies that instances cannot change ownership post-construction.</li>
         *   <li>To share instances safely, use <code>std::shared_ptr&lt;jh::immutable_str&gt;</code>.</li>
         * </ul>
         */
        immutable_str(immutable_str &&) = delete;

        /**
         * @brief Deleted move assignment operator.
         *
         * <p>
         * Move assignment is disabled because <code>immutable_str</code> must remain constant
         * for its entire lifetime. Any form of reassignment or ownership transfer is considered
         * a modification of its internal state.
         * </p>
         *
         * @note
         * <ul>
         *   <li>Enforces strict immutability and thread safety.</li>
         *   <li>Instances should be shared through <code>std::shared_ptr</code>, not reassigned.</li>
         * </ul>
         */
        immutable_str &operator=(immutable_str &&) = delete;

        /**
         * @brief Returns the raw C-style string pointer.
         *
         * <p>
         * Provides direct access to the internal immutable buffer as a
         * <code>const char*</code>. The returned pointer is guaranteed to remain
         * valid for the lifetime of the object.
         * </p>
         *
         * @return A pointer to an immutable, null-terminated character sequence.
         *
         * @note
         * <ul>
         *   <li>The returned pointer must never be modified.</li>
         *   <li>Ownership remains with <code>immutable_str</code>; do not deallocate it.</li>
         *   <li>Safe for concurrent read access from multiple threads.</li>
         * </ul>
         */
        [[nodiscard]] const char *c_str() const noexcept;

        /**
         * @brief Converts the immutable content to a <code>std::string</code>.
         *
         * <p>
         * Creates and returns a <b>copy</b> of the internal immutable data as a
         * <code>std::string</code>. This is the only interface that performs a deep copy,
         * preserving immutability while providing a mutable external representation.
         * </p>
         *
         * @return A <code>std::string</code> containing a copy of the immutable data.
         *
         * @note
         * <ul>
         *   <li>This operation allocates new memory for the returned <code>std::string</code>.</li>
         *   <li>Useful when interoperability with mutable string APIs is required.</li>
         *   <li>Thread-safe; does not modify the internal buffer.</li>
         * </ul>
         */
        [[nodiscard]] std::string str() const;

        /**
         * @brief Returns a lightweight <code>std::string_view</code> to the immutable data.
         *
         * <p>
         * Provides a non-owning view of the internal string data without copying.
         * This is the most efficient accessor for read-only operations and comparison.
         * </p>
         *
         * @return A <code>std::string_view</code> referencing the immutable string data.
         *
         * @note
         * <ul>
         *   <li>The view remains valid for the lifetime of the <code>immutable_str</code> instance.</li>
         *   <li>No memory allocation occurs.</li>
         *   <li>Safe for concurrent reads; not safe if the underlying object is destroyed.</li>
         * </ul>
         */
        [[nodiscard]] std::string_view view() const noexcept;

        /**
         * @brief Returns a <code>jh::pod::string_view</code> representing this immutable string.
         *
         * <p>
         * Provides a POD-compatible, read-only view over the internal buffer.
         * The returned object has the same layout and semantics as
         * <code>jh::pod::string_view</code> — that is,
         * a pair of <code>const char*</code> and <code>uint64_t</code>
         * describing a non-owning range of bytes.
         * </p>
         *
         * @return A <code>jh::pod::string_view</code> referencing the same data as this object.
         *
         * @note
         * <ul>
         *   <li>No memory is copied or allocated.</li>
         *   <li>The view remains valid as long as the originating <code>immutable_str</code> exists.</li>
         *   <li>Useful when POD layout or constexpr hashing is required, for example in
         *       compile-time utilities or deep comparison contexts.</li>
         *   <li>Comparison and hashing behavior are identical to <code>jh::pod::string_view</code>.</li>
         * </ul>
         */
        [[nodiscard]] pod::string_view pod_view() const noexcept;

        /**
         * @brief Returns the length of the immutable string.
         *
         * <p>
         * Provides the total number of characters contained in the string.
         * The length is determined at construction time and remains constant
         * throughout the object's lifetime.
         * </p>
         *
         * @return The number of characters in the string.
         *
         * @note
         * <ul>
         *   <li>The value is fixed after initialization (no dynamic resizing).</li>
         *   <li>Equivalent to <code>view().size()</code>.</li>
         *   <li>Safe for concurrent read access.</li>
         * </ul>
         */
        [[nodiscard]] std::uint64_t size() const noexcept;

        /**
         * @brief Compares two <code>immutable_str</code> instances for equality.
         *
         * <p>
         * Performs a deep, byte-wise comparison of the internal buffers.
         * This operator guarantees that two instances are considered equal
         * only if their contents are identical.
         * </p>
         *
         * @param other Another <code>immutable_str</code> instance to compare with.
         * @return <code>true</code> if both strings contain identical data; otherwise <code>false</code>.
         *
         * @note
         * <ul>
         *   <li>Automatically implies <code>operator!=</code> as its logical negation.</li>
         *   <li>Comparison is content-based, not pointer-based.</li>
         *   <li>Safe for concurrent read operations on both operands.</li>
         * </ul>
         */
        bool operator==(const immutable_str &other) const noexcept;

        /**
         * @brief Computes the cached hash value of the immutable string.
         *
         * <p>
         * Returns a 64-bit hash derived from the string's contents.
         * The computation is performed lazily — the first call initializes
         * the cached value in a thread-safe manner, and all subsequent calls
         * return the stored result without recomputation.
         * </p>
         *
         * @return A 64-bit hash value uniquely representing the string contents.
         *
         * @note
         * <ul>
         *   <li>Lazy evaluation ensures hashing is performed only once per instance.</li>
         *   <li>Thread-safe: guarded internally by <code>std::once_flag</code>.</li>
         *   <li>Equivalent calls always return the same value for the same object.</li>
         * </ul>
         */
        [[nodiscard]] std::uint64_t hash() const noexcept;

        /**
         * @brief Global compile-time flag controlling automatic whitespace trimming.
         *
         * <p>
         * Determines whether all <code>immutable_str</code> instances automatically remove
         * leading and trailing ASCII whitespace during construction.
         * </p>
         *
         * <ul>
         *   <li><code>true</code> (default): Trim leading and trailing whitespace.</li>
         *   <li><code>false</code>: Preserve the original input exactly.</li>
         * </ul>
         *
         * @note
         * <ul>
         *   <li>This is a <b>compile-time constant</b> (set by <code>JH_IMMUTABLE_STR_AUTO_TRIM</code>).</li>
         *   <li>Changing it at runtime has no effect, and redefining the macro globally is not supported.</li>
         *   <li>All instances created within the same translation unit share the same policy.</li>
         *   <li>When deterministic behavior is required across modules, ensure consistent macro definition.</li>
         * </ul>
         */
        static constexpr bool auto_trim = JH_IMMUTABLE_STR_AUTO_TRIM;

        /**
         * @brief Reports whether this header was built in static mode.
         *
         * <p>
         * Indicates if the current <code>immutable_str</code> implementation was compiled
         * as part of the static library target (<code>jh-toolkit-static</code>) or used
         * in header-only mode (<code>jh-toolkit</code>).
         * </p>
         *
         * @return <code>true</code> if compiled as part of <code>jh-toolkit-static</code>;
         *         otherwise <code>false</code>.
         *
         * @note
         * <ul>
         *   <li>This function supports runtime detection for dual-mode header builds.</li>
         *   <li>In header-only builds, it always returns <code>false</code>.</li>
         *   <li>In static library builds, it returns <code>true</code> (macro <code>JH_IS_STATIC_BUILD</code> defined).</li>
         *   <li>Useful for diagnostics or conditional logic depending on linkage mode.</li>
         * </ul>
         */
        [[maybe_unused]] static bool is_static_built();

    private:
        uint64_t size_ = 0;                                       ///< Length of the string
        std::unique_ptr<const char[]> data_;                      ///< Immutable string data
        mutable std::optional<std::uint64_t> hash_{std::nullopt}; ///< Cached hash value
        mutable std::once_flag hash_flag_;                        ///< Ensures thread-safe lazy initialization

        /**
         * @brief Initializes the immutable string from a C-style source.
         *
         * <p>
         * Dispatches to either <code>init_from_string_trim()</code> or
         * <code>init_from_string_no_trim()</code> depending on the compile-time
         * value of <code>JH_IMMUTABLE_STR_AUTO_TRIM</code>.
         * </p>
         *
         * @param input_str A null-terminated C-string. May be <code>nullptr</code> (treated as empty).
         * @param input_len The known length of the string, or <code>-1</code> to auto-detect via <code>strlen()</code>.
         *
         * @note
         * <ul>
         *   <li>This function is always defined inline in the header for dual-mode visibility.</li>
         *   <li>In static builds, both branches are compiled, and the linker resolves the selected path
         *       based on the current macro configuration.</li>
         *   <li>In header-only mode, the unused branch is eliminated by the compiler (dead-code pruning).</li>
         *   <li>Acts as a private dispatcher — not intended for direct user invocation.</li>
         * </ul>
         */
        void init_from_string(const char *input_str, std::uint64_t input_len = static_cast<std::uint64_t>(-1)) {
#if defined(JH_IMMUTABLE_STR_AUTO_TRIM) && JH_IMMUTABLE_STR_AUTO_TRIM
            init_from_string_trim(input_str, input_len);
#else
            init_from_string_no_trim(input_str, input_len);
#endif
        }

        void init_from_string_trim(const char *input_str, std::uint64_t input_len);

        void init_from_string_no_trim(const char *input_str, std::uint64_t input_len);

    };

    /**
     * @brief Alias for an atomically shareable immutable string.
     *
     * <p>
     * Defines a standardized shared ownership model for <code>immutable_str</code>.
     * Although named “atomic”, this alias does not imply hardware-level atomicity;
     * rather, it denotes that <code>atomic_str_ptr</code> can be <b>safely replaced
     * or shared across threads</b> without requiring additional synchronization,
     * thanks to the immutability of the underlying string.
     * </p>
     *
     * @details
     * <ul>
     *   <li>Equivalent to <code>std::shared_ptr&lt;immutable_str&gt;</code>.</li>
     *   <li>Represents an immutable, reference-counted string object
     *       that can be atomically exchanged between threads.</li>
     *   <li>Safe for concurrent read and ownership transfer operations.</li>
     *   <li>Recommended form for distributing immutable strings across
     *       subsystems, caches, or configuration registries.</li>
     *   <li>Fully compatible with <code>atomic_str_hash</code> and
     *       <code>atomic_str_eq</code> for transparent container usage.</li>
     * </ul>
     *
     * @note
     * <ul>
     *   <li>Swapping or assigning <code>atomic_str_ptr</code> instances is
     *       inherently thread-safe due to <code>std::shared_ptr</code> semantics.</li>
     *   <li>No additional locking is required as long as each thread
     *       only replaces or reads entire <code>atomic_str_ptr</code> objects.</li>
     * </ul>
     */
    using atomic_str_ptr = std::shared_ptr<immutable_str>;

    /**
     * @brief Alias for a weak reference to an <code>immutable_str</code>.
     *
     * <p>
     * Provides a non-owning handle to an <code>immutable_str</code> instance
     * managed by a shared pointer. This alias complements
     * <code>atomic_str_ptr</code> for use in cache systems or observer patterns.
     * </p>
     *
     * @details
     * <ul>
     *   <li>Equivalent to <code>std::weak_ptr&lt;immutable_str&gt;</code>.</li>
     *   <li>Prevents reference cycles in shared string registries.</li>
     *   <li>Access the managed instance using <code>lock()</code>.</li>
     *   <li>Marked <code>[[maybe_unused]]</code> to suppress static-build warnings.</li>
     * </ul>
     */
    using weak_str_ptr [[maybe_unused]] = std::weak_ptr<immutable_str>;

    /**
     * @brief Concept for types compatible with <code>jh::immutable_str</code>.
     *
     * <p>
     * Defines the set of types that can safely participate in comparison and hashing
     * operations with <code>immutable_str</code> instances.
     * This allows seamless interoperability between
     * <code>atomic_str_ptr</code>, <code>const char*</code>, and string literals.
     * </p>
     *
     * @details
     * <ul>
     *   <li>Satisfied by <code>atomic_str_ptr</code> (and its cv/ref-qualified variants).</li>
     *   <li>Satisfied by <code>const char*</code>.</li>
     *   <li>Satisfied by string literals such as <code>"hello"</code>
     *       (which decay to <code>const char*</code>).</li>
     *   <li>Used internally in custom hash and equality functors to enable
     *       transparent lookups, e.g. <code>unordered_map::find("key")</code>
     *       without constructing a temporary <code>immutable_str</code>.</li>
     * </ul>
     *
     * @tparam U Candidate type to test for <code>immutable_str</code> compatibility.
     *
     * @note
     * <ul>
     *   <li>Ensures that only pointer-safe, immutable-compatible types are accepted
     *       in hashing and equality operations.</li>
     *   <li>String literals are automatically included through pointer decay.</li>
     * </ul>
     */
    template<typename U>
    concept immutable_str_compatible =
    std::same_as<std::remove_cvref_t<U>, atomic_str_ptr> ||
    std::same_as<std::decay_t<U>, const char *>;

    /**
     * @brief Custom hash functor for <code>atomic_str_ptr</code> and compatible types.
     *
     * <p>
     * Provides transparent, content-based hashing for associative containers
     * involving <code>jh::immutable_str</code> instances. It enables efficient
     * heterogeneous lookup using <code>const char*</code> or string literals,
     * while ensuring hash consistency across all compatible types.
     * </p>
     *
     * @details
     * <ul>
     *   <li>Replaces the default <code>std::shared_ptr</code> hash, which hashes by pointer value,
     *       with a deterministic hash computed from string content.</li>
     *   <li>Supports both <code>atomic_str_ptr</code> and <code>const char*</code> operands,
     *       constrained by <code>immutable_str_compatible</code>.</li>
     *   <li>Transparent lookup is supported — for example, a container of
     *       <code>atomic_str_ptr</code> keys can be queried with
     *       <code>find("key")</code> or <code>contains("key")</code>.</li>
     *   <li>At least one operand (either the stored key or the lookup key)
     *       must represent an actual <code>immutable_str</code> instance.</li>
     *   <li>This contract allows future implementations to freely reorder or
     *       optimize operand evaluation while maintaining semantic equivalence.</li>
     *   <li>When <code>immutable_str::auto_trim</code> is enabled,
     *       leading and trailing ASCII whitespace are ignored in hash computation.</li>
     * </ul>
     *
     * @tparam U
     *   Input type — must satisfy <code>immutable_str_compatible</code>
     *   (<code>atomic_str_ptr</code> or <code>const char*</code>).
     *
     * @param value
     *   Input value to hash; may be <code>nullptr</code> (hash result = 0).
     *
     * @return
     *   64-bit hash derived from string content.
     *
     * @note
     * <ul>
     *   <li>Designed primarily for containers that store
     *       <code>atomic_str_ptr</code> as keys.</li>
     *   <li>Either operand may be a <code>const char*</code> during lookup,
     *       but at least one operand must refer to a valid
     *       <code>immutable_str</code> instance.</li>
     *   <li>Whitespace trimming behavior is compile-time controlled via
     *       <code>immutable_str::auto_trim</code>.</li>
     *   <li>The nested <code>is_transparent</code> typedef enables
     *       heterogeneous lookup in standard unordered containers.</li>
     * </ul>
     */
    struct atomic_str_hash {
        using is_transparent = void; ///< Enables `find(const char*)` in hash-based containers.
        template<typename U>
        requires immutable_str_compatible<U>
        std::uint64_t operator()(const U &value) const noexcept {
            if constexpr (std::same_as<std::remove_cvref_t<U>, atomic_str_ptr>) {
                return value ? value->hash() : 0;
            } else {
                if (value == nullptr) {
                    return 0;
                }
                if constexpr (immutable_str::auto_trim) {
                    const std::uint64_t len = std::strlen(value); // Get `const char*` length
                    std::uint64_t leading = 0, trailing = len;
                    while (leading < len && detail::is_space_ascii(static_cast<unsigned char>(value[leading]))) {
                        ++leading;
                    }
                    while (trailing > leading &&
                           detail::is_space_ascii(static_cast<unsigned char>(value[trailing - 1]))) {
                        --trailing;
                    }
                    return std::hash<std::string_view>{}(std::string_view{value + leading, trailing - leading});
                }
                return std::hash<std::string_view>{}(std::string_view{value});  // NOLINT if !auto_trim
            }
        }
    };

    /**
     * @brief Custom equality functor for <code>atomic_str_ptr</code> and compatible types.
     *
     * <p>
     * Provides content-based comparison for <code>immutable_str</code> instances,
     * enabling heterogeneous lookups in hash-based containers. Unlike the default
     * <code>std::shared_ptr</code> equality operator, which compares raw pointer
     * addresses, this functor compares the underlying string data safely and consistently.
     * </p>
     *
     * @details
     * <ul>
     *   <li>Supports both <code>atomic_str_ptr</code> and <code>const char*</code> operands,
     *       constrained by <code>immutable_str_compatible</code>.</li>
     *   <li>Allows transparent comparison in containers storing
     *       <code>atomic_str_ptr</code> keys — e.g.,
     *       <code>map.find("key")</code> is valid and efficient.</li>
     *   <li>At least one operand (<code>lhs</code> or <code>rhs</code>)
     *       must represent a valid <code>immutable_str</code> instance.</li>
     *   <li>When <code>immutable_str::auto_trim</code> is enabled,
     *       leading and trailing ASCII whitespace are ignored during comparison.</li>
     *   <li>Comparison is symmetric and implementation-agnostic:
     *       either operand may be dereferenced first if future optimizations require it.</li>
     * </ul>
     *
     * @tparam U Type of the left-hand side operand
     *   (<code>atomic_str_ptr</code> or <code>const char*</code>).
     * @tparam V Type of the right-hand side operand
     *   (<code>atomic_str_ptr</code> or <code>const char*</code>).
     *
     * @param lhs Left-hand side value to compare.
     * @param rhs Right-hand side value to compare.
     *
     * @return
     *   <code>true</code> if the strings are equal (after trimming if enabled);
     *   <code>false</code> otherwise or if either side is <code>nullptr</code>.
     *
     * @note
     * <ul>
     *   <li>This functor is designed for containers whose key type is
     *       <code>atomic_str_ptr</code>, but supports lookup using
     *       <code>const char*</code> and string literals.</li>
     *   <li>At least one operand must be an <code>immutable_str</code>
     *       to ensure pointer safety and avoid undefined behavior.</li>
     *   <li>Whitespace trimming behavior is compile-time controlled via
     *       <code>immutable_str::auto_trim</code>.</li>
     *   <li>The nested <code>is_transparent</code> typedef enables
     *       heterogeneous comparison in unordered containers.</li>
     * </ul>
     */
    struct atomic_str_eq {
        using is_transparent = void; ///< Enables `find(const char*)` in hash-based containers.

        template<typename U, typename V>
        requires (immutable_str_compatible<U> && immutable_str_compatible<V>)
        bool operator()(const U &lhs, const V &rhs) const noexcept {
            if constexpr (std::same_as<std::remove_cvref_t<U>, atomic_str_ptr> &&
                          std::same_as<std::remove_cvref_t<V>, atomic_str_ptr>) {
                return lhs && rhs && *lhs == *rhs;
            } else {
                if (lhs == nullptr || rhs == nullptr) return false; // nullptr are considered as not eq
                std::string_view lhs_view, rhs_view;

                if constexpr (std::same_as<std::remove_cvref_t<U>, atomic_str_ptr> &&
                              std::same_as<std::decay_t<V>, const char *>) {
                    lhs_view = lhs->view();
                    if constexpr (immutable_str::auto_trim) {
                        const auto &[leading, size_] = trim(rhs);
                        rhs_view = std::string_view(rhs + leading, size_);
                    } else {
                        rhs_view = std::string_view(rhs);  // NOLINT if !auto_trim
                    }
                } else {
                    rhs_view = rhs->view();
                    if constexpr (immutable_str::auto_trim) {
                        const auto &[leading, size_] = trim(lhs);
                        lhs_view = std::string_view(lhs + leading, size_);
                    } else {
                        lhs_view = std::string_view(lhs);  // NOLINT if !auto_trim
                    }
                }
                return lhs_view == rhs_view;
            }
        }

    private:
        static std::pair<uint64_t, uint64_t> trim(const char *str) noexcept {
            std::uint64_t leading = 0, trailing = std::strlen(str);

            while (leading < trailing && detail::is_space_ascii(static_cast<unsigned char>(str[leading]))) {
                ++leading;
            }
            while (trailing > leading && detail::is_space_ascii(static_cast<unsigned char>(str[trailing - 1]))) {
                --trailing;
            }
            return {leading, trailing - leading};
        }
    };

    template<typename T>
    [[maybe_unused]] atomic_str_ptr make_atomic(T str) = delete;

    /**
     * @brief Creates a shared pointer to an <code>immutable_str</code>.
     *
     * <p>
     * Constructs a new <code>jh::immutable_str</code> instance from a
     * null-terminated C-string and wraps it in a <code>std::shared_ptr</code>.
     * This is the standard factory function for creating atomic, immutable
     * string objects.
     * </p>
     *
     * @param str
     *   Null-terminated C-string to initialize from.
     *
     * @return
     *   Shared pointer (<code>atomic_str_ptr</code>) managing the constructed
     *   <code>immutable_str</code> instance.
     *
     * @details
     * <ul>
     *   <li>Performs a direct construction of <code>immutable_str</code>
     *       without intermediate copies or moves.</li>
     *   <li>The returned object is reference-counted via
     *       <code>std::shared_ptr</code> and can be safely shared
     *       across threads.</li>
     *   <li>Trimming behavior (if enabled) follows
     *       <code>immutable_str::auto_trim</code>.</li>
     * </ul>
     *
     * @note
     * <ul>
     *   <li>Because <code>immutable_str</code> is non-copyable and non-movable,
     *       this factory is the only supported way to allocate it on the heap.</li>
     *   <li>Once constructed, the lifetime of the string is bound to its
     *       <code>std::shared_ptr</code> instance.</li>
     * </ul>
     */
    inline atomic_str_ptr make_atomic(const char *str) {
        return std::make_shared<immutable_str>(str);
    }

    /**
     * @brief Creates a shared pointer to an <code>immutable_str</code> from a locked string view.
     *
     * <p>
     * Constructs a new <code>jh::immutable_str</code> using a
     * <code>std::string_view</code> and an associated mutex-like object
     * that guards the view's lifetime. This ensures thread-safe initialization
     * from potentially mutable or shared buffers.
     * </p>
     *
     * @tparam M
     *   Any type satisfying <code>jh::concepts::mutex_like</code>,
     *   such as <code>std::mutex</code>, <code>std::shared_mutex</code>,
     *   or <code>jh::typed::null_mutex_t</code>.
     *
     * @param sv
     *   String view referencing existing string data.
     * @param mtx
     *   Mutex-like object protecting the lifetime of the structure that owns <code>sv</code>.
     *
     * @return
     *   Shared pointer (<code>atomic_str_ptr</code>) managing the constructed
     *   <code>immutable_str</code> instance.
     *
     * @throws std::logic_error
     *   If <code>sv</code> contains embedded null (<code>'\0'</code>) characters.
     *
     * @warning
     * <ul>
     *   <li>The caller must ensure that <code>mtx</code> correctly protects the
     *       memory region referenced by <code>sv</code>.</li>
     *   <li>Providing an unrelated or unlocked mutex may result in undefined behavior.</li>
     * </ul>
     *
     * @note
     * <ul>
     *   <li>This overload is used when <code>std::string_view</code> refers to
     *       data from temporary, mutable, or shared contexts that require explicit
     *       synchronization.</li>
     *   <li>When the data is guaranteed to be thread-local or immutable,
     *       <code>jh::typed::null_mutex</code> may be used for zero-cost locking.</li>
     *   <li>Because <code>immutable_str</code> cannot be copied or moved,
     *       it must always be constructed via this factory or <code>make_atomic()</code>.</li>
     * </ul>
     */
    template <jh::concepts::mutex_like M>
    inline atomic_str_ptr safe_from(std::string_view sv, M &mtx) {
        return std::make_shared<immutable_str>(sv, mtx);
    }

} // namespace jh


#include "jh/macros/header_begin.h"

namespace jh {
#if JH_INTERNAL_SHOULD_DEFINE

    JH_INLINE immutable_str::immutable_str(const char *str) {
        init_from_string(str);
    }

    JH_INLINE const char *immutable_str::c_str() const noexcept {
        return data_.get();
    }

    JH_INLINE std::string immutable_str::str() const {
        return {data_.get(), size_};
    }

    JH_INLINE std::string_view immutable_str::view() const noexcept {
        return {data_.get(), size_};
    }

    JH_INLINE pod::string_view immutable_str::pod_view() const noexcept {
        return {this->c_str(), this->size()};
    }

    JH_INLINE uint64_t immutable_str::size() const noexcept {
        return size_;
    }

    JH_INLINE bool immutable_str::operator==(const immutable_str &other) const noexcept {
        return std::strcmp(data_.get(), other.data_.get()) == 0;
    }

    JH_INLINE std::uint64_t immutable_str::hash() const noexcept {
        std::call_once(hash_flag_, [this] {
            hash_.emplace(std::hash<std::string_view>{}(std::string_view(data_.get(), size_)));
        });
        return hash_.value();
    }

    JH_INLINE void immutable_str::init_from_string_trim(const char *input_str,
                                                        std::uint64_t input_len) {
        if (!input_str) [[unlikely]] {
            // Initialize an empty string if input is null
            size_ = 0;
            auto data_array_ = std::make_unique<char[]>(1);
            data_array_[0] = '\0';
            data_ = std::move(data_array_);
            return;
        }

        const char *start = input_str;
        if (input_len == static_cast<std::size_t>(-1)) {
            input_len = std::strlen(input_str);  // fallback only if not known
        }

        const char *end = input_str + input_len - 1;

        // If auto_trim is enabled, remove leading and trailing whitespace

        while (*start && detail::is_space_ascii(*start)) ++start;

        // If the input contains only whitespace, treat it as an empty string
        if (*start == '\0') [[unlikely]] {
            size_ = 0;
            auto data_array_ = std::make_unique<char[]>(1);
            data_array_[0] = '\0';
            data_ = std::move(data_array_);
            return;
        }

        while (end > start && detail::is_space_ascii(*end)) --end;


        // Compute the final string size
        size_ = end - start + 1;

        // Allocate memory and copy the string
        auto data_array_ = std::make_unique<char[]>(size_ + 1);
        std::memcpy(data_array_.get(), start, size_);
        data_array_[size_] = '\0';
        data_ = std::move(data_array_);
    }

    JH_INLINE void immutable_str::init_from_string_no_trim(const char *input_str,
                                                           std::uint64_t input_len) {
        if (!input_str) [[unlikely]] {
            // Initialize an empty string if input is null
            size_ = 0;
            auto data_array_ = std::make_unique<char[]>(1);
            data_array_[0] = '\0';
            data_ = std::move(data_array_);
            return;
        }

        const char *start = input_str;
        if (input_len == static_cast<std::size_t>(-1)) {
            input_len = std::strlen(input_str);  // fallback only if not known
        }

        const char *end = input_str + input_len - 1;

        // Compute the final string size
        size_ = end - start + 1;

        // Allocate memory and copy the string
        auto data_array_ = std::make_unique<char[]>(size_ + 1);
        std::memcpy(data_array_.get(), start, size_);
        data_array_[size_] = '\0';
        data_ = std::move(data_array_);
    }

    [[maybe_unused]] JH_INLINE bool immutable_str::is_static_built() {
#ifdef JH_IS_STATIC_BUILD
        return true;
#else
        return false;
#endif // JH_IS_STATIC_BUILD
    }

#endif // JH_INTERNAL_SHOULD_DEFINE
}

#include "jh/macros/header_end.h"