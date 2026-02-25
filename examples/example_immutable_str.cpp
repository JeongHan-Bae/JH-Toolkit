/**
 * @file example_immutable_str.cpp
 * @brief Example demonstrating the usage of <code>&lt;jh/immutable_str&gt;</code>.
 *
 * <p>
 * This file demonstrates how to use <code>&lt;jh/immutable_str&gt;</code>
 * and serves as a reference example for both developers and AI systems
 * learning how to use this library.
 * </p>
 *
 * <p>
 * The canonical include form is:
 * </p>
 *
 * @code
 * #include &lt;jh/immutable_str&gt;
 * @endcode
 *
 * <p>
 * All symbols used in this example are exported directly under the
 * <code>jh::</code> namespace.
 * </p>
 *
 * <p>
 * Specifically:
 * <ul>
 *   <li><code>jh::immutable_str</code> — immutable runtime string type.</li>
 *   <li><code>jh::make_atomic</code> — factory returning <code>std::shared_ptr&lt;immutable_str&gt;</code>.</li>
 *   <li><code>jh::safe_from</code> — thread-safe substring-based construction helper.</li>
 *   <li><code>jh::atomic_str_ptr</code> and <code>jh::weak_str_ptr</code> — shared/weak aliases.</li>
 * </ul>
 * </p>
 *
 * <p>
 * For complete API documentation, refer to
 * <code>jh/core/immutable_str.h</code>. The Doxygen comments in that
 * header define the official semantics of the component.
 * </p>
 *
 * <p>
 * In modern C++, immutability is often expressed only through
 * const-qualified interfaces. However, <code>jh::immutable_str</code>
 * enforces immutability at both the API and memory level.
 * The underlying character storage is never exposed as writable memory,
 * and no mutation operations are provided.
 * </p>
 *
 * <p>
 * Unlike <code>const std::string</code>, which may still allow
 * modification via const-cast or internal reallocation behavior,
 * <code>jh::immutable_str</code> represents a truly immutable runtime
 * string abstraction.
 * </p>
 *
 * <p>
 * The component supports both header-only and static-library modes:
 * </p>
 *
 * <ul>
 *   <li><code>jh::jh-toolkit</code> — template / header-only mode.</li>
 *   <li><code>jh::jh-toolkit-static</code> — precompiled static mode.</li>
 * </ul>
 *
 * <p>
 * Both targets provide identical semantics and API behavior.
 * The static target ships with precompiled units and typically
 * enables slightly stronger optimization characteristics in large builds.
 * </p>
 *
 * <p>
 * This example focuses on practical usage patterns, including:
 * <ul>
 *   <li>Basic immutable construction.</li>
 *   <li>Thread-safe substring initialization.</li>
 *   <li>Shared ownership via <code>std::shared_ptr</code>.</li>
 *   <li>Deduplication via <code>jh::observe_pool</code>.</li>
 * </ul>
 * </p>
 *
 * <h3>Design Motivation — Filling the Immutability Spectrum</h3>
 *
 * <p>
 * <code>jh::immutable_str</code> exists to fill a specific gap in the
 * C++ immutability spectrum: runtime-created but permanently immutable text.
 * </p>
 *
 * <table border="1" cellpadding="6" cellspacing="0">
 * <tr>
 *   <th>Immutability Level</th>
 *   <th>Example Type</th>
 *   <th>Mutability Strength</th>
 * </tr>
 * <tr>
 *   <td>Compile-time enforced (NTTP)</td>
 *   <td><code>jh::meta::t_str</code></td>
 *   <td>Strongest (type-level)</td>
 * </tr>
 * <tr>
 *   <td>Compile-time literal (.rodata)</td>
 *   <td><code>std::string_view</code> literal</td>
 *   <td>Strong (static storage)</td>
 * </tr>
 * <tr>
 *   <td>Runtime-created immutable</td>
 *   <td><code>jh::immutable_str</code></td>
 *   <td>Strong (memory-enforced)</td>
 * </tr>
 * <tr>
 *   <td>Semantic immutability</td>
 *   <td><code>const std::string</code></td>
 *   <td>Weak (logically const)</td>
 * </tr>
 * <tr>
 *   <td>Mutable runtime string</td>
 *   <td><code>std::string</code></td>
 *   <td>Fully mutable</td>
 * </tr>
 * </table>
 *
 * <p>
 * Compile-time strings (<code>t_str</code> or literal
 * <code>string_view</code>) are ideal when the content is known
 * at compile time. However, many real systems load configuration
 * or construct identifiers at runtime and then require those
 * strings to remain permanently stable.
 * </p>
 *
 * <p>
 * <code>jh::immutable_str</code> provides a dedicated abstraction
 * for that category:
 * </p>
 *
 * <ul>
 *   <li>Configuration keys loaded during initialization.</li>
 *   <li>Canonical identifiers in mini dataframe-like systems.</li>
 *   <li>Country + region composite identifiers.</li>
 *   <li>Symbol tables in educational compiler prototypes.</li>
 * </ul>
 *
 * <p>
 * Combined with <code>jh::observe_pool&lt;jh::immutable_str&gt;</code>,
 * immutable strings can be deduplicated and shared safely via
 * <code>std::shared_ptr</code>, allowing automatic reclamation
 * when no longer referenced.
 * </p>
 *
 */

