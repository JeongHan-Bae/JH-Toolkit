/**
 * @file example_occ_box.cpp
 * @brief Demonstrates the usage of <code>jh::conc::occ_box</code> in <code>jh-toolkit</code>.
 */

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>    // NOLINT force include for std::chrono_literals
#include <atomic>
#include <random>
#include "jh/concurrent/occ_box.h"

namespace example {

    struct Foo {
        int x;
        std::string name;

        [[nodiscard]] std::string to_string() const {
            return "Foo{x=" + std::to_string(x) + ", name=\"" + name + "\"}";
        }
    };

    /**
     * <strong>Demonstrates pointer-based replacement without unnecessary copy.</strong>
     *
     * <h4>Overview</h4>
     * <ul>
     *   <li>Instead of copying <code>*old</code>, we build a brand new <code>Foo</code> object.</li>
     *   <li>This avoids expensive deep copies (especially when <code>std::string</code> or other heavy members are not reused).</li>
     *   <li>In this example, we construct a new <code>Foo</code> with <code>x + 2</code> and a different name.</li>
     * </ul>
     *
     * <h4>Expected Result</h4>
     * <p>
     * The final stored value will be a newly created <code>Foo</code> instance,
     * proving that <strong>write_ptr()</strong> can be used to replace the object efficiently.
     * </p>
     *
     * @return void
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
     * <strong>Demonstrates a deterministic OCC update with per-thread backoff.</strong>
     *
     * <h4>Overview</h4>
     * <ul>
     *   <li>Four worker threads attempt to update the same <code>occ_box&lt;int&gt;</code> concurrently.</li>
     *   <li>Each thread applies a fixed delta (<code>+10</code>, <code>-15</code>, <code>+20</code>, <code>-5</code>).</li>
     *   <li>A per-thread exponential backoff (<code>std::chrono::microseconds</code>) is used 
     *       to avoid aggressive spinning when CAS fails.</li>
     *   <li>Each thread tracks its own attempt counter (<code>std::atomic&lt;uint32_t&gt;</code>).</li>
     *   <li>Logging with <code>std::ostringstream</code> ensures atomic output and simulates syscall overhead,
     *       making retries more likely (to showcase OCC conflict resolution).</li>
     * </ul>
     *
     * <h4>Expected Result</h4>
     * <p>
     * Since the operations are commutative and independent:
     * </p>
     * <pre>
     *   Initial value = 40;
     *   +10 - 15 + 20 - 5 = +10;
     *   Final value = 50
     * </pre>
     * <p>
     * Regardless of execution order or conflicts, the final result is <strong>deterministic</strong>.
     * </p>
     *
     * @param none This example function has no parameters.
     * @return void
     */
    void deterministic_backoff_example() {
        std::cout << "\n\U0001F539 Deterministic OCC with Backoff:\n";
        using namespace std::chrono_literals;

        jh::conc::occ_box<int> box(40);    ///< Initial value of the box
        std::atomic<bool> start{false};  ///< Synchronization flag for simultaneous start

        // Per-thread attempt counters
        std::atomic<uint32_t> attemptsA{0}, attemptsB{0}, attemptsC{0}, attemptsD{0};

        /**
         * <strong>Helper factory for worker threads.</strong><br>
         * Each worker will repeatedly attempt to update the box with its own delta.
         *
         * @param delta The value to add or subtract in this worker.
         * @param counter A reference to the attempt counter for this worker.
         * @return A lambda suitable for execution in <code>std::thread</code>.
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
     * <strong>Demonstrates usage of <code>apply_to</code> for multi-object atomic updates.</strong>
     *
     * <h4>Overview</h4>
     * <ul>
     *   <li>We create two <code>occ_box&lt;int&gt;</code> objects: one representing an "account A"
     *       and the other "account B".</li>
     *   <li>We want to perform a <strong>transfer</strong> of <code>50</code> units
     *       from A to B.</li>
     *   <li>Using <code>apply_to</code>, both modifications are committed atomically,
     *       so there is no risk of:
     *       <ul>
     *         <li>Money disappearing (decrement applied, increment not applied).</li>
     *         <li>Money duplication (increment applied without decrement).</li>
     *       </ul>
     *   </li>
     *   <li>This ensures <strong>all-or-nothing semantics</strong> across multiple boxes.</li>
     * </ul>
     *
     * <h4>Expected Result</h4>
     * <pre>
     * Initial: A = 100, B = 200;
     * Transfer: -50 from A, +50 to B;
     * Final:   A = 50, B = 250
     * </pre>
     *
     * @return void
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
     * <strong>Demonstrates pointer-based <code>apply_to</code> for multi-object atomic updates.</strong>
     *
     * <h4>Overview</h4>
     * <ul>
     *   <li>We define two <code>occ_box&lt;Foo&gt;</code> objects, representing two user records.</li>
     *   <li>Using <code>apply_to</code> (pointer version), both are replaced with brand new
     *       <code>shared_ptr&lt;Foo&gt;</code> objects.</li>
     *   <li>This avoids deep copy of existing <code>Foo</code> objects (which contain strings).</li>
     *   <li>Both updates are committed atomically, ensuring consistency across multiple boxes.</li>
     * </ul>
     *
     * <h4>Expected Result</h4>
     * <pre>
     * Initial: A = Foo{x=1, name="Alice"}
     *          B = Foo{x=2, name="Bob"}
     * Update:  A → Foo{x=11, name="Alice-updated"}
     *          B → Foo{x=22, name="Bob-updated"}
     * </pre>
     *
     * @return void
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
     * <strong>Demonstrates passing external variables into apply_to via lambda captures.</strong>
     *
     * <h4>Overview</h4>
     * <ul>
     *   <li><code>apply_to</code> does not accept extra parameters by design
     *       (to keep template deduction tractable).</li>
     *   <li>Instead, external variables should be captured inside the lambda.</li>
     *   <li>This example simulates a business transfer where the transfer amount
     *       is provided by a captured variable (<code>amount</code>).</li>
     * </ul>
     *
     * <h4>Expected Result</h4>
     * <pre>
     * Initial: A = 300, B = 100;
     * Transfer: -75 from A, +75 to B;
     * Final:   A = 225, B = 175
     * </pre>
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
 * @brief Main entry point to run all occ_box examples.
 */
int main() {
    example::pointer_replacement_no_copy();
    example::deterministic_backoff_example();
    example::apply_to_example();
    example::apply_to_ptr_example();
    example::apply_to_with_captures_example();
    return 0;
}
