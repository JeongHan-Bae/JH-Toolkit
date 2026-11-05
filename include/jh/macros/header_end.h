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
 * @brief Clears the temporary macros set by <code>jh/macros/header_begin.h</code>.
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 *
 * Include this file after a dual-mode header's guarded definitions. It has
 * no include guard because each dual-mode header clears its own temporary
 * macros. The build-setting macros remain available to later headers.
 *
 * @see jh/macros/header_begin.h
 */

// ====================================================
// Macro cleanup
// ====================================================
#undef JH_INLINE
#undef JH_INTERNAL_SHOULD_DEFINE
