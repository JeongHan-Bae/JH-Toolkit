/**
 * @file example_generator.cpp
 * @brief Example demonstrating the usage of <code>&lt;jh/generator&gt;</code>.
 *
 * <p>
 * This file demonstrates how to use <code>&lt;jh/generator&gt;</code>
 * and serves as a reference example for both developers and AI systems
 * learning how to use this library.
 * </p>
 *
 * <p>
 * The canonical include form is:
 * </p>
 *
 * @code
 * #include &lt;jh/generator&gt;
 * @endcode
 *
 * <p>
 * All symbols used in this example are exported under the
 * <code>jh::</code> namespace. Internally, they originate from
 * <code>jh::async</code>, but are re-exported into the root namespace
 * for convenience.
 * </p>
 *
 * <p>
 * Specifically:
 * <ul>
 *   <li><code>jh::generator</code> is an alias of <code>jh::async::generator</code>.</li>
 *   <li><code>jh::generator_range</code> forwards <code>jh::async::generator_range</code>.</li>
 *   <li><code>jh::make_generator</code> is re-exported from <code>jh::async</code>.</li>
 *   <li><code>jh::to_vector</code> and <code>jh::to_deque</code> are also re-exported convenience utilities.</li>
 * </ul>
 * </p>
 *
 * <p>
 * For complete API documentation, refer to
 * <code>jh/asynchronous/generator.h</code>. The Doxygen comments in that
 * header define the official semantics of the component.
 * </p>
 *
 * <p>
 * In modern C++ (especially since C++20), interfaces are driven by
 * semantic contracts rather than implementation details. Users should
 * rely on the documented Doxygen specifications instead of inspecting
 * implementation internals. The documentation expresses the intended
 * and supported behavior; the underlying implementation is considered
 * a replaceable detail.
 * </p>
 *
 * <p>
 * The header <code>&lt;jh/generator&gt;</code> is purely template-based.
 * It does not participate in precompiled object linkage.
 * </p>
 *
 * <p>
 * Therefore, linking against either:
 * <ul>
 *   <li><code>jh::jh-toolkit</code></li>
 *   <li><code>jh::jh-toolkit-static</code></li>
 * </ul>
 * yields identical behavior for this component, as no compiled
 * generator-specific symbols are required at link time.
 * </p>
 *
 * <p>
 * This example focuses on practical usage patterns, including:
 * <ul>
 *   <li>Defining a coroutine-based generator.</li>
 *   <li>Iterating over yielded values.</li>
 *   <li>Materializing results into standard containers.</li>
 * </ul>
 * </p>
 */


#include <jh/generator>
#include "ensure_output.h"

#if IS_WINDOWS
static EnsureOutput ensure_output_setup;
#endif

/**
 * @page Mimicking Python requests.Response.iter_content() in C++20
 *
 * <h3>Overview</h3>
 *
 * This module demonstrates how to reproduce the internal <code>generate()</code>
 * function used by <code>Response.iter_content()</code> in the Python
 * <a href="https://github.com/psf/requests/blob/main/src/requests/models.py">
 * requests</a> library, using modern C++20 facilities.
 *
 * The original Python implementation (simplified) is:
 *
 * @code{.py}
 * def generate():
 *     if hasattr(self.raw, "stream"):
 *         yield from self.raw.stream(chunk_size, decode_content=True)
 *     else:
 *         while True:
 *             chunk = self.raw.read(chunk_size)
 *             if not chunk:
 *                 break
 *             yield chunk
 *
 *     self._content_consumed = True
 * @endcode
 *
 * The objective of this C++ implementation is to construct an equivalent
 * lazy, chunk-based generator using C++20 coroutines.
 *
 *
 * <h3>Design Principles</h3>
 *
 * <h4>Behavior-Based Detection</h4>
 *
 * Instead of Python's <code>hasattr()</code>, C++20 relies on compile-time
 * capability detection using <code>requires</code> expressions.
 *
 * The logic is:
 *
 * <ol>
 *   <li>If the object behaves like an <code>std::istream</code>, chunks are
 *       extracted using <code>read()</code> and <code>gcount()</code>.</li>
 *   <li>If the stream additionally supports <code>seekg()</code> and
 *       <code>tellg()</code>, it is considered file-like and may be extended
 *       with file-specific logic.</li>
 * </ol>
 *
 * No inheritance checks are required. Only observable behavior matters.
 *
 *
 * <h4>Coroutine-Based Yielding</h4>
 *
 * Python:
 *
 * <code>yield chunk</code>
 *
 * C++20:
 *
 * <code>co_yield chunk;</code>
 *
 * The return type is:
 *
 * <code>
 * jh::generator&lt;std::string&gt;
 * </code>
 *
 * This preserves Python-style lazy iteration semantics.
 *
 *
 * <h4>Error Handling</h4>
 *
 * The original Python code maps lower-level exceptions into higher-level
 * request-specific exceptions.
 *
 * For simplification, this C++ implementation removes the exception
 * conversion layer but fully supports <code>throw</code>.
 * Exception translation can be introduced later as a policy layer.
 *
 *
 * <h3>Conceptual C++ Structure</h3>
 *
 * @code
 * template&lt;typename Raw&gt;
 * requires StreamLike&lt;Raw&gt;
 * jh::generator&lt;std::string&gt;
 * make_generator(Raw&amp; raw, std::size_t chunk_size, bool&amp; consumed)
 * {
 *     while (...) {
 *         co_yield chunk;
 *     }
 *
 *     consumed = true;
 * }
 * @endcode
 *
 * This directly mirrors:
 *
 * @code{.py}
 * yield ...
 * self._content_consumed = True
 * @endcode
 *
 *
 * <h3>Historical Background and Practical Motivation</h3>
 *
 * This module was originally developed during an attempt to port a
 * third-party pure Python PCB processing library to C++20.
 *
 * The purpose of that migration effort was:
 *
 * <ol>
 *   <li>Replace CPU-based rendering with GPU-accelerated rendering
 *       using OpenCV CUDA.</li>
 *   <li>Maintain streaming and generator semantics equivalent to Python.</li>
 *   <li>Improve performance while preserving behavioral compatibility.</li>
 * </ol>
 *
 * The port was ultimately discontinued.
 *
 * The main difficulty was the Gerber format ecosystem:
 *
 * <ul>
 *   <li>Multiple historical revisions</li>
 *   <li>Vendor-specific extensions</li>
 *   <li>Implementation-defined interpretations</li>
 *   <li>Format behavior defined by industry practice rather than strict specification</li>
 * </ul>
 *
 * Although these variants cannot strictly be called "non-standard",
 * they are effectively fragmented across manufacturers.
 *
 * Full compatibility was therefore impractical.
 *
 *
 * <h3>Outcome</h3>
 *
 * Even though the PCB migration project did not succeed,
 * the coroutine-based generator abstraction proved valuable.
 *
 * The reusable <code>jh::generator</code> component was later extracted
 * and consolidated into the standalone <b>jh-toolkit</b> as one of the first
 * major features of the library.
 *
 * This demonstrates that:
 *
 * <ul>
 *   <li>Python-style lazy iteration patterns have real-world business value.</li>
 *   <li>C++20 coroutines can faithfully reproduce generator semantics.</li>
 *   <li>Behavior-based compile-time detection enables flexible integration.</li>
 * </ul>
 *
 *
 * <h3>Business Significance</h3>
 *
 * This design originated from a real industrial requirement involving:
 *
 * <ul>
 *   <li>High-performance PCB visualization</li>
 *   <li>GPU-based rendering acceleration</li>
 *   <li>Asynchronous conversion of <b>Gerber (PCB)</b> format data to <b>XML</b>
 *       and rendering of PCB optical primitives defined in the <b>XML tree</b> using <b>OpenCV</b>.
 *   </li>
 * </ul>
 *
 * Although the initial migration effort was halted,
 * the resulting generator abstraction remains a reusable architectural asset.
 *
 */

