#define CATCH_CONFIG_MAIN
#include <random>
#include <catch2/catch_all.hpp>
#include "jh/immutable_str.h"

namespace test {
    // ✅ Generates a random string with visible characters only
    std::string generate_random_string(size_t length) {
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