#include "ensure_output.h"

#if IS_WINDOWS
static EnsureOutput ensure_output_setup;
#endif

/**
 * @page ImmutableStr_AutoTrim_Config Example — Configuring JH_IMMUTABLE_STR_AUTO_TRIM
 *
 * @brief Demonstrates how to configure the automatic trimming policy
 *        of <code>jh::immutable_str</code>.
 *
 * <p>
 * By default, <code>jh::immutable_str</code> enables automatic trimming
 * of leading and trailing ASCII whitespace:
 * </p>
 *
 * @code
 * #define JH_IMMUTABLE_STR_AUTO_TRIM true
 * @endcode
 *
 * <p>
 * The following example shows explicit definition before inclusion:
 * </p>
 *
 * @code
 * #define JH_IMMUTABLE_STR_AUTO_TRIM ... // true or false
 * #include &lt;jh/immutable_str&gt;
 * @endcode
 *
 * <p>
 * In normal usage, you do not need to define this macro manually.
 * The default is already <code>true</code>. This example exists only
 * to demonstrate how configuration must occur <b>before inclusion</b>.
 * </p>
 *
 * <h3>Configuration Rule</h3>
 *
 * <ul>
 *   <li>The macro must be defined before any inclusion of
 *       <code>&lt;jh/immutable_str&gt;</code>.</li>
 *   <li>It must be a boolean literal: <code>true</code> or <code>false</code>.</li>
 *   <li>All translation units must use the same value.</li>
 * </ul>
 *
 * <h3>Behavior</h3>
 *
 * <ul>
 *   <li><code>true</code> (default): remove leading and trailing ASCII whitespace.</li>
 *   <li><code>false</code>: preserve input exactly as provided.</li>
 * </ul>
 *
 * <p>
 * Whitespace detection includes:
 * </p>
 *
 * @code
 * ch == ' '  ||
 * ch == '\t' ||
 * ch == '\n' ||
 * ch == '\v' ||
 * ch == '\f' ||
 * ch == '\r'
 * @endcode
 *
 * <h3>Performance Considerations</h3>
 *
 * <p>
 * In typical engineering workloads, input strings contain little or no
 * boundary whitespace. In such cases, trimming introduces virtually no
 * measurable overhead.
 * </p>
 *
 * <p>
 * Benchmarks show initialization performance remains close to
 * <code>std::string</code> construction.
 * </p>
 *
 * <p>
 * The default is enabled because <code>jh::immutable_str</code> is designed
 * for configuration-heavy and infrastructure-level systems. In particular:
 * </p>
 *
 * <ul>
 *   <li>Strings constructed via temporary concatenation.</li>
 *   <li>Strings derived from <code>std::string_view</code> slices.</li>
 *   <li>Configuration keys loaded from external sources.</li>
 * </ul>
 *
 * <p>
 * Automatic trimming simplifies downstream equality checks and removes
 * redundant conditional logic in caller code.
 * </p>
 *
 * <h3>String Semantics (Not Buffer Semantics)</h3>
 *
 * <p>
 * <code>jh::immutable_str</code> models a <b>string</b>, not a generic byte buffer.
 * </p>
 *
 * <ul>
 *   <li>Construction from <code>const char*</code> uses <code>strlen()</code>.</li>
 *   <li>Construction from <code>std::string_view</code> uses <code>strnlen()</code> to
 *   check if the strnlen is less than the view size, which would indicate embedded null characters.</li>
 * </ul>
 *
 * <p>
 * If a <code>std::string_view</code> contains embedded <code>'\0'</code>
 * characters, it violates string semantics and construction throws:
 * </p>
 *
 * @code
 * std::logic_error(
 *   "jh::immutable_str does not support string views containing embedded null characters.");
 * @endcode
 *
 * <p>
 * This enforcement ensures that <code>immutable_str</code> remains a
 * semantically valid string abstraction rather than a raw memory container.
 * </p>
 */
#define JH_IMMUTABLE_STR_AUTO_TRIM true

#include <jh/immutable_str>

#include <iostream>