#include <concepts>
#include <istream>
#include <sstream>
#include <iostream>
#include <string_view>

namespace example::concepts {
    template<typename T>
    concept IStreamLike =requires(T &s, char *buf, std::streamsize n) {
        { s.read(buf, n) } -> std::convertible_to<std::istream &>;
        { s.gcount() } -> std::convertible_to<std::streamsize>;
    };

    template<typename T>
    concept SeekableStream =
    IStreamLike<T> && requires(T &s) {
        { s.seekg(0, std::ios::cur) };
        { s.tellg() } -> std::convertible_to<std::streampos>;
    };
}

namespace example::simulated {
    struct MemoryFile : std::istream {
        std::stringbuf buffer;

        MemoryFile(const std::string &data)
                : std::istream(&buffer),
                  buffer(data) {}
    };
}

namespace example {
    template<typename Raw>
    requires (concepts::IStreamLike<Raw>)
    jh::generator<std::string>
    generate_chunks(Raw &raw,
                    std::size_t chunk_size,
                    bool &content_consumed) {
        if constexpr (concepts::SeekableStream<Raw>) {
            // file-like stream logic (e.g., could add file-specific optimizations here)
            while (true) {
                std::string buffer(chunk_size, '\0');
                raw.read(buffer.data(), chunk_size);
                auto n = raw.gcount();
                if (n == 0)
                    break;
                buffer.resize(n);
                co_yield buffer;
            }
        } else {
            // normal stream mode (non-seekable)
            while (true) {
                std::string buffer(chunk_size, '\0');
                raw.read(buffer.data(), chunk_size);
                auto n = raw.gcount();
                if (n == 0)
                    break;
                buffer.resize(n);
                co_yield buffer;
            }
        }
        content_consumed = true;
    }

    void example_generate_chunks() {
        std::cout << "\n===== Industrial-style function-based generator "
                     "(mimicking Python Response.iter_content) =====\n\n";

        using namespace std::literals;

        bool consumed = false;

        // istringstream
        std::istringstream iss("HelloIStringStream");

        for (auto &&chunk: generate_chunks(iss, 5, consumed)) {
            std::cout << "[" << chunk << "]";
        }

        std::cout << "\nconsumed=" << (consumed ? "true"sv : "false"sv) << "\n\n";

        consumed = false;

        simulated::MemoryFile memFile("HelloMemoryFileStream");

        for (auto &&chunk: generate_chunks(memFile, 6, consumed)) {
            std::cout << "[" << chunk << "]";
        }

        std::cout << "\nconsumed=" << (consumed ? "true"sv : "false"sv) << "\n";

        std::cout << "\n===== end industrial-style function-based generator demo =====\n";

    }

    void example_lambda_gen() {
        std::cout << "\n===== Simple lambda-based generator demo =====\n\n";

        {
            // capture by value
            int x = 10;
            auto gen = [x]() -> jh::generator<int> {
                for (int i = 0; i < 3; ++i) {
                    co_yield x + i;
                }
            }();

            for (auto v: gen)
                std::cout << "by value: " << v << "\n";
        }

        std::cout << "\n";

        {
            // capture by value with mutable lambda (copy is mutable)
            int x = 20;
            auto gen = [x]() mutable -> jh::generator<int> {
                for (int i = 0; i < 3; ++i) {
                    x += 2;
                    co_yield x;
                }
            }();

            for (auto v: gen)
                std::cout << "by value mutable: " << v << "\n";

            std::cout << "original x unchanged: " << x << "\n";
        }

        std::cout << "\n";

        {
            // capture by reference
            int x = 30;
            auto gen = [&x]() -> jh::generator<int> {
                for (int i = 0; i < 3; ++i) {
                    x += 3;
                    co_yield x;
                }
            }();

            for (auto v: gen)
                std::cout << "by reference: " << v << "\n";

            std::cout << "original x modified: " << x << "\n";
        }

        std::cout << "\n";

        {
            // move capture (C++14 init capture with std::move)
            std::string s = "MoveCapture";
            auto gen = [str = std::move(s)]() -> jh::generator<int> {
                for (int i = 0; i < 3; ++i) {
                    co_yield static_cast<int>(str.size() + i);
                }
            }();

            for (auto v: gen)
                std::cout << "move capture size+: " << v << "\n";
            // We do NOT attempt to use 's' after moving, as it is in a valid but unspecified state.
        }

        std::cout << "\n";

        {
            // explicit initialized capture (C++14 init capture with expression)
            int base = 100;
            int factor = 5;

            auto gen = [result = base * factor]() -> jh::generator<int> {
                for (int i = 0; i < 3; ++i) {
                    co_yield result + i;
                }
            }();

            for (auto v: gen)
                std::cout << "explicit initialized capture: " << v << "\n";
        }

        std::cout << "\n===== end lambda-based generator demo =====\n";
    }

}

#include <numeric>
#include <ranges>

