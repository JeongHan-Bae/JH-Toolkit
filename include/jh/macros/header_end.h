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
 * @file header_end.h
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 * @brief Cleanup header for <code>jh/macros/header_begin.h</code>.
 *
 * @attention This header is intended for multiple inclusions.
 * Do NOT use include guards or <code>#pragma once</code>.
 *
 * <h3>Purpose</h3>
 * <p>
 * Undefines temporary macros (<code>JH_INLINE</code> and
 * <code>JH_INTERNAL_SHOULD_DEFINE</code>) introduced by
 * <code>jh/macros/header_begin.h</code>.
 * Always include it at the <b>end</b> of a dual-mode header.
 * </p>
 *
 * <h3>Pseudocode Layout</h3>
 * @code
 * #include &lt;jh/macros/header_begin.h&gt;
 *
 * declare_things();
 *
 * must_be_visible_implementation();  // outside conditional
 *
 * #if JH_INTERNAL_SHOULD_DEFINE
 *     JH_INLINE normal_definition();
 * #endif
 *
 * #include &lt;jh/macros/header_end.h&gt;
 * @endcode
 *
 * @see jh/macros/header_begin.h
 */

// ====================================================
// Macro cleanup
// ====================================================
#undef JH_INLINE
#undef JH_INTERNAL_SHOULD_DEFINE