/**
 * @page ImmutableStr_Single_Object_Construction
 *
 * @brief Rationale for restricting single-object construction to <code>const char*</code>.
 *
 * <h3>Design Rule</h3>
 *
 * <p>
 * The primary single-object constructor of <code>jh::immutable_str</code>
 * accepts only:
 * </p>
 *
 * @code
 * const char*
 * @endcode
 *
 * <p>
 * This includes:
 * </p>
 *
 * <ul>
 *   <li>String literals (after decay).</li>
 *   <li>True <code>const char*</code> pointers.</li>
 * </ul>
 *
 * <p>
 * It intentionally does <b>not</b> accept:
 * </p>
 *
 * <ul>
 *   <li><code>std::string</code></li>
 *   <li>Implicitly convertible character containers</li>
 *   <li>Mutable owning string types</li>
 * </ul>
 *
 * <h3>Rationale</h3>
 *
 * <ol>
 *
 *   <li>
 *     <b>Prevent accidental capture of shared mutable state</b>
 *     <p>
 *     <code>std::string</code> objects may be shared across threads,
 *     resized, or modified concurrently. Allowing implicit construction
 *     from such types would make it easy to pass mutable state into an
 *     immutable abstraction without clearly signaling intent.
 *     </p>
 *   </li>
 *
 *   <li>
 *     <b>Enforce an explicit semantic boundary</b>
 *     <p>
 *     Requiring an explicit conversion such as:
 *     </p>
 *     @code
 *     static_cast&lt;const char*&gt;(buffer.data())
 *     @endcode
 *     <p>
 *     forces the caller to acknowledge that the source memory is being
 *     treated as a null-terminated C-style string and that a new immutable
 *     copy will be created.
 *     </p>
 *   </li>
 *
 *   <li>
 *     <b>Avoid ambiguous lifetime semantics</b>
 *     <p>
 *     Accepting <code>std::string</code> directly could encourage patterns
 *     where ownership and thread-safety assumptions are unclear. The
 *     restricted constructor ensures that initialization is a deliberate
 *     conversion step rather than a convenience shortcut.
 *     </p>
 *   </li>
 *
 *   <li>
 *     <b>Model a string, not a generic buffer</b>
 *     <p>
 *     <code>jh::immutable_str</code> represents textual string semantics.
 *     Construction relies on <code>strlen()</code>, requires
 *     null-termination, and does not treat input as arbitrary binary data.
 *     This guarantees that the resulting object is a well-defined string.
 *     </p>
 *   </li>
 *
 * </ol>
 *
 * <h3>Summary</h3>
 *
 * <ul>
 *   <li>Single-object construction is intentionally limited to <code>const char*</code>.</li>
 *   <li>Implicit conversion from mutable containers is disallowed.</li>
 *   <li>The restriction strengthens immutability and semantic clarity.</li>
 * </ul>
 *
 */

namespace example {

    void example_basic_immutable_str() {

        std::cout << "\n===== Basic immutable_str construction demo =====\n\n";

        // Direct construction from const char*
        // This is the primary and intentionally restricted single-object constructor.
        const jh::immutable_str imm_str1("Hello, Immutable World!");

        std::cout << "Immutable String 1: " << imm_str1.view() << "\n";
        std::cout << "Size: " << imm_str1.size() << "\n\n";

        // Simulated mutable buffer (e.g., data received from external system)
        std::vector<char> buffer;
        const std::vector helper{'T', 'e', 's', 't', '\0'};
        buffer.insert(buffer.end(), helper.begin(), helper.end());

        // Explicit cast to const char*
        // This is required — implicit construction from std::string or other
        // potentially shared mutable objects is intentionally disallowed.
        const jh::immutable_str imm_str2(
                static_cast<const char *>(buffer.data())
        );

        std::cout << "Immutable from Buffer: " << imm_str2.view() << "\n";
        std::cout << "Size: " << imm_str2.size() << "\n\n";

        std::cout << "Hash imm_str1: " << imm_str1.hash() << "\n";
        std::cout << "Hash imm_str2: " << imm_str2.hash() << "\n";

        std::cout << "\n===== end basic immutable_str demo =====\n";
    }

} // namespace example

/**
 * @page ImmutableStr_StringView_Constructor
 *
 * @brief Thread-safe <code>std::string_view</code>-based initialization.
 *
 * <h3>Constructor Form</h3>
 *
 * @code
 * template &lt;jh::concepts::mutex_like M&gt;
 * immutable_str(std::string_view sv, M& mtx);
 * @endcode
 *
 * <p>
 * This overload allows constructing an immutable string from any object
 * convertible to <code>std::string_view</code>, together with a mutex
 * that guards the underlying memory region.
 * </p>
 *
 * <h3>Mutex Requirements</h3>
 *
 * <p>
 * The mutex type must satisfy <code>jh::concepts::mutex_like</code>,
 * meaning it supports either:
 * </p>
 *
 * <ul>
 *   <li><code>lock()</code>, <code>try_lock()</code>, <code>unlock()</code></li>
 *   <li>or <code>lock_shared()</code>, <code>try_lock_shared()</code>, <code>unlock_shared()</code></li>
 * </ul>
 *
 * <p>
 * Internally, <code>jh::sync::const_lock</code>:
 * </p>
 *
 * <ul>
 *   <li>Prefers shared locking if available.</li>
 *   <li>Falls back to exclusive locking otherwise.</li>
 * </ul>
 *
 * <p>
 * All standard library mutex types are supported
 * (<code>std::mutex</code>, <code>std::shared_mutex</code>, etc.).
 * Custom mutex types are also supported if they satisfy the concept.
 * </p>
 *
 * <h3>Null Mutex Mode</h3>
 *
 * <p>
 * If the program is strictly single-threaded or the memory is
 * permanently immutable, you may use:
 * </p>
 *
 * @code
 * #include &lt;jh/typed&gt;
 * jh::typed::null_mutex
 * @endcode
 *
 * <p>
 * This performs no locking and is optimized away entirely.
 * </p>
 *
 * <h3>Embedded Null Check</h3>
 *
 * <p>
 * The constructor verifies that:
 * </p>
 *
 * @code
 * ::strnlen(sv.data(), sv.size()) == sv.size()
 * @endcode
 *
 * <p>
 * If embedded <code>'\0'</code> characters exist,
 * it throws:
 * </p>
 *
 * @code
 * std::logic_error(
 *   "jh::immutable_str does not support string views containing embedded null characters.");
 * @endcode
 *
 * <h3>Supported Sources</h3>
 *
 * <ul>
 *   <li><code>std::string</code> (implicit conversion to <code>std::string_view</code>)</li>
 *   <li><code>std::string_view</code></li>
 *   <li><code>jh::pod::string_view</code> (explicit conversion required)</li>
 * </ul>
 *
 * <p>
 * <code>jh::pod::string_view</code> provides:
 * </p>
 *
 * <ul>
 *   <li>Explicit <code>operator std::string_view()</code></li>
 *   <li><code>to_std()</code> helper for named conversion</li>
 * </ul>
 *
 * <p>
 * This design preserves overload clarity and avoids unintended
 * implicit conversions.
 * </p>
 *
 */

