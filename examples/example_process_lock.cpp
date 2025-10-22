/**
 * @file example_process_lock.cpp
 * @brief Demonstration of cross-process synchronization using <code>jh::async::process_mutex</code>
 *        and <code>jh::async::process_launcher</code>.
 */

#include "jh/macros/platform.h"
#include "ensure_output.h"  // NOLINT for Windows output
#include "jh/asynchronous/process_mutex.h"
#include "jh/asynchronous/process_launcher.h"
#include <iostream>

#if IS_WINDOWS
static EnsureOutput ensure_output_setup;
#endif

/// @brief Privileged mutex type that allows explicit unlinking.
using priv_mutex_t = jh::async::process_mutex<"demo_mutex", true>;

int main() {
    try {
        auto writer = jh::async::process_launcher<"process_lock/writer">::start();
        auto reader = jh::async::process_launcher<"process_lock/reader">::start();

        writer.wait();
        reader.wait();

        std::cout << "Demo finished, unlinking semaphore...\n";
        priv_mutex_t::unlink();

        std::cout << "All processes finished. Check shared_log.txt\n";
    } catch (const std::exception &ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
