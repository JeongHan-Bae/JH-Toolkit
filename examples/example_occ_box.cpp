/**
 * @file example_occ_box.cpp
 * @brief Usage examples for <code>jh::conc::occ_box</code>.
 *
 * @details
 * <h4>Philosophy</h4>
 * <ul>
 *   <li>Prefer <b>correctness</b> and <b>cognitive simplicity</b> over lock choreography.</li>
 *   <li>Read from validated snapshots and commit via atomic whole-state replacement.</li>
 *   <li>Treat updates as <em>build next state, then publish</em>, not in-place mutation.</li>
 * </ul>
 *
 * <p>
 * <code>occ_box</code> is a strong fit for prototype/MVP phases because it keeps concurrent
 * business logic explicit and auditable. The trade-off is structural: each write path may
 * allocate and retry, so heavy write-hot paths eventually benefit from a specialized model
 * derived from the same OCC principles.
 * </p>
 */

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <sstream>
#include <chrono>    // NOLINT force include for std::chrono_literals
#include <atomic>
#include <random>
#include <jh/concurrency>

#include "ensure_output.h"

#if IS_WINDOWS
static EnsureOutput ensure_output_setup;
#endif

namespace example {

    struct Foo {
        int x;
        std::string name;

        [[nodiscard]] std::string to_string() const {
            return "Foo{x=" + std::to_string(x) + ", name=\"" + name + "\"}";
        }
    };

    /**
     * @brief Demonstrates pointer-based replacement without deep-copying old state.
     *
     * @details
     * <p>
     * <code>write_ptr()</code> expresses an important OCC idea: build the next immutable state
     * directly, then publish it with a single commit. Instead of mutating or cloning
     * the current object graph, this example creates a fresh <code>Foo</code>.
     * </p>
     *
     * @par Philosophy
     * <p>
     * The box owns versioned snapshots. Writers produce a candidate snapshot and readers
     * observe either the old snapshot or the new one, never a partial state.
     * </p>
     *
     * @par Expected result
     * <p>
     * Final value is a newly constructed <code>Foo</code> with <code>x + 2</code> and a changed
     * <code>name</code>.
     * </p>
     */
    void pointer_replacement_no_copy() {
        std::cout << "\n\U0001F539 Pointer Replacement (No Copy):\n";

        jh::conc::occ_box<Foo> box(10, "original");

        // Replace with a *new object* instead of copying the old one
        box.write_ptr([](const std::shared_ptr<Foo> &old) {
            return std::make_shared<Foo>(Foo{old->x + 2, "new-constructed"});
        });

        // Verify result
        auto result = box.read([](const Foo &f) {
            return f.to_string();
        });
        std::cout << "Replaced: " << result << "\n";
    }