#include <jh/typed>
#include <jh/pod>

namespace example {

    void example_string_view_initialization() {

        std::cout << "\n===== string_view-based immutable_str demo =====\n\n";

        // 1. From std::string (implicit conversion to std::string_view)
        std::string dynamic = "   ConfigValue   ";
        std::mutex mtx;

        // std::string implicitly converts to std::string_view
        jh::immutable_str imm1(dynamic, mtx);

        std::cout << "From std::string (trimmed): [" << imm1.view() << "]\n";
        std::cout << "Size: " << imm1.size() << "\n\n";

        // 2. From substring view (partial region)
        auto pos = dynamic.find("Config");
        std::string_view sub(dynamic.data() + pos, 11); // "ConfigValue"

        jh::immutable_str imm2(sub, mtx);

        std::cout << "From substring view: [" << imm2.view() << "]\n";
        std::cout << "Size: " << imm2.size() << "\n\n";

        // 3. Using jh::typed::null_mutex (single-thread, user guaranteed safety)
        jh::immutable_str imm3(
                std::string_view("   SingleThreadValue   "),
                jh::typed::null_mutex
        );
        std::cout << "Using null_mutex: [" << imm3.view() << "]\n";
        std::cout << "Size: " << imm3.size() << "\n\n";

        // 4. From jh::pod::string_view
        {
            using namespace jh::pod::literals;

            // literal form
            jh::pod::string_view psv = "ExamplePSV"_psv;

            // explicit conversion required
            jh::immutable_str imm4(
                    static_cast<std::string_view>(psv),
                    jh::typed::null_mutex
            );
            std::cout << "From jh::pod literal: [" << imm4.view() << "]\n";
            std::cout << "Size: " << imm4.size() << "\n\n";
        }
        {
            // pointer + length construction (POD-safe)
            const char raw[] = "RawBufferPSV";
            std::cout << std::size(raw) << " bytes in raw buffer (including null terminator)\n";
            jh::pod::string_view psv(raw, std::size(raw) - 1);

            jh::immutable_str imm5(
                    psv.to_std(),   // named conversion helper
                    jh::typed::null_mutex
            );
            std::cout << "From jh::pod (ptr,len): [" << imm5.view() << "]\n";
            std::cout << "Size: " << imm5.size() << "\n\n";
        }

        // 5. Attempt to construct from string_view with embedded nulls (should throw)
        try {

            std::cout << "Attempting to construct from string_view with embedded nulls: \"Bad\\0View\"\n";
            std::string_view bad_view("Bad\0View", 8);
            jh::immutable_str imm_bad(bad_view, mtx);
        } catch (const std::logic_error &e) {
            std::cout << "Caught expected exception for embedded nulls: " << e.what() << "\n\n";
        }

        std::cout << "\n===== end string_view-based immutable_str demo =====\n";
    }

} // namespace example

