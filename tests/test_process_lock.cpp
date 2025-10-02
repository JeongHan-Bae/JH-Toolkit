#define CATCH_CONFIG_MAIN

#include <catch2/catch_all.hpp>

#define JH_ALLOW_PARENT_PATH 1

#include "jh/asynchronous/process_mutex.h"
#include "jh/asynchronous/process_launcher.h"
#include <fstream>
#include <mutex>                              // for std::lock_guard

/**
 * @brief Helper functions for testing.
 *
 * These utilities are scoped inside the `test` namespace
 * to avoid polluting the global namespace.
 */
namespace test {
    /// Count how many times `pattern` occurs in `text`.
    static std::size_t count_occurrences(const std::string &text,
                                         const std::string &pattern) {
        std::size_t count = 0;
        std::size_t pos = text.find(pattern);
        while (pos != std::string::npos) {
            ++count;
            pos = text.find(pattern, pos + 1);
        }
        return count;
    }

}

using mutex_t = jh::sync::process_mutex<"test_mutex">;

// ---- Single-process tests ----
TEST_CASE("process_mutex basic lock/unlock") {
    auto &m = mutex_t::instance();

    SECTION("lock and unlock") {
        m.lock();
        REQUIRE(m.try_lock() == false); // already locked
        m.unlock();
        REQUIRE(m.try_lock() == true);  // can re-acquire after unlock
        m.unlock();
    }

    SECTION("timed try_lock_for") {
        m.lock();
        auto acquired = m.try_lock_for(std::chrono::milliseconds(200));
        REQUIRE(acquired == false);  // timeout, could not acquire
        m.unlock();
    }
    jh::sync::process_mutex<"test_mutex", true>::unlink();
}

// ---- Cross-process tests ----

/// writer and reader are launched as external processes
using writer_launcher = jh::async::process_launcher<"../examples/process_lock/writer">;
using reader_launcher = jh::async::process_launcher<"../examples/process_lock/reader">;

/**
 * @brief priv_mutex_t represents the same named process-wide mutex ("demo_mutex")
 * used by writer/reader. It is declared with HighPriv=true so this test
 * can call unlink() to clean up the semaphore afterwards.
 */
using priv_mutex_t = jh::sync::process_mutex<"demo_mutex", true>;

TEST_CASE("process_launcher runs writer and reader") {
    // Launch writer & reader
    auto writer = writer_launcher::start();
    auto reader = reader_launcher::start();

    writer.wait();
    reader.wait();

    auto &m = priv_mutex_t::instance();

    {
        /**
         * @brief std::lock_guard ensures RAII-style locking.
         * The mutex is automatically unlocked when lock goes out of scope,
         * even if a test assertion fails in between.
         */
        std::lock_guard<priv_mutex_t> lock(m);

        // Verify the output file
        std::ifstream ifs("shared_log.txt");
        REQUIRE(ifs.good());

        std::string content((std::istreambuf_iterator<char>(ifs)),
                            std::istreambuf_iterator<char>());

        // Count occurrences of "[writer]" in the log.
        // There should be exactly 3 lines written by example_writer.cpp.
        REQUIRE(test::count_occurrences(content, "[writer]") == 3);
    }

    priv_mutex_t::unlink();
}