/**
 * @page Generator_Stepping Canonical Stepping & Send Semantics
 *
 * <h3>Bidirectional Coroutine Model</h3>
 *
 * <code>jh::async::generator&lt;T, U&gt;</code> is a C++20 coroutine-based
 * generator inspired by Python's:
 *
 * @code{.py}
 * Generator[T, U, R]
 * @endcode
 *
 * In the JH Toolkit, this is modeled as:
 *
 * @code
 * template&lt;typename T, typename U = jh::typed::monostate&gt;
 * class generator;
 * @endcode
 *
 * <ul>
 *   <li><b>T</b> — type yielded via <code>co_yield</code></li>
 *   <li><b>U</b> — type received via <code>co_await</code> (through <code>send()</code>)</li>
 * </ul>
 *
 * When:
 *
 * @code
 * U == jh::typed::monostate
 * @endcode
 *
 * the generator is <b>output-only</b> and compatible with range-style-for.
 *
 * When:
 *
 * @code
 * U != jh::typed::monostate
 * @endcode
 *
 * the generator becomes <b>interactive</b> (bidirectional) and must be
 * stepped manually.
 *
 *
 * <h3>Two-Phase Execution Contract</h3>
 *
 * Each iteration consists of exactly:
 *
 * <ul>
 *   <li>One input phase (<code>co_await U{}</code>)</li>
 *   <li>One output phase (<code>co_yield T</code>)</li>
 * </ul>
 *
 * External control APIs:
 *
 * <ul>
 *   <li><code>send(U)</code> — deliver input</li>
 *   <li><code>next()</code> — advance to next yield</li>
 *   <li><code>send_ite(U)</code> — canonical combined step</li>
 * </ul>
 *
 *
 * <h3>Canonical Stepping Form</h3>
 *
 * Although the API permits:
 *
 * @code
 * gen.send(x);
 * gen.next();
 *
 * // or
 *
 * gen.next();
 * gen.send(x);
 * @endcode
 *
 * the canonical and recommended form is:
 *
 * @code
 * gen.send_ite(x);
 * @endcode
 *
 * <b>send_ite()</b> expresses the full iteration contract:
 *
 * <ul>
 *   <li>Provide exactly one input</li>
 *   <li>Advance exactly one coroutine cycle</li>
 *   <li>Produce exactly one output</li>
 * </ul>
 *
 * It prevents partial stepping and makes coroutine pacing explicit.
 *
 * <h3>Output-Only Generators</h3>
 *
 * When <code>U == jh::typed::monostate</code>:
 *
 * <ul>
 *   <li><code>send()</code> is not used</li>
 *   <li><code>send_ite()</code> degenerates to <code>next()</code></li>
 *   <li>The generator supports <code>begin()</code>/<code>end()</code></li>
 * </ul>
 *
 * Example:
 *
 * @code
 * jh::async::generator<int> g();
 *
 * for (auto v : g()) {
 *     use(v);
 * }
 *
 * // or
 *
 * // use step by step
 * auto gen = g();
 *
 * while (gen.next() && additional_condition()) {
 *    auto v = gen.value().value();
 *    use(v);
 * }
 * @endcode
 *
 * @warning
 * For infinite generators, range-for is not recommended as it may lead to infinite loops.
 * use manual stepping with appropriate guards instead.
 *
 * <h3>Interactive Generators</h3>
 *
 * When <code>U != jh::typed::monostate</code>:
 *
 * <ul>
 *   <li>No <code>begin()</code>/<code>end()</code></li>
 *   <li>No range-for support</li>
 *   <li>Manual stepping required</li>
 * </ul>
 *
 * Example:
 *
 * @code
 * jh::async::generator<int, int> gen_await();
 *
 * auto gen = gen_await();
 *
 * if (gen.send_ite(5)) {
 *     auto v = gen.value().value();
 * }
 * @endcode
 *
 *
 * <h3>Value Access Semantics</h3>
 *
 * The member:
 *
 * @code
 * std::optional<T> value() const;
 * @endcode
 *
 * returns the last yielded value.
 *
 * Important properties:
 *
 * <ul>
 *   <li>Accessing <code>value()</code> does <b>not</b> consume it.</li>
 *   <li>Repeated calls are idempotent.</li>
 *   <li>The stored value changes only after successful stepping.</li>
 * </ul>
 *
 *
 * <h3>Correct Access Discipline</h3>
 *
 * When stepping is guarded:
 *
 * @code
 * if (gen.next()) {
 *     auto v = gen.value().value();
 * }
 *
 * if (gen.send_ite(x)) {
 *     auto v = gen.value().value();
 * }
 * @endcode
 *
 * it is guaranteed that the optional is engaged.
 *
 * In these guarded forms,
 * <code>value().value()</code> is the correct access.
 *
 *
 * <h3>Optional-aware Inspection</h3>
 *
 * Use:
 *
 * @code
 * gen.value().has_value();
 * gen.value().value_or(default_value);
 * @endcode
 *
 * only when:
 *
 * <ul>
 *   <li>No preceding guarded stepping exists</li>
 *   <li>State is inspected without advancing</li>
 * </ul>
 *
 * These forms are for defensive inspection — not normal iteration.
 *
 *
 * <h3>Design Rules</h3>
 *
 * <ul>
 *   <li><b>Use send_ite() as the canonical stepping form.</b></li>
 *   <li><b>Use value().value() after guarded stepping.</b></li>
 *   <li><b>Use has_value()/value_or() only for unguarded inspection.</b></li>
 *   <li><b>value() access never consumes the yield.</b></li>
 *   <li><b>Interactive generators require manual stepping.</b></li>
 * </ul>
 *
 */

namespace example {

    jh::generator<int> infinite_sequence() {
        // This generator yields an infinite sequence of integers starting from 0.
        // It should never be used with range-for without an external guard, as it is infinite.
        for (int i: std::views::iota(0))
            co_yield i;
    }

    jh::generator<int, int> interactive_counter() {
        int base = 0;
        while (true) {
            int delta = co_await int{};
            base += delta;
            co_yield base;
        }
    }

    void example_step_by_step() {
        std::cout << "\n===== Step-by-step generator control =====\n\n";

        // Output-only generator<int>
        {
            std::cout << "[Output-only stepping]\n";

            auto gen = infinite_sequence();
            std::size_t count = 0;
            while (gen.next() && count < 5) {
                auto v = gen.value().value();
                std::cout << "value = " << v << "\n";
                ++count;
            }
        }

        std::cout << "\n";

        // Interactive generator<int, int>
        {
            std::cout << "[Interactive stepping]\n";

            auto gen = interactive_counter();

            if (gen.send_ite(5)) {
                auto v = gen.value().value();
                std::cout << "after +5 = " << v << "\n";
            }

            if (gen.send_ite(3)) {
                auto v = gen.value().value();
                std::cout << "after +3 = " << v << "\n";
            }

            if (gen.send_ite(2)) {
                auto v = gen.value().value();
                std::cout << "after +2 = " << v << "\n";
            }
        }

        std::cout << "\n===== end step-by-step demo =====\n";
    }

} // namespace example

