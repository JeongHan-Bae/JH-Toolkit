/**
 * @file ipc.h (asynchronous)
 * @brief Aggregated header for inter-process communication primitives under <code>jh::async</code>.
 *
 * <p>
 * This header collects all IPC-related asynchronous primitives into a single entry point:
 * </p>
 *
 * <pre><code>#include &lt;jh/asynchronous/ipc.h&gt;</code></pre>
 *
 * <p>
 * It includes and re-exports all IPC components under
 * <code>jh::async::ipc</code>, such as shared-memory synchronization and
 * inter-process coordination utilities.
 * </p>
 *
 * <h4>Included Components</h4>
 * <ul>
 *   <li><code>ipc_limits</code> — compile-time IPC capacity and name validation utilities.</li>
 *   <li><code>process_mutex</code> — basic inter-process mutex, functionally
 *       similar to <code>std::timed_mutex</code>; non-recursive and minimal,
 *       used as the fundamental synchronization primitive.</li>
 *   <li><code>process_condition</code> — condition variable for processes.</li>
 *   <li><code>process_counter</code> — atomic counter for process coordination.</li>
 *   <li><code>shared_process_memory</code> — shared memory allocator and container.</li>
 *   <li><code>shared_process_mutex</code> — engineering-grade reader–writer lock
 *       built on shared memory, conceptually similar to
 *       <code>std::shared_timed_mutex</code> but supporting
 *       <b>reentrancy</b> and <b>privileged read-to-write promotion</b>
 *       under elevated contexts.</li>
 *   <li><code>process_launcher</code> — process orchestration utilities.</li>
 * </ul>
 *
 * <h4>Philosophy</h4>
 * <p>
 * Unlike <b>Boost.Interprocess</b>, which centralizes resource management within a
 * managed shared memory segment, the <code>jh::async::ipc</code> system implements
 * <b>compile-time named</b>, <b>process-independent</b> primitives built directly
 * on OS-level shared memory and semaphores.
 * </p>
 *
 * <p>
 * Each primitive (<code>mutex</code>, <code>condition</code>, <code>counter</code>,
 * <code>shared memory</code>) is a self-contained, globally addressable IPC object.
 * No central allocator or parent process is required — all participants synchronize
 * via shared OS namespaces.
 * </p>
 *
 * <p>
 * This design enables <b>decentralized, single-machine distributed coordination</b>
 * with <b>compile-time safety</b> and <b>zero runtime registration</b>.
 * The system itself, not a supervising process, performs the scheduling.
 * Daemons only perform minimal orchestration, reducing cognitive overhead.
 * </p>
 *
 * <h4>Compile-time naming contract</h4>
 * <p>
 * All IPC primitives in <code>jh::async::ipc</code> rely on a
 * <strong>compile-time naming convention</strong> enforced by
 * <code>jh::async::ipc::limits::valid_object_name</code>.
 * </p>
 * <ul>
 *   <li>Each synchronization object (e.g., semaphore, shared memory region, condition)
 *       is bound to a <b>name known at compile time</b>.</li>
 *   <li>The name serves as the <b>linkage contract</b> between processes —
 *       identical template literals across binaries guarantee consistent mapping.</li>
 *   <li>Invalid or conflicting names produce <b>compile-time errors</b>, ensuring
 *       namespace safety and deterministic inter-process behavior.</li>
 *   <li>Names may be hardcoded or provided through macros at build time, but
 *       must be resolvable during compilation to ensure deterministic layout.</li>
 * </ul>
 *
 * <p>
 * This model guarantees mapping consistency, eliminates runtime name collisions,
 * and provides a static coordination topology — effectively
 * <b>compile-time declared IPC fabric</b>.
 * </p>
 *
 * <h4>Structure Policy</h4>
 * <p>
 * The <code>jh/asynchronous/</code> directory remains <b>flat</b> by design.
 * Even though <code>ipc</code>-related components could form a submodule in the future,
 * this header always serves as the navigation and aggregation point for IPC facilities.
 * </p>
 */
#pragma once

#include "jh/asynchronous/ipc_limits.h"
#include "jh/asynchronous/process_mutex.h"
#include "jh/asynchronous/process_condition.h"
#include "jh/asynchronous/process_counter.h"
#include "jh/asynchronous/shared_process_memory.h"
#include "jh/asynchronous/shared_process_mutex.h"
#include "jh/asynchronous/process_launcher.h"
