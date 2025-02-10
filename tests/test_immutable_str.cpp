#define CATCH_CONFIG_MAIN
#include <random>
#include <thread>
#include <catch2/catch_all.hpp>
#include "jh/immutable_str.h"

namespace test {

    using ImmutablePool = jh::pool<jh::immutable_str>;

    // ✅ Generates a random string with visible characters only
    std::string generate_random_string(const size_t length) {
        static constexpr char charset[] =
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789";

        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<size_t> dist(0, sizeof(charset) - 2);

        std::string str;
        str.reserve(length);
        for (size_t i = 0; i < length; ++i) {
            str += charset[dist(gen)];
        }
        return str;
    }

    // ✅ Adds random leading and trailing whitespace for auto_trim tests
    std::string add_random_whitespace(const std::string& input) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<size_t> space_dist(0, 5);
        const auto before = space_dist(gen);
        auto trail = space_dist(gen);
        if (!before + trail) {
            trail++;
        }
        const std::string prefix(before, ' ');
        const std::string suffix(trail, ' ');

        return prefix + input + suffix;
    }
}

TEST_CASE("Immutable String - Disabled Operations") {
    using jh::immutable_str;

    static_assert(!std::is_copy_constructible_v<immutable_str>, "immutable_str should not be copy-constructible");
    static_assert(!std::is_copy_assignable_v<immutable_str>, "immutable_str should not be copy-assignable");
    static_assert(!std::is_move_constructible_v<immutable_str>, "immutable_str should not be move-constructible");
    static_assert(!std::is_move_assignable_v<immutable_str>, "immutable_str should not be move-assignable");

    immutable_str str("Hello, World!");
}

// ✅ Basic functionality tests for immutable_str
TEST_CASE("Immutable String Functionality") {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> len_dist(5, 20);
    constexpr int total_tests = 1024;

    for (int i = 0; i < total_tests; ++i) {
        SECTION("Immutable String Test " + std::to_string(i + 1)) {
            std::string original = test::generate_random_string(len_dist(gen));

            jh::immutable_str imm_str(original.c_str());
            REQUIRE(imm_str.str() == original);
            REQUIRE(std::string(imm_str.c_str()) == original);
            REQUIRE(imm_str.view() == original);
            REQUIRE(imm_str.size() == original.size());

            REQUIRE(imm_str.hash() == std::hash<std::string>{}(original));
        }
    }
}

// ✅ Auto-trim enabled: Whitespace should be removed automatically
TEST_CASE("Immutable String Auto Trim Enabled") {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> len_dist(5, 20);
    constexpr int total_tests = 1024;

    jh::immutable_str::auto_trim = true;

    for (int i = 0; i < total_tests; ++i) {
        SECTION("Auto Trim Test " + std::to_string(i + 1)) {
            std::string original = test::generate_random_string(len_dist(gen));
            std::string trimmed = test::add_random_whitespace(original);

            jh::immutable_str imm_trimmed(trimmed.c_str());
            jh::immutable_str imm_original(original.c_str());

            REQUIRE(imm_trimmed.view() == original);
            REQUIRE(imm_trimmed.hash() == imm_original.hash());
            REQUIRE(imm_trimmed == imm_original);
        }
    }
}

// ✅ Mutex-protected string tests
TEST_CASE("Immutable String with Mutex-Protected std::string") {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> len_dist(5, 20);
    constexpr int total_tests = 1024;

    for (int i = 0; i < total_tests; ++i) {
        SECTION("Mutex-Protected String Test " + std::to_string(i + 1)) {
            std::string original = test::generate_random_string(len_dist(gen));

            std::mutex str_mutex;
            std::string base_string = original; // Create a base string in scope

            const jh::atomic_str_ptr imm_str = jh::safe_from(base_string, str_mutex);
            // string will be implicitly converted to string_view to create immutable_str

            REQUIRE(imm_str->view() == original);
            REQUIRE(imm_str->hash() == std::hash<std::string>{}(original));
        }
    }
}