/**
 * @page Generator_Stream_Semantics jh::generator Semantic Model — Consuming Stream, Not a Range
 *
 * <h3>Purpose</h3>
 *
 * This page clarifies the semantic nature of
 * <code>jh::generator&lt;T, U&gt;</code>.
 *
 * The generator type is a coroutine-driven consuming stream.
 * It must not be interpreted as a standard C++ range abstraction.
 *
 *
 * <h3>Core Statement</h3>
 *
 * <code>jh::generator</code> represents:
 *
 * <ul>
 *   <li>A progressing coroutine execution context</li>
 *   <li>A single-pass stream of values</li>
 *   <li>A stateful and mutating computation</li>
 * </ul>
 *
 * It does <b>not</b> represent:
 *
 * <ul>
 *   <li>A stable view over underlying data</li>
 *   <li>A reusable sequence</li>
 *   <li>A forward-iterable container abstraction</li>
 * </ul>
 *
 *
 * <h3>Single-Pass Consumption Model</h3>
 *
 * Every successful advancement of a generator:
 *
 * <ul>
 *   <li>Resumes coroutine execution</li>
 *   <li>Mutates internal coroutine state</li>
 *   <li>Consumes progression history</li>
 * </ul>
 *
 * Once exhausted, the generator cannot be restarted.
 * Iteration is inherently destructive.
 *
 * This consumption property is fundamental to coroutine semantics
 * and is not an implementation detail.
 *
 *
 * <h3>Concept Matching Does Not Imply Range Semantics</h3>
 *
 * In some configurations, an output-only generator
 * may satisfy certain C++20 range concepts
 * (such as <code>std::ranges::input_range</code>).
 *
 * This syntactic compatibility does not redefine its semantic model.
 *
 * Concept satisfaction guarantees that required expressions exist.
 * It does not guarantee:
 *
 * <ul>
 *   <li>Referential stability</li>
 *   <li>Reusability</li>
 *   <li>Iterator independence</li>
 * </ul>
 *
 * A generator remains a consuming stream
 * even if it matches input-range concepts.
 *
 *
 * <h3>Non-Forward Semantics</h3>
 *
 * A generator:
 *
 * <ul>
 *   <li>Is single-pass only</li>
 *   <li>Does not provide forward iteration guarantees</li>
 *   <li>Does not allow duplication of iteration state</li>
 * </ul>
 *
 * Iterator copies (when technically possible)
 * must not be interpreted as independent traversal cursors.
 *
 *
 * <h3>Recommended Abstraction for Range Semantics</h3>
 *
 * When true range semantics are required,
 * the correct abstraction is:
 *
 * <ul>
 *   <li><code>jh::generator_range&lt;T&gt;</code></li>
 * </ul>
 *
 * A <code>generator_range</code> stores a factory
 * capable of constructing a fresh generator instance
 * upon each call to <code>begin()</code>.
 *
 * This restores:
 *
 * <ul>
 *   <li>Proper input-range modeling</li>
 *   <li>Well-defined iteration boundaries</li>
 *   <li>Safe interoperability with C++20 views</li>
 * </ul>
 *
 * The resulting range remains input-only
 * and intentionally does not model forward-range.
 *
 *
 * <h3>Design Rule</h3>
 *
 * <ul>
 *   <li>Treat <code>jh::generator</code> as a stream abstraction.</li>
 *   <li>Do not treat it as a reusable container.</li>
 *   <li>Do not infer semantic guarantees from concept matching.</li>
 *   <li>Use <code>generator_range</code> when range semantics are required.</li>
 * </ul>
 *
 * In summary:
 *
 * <ul>
 *   <li><code>generator</code> expresses coroutine state progression.</li>
 *   <li><code>generator_range</code> expresses range abstraction.</li>
 * </ul>
 */

namespace example {

    void example_generator_range() {

        std::cout << "\n===== Using jh::to_range to create a repeatable generator range =====\n\n";
        // example of using jh::to_range to create a repeatable range from a generator factory (lambda)
        auto range = jh::to_range([] {
            return infinite_sequence();
        });

        std::cout << "Range created from generator factory lambda.\n";
        std::cout << "Iterating over the first 10 values:\n";

        // iterate over the first 10 values
        for (int v: range | std::views::take(10)) {
            std::cout << v << " ";
        }

        std::cout << "\nResetting and iterating again...\n";
        std::cout << "Iterating over the first 5 values again:\n";

        // re-iterate over the first 5 values again, demonstrating repeatability
        for (int v: range | std::views::take(5)) {
            std::cout << v << " ";
        }

        std::cout << "\n===== end generator_range demo =====\n";
    }
}

