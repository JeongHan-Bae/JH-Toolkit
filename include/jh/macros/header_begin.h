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
 * @file header_begin.h
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 * @brief Macro setup for dual-mode headers (header-only / static).
 *
 * @attention This header is intended for multiple inclusions.
 * Do NOT use include guards or <code>#pragma once</code>.
 *
 * <h3>Overview</h3>
 * <p>
 * This header initializes macros that control whether a header emits
 * inline definitions (header-only mode) or only declarations
 * (linked static/shared mode).
 * It allows one unified <code>.h</code> file to serve both build styles.
 * </p>
 *
 * <h3>Build Modes</h3>
 * <table>
 *   <tr>
 *     <th>Macro</th>
 *     <th>Definition Behavior</th>
 *     <th>Description</th></tr>
 *   <tr>
 *     <td><code>(none)</code></td>
 *     <td>Emit inline definitions</td>
 *     <td>Default header-only mode.</td>
 *   </tr>
 *   <tr>
 *     <td><code>JH_HEADER_IMPL_BUILD</code></td>
 *     <td>Emit non-inline definitions</td>
 *     <td>Used by the single implementation TU.</td>
 *   </tr>
 *   <tr>
 *     <td><code>JH_HEADER_NO_IMPL</code></td>
 *     <td>Suppress definitions (declarations only)</td>
 *     <td>Used when linking prebuilt objects.</td>
 *   </tr>
 * </table>
 *
 * <h3>Pseudocode Layout</h3>
 * @code
 * #pragma once
 * #include &lt;jh/macros/header_begin.h&gt;
 *
 * // === Declarations ===
 * declare_classes_or_functions();
 *
 * // === Implementations ===
 * // 1. Must-visible functions (depend on user macros)
 * must_be_visible_implementation();  // placed outside conditional
 *
 * // 2. Conditionally defined implementations
 * #if JH_INTERNAL_SHOULD_DEFINE
 *     JH_INLINE normal_or_inline_implementation();
 * #endif  // JH_INTERNAL_SHOULD_DEFINE
 *
 * #include &lt;jh/macros/header_end.h&gt;
 * @endcode
 *
 * <h3>Visibility Rules</h3>
 * <ul>
 *   <li><b>Functions whose behavior depends on caller-side compile-time
 *       macros</b> (e.g. platform, feature toggles, template specializations)
 *       <b>must remain visible</b> in every translation unit &mdash; therefore they
 *       belong <b>outside</b> the
 *       <code>#if&nbsp;JH_INTERNAL_SHOULD_DEFINE</code> block.</li>
 *   <li>All other normal definitions should be wrapped inside
 *       <code>#if&nbsp;JH_INTERNAL_SHOULD_DEFINE</code> to avoid redefinition
 *       when linked from a static or shared object.</li>
 *   <li>Always finish the header with
 *       <code>#include&nbsp;&lt;jh/macros/header_end.h&gt;</code>.</li>
 * </ul>
 *
 * <hr/>
 * <h3>Usage Guide: Using Dual-Mode Headers</h3>
 *
 * <p>
 * Once you have written a header using
 * <code>&lt;jh/macros/header_begin.h&gt;</code> and
 * <code>&lt;jh/macros/header_end.h&gt;</code>,
 * you can control how that header behaves in each translation unit (TU)
 * by defining specific macros <em>before</em> including it.
 * </p>
 *
 * <h4>Basic Inclusion Modes</h4>
 * <ul>
 *   <li><strong>Header-only (default)</strong><br/>
 *       <code>#include "example.h"</code><br/>
 *       Emits <tt>inline</tt> definitions. Safe for multiple inclusion.
 *       Ideal for direct inclusion without linking any static library.</li>
 *
 *   <li><strong>Implementation TU (static/shared build)</strong><br/>
 *       <code>#define JH_HEADER_IMPL_BUILD</code><br/>
 *       <code>#include "example.h"</code><br/>
 *       Emits <em>non-inline</em> strong definitions.
 *       Only one TU should use this mode.</li>
 *
 *   <li><strong>Interface-only (no definition)</strong><br/>
 *       <code>#define JH_HEADER_NO_IMPL</code><br/>
 *       <code>#include "example.h"</code><br/>
 *       <code>#undef JH_HEADER_NO_IMPL</code><br/>
 *       Suppresses all definitions and exposes only declarations.
 *       Useful when only type visibility is needed.</li>
 * </ul>
 *
 * <h4>Dependency Example</h4>
 * <p>
 * Suppose <code>other.h</code> depends on <code>example.h</code>.
 * When you want to strongly instantiate <code>other</code> but not
 * <code>example</code> in the same TU:
 * </p>
 * @code
 * #include "example.h"          // header-only (inline) mode
 * #define JH_HEADER_IMPL_BUILD
 * #include "other.h"            // strong definition for other
 * @endcode
 *
 * <p>
 * Because <code>example.h</code> has already been included once,
 * its definitions are fixed for this TU. Any further includes
 * (directly or indirectly) are guarded and will not re-instantiate.
 * </p>
 *
 * <h4>Special Case (not recommended)</h4>
 * <p>
 * If you need to build <code>other</code> but not include any
 * <code>example</code> implementation at all:
 * </p>
 * @code
 * #define JH_HEADER_NO_IMPL
 * #include "example.h"
 * #undef  JH_HEADER_NO_IMPL
 * #define JH_HEADER_IMPL_BUILD
 * #include "other.h"
 * @endcode
 * <p>
 * This is valid only if another TU already provides
 * the strong definition of <code>example</code>.
 * </p>
 *
 * <h4>Key Principle</h4>
 * <ul>
 *   <li>The first inclusion of a dual-mode header within a TU
 *       <strong>locks its behavior</strong>.</li>
 *   <li>Later inclusions (even indirect ones) reuse that locked mode
 *       because of header guards or <tt>#pragma once</tt>.</li>
 *   <li>This allows fine-grained per-TU control without breaking
 *       the One Definition Rule (ODR).</li>
 * </ul>
 *
 * <h4>Summary Table</h4>
 * <table>
 *   <tr><th>Mode</th><th>Macro</th><th>Effect</th><th>Typical Use</th></tr>
 *   <tr><td>Header-only</td><td><code>(none)</code></td>
 *       <td>Emit <tt>inline</tt> definitions</td>
 *       <td>Direct inclusion</td></tr>
 *   <tr><td>Static/Shared</td><td><code>JH_HEADER_IMPL_BUILD</code></td>
 *       <td>Emit non-inline strong definitions</td>
 *       <td>Library implementation TU</td></tr>
 *   <tr><td>Interface-only</td><td><code>JH_HEADER_NO_IMPL</code></td>
 *       <td>Suppress definitions (declarations only)</td>
 *       <td>Type visibility only</td></tr>
 * </table>
 *
 * <h4>Conceptual Summary</h4>
 * <ul>
 *   <li>Each TU determines how a header behaves at compile time.</li>
 *   <li>The first inclusion defines the mode for that TU.</li>
 *   <li>Multiple TUs can coexist using different modes safely.</li>
 *   <li>Macros are automatically cleaned by
 *       <code>&lt;jh/macros/header_end.h&gt;</code>,
 *       ensuring no cross-header contamination.</li>
 * </ul>
 *
 * <p><em>
 * With this pattern, a single C++ header can flexibly serve as
 * both a header-only and a static/shared library implementation &mdash;
 * eliminating the traditional need to maintain duplicate
 * <code>.h</code> / <code>.cpp</code> files.
 * </em></p>
 *
 * <hr/>
 * <h3>Integration with CMake and Library Packaging</h3>
 *
 * <p>
 * Dual-mode headers are designed for both in-library use and external reuse
 * through CMake targets. This section explains how to apply them in practice.
 * </p>
 *
 * <ol>
 *   <li>
 *     <strong>Inside your own library build</strong><br/>
 *     Define <code>JH_HEADER_IMPL_BUILD</code> before including each dual-mode
 *     header in the single implementation translation unit (TU).
 *     This causes strong, non-inline definitions to be generated and linked
 *     into the static or shared library.<br/>
 *     Other headers or source files in the same project can simply include
 *     those headers normally (header-only mode); header guards prevent
 *     redefinition.
 *   </li>
 *
 *   <li>
 *     <strong>When providing your library to external users</strong><br/>
 *     In your CMake packaging, propagate
 *     <code>JH_HEADER_NO_IMPL</code> via
 *     <code>target_compile_definitions(... INTERFACE JH_HEADER_NO_IMPL)</code>.
 *     This ensures that all consumers automatically include headers in
 *     declaration-only mode when they link your exported target.<br/>
 *     Consumers do not need to define any macros manually &mdash; simply linking to
 *     the correct target selects the right mode.
 *   </li>
 *
 *   <li>
 *     <strong>Example CMake configuration</strong><br/>
 *     <pre><code>
 *     add_library(jh-toolkit INTERFACE ${INCLUDE})
 *     </code></pre>
 *     <pre><code>
 *     add_library(jh-toolkit-static STATIC ${SRC})
 *     </code></pre>
 *     <pre><code>
 *     target_compile_definitions(jh-toolkit-static INTERFACE JH_HEADER_NO_IMPL)
 *     </code></pre>
 *   </li>
 *
 *   <li>
 *     <strong>Resulting usage summary</strong><br/>
 *     <ul>
 *       <li><code>jh::jh-toolkit</code> &rarr; header-only mode (full inline)</li>
 *       <li><code>jh::jh-toolkit-static</code> &rarr; prebuilt static object
 *           (declarations only)</li>
 *     </ul>
 *     This structure allows seamless interoperability between source inclusion
 *     and prebuilt linking, all from a single unified header source.
 *   </li>
 * </ol>
 *
 * <hr/>
 * <h3>Important Note</h3>
 * <ul>
 *   <li>These two helper headers
 *       (<code>jh/macros/header_begin.h</code> and
 *       <code>jh/macros/header_end.h</code>)
 *       are <strong>intended to be copied directly into your own project</strong>,
 *       rather than included from <code>jh-toolkit</code>.</li>
 *   <li>If your project already depends on <code>jh-toolkit</code>,
 *       you <strong>should rename the macros</strong> (e.g.
 *       <code>MYLIB_HEADER_NO_IMPL</code>) to avoid namespace pollution or
 *       conflicts.</li>
 *   <li>You may freely modify or rename them, as long as you keep
 *       the original license notice.</li>
 * </ul>
 *
 * @see jh/macros/header_end.h
 */

// ====================================================
// Definition inference logic
// ====================================================
#if defined(JH_HEADER_IMPL_BUILD)
#  define JH_INTERNAL_SHOULD_DEFINE 1
#  define JH_INLINE
#elif defined(JH_HEADER_NO_IMPL)
#  define JH_INTERNAL_SHOULD_DEFINE 0
#  define JH_INLINE inline
#else
#  define JH_INTERNAL_SHOULD_DEFINE 1
#  define JH_INLINE inline
#endif
