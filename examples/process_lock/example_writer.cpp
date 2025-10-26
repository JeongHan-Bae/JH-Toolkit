/**
 * @file example_writer.cpp
 * @brief Example program demonstrating process-wide mutex for safe file writing.
 *
 * <h3>Overview</h3>
 * <p>
 * This example is part of the <code>example_process_lock</code> demo.
 * It simulates a <strong>writer</strong> process that appends log entries
 * to a shared file (<code>shared_log.txt</code>).
 * </p>
 *
 * <h3>Details</h3>
 * <ul>
 *   <li>Synchronization is achieved using <code>jh::async::ipc::process_mutex</code>.</li>
 *   <li>Before writing iterations, the writer clears the file using
 *       <code>std::ios::trunc</code> to ensure a clean start.</li>
 *   <li>Each iteration acquires the lock, appends a timestamped entry,
 *       then releases the lock.</li>
 *   <li>A 2-second pause between iterations simulates periodic writing.</li>
 * </ul>
 *
 * <h3>Key point</h3>
 * <p>
 * The <strong>process-level semaphore</strong> ensures exclusive file access.
 * This prevents conflicts and guarantees that the <strong>reader</strong>
 * only observes consistent data without relying on file system–level arbitration.
 * </p>
 *
 * <h4>Note</h4>
 * <p>
 * Run together with the corresponding <strong>reader</strong> example under
 * <code>example_process_lock</code> to observe lock coordination.
 * </p>
 */

#include "jh/asynchronous/process_mutex.h"
#include <fstream>
#include <iostream>
#include <thread>

using namespace std::chrono_literals;
using clock_t_ = std::chrono::steady_clock;
using mutex_t = jh::async::ipc::process_mutex<"demo_mutex">;

/**
 * @brief Entry point of the writer example.
 *
 * <p>The writer performs the following steps:</p>
 * <ol>
 *   <li>Acquires the process mutex and truncates <code>shared_log.txt</code>
 *       to clear old contents.</li>
 *   <li>Performs 3 iterations of:
 *     <ul>
 *       <li>Locking the mutex.</li>
 *       <li>Appending a timestamped log entry to the file.</li>
 *       <li>Unlocking the mutex.</li>
 *       <li>Sleeping for 2 seconds before the next iteration.</li>
 *     </ul>
 *   </li>
 * </ol>
 *
 * @return Exit code (<strong>0</strong> on success).
 */
int main() {
    auto &m = mutex_t::instance();

    // Step 1: clear the file at the beginning
    m.lock();
    {
        std::ofstream ofs("shared_log.txt", std::ios::trunc);
    }
    m.unlock();

    // Step 2: periodic writing
    for (int i = 0; i < 3; ++i) {
        m.lock();
        {
            std::ofstream ofs("shared_log.txt", std::ios::app);
            ofs << "[writer] iteration " << i
                << " at " << std::chrono::system_clock::now().time_since_epoch().count()
                << "\n";
            std::cout << "[writer] wrote iteration " << i << "\n";
        }
        m.unlock();

        std::this_thread::sleep_for(1s);
    }

    // Step 3: contention test using timed lock
    std::ostringstream temp;
    std::cout << "[writer] waiting 1s before contention test...\n";
    std::this_thread::sleep_for(1s);

    // try immediate lock
    if (!m.try_lock()) {
        temp << "[writer] immediate try_lock failed (lock held by another)\n";
    } else {
        temp << "[writer] immediate try_lock succeeded unexpectedly!\n";
        m.unlock();
    }

    // try timed lock (RAII style)
    {
        auto start = clock_t_::now();
        if (m.try_lock_for(3s)) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                    clock_t_::now() - start
            );
            temp << "[writer] timed lock succeeded after "
                 << elapsed.count() << " ms\n";
            m.unlock();
        } else {
            temp << "[writer] timed lock timed out after 3s\n";
        }
    }

    // Write test results back into shared_log.txt
    {
        std::lock_guard lock{m};
        std::ofstream ofs("shared_log.txt", std::ios::app);
        ofs << temp.str();
        std::cout << "[writer] wrote contention test results\n";
    }

    return 0;
}
