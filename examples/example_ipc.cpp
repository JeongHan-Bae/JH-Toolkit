/**
 * @file example_ipc.cpp
 * @brief Example demonstrating the usage of <code>&lt;jh/ipc&gt;</code>.
 *
 * <p>
 * This file demonstrates how to use the process-level API from
 * <code>&lt;jh/ipc&gt;</code> and serves as a practical reference example.
 * </p>
 *
 * <p>
 * The canonical include form is:
 * </p>
 *
 * @code
 * #include &lt;jh/ipc&gt;
 * @endcode
 *
 * <p>
 * In JH Toolkit, IPC means <b>InterProcess Coordination</b>, not message transport.
 * This example merges lock-based and shared-memory workflows into one runnable demo.
 * </p>
 *
 * <h3>What This Example Covers</h3>
 *
 * <ul>
 *   <li><code>process_mutex</code> + <code>process_launcher</code> orchestration</li>
 *   <li><code>process_counter</code> for cross-process RMW coordination</li>
 *   <li><code>process_cond_var</code> for process-visible wake-up signaling</li>
 *   <li><code>process_shm_obj</code> for pure POD shared state</li>
 * </ul>
 *
 * <h3>Naming and Identity Model</h3>
 *
 * <p>
 * All IPC components in this file are declared by compile-time NTTP strings
 * and accessed via singleton <code>::instance()</code> handles.
 * Different executables synchronize by instantiating the same names.
 * </p>
 *
 * <h3>Platform Notes (Summary)</h3>
 *
 * <ul>
 *   <li><b>POSIX default (Darwin/FreeBSD and other standard POSIX targets):</b>
 *       supported as the baseline model.</li>
 *   <li><b>POSIX.1b extensions (for example Linux):</b>
 *       used when available (such as native timed semaphore waits).</li>
 *   <li><b>Windows (Win32):</b>
 *       supported with minimal compatibility layer and stricter limits/privilege
 *       constraints; refer to <code>docs/synchronous/ipc.md</code> and
 *       the module reference pages under <code>docs/synchronous/ipc/</code>
 *       for authoritative details.</li>
 * </ul>
 *
 * <h3>Launcher Path Rules</h3>
 *
 * <p>
 * <code>process_launcher&lt;Path, IsBinary&gt;</code> requires a POSIX-style relative path
 * literal for all platforms. Paths are resolved by the filesystem at runtime.
 * Therefore always write path names with <code>'/'</code> separators.
 * </p>
 *
 * <p>
 * The second template argument handles non-binary launch targets:
 * </p>
 *
 * @code
 * // Binary target (default): Windows appends ".exe" automatically.
 * using writer_launcher_t = jh::ipc::process_launcher<"process_lock/writer", true>;
 *
 * // Non-binary target: path is used as-is (e.g. script wrappers).
 * using script_launcher_t = jh::ipc::process_launcher<"tools/run_demo.bat", false>;
 * @endcode
 *
 * <p>
 * This example uses binary worker executables, so <code>IsBinary</code> is omitted
 * and defaults to <code>true</code>.
 * </p>
 *
 */

#include <jh/ipc>
#include <jh/pod>

#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <vector>

#include "ensure_output.h"  // NOLINT for Windows output
#if IS_WINDOWS
static EnsureOutput ensure_output_setup;
#endif

// -----------------------------------------------------------------------------
// Shared process-level primitives (compile-time names + singleton instances)
// -----------------------------------------------------------------------------
/**
 * @brief Process-wide mutex aliases.
 *
 * <ul>
 *   <li><code>mutex_t</code> is the normal coordination handle.</li>
 *   <li><code>priv_mutex_t</code> enables privileged cleanup (<code>unlink()</code>).</li>
 * </ul>
 */
using mutex_t = jh::ipc::process_mutex<"demo_mutex">;
using priv_mutex_t = jh::ipc::process_mutex<"demo_mutex", true>;

