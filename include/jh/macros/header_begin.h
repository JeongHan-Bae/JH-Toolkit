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
 * @brief Selects how a dual-mode header emits guarded definitions.
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 *
 * A dual-mode header holds one implementation for both header-only use and a
 * precompiled library. Changes to the implementation are made in that header
 * once. The compiled form can be built with chosen optimization flags and
 * reused by consumers.
 *
 * In a dual-mode header, declare each function or concrete specialization
 * before including this file. Place its guarded definitions after the include,
 * then include <code>jh/macros/header_end.h</code>. Static consumers see the
 * declarations while the definitions come from the linked library.
 * This file has no include guard because each dual-mode header sets up its
 * own mode.
 *
 * <table>
 *   <tr><th>Build setting</th><th>JH_INTERNAL_SHOULD_DEFINE</th><th>JH_INLINE</th></tr>
 *   <tr><td>Default header-only</td><td>1</td><td><code>inline</code></td></tr>
 *   <tr><td>JH_HEADER_IMPL_BUILD</td><td>1</td><td>empty</td></tr>
 *   <tr><td>JH_HEADER_NO_IMPL</td><td>0</td><td><code>inline</code></td></tr>
 * </table>
 *
 * Guard ordinary non-template functions and out-of-class members of a
 * concrete class-template specialization, such as <code>runtime_arr&lt;bool&gt;</code>.
 * Keep primary templates and templates with remaining type parameters visible
 * in the header.
 *
 * @code
 * #pragma once
 *
 * int non_template_helper();
 * template&lt;typename T&gt;
 * struct sample_box { T value; };
 * template&lt;&gt;
 * struct sample_box&lt;bool&gt; { bool value() const; };
 *
 * #include &lt;jh/macros/header_begin.h&gt;
 * #if JH_INTERNAL_SHOULD_DEFINE
 * JH_INLINE int non_template_helper() { return 0; }
 * JH_INLINE bool sample_box&lt;bool&gt;::value() const { return true; }
 * #endif
 * #include &lt;jh/macros/header_end.h&gt;
 * @endcode
 *
 * A non-template compilation unit selects implementation mode and includes
 * the header:
 * @code
 * #define JH_HEADER_IMPL_BUILD
 * #include "jh/serialize_io/base64.h"
 * @endcode
 * For a class template, an instantiation file also names the supported types:
 * @code
 * #define JH_HEADER_IMPL_BUILD
 * #include "jh/core/immutable_str.h"
 * template class jh::immutable_str&lt;true&gt;;
 * template class jh::immutable_str&lt;false&gt;;
 * @endcode
 * The static CMake target gives consumers <code>JH_HEADER_NO_IMPL</code>.
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
