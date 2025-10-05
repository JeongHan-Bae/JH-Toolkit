/**
 * @file example_reader.cpp
 * @brief Example program demonstrating process-wide mutex protecting file access.
 *
 * <h3>Overview</h3>
 * <p>
 * This example is part of the <code>example_process_lock</code> demo.
 * It simulates a <strong>reader</strong> process that attempts to open and
 * read from a shared file (<code>shared_log.txt</code>).
 * </p>
 *
 * <h3>Details</h3>
 * <ul>
 *   <li>Synchronization is achieved using <code>jh::async::process_mutex</code>.</li>
 *   <li>The <code>try_lock_for()</code> call ensures that the reader waits up to
 *       2 seconds to acquire the lock before opening the file.</li>
 *   <li>The timing of <code>std::ifstream</code> open is measured in
 *       <strong>microseconds (µs)</strong>, showing that with proper locking,
 *       file open incurs only the lightweight cost of a normal I/O operation.</li>
 * </ul>
 *
 * <h3>Key point</h3>
 * <p>
 * This demonstrates that the <strong>process-level semaphore</strong>
 * prevents file system–level contention, moving all waiting into
 * the <code>lock</code> acquisition phase instead.
 * </p>
 *
 * <h4>Note</h4>
 * <p>
 * Run together with the corresponding <strong>writer</strong> example under
 * <code>example_process_lock</code> to observe synchronization behavior.
 * </p>
 */

#include "jh/asynchronous/process_mutex.h"
#include <fstream>
#include <iostream>
#include <thread>

using namespace std::chrono_literals;
using clock_t_ = std::chrono::steady_clock;
using mutex_t = jh::async::process_mutex<"demo_mutex">;

/**
 * @brief Entry point of the reader example.
 *
 * <p>The reader does the following:</p>
 * <ol>
 *   <li>Waits briefly (500ms) to allow the writer to write first.</li>
 *   <li>Attempts to acquire the process mutex with a 2-second timeout.</li>
 *   <li>On success, opens <code>shared_log.txt</code> and prints its contents.</li>
 *   <li>Measures the time it takes to open the file stream.</li>
 * </ol>
 *
 * @return Exit code (<strong>0</strong> on success).
 */
int main() {
    auto &m = mutex_t::instance();

    std::this_thread::sleep_for(500ms); // delay start

    for (int i = 0; i < 3; ++i) {
        if (m.try_lock_for(2s)) {
            auto start = clock_t_::now();

            std::ifstream ifs("shared_log.txt");

            auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                    clock_t_::now() - start
            );
            std::cout << "[reader] ifstream open took "
                      << elapsed.count() << " µs\n";

            std::string line;
            while (std::getline(ifs, line)) {
                std::cout << "[reader] " << line << "\n";
            }

            m.unlock();
        } else {
            std::cout << "[reader] timeout after 2s, could not acquire lock\n";
        }

        std::this_thread::sleep_for(2s);
    }

    return 0;
}