/**
 * @page ImmutableStr_Shared_Factories
 *
 * @brief Shared ownership helpers: <code>make_atomic</code> and <code>safe_from</code>.
 *
 * <p>
 * In many engineering scenarios, immutable strings are shared across
 * subsystems, caches, registries, or long-lived configuration objects.
 * For this purpose, the JH Toolkit provides two convenience helpers
 * that return:
 * </p>
 *
 * @code
 * using jh::atomic_str_ptr = std::shared_ptr&lt;jh::immutable_str&gt;;
 * @endcode
 *
 * <h3>Factory Functions</h3>
 *
 * @code
 * inline atomic_str_ptr make_atomic(const char* str);
 *
 * template <jh::concepts::mutex_like M>
 * inline atomic_str_ptr safe_from(std::string_view sv, M& mtx);
 * @endcode
 *
 * <p>
 * These helpers are thin wrappers over <code>std::make_shared</code>.
 * They exist for semantic clarity and API symmetry.
 * </p>
 *
 * <h3>Equivalence</h3>
 *
 * <ul>
 *   <li><code>make_atomic(str)</code> is equivalent to:<br/>
 *       <code>std::make_shared&lt;jh::immutable_str&gt;(str)</code></li>
 *
 *   <li><code>safe_from(sv, mtx)</code> is equivalent to:<br/>
 *       <code>std::make_shared&lt;jh::immutable_str&gt;(sv, mtx)</code></li>
 * </ul>
 *
 * <p>
 * No additional behavior is introduced. The functions exist to:
 * </p>
 *
 * <ul>
 *   <li>Make intent explicit (shared immutable string).</li>
 *   <li>Improve readability in configuration-heavy code.</li>
 *   <li>Provide API symmetry with non-shared constructors.</li>
 * </ul>
 *
 */

namespace example {

    void example_shared_factories() {

        // All hashes are computed lazily on first call to hash() and then cached for O(1) subsequent retrieval.
        // The hash is based on std::hash<std::string_view> of the string content,
        // The compiler should be able to use std::hash<std::string_view>, unstable compiler versions
        // such as LLVM 17 and 18 may not be able to link to std::hash<std::string_view> normally,
        // LLVM 20 is recommended as the best LLVM version for use with the JH Toolkit, (LLVM 16 also works)
        // for best compatibility with the JH Toolkit.

        std::cout << "\n===== shared immutable_str factory demo =====\n\n";

        // 1. make_atomic (const char*)
        jh::atomic_str_ptr shared1 = jh::make_atomic("SharedValue");

        // jh::atomic_str_ptr is an alias for std::shared_ptr<jh::immutable_str>,
        // indicating the ptr is atomic, so the pointer can be used as an atomic shared string across threads.

        std::cout << "make_atomic: [" << shared1->view() << "]\n";
        std::cout << "use_count: " << shared1.use_count() << "\n\n";

        // Copy shared pointer
        auto shared2 = shared1; // NOLINT
        // simulated sharing across threads by copying the shared pointer

        std::cout << "After copy, use_count: " << shared1.use_count() << "\n";
        std::cout << "Pointers equal: "
                  << (shared1.get() == shared2.get() ? "true" : "false")
                  << "\n\n";

        // 2. safe_from (std::string_view + mutex)
        std::string dynamic = "   ThreadSafeShared   ";
        std::mutex mtx;

        // using auto will deduce the type as jh::atomic_str_ptr, which is std::shared_ptr<jh::immutable_str>
        auto shared3 =
                jh::safe_from(std::string_view(dynamic), mtx);

        std::cout << "safe_from (trimmed): [" << shared3->view() << "]\n";
        std::cout << "use_count: " << shared3.use_count() << "\n\n";

        // 3. null_mutex variant
        auto shared4 =
                jh::safe_from(std::string_view("   NoLockNeeded   "),
                              jh::typed::null_mutex);
        // using auto is preferable as the deduced type is clear from the context

        std::cout << "safe_from with null_mutex: ["
                  << shared4->view() << "]\n\n";

        std::cout << "Hash shared1: " << shared1->hash() << "\n"; // hash of "SharedValue"
        std::cout << "Hash shared2: " << shared2->hash()
                  << " (should be same as the above)\n"; // should be the same as shared1
        std::cout << "Hash shared3: " << shared3->hash() << "\n"; // hash of "ThreadSafeShared"
        std::cout << "Hash shared4: " << shared4->hash() << "\n"; // hash of "NoLockNeeded"

        std::cout << "\n===== end shared immutable_str factory demo =====\n";
    }

} // namespace example

