#define TINY_TEST_MAIN
#include "jh/test/tiny_test/tiny_test.hpp"

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

#include <random>
#include <thread>
#include <shared_mutex>
#include <string_view>

#include "jh/immutable_str"
#include "jh/typed"

namespace test {
    using ImmutablePool = jh::observe_pool<jh::immutable_str>;

    // Generates a random string with visible characters only
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

    // Adds random leading and trailing whitespace for auto_trim tests
    std::string add_random_whitespace(const std::string &input) {
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


namespace test {
void tiny_test_case_1() {
    jh::immutable_str empty(nullptr);
    jh::test::tiny_test::expect(static_cast<bool>((empty.size() == 0)), "empty.size() == 0");
    jh::test::tiny_test::expect(static_cast<bool>((empty.view().empty())), "empty.view().empty()");

}
}
template<>
struct jh::test::tiny_test::test<"Immutable String nullptr construction">
    : jh::test::tiny_test::test_definition<"Immutable String nullptr construction", &::test::tiny_test_case_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_immutable_str case 1">
    : jh::test::tiny_test::session_definition<
          "test module test_immutable_str case 1", jh::test::tiny_test::test<"Immutable String nullptr construction">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_immutable_str case 1"> registration_1{};
}


// Basic functionality tests for immutable_str

namespace test {
void tiny_test_case_2_body(const int selected_section) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> len_dist(5, 20);
    constexpr int total_tests = 128;

    for (int i = 0; i < total_tests; ++i) {
        if (selected_section == 1) {
            std::string original = test::generate_random_string(len_dist(gen));

            jh::immutable_str imm_str(original.c_str());
            jh::test::tiny_test::expect(static_cast<bool>((imm_str.str() == original)), "imm_str.str() == original");
            jh::test::tiny_test::expect(static_cast<bool>((std::string(imm_str.c_str()) == original)), "std::string(imm_str.c_str()) == original");
            jh::test::tiny_test::expect(static_cast<bool>((imm_str.view() == original)), "imm_str.view() == original");
            jh::test::tiny_test::expect(static_cast<bool>((imm_str.size() == original.size())), "imm_str.size() == original.size()");

            jh::test::tiny_test::expect(static_cast<bool>((imm_str.hash() == std::hash<std::string>{}(original))), "imm_str.hash() == std::hash<std::string>{}(original)");
        }
    }

}
void tiny_test_case_2_section_1() { tiny_test_case_2_body(1); }
}
template<>
struct jh::test::tiny_test::test<"Immutable String Functionality / section 1">
    : jh::test::tiny_test::test_definition<"Immutable String Functionality / section 1", &::test::tiny_test_case_2_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_immutable_str case 2">
    : jh::test::tiny_test::session_definition<
          "test module test_immutable_str case 2", jh::test::tiny_test::test<"Immutable String Functionality / section 1">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_immutable_str case 2"> registration_2{};
}


#ifdef JH_IMMUTABLE_STR_AUTO_TRIM
// Auto-trim enabled: Whitespace should be removed automatically

namespace test {
void tiny_test_case_3_body(const int selected_section) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> len_dist(5, 20);
    constexpr int total_tests = 128;

    for (int i = 0; i < total_tests; ++i) {
        if (selected_section == 1) {
            std::string original = test::generate_random_string(len_dist(gen));
            std::string trimmed = test::add_random_whitespace(original);

            jh::immutable_str imm_trimmed(trimmed.c_str());
            jh::immutable_str imm_original(original.c_str());

            jh::test::tiny_test::expect(static_cast<bool>((imm_trimmed.view() == original)), "imm_trimmed.view() == original");
            jh::test::tiny_test::expect(static_cast<bool>((imm_trimmed.hash() == imm_original.hash())), "imm_trimmed.hash() == imm_original.hash()");
            jh::test::tiny_test::expect(static_cast<bool>((imm_trimmed == imm_original)), "imm_trimmed == imm_original");
        }
    }

}
void tiny_test_case_3_section_1() { tiny_test_case_3_body(1); }
}
template<>
struct jh::test::tiny_test::test<"Immutable String Auto Trim Enabled / section 1">
    : jh::test::tiny_test::test_definition<"Immutable String Auto Trim Enabled / section 1", &::test::tiny_test_case_3_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_immutable_str case 3">
    : jh::test::tiny_test::session_definition<
          "test module test_immutable_str case 3", jh::test::tiny_test::test<"Immutable String Auto Trim Enabled / section 1">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_immutable_str case 3"> registration_3{};
}