// ✅ Mutex-protected string with different inputs
TEST_CASE("Immutable String Mutex-Protected Mismatched") {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> len_dist(5, 20);
    constexpr int total_tests = 1024;

    for (int i = 0; i < total_tests; ++i) {
        SECTION("Mutex-Protected Mismatched Test " + std::to_string(i + 1)) {
            std::string str1 = test::generate_random_string(len_dist(gen));
            std::string str2 = test::generate_random_string(len_dist(gen));

            while (str1 == str2) {
                str2 = test::generate_random_string(len_dist(gen));
            }

            std::mutex str_mutex;
            std::string_view sv1(str1);
            std::string_view sv2(str2);

            jh::atomic_str_ptr imm1 = jh::safe_from(sv1, str_mutex);
            jh::atomic_str_ptr imm2 = jh::safe_from(sv2, str_mutex);

            REQUIRE(imm1->view() != imm2->view());
            REQUIRE(imm1->hash() != imm2->hash());
            REQUIRE_FALSE(*imm1 == *imm2);
        }
    }
}

// ✅ Auto-trim disabled: Whitespace should affect hashing and equality
TEST_CASE("Immutable String Auto Trim Disabled") {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> len_dist(5, 20);
    constexpr int total_tests = 1024;

    jh::immutable_str::auto_trim = false;

    for (int i = 0; i < total_tests; ++i) {
        SECTION("No Trim Test " + std::to_string(i + 1)) {
            std::string original = test::generate_random_string(len_dist(gen));
            std::string padded = test::add_random_whitespace(original);

            jh::immutable_str imm_trimmed(padded.c_str());
            jh::immutable_str imm_original(original.c_str());

            REQUIRE(imm_trimmed.view() != original);
            REQUIRE(imm_trimmed.hash() != imm_original.hash());
            REQUIRE_FALSE(imm_trimmed == imm_original);
        }
    }
    jh::immutable_str::auto_trim = true;
}

// ✅ Different strings should always be considered unequal
TEST_CASE("Different Immutable String Instances") {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> len_dist(5, 20);
    constexpr int total_tests = 1024;

    for (int i = 0; i < total_tests; ++i) {
        SECTION("Different Strings Test " + std::to_string(i + 1)) {
            std::string str1 = test::generate_random_string(len_dist(gen));
            std::string str2 = test::generate_random_string(len_dist(gen));

            while (str1 == str2) {
                str2 = test::generate_random_string(len_dist(gen));
            }

            jh::immutable_str imm1(str1.c_str());
            jh::immutable_str imm2(str2.c_str());

            REQUIRE(imm1.view() != imm2.view());
            REQUIRE(imm1.hash() != imm2.hash());
            REQUIRE_FALSE(imm1 == imm2);
        }
    }
}

// ✅ Custom hash and equality function tests for atomic_str_ptr
TEST_CASE("Atomic String Hashing & Equality") {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> len_dist(5, 20);
    constexpr int total_tests = 1024;

    std::unordered_set<jh::atomic_str_ptr, jh::atomic_str_hash, jh::atomic_str_eq> str_set;

    for (int i = 0; i < total_tests; ++i) {
        SECTION("Atomic String Hash Test " + std::to_string(i + 1)) {
            std::string original = test::generate_random_string(len_dist(gen));
            std::string padded = test::add_random_whitespace(original);

            jh::atomic_str_ptr imm_str1 = jh::make_atomic(original.c_str());
            jh::atomic_str_ptr imm_str2 = jh::make_atomic(padded.c_str());

            str_set.insert(imm_str1);
            REQUIRE(str_set.find(imm_str2) != str_set.end()); // Ensures hash consistency
        }
    }
}

// ✅ Basic functionality tests
TEST_CASE("pool<immutable_str> - Basic Functionality") {
    test::ImmutablePool pool;

    auto str1 = pool.acquire("Hello, World!");
    auto str2 = pool.acquire("Hello, World!");
    auto str3 = pool.acquire("Different String");

    REQUIRE(str1 == str2);  // 🎯 Same string should return the same shared_ptr
    REQUIRE(str1 != str3);  // 🎯 Different strings should return different shared_ptr
    REQUIRE(pool.size() == 2); // 🎯 There should be 2 unique string objects in the pool
}

// ✅ Cleanup behavior tests
TEST_CASE("pool<immutable_str> - Cleanup Behavior") {
    test::ImmutablePool pool;

    auto str1 = pool.acquire("Persistent String");
    REQUIRE(pool.size() == 1);

    str1.reset();  // 🎯 Release shared_ptr
    REQUIRE(pool.size() == 1);  // 🎯 Since it's a weak_ptr, it won't be deleted before cleanup

    pool.cleanup();  // 🎯 Trigger cleanup
    REQUIRE(pool.size() == 0); // 🎯 The pool should be empty
}

