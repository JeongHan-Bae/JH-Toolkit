/**
 * \verbatim
 * Copyright 2025 JeongHan-Bae <mastropseudo@gmail.com>
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
 * @file pod.h (POD system umbrella header)
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 * @brief Aggregated interface for defining and validating POD-like types.
 *
 * This header exposes the full <code>jh::pod</code> subsystem:
 * <ul>
 *   <li>POD detection via <code>pod_like&lt;T&gt;</code> concept</li>
 *   <li>Verified POD struct macro <code>JH_POD_STRUCT(...)</code></li>
 *   <li>Compile-time POD assertions via <code>JH_ASSERT_POD_LIKE</code></li>
 *   <li>Core POD value types (alphabetical order):
 *     <ul>
 *       <li><code>array&lt;T, N&gt;</code> — fixed buffer with POD enforcement</li>
 *       <li><code>bitflags&lt;N&gt;</code> — POD bitset for flag control</li>
 *       <li><code>bytes_view</code> — raw byte view, safe reinterpretation and cloning</li>
 *       <li><code>optional&lt;T&gt;</code> — POD-safe nullable wrapper (no constructors)</li>
 *       <li><code>pair&lt;T1, T2&gt;</code> — 2-field struct, POD alternative to <code>std::pair</code></li>
 *       <li><code>span&lt;T&gt;</code> — POD-only view into contiguous typed memory</li>
 *       <li><code>string_view</code> — POD-safe immutable text view (<code>char*, len</code>)</li>
 *       <li><code>tuple&lt;...&gt;</code> — transitional, fixed-slot POD tuple (deprecated)</li>
 *     </ul>
 *   </li>
 *   <li>Stream utilities in <code>stringify.h</code> for debug-only, human-readable printing</li>
 * </ul>
 *
 * <h3>Design Goals:</h3>
 * <ul>
 *   <li>Leverage compiler/STL optimizations:
 *       In <code>-O2</code> and higher, most modern standard libraries provide
 *       special-case optimizations when a type can be semantically recognized
 *       as POD (e.g., memcpy/memmove instead of element-wise operations).</li>
 *   <li>Enable specialized optimization paths in <code>jh::runtime_arr&lt;T&gt;</code>:
 *       POD compliance is automatically detected and used to eliminate redundant
 *       construction/destruction overhead in dynamic arrays.</li>
 *   <li>Provide a strict POD array type:
 *       <code>jh::pod::array&lt;char, N&gt;</code> serves as the underlying building block
 *       of <code>jh::str_template::cstr&lt;N&gt;</code> (introduced in 1.4.x), which replaces
 *       raw <code>char[]</code> literals with a fully traversable and semantically rich
 *       compile-time string representation.</li>
 *   <li>Enforce raw-layout safety and zero-overhead memory access across all POD utilities.</li>
 *   <li>Provide layout-stable, ABI-safe alternatives to STL containers where runtime
 *       guarantees are weaker (e.g., <code>std::pair</code>, <code>std::optional</code>,
 *       <code>std::tuple</code>).</li>
 * </ul>
 *
 * <h3>Important Notes:</h3>
 * <ul>
 *   <li>These types are strictly POD-like: trivially copyable, trivially destructible, standard layout.</li>
 *   <li>Debug printers (<code>stringify.h</code>) are for inspection only, not serialization.</li>
 *   <li>For real serialization, use <code>jh::utils::base64</code> helpers or implement your own protocol.</li>
 * </ul>
 *
 * @note Prefer <code>#include &lt;jh/pod&gt;</code> over individual headers unless performing targeted SFINAE.
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include "pods/pod_like.h"
#include "pods/pair.h"
#include "pods/array.h"
#include "pods/bits.h"
#include "pods/bytes_view.h"
#include "pods/span.h"
#include "pods/string_view.h"
#include "pods/tools.h"
#include "pods/optional.h"
#include "pods/stringify.h"
