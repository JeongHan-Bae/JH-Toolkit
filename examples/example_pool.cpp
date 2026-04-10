/**
 * @file example_pool.cpp
 * @brief Example demonstrating the usage of <code>&lt;jh/pool&gt;</code>.
 *
 * <p>
 * This file demonstrates how to use the pool-focused aggregate header:
 * </p>
 *
 * @code
 * #include &lt;jh/pool&gt;
 * @endcode
 *
 * <p>
 * The examples cover the major user-facing and pool-related interfaces:
 * </p>
 *
 * <ul>
 *   <li><code>jh::observe_pool&lt;T&gt;</code> (content-based pointer interning).</li>
 *   <li><code>jh::conc::pointer_pool&lt;T, Hash, Eq&gt;</code> (customized pointer interning).</li>
 *   <li><code>jh::resource_pool_set&lt;Key&gt;</code> (set-like key interning).</li>
 *   <li><code>jh::resource_pool&lt;Key, Value&gt;</code> (map-like key/value interning).</li>
 * </ul>
 *
 * <h3>Philosophy Summary</h3>
 *
 * <p>
 * Pool families in JH Toolkit separate two identity models:
 * </p>
 *
 * <ol>
 *   <li><b>Object-intrinsic identity</b>: use pointer-based pools
 *       (<code>pointer_pool</code>, <code>observe_pool</code>).</li>
 *   <li><b>External-key identity</b>: use contiguous key-based pools
 *       (<code>resource_pool</code>, <code>resource_pool_set</code>).</li>
 * </ol>
 *
 * <p>
 * This example intentionally uses <code>&lt;jh/pool&gt;</code> only, without
 * <code>&lt;jh/concurrency&gt;</code>, to demonstrate the pool-only include path.
 * </p>
 */

#include <jh/pool>
#include "ensure_output.h"

#if IS_WINDOWS
static EnsureOutput ensure_output_setup;
#endif

#include <chrono>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

namespace example {

    struct Symbol final {
        std::string text;
        int tag;

        Symbol(std::string t, const int x) : text(std::move(t)), tag(x) {}

        [[nodiscard]] std::size_t hash() const noexcept {
            return std::hash<std::string>{}(text) ^ (std::hash<int>{}(tag) << 1U);
        }

        bool operator==(const Symbol &rhs) const noexcept {
            return tag == rhs.tag && text == rhs.text;
        }
    };

    struct HeavyNode final {
        int id;
        std::string name;

        HeavyNode(const int i, std::string n) : id(i), name(std::move(n)) {}

        HeavyNode(const HeavyNode &) = delete;
        HeavyNode &operator=(const HeavyNode &) = delete;
        HeavyNode(HeavyNode &&) = delete;
        HeavyNode &operator=(HeavyNode &&) = delete;

        [[nodiscard]] std::size_t object_hash() const noexcept {
            return std::hash<int>{}(id) ^ (std::hash<std::string>{}(name) << 1U);
        }

        bool operator==(const HeavyNode &rhs) const noexcept {
            return id == rhs.id && name == rhs.name;
        }
    };

    struct HeavyNodeWeakHash final {
        std::size_t operator()(const std::weak_ptr<HeavyNode> &w) const noexcept {
            if (const auto p = w.lock()) {
                return p->object_hash();
            }
            return 0U;
        }
    };

    struct HeavyNodeWeakEq final {
        bool operator()(const std::weak_ptr<HeavyNode> &lhs,
                        const std::weak_ptr<HeavyNode> &rhs) const noexcept {
            const auto l = lhs.lock();
            const auto r = rhs.lock();
            if (!l || !r) {
                return false;
            }
            return *l == *r;
        }
    };

    using custom_pointer_pool = jh::conc::pointer_pool<HeavyNode, HeavyNodeWeakHash, HeavyNodeWeakEq>;

    void observe_pool_basic_demo() {
        std::cout << "\n===== observe_pool basic =====\n";

        jh::observe_pool<Symbol> pool;

        auto a = pool.acquire("alpha", 7);
        auto b = pool.acquire("alpha", 7);
        auto c = pool.acquire("beta", 7);

        std::cout << "a == b (same canonical object): " << std::boolalpha << (a == b) << "\n";
        std::cout << "a != c (different content): " << std::boolalpha << (a != c) << "\n";
        std::cout << "pool.size() before reset: " << pool.size() << "\n";

        a.reset();
        b.reset();
        c.reset();

        std::cout << "pool.size() after reset (expired weak entries remain): "
                  << pool.size() << "\n";

        pool.cleanup();
        std::cout << "pool.size() after cleanup: " << pool.size() << "\n";
    }

    void pointer_pool_basic_demo() {
        std::cout << "\n===== pointer_pool basic (custom Hash/Eq) =====\n";

        custom_pointer_pool pool;

        auto n1 = pool.acquire(10, "socket-A");
        auto n2 = pool.acquire(10, "socket-A");
        auto n3 = pool.acquire(11, "socket-A");

        std::cout << "n1 == n2 (deduplicated): " << std::boolalpha << (n1 == n2) << "\n";
        std::cout << "n1 != n3 (different identity): " << std::boolalpha << (n1 != n3) << "\n";
        std::cout << "pool.size() before reset: " << pool.size() << "\n";

        n1.reset();
        n2.reset();
        n3.reset();

        pool.cleanup_shrink();
        std::cout << "pool.size() after cleanup_shrink: " << pool.size() << "\n";
        std::cout << "pool.capacity() after cleanup_shrink: " << pool.capacity() << "\n";
    }

