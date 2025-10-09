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
 * @file header_end.h (marcos)
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 * @brief Cleanup header for <code>jh/marcos/header_begin.h</code>.
 *
 * @attention This header is intended for multiple inclusions.
 * Do NOT use include guards or <code>#pragma once</code>.
 *
 * <h3>Purpose</h3>
 * <p>
 * Undefines temporary macros (<code>JH_INLINE</code> and
 * <code>JH_INTERNAL_SHOULD_DEFINE</code>) introduced by
 * <code>jh/marcos/header_begin.h</code>.
 * Always include it at the <b>end</b> of a dual-mode header.
 * </p>
 *
 * <h3>Pseudocode Layout</h3>
 * @code
 * #include &lt;jh/marcos/header_begin.h&gt;
 *
 * declare_things();
 *
 * must_be_visible_implementation();  // outside conditional
 *
 * #if JH_INTERNAL_SHOULD_DEFINE
 *     JH_INLINE normal_definition();
 * #endif
 *
 * #include &lt;jh/marcos/header_end.h&gt;
 * @endcode
 *
 * @see jh/marcos/header_begin.h
 */

// ====================================================
// Macro cleanup
// ====================================================
#undef JH_INLINE
#undef JH_INTERNAL_SHOULD_DEFINE