// ✅ Hashing and equality tests
TEST_CASE("pool<immutable_str> - Hashing and Equality") {
    test::ImmutablePool pool;

    auto str1 = pool.acquire("Hash Test");
    auto str2 = pool.acquire("Hash Test");

    REQUIRE(str1 == str2);  // 🎯 Ensure hash equality
    REQUIRE(str1->hash() == str2->hash());  // 🎯 Ensure consistent hash
}

// ✅ Multithreading test: Pooling the same string
TEST_CASE("pool<immutable_str> - Multithreading Same String") {
    test::ImmutablePool pool;
    constexpr int THREADS = 4;
    constexpr int OBJECTS_PER_THREAD = 100;

    std::vector<std::shared_ptr<jh::immutable_str>> stored_objects;
    std::mutex stored_mutex;
    std::vector<std::thread> workers;

    workers.reserve(THREADS);
    for (int t = 0; t < THREADS; ++t) {
        workers.emplace_back([&pool, &stored_objects, &stored_mutex, t]() {
            for (int i = t * OBJECTS_PER_THREAD; i < (t + 1) * OBJECTS_PER_THREAD; ++i) { // 🎯 Avoid duplicate values
                auto obj = pool.acquire("Shared String");

                {  // 🎯 Protect `stored_objects` with a lock
                    std::lock_guard lock(stored_mutex);
                    stored_objects.push_back(obj);
                }
            }
        });
    }

    for (auto &w : workers) {
        w.join();
    }

    REQUIRE(pool.size() == 1);  // Same immutable_str should be pooled
}

// ✅ Multithreading test: Correctly storing shared_ptr
TEST_CASE("pool<immutable_str> - Multithreading with Stored Shared_Ptr") {
    test::ImmutablePool pool;
    constexpr int THREADS = 4;
    constexpr int OBJECTS_PER_THREAD = 100;

    std::vector<std::shared_ptr<jh::immutable_str>> stored_objects;
    std::mutex stored_mutex;
    std::vector<std::thread> workers;

    workers.reserve(THREADS);
    for (int t = 0; t < THREADS; ++t) {
        workers.emplace_back([&pool, &stored_objects, &stored_mutex, t]() {
            for (int i = t * OBJECTS_PER_THREAD; i < (t + 1) * OBJECTS_PER_THREAD; ++i) { // 🎯 Avoid duplicate values
                auto obj = pool.acquire(("Thread-" + std::to_string(t) + "-String-" + std::to_string(i)).c_str());

                {  // 🎯 Protect `stored_objects` with a lock
                    std::lock_guard lock(stored_mutex);
                    stored_objects.push_back(obj);
                }
            }
        });
    }

    for (auto &w : workers) {
        w.join();
    }

    REQUIRE(pool.size() == THREADS * OBJECTS_PER_THREAD);  // 🎯 Each thread should store different strings
}

// ✅ Automatic expansion and contraction
TEST_CASE("pool<immutable_str> - Expansion and Contraction") {
    test::ImmutablePool pool(4); // 🎯 Initial size 4

    std::vector<std::shared_ptr<jh::immutable_str>> objects;
    objects.reserve(10);

    for (int i = 0; i < 10; ++i) {
        std::string str = test::generate_random_string(8);
        objects.push_back(pool.acquire(str.c_str()));
    }

    REQUIRE(pool.size() == 10);
    REQUIRE(pool.reserved_size() >= 16);  // 🎯 Expansion

    objects.clear(); // 🎯 Release all objects
    pool.cleanup(); // 🎯 Trigger contraction
    REQUIRE(pool.reserved_size() <= 16);  // 🎯 Contraction
}

// ✅ Clear pool
TEST_CASE("pool<immutable_str> - Clear Pool") {
    test::ImmutablePool pool;

    auto str1 = pool.acquire("To be removed");
    auto str2 = pool.acquire("Also removed");

    REQUIRE(pool.size() == 2);
    pool.clear();
    REQUIRE(pool.size() == 0);
    REQUIRE(pool.reserved_size() == test::ImmutablePool::MIN_RESERVED_SIZE);
}