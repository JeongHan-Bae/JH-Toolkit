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
 * @file immutable_str.h
 * @brief Immutable, thread-safe string with optional auto-trimming and dual-mode build support.
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 *
 * <h3>Overview</h3>
 * <p>
 * <code>jh::immutable_str</code> provides a read-only string API in modern C++.
 * The library exposes no mutation operation after construction, so instances
 * support concurrent reads. This makes it useful for concurrent environments,
 * global configuration caches, or static metadata storage.
 * </p>
 *
 * <h4>Key Characteristics</h4>
 * <ul>
 *   <li>No public operation modifies the character data after construction.</li>
 *   <li>Thread-safe by design &mdash; multiple threads can safely share instances.</li>
 *   <li>Optional <b>automatic whitespace trimming</b> during construction.</li>
 *   <li>Compact, zero-reallocation model using <code>unique_ptr&lt;const char[]&gt;</code>.</li>
 *   <li>Transparent hashing and equality for unordered containers.</li>
 *   <li>Seamless integration with <code>std::shared_ptr&lt;immutable_str&lt;&gt;&gt;</code> for safe sharing.</li>
 * </ul>
 *
 * <h3>Motivation</h3>
 * <p>
 * A non-const <code>std::string</code> exposes mutation and may have writable
 * aliases that require coordination across threads. <code>jh::immutable_str</code>
 * exposes only read access to its owned character data after construction.
 * </p>
 *
 * <h3>Comparison with <code>std::string</code></h3>
 * <table>
 *   <tr><th>Feature</th><th><code>jh::immutable_str</code></th><th><code>std::string</code></th></tr>
 *   <tr><td>Public mutation API</td><td>&#10060; None</td><td>&#9989; Available</td></tr>
 *   <tr><td>Concurrent reads</td><td>&#9989; Safe by design</td><td>&#9989; Safe without concurrent mutation</td></tr>
 *   <tr><td>Reallocation risk</td><td>&#10060; None</td><td>&#9989; Possible</td></tr>
 *   <tr><td>Hashing</td><td>&#9989; Cached, thread-safe</td><td>&#10060; Recomputed each time</td></tr>
 *   <tr><td>Storage model</td><td>Compact (unique_ptr)</td><td>Dynamic capacity-managed</td></tr>
 * </table>
 *
 * <h3>Core Features</h3>
 * <ul>
 *   <li><b>Read-only Data:</b> Exposed through const access to an owned character array.</li>
 *   <li><b>Thread-Safe Hashing:</b> Lazy-evaluated via <code>std::once_flag</code> to ensure safe caching.</li>
 *   <li><b>Auto Trimming:</b> Optional compile-time whitespace removal selected by <code>AutoTrim</code>; the macro supplies its default.</li>
 *   <li><b>Shared Ownership:</b> Distributed through copyable <code>jh::shared_str_ptr</code> handles; publish with <code>jh::atomic_str_slot</code>.</li>
 *   <li><b>Interop:</b> Compatible with <code>std::string_view</code> and C-string APIs.</li>
 *   <li><b>Custom Hash &amp; Eq:</b> Support for transparent <code>unordered_map</code> lookup via <code>const char*</code>.</li>
 * </ul>
 *
 * <h3>Dual-Mode Header Integration</h3>
 * <p>
 * <code>jh::immutable_str</code> supports the <b>Dual-Mode Header</b> system:
 * </p>
 * <ul>
 *   <li>Linked through <code>jh::jh-toolkit</code> &rarr; acts as a <b>header-only</b> component.</li>
 *   <li>Linked through <code>jh::jh-toolkit-static</code> &rarr; compiled as a <b>static implementation</b>
 *       for performance and deterministic linking.</li>
 *   <li>Non-template helper definitions use <code>JH_INTERNAL_SHOULD_DEFINE</code>.</li>
 * </ul>
 *
 * <h4>Static Build Detection</h4>
 * <p>
 * The method <code>bool jh::immutable_str&lt;&gt;::is_static_built()</code> allows runtime mode verification:
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
 *     auto pool = jh::observe_pool&lt;jh::immutable_str&lt;&gt;&gt;();
 *     const auto str = pool.acquire("Hello, JH-Toolkit!");
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
 * This header automatically includes <code>jh/core/pool.h</code>, exposing the full
 * pooling behavior of <code>jh::immutable_str</code> without requiring any
 * additional include directives. Because <code>jh::observe_pool</code> performs
 * duck-typed deduction (detecting <code>hash()</code> and <code>operator==</code>),
 * <code>immutable_str</code> instances are automatically compatible with
 * <code>jh::observe_pool</code>.
 * </p>
 *
 * <p>
 * In addition to providing <code>hash()</code> and <code>operator==</code>,
 * types used with <code>jh::observe_pool</code> must guarantee <b>semantic immutability</b>:
 * once an object is inserted into the pool, its <code>hash()</code> and
 * equality semantics must remain constant for its entire lifetime.
 * This ensures that pooled instances remain stable and deduplicated.
 * </p>
 *
 * <p>
 * <code>jh::immutable_str</code> satisfies this requirement through its public API:
 * the owned content does not change after construction, and its content hash
 * is cached on first access.
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
 *     jh::observe_pool&lt;jh::immutable_str&lt;&gt;&gt; pool;  // automatically available
 *     auto a = pool.acquire("JH-Toolkit");
 *     auto b = pool.acquire("JH-Toolkit");
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
 *   <li><b>Duck-typed deduction:</b> <code>jh::observe_pool</code> automatically recognizes
 *       compatible types implementing <code>hash()</code> and <code>operator==</code>.</li>
 * </ul>
 *
 * <h3>Performance Notes</h3>
 * <ul>
 *   <li>Immutable buffer &mdash; no internal reallocation or mutation.</li>
 *   <li>Content comparison scans the string; cached hash access is constant-time after first computation.</li>
 *   <li>Optimized for concurrent, read-dominant workloads.</li>
 *   <li>Each instance stores a pointer, length, cached hash state, and a one-time flag.</li>
 * </ul>
 *
 * <h3>See Also</h3>
 * <ul>
 *   <li><code>jh::observe_pool</code> &mdash; efficient pooling system compatible with immutable_str.</li>
 *   <li><code>jh::atomic_str_slot</code> &mdash; atomic storage for a shared string handle.</li>
 * </ul>
 *
 * @date <pre>2025</pre>
 */

#pragma once

#include "jh/atomic/weak_ptr.h"

/**
 * @brief Controls whether <code>jh::immutable_str</code> performs automatic
 *        leading/trailing ASCII whitespace trimming.
 *
 * <p><b>Default:</b> <code>true</code></p>
 *
 * <p>When no project-wide value is supplied, the header defaults to:</p>
 * <pre>
 * \#define JH_IMMUTABLE_STR_AUTO_TRIM true
 * </pre>
 *
 * <p>
 * Configure this default once for the whole program. Every translation unit
 * that includes <code>&lt;jh/immutable_str&gt;</code>, directly or indirectly,
 * must see the same value before its first inclusion. A source-local
 * <code>\#define</code> is suitable only when that translation unit is the
 * program's sole user of this header.
 * </p>
 *
 * <p><b>Configuration methods:</b></p>
 * <ul>
 *   <li>
 *     Shared project configuration header, included before this header in
 *     every translation unit that uses it:
 *     <pre>
 *     // project_config.h
 *     \#define JH_IMMUTABLE_STR_AUTO_TRIM false
 *     </pre>
 *   </li>
 *   <li>
 *     CMake:
 *     <pre>
 *     add_library(project_config INTERFACE)
 *     target_compile_definitions(project_config INTERFACE JH_IMMUTABLE_STR_AUTO_TRIM=false)
 *     target_link_libraries(every_consumer PRIVATE project_config)
 *     </pre>
 *   </li>
 *   <li>
 *     Compiler flag:
 *     <pre>
 *     -DJH_IMMUTABLE_STR_AUTO_TRIM=false
 *     </pre>
 *   </li>
 * </ul>
 *
 * <p><b>Important:</b></p>
 * <ul>
 *   <li>Must be a boolean literal: <code>true</code> or <code>false</code>
 *       (not <code>1</code>/<code>0</code>).</li>
 *   <li>Do not assign different default values in translation units of one
 *       program. Explicit <code>immutable_str&lt;true&gt;</code> and
 *       <code>immutable_str&lt;false&gt;</code> remain available in either configuration.</li>
 *   <li>Acts as a type-level compile-time policy parameter via
 *       <code>static constexpr bool immutable_str&lt;&gt;::auto_trim</code>.</li>
 *   <li>Branch selection is performed using <code>if constexpr</code>,
 *       not preprocessor elimination.</li>
 *   <li>In the static library target (<code>jh-toolkit-static</code>),
 *       both policy specializations are instantiated.</li>
 * </ul>
 *
 * <p><b>Behavior:</b></p>
 * <ul>
 *   <li><code>true</code>: remove leading/trailing ASCII whitespace at construction.</li>
 *   <li><code>false</code>: preserve input exactly.</li>
 * </ul>
 */
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
#include <cstddef>          // for std::nullptr_t
#include <optional>         // for std::optional
#include <type_traits>      // for std::remove_cvref_t
#include <stdexcept>

#include "jh/synchronous/const_lock.h"
#include "jh/concurrent/observe_pool.h"
#include "jh/pods/pair.h"
#include "jh/pods/string_view.h"