/**
 * @page Generator_Namespace_and_Policy jh::generator Namespace Elevation and Execution Policy
 *
 * <h3>Namespace Positioning</h3>
 *
 * <p>
 * <code>jh::generator</code> is re-exported into the root <code>jh::</code>
 * namespace for convenience. Its canonical implementation resides in
 * <code>jh::async::generator</code>.
 * </p>
 *
 * <p>
 * The elevation into <code>jh::</code> does not alter semantics.
 * It remains architecturally part of the asynchronous subsystem.
 * </p>
 *
 *
 * <h3>Shared Philosophy with jh::async</h3>
 *
 * <p>
 * <code>jh::generator</code> follows the same design discipline as all
 * components under <code>jh::async</code>:
 * </p>
 *
 * <ul>
 *   <li>Asynchronous mechanisms handle asynchronous coordination.</li>
 *   <li>Synchronous aggregation and distribution remain external.</li>
 * </ul>
 *
 * <p>
 * The generator is not a workflow engine.
 * It is a coroutine stepping abstraction.
 * </p>
 *
 *
 * <h3>One-Round Execution Contract</h3>
 *
 * <p>
 * Each logical iteration consists of exactly:
 * </p>
 *
 * <ul>
 *   <li>One permitted <code>co_await</code> (input phase)</li>
 *   <li>One permitted <code>co_yield</code> (output phase)</li>
 * </ul>
 *
 * <p>
 * This models a single bidirectional exchange step.
 * </p>
 *
 * <p>
 * Multiple awaits per round or multiple yields per round are intentionally
 * not part of the supported execution discipline.
 * </p>
 *
 *
 * <h3>No Internal Multi-Dispatch or Multi-Collection</h3>
 *
 * <p>
 * If multiple values must be gathered before processing,
 * the aggregation must be completed synchronously
 * by the caller before invoking the generator.
 * </p>
 *
 * <p>
 * If a yielded value must be distributed to multiple destinations,
 * the fan-out must be handled externally after retrieval.
 * </p>
 *
 * <p>
 * The generator does not manage:
 * </p>
 *
 * <ul>
 *   <li>Internal batching logic</li>
 *   <li>Multi-recipient dispatch</li>
 *   <li>Implicit synchronization barriers</li>
 * </ul>
 *
 *
 * <h3>Structured Data as Exchange Unit</h3>
 *
 * <p>
 * A single input or output may represent structured data.
 * </p>
 *
 * <p>
 * For example:
 * </p>
 *
 * <ul>
 *   <li><code>co_await std::tuple&lt;A, B, C&gt;</code></li>
 *   <li><code>co_yield MyDataClass</code></li>
 * </ul>
 *
 * <p>
 * The responsibility for assembling the tuple or struct
 * lies with the caller before invoking <code>send()</code>.
 * </p>
 *
 * <p>
 * Likewise, any downstream distribution of yielded data
 * must be performed outside the generator.
 * </p>
 *
 *
 * <h3>Copy-Based Value Sharing</h3>
 *
 * <p>
 * The generator assumes that exchanged data is
 * pure data rather than semantic or identity-sensitive objects.
 * </p>
 *
 * <p>
 * Value copying during stepping is therefore the default model.
 * </p>
 *
 * <p>
 * This ensures:
 * </p>
 *
 * <ul>
 *   <li>Isolation between coroutine state and caller state</li>
 *   <li>Deterministic lifetime boundaries</li>
 *   <li>Reduced semantic coupling</li>
 * </ul>
 *
 *
 * <h3>Handling Heavier Objects</h3>
 *
 * <p>
 * If semantic objects must be exchanged,
 * the recommended approach is to wrap them in
 * <code>std::shared_ptr&lt;T&gt;</code>.
 * </p>
 *
 * <p>
 * This provides:
 * </p>
 *
 * <ul>
 *   <li>Cheap copy semantics</li>
 *   <li>Explicit lifetime control</li>
 *   <li>User-defined destruction behavior</li>
 * </ul>
 *
 * <p>
 * The generator itself does not enforce ownership policies.
 * Ownership strategy remains a caller-level decision.
 * </p>
 *
 *
 * <h3>Design Rule Summary</h3>
 *
 * <ul>
 *   <li><code>jh::generator</code> is semantically an asynchronous component.</li>
 *   <li>Namespace elevation does not change its subsystem identity.</li>
 *   <li>Each round permits exactly one <code>co_await</code> and one <code>co_yield</code>.</li>
 *   <li>Multi-value coordination must be externally synchronized.</li>
 *   <li>Data exchange is copy-oriented and value-centric.</li>
 *   <li>Use <code>std::shared_ptr</code> when object semantics are required.</li>
 * </ul>
 *
 */

#include <stdexcept>
#include <format>

namespace example::simulated {
    struct Employee {
        std::string age_str;       // width 3, left padded with space
        std::string service_str;   // width 3, left padded with space
        std::string name_str;      // width 15, left padded with space
    };

    inline std::string left_pad(const std::string &s, std::size_t width) {
        if (s.size() >= width) return s;
        return std::string(width - s.size(), ' ') + s;
    }
}

namespace example {
    jh::generator<std::shared_ptr<simulated::Employee>,
            std::tuple<unsigned int, unsigned int, std::string>>
    gen_person() {
        while (true) {

#if IS_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
            // NOLINTNEXTLINE(readability-static-accessed-through-instance)
            auto [age, service, name] =
                    co_await std::tuple<unsigned int, unsigned int, std::string>{};
#if IS_GCC
#pragma GCC diagnostic pop
#endif
            auto person = std::make_shared<simulated::Employee>();

            if (age < 18) {
                throw std::runtime_error(
                        std::format("Age must be at least 18. name: {}, age: {}", name, age)
                );
            }

            // This demonstration assumes the generator is handling a critical business flow that doesn't
            // allow abnormal input, and also shows how to catch exceptions
            // (the generator is done if an exception crosses its boundaries).

            // If you actually need it to run continuously, you can change the output to
            // std::shared_ptr<std::variant<Employee, ExceptionMsg>> or more brutally return nullptr.

            person->age_str = simulated::left_pad(std::to_string(age), 3);
            person->service_str = simulated::left_pad(std::to_string(service), 3);
            person->name_str = simulated::left_pad(name, 15);

            co_yield person;
        }
    }

    void example_outer_ctrled_multi_input() {

        using namespace std::literals;

        std::cout << "\n===== Outer-controlled multi-input demo =====\n\n";

        std::istringstream age_stream("25 41 30 8");
        std::istringstream service_stream("3 12 10 1");
        std::istringstream name_stream("Alice Bob Charlie John");

        // Here we demonstrate an interactive generator that takes multiple inputs
        // as a tuple and produces a structured output.

        // The generator is expected to throw an exception when the age is below 18 (for John),
        // which we will catch and display.

        auto gen = gen_person();

        std::cout << "Employee data input streams prepared. Starting generator iteration...\n";

        try {
            while (true) {

                unsigned int age;
                unsigned int service;
                std::string name;

                if (!(age_stream >> age)) break;
                if (!(service_stream >> service)) break;
                if (!(name_stream >> name)) break;

                std::tuple<unsigned int, unsigned int, std::string> input{
                        age, service, name
                };

                if (!gen.send_ite(input))
                    break;

                auto person_ptr = gen.value().value();

                std::cout
                        << "[" << person_ptr->age_str
                        << "] [" << person_ptr->service_str
                        << "] [" << person_ptr->name_str
                        << "]\n";
            }
        }
        catch (const std::exception &e) {
            std::cout << "\nGenerator failed (as expected for invalid input) with error: "
                      << e.what() << "\n";
        }

        // note: the try-catch works exactly as synchronous exception handling,
        // even though the exception is thrown from within the generator's coroutine context.

        // the error is defined before yielding,
        // so it is thrown at the point of send_ite() that triggers the relevant coroutine execution step.
        // "John" is not processed at all, as the exception occurs before any yield happens for that input.

        std::cout << "generator is done: " << (gen.done() ? "yes"sv : "no"sv) << "\n";
        // generator is done: yes, as the exception unwound the coroutine and marked it as done.

        std::cout << "\n===== end multi-input demo =====\n";
    }
}