    /**
     * @brief Demonstrates deterministic OCC updates under contention with backoff.
     *
     * @details
     * <ul>
     *   <li>Four threads update one <code>occ_box&lt;int&gt;</code> with fixed deltas.</li>
     *   <li>Retries are expected under contention.</li>
     *   <li>Each worker uses exponential backoff with jitter to avoid pathological spinning.</li>
     * </ul>
     *
     * @par Philosophy
     * <p>
     * OCC does not promise deterministic execution order; it promises atomic commits.
     * If each update is a valid state transition, final business invariants remain
     * deterministic even when interleavings are not.
     * </p>
     *
     * @par Expected result
     * <p>
     * Initial value <code>40</code> plus deltas <code>(+10, -15, +20, -5)</code> yields
     * final value <code>50</code>.
     * </p>
     */
    void deterministic_backoff_example() {
        std::cout << "\n\U0001F539 Deterministic OCC with Backoff:\n";
        using namespace std::chrono_literals;

        jh::conc::occ_box<int> box(40);    ///< Initial value of the box
        std::atomic<bool> start{false};  ///< Synchronization flag for simultaneous start

        // Per-thread attempt counters
        std::atomic<uint32_t> attemptsA{0}, attemptsB{0}, attemptsC{0}, attemptsD{0};

        /**
         * @brief Creates a worker that repeatedly applies one delta until committed.
         *
         * @param delta Value to add/subtract.
         * @param counter Attempt counter for the worker.
         * @return Callable suitable for <code>std::thread</code>.
         */
        auto make_worker = [&](int delta, std::atomic<uint32_t> &counter) {
            return [&, delta]() {
                std::chrono::microseconds delay{0};
                /// <strong>Backoff with jitter (C++11 random)</strong>
                static thread_local std::mt19937 rng{std::random_device{}()};
                while (!start.load()) { std::this_thread::yield(); } // wait for simultaneous start

                box.write([&](int &v) {

                    /// <strong>Backoff with jitter</strong>
                    if (delay.count() > 0) {
                        auto base_delay = delay;

                        std::uniform_int_distribution<int> dist(0, static_cast<int>(delay.count() / 2));
                        auto jitter = std::chrono::microseconds(dist(rng));

                        std::this_thread::sleep_for(base_delay + jitter);
                    }

                    delay = (delay.count() == 0) ? 50us : std::min(delay * 2, 5000us);

                    /// <strong>Actual update</strong>
                    v += delta;

                    // Add an output syscall to simulate real-world operation cost,
                    // ensuring that not all updates succeed in a single CAS attempt.
                    std::ostringstream oss;
                    oss << "Thread " << std::this_thread::get_id()
                        << " attempt to add " << delta
                        << ", result=" << v << "\n";
                    std::cout << oss.str();

                    /// <strong>Count this attempt</strong>
                    counter.fetch_add(1, std::memory_order_relaxed);
                });
            };
        };

        // Launch four threads with different deltas
        std::thread tA(make_worker(+10, attemptsA));
        std::thread tB(make_worker(-15, attemptsB));
        std::thread tC(make_worker(+20, attemptsC));
        std::thread tD(make_worker(-5, attemptsD));

        start.store(true); // release all workers simultaneously

        tA.join();
        tB.join();
        tC.join();
        tD.join();

        // Check final result
        auto result = box.read([](const int &v) { return v; });
        std::cout << "Final value = " << result << " (expected 50)\n";

        std::cout << "Attempts A (+10): " << attemptsA.load() << "\n";
        std::cout << "Attempts B (-15): " << attemptsB.load() << "\n";
        std::cout << "Attempts C (+20): " << attemptsC.load() << "\n";
        std::cout << "Attempts D (-5): " << attemptsD.load() << "\n";
    }