namespace jh::detail {
    constexpr bool is_space_ascii(const unsigned char ch) {
        return ch == ' ' || ch == '\t' || ch == '\n' ||
               ch == '\v' || ch == '\f' || ch == '\r';
    }

    /// @brief Copy a string after removing boundary ASCII whitespace.
    void init_from_string_trim(const char *input_str, std::size_t input_len,
                               std::size_t &size, std::unique_ptr<const char[]> &data);

    /// @brief Copy a string without removing boundary whitespace.
    void init_from_string_no_trim(const char *input_str, std::size_t input_len,
                                  std::size_t &size, std::unique_ptr<const char[]> &data);
} // namespace jh::detail

namespace jh {

    /**
     * @brief Immutable string with a type-level trimming policy and thread-safe hash caching.
     *
     * <h4>Overview</h4>
     * <p>
     * <code>immutable_str</code> exposes an immutable public API and supports concurrent reads.
     * Once constructed, no public operation modifies or replaces its owned data.
     * It is designed for efficient read-only access in concurrent systems, configuration registries,
     * and shared constant datasets.
     * </p>
     *
     * <h4>Design Goals</h4>
     * <ul>
     *   <li>Expose no write operation after initialization.</li>
     *   <li>Provide deterministic lifetime ownership using <code>unique_ptr&lt;const char[]&gt;</code>.</li>
     *   <li>Support concurrent reads safely without synchronization overhead.</li>
     *   <li>Enable efficient sharing via <code>std::shared_ptr&lt;immutable_str&lt;&gt;&gt;</code> (<code>shared_str_ptr</code>).</li>
     *   <li>Offer consistent hashing and transparent equality for unordered containers.</li>
     * </ul>
     *
     * <h4>Key Features</h4>
     * <ul>
     *   <li><b>True Immutability:</b> Internal buffer is never exposed for modification.</li>
     *   <li><b>Thread-Safe Hashing:</b> Cached on first access using <code>std::once_flag</code>.</li>
     *   <li><b>Optional Auto-Trim:</b> Leading/trailing whitespace removed when <code>AutoTrim</code> is true.</li>
     *   <li><b>Memory Efficiency:</b> Minimal overhead &mdash; stores only a pointer, size, and cached hash.</li>
     *   <li><b>Transparent Lookup:</b> Works directly with <code>const char*</code> keys in hash tables.</li>
     * </ul>
     *
     * @tparam AutoTrim Whether construction removes boundary ASCII whitespace. Defaults to the global macro.
     *
     * <h4>View Access</h4>
     * <p>
     * The class provides several view accessors for interoperability:
     * </p>
     * <ul>
     *   <li><code>c_str()</code> &mdash; Returns a <b>null-terminated</b> read-only C-string pointer.</li>
     *   <li><code>view()</code> &mdash; Returns a <code>std::string_view</code> to the internal data (no copy).</li>
     *   <li><code>pod_view()</code> &mdash; Returns a <code>jh::pod::string_view</code> for POD-style access.</li>
     *   <li><code>str()</code> &mdash; Returns a full <code>std::string</code> <b>copy</b> of the immutable buffer.</li>
     * </ul>
     * <p>
     * <b>Note:</b> Only <code>str()</code> performs data copying; other view functions are zero-copy.
     * </p>
     *
     * <h4>Construction Semantics</h4>
     * <ul>
     *   <li>Constructible from <code>const char*</code> or <code>std::string_view</code> (with a lock).</li>
     *   <li>Deleted copy and move semantics to preserve immutability guarantees.</li>
     *   <li>Preferred factory: <code>jh::make_atomic()</code> for shared ownership; use <code>jh::make_atomic_slot()</code> for atomic publication.</li>
     * </ul>
     *
     * <h4>Dual-Mode Header Integration</h4>
     * <ul>
     *   <li>Works under both <b>header-only</b> and <b>static library</b> modes via Dual-Mode Header system.</li>
     *   <li>The two non-template initialization helpers use the dual-mode header controls.</li>
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
     *   <li><code>jh::observe_pool</code> &mdash; object pool compatible with <code>immutable_str</code>.</li>
     *   <li><code>jh::shared_str_ptr</code> &mdash; ordinary shared handle; use <code>jh::atomic_str_slot</code> for atomic storage.</li>
     * </ul>
     */
    template<bool AutoTrim = JH_IMMUTABLE_STR_AUTO_TRIM>
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
         *   <li>Trimming behavior depends on this specialization's <code>AutoTrim</code> value.</li>
         *   <li>For non-null-terminated or shared buffers, use
         *       <code>immutable_str(std::string_view, Mutex&amp;)</code> instead.</li>
         *   <li>Designed for safe interoperation with C-style APIs (e.g., LLVM <code>extern "C"</code>).</li>
         * </ul>
         */
        explicit immutable_str(const char *str);