/**
 * @page Generator_Make_Generator_Sequence_vs_Range make_generator() — Sequence vs Range Overloads
 *
 * <h3>Original Design Motivation</h3>
 *
 * The original purpose of <code>make_generator()</code> was to align with
 * the Python generator expression:
 *
 * @code{.py}
 * (x for x in seq)
 * @endcode
 *
 * The semantic meaning is:
 *
 * <ul>
 *   <li>Transform traversal into lazy evaluation.</li>
 *   <li>Do not modify the underlying sequence.</li>
 *   <li>Do not consume the source container.</li>
 * </ul>
 *
 *
 * <h3>Overload Structure</h3>
 *
 * <h4>Duck-typed jh::concepts::sequence overload</h4>
 *
 * @code
 * template&lt;concepts::sequence SeqType&gt;
 * requires (!std::ranges::range&lt;SeqType&gt;)
 * generator&lt;concepts::sequence_value_t&lt;SeqType&gt;&gt;
 * make_generator(const SeqType& seq);
 * @endcode
 *
 * <p>
 * A type qualifies if:
 * </p>
 *
 * <ul>
 *   <li>It provides <code>begin()</code> and <code>end()</code>.</li>
 *   <li>It supports range-style-for traversal.</li>
 *   <li>Traversal does not consume or mutate the container.</li>
 * </ul>
 *
 * <p>
 * Concept relationship clarification:
 * </p>
 *
 * <ul>
 *   <li>
 *   It is broader than the subset of <code>std::ranges::range</code>
 *   representing non-consuming, container-like ranges.
 *   </li>
 *   <li>
 *   It intentionally excludes consuming streams.
 *   </li>
 * </ul>
 *
 * <p>
 * This overload always takes <code>const SeqType&</code>, even if the caller
 * provides a non-const object. Const is enforced to guarantee non-consuming,
 * non-mutating traversal and to avoid unpredictable behavior in legacy containers.
 * </p>
 *
 * <h4>std::ranges::range overload</h4>
 *
 * @code
 * template&lt;std::ranges::range R&gt;
 * generator&lt;std::ranges::range_value_t&lt;R&gt;&gt;
 * make_generator(R&& rng);
 * @endcode
 *
 * <p>
 * This overload accepts a universal reference and forwards the range.
 * </p>
 *
 * <p>
 * The <code>R&&</code> parameter follows normal reference collapsing rules:
 * </p>
 *
 * <ul>
 *   <li>Non-const lvalue → deduces to <code>R&</code></li>
 *   <li>Const lvalue → deduces to <code>const R&</code></li>
 *   <li>Rvalue → deduces to <code>R&&</code></li>
 * </ul>
 *
 * <p>
 * Therefore constness is preserved automatically.
 * </p>
 *
 * <p>
 * Unlike the sequence overload, this version:
 * </p>
 *
 * <ul>
 *   <li>Supports forwarded rvalue ranges.</li>
 *   <li>Supports lazy views.</li>
 *   <li>Allows ownership transfer into the coroutine.</li>
 *   <li>Does not enforce non-consumption.</li>
 * </ul>
 *
 *
 * <h3>Iterator Invalidation Rule</h3>
 *
 * <p>
 * When using the sequence overload, the generator internally stores
 * and advances an iterator.
 * </p>
 *
 * <p>
 * For containers such as <code>std::vector</code> or <code>std::deque</code>,
 * any operation that invalidates iterators (e.g. changing container size)
 * must not be performed while the generator is still iterating.
 * </p>
 *
 * <p>
 * The generator conceptually behaves as if it holds:
 * </p>
 *
 * @code
 * auto it = seq.begin();
 * @endcode
 *
 *
 * <h3>Design Summary</h3>
 *
 * <ul>
 *   <li>Sequence overload enforces non-consuming traversal via const reference.</li>
 *   <li>Range overload forwards and preserves value category.</li>
 *   <li>Constness is preserved through reference collapsing in the T&& overload.</li>
 *   <li>Sequence is broader than non-consuming ranges but excludes consuming streams.</li>
 *   <li>Iterator invalidation remains the caller's responsibility.</li>
 * </ul>
 */

namespace example {

    void example_make_generator() {

        std::cout << "\n===== make_generator demo =====\n\n";
        {
            // lvalue std::vector
            // Calling jh::make_generator(vec);
            // results in a non-consuming traversal.
            // In this context the deduced type behaves as
            // const std::vector<int>&.

            std::vector<int> vec{1, 2, 3, 4, 5};
            auto gen_lvalue = jh::make_generator(vec);

            std::cout << "(lvalue std::vector)\n";
            while (gen_lvalue.next()) {
                std::cout << gen_lvalue.value().value() << " ";
            }
            std::cout << "\n";
        }
        {
            // rvalue std::vector (recommended explicit move pattern)
            // Do NOT write: jh::make_generator(std::vector<int>{...});
            // On some GCC 14+ front-end configurations,
            // interaction between temporary materialization and coroutines
            // may lead to dangling lifetime issues.
            //
            // Instead, declare and then move.
            // In fact, jh::make_generator(std::vector<int>{...}); is semantically correct
            // and should work in compliant compilers (and does work in Clang and GCC13),
            // but the explicit move pattern is more robust across
            // different compiler versions and configurations.

            std::vector<int> tmp{100, 200, 300};
            auto gen_rvalue = jh::make_generator(std::move(tmp));

            std::cout << "(rvalue std::vector via std::move)\n";
            while (gen_rvalue.next()) {
                std::cout << gen_rvalue.value().value() << " ";
            }
            std::cout << "\n";
        }
        {
            // std::views::iota (lazy range)
            //
            // Avoid directly passing a temporary view into make_generator.
            // Declare the view first to ensure stable lifetime.

            auto iota_view = std::views::iota(10, 15);
            auto gen_iota = jh::make_generator(iota_view);

            std::cout << "(std::views::iota lvalue view)\n";
            while (gen_iota.next()) {
                std::cout << gen_iota.value().value() << " ";
            }
            std::cout << "\n";
        }
        std::cout << "\n===== end make_generator demo =====\n";
    }

}

