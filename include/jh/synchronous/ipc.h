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
 * @file ipc.h
 * @brief Aggregated header for InterProcess Coordination primitives under <code>jh::sync</code>.
 *
 * <p>
 * In JH Toolkit, IPC means <b>InterProcess Coordination</b>, not a message-transport framework.
 * This header collects all process-level coordination primitives into a single entry point:
 * </p>
 *
 * <pre><code>#include &lt;jh/synchronous/ipc.h&gt;</code></pre>
 *
 * <p>
 * It includes and re-exports all IPC components under <code>jh::sync::ipc</code>.
 * </p>
 *
 * <h4>Included components</h4>
 * <ul>
 *   <li><code>limits</code> &mdash; compile-time validation for object names and launcher paths.</li>
 *   <li><code>process_mutex</code> &mdash; process-wide counterpart of <code>std::timed_mutex</code>.</li>
 *   <li><code>process_cond_var</code> &mdash; process-wide counterpart of <code>std::condition_variable</code>.</li>
 *   <li><code>process_counter</code> &mdash; named shared counter for process coordination.</li>
 *   <li><code>process_shm_obj</code> &mdash; single shared POD object (pure data only).</li>
 *   <li><code>shared_process_mutex</code> &mdash; process-wide counterpart of <code>std::shared_timed_mutex</code>.</li>
 *   <li><code>process_launcher</code> &mdash; <code>std::thread</code>-like process launcher with compile-time path identity.</li>
 * </ul>
 *
 * <h4>Core model</h4>
 * <p>
 * Process-level primitives are intentionally close to in-process STL syntax.
 * The key differences are:
 * </p>
 * <ul>
 *   <li>Types are identified by compile-time NTTP strings.</li>
 *   <li>Named primitives are accessed through process-wide singletons via <code>::instance()</code>.</li>
 *   <li>The same NTTP name across different executables binds to the same OS object.</li>
 * </ul>
 *
 * <p>
 * Unlike parent/child-centric models, coordination is decentralized.
 * There is no required parent process, broker, runtime registry, or daemon.
 * Binaries synchronize by sharing the same compile-time names.
 * </p>
 *
 * <p>
 * Shared user state follows a strict rule:
 * <code>process_shm_obj</code> stores only POD-like pure data. Meaning and object semantics
 * are interpreted locally by each process.
 * </p>
 *
 * <h4>Compile-time naming contract</h4>
 * <p>
 * All IPC primitives in <code>jh::sync::ipc</code> rely on a
 * <strong>compile-time naming convention</strong> enforced by
 * <code>jh::sync::ipc::limits::valid_object_name</code>.
 * </p>
 * <ul>
 *   <li>Each object is bound to a <b>name known at compile time</b>.</li>
 *   <li>The name is the linkage contract between processes and executables.</li>
 *   <li>Invalid names fail at compile time, preserving deterministic topology.</li>
 * </ul>
 *
 * <p>
 * This model guarantees mapping consistency and avoids runtime namespace negotiation.
 * </p>
 */
#pragma once

#include "jh/synchronous/ipc/ipc_limits.h"
#include "jh/synchronous/ipc/process_mutex.h"
#include "jh/synchronous/ipc/process_cond_var.h"
#include "jh/synchronous/ipc/process_counter.h"
#include "jh/synchronous/ipc/process_shm_obj.h"
#include "jh/synchronous/ipc/shared_process_mutex.h"
#include "jh/synchronous/ipc/process_launcher.h"

/**
 * @brief Synchronous inter-process coordination primitives.
 *
 * The <code>jh::sync::ipc</code> namespace defines the complete set of OS-backed,
 * process-shared coordination primitives in JH Toolkit.
 * <br>
 * IPC here explicitly means <b>InterProcess Coordination</b>:
 * synchronization plus shared pure data, not high-level object sharing or messaging.
 * Components map directly to named OS primitives and use compile-time identities.
 * <br>
 * This namespace represents the authoritative IPC layer. Any aliases
 * or flattened entry points (such as <code>jh::ipc</code>) forward to
 * this namespace without introducing additional abstraction.
 */
namespace jh::sync::ipc {}