    /**
     * @brief Demonstrates <code>apply_to</code> for multi-box atomic updates.
     *
     * @details
     * <ul>
     *   <li>Two account boxes are updated as one logical transaction using <code>apply_to</code>.</li>
     *   <li>Debit and credit either both commit or both fail.</li>
     * </ul>
     *
     * @par Philosophy
     * <p>
     * Cross-object invariants belong to one atomic commit boundary. OCC can extend
     * from single-value correctness to multi-value correctness when updates are
     * composed into one transaction attempt.
     * </p>
     *
     * @par Expected result
     * <p>
     * <code>A: 100 -&gt; 50</code>, <code>B: 200 -&gt; 250</code>.
     * </p>
     */
    void apply_to_example() {
        std::cout << "\n\U0001F539 Apply-To Example (Atomic Transfer):\n";

        jh::conc::occ_box<int> accountA(100);
        jh::conc::occ_box<int> accountB(200);

        bool ok = jh::conc::apply_to(
                std::tie(accountA, accountB),
                std::make_tuple(
                        [](int &a) { a -= 50; },  // withdraw
                        [](int &b) { b += 50; }   // deposit
                )
        );

        if (!ok) {
            std::cout << "Transfer failed due to conflict\n";
        }

        auto finalA = accountA.read([](const int &v) { return v; });
        auto finalB = accountB.read([](const int &v) { return v; });

        std::cout << "Final balances: A=" << finalA << ", B=" << finalB
                  << " (expected A=50, B=250)\n";
        std::cout << "Versions: A=" << accountA.get_version()
                  << ", B=" << accountB.get_version() << "\n";
    }
    /**
     * @brief Demonstrates pointer-based <code>apply_to</code> with object replacement.
     *
     * @details
     * <ul>
     *   <li>Uses multi-box atomicity with <code>apply_to</code>.</li>
     *   <li>Uses build-new-and-publish with pointer-returning lambdas.</li>
     * </ul>
     *
     * @par Philosophy
     * <p>
     * When payloads are non-trivial, replacing snapshots with freshly built objects
     * keeps update intent explicit and avoids accidental in-place mutation patterns.
     * </p>
     *
     * @par Expected result
     * <ul>
     *   <li><code>A: Foo{x=1, "Alice"} -&gt; Foo{x=11, "Alice-updated"}</code></li>
     *   <li><code>B: Foo{x=2, "Bob"} -&gt; Foo{x=22, "Bob-updated"}</code></li>
     * </ul>
     */
    void apply_to_ptr_example() {
        std::cout << "\n\U0001F539 Apply-To Example (Pointer Version, Foo):\n";

        jh::conc::occ_box<Foo> userA(1, "Alice");
        jh::conc::occ_box<Foo> userB(2, "Bob");

        bool ok = jh::conc::apply_to(
                std::tie(userA, userB),
                std::make_tuple(
                        [](const std::shared_ptr<Foo> &a) {
                            return std::make_shared<Foo>(Foo{a->x + 10, a->name + "-updated"});
                        },
                        [](const std::shared_ptr<Foo> &b) {
                            return std::make_shared<Foo>(Foo{b->x * 11, b->name + "-updated"});
                        }
                )
        );

        if (!ok) {
            std::cout << "Atomic update failed due to conflict\n";
        }

        auto finalA = userA.read([](const Foo &f) { return f.to_string(); });
        auto finalB = userB.read([](const Foo &f) { return f.to_string(); });

        std::cout << "Final values:\n"
                  << "  A = " << finalA << "\n"
                  << "  B = " << finalB << "\n";
        std::cout << "Versions: A=" << userA.get_version()
                  << ", B=" << userB.get_version() << "\n";
    }

    /**
     * @brief Demonstrates passing runtime parameters to <code>apply_to</code> via captures.
     *
     * @details
     * <p>
     * <code>apply_to</code> intentionally keeps a tight callable shape.
     * External policy inputs (such as transfer amount) are injected through lambda captures.
     * </p>
     *
     * @par Philosophy
     * <p>
     * Keep transactional API signatures strict, and move dynamic business context
     * to capture scope. This keeps template contracts simple while preserving
     * expressive user logic.
     * </p>
     *
     * @par Expected result
     * <p>
     * <code>A: 300 -&gt; 225</code>, <code>B: 100 -&gt; 175</code>.
     * </p>
     */
    void apply_to_with_captures_example() {
        std::cout << "\n\U0001F539 Apply-To Example (Lambda Capture for Parameters):\n";

        jh::conc::occ_box<int> accountA(300);
        jh::conc::occ_box<int> accountB(100);

        int amount = 75; // external variable

        bool ok = jh::conc::apply_to(
                std::tie(accountA, accountB),
                std::make_tuple(
                        [=](int &a) { a -= amount; },  // capture amount
                        [=](int &b) { b += amount; }   // capture amount
                )
        );

        if (!ok) {
            std::cout << "Transfer failed due to conflict\n";
        }

        auto finalA = accountA.read([](const int &v) { return v; });
        auto finalB = accountB.read([](const int &v) { return v; });

        std::cout << "Final balances: A=" << finalA << ", B=" << finalB
                  << " (expected A=225, B=175)\n";
    }

} // namespace example


/**
 * @brief Runs all <code>occ_box</code> examples in this file.
 * @return Process exit code.
 */
int main() {
    example::pointer_replacement_no_copy();
    example::deterministic_backoff_example();
    example::apply_to_example();
    example::apply_to_ptr_example();
    example::apply_to_with_captures_example();
    return 0;
}