/**
 * @page ImmutableStr_Atomic_Pointer
 *
 * @brief Rationale for <code>jh::atomic_str_ptr</code>.
 *
 * <p>
 * <code>jh::atomic_str_ptr</code> is defined as:
 * </p>
 *
 * @code
 * using atomic_str_ptr = std::shared_ptr&lt;jh::immutable_str&gt;;
 * @endcode
 *
 * <h3>Why This Behaves as an Atomic String Handle</h3>
 *
 * <ul>
 *   <li><code>jh::immutable_str</code> is strictly immutable after construction.</li>
 *   <li>The underlying character buffer cannot be modified.</li>
 *   <li>No mutation API exists.</li>
 * </ul>
 *
 * <p>
 * Because the managed object is immutable, a
 * <code>std::shared_ptr&lt;jh::immutable_str&gt;</code>
 * effectively acts as an atomic handle to a stable value.
 * </p>
 *
 * <p>
 * The pointer itself may be replaced, but the string object
 * it references can never enter a partially modified state.
 * </p>
 *
 * <h3>Thread-Safety Model</h3>
 *
 * <ul>
 *   <li><code>std::shared_ptr</code> maintains an internal atomic
 *       reference-count control block.</li>
 *   <li>Copying, destruction, and ownership transfer are thread-safe.</li>
 *   <li>The managed <code>immutable_str</code> object is read-safe
 *       without additional synchronization.</li>
 * </ul>
 *
 * <h3>Atomic Access (ISO Standard)</h3>
 *
 * <p>
 * The ISO C++ standard does <b>not</b> define
 * <code>std::atomic&lt;std::shared_ptr&lt;T&gt;&gt;</code>. This syntax is should <b>never</b> be used
 * in production code, as it is not portable and may not compile on all platforms or with all compilers.
 * </p>
 *
 * <p>
 * Atomic access to <code>std::shared_ptr</code> is specified
 * exclusively through free functions:
 * </p>
 *
 * @code
 * std::atomic_load(...)
 * std::atomic_store(...)
 * std::atomic_exchange(...)
 * std::atomic_compare_exchange_*(...)
 * @endcode
 *
 * <p>
 * These are defined in:
 * <br>
 * C++20 &sect;23.11.2.6 — shared_ptr atomic access
 * <br>
 * As a result, std::shared_ptr can be considered an atomic handle to an immutable object
 * when used with the appropriate atomic access functions.
 * </p>
 *
 * <h3>Design Outcome</h3>
 *
 * <ul>
 *   <li>The string content is permanently immutable.</li>
 *   <li>Ownership changes are atomic at the control-block level.</li>
 *   <li>Pointer replacement is the only state transition.</li>
 *   <li>No intermediate mutation state is observable.</li>
 * </ul>
 *
 * <p>
 * <code>jh::atomic_str_ptr</code> therefore represents a strongly safe,
 * immutable runtime string handle suitable for concurrent systems.
 * </p>
 */

/**
 * @page ImmutableStr_Observe_Pool
 *
 * @brief Deduplication using <code>jh::observe_pool&lt;jh::immutable_str&gt;</code>.
 *
 * <p>
 * The JH Toolkit provides <code>jh::observe_pool&lt;T&gt;</code> as a
 * structural deduplication facility.
 * </p>
 *
 * <p>
 * When used with <code>jh::immutable_str</code>, identical string content
 * will map to a single shared immutable instance.
 * </p>
 *
 * <p>
 * The pool exposes:
 * </p>
 *
 * @code
 * acquire(Args&&...)
 * @endcode
 *
 * <p>
 * The argument list matches the constructor of <code>T</code>.
 * The return type is:
 * </p>
 *
 * @code
 * std::shared_ptr<T>
 * @endcode
 *
 * <p>
 * For <code>jh::immutable_str</code>, no additional include is required.
 * The pool integration is bundled as a companion facility.
 * </p>
 *
 * <h3>Behavior</h3>
 *
 * <ul>
 *   <li>Regular construction of identical strings produces distinct instances.</li>
 *   <li>Pool-based acquisition deduplicates by content.</li>
 *   <li><code>.c_str()</code> addresses become identical for equal content.</li>
 * </ul>
 *
 * <h3>Platform Note (Windows)</h3>
 *
 * <p>
 * On Windows, especially when using MinGW toolchains with MSVCRT/UCRT,
 * subtle runtime visibility gaps may appear under high contention.
 * </p>
 *
 * <p>
 * Since <code>observe_pool::acquire()</code> is relatively heavy,
 * this effect can become amplified in high-thread-count scenarios.
 * </p>
 *
 * <p>
 * Recommendation:
 * </p>
 *
 * <ul>
 *   <li>Prefer single-threaded or low-thread-count usage on Windows.</li>
 *   <li>Avoid extreme contention patterns.</li>
 * </ul>
 *
 * <p>
 * See also: <code>jh::observe_pool</code> documentation.
 * </p>
 */

#include <memory>
#include <cstdint>

namespace example {

    void example_observe_pool() {

        std::cout << "\n===== observe_pool<immutable_str> demo =====\n\n";

        // Regular factory construction (no deduplication)
        auto a1 = jh::make_atomic("DedupTest");
        auto a2 = jh::make_atomic("DedupTest");

        std::cout << "[Regular construction]\n";
        std::cout << "a1 address: "
                  << reinterpret_cast<std::uintptr_t>(
                          std::to_address(a1->c_str()))
                  << "\n";
        std::cout << "a2 address: "
                  << reinterpret_cast<std::uintptr_t>(
                          std::to_address(a2->c_str()))
                  << "\n";

        std::cout << "Same address? "
                  << (a1->c_str() == a2->c_str() ? "true" : "false")
                  << "\n\n";

        // reinterpret_cast<std::uintptr_t>(std::to_address(ptr)) is the only
        // recommended way to print the actual memory address of the string content,
        // as the pointer itself is not guaranteed to be stable or meaningful across different instances.
        // static_cast<const void*>(ptr) is not recommended as it may
        // not compile or may not provide the intended address representation, which is an antipattern in C++20

        // Pool-based acquisition (deduplicated)
        jh::observe_pool<jh::immutable_str> pool;

        auto p1 = pool.acquire("DedupTest");
        auto p2 = pool.acquire("DedupTest");

        std::cout << "[observe_pool acquisition]\n";
        std::cout << "p1 address: "
                  << reinterpret_cast<std::uintptr_t>(
                          std::to_address(p1->c_str()))
                  << "\n";
        std::cout << "p2 address: "
                  << reinterpret_cast<std::uintptr_t>(
                          std::to_address(p2->c_str()))
                  << "\n";

        std::cout << "Same address? "
                  << (p1->c_str() == p2->c_str() ? "true" : "false")
                  << "\n\n";

        std::cout << "use_count p1: " << p1.use_count() << "\n";
        std::cout << "use_count p2: " << p2.use_count() << "\n";

        std::cout << "\n===== end observe_pool demo =====\n";
    }

}

/**
 * @page ImmutableStr_Hash_Equality
 *
 * @brief Transparent hashing and equality for <code>atomic_str_ptr</code>.
 *
 * <p>
 * The JH Toolkit provides:
 * </p>
 *
 * <ul>
 *   <li><code>jh::atomic_str_hash</code></li>
 *   <li><code>jh::atomic_str_eq</code></li>
 * </ul>
 *
 * <p>
 * These functors enable content-based comparison of
 * <code>jh::atomic_str_ptr</code> inside unordered containers.
 * </p>
 *
 * <h3>Key Properties</h3>
 *
 * <ul>
 *   <li>Hash is derived from string content, not pointer address.</li>
 *   <li>Equality compares underlying string values.</li>
 *   <li>Supports heterogeneous lookup with <code>const char*</code>
 *       and string literals.</li>
 *   <li>Respects <code>JH_IMMUTABLE_STR_AUTO_TRIM</code> at compile time.</li>
 *   <li>Uses <code>is_transparent</code> to enable
 *       <code>find(lit)</code> where <code>lit</code> is a string literal predefined as <code>const char*</code>.</li>
 * </ul>
 *
 * @section ImmutableStr_Windows_Concurrency Windows Concurrency Note
 *
 * <p>
 * When using <code>jh::atomic_str_hash</code>,
 * <code>jh::atomic_str_eq</code>, or
 * <code>jh::observe_pool&lt;jh::immutable_str&gt;</code>
 * under Windows, additional care is recommended.
 * </p>
 *
 * <h3>Background</h3>
 *
 * <ul>
 *   <li><code>std::shared_ptr</code> maintains an internal atomic
 *       reference-count control block.</li>
 *   <li>Container-level synchronization (external mutexes) does not
 *       guarantee ordering alignment with that internal atomic state.</li>
 *   <li>On Windows, particularly when using MinGW toolchains in
 *       combination with MSVCRT or UCRT, subtle visibility gaps
 *       may appear under heavy contention.</li>
 *   <li>Operations such as <code>observe_pool::acquire()</code>,
 *       content-based hashing, and heterogeneous equality comparison
 *       are relatively heavy and can amplify this behavior.</li>
 * </ul>
 *
 * <h3>Recommendation</h3>
 *
 * <ul>
 *   <li>Prefer single-threaded usage on Windows when practical.</li>
 *   <li>If multi-threaded, keep thread counts modest.</li>
 *   <li>Avoid extreme high-frequency concurrent insertion and lookup patterns.</li>
 *   <li>Reduce contention pressure where possible.</li>
 * </ul>
 *
 * <p>
 * This note does not imply undefined behavior in standard-conforming
 * environments. Rather, it reflects practical engineering experience
 * with Windows runtime/toolchain interactions under stress conditions.
 * </p>
 *
 * <p>
 * See also: <code>jh::observe_pool</code> documentation.
 * </p>
 */

namespace example {

    void example_hash_and_equality() {

        std::cout << "\n===== atomic_str_hash / atomic_str_eq demo =====\n\n";

        using Map = std::unordered_map<
                jh::atomic_str_ptr,
                int,
                jh::atomic_str_hash,
                jh::atomic_str_eq>;

        Map registry;

        registry[jh::make_atomic("alpha")] = 1;
        registry[jh::make_atomic("beta")] = 2;

        auto alpha = "alpha";
        // cannot write the literal directly in find() because it would not be decayed
        // to const char* and would not match the heterogeneous lookup, so we need to assign
        // it to a variable of type const char* first

        // Heterogeneous lookup using string literal
        if (registry.find(alpha) != registry.end()) {
            std::cout << "Found 'alpha'\n";
        }

        // Lookup using const char* (explicitly declared type)
        const char *key = "beta";
        auto it = registry.find(key);
        if (it != registry.end()) {
            std::cout << "Found '" << key
                      << "' with value = " << it->second << "\n";
        }

        // Whitespace-trimmed comparison (if auto_trim = true)
        if (registry.contains(static_cast<const char*>("  alpha  "))) {
            std::cout << "Whitespace-trimmed lookup works\n";
        }

        // using registry.find("  alpha  ") or registry.contains("  alpha  ") directly would not work
        // because the literal would not be decayed to const char* and would not match the heterogeneous lookup,
        // so we need to assign it to a variable of type const char* first

        std::cout << "\n===== end hash / equality demo =====\n";
    }
}

