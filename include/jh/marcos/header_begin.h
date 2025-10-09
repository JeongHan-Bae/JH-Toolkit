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
 * @file header_begin.h (marcos)
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
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
 *   <tr><th>Macro</th><th>Definition Behavior</th><th>Description</th></tr>
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
 * #include &lt;jh/marcos/header_begin.h&gt;
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
 * #include &lt;jh/marcos/header_end.h&gt;
 * @endcode
 *
 * <h3>Visibility Rules</h3>
 * <ul>
 *   <li><b>Functions whose behavior depends on caller-side compile-time
 *       macros</b> (e.g. platform, feature toggles, template specializations)
 *       <b>must remain visible</b> in every translation unit — therefore they
 *       belong <b>outside</b> the
 *       <code>#if&nbsp;JH_INTERNAL_SHOULD_DEFINE</code> block.</li>
 *   <li>All other normal definitions should be wrapped inside
 *       <code>#if&nbsp;JH_INTERNAL_SHOULD_DEFINE</code> to avoid redefinition
 *       when linked from a static or shared object.</li>
 *   <li>Always finish the header with
 *       <code>#include&nbsp;&lt;jh/marcos/header_end.h&gt;</code>.</li>
 * </ul>
 *
 * <hr/>
 * <h3>Usage Guide: Using Dual-Mode Headers</h3>
 *
 * <p>
 * Once you have written a header using
 * <code>&lt;jh/marcos/header_begin.h&gt;</code> and
 * <code>&lt;jh/marcos/header_end.h&gt;</code>,
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
 *   <tr><td>Header-only</td><td><tt>(none)</tt></td>
 *       <td>Emit <tt>inline</tt> definitions</td>
 *       <td>Direct inclusion</td></tr>
 *   <tr><td>Static/Shared</td><td><tt>JH_HEADER_IMPL_BUILD</tt></td>
 *       <td>Emit non-inline strong definitions</td>
 *       <td>Library implementation TU</td></tr>
 *   <tr><td>Interface-only</td><td><tt>JH_HEADER_NO_IMPL</tt></td>
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
 *       <code>&lt;jh/marcos/header_end.h&gt;</code>,
 *       ensuring no cross-header contamination.</li>
 * </ul>
 *
 * <p><em>
 * With this pattern, a single C++ header can flexibly serve as
 * both a header-only and a static/shared library implementation —
 * eliminating the traditional need to maintain duplicate
 * <code>.h</code> / <code>.cpp</code> files.
 * </em></p>
 *
 * @see jh/marcos/header_end.h
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