/**
 * @brief Process-wide counter aliases.
 *
 * <p>
 * The counter is named at compile time and accessed via <code>counter_t::instance()</code>.
 * </p>
 */
using counter_t = jh::ipc::process_counter<"demo_counter">;
using priv_counter_t = jh::ipc::process_counter<"demo_counter", true>;

/**
 * @brief Process-wide condition variable aliases.
 */
using cond_t = jh::ipc::process_cond_var<"demo_cond_var">;
using priv_cond_t = jh::ipc::process_cond_var<"demo_cond_var", true>;

/**
 * @brief Shared pure-data payload for <code>process_shm_obj</code>.
 *
 * <p>
 * This intentionally uses POD-only fields and avoids pointers or other process-local handles.
 * </p>
 */
JH_POD_STRUCT(DemoPod,
    std::uint64_t xor_field;
    std::uint64_t add_field;
    double mul_field;
);

/**
 * @brief Shared POD object aliases.
 */
using shm_t = jh::ipc::process_shm_obj<"demo_shared_pod", DemoPod>;
using priv_shm_t = jh::ipc::process_shm_obj<"demo_shared_pod", DemoPod, true>;

/**
 * @brief Compile-time bound process launchers.
 *
 * <p>
 * All launcher paths are POSIX-style relative names. On Windows, binary launchers
 * append <code>".exe"</code> automatically when <code>IsBinary == true</code> (default).
 * </p>
 */
using writer_launcher_t = jh::ipc::process_launcher<"process_lock/writer">;
using reader_launcher_t = jh::ipc::process_launcher<"process_lock/reader">;
using counter_launcher_t = jh::ipc::process_launcher<"process_lock/counter">;
using sleeper_launcher_t = jh::ipc::process_launcher<"process_lock/sleeper">;
using awaker_launcher_t = jh::ipc::process_launcher<"process_lock/awaker">;
using pod_writer_launcher_t = jh::ipc::process_launcher<"process_lock/pod_writer">;

// -----------------------------------------------------------------------------
// Example 1: process_mutex + process_launcher
// -----------------------------------------------------------------------------
/**
 * @brief Run the process mutex + launcher coordination demo.
 *
 * <p>
 * Launches writer and reader workers, waits for completion, then performs privileged
 * cleanup of the named mutex object.
 * </p>
 */
void run_mutex_launcher_example() {
    std::cout << "\n==================== process_mutex + process_launcher ====================\n";

    auto writer = writer_launcher_t::start();
    auto reader = reader_launcher_t::start();

    writer.wait();
    reader.wait();

    std::cout << "Mutex demo finished, unlinking named semaphore...\n";
    priv_mutex_t::unlink();
    std::cout << "Writer/reader demo completed. Check shared_log.txt\n";
}

// -----------------------------------------------------------------------------
// Example 2: process_counter
// -----------------------------------------------------------------------------
/**
 * @brief Run the shared counter demo.
 *
 * <p>
 * Demonstrates process-wide read/modify/write operations and strong/forced load APIs.
 * </p>
 */
void run_counter_example() {
    std::cout << "\n==================== process_counter ====================\n";

    constexpr int worker_count = 4;
    constexpr int increments_per_worker = 20'000;

    counter_t::instance().store(0);
    std::cout << "Launching " << worker_count << " counter workers...\n";

    std::vector<decltype(counter_launcher_t::start())> handles;
    handles.reserve(worker_count);

    for (int i = 0; i < worker_count; ++i) {
        handles.push_back(counter_launcher_t::start());
    }

    for (auto &h : handles) {
        h.wait();
    }

    const auto total = counter_t::instance().load_strong();
    const std::uint64_t expected = worker_count * increments_per_worker;

    std::cout << "Total = " << total << " (expected " << expected << ")\n";

    const auto old = counter_t::instance().fetch_apply([](std::uint64_t v) { return v + 10; });
    std::cout << "fetch_apply(+10): old=" << old
              << ", new=" << counter_t::instance().load_strong() << "\n";

    counter_t::instance().store(12345);
    std::cout << "store(12345), load_force() = " << counter_t::instance().load_force() << "\n";

    priv_counter_t::unlink();
    std::cout << "Unlinked shared counter.\n";
}