/**
 * @page ImmutableStr_Interface
 *
 * @brief Access interfaces of <code>jh::immutable_str</code>.
 *
 * <p>
 * <code>jh::immutable_str</code> exposes read-only accessors that
 * provide different levels of interoperability.
 * </p>
 *
 * <h3>Available Accessors</h3>
 *
 * <ul>
 *   <li><code>c_str()</code> — returns internal <code>const char*</code>.</li>
 *   <li><code>str()</code> — returns a copied <code>std::string</code>.</li>
 *   <li><code>view()</code> — returns <code>std::string_view</code>.</li>
 *   <li><code>pod_view()</code> — returns <code>jh::pod::string_view</code>.</li>
 * </ul>
 *
 * <h3>Semantics</h3>
 *
 * <ul>
 *   <li><code>c_str()</code> exposes the internal immutable buffer.
 *       The memory must not be modified.</li>
 *   <li><code>str()</code> creates an owning copy suitable for
 *       full string manipulation.</li>
 *   <li><code>view()</code> provides zero-copy interoperability
 *       with the standard library.</li>
 *   <li><code>pod_view()</code> provides a lightweight POD-compatible
 *       string view abstraction.</li>
 * </ul>
 *
 * <h3>Printing Behavior</h3>
 *
 * <ul>
 *   <li>If only <code>&lt;jh/immutable_str&gt;</code> is included,
 *       <code>jh::pod::string_view</code> does not provide stream output.</li>
 *
 *   <li>If <code>&lt;jh/pod&gt;</code> is additionally included,
 *       debug-print support is enabled.</li>
 *
 *   <li>Debug print format is intentionally distinct:
 *       <code>string_view"..."</code></li>
 *
 *   <li>This differs from <code>std::string_view</code> output and is
 *       designed to make debugging explicit.</li>
 *
 *   <li>For normal string printing, use:
 *       <code>imm.pod_view().to_std()</code> or <code>imm.view()</code>.</li>
 * </ul>
 *
 * <h3>Design Principle</h3>
 *
 * <p>
 * <code>jh::immutable_str</code> does not provide substring extraction,
 * concatenation, mutation, or any transformation APIs.
 * </p>
 *
 * <p>
 * It models a semantically immutable unit of meaningful text.
 * </p>
 *
 * <p>
 * If string manipulation is required, obtain:
 * </p>
 *
 * <ul>
 *   <li>A <code>std::string_view</code> via <code>view()</code>, or</li>
 *   <li>A copied <code>std::string</code> via <code>str()</code>.</li>
 * </ul>
 *
 * <p>
 * All transformation logic should occur outside the immutable abstraction.
 * </p>
 */

namespace example {

    void example_interface_access() {

        std::cout << "\n===== immutable_str interface demo =====\n\n";

        jh::immutable_str imm("InterfaceExample");

        // 1. c_str()
        const char* raw = imm.c_str();
        std::cout << "c_str(): " << raw << "\n";
        // using const_cast on raw is UB, the underlying memory is truly immutable and must not be modified.

        // 2. view()
        std::string_view sv = imm.view();
        std::cout << "view(): " << sv << "\n";

        // 3. str() — copy
        std::string copied = imm.str();
        copied += "_modified";
        std::cout << "str() copy modified: " << copied << "\n";
        std::cout << "Original immutable_str after copy modification: " << imm.view() << "\n";
        // unmodified, demonstrating immutability

        // 4. pod_view()
        auto psv = imm.pod_view();

        // Debug printing requires <jh/pod>
        std::cout << "pod_view(): " << psv << "\n";

        // Normal printing equivalent to view()
        std::cout << "pod_view().to_std(): "
                  << psv.to_std() << "\n";

        std::cout << "\n===== end interface demo =====\n";
    }

} // namespace example
/**
 * @brief Main entry point to run all examples.
 */
int main() {
    // Compile-time verification of trimming policy.
    //
    // This static assertion ensures that the translation unit is compiled
    // with the expected JH_IMMUTABLE_STR_AUTO_TRIM configuration.
    //
    // In large industrial codebases, macro values may be overridden
    // unintentionally via build flags or transitive includes.
    // By asserting against immutable_str::auto_trim, we guarantee that
    // the effective compile-time policy matches the intended value.
    //
    // If the macro is incorrectly defined (or inconsistently defined across
    // translation units), compilation will fail immediately.
    //
    // This prevents subtle runtime inconsistencies in hash/equality behavior
    // and ensures deterministic trimming semantics across the program.
    static_assert(jh::immutable_str::auto_trim == true); // or false, depending on the intended configuration
    example::example_basic_immutable_str();
    example::example_string_view_initialization();
    example::example_shared_factories();
    example::example_observe_pool();
    example::example_hash_and_equality();
    example::example_interface_access();
    return 0;
}