    void resource_pool_set_demo() {
        std::cout << "\n===== resource_pool_set basic =====\n";

        jh::resource_pool_set<int> pool;

        auto p1 = pool.acquire(42);
        auto p2 = pool.acquire(42);
        auto p3 = pool.acquire(100);

        std::cout << "*p1=" << *p1 << ", *p3=" << *p3 << "\n";
        std::cout << "p1 == p2 (same key interned once): " << std::boolalpha << (p1 == p2) << "\n";
        std::cout << "pool.size() with live handles: " << pool.size() << "\n";

        p1.reset();
        p2.reset();
        p3.reset();

        std::cout << "pool.size() after reset: " << pool.size() << "\n";
        pool.resize_pool();
        std::cout << "pool.size() after resize_pool: " << pool.size() << "\n";
        std::cout << "pool.capacity() after resize_pool: " << pool.capacity() << "\n";

        const auto check = pool.find(42);
        std::cout << "pool.find(42) == nullptr: " << std::boolalpha << (check == nullptr) << "\n";
    }

    void resource_pool_map_demo() {
        std::cout << "\n===== resource_pool<Key, Value> basic =====\n";

        jh::resource_pool<int, std::string> pool;

        auto e1 = pool.acquire(1, std::forward_as_tuple("first-value"));
        auto e2 = pool.acquire(1, std::tie("ignored-value"));
        auto e3 = pool.acquire(2, std::forward_as_tuple("second-value"));

        std::cout << "e1->first=" << e1->first << ", e1->second=" << e1->second << "\n";
        std::cout << "same key keeps initial value: " << std::boolalpha
                  << (e2->second == "first-value") << "\n";
        std::cout << "e3->first=" << e3->first << ", e3->second=" << e3->second << "\n";

        e1.reset();
        e2.reset();
        e3.reset();

        pool.resize_pool();
        const auto found = pool.find(1);
        std::cout << "pool.find(1) after releasing handles == nullptr: "
                  << std::boolalpha << (found == nullptr) << "\n";
    }

#if !IS_WINDOWS

    void observe_pool_mt_demo() {
        std::cout << "\n===== observe_pool multithread =====\n";

        jh::observe_pool<Symbol> pool;
        constexpr int threads = 8;
        constexpr int objects_per_thread = 200;

        std::vector<std::thread> workers;
        workers.reserve(threads);

        for (int t = 0; t < threads; ++t) {
            workers.emplace_back([&pool] {
                for (int i = 0; i < objects_per_thread; ++i) {
                    pool.acquire("node-" + std::to_string(i), i % 11);
                }
            });
        }

        for (auto &w: workers) {
            w.join();
        }

        std::cout << "pool.size() before cleanup: " << pool.size() << "\n";
        pool.cleanup_shrink();
        std::cout << "pool.size() after cleanup_shrink: " << pool.size() << "\n";
        std::cout << "pool.capacity() after cleanup_shrink: " << pool.capacity() << "\n";
    }

    void resource_pool_set_mt_demo() {
        std::cout << "\n===== resource_pool_set multithread =====\n";

        jh::resource_pool_set<int> pool;
        constexpr int threads = 8;
        constexpr int objects_per_thread = 200;

        std::vector<jh::resource_pool_set<int>::ptr> stored;
        stored.reserve(threads * objects_per_thread);
        std::mutex store_mtx;

        std::vector<std::thread> workers;
        workers.reserve(threads);

        for (int t = 0; t < threads; ++t) {
            workers.emplace_back([&pool, &stored, &store_mtx, t] {
                for (int i = t * objects_per_thread; i < (t + 1) * objects_per_thread; ++i) {
                    auto p = pool.acquire(i);
                    std::lock_guard lk(store_mtx);
                    stored.push_back(p);
                }
            });
        }

        for (auto &w: workers) {
            w.join();
        }

        std::cout << "pool.size() with stored handles: " << pool.size() << "\n";
        std::cout << "pool.capacity() with stored handles: " << pool.capacity() << "\n";

        stored.clear();
        pool.resize_pool();

        std::cout << "pool.size() after clear + resize_pool: " << pool.size() << "\n";
        std::cout << "pool.capacity() after clear + resize_pool: " << pool.capacity() << "\n";
    }

#endif

} // namespace example

int main() {
    example::observe_pool_basic_demo();
    example::pointer_pool_basic_demo();
    example::resource_pool_set_demo();
    example::resource_pool_map_demo();

#if !IS_WINDOWS
    example::observe_pool_mt_demo();
    example::resource_pool_set_mt_demo();
#else
    std::cout << "\n===== multithread demos skipped on Windows =====\n";
    std::cout << "This follows the same policy used in test_pool.cpp.\n";
#endif

    return 0;
}