#else
// Auto-trim disabled: Whitespace should affect hashing and equality

namespace test {
void tiny_test_case_4_body(const int selected_section) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> len_dist(5, 20);
    constexpr int total_tests = 1024;

    for (int i = 0; i < total_tests; ++i) {
        if (selected_section == 1) {
            std::string original = test::generate_random_string(len_dist(gen));
            std::string padded = test::add_random_whitespace(original);

            jh::immutable_str imm_trimmed(padded.c_str());
            jh::immutable_str imm_original(original.c_str());

            jh::test::tiny_test::expect(static_cast<bool>((imm_trimmed.view() != original)), "imm_trimmed.view() != original");
            jh::test::tiny_test::expect(static_cast<bool>((imm_trimmed.hash() != imm_original.hash())), "imm_trimmed.hash() != imm_original.hash()");
            jh::test::tiny_test::expect_not(static_cast<bool>((imm_trimmed == imm_original)), "imm_trimmed == imm_original");
        }
    }

}
void tiny_test_case_4_section_1() { tiny_test_case_4_body(1); }
}
template<>
struct jh::test::tiny_test::test<"Immutable String Auto Trim Disabled / section 1">
    : jh::test::tiny_test::test_definition<"Immutable String Auto Trim Disabled / section 1", &::test::tiny_test_case_4_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_immutable_str case 4">
    : jh::test::tiny_test::session_definition<
          "test module test_immutable_str case 4", jh::test::tiny_test::test<"Immutable String Auto Trim Disabled / section 1">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_immutable_str case 4"> registration_4{};
}

#endif

// Mutex-protected string tests

namespace test {
void tiny_test_case_5_body(const int selected_section) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> len_dist(5, 20);
    constexpr int total_tests = 128;

    for (int i = 0; i < total_tests; ++i) {
        if (selected_section == 1) {
            std::string original = test::generate_random_string(len_dist(gen));

            std::mutex str_mutex;
            std::string base_string = original; // Create a base string in scope

            const jh::atomic_str_ptr imm_str = jh::safe_from(base_string, str_mutex);
            // string will be implicitly converted to string_view to create immutable_str

            jh::test::tiny_test::expect(static_cast<bool>((imm_str->view() == original)), "imm_str->view() == original");
            jh::test::tiny_test::expect(static_cast<bool>((imm_str->hash() == std::hash<std::string>{}(original))), "imm_str->hash() == std::hash<std::string>{}(original)");
        }
    }

}
void tiny_test_case_5_section_1() { tiny_test_case_5_body(1); }
}
template<>
struct jh::test::tiny_test::test<"Immutable String with Mutex-Protected std::string / section 1">
    : jh::test::tiny_test::test_definition<"Immutable String with Mutex-Protected std::string / section 1", &::test::tiny_test_case_5_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_immutable_str case 5">
    : jh::test::tiny_test::session_definition<
          "test module test_immutable_str case 5", jh::test::tiny_test::test<"Immutable String with Mutex-Protected std::string / section 1">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_immutable_str case 5"> registration_5{};
}


// No-op mutex string tests

namespace test {
void tiny_test_case_6_body(const int selected_section) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> len_dist(5, 20);
    constexpr int total_tests = 128;

    for (int i = 0; i < total_tests; ++i) {
        if (selected_section == 1) {
            std::string original = test::generate_random_string(len_dist(gen));

            std::string base_string = original; // Create a base string in scope

            const jh::atomic_str_ptr imm_str = jh::safe_from(base_string, jh::typed::null_mutex);
            // string will be implicitly converted to string_view to create immutable_str

            jh::test::tiny_test::expect(static_cast<bool>((imm_str->view() == original)), "imm_str->view() == original");
            jh::test::tiny_test::expect(static_cast<bool>((imm_str->hash() == std::hash<std::string>{}(original))), "imm_str->hash() == std::hash<std::string>{}(original)");
        }
    }

}
void tiny_test_case_6_section_1() { tiny_test_case_6_body(1); }
}
template<>
struct jh::test::tiny_test::test<"Immutable String with No-op Mutex std::string / section 1">
    : jh::test::tiny_test::test_definition<"Immutable String with No-op Mutex std::string / section 1", &::test::tiny_test_case_6_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_immutable_str case 6">
    : jh::test::tiny_test::session_definition<
          "test module test_immutable_str case 6", jh::test::tiny_test::test<"Immutable String with No-op Mutex std::string / section 1">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_immutable_str case 6"> registration_6{};
}


// Mutex-protected string with different inputs

namespace test {
void tiny_test_case_7_body(const int selected_section) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> len_dist(5, 20);
    constexpr int total_tests = 128;

    for (int i = 0; i < total_tests; ++i) {
        if (selected_section == 1) {
            std::string str1 = test::generate_random_string(len_dist(gen));
            std::string str2 = test::generate_random_string(len_dist(gen));

            while (str1 == str2) {
                str2 = test::generate_random_string(len_dist(gen));
            }

            std::shared_mutex str_mutex;
            std::string_view sv1(str1);
            std::string_view sv2(str2);

            jh::atomic_str_ptr imm1 = jh::safe_from(sv1, str_mutex);
            jh::atomic_str_ptr imm2 = jh::safe_from(sv2, str_mutex);

            jh::test::tiny_test::expect(static_cast<bool>((imm1->view() != imm2->view())), "imm1->view() != imm2->view()");
            jh::test::tiny_test::expect(static_cast<bool>((imm1->hash() != imm2->hash())), "imm1->hash() != imm2->hash()");
            jh::test::tiny_test::expect_not(static_cast<bool>((*imm1 == *imm2)), "*imm1 == *imm2");
        }
    }

}
void tiny_test_case_7_section_1() { tiny_test_case_7_body(1); }
}
template<>
struct jh::test::tiny_test::test<"Immutable String Mutex-Protected Mismatched / section 1">
    : jh::test::tiny_test::test_definition<"Immutable String Mutex-Protected Mismatched / section 1", &::test::tiny_test_case_7_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_immutable_str case 7">
    : jh::test::tiny_test::session_definition<
          "test module test_immutable_str case 7", jh::test::tiny_test::test<"Immutable String Mutex-Protected Mismatched / section 1">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_immutable_str case 7"> registration_7{};
}


// Different strings should always be considered unequal

namespace test {
void tiny_test_case_8_body(const int selected_section) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> len_dist(5, 20);
    constexpr int total_tests = 128;

    for (int i = 0; i < total_tests; ++i) {
        if (selected_section == 1) {
            std::string str1 = test::generate_random_string(len_dist(gen));
            std::string str2 = test::generate_random_string(len_dist(gen));

            while (str1 == str2) {
                str2 = test::generate_random_string(len_dist(gen));
            }

            jh::immutable_str imm1(str1.c_str());
            jh::immutable_str imm2(str2.c_str());

            jh::test::tiny_test::expect(static_cast<bool>((imm1.view() != imm2.view())), "imm1.view() != imm2.view()");
            jh::test::tiny_test::expect(static_cast<bool>((imm1.hash() != imm2.hash())), "imm1.hash() != imm2.hash()");
            jh::test::tiny_test::expect_not(static_cast<bool>((imm1 == imm2)), "imm1 == imm2");
        }
    }

}
void tiny_test_case_8_section_1() { tiny_test_case_8_body(1); }
}
template<>
struct jh::test::tiny_test::test<"Different Immutable String Instances / section 1">
    : jh::test::tiny_test::test_definition<"Different Immutable String Instances / section 1", &::test::tiny_test_case_8_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_immutable_str case 8">
    : jh::test::tiny_test::session_definition<
          "test module test_immutable_str case 8", jh::test::tiny_test::test<"Different Immutable String Instances / section 1">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_immutable_str case 8"> registration_8{};
}


// Custom hash and equality function tests for atomic_str_ptr

namespace test {
void tiny_test_case_9_body(const int selected_section) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> len_dist(5, 20);
    constexpr int total_tests = 128;

    std::unordered_set<jh::atomic_str_ptr, jh::atomic_str_hash, jh::atomic_str_eq> str_set;

    for (int i = 0; i < total_tests; ++i) {
        if (selected_section == 1) {
            std::string original = test::generate_random_string(len_dist(gen));
            std::string padded = test::add_random_whitespace(original);

            jh::atomic_str_ptr imm_str1 = jh::make_atomic(original.c_str());
            jh::atomic_str_ptr imm_str2 = jh::make_atomic(padded.c_str());

            str_set.insert(imm_str1);
            jh::test::tiny_test::expect(static_cast<bool>((str_set.find(imm_str2) != str_set.end())), "str_set.find(imm_str2) != str_set.end()"); // Ensures hash consistency
        }
    }

}
void tiny_test_case_9_section_1() { tiny_test_case_9_body(1); }
}
template<>
struct jh::test::tiny_test::test<"Atomic String Hashing & Equality / section 1">
    : jh::test::tiny_test::test_definition<"Atomic String Hashing & Equality / section 1", &::test::tiny_test_case_9_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_immutable_str case 9">
    : jh::test::tiny_test::session_definition<
          "test module test_immutable_str case 9", jh::test::tiny_test::test<"Atomic String Hashing & Equality / section 1">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_immutable_str case 9"> registration_9{};
}


// Basic functionality tests

namespace test {
void tiny_test_case_10() {
    test::ImmutablePool pool;

    auto str1 = pool.acquire("Hello, World!");
    auto str2 = pool.acquire("Hello, World!");
    auto str3 = pool.acquire("Different String");

    jh::test::tiny_test::expect(static_cast<bool>((str1 == str2)), "str1 == str2"); // Same string should return the same shared_ptr
    jh::test::tiny_test::expect(static_cast<bool>((str1 != str3)), "str1 != str3"); // Different strings should return different shared_ptr
    jh::test::tiny_test::expect(static_cast<bool>((pool.size() == 2)), "pool.size() == 2"); // There should be 2 unique string objects in the pool

}
}
template<>
struct jh::test::tiny_test::test<"observe_pool<immutable_str> - Basic Functionality">
    : jh::test::tiny_test::test_definition<"observe_pool<immutable_str> - Basic Functionality", &::test::tiny_test_case_10> {};
template<>
struct jh::test::tiny_test::session<"test module test_immutable_str case 10">
    : jh::test::tiny_test::session_definition<
          "test module test_immutable_str case 10", jh::test::tiny_test::test<"observe_pool<immutable_str> - Basic Functionality">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_immutable_str case 10"> registration_10{};
}


// Cleanup behavior tests

namespace test {
void tiny_test_case_11() {
    test::ImmutablePool pool;

    auto str1 = pool.acquire("Persistent String");
    jh::test::tiny_test::expect(static_cast<bool>((pool.size() == 1)), "pool.size() == 1");

    str1.reset(); // Release shared_ptr
    jh::test::tiny_test::expect(static_cast<bool>((pool.size() == 1)), "pool.size() == 1"); // Since it's a weak_ptr, it won't be deleted before cleanup

    pool.cleanup(); // Trigger cleanup
    jh::test::tiny_test::expect(static_cast<bool>((pool.size() == 0)), "pool.size() == 0"); // The pool should be empty

}
}
template<>
struct jh::test::tiny_test::test<"observe_pool<immutable_str> - Cleanup Behavior">
    : jh::test::tiny_test::test_definition<"observe_pool<immutable_str> - Cleanup Behavior", &::test::tiny_test_case_11> {};
template<>
struct jh::test::tiny_test::session<"test module test_immutable_str case 11">
    : jh::test::tiny_test::session_definition<
          "test module test_immutable_str case 11", jh::test::tiny_test::test<"observe_pool<immutable_str> - Cleanup Behavior">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_immutable_str case 11"> registration_11{};
}


// Hashing and equality tests

namespace test {
void tiny_test_case_12() {
    test::ImmutablePool pool;

    auto str1 = pool.acquire("Hash Test");
    auto str2 = pool.acquire("Hash Test");

    jh::test::tiny_test::expect(static_cast<bool>((str1 == str2)), "str1 == str2"); // Ensure hash equality
    jh::test::tiny_test::expect(static_cast<bool>((str1->hash() == str2->hash())), "str1->hash() == str2->hash()"); // Ensure consistent hash

}
}
template<>
struct jh::test::tiny_test::test<"observe_pool<immutable_str> - Hashing and Equality">
    : jh::test::tiny_test::test_definition<"observe_pool<immutable_str> - Hashing and Equality", &::test::tiny_test_case_12> {};
template<>
struct jh::test::tiny_test::session<"test module test_immutable_str case 12">
    : jh::test::tiny_test::session_definition<
          "test module test_immutable_str case 12", jh::test::tiny_test::test<"observe_pool<immutable_str> - Hashing and Equality">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_immutable_str case 12"> registration_12{};
}


// Multithreading test: Pooling the same string

namespace test {
void tiny_test_case_13() {
    test::ImmutablePool pool;
    constexpr int THREADS = 4;
    constexpr int OBJECTS_PER_THREAD = 100;

    std::vector<std::shared_ptr<jh::immutable_str> > stored_objects;
    std::mutex stored_mutex;
    std::vector<std::thread> workers;

    workers.reserve(THREADS);
    for (int t = 0; t < THREADS; ++t) {
        workers.emplace_back([&pool, &stored_objects, &stored_mutex, t]() {
            for (int i = t * OBJECTS_PER_THREAD; i < (t + 1) * OBJECTS_PER_THREAD; ++i) {
                // Avoid duplicate values
                {
                    auto obj = pool.acquire("Shared String");
                    // Protect `stored_objects` with a lock
                    std::lock_guard lock(stored_mutex);
                    stored_objects.push_back(obj);
                }
            }
        });
    }

    for (auto &w: workers) {
        w.join();
    }
    pool.cleanup();

    jh::test::tiny_test::expect(static_cast<bool>((pool.size() == 1)), "pool.size() == 1"); // Same immutable_str should be pooled

}
}
template<>
struct jh::test::tiny_test::test<"observe_pool<immutable_str> - Multithreading Same String">
    : jh::test::tiny_test::test_definition<"observe_pool<immutable_str> - Multithreading Same String", &::test::tiny_test_case_13> {};
template<>
struct jh::test::tiny_test::session<"test module test_immutable_str case 13">
    : jh::test::tiny_test::session_definition<
          "test module test_immutable_str case 13", jh::test::tiny_test::test<"observe_pool<immutable_str> - Multithreading Same String">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_immutable_str case 13"> registration_13{};
}


// Multithreading test: Correctly storing shared_ptr

namespace test {
void tiny_test_case_14() {
    test::ImmutablePool pool;
    constexpr int THREADS = 4;
    constexpr int OBJECTS_PER_THREAD = 100;

    std::vector<std::shared_ptr<jh::immutable_str> > stored_objects;
    std::mutex stored_mutex;
    std::vector<std::thread> workers;

    workers.reserve(THREADS);
    for (int t = 0; t < THREADS; ++t) {
        workers.emplace_back([&pool, &stored_objects, &stored_mutex, t]() {
            for (int i = t * OBJECTS_PER_THREAD; i < (t + 1) * OBJECTS_PER_THREAD; ++i) {
                // Avoid duplicate values
                {
                    auto obj = pool.acquire(("Thread-" + std::to_string(t) + "-String-" + std::to_string(i)).c_str());
                    // Protect `stored_objects` with a lock
                    std::lock_guard lock(stored_mutex);
                    stored_objects.push_back(obj);
                }
            }
        });
    }

    for (auto &w: workers) {
        w.join();
    }

    jh::test::tiny_test::expect(static_cast<bool>((pool.size() == THREADS * OBJECTS_PER_THREAD)), "pool.size() == THREADS * OBJECTS_PER_THREAD"); // Each thread should store different strings

}
}
template<>
struct jh::test::tiny_test::test<"observe_pool<immutable_str> - Multithreading with Stored Shared_Ptr">
    : jh::test::tiny_test::test_definition<"observe_pool<immutable_str> - Multithreading with Stored Shared_Ptr", &::test::tiny_test_case_14> {};
template<>
struct jh::test::tiny_test::session<"test module test_immutable_str case 14">
    : jh::test::tiny_test::session_definition<
          "test module test_immutable_str case 14", jh::test::tiny_test::test<"observe_pool<immutable_str> - Multithreading with Stored Shared_Ptr">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_immutable_str case 14"> registration_14{};
}


// Automatic expansion and contraction

namespace test {
void tiny_test_case_15() {
    test::ImmutablePool pool(4); // Initial size 4

    std::vector<std::shared_ptr<jh::immutable_str> > objects;
    objects.reserve(10);

    for (int i = 0; i < 10; ++i) {
        std::string str = test::generate_random_string(8);
        objects.push_back(pool.acquire(str.c_str()));
    }

    jh::test::tiny_test::expect(static_cast<bool>((pool.size() == 10)), "pool.size() == 10");
    jh::test::tiny_test::expect(static_cast<bool>((pool.capacity() >= 16)), "pool.capacity() >= 16"); // Expansion

    objects.clear(); // Release all objects
    pool.cleanup(); // Trigger contraction
    jh::test::tiny_test::expect(static_cast<bool>((pool.capacity() <= 16)), "pool.capacity() <= 16"); // Contraction

}
}
template<>
struct jh::test::tiny_test::test<"observe_pool<immutable_str> - Expansion and Contraction">
    : jh::test::tiny_test::test_definition<"observe_pool<immutable_str> - Expansion and Contraction", &::test::tiny_test_case_15> {};
template<>
struct jh::test::tiny_test::session<"test module test_immutable_str case 15">
    : jh::test::tiny_test::session_definition<
          "test module test_immutable_str case 15", jh::test::tiny_test::test<"observe_pool<immutable_str> - Expansion and Contraction">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_immutable_str case 15"> registration_15{};
}


// Clear pool

namespace test {
void tiny_test_case_16() {
    test::ImmutablePool pool;

    auto str1 = pool.acquire("To be removed");
    auto str2 = pool.acquire("Also removed");

    jh::test::tiny_test::expect(static_cast<bool>((pool.size() == 2)), "pool.size() == 2");
    pool.clear();
    jh::test::tiny_test::expect(static_cast<bool>((pool.size() == 0)), "pool.size() == 0");
    jh::test::tiny_test::expect(static_cast<bool>((pool.capacity() == test::ImmutablePool::MIN_RESERVED_SIZE)), "pool.capacity() == test::ImmutablePool::MIN_RESERVED_SIZE");

}
}
template<>
struct jh::test::tiny_test::test<"observe_pool<immutable_str> - Clear Pool">
    : jh::test::tiny_test::test_definition<"observe_pool<immutable_str> - Clear Pool", &::test::tiny_test_case_16> {};
template<>
struct jh::test::tiny_test::session<"test module test_immutable_str case 16">
    : jh::test::tiny_test::session_definition<
          "test module test_immutable_str case 16", jh::test::tiny_test::test<"observe_pool<immutable_str> - Clear Pool">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_immutable_str case 16"> registration_16{};
}



namespace test {
void tiny_test_case_17_body(const int selected_section) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> rand_dist(0, 5);
    constexpr int total_tests = 128;

    jh::observe_pool<jh::immutable_str> pool;
    // Recommended method: Directly map atomic immutable strings to unique identifiers
    static const std::unordered_map<jh::atomic_str_ptr, size_t, jh::atomic_str_hash, jh::atomic_str_eq> immutable_map =
    {
        {jh::make_atomic("example 0"), 0},
        {jh::make_atomic("example 1"), 1},
        {jh::make_atomic("example 2"), 2},
        {jh::make_atomic("example 3"), 3},
        {jh::make_atomic("example 4"), 4}
    };

    for (int i = 0; i < total_tests; ++i) {
        if (selected_section == 1) {
            int rand_num = rand_dist(gen);
            std::string str_value;

            if (rand_num < 5) {
                str_value = "example " + std::to_string(rand_num);
            } else {
                str_value = test::generate_random_string(10); // Generate a random string
            }

            const auto str = pool.acquire(str_value.c_str());
            const auto it = immutable_map.find(str);

            std::string check_str;

            switch (auto num = it == immutable_map.end() ? 5 : it->second) {
                case 0:
                case 1:
                case 2:
                case 3:
                case 4:
                check_str = "example " + std::to_string(num);
                    jh::test::tiny_test::expect(static_cast<bool>((str->view() == check_str)), "str->view() == check_str");
                    jh::test::tiny_test::expect(static_cast<bool>((immutable_map.find(check_str.c_str()) != immutable_map.end())), "immutable_map.find(check_str.c_str()) != immutable_map.end()");
                    break;
                default:
                    jh::test::tiny_test::expect(static_cast<bool>((str->view() != "example 0")), "str->view() != \"example 0\"");
                    jh::test::tiny_test::expect(static_cast<bool>((str->view() != "example 1")), "str->view() != \"example 1\"");
                    jh::test::tiny_test::expect(static_cast<bool>((str->view() != "example 2")), "str->view() != \"example 2\"");
                    jh::test::tiny_test::expect(static_cast<bool>((str->view() != "example 3")), "str->view() != \"example 3\"");
                    jh::test::tiny_test::expect(static_cast<bool>((str->view() != "example 4")), "str->view() != \"example 4\"");
                    break;
            }
        }
    }

}
void tiny_test_case_17_section_1() { tiny_test_case_17_body(1); }
}
template<>
struct jh::test::tiny_test::test<"Switch with HashMap Example Test / section 1">
    : jh::test::tiny_test::test_definition<"Switch with HashMap Example Test / section 1", &::test::tiny_test_case_17_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_immutable_str case 17">
    : jh::test::tiny_test::session_definition<
          "test module test_immutable_str case 17", jh::test::tiny_test::test<"Switch with HashMap Example Test / section 1">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_immutable_str case 17"> registration_17{};
}