/**
 * @page Generator_Eager_Materialization Eager Materialization — Python-style list(gen) Semantics
 *
 * <h3>Overview</h3>
 *
 * Python supports eager materialization of generators:
 *
 * @code{.py}
 * list(gen)
 * @endcode
 *
 * This converts a lazy generator into a concrete container.
 *
 * The JH Toolkit provides equivalent eager utilities:
 *
 * <ul>
 *   <li><code>jh::async::to_vector()</code></li>
 *   <li><code>jh::async::to_deque()</code></li>
 * </ul>
 *
 * These utilities consume the generator to completion
 * and collect all yielded values into a container.
 *
 *
 * <h3>Primary Design Recommendation</h3>
 *
 * The most recommended overload is:
 *
 * @code
 * template&lt;typename T&gt;
 * std::deque&lt;T&gt; to_deque(generator&lt;T&gt;& gen);
 * @endcode
 *
 * or
 *
 * @code
 * template&lt;typename T&gt;
 * std::vector&lt;T&gt; to_vector(generator&lt;T&gt;& gen);
 * @endcode
 *
 * This applies to <b>output-only generators</b>
 * (<code>U == typed::monostate</code>).
 *
 *
 * <h3>Why Output-Only Is Preferred</h3>
 *
 * When constructing a generator — especially via lambda —
 * external state can be captured directly:
 *
 * @code
 * int base = 5;
 * auto gen = [base]() -> jh::generator&lt;int&gt; {
 *     for (int i = 0; i < 3; ++i)
 *         co_yield base + i;
 * }();
 *
 * auto vec = jh::async::to_vector(gen);
 * @endcode
 *
 * In modern C++, external data should be captured
 * rather than forced through <code>send()</code>.
 *
 * Interactive stepping is intended for:
 *
 * <ul>
 *   <li>Externally driven workflows</li>
 *   <li>Streaming systems</li>
 *   <li>Real-time coordination</li>
 * </ul>
 *
 * It is <b>not</b> intended as a bulk parameter injection mechanism.
 *
 *
 * <h3>Interactive Overloads</h3>
 *
 * Additional overloads exist:
 *
 * <ul>
 *   <li>Fixed input value at every step</li>
 *   <li>Range of input values</li>
 * </ul>
 *
 * These allow:
 *
 * @code
 * auto vec = jh::async::to_vector(gen, fixed_input);
 * auto vec = jh::async::to_vector(gen, input_range);
 * @endcode
 *
 * However:
 *
 * <ul>
 *   <li>They assume that input data is already available.</li>
 *   <li>They implicitly batch interactive behavior.</li>
 *   <li>They obscure the intended coroutine stepping model.</li>
 * </ul>
 *
 * <h3>Production Guidance</h3>
 *
 * These overloads are retained as a <b>development fallback</b>.
 *
 * They are useful:
 *
 * <ul>
 *   <li>During early experimentation</li>
 *   <li>When prototyping generator logic</li>
 *   <li>When refactoring synchronous code into coroutine form</li>
 * </ul>
 *
 * In production-grade systems, interactive generators
 * should be driven explicitly by external control flow,
 * rather than by bulk eager injection of inputs.
 *
 *
 * <h3>Container Policy</h3>
 *
 * Two container targets are provided:
 *
 * <ul>
 *   <li><code>std::vector</code></li>
 *   <li><code>std::deque</code></li>
 * </ul>
 *
 * Rationale:
 *
 * <ul>
 *   <li>Both are high-performance contiguous or segmented linear containers.</li>
 *   <li>They provide amortized O(1) <code>push_back()</code>.</li>
 *   <li>They are suitable for modern high-throughput systems.</li>
 * </ul>
 *
 * <h4>Why Not std::list?</h4>
 *
 * <ul>
 *   <li>Poor cache locality</li>
 *   <li>High fragmentation</li>
 *   <li>Significant allocator overhead</li>
 *   <li>Inferior performance on modern CPUs</li>
 * </ul>
 *
 * Linked lists are not suitable for modern high-performance
 * coroutine pipelines and are intentionally excluded.
 *
 *
 * <h3>Semantic Warning</h3>
 *
 * Eager materialization:
 *
 * <ul>
 *   <li>Consumes the generator.</li>
 *   <li>Destroys lazy evaluation benefits.</li>
 *   <li>May allocate large memory blocks.</li>
 * </ul>
 *
 * It should be used intentionally,
 * not as a default pattern.
 *
 *
 * <h3>Design Summary</h3>
 *
 * <ul>
 *   <li>Use output-only generators whenever possible.</li>
 *   <li>Prefer <code>to_vector(gen)</code> or <code>to_deque(gen)</code>.</li>
 *   <li>Avoid bulk input injection in production systems.</li>
 *   <li>Interactive stepping should remain externally controlled.</li>
 *   <li>Only vector and deque are supported by design.</li>
 * </ul>
 *
 */

namespace example {

    void example_eager_materialization() {

        std::cout << "\n===== Eager materialization demo =====\n\n";
        {
            // Capture a single external parameter.
            //
            // Philosophy:
            // Instead of using an interactive generator and repeatedly sending
            // a constant value, capture the parameter directly at construction time.
            // This keeps the generator output-only and semantically clean.

            int base = 10;
            auto gen = [base]() -> jh::generator<int> {
                for (int i = 0; i < 5; ++i) {
                    co_yield base + i;
                }
            }();

            auto vec = jh::async::to_vector(gen);
            std::cout << "[Capture single parameter] ";
            for (auto v: vec)
                std::cout << v << " ";
            std::cout << "\n";
        }

        std::cout << "\n";

        {
            // Capture an external range (std::vector).
            //
            // Philosophy:
            // If the generator logic depends on a known sequence,
            // capture the container and iterate internally.
            // Do not push the data back through send().
            //
            // The generator remains output-only and externally deterministic.

            std::vector<int> data{1, 2, 3, 4, 5};

            // Use reference to avoid possible dangling (copying leads to a temporary
            // that may be destroyed before the generator is done).
            auto gen = [&data]() -> jh::generator<int> {
                for (auto v: data) {
                    co_yield v * 3;
                }
            }();

            auto vec = jh::async::to_vector(gen);
            std::cout << "[Capture range] ";
            for (auto v: vec)
                std::cout << v << " ";
            std::cout << "\n";
        }

        std::cout << "\n";

        {
            // Capture multiple external values (structured configuration).
            //
            // Philosophy:
            // If multiple parameters are required, assemble them
            // into a structured external context and capture that context.
            //
            // This avoids turning the generator into a parameter-injection loop.
            int multiplier = 2;
            int offset = 5;

            auto gen = [multiplier, offset]() -> jh::generator<int> {
                for (int i = 0; i < 4; ++i) {
                    co_yield i * multiplier + offset;
                }
            }();

            auto dq = jh::async::to_deque(gen);
            std::cout << "[Capture structured config] ";
            for (auto v: dq)
                std::cout << v << " ";
            std::cout << "\n";
        }

        std::cout << "\n===== end eager materialization demo =====\n";
    }
}


