/**
 * @file example_async.cpp
 * @brief Example demonstrating the usage of <code>&lt;jh/async&gt;</code>.
 *
 * <p>
 * <code>&lt;jh/async&gt;</code> is a coroutine-based asynchronous module designed to be
 * used in a semantically <b>synchronous</b> way: execution progress is explicit,
 * state transitions are visible, and control flow is manually driven by user code.
 * </p>
 *
 * <p>
 * This example focuses on:
 * </p>
 *
 * <ul>
 *   <li><code>jh::async::fiber</code> for explicit step-by-step state progression.</li>
 *   <li><code>jh::async::slot</code> / <code>listener</code> / <code>event_signal</code> for eager signal handling.</li>
 *   <li>Header/namespace behavior for generator naming.</li>
 * </ul>
 *
 * <h3>Generator Naming Note</h3>
 *
 * <p>
 * When including only <code>&lt;jh/async&gt;</code>, the generator type is
 * <code>jh::async::generator</code>.
 * </p>
 *
 * <p>
 * The promoted root alias <code>jh::generator</code> is available only when including
 * <code>&lt;jh/generator&gt;</code>.
 * </p>
 *
 * <p>
 * This file keeps generator usage intentionally minimal. For full generator patterns,
 * refer to <code>examples/example_generator.cpp</code>.
 * </p>
 *
 * <h3>Warnings from Doxygen (fiber.h / slot.h)</h3>
 *
 * <ul>
 *   <li><b>fiber:</b> uncaught exceptions inside coroutine body call <code>std::terminate()</code>.</li>
 *   <li><b>fiber:</b> avoid immediately-invoked coroutine lambdas on GCC 14+.</li>
 *   <li><b>fiber:</b> treat a fiber as thread-affine; resume from one execution context consistently.</li>
 *   <li><b>slot:</b> one hub binds one slot; do not bind multiple slots to one hub.</li>
 *   <li><b>slot:</b> call <code>spawn()</code> before emitting, and connect signals before first emit.</li>
 *   <li><b>slot:</b> await exactly one listener per phase; do not await multiple listeners in one loop step.</li>
 *   <li><b>slot:</b> <code>slot</code>, <code>slot_hub</code>, and listeners must share lifetime;
 *       <code>event_signal</code> must not outlive its listener.</li>
 * </ul>
 */

#include <jh/async>
#include "ensure_output.h"

#if IS_WINDOWS
static EnsureOutput ensure_output_setup;
#endif

#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

namespace example {

    // Minimal usage only. For full patterns, see examples/example_generator.cpp.
    jh::async::generator<int> tiny_generator() {
        co_yield 1;
        co_yield 2;
        co_yield 3;
    }

    void example_generator_name_note() {
        std::cout << "\n===== generator naming note =====\n\n";
        std::cout << "with <jh/async>: use jh::async::generator\n";
        std::cout << "with <jh/generator>: jh::generator is promoted alias\n";

        auto g = tiny_generator();
        std::cout << "tiny_generator values: ";
        while (g.next()) {
            std::cout << g.value().value() << " ";
        }
        std::cout << "\n";
    }

    void example_fiber_round_robin() {
        std::cout << "\n===== fiber round-robin (manual progression) =====\n\n";

        std::ostringstream out;

        // GCC-safe two-step coroutine lambda construction pattern.
        auto make_worker = [&](int id) -> jh::async::fiber {
            // Fiber body should guard/handle exceptions internally.
            try {
                out << "[fiber " << id << "] A\n";
                co_await jh::async::resume_tag;
                out << "[fiber " << id << "] B\n";
                co_await jh::async::resume_tag;
                out << "[fiber " << id << "] done\n";
            } catch (...) {
                out << "[fiber " << id << "] caught error\n";
                co_return;
            }
        };

        std::vector<jh::async::fiber> fibers;
        fibers.emplace_back(make_worker(1));
        fibers.emplace_back(make_worker(2));
        fibers.emplace_back(make_worker(3));

        for (auto &f: fibers) {
            f.resume();
        }

        bool all_done = false;
        while (!all_done) {
            all_done = true;
            for (auto &f: fibers) {
                if (!f.done()) {
                    f.resume();
                    all_done = false;
                }
            }
        }

        std::cout << out.str();
    }

    void example_slot_phase_switch() {
        std::cout << "\n===== slot phase switch (one listener per phase) =====\n\n";

        using namespace std::chrono_literals;
        using jh::async::event_signal;
        using jh::async::listener;
        using jh::async::slot;
        using jh::async::slot_hub;

        constexpr int stop_value = 999;

        std::vector<int> observed_ints;
        std::vector<std::string> observed_strings;

        slot_hub hub(200ms);

        auto li_int = hub.make_listener<int>();
        auto li_str = hub.make_listener<std::string>();

        auto make_slot = [&](listener<int> &aw_int, listener<std::string> &aw_str) -> slot {
            // Phase 1: await int listener only.
            for (;;) {
                int v = co_await aw_int;
                observed_ints.push_back(v);
                if (v == stop_value) {
                    break;
                }
                co_yield {};
            }

            // Phase 2: await string listener only.
            for (;;) {
                std::string s = co_await aw_str;
                if (s == "STOP") {
                    co_return;
                }
                observed_strings.push_back(std::move(s));
                co_yield {};
            }
        };

        slot s = make_slot(li_int, li_str);
        hub.bind_slot(s);
        s.spawn();

        event_signal<int> sig_int;
        event_signal<std::string> sig_str;
        sig_int.connect(&li_int);
        sig_str.connect(&li_str);

        for (int v: {1, 2, 3, stop_value}) {
            const bool accepted = sig_int.emit(v);
            std::cout << "emit int(" << v << ") -> "
                      << (accepted ? "accepted" : "rejected") << "\n";
        }

        for (const std::string& svalue: {std::string{"A"}, std::string{"B"}, std::string{"C"}}) {
            const bool accepted = sig_str.emit(svalue);
            std::cout << "emit str(" << svalue << ") -> "
                      << (accepted ? "accepted" : "rejected") << "\n";
        }
        (void) sig_str.emit("STOP");

        std::cout << "phase-1 ints: ";
        for (int v: observed_ints) {
            std::cout << v << " ";
        }
        std::cout << "\n";

        std::cout << "phase-2 strings: ";
        for (const auto &sv: observed_strings) {
            std::cout << sv << " ";
        }
        std::cout << "\n";
    }

} // namespace example

int main() {
    example::example_generator_name_note();
    example::example_fiber_round_robin();
    example::example_slot_phase_switch();
    return 0;
}