        /**
         * @brief Constructs an empty immutable string from <code>nullptr</code>.
         */
        explicit immutable_str(std::nullptr_t);

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
         *       use <code>immutable_str(std::string_view, Mutex&amp;)</code> instead.</li>
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
         * @tparam M Any type satisfying <code>jh::concepts::mutex_like</code> &mdash;
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
        template<jh::concepts::mutex_like M>
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
         *   <li>Use <code>std::shared_ptr&lt;jh::immutable_str&lt;&gt;&gt;</code> if sharing semantics are required.</li>
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
         *   <li>To share instances safely, use <code>std::shared_ptr&lt;jh::immutable_str&lt;&gt;&gt;</code>.</li>
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
         * <code>jh::pod::string_view</code> &mdash; that is,
         * a pair of <code>const char*</code> and <code>std::size_t</code>
         * describing a non-owning range of bytes.
         * </p>
         *
         * @return A <code>jh::pod::string_view</code> referencing the same data as this object.
         *
         * @note
         * <ul>
         *   <li>No memory is copied or allocated.</li>
         *   <li>The view remains valid as long as the originating <code>immutable_str</code> exists.</li>
         *   <li>It enables deterministic hashing by allowing <code>jh::pod::string_view</code>
         *       to compute stable hashes via any algorithm provided by <code>jh::meta::c_hash</code>,
         *       unlike the non-deterministic nature of typical runtime hash functions.</li>
         *   <li>The returned view uses <code>jh::pod::string_view</code> operations;
         *       <code>immutable_str::hash()</code> uses <code>std::hash&lt;std::string_view&gt;</code>.</li>
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
        [[nodiscard]] std::size_t size() const noexcept;

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
         * Returns a <code>std::size_t</code> hash derived from the string's contents.
         * The computation is performed lazily &mdash; the first call initializes
         * the cached value in a thread-safe manner, and all subsequent calls
         * return the stored result without recomputation.
         * </p>
         *
         * @return A <code>std::size_t</code> hash value for the string contents.
         *
         * @note
         * <ul>
         *   <li>Lazy evaluation ensures hashing is performed only once per instance.</li>
         *   <li>Thread-safe: guarded internally by <code>std::once_flag</code>.</li>
         *   <li>Equivalent calls always return the same value for the same object.</li>
         * </ul>
         */
        [[nodiscard]] std::size_t hash() const noexcept;

        /**
         * @brief Policy flag selecting whether this specialization performs automatic
         *        leading/trailing whitespace trimming.
         *
         * <p>
         * The template argument selects this value. The project-wide
         * <code>JH_IMMUTABLE_STR_AUTO_TRIM</code> setting supplies only the default
         * template argument; when unset, that default is <code>true</code>.
         * </p>
         *
         * <p>
         * Because <code>auto_trim</code> is an ordinary <code>constexpr bool</code>,
         * it can be inspected in <code>constexpr</code> contexts and used inside concepts,
         * templates, and other compile-time logic&mdash;avoiding the need for preprocessor
         * conditionals (<code>\#if</code> / <code>\#ifdef</code>).
         * </p>
         *
         * <ul>
         *   <li><code>true</code>: This specialization removes leading and trailing ASCII whitespace.</li>
         *   <li><code>false</code>: Whitespace is preserved exactly.</li>
         * </ul>
         *
         * <p>
         * Set the macro through a shared project configuration header or build
         * definitions used by every translation unit that includes this header.
         * A local definition is safe only when one translation unit in the
         * program includes the header. Do not give different translation units
         * different defaults; a per-translation-unit assertion cannot check
         * the configuration of another translation unit.
         * </p>
         */
        static constexpr bool auto_trim = AutoTrim;

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
         *   <li>In static library builds, it returns <code>true</code> when the static target supplies
         *       <code>JH_HEADER_NO_IMPL</code> to consumers.</li>
         *   <li>Useful for diagnostics or conditional logic depending on linkage mode.</li>
         * </ul>
         */
        [[maybe_unused]] static bool is_static_built();

    private:
        std::size_t size_ = 0;                                       ///< Length of the string
        std::unique_ptr<const char[]> data_;                      ///< Immutable string data
        mutable std::optional<std::size_t> hash_{std::nullopt}; ///< Cached hash value
        mutable std::once_flag hash_flag_;                        ///< Ensures thread-safe lazy initialization