// -----------------------------------------------------------------------------
// Example 3: process_cond_var
// -----------------------------------------------------------------------------
/**
 * @brief Run the process condition-variable demo.
 *
 * <p>
 * Spawns sleepers, triggers a delayed wake-up process, then checks observed timing.
 * </p>
 */
void run_cond_var_example() {
    std::cout << "\n==================== process_cond_var ====================\n";

    constexpr int sleeper_count = 4;
    using namespace std::chrono;

    std::vector<decltype(sleeper_launcher_t::start())> handles;
    handles.reserve(sleeper_count);

    for (int i = 0; i < sleeper_count; ++i) {
        handles.push_back(sleeper_launcher_t::start());
    }

    const auto start = steady_clock::now();
    auto awaker = awaker_launcher_t::start();

    awaker.wait();
    for (auto &h : handles) {
        h.wait();
    }

    const auto elapsed_ms = duration_cast<milliseconds>(steady_clock::now() - start).count();
    constexpr int min_expected = 500;
    const int max_expected = sleeper_count * 500;

    std::cout << "Expected wake range: [" << min_expected << ", " << max_expected << "] ms\n";
    std::cout << "Observed wake time : " << elapsed_ms << " ms\n";

    if (elapsed_ms >= min_expected && elapsed_ms < max_expected) {
        std::cout << "Wake timing is within expected range.\n";
    } else {
        std::cout << "Wake timing outside expected range (possible contention).\n";
    }

    cond_t::instance().notify_all();
    priv_cond_t::unlink();
    std::cout << "Unlinked shared condition.\n";
}

// -----------------------------------------------------------------------------
// Example 4: process_shm_obj
// -----------------------------------------------------------------------------
/**
 * @brief Run the shared POD memory demo.
 *
 * <p>
 * Initializes a shared POD object, runs multiple writers, and prints the final
 * coordinated state.
 * </p>
 */
void run_shared_pod_example() {
    std::cout << "\n==================== process_shm_obj ====================\n";

    auto &shm = shm_t::instance();

    {
        std::lock_guard guard(shm.lock());
        shm.flush_acquire();
        shm.ref() = DemoPod{0, 0, 1.0};
        shm.flush_seq();
    }

    constexpr int writer_count = 4;
    std::cout << "Launching " << writer_count << " POD writer processes...\n";

    std::vector<decltype(pod_writer_launcher_t::start())> writers;
    writers.reserve(writer_count);

    for (int i = 0; i < writer_count; ++i) {
        writers.push_back(pod_writer_launcher_t::start());
    }

    for (auto &w : writers) {
        w.wait();
    }

    shm.flush_acquire();
    const auto &ref = shm.ref();

    constexpr std::uint64_t add_inc = 10;
    constexpr double mul_factor = 1.0001;
    constexpr int iterations = 20'000;

    const std::uint64_t expected_add = add_inc * iterations * writer_count;
    const double expected_mul = std::pow(mul_factor, iterations * writer_count);

    std::cout << "xor_field = " << ref.xor_field << " (expected invariant 0)\n";
    std::cout << "add_field = " << ref.add_field << " (expected " << expected_add << ")\n";
    std::cout << "mul_field = " << ref.mul_field << " (expected ~= " << expected_mul << ")\n";

    priv_shm_t::unlink();
    std::cout << "Unlinked shared POD memory.\n";
}

/**
 * @brief Entry point for the unified IPC demonstration.
 *
 * @return <code>0</code> on success, non-zero on failure.
 */
int main() {
    try {
        run_mutex_launcher_example();
        run_counter_example();
        run_cond_var_example();
        run_shared_pod_example();

        std::cout << "\nAll IPC examples completed successfully.\n";
    } catch (const std::exception &ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