/**
 * @page Generator_Range_Pipe_Interaction
 *
 * @brief Interaction model between jh::generator_range and the JH ranges framework.
 *
 * <h3>Overview</h3>
 *
 * <p>
 * <code>jh::generator_range&lt;T&gt;</code> is a range façade constructed
 * from a generator factory. Each call to <code>begin()</code> creates
 * a new <code>jh::generator&lt;T&gt;</code>, restoring input-range
 * semantics over a coroutine-based stream.
 * </p>
 *
 * <p>
 * This enables structural compatibility with:
 * </p>
 *
 * <ul>
 *   <li><code>&lt;jh/views&gt;</code></li>
 *   <li><code>&lt;jh/ranges_ext&gt;</code></li>
 *   <li>Standard <code>&lt;ranges&gt;</code> adaptors</li>
 * </ul>
 *
 *
 * <h3>Iterator Semantics</h3>
 *
 * <p>
 * The iterator type produced by <code>generator_range</code> is:
 * </p>
 *
 * <ul>
 *   <li>Single-pass</li>
 *   <li>Non-copyable</li>
 * </ul>
 *
 * <p>
 * The restriction is intentional. A generator instance represents a
 * stateful coroutine. If iterator copying were permitted, two iterators
 * could attempt to advance the same coroutine instance, resulting in
 * ambiguous execution order and undefined semantics.
 * </p>
 *
 * <p>
 * Therefore, the iterator enforces unique ownership of coroutine
 * progression.
 * </p>
 *
 *
 * <h3>Pipeline Interaction Rule</h3>
 *
 * <p>
 * Many lazy range adaptors internally copy iterators. Because
 * <code>generator_range</code> prohibits copying, it cannot safely
 * participate in arbitrary lazy pipelines.
 * </p>
 *
 * <p>
 * To integrate a generator-driven range into a full adaptor chain,
 * an explicit eager materialization boundary is required.
 * </p>
 *
 *
 * <h3>Materialization Boundary Pattern</h3>
 *
 * @code
 * auto range =
 *     jh::to_range([] {
 *         some_finite_generator();
 *     })
 *     | jh::ranges::collect<std::vector<long>>();
 * @endcode
 *
 * <p>
 * The evaluation proceeds as follows:
 * </p>
 *
 * <ol>
 *   <li><code>collect&lt;std::vector&lt;&gt;&gt;</code> eagerly consumes the
 *       single-pass generator.</li>
 *   <li>The result becomes a stable, multi-pass container.</li>
 * </ol>
 *
 * <p>
 * After this boundary, the data may freely interact with:
 * </p>
 *
 * <ul>
 *   <li>All JH view adaptors</li>
 *   <li>Structural collectors</li>
 *   <li>Container construction adaptors</li>
 * </ul>
 *
 *
 * <h3>Role of jh::ranges::collect</h3>
 *
 * <p>
 * <code>jh::ranges::collect&lt;C&gt;</code> defines an explicit evaluation
 * boundary within a pipeline. It consumes a range and materializes it
 * into a concrete container <code>C</code>.
 * </p>
 *
 * <p>
 * In the context of <code>generator_range</code>, it provides the
 * semantic stabilization necessary to transition from a coroutine-driven
 * single-pass stream to a conventional container-based range model.
 * </p>
 *
 *
 * <h3>Design Summary</h3>
 *
 * <ul>
 *   <li><code>generator_range</code> restores range syntax over generators.</li>
 *   <li>Its iterator is single-pass and intentionally non-copyable.</li>
 *   <li>Lazy adaptor stacks may require iterator copying and are therefore restricted.</li>
 *   <li>An eager materialization boundary must be introduced before deep pipeline interaction.</li>
 *   <li><code>collect&lt;&gt;</code> provides the recommended boundary mechanism.</li>
 * </ul>
 *
 */

#include <jh/ranges_ext>
#include <jh/views>
#include <memory_resource>
#include <jh/runtime_arr>

namespace example::simulated {
    struct mid {
        long index;
        int id;
        std::string name;
        int value;

        mid(long i, int id_, std::string n, int v)
                : index(i), id(id_), name(std::move(n)), value(v) {}

        [[nodiscard]] std::pair<int, std::string> as_pair() const {
            return {static_cast<int>(index), name + ":(" + std::to_string(value) + ", " + std::to_string(id) + ")"};
        }
    };
}

namespace example {

    jh::generator<long> finite_long_sequence(long end = std::numeric_limits<long>::max()) {
        // This generator yields an infinite sequence of integers starting from 0.
        // It should never be used with range-for without an external guard, as it is infinite.
        for (std::weakly_incrementable auto i: std::views::iota(static_cast<long>(0))) {
            if (i >= end)
                break;
            co_yield static_cast<long>(i);
        }
    }

    void example_range_pipe() {

        std::cout << "\n===== generator_range × views × ranges_ext demo =====\n\n";

        jh::runtime_arr<std::string> names(3);
        jh::runtime_arr<int> values(3);

        for (auto [i, x]: std::views::iota(0, 3) | jh::ranges::views::enumerate(1))
            x = static_cast<int>(i * 10);

        for (auto [i, x]: values | jh::ranges::views::enumerate())
            x = static_cast<int>((i + 1) * 100);

        names[0] = "Alice";
        names[1] = "Bob";
        names[2] = "Carol";
        long end_point = 5;

        std::pmr::monotonic_buffer_resource pool;

        // [id] -> [(index, id)] -> [((index, id), name, value)] -> [(index, id, name, value)] -> [mid] -> [pair]

        auto pmr_map = jh::to_range([&end_point] {
            return finite_long_sequence(end_point);
        })
                       | jh::ranges::collect<std::vector<long>>()
                       | jh::ranges::views::enumerate(100)
                       | jh::ranges::views::zip_pipe(names, values)
                       | jh::ranges::views::flatten()
                       | jh::ranges::collect<std::vector<simulated::mid>>()
                       | jh::ranges::views::transform(&simulated::mid::as_pair)
                       | jh::ranges::to<std::pmr::unordered_map<int, std::string>>(
                0,
                std::hash<int>{},
                std::equal_to<int>{},
                std::pmr::polymorphic_allocator<std::pair<const int, std::string>>(&pool));
        std::cout << "Resulting pmr::unordered_map contents:\n";

        for (auto &it: pmr_map) {
            std::cout << "  key = " << it.first
                      << ", value = " << it.second << "\n";
        }

        std::cout << "\n===== end generator_range pipeline demo =====\n";
    }

}

int main() {
    example::example_generate_chunks();
    example::example_lambda_gen();
    example::example_step_by_step();
    example::example_generator_range();
    example::example_outer_ctrled_multi_input();
    example::example_make_generator();
    example::example_eager_materialization();
    example::example_range_pipe();
    return 0;
}