        /**
         * @brief Initializes the immutable string from a C-style source.
         *
         * <p>
         * Dispatches to the non-template Trim or No-Trim helper depending on
         * <code>AutoTrim</code>.
         * </p>
         *
         * @param input_str A null-terminated C-string. May be <code>nullptr</code> (treated as empty).
         * @param input_len The known length of the string, or <code>-1</code> to auto-detect via <code>strlen()</code>.
         *
         * @note
         * <ul>
         *   <li>The dispatcher is visible in every translation unit.</li>
         *   <li>In static builds, both non-template helpers are compiled once.</li>
         *   <li>In header-only mode, the unused branch is eliminated by the compiler (dead-code pruning).</li>
         *   <li>Acts as a private dispatcher &mdash; not intended for direct user invocation.</li>
         * </ul>
         */
        void init_from_string(const char *input_str, std::size_t input_len = static_cast<std::size_t>(-1)) {
            if constexpr (AutoTrim) {
                detail::init_from_string_trim(input_str, input_len, size_, data_);
            } else {
                detail::init_from_string_no_trim(input_str, input_len, size_, data_);
            }
        }

    };

    template<bool AutoTrim>
    immutable_str<AutoTrim>::immutable_str(const char *str) {
        init_from_string(str);
    }

    template<bool AutoTrim>
    immutable_str<AutoTrim>::immutable_str(std::nullptr_t) {
        init_from_string(static_cast<const char *>(nullptr));
    }

    template<bool AutoTrim>
    const char *immutable_str<AutoTrim>::c_str() const noexcept {
        return data_.get();
    }

    template<bool AutoTrim>
    std::string immutable_str<AutoTrim>::str() const {
        return {data_.get(), size_};
    }

    template<bool AutoTrim>
    std::string_view immutable_str<AutoTrim>::view() const noexcept {
        return {data_.get(), size_};
    }

    template<bool AutoTrim>
    pod::string_view immutable_str<AutoTrim>::pod_view() const noexcept {
        return {this->c_str(), this->size()};
    }

    template<bool AutoTrim>
    std::size_t immutable_str<AutoTrim>::size() const noexcept {
        return size_;
    }

    template<bool AutoTrim>
    bool immutable_str<AutoTrim>::operator==(const immutable_str<AutoTrim> &other) const noexcept {
        return std::strcmp(data_.get(), other.data_.get()) == 0;
    }

    template<bool AutoTrim>
    std::size_t immutable_str<AutoTrim>::hash() const noexcept {
        std::call_once(hash_flag_, [this] {
            hash_.emplace(std::hash<std::string_view>{}(std::string_view(data_.get(), size_)));
        });
        return hash_.value();
    }

    template<bool AutoTrim>
    [[maybe_unused]] bool immutable_str<AutoTrim>::is_static_built() {
#if defined(JH_HEADER_NO_IMPL) || defined(JH_HEADER_IMPL_BUILD)
        return true;
#else
        return false;
#endif
    }


    /**
     * @brief Copyable shared ownership handle for an immutable string.
     * @tparam AutoTrim Construction policy of the pointed-to string.
     * @note Concurrent mutation of the same handle requires external synchronization.
     *       Use <code>atomic_str_slot</code> for atomic publication.
     */
    template<bool AutoTrim = JH_IMMUTABLE_STR_AUTO_TRIM>
    using shared_str_ptr = std::shared_ptr<immutable_str<AutoTrim>>;

    /**
     * @brief Non-owning observer for an immutable string.
     * @tparam AutoTrim Construction policy of the observed string.
     * @details Alias of <code>std::weak_ptr&lt;immutable_str&lt;&gt;&gt;</code>.
     * Call <code>lock()</code> to obtain a temporary shared handle.
     */
    template<bool AutoTrim = JH_IMMUTABLE_STR_AUTO_TRIM>
    using weak_str_ptr = std::weak_ptr<immutable_str<AutoTrim>>;

    /// @brief Atomically replaceable immutable string handle; load before dereferencing.
    template<bool AutoTrim = JH_IMMUTABLE_STR_AUTO_TRIM>
    using atomic_str_slot = jh::atomic::shared_ptr<immutable_str<AutoTrim>>;

    /// @brief Atomically replaceable weak observer; load before calling lock.
    template<bool AutoTrim = JH_IMMUTABLE_STR_AUTO_TRIM>
    using atomic_weak_str_slot = jh::atomic::weak_ptr<immutable_str<AutoTrim>>;


    /**
     * @brief Concept for types compatible with <code>jh::immutable_str</code>.
     *
     * <p>
     * Defines the set of types that can safely participate in comparison and hashing
     * operations with <code>immutable_str</code> instances.
     * This allows seamless interoperability between
     * <code>shared_str_ptr</code>, <code>const char*</code>, and string literals.
     * </p>
     *
     * @details
     * <ul>
     *   <li>Satisfied by <code>shared_str_ptr</code> (and its cv/ref-qualified variants).</li>
     *   <li>Satisfied by <code>const char*</code>.</li>
     *   <li>Satisfied by string literals such as <code>"hello"</code>
     *       (which decay to <code>const char*</code>).</li>
     *   <li>Used internally in custom hash and equality functors to enable
     *       transparent lookups, e.g. <code>unordered_map::find("key")</code>
     *       without constructing a temporary <code>immutable_str</code>.</li>
     * </ul>
     *
     * @tparam U Candidate type to test for <code>immutable_str</code> compatibility.
     * @tparam AutoTrim Policy of the matching shared string pointer.
     *
     * @note
     * <ul>
     *   <li>Ensures that only pointer-safe, immutable-compatible types are accepted
     *       in hashing and equality operations.</li>
     *   <li>String literals are automatically included through pointer decay.</li>
     * </ul>
     */
    template<typename U, bool AutoTrim = JH_IMMUTABLE_STR_AUTO_TRIM>
    concept immutable_str_compatible =
    std::same_as<std::remove_cvref_t<U>, shared_str_ptr<AutoTrim>> ||
    std::is_convertible_v<const U &, const char *> ||
    std::same_as<std::remove_cvref_t<U>, std::nullptr_t>;

    /**
     * @brief Custom hash functor for a policy-matched <code>shared_str_ptr</code> and C-strings.
     * @tparam AutoTrim Policy used to normalize C-string lookup values.
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
     *       with a hash computed from string content.</li>
     *   <li>Supports both <code>shared_str_ptr</code> and <code>const char*</code> operands,
     *       constrained by <code>immutable_str_compatible</code>.</li>
     *   <li>Transparent lookup is supported &mdash; for example, a container of
     *       <code>shared_str_ptr</code> keys can be queried with
     *       <code>find("key")</code> or <code>contains("key")</code>.</li>
     *   <li>At least one operand (either the stored key or the lookup key)
     *       must represent an actual <code>immutable_str</code> instance.</li>
     *   <li>This contract allows future implementations to freely reorder or
     *       optimize operand evaluation while maintaining semantic equivalence.</li>
     *   <li>When <code>AutoTrim</code> is true,
     *       leading and trailing ASCII whitespace are ignored in hash computation.</li>
     * </ul>
     *
     * @tparam U
     *   Input type &mdash; must satisfy <code>immutable_str_compatible</code>
     *   (<code>shared_str_ptr</code> or <code>const char*</code>).
     *
     * @param value
     *   Input value to hash; may be <code>nullptr</code> (hash result = 0).
     *
     * @return
     *   <code>std::size_t</code> hash derived from string content.
     *
     * @note
     * <ul>
     *   <li>Designed primarily for containers that store
     *       <code>shared_str_ptr</code> as keys.</li>
     *   <li>Either operand may be a <code>const char*</code> during lookup,
     *       but at least one operand must refer to a valid
     *       <code>immutable_str</code> instance.</li>
     *   <li>Whitespace trimming behavior is compile-time controlled by
     *       <code>AutoTrim</code>.</li>
     *   <li>The nested <code>is_transparent</code> typedef enables
     *       heterogeneous lookup in standard unordered containers.</li>
     * </ul>
     */
    template<bool AutoTrim = JH_IMMUTABLE_STR_AUTO_TRIM>
    struct atomic_str_hash final {
        /// @brief Enables <code>find(const char*)</code> or <code>find(const char (&lit)[N])</code> in hash-based containers.
        using is_transparent [[maybe_unused]] = void;

        /// @brief Hashes the supplied value for immutable string lookup.
        template<typename U>
        requires immutable_str_compatible<U, AutoTrim>
        std::size_t operator()(const U &value) const noexcept {
            if constexpr (std::same_as<std::remove_cvref_t<U>, shared_str_ptr<AutoTrim>>) {
                return value ? value->hash() : 0;
            } else if constexpr (std::same_as<std::remove_cvref_t<U>, std::nullptr_t>) {
                return 0;
            } else {
                const char *str = value;
                if (str == nullptr) {
                    return 0;
                }
                if constexpr (AutoTrim) {
                    const std::size_t len = std::strlen(str); // Get const char* length
                    std::size_t leading = 0, trailing = len;
                    while (leading < len && detail::is_space_ascii(static_cast<unsigned char>(str[leading]))) {
                        ++leading;
                    }
                    while (trailing > leading &&
                           detail::is_space_ascii(static_cast<unsigned char>(str[trailing - 1]))) {
                        --trailing;
                    }
                    return std::hash<std::string_view>{}(std::string_view{str + leading, trailing - leading});
                }
                return std::hash<std::string_view>{}(std::string_view{str});  // NOLINT if !auto_trim
            }
        }
    };

    /**
     * @brief Custom equality functor for a policy-matched <code>shared_str_ptr</code> and C-strings.
     * @tparam AutoTrim Policy used to normalize C-string lookup values.
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
     *   <li>Supports both <code>shared_str_ptr</code> and <code>const char*</code> operands,
     *       constrained by <code>immutable_str_compatible</code>.</li>
     *   <li>Allows transparent comparison in containers storing
     *       <code>shared_str_ptr</code> keys &mdash; e.g.,
     *       <code>map.find("key")</code> is valid and efficient.</li>
     *   <li>At least one operand (<code>lhs</code> or <code>rhs</code>)
     *       must represent a valid <code>immutable_str</code> instance.</li>
     *   <li>When <code>AutoTrim</code> is true,
     *       leading and trailing ASCII whitespace are ignored during comparison.</li>
     *   <li>Comparison is symmetric and implementation-agnostic:
     *       either operand may be dereferenced first if future optimizations require it.</li>
     * </ul>
     *
     * @tparam U Type of the left-hand side operand
     *   (<code>shared_str_ptr</code> or <code>const char*</code>).
     * @tparam V Type of the right-hand side operand
     *   (<code>shared_str_ptr</code> or <code>const char*</code>).
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
     *       <code>shared_str_ptr</code>, but supports lookup using
     *       <code>const char*</code> and string literals.</li>
     *   <li>At least one operand must be an <code>immutable_str</code>
     *       to ensure pointer safety and avoid undefined behavior.</li>
     *   <li>Whitespace trimming behavior is compile-time controlled by
     *       <code>AutoTrim</code>.</li>
     *   <li>The nested <code>is_transparent</code> typedef enables
     *       heterogeneous comparison in unordered containers.</li>
     * </ul>
     */
    template<bool AutoTrim = JH_IMMUTABLE_STR_AUTO_TRIM>
    struct atomic_str_eq final {
        /// @brief Enables <code>find(const char*)</code> or <code>find(const char (&lit)[N])</code> in hash-based containers.
        using is_transparent [[maybe_unused]] = void;

        /// @brief Compares two values for immutable string equality.
        template<typename U, typename V>
        requires (immutable_str_compatible<U, AutoTrim> && immutable_str_compatible<V, AutoTrim> &&
                  (std::same_as<std::remove_cvref_t<U>, shared_str_ptr<AutoTrim>> ||
                   std::same_as<std::remove_cvref_t<V>, shared_str_ptr<AutoTrim>>))
        bool operator()(const U &lhs, const V &rhs) const noexcept {
            if constexpr (std::same_as<std::remove_cvref_t<U>, std::nullptr_t> ||
                          std::same_as<std::remove_cvref_t<V>, std::nullptr_t>) {
                return false;
            } else if constexpr (std::same_as<std::remove_cvref_t<U>, shared_str_ptr<AutoTrim>> &&
                                 std::same_as<std::remove_cvref_t<V>, shared_str_ptr<AutoTrim>>) {
                return lhs && rhs && *lhs == *rhs;
            } else {
                if (lhs == nullptr || rhs == nullptr) return false;
                if constexpr (std::same_as<std::remove_cvref_t<U>, shared_str_ptr<AutoTrim>>) {
                    if constexpr (AutoTrim) {
                        const auto [leading, size] = trim(rhs);
                        return lhs->view() == std::string_view{rhs + leading, size};
                    } else {
                        return lhs->view() == std::string_view{rhs};
                    }
                } else {
                    if constexpr (AutoTrim) {
                        const auto [leading, size] = trim(lhs);
                        return std::string_view{lhs + leading, size} == rhs->view();
                    } else {
                        return std::string_view{lhs} == rhs->view();
                    }
                }
            }
        }

    private:
        static pod::pair<std::size_t, std::size_t> trim(const char *str) noexcept {
            std::size_t leading = 0, trailing = std::strlen(str);

            while (leading < trailing && detail::is_space_ascii(static_cast<unsigned char>(str[leading]))) {
                ++leading;
            }
            while (trailing > leading && detail::is_space_ascii(static_cast<unsigned char>(str[trailing - 1]))) {
                --trailing;
            }
            return {leading, trailing - leading};
        }
    };

    template<bool AutoTrim = JH_IMMUTABLE_STR_AUTO_TRIM, typename T>
    [[maybe_unused]] shared_str_ptr<AutoTrim> make_atomic(T str) = delete;

    /**
     * @brief Creates a shared pointer to an <code>immutable_str</code>.
     * @param str Null-terminated C-string to copy.
     * @return A copyable <code>shared_str_ptr</code> handle.
     * @tparam AutoTrim Construction policy; defaults to the global macro.
     */
    template<bool AutoTrim = JH_IMMUTABLE_STR_AUTO_TRIM>
    inline shared_str_ptr<AutoTrim> make_atomic(const char *str) {
        return std::make_shared<immutable_str<AutoTrim>>(str);
    }

    /// @brief Reject mutable C-string pointers and user-defined C-string conversions.
    template<bool AutoTrim = JH_IMMUTABLE_STR_AUTO_TRIM, typename T>
    requires (!std::same_as<std::decay_t<T>, const char *> &&
              !std::same_as<std::decay_t<T>, std::nullptr_t>)
    atomic_str_slot<AutoTrim> make_atomic_slot(T) = delete;

    /**
     * @brief Create an atomic slot containing a new immutable string.
     * @tparam AutoTrim Construction policy; defaults to the global macro.
     * @param str Null-terminated string to copy.
     * @return A new atomic shared pointer slot.
     */
    template<bool AutoTrim = JH_IMMUTABLE_STR_AUTO_TRIM>
    inline atomic_str_slot<AutoTrim> make_atomic_slot(const char *str) {
        return atomic_str_slot<AutoTrim>(make_atomic<AutoTrim>(str));
    }

    /**
     * @brief Creates a shared pointer to an <code>immutable_str</code> from a locked string view.
     *
     * <p>
     * Constructs a new <code>jh::immutable_str</code> using a
     * <code>std::string_view</code> and an associated mutex-like object
     * that guards the source during the copy. The caller must keep the view
     * valid while the lock is being acquired.
     * </p>
     *
     * @tparam AutoTrim Construction policy; defaults to the global macro.
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
     *   Shared pointer (<code>shared_str_ptr</code>) managing the constructed
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
     * </ul>
     */
    template<bool AutoTrim = JH_IMMUTABLE_STR_AUTO_TRIM, jh::concepts::mutex_like M>
    inline shared_str_ptr<AutoTrim> safe_from(std::string_view sv, M &mtx) {
        return std::make_shared<immutable_str<AutoTrim>>(sv, mtx);
    }

    /**
     * @brief Create an atomic slot from a string view protected by a mutex-like object.
     * @tparam AutoTrim Construction policy; defaults to the global macro.
     * @tparam M Mutex-like type.
     * @param sv String view to copy.
     * @param mtx Lock protecting the view during construction.
     * @return A new atomic shared pointer slot.
     */
    template<bool AutoTrim = JH_IMMUTABLE_STR_AUTO_TRIM, jh::concepts::mutex_like M>
    inline atomic_str_slot<AutoTrim> safe_from_slot(std::string_view sv, M &mtx) {
        return atomic_str_slot<AutoTrim>(safe_from<AutoTrim>(sv, mtx));
    }

} // namespace jh


#include "jh/macros/header_begin.h"

#if JH_INTERNAL_SHOULD_DEFINE

namespace jh::detail {
    JH_INLINE void init_from_string_trim(const char *input_str, std::size_t input_len,
                                         std::size_t &size, std::unique_ptr<const char[]> &data) {
        if (!input_str) [[unlikely]] {
            // Initialize an empty string if input is null
            size = 0;
            auto data_array_ = std::make_unique<char[]>(1);
            data_array_[0] = '\0';
            data = std::move(data_array_);
            return;
        }

        const char *start = input_str;
        if (input_len == static_cast<std::size_t>(-1)) {
            input_len = std::strlen(input_str);  // fallback only if not known
        }

        const char *end = input_str + input_len - 1;

        // If auto_trim is enabled, remove leading and trailing whitespace

        while (*start && is_space_ascii(*start)) ++start;

        // If the input contains only whitespace, treat it as an empty string
        if (*start == '\0') [[unlikely]] {
            size = 0;
            auto data_array_ = std::make_unique<char[]>(1);
            data_array_[0] = '\0';
            data = std::move(data_array_);
            return;
        }

        while (end > start && is_space_ascii(*end)) --end;

        // Compute the final string size
        size = end - start + 1;

        // Allocate memory and copy the string
        auto data_array_ = std::make_unique<char[]>(size + 1);
        std::memcpy(data_array_.get(), start, size);
        data_array_[size] = '\0';
        data = std::move(data_array_);
    }

    JH_INLINE void init_from_string_no_trim(const char *input_str, std::size_t input_len,
                                            std::size_t &size, std::unique_ptr<const char[]> &data) {
        if (!input_str) [[unlikely]] {
            // Initialize an empty string if input is null
            size = 0;
            auto data_array_ = std::make_unique<char[]>(1);
            data_array_[0] = '\0';
            data = std::move(data_array_);
            return;
        }

        const char *start = input_str;
        if (input_len == static_cast<std::size_t>(-1)) {
            input_len = std::strlen(input_str);  // fallback only if not known
        }

        const char *end = input_str + input_len - 1;

        // Compute the final string size
        size = end - start + 1;

        // Allocate memory and copy the string
        auto data_array_ = std::make_unique<char[]>(size + 1);
        std::memcpy(data_array_.get(), start, size);
        data_array_[size] = '\0';
        data = std::move(data_array_);
    }

} // namespace jh::detail

#endif // JH_INTERNAL_SHOULD_DEFINE

#include "jh/macros/header_end.h"
