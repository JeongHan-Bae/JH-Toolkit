#define TINY_TEST_MAIN
#include "jh/test/tiny_test/tiny_test.hpp"

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

#include <cmath>
#include <memory_resource>
#include <ranges>
#include "jh/runtime_arr"
#include "jh/macros/platform.h"
#include <tuple>
#include <memory>
#include <vector>
#include <random>
#include "jh/pod"

using namespace jh;

// Dummy allocator for testing
template<typename T>
struct test_allocator {
    static T *allocate(std::size_t n) {
        return static_cast<T *>(operator new[](n * sizeof(T)));
    }

    static void deallocate(T *p, std::size_t) {
        ::operator delete[](p);
    }
};


namespace test {
void tiny_test_case_1_body(const int selected_section) {
    constexpr int N = 32;
    runtime_arr<int> arr(N);

    if (selected_section == 1) {
        for (int i = 0; i < N; ++i)
            arr[i] = i * i;

        for (int i = 0; i < N; ++i)
            jh::test::tiny_test::expect(static_cast<bool>((arr[i] == i * i)), "arr[i] == i * i");
    }

    if (selected_section == 2) {
        arr.reset_all();
        for (int i = 0; i < N; ++i)
            jh::test::tiny_test::expect(static_cast<bool>((arr[i] == 0)), "arr[i] == 0");
    }

    if (selected_section == 3) {
        for (int i = 0; i < N; ++i)
            arr[i] = i;

        runtime_arr<int> moved = std::move(arr);
        for (int i = 0; i < N; ++i)
            jh::test::tiny_test::expect(static_cast<bool>((moved[i] == i)), "moved[i] == i");
        jh::test::tiny_test::expect(static_cast<bool>((arr.data() == nullptr)), "arr.data() == nullptr");
    }

    if (selected_section == 4) {
        for (int i = 0; i < N; ++i)
            arr[i] = N - i;

        std::vector<int> vec = static_cast<std::vector<int>>(std::move(arr));
        for (int i = 0; i < N; ++i)
            jh::test::tiny_test::expect(static_cast<bool>((vec[i] == N - i)), "vec[i] == N - i");
    }

}
void tiny_test_case_1_section_1() { tiny_test_case_1_body(1); }
void tiny_test_case_1_section_2() { tiny_test_case_1_body(2); }
void tiny_test_case_1_section_3() { tiny_test_case_1_body(3); }
void tiny_test_case_1_section_4() { tiny_test_case_1_body(4); }
}
template<>
struct jh::test::tiny_test::test<"runtime_arr<int> full test / set and verify values">
    : jh::test::tiny_test::test_definition<"runtime_arr<int> full test / set and verify values", &::test::tiny_test_case_1_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_runtime_arr case 1">
    : jh::test::tiny_test::session_definition<
          "test module test_runtime_arr case 1", jh::test::tiny_test::test<"runtime_arr<int> full test / set and verify values">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_runtime_arr case 1"> registration_1{};
}
template<>
struct jh::test::tiny_test::test<"runtime_arr<int> full test / reset_all clears to zero">
    : jh::test::tiny_test::test_definition<"runtime_arr<int> full test / reset_all clears to zero", &::test::tiny_test_case_1_section_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_runtime_arr case 2">
    : jh::test::tiny_test::session_definition<
          "test module test_runtime_arr case 2", jh::test::tiny_test::test<"runtime_arr<int> full test / reset_all clears to zero">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_runtime_arr case 2"> registration_2{};
}
template<>
struct jh::test::tiny_test::test<"runtime_arr<int> full test / move constructor">
    : jh::test::tiny_test::test_definition<"runtime_arr<int> full test / move constructor", &::test::tiny_test_case_1_section_3> {};
template<>
struct jh::test::tiny_test::session<"test module test_runtime_arr case 3">
    : jh::test::tiny_test::session_definition<
          "test module test_runtime_arr case 3", jh::test::tiny_test::test<"runtime_arr<int> full test / move constructor">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_runtime_arr case 3"> registration_3{};
}
template<>
struct jh::test::tiny_test::test<"runtime_arr<int> full test / conversion to vector">
    : jh::test::tiny_test::test_definition<"runtime_arr<int> full test / conversion to vector", &::test::tiny_test_case_1_section_4> {};
template<>
struct jh::test::tiny_test::session<"test module test_runtime_arr case 4">
    : jh::test::tiny_test::session_definition<
          "test module test_runtime_arr case 4", jh::test::tiny_test::test<"runtime_arr<int> full test / conversion to vector">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_runtime_arr case 4"> registration_4{};
}



JH_POD_STRUCT(MyPod,
              int id;
                      float score;
);


namespace test {
void tiny_test_case_2_body(const int selected_section) {
    constexpr int N = 64;
    runtime_arr<MyPod> arr(N);

    if (selected_section == 1) {
        for (int i = 0; i < N; ++i) {
            arr.set(i, i, static_cast<float>(i) * 0.5f);
        }

        for (int i = 0; i < N; ++i) {
            jh::test::tiny_test::expect(static_cast<bool>((arr[i].id == i)), "arr[i].id == i");
            jh::test::tiny_test::expect(static_cast<bool>((std::abs(arr[i].score - i * 0.5f) < 1e-5f)), "std::abs(arr[i].score - i * 0.5f) < 1e-5f");
        }
    }

    if (selected_section == 2) {
        arr.reset_all();
        for (int i = 0; i < N; ++i) {
            jh::test::tiny_test::expect(static_cast<bool>((arr[i].id == 0)), "arr[i].id == 0");
            jh::test::tiny_test::expect(static_cast<bool>((arr[i].score == 0.0f)), "arr[i].score == 0.0f");
        }
    }

    if (selected_section == 3) {
        for (int i = 0; i < N; ++i)
            arr.set(i, i, static_cast<float>(i) + 0.1f);

        std::vector<MyPod> vec = static_cast<std::vector<MyPod>>(std::move(arr));
        jh::test::tiny_test::expect(static_cast<bool>((vec.size() == N)), "vec.size() == N");
        jh::test::tiny_test::expect(static_cast<bool>((vec[5].id == 5)), "vec[5].id == 5");
        jh::test::tiny_test::expect(static_cast<bool>((std::abs(vec[5].score - 5.1f) < 1e-5f)), "std::abs(vec[5].score - 5.1f) < 1e-5f");
    }

    if (selected_section == 4) {
        for (int i = 0; i < N; ++i)
            arr.set(i, 100 + i, 2.0f * static_cast<float>(i));

        auto moved = std::move(arr);
        for (int i = 0; i < N; ++i) {
            jh::test::tiny_test::expect(static_cast<bool>((moved[i].id == 100 + i)), "moved[i].id == 100 + i");
            jh::test::tiny_test::expect(static_cast<bool>((std::abs(moved[i].score - 2.0f * i) < 1e-5f)), "std::abs(moved[i].score - 2.0f * i) < 1e-5f");
        }
    }

}
void tiny_test_case_2_section_1() { tiny_test_case_2_body(1); }
void tiny_test_case_2_section_2() { tiny_test_case_2_body(2); }
void tiny_test_case_2_section_3() { tiny_test_case_2_body(3); }
void tiny_test_case_2_section_4() { tiny_test_case_2_body(4); }
}
template<>
struct jh::test::tiny_test::test<"runtime_arr<MyPod> full test / initialize values">
    : jh::test::tiny_test::test_definition<"runtime_arr<MyPod> full test / initialize values", &::test::tiny_test_case_2_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_runtime_arr case 5">
    : jh::test::tiny_test::session_definition<
          "test module test_runtime_arr case 5", jh::test::tiny_test::test<"runtime_arr<MyPod> full test / initialize values">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_runtime_arr case 5"> registration_5{};
}
template<>
struct jh::test::tiny_test::test<"runtime_arr<MyPod> full test / reset_all to zero">
    : jh::test::tiny_test::test_definition<"runtime_arr<MyPod> full test / reset_all to zero", &::test::tiny_test_case_2_section_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_runtime_arr case 6">
    : jh::test::tiny_test::session_definition<
          "test module test_runtime_arr case 6", jh::test::tiny_test::test<"runtime_arr<MyPod> full test / reset_all to zero">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_runtime_arr case 6"> registration_6{};
}
template<>
struct jh::test::tiny_test::test<"runtime_arr<MyPod> full test / move to vector<MyPod>">
    : jh::test::tiny_test::test_definition<"runtime_arr<MyPod> full test / move to vector<MyPod>", &::test::tiny_test_case_2_section_3> {};
template<>
struct jh::test::tiny_test::session<"test module test_runtime_arr case 7">
    : jh::test::tiny_test::session_definition<
          "test module test_runtime_arr case 7", jh::test::tiny_test::test<"runtime_arr<MyPod> full test / move to vector<MyPod>">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_runtime_arr case 7"> registration_7{};
}
template<>
struct jh::test::tiny_test::test<"runtime_arr<MyPod> full test / move construction keeps values">
    : jh::test::tiny_test::test_definition<"runtime_arr<MyPod> full test / move construction keeps values", &::test::tiny_test_case_2_section_4> {};
template<>
struct jh::test::tiny_test::session<"test module test_runtime_arr case 8">
    : jh::test::tiny_test::session_definition<
          "test module test_runtime_arr case 8", jh::test::tiny_test::test<"runtime_arr<MyPod> full test / move construction keeps values">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_runtime_arr case 8"> registration_8{};
}




namespace test {
void tiny_test_case_3_body(const int selected_section) {
    using T = runtime_arr<int, test_allocator<int> >;
    T arr(5, test_allocator<int>{});

    if (selected_section == 1) {
        for (int i = 0; i < 5; ++i)
            arr.set(i, i + 100);
        jh::test::tiny_test::expect(static_cast<bool>((arr[2] == 102)), "arr[2] == 102");
    }

    if (selected_section == 2) {
        arr.reset_all();
        for (int i = 0; i < 5; ++i)
            jh::test::tiny_test::expect(static_cast<bool>((arr[i] == 0)), "arr[i] == 0");
    }

}
void tiny_test_case_3_section_1() { tiny_test_case_3_body(1); }
void tiny_test_case_3_section_2() { tiny_test_case_3_body(2); }
}
template<>
struct jh::test::tiny_test::test<"runtime_arr<int, test_allocator> behavior / set and get">
    : jh::test::tiny_test::test_definition<"runtime_arr<int, test_allocator> behavior / set and get", &::test::tiny_test_case_3_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_runtime_arr case 9">
    : jh::test::tiny_test::session_definition<
          "test module test_runtime_arr case 9", jh::test::tiny_test::test<"runtime_arr<int, test_allocator> behavior / set and get">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_runtime_arr case 9"> registration_9{};
}
template<>
struct jh::test::tiny_test::test<"runtime_arr<int, test_allocator> behavior / reset_all and verify">
    : jh::test::tiny_test::test_definition<"runtime_arr<int, test_allocator> behavior / reset_all and verify", &::test::tiny_test_case_3_section_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_runtime_arr case 10">
    : jh::test::tiny_test::session_definition<
          "test module test_runtime_arr case 10", jh::test::tiny_test::test<"runtime_arr<int, test_allocator> behavior / reset_all and verify">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_runtime_arr case 10"> registration_10{};
}




namespace test {
void tiny_test_case_4_body(const int selected_section) {
    using tup = std::tuple<int, int>;

    runtime_arr<tup> arr(3);

    if (selected_section == 1) {
        arr.set(0, 10, 20);
        arr.set(1, 30, 40);
        jh::test::tiny_test::expect(static_cast<bool>((std::get<0>(arr[1]) == 30)), "std::get<0>(arr[1]) == 30");
        jh::test::tiny_test::expect(static_cast<bool>((std::get<1>(arr[1]) == 40)), "std::get<1>(arr[1]) == 40");
    }

    if (selected_section == 2) {
        arr.set(0, 1, 2);
        arr.set(1, 3, 4);
        arr.set(2, 5, 6);
        std::vector<tup> vec = static_cast<std::vector<tup>>(std::move(arr));
        jh::test::tiny_test::expect(static_cast<bool>((vec[2] == std::make_tuple(5, 6))), "vec[2] == std::make_tuple(5, 6)");
    }

}
void tiny_test_case_4_section_1() { tiny_test_case_4_body(1); }
void tiny_test_case_4_section_2() { tiny_test_case_4_body(2); }
}
template<>
struct jh::test::tiny_test::test<"runtime_arr<tuple> structured ops / set and access via get">
    : jh::test::tiny_test::test_definition<"runtime_arr<tuple> structured ops / set and access via get", &::test::tiny_test_case_4_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_runtime_arr case 11">
    : jh::test::tiny_test::session_definition<
          "test module test_runtime_arr case 11", jh::test::tiny_test::test<"runtime_arr<tuple> structured ops / set and access via get">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_runtime_arr case 11"> registration_11{};
}
template<>
struct jh::test::tiny_test::test<"runtime_arr<tuple> structured ops / move to vector">
    : jh::test::tiny_test::test_definition<"runtime_arr<tuple> structured ops / move to vector", &::test::tiny_test_case_4_section_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_runtime_arr case 12">
    : jh::test::tiny_test::session_definition<
          "test module test_runtime_arr case 12", jh::test::tiny_test::test<"runtime_arr<tuple> structured ops / move to vector">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_runtime_arr case 12"> registration_12{};
}



namespace test {
void tiny_test_case_5_body(const int selected_section) {
    constexpr std::size_t N = 8;
    jh::runtime_arr<int> arr(N);

    for (std::size_t i = 0; i < N; ++i)
        arr[i] = static_cast<int>(i * 2);

    if (selected_section == 1) {
        auto s = arr.as_span();
        jh::test::tiny_test::expect(static_cast<bool>((s.size() == N)), "s.size() == N");
        for (std::size_t i = 0; i < N; ++i)
            jh::test::tiny_test::expect(static_cast<bool>((s[i] == static_cast<int>(i * 2))), "s[i] == static_cast<int>(i * 2)");

        s[3] = 999;
        jh::test::tiny_test::expect(static_cast<bool>((arr[3] == 999)), "arr[3] == 999");
    }

    if (selected_section == 2) {
        for (std::size_t i = 0; i < N; ++i)
            arr[i] = static_cast<int>(i * 2);

        const auto &cref = arr;
        auto s = cref.as_span();

        jh::test::tiny_test::expect(static_cast<bool>((s.size() == N)), "s.size() == N");
        jh::test::tiny_test::expect(static_cast<bool>((std::is_same_v<decltype(s), std::span<const int>>)), "std::is_same_v<decltype(s), std::span<const int>>");
        jh::test::tiny_test::expect(static_cast<bool>((s[3] == 6)), "s[3] == 6");
    }

}
void tiny_test_case_5_section_1() { tiny_test_case_5_body(1); }
void tiny_test_case_5_section_2() { tiny_test_case_5_body(2); }
}
template<>
struct jh::test::tiny_test::test<"runtime_arr<T> as_span() and const variant / as_span() non-const reflects underlying data">
    : jh::test::tiny_test::test_definition<"runtime_arr<T> as_span() and const variant / as_span() non-const reflects underlying data", &::test::tiny_test_case_5_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_runtime_arr case 13">
    : jh::test::tiny_test::session_definition<
          "test module test_runtime_arr case 13", jh::test::tiny_test::test<"runtime_arr<T> as_span() and const variant / as_span() non-const reflects underlying data">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_runtime_arr case 13"> registration_13{};
}
template<>
struct jh::test::tiny_test::test<"runtime_arr<T> as_span() and const variant / as_span() const returns read-only view">
    : jh::test::tiny_test::test_definition<"runtime_arr<T> as_span() and const variant / as_span() const returns read-only view", &::test::tiny_test_case_5_section_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_runtime_arr case 14">
    : jh::test::tiny_test::session_definition<
          "test module test_runtime_arr case 14", jh::test::tiny_test::test<"runtime_arr<T> as_span() and const variant / as_span() const returns read-only view">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_runtime_arr case 14"> registration_14{};
}




namespace test {
void tiny_test_case_6_body(const int selected_section) {
    constexpr std::size_t N = 128;
    runtime_arr<bool> bits(N);

    if (selected_section == 1) {
        std::mt19937 rng(123);  // NOLINT
        std::bernoulli_distribution dist(0.5);
        std::vector<bool> ref(N);

        for (std::size_t i = 0; i < N; ++i) {
            bool b = dist(rng);
            ref[i] = b;
            bits.set(i, b);
        }

        for (std::size_t i = 0; i < N; ++i) {
            jh::test::tiny_test::expect(static_cast<bool>((static_cast<bool>(bits[i]) == static_cast<bool>(ref[i]))), "static_cast<bool>(bits[i]) == static_cast<bool>(ref[i])");
        }
    }

    if (selected_section == 2) {
        bits.reset_all();
        for (std::size_t i = 0; i < N; ++i)
            jh::test::tiny_test::expect(static_cast<bool>((!bits[i])), "!bits[i]");
    }

    if (selected_section == 3) {
        bits.reset_all();
        bits.set(3);
        bits.set(7);
        jh::test::tiny_test::expect(static_cast<bool>((bits.test(3))), "bits.test(3)");
        jh::test::tiny_test::expect(static_cast<bool>((bits.test(7))), "bits.test(7)");

        bits.unset(3);
        jh::test::tiny_test::expect_not(static_cast<bool>((bits.test(3))), "bits.test(3)");
        jh::test::tiny_test::expect(static_cast<bool>((bits.test(7))), "bits.test(7)");
    }

    if (selected_section == 4) {
        bits.reset_all();
        constexpr std::size_t NWORDS = (N + 63) / 64;
        auto *raw = bits.raw_data();
        jh::test::tiny_test::expect(static_cast<bool>((raw != nullptr)), "raw != nullptr");
        jh::test::tiny_test::expect(static_cast<bool>((bits.raw_word_count() == NWORDS)), "bits.raw_word_count() == NWORDS");

        bits.set(1);
        bits.set(65);

        jh::test::tiny_test::expect(static_cast<bool>(((raw[0] & (1ULL << 1)) != 0)), "(raw[0] & (1ULL << 1)) != 0");
        jh::test::tiny_test::expect(static_cast<bool>(((raw[1] & (1ULL << 1)) != 0)), "(raw[1] & (1ULL << 1)) != 0");
        jh::test::tiny_test::expect(static_cast<bool>((bits.test(1))), "bits.test(1)");
        jh::test::tiny_test::expect(static_cast<bool>((bits.test(65))), "bits.test(65)");
    }


}
void tiny_test_case_6_section_1() { tiny_test_case_6_body(1); }
void tiny_test_case_6_section_2() { tiny_test_case_6_body(2); }
void tiny_test_case_6_section_3() { tiny_test_case_6_body(3); }
void tiny_test_case_6_section_4() { tiny_test_case_6_body(4); }
}
template<>
struct jh::test::tiny_test::test<"runtime_arr<bool> full test / set bits randomly and test">
    : jh::test::tiny_test::test_definition<"runtime_arr<bool> full test / set bits randomly and test", &::test::tiny_test_case_6_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_runtime_arr case 15">
    : jh::test::tiny_test::session_definition<
          "test module test_runtime_arr case 15", jh::test::tiny_test::test<"runtime_arr<bool> full test / set bits randomly and test">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_runtime_arr case 15"> registration_15{};
}
template<>
struct jh::test::tiny_test::test<"runtime_arr<bool> full test / reset_all and verify zeroed">
    : jh::test::tiny_test::test_definition<"runtime_arr<bool> full test / reset_all and verify zeroed", &::test::tiny_test_case_6_section_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_runtime_arr case 16">
    : jh::test::tiny_test::session_definition<
          "test module test_runtime_arr case 16", jh::test::tiny_test::test<"runtime_arr<bool> full test / reset_all and verify zeroed">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_runtime_arr case 16"> registration_16{};
}
template<>
struct jh::test::tiny_test::test<"runtime_arr<bool> full test / unset and test specific bits">
    : jh::test::tiny_test::test_definition<"runtime_arr<bool> full test / unset and test specific bits", &::test::tiny_test_case_6_section_3> {};
template<>
struct jh::test::tiny_test::session<"test module test_runtime_arr case 17">
    : jh::test::tiny_test::session_definition<
          "test module test_runtime_arr case 17", jh::test::tiny_test::test<"runtime_arr<bool> full test / unset and test specific bits">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_runtime_arr case 17"> registration_17{};
}
template<>
struct jh::test::tiny_test::test<"runtime_arr<bool> full test / raw_data and raw_word_count structure">
    : jh::test::tiny_test::test_definition<"runtime_arr<bool> full test / raw_data and raw_word_count structure", &::test::tiny_test_case_6_section_4> {};
template<>
struct jh::test::tiny_test::session<"test module test_runtime_arr case 18">
    : jh::test::tiny_test::session_definition<
          "test module test_runtime_arr case 18", jh::test::tiny_test::test<"runtime_arr<bool> full test / raw_data and raw_word_count structure">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_runtime_arr case 18"> registration_18{};
}



namespace test {
void tiny_test_case_7_body(const int selected_section) {
    using namespace jh;

    if (selected_section == 1) {
        runtime_arr<int> arr{1, 2, 3, 4, 5};
        jh::test::tiny_test::expect(static_cast<bool>((arr.size() == 5)), "arr.size() == 5");
        for (int i = 0; i < 5; ++i)
            jh::test::tiny_test::expect(static_cast<bool>((arr[i] == i + 1)), "arr[i] == i + 1");
    }

    if (selected_section == 2) {
        std::pmr::monotonic_buffer_resource res;
        runtime_arr<int, std::pmr::polymorphic_allocator<int>> arr({10, 20, 30},
                                                                   std::pmr::polymorphic_allocator<int>(&res));
        jh::test::tiny_test::expect(static_cast<bool>((arr.size() == 3)), "arr.size() == 3");
        jh::test::tiny_test::expect(static_cast<bool>((arr[0] == 10)), "arr[0] == 10");
        jh::test::tiny_test::expect(static_cast<bool>((arr[1] == 20)), "arr[1] == 20");
        jh::test::tiny_test::expect(static_cast<bool>((arr[2] == 30)), "arr[2] == 30");
    }

    if (selected_section == 3) {
        runtime_arr<bool> bits{true, false, true, true, false};
        jh::test::tiny_test::expect(static_cast<bool>((bits.size() == 5)), "bits.size() == 5");
        jh::test::tiny_test::expect(static_cast<bool>((bits.test(0))), "bits.test(0)");
        jh::test::tiny_test::expect_not(static_cast<bool>((bits.test(1))), "bits.test(1)");
        jh::test::tiny_test::expect(static_cast<bool>((bits.test(2))), "bits.test(2)");
        jh::test::tiny_test::expect(static_cast<bool>((bits.test(3))), "bits.test(3)");
        jh::test::tiny_test::expect_not(static_cast<bool>((bits.test(4))), "bits.test(4)");
    }

    if (selected_section == 4) {
        using jh::runtime_arr_helper::bool_flat_alloc;
        runtime_arr<bool, bool_flat_alloc> arr{{true, false, false, true},
                                               {}};
        jh::test::tiny_test::expect(static_cast<bool>((arr.size() == 4)), "arr.size() == 4");
        jh::test::tiny_test::expect(static_cast<bool>((arr[0])), "arr[0]");
        jh::test::tiny_test::expect_not(static_cast<bool>((arr[1])), "arr[1]");
        jh::test::tiny_test::expect_not(static_cast<bool>((arr[2])), "arr[2]");
        jh::test::tiny_test::expect(static_cast<bool>((arr[3])), "arr[3]");
    }

}
void tiny_test_case_7_section_1() { tiny_test_case_7_body(1); }
void tiny_test_case_7_section_2() { tiny_test_case_7_body(2); }
void tiny_test_case_7_section_3() { tiny_test_case_7_body(3); }
void tiny_test_case_7_section_4() { tiny_test_case_7_body(4); }
}
template<>
struct jh::test::tiny_test::test<"runtime_arr initializer_list construction / int version">
    : jh::test::tiny_test::test_definition<"runtime_arr initializer_list construction / int version", &::test::tiny_test_case_7_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_runtime_arr case 19">
    : jh::test::tiny_test::session_definition<
          "test module test_runtime_arr case 19", jh::test::tiny_test::test<"runtime_arr initializer_list construction / int version">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_runtime_arr case 19"> registration_19{};
}
template<>
struct jh::test::tiny_test::test<"runtime_arr initializer_list construction / pmr<int> version">
    : jh::test::tiny_test::test_definition<"runtime_arr initializer_list construction / pmr<int> version", &::test::tiny_test_case_7_section_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_runtime_arr case 20">
    : jh::test::tiny_test::session_definition<
          "test module test_runtime_arr case 20", jh::test::tiny_test::test<"runtime_arr initializer_list construction / pmr<int> version">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_runtime_arr case 20"> registration_20{};
}
template<>
struct jh::test::tiny_test::test<"runtime_arr initializer_list construction / bit-packed bool version">
    : jh::test::tiny_test::test_definition<"runtime_arr initializer_list construction / bit-packed bool version", &::test::tiny_test_case_7_section_3> {};
template<>
struct jh::test::tiny_test::session<"test module test_runtime_arr case 21">
    : jh::test::tiny_test::session_definition<
          "test module test_runtime_arr case 21", jh::test::tiny_test::test<"runtime_arr initializer_list construction / bit-packed bool version">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_runtime_arr case 21"> registration_21{};
}
template<>
struct jh::test::tiny_test::test<"runtime_arr initializer_list construction / flat bool version (byte-based allocator)">
    : jh::test::tiny_test::test_definition<"runtime_arr initializer_list construction / flat bool version (byte-based allocator)", &::test::tiny_test_case_7_section_4> {};
template<>
struct jh::test::tiny_test::session<"test module test_runtime_arr case 22">
    : jh::test::tiny_test::session_definition<
          "test module test_runtime_arr case 22", jh::test::tiny_test::test<"runtime_arr initializer_list construction / flat bool version (byte-based allocator)">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_runtime_arr case 22"> registration_22{};
}



namespace test {
void tiny_test_case_8_body(const int selected_section) {
    using AllocD = std::allocator<double>;
    using Arr = runtime_arr<int, AllocD>;

    if (selected_section == 1) {
        Arr arr(8, AllocD{});   // AllocD::value_type = double, but T = int → MUST rebind

        for (int i = 0; i < 8; ++i)
            arr.set(i, i * 10);

        for (int i = 0; i < 8; ++i)
            jh::test::tiny_test::expect(static_cast<bool>((arr[i] == i * 10)), "arr[i] == i * 10");
    }

    if (selected_section == 2) {
        Arr arr(5, AllocD{});

        for (int i = 0; i < 5; ++i)
            arr.set(i, 123);

        arr.reset_all();

        for (int i = 0; i < 5; ++i)
            jh::test::tiny_test::expect(static_cast<bool>((arr[i] == 0)), "arr[i] == 0");
    }

    if (selected_section == 3) {
        Arr arr({1, 2, 3, 4}, AllocD{});

        jh::test::tiny_test::expect(static_cast<bool>((arr.size() == 4)), "arr.size() == 4");
        jh::test::tiny_test::expect(static_cast<bool>((arr[0] == 1)), "arr[0] == 1");
        jh::test::tiny_test::expect(static_cast<bool>((arr[1] == 2)), "arr[1] == 2");
        jh::test::tiny_test::expect(static_cast<bool>((arr[2] == 3)), "arr[2] == 3");
        jh::test::tiny_test::expect(static_cast<bool>((arr[3] == 4)), "arr[3] == 4");
    }

    if (selected_section == 4) {
        Arr arr(4, AllocD{});
        arr.set(0, 10);
        arr.set(1, 20);
        arr.set(2, 30);
        arr.set(3, 40);

        Arr moved = std::move(arr);

        jh::test::tiny_test::expect(static_cast<bool>((moved[0] == 10)), "moved[0] == 10");
        jh::test::tiny_test::expect(static_cast<bool>((moved[1] == 20)), "moved[1] == 20");
        jh::test::tiny_test::expect(static_cast<bool>((moved[2] == 30)), "moved[2] == 30");
        jh::test::tiny_test::expect(static_cast<bool>((moved[3] == 40)), "moved[3] == 40");
        jh::test::tiny_test::expect(static_cast<bool>((arr.data() == nullptr)), "arr.data() == nullptr");  // moved-from safety
    }

    if (selected_section == 5) {
        Arr arr({5, 6, 7, 8}, AllocD{});

        std::vector<int> vec = static_cast<std::vector<int>>(std::move(arr));

        jh::test::tiny_test::expect(static_cast<bool>((vec.size() == 4)), "vec.size() == 4");
        jh::test::tiny_test::expect(static_cast<bool>((vec[0] == 5)), "vec[0] == 5");
        jh::test::tiny_test::expect(static_cast<bool>((vec[1] == 6)), "vec[1] == 6");
        jh::test::tiny_test::expect(static_cast<bool>((vec[2] == 7)), "vec[2] == 7");
        jh::test::tiny_test::expect(static_cast<bool>((vec[3] == 8)), "vec[3] == 8");
    }

}
void tiny_test_case_8_section_1() { tiny_test_case_8_body(1); }
void tiny_test_case_8_section_2() { tiny_test_case_8_body(2); }
void tiny_test_case_8_section_3() { tiny_test_case_8_body(3); }
void tiny_test_case_8_section_4() { tiny_test_case_8_body(4); }
void tiny_test_case_8_section_5() { tiny_test_case_8_body(5); }
}
template<>
struct jh::test::tiny_test::test<"runtime_arr<int, std::allocator<double>> rebind behavior / basic construction and write/read">
    : jh::test::tiny_test::test_definition<"runtime_arr<int, std::allocator<double>> rebind behavior / basic construction and write/read", &::test::tiny_test_case_8_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_runtime_arr case 23">
    : jh::test::tiny_test::session_definition<
          "test module test_runtime_arr case 23", jh::test::tiny_test::test<"runtime_arr<int, std::allocator<double>> rebind behavior / basic construction and write/read">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_runtime_arr case 23"> registration_23{};
}
template<>
struct jh::test::tiny_test::test<"runtime_arr<int, std::allocator<double>> rebind behavior / reset_all works">
    : jh::test::tiny_test::test_definition<"runtime_arr<int, std::allocator<double>> rebind behavior / reset_all works", &::test::tiny_test_case_8_section_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_runtime_arr case 24">
    : jh::test::tiny_test::session_definition<
          "test module test_runtime_arr case 24", jh::test::tiny_test::test<"runtime_arr<int, std::allocator<double>> rebind behavior / reset_all works">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_runtime_arr case 24"> registration_24{};
}
template<>
struct jh::test::tiny_test::test<"runtime_arr<int, std::allocator<double>> rebind behavior / initializer_list construction works">
    : jh::test::tiny_test::test_definition<"runtime_arr<int, std::allocator<double>> rebind behavior / initializer_list construction works", &::test::tiny_test_case_8_section_3> {};
template<>
struct jh::test::tiny_test::session<"test module test_runtime_arr case 25">
    : jh::test::tiny_test::session_definition<
          "test module test_runtime_arr case 25", jh::test::tiny_test::test<"runtime_arr<int, std::allocator<double>> rebind behavior / initializer_list construction works">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_runtime_arr case 25"> registration_25{};
}
template<>
struct jh::test::tiny_test::test<"runtime_arr<int, std::allocator<double>> rebind behavior / move construction keeps values">
    : jh::test::tiny_test::test_definition<"runtime_arr<int, std::allocator<double>> rebind behavior / move construction keeps values", &::test::tiny_test_case_8_section_4> {};
template<>
struct jh::test::tiny_test::session<"test module test_runtime_arr case 26">
    : jh::test::tiny_test::session_definition<
          "test module test_runtime_arr case 26", jh::test::tiny_test::test<"runtime_arr<int, std::allocator<double>> rebind behavior / move construction keeps values">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_runtime_arr case 26"> registration_26{};
}
template<>
struct jh::test::tiny_test::test<"runtime_arr<int, std::allocator<double>> rebind behavior / conversion to vector<int> still works">
    : jh::test::tiny_test::test_definition<"runtime_arr<int, std::allocator<double>> rebind behavior / conversion to vector<int> still works", &::test::tiny_test_case_8_section_5> {};
template<>
struct jh::test::tiny_test::session<"test module test_runtime_arr case 27">
    : jh::test::tiny_test::session_definition<
          "test module test_runtime_arr case 27", jh::test::tiny_test::test<"runtime_arr<int, std::allocator<double>> rebind behavior / conversion to vector<int> still works">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_runtime_arr case 27"> registration_27{};
}



namespace test {
void tiny_test_case_9_body(const int selected_section) {

    if (selected_section == 1) {

        using VecAlloc = std::allocator<int>;
        using ArrAlloc = test_allocator<int>;

        std::vector<int, VecAlloc> vec{1,2,3,4,5};

        runtime_arr<int, ArrAlloc> arr(std::move(vec), ArrAlloc{});

        jh::test::tiny_test::expect(static_cast<bool>((arr.size() == 5)), "arr.size() == 5");
        for (int i = 0; i < 5; ++i)
            jh::test::tiny_test::expect(static_cast<bool>((arr[i] == i + 1)), "arr[i] == i + 1");
    }

    if (selected_section == 2) {

        std::pmr::monotonic_buffer_resource res;

        std::vector<int, std::pmr::polymorphic_allocator<int>> vec(
                {10,20,30,40},
                std::pmr::polymorphic_allocator<int>(&res)
        );

        runtime_arr<int, std::allocator<int>> arr(std::move(vec), std::allocator<int>{});

        jh::test::tiny_test::expect(static_cast<bool>((arr.size() == 4)), "arr.size() == 4");
        jh::test::tiny_test::expect(static_cast<bool>((arr[0] == 10)), "arr[0] == 10");
        jh::test::tiny_test::expect(static_cast<bool>((arr[3] == 40)), "arr[3] == 40");
    }

    if (selected_section == 3) {

        std::pmr::monotonic_buffer_resource res;

        std::vector<int, std::pmr::polymorphic_allocator<int>> vec(
                {7,8,9},
                std::pmr::polymorphic_allocator<int>(&res)
        );

        runtime_arr<int, std::pmr::polymorphic_allocator<int>> arr(std::move(vec));

        jh::test::tiny_test::expect(static_cast<bool>((arr.size() == 3)), "arr.size() == 3");
        jh::test::tiny_test::expect(static_cast<bool>((arr[0] == 7)), "arr[0] == 7");
        jh::test::tiny_test::expect(static_cast<bool>((arr[2] == 9)), "arr[2] == 9");
    }

    if (selected_section == 4) {

        std::vector<int> src{11,22,33,44};

        runtime_arr<int, test_allocator<int>> arr(
                src.begin(),
                src.end(),
                test_allocator<int>{}
        );

        jh::test::tiny_test::expect(static_cast<bool>((arr.size() == 4)), "arr.size() == 4");
        jh::test::tiny_test::expect(static_cast<bool>((arr[0] == 11)), "arr[0] == 11");
        jh::test::tiny_test::expect(static_cast<bool>((arr[3] == 44)), "arr[3] == 44");
    }

    if (selected_section == 5) {

        std::vector<int> src{5,6,7};

        runtime_arr<int, test_allocator<int>> arr(
                src.begin(),
                src.end(),
                test_allocator<int>{}
        );

        arr.reset_all();

        for (int i : arr)
            jh::test::tiny_test::expect(static_cast<bool>((i == 0)), "i == 0");
    }

}
void tiny_test_case_9_section_1() { tiny_test_case_9_body(1); }
void tiny_test_case_9_section_2() { tiny_test_case_9_body(2); }
void tiny_test_case_9_section_3() { tiny_test_case_9_body(3); }
void tiny_test_case_9_section_4() { tiny_test_case_9_body(4); }
void tiny_test_case_9_section_5() { tiny_test_case_9_body(5); }
}
template<>
struct jh::test::tiny_test::test<"runtime_arr allocator-aware constructors from vector and iterators / vector with different allocator -> runtime_arr with custom allocator">
    : jh::test::tiny_test::test_definition<"runtime_arr allocator-aware constructors from vector and iterators / vector with different allocator -> runtime_arr with custom allocator", &::test::tiny_test_case_9_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_runtime_arr case 28">
    : jh::test::tiny_test::session_definition<
          "test module test_runtime_arr case 28", jh::test::tiny_test::test<"runtime_arr allocator-aware constructors from vector and iterators / vector with different allocator -> runtime_arr with custom allocator">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_runtime_arr case 28"> registration_28{};
}
template<>
struct jh::test::tiny_test::test<"runtime_arr allocator-aware constructors from vector and iterators / vector with pmr allocator -> runtime_arr with std::allocator">
    : jh::test::tiny_test::test_definition<"runtime_arr allocator-aware constructors from vector and iterators / vector with pmr allocator -> runtime_arr with std::allocator", &::test::tiny_test_case_9_section_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_runtime_arr case 29">
    : jh::test::tiny_test::session_definition<
          "test module test_runtime_arr case 29", jh::test::tiny_test::test<"runtime_arr allocator-aware constructors from vector and iterators / vector with pmr allocator -> runtime_arr with std::allocator">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_runtime_arr case 29"> registration_29{};
}
template<>
struct jh::test::tiny_test::test<"runtime_arr allocator-aware constructors from vector and iterators / vector<T, Alloc> constructor using vec.get_allocator()">
    : jh::test::tiny_test::test_definition<"runtime_arr allocator-aware constructors from vector and iterators / vector<T, Alloc> constructor using vec.get_allocator()", &::test::tiny_test_case_9_section_3> {};
template<>
struct jh::test::tiny_test::session<"test module test_runtime_arr case 30">
    : jh::test::tiny_test::session_definition<
          "test module test_runtime_arr case 30", jh::test::tiny_test::test<"runtime_arr allocator-aware constructors from vector and iterators / vector<T, Alloc> constructor using vec.get_allocator()">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_runtime_arr case 30"> registration_30{};
}
template<>
struct jh::test::tiny_test::test<"runtime_arr allocator-aware constructors from vector and iterators / iterator + allocator constructor">
    : jh::test::tiny_test::test_definition<"runtime_arr allocator-aware constructors from vector and iterators / iterator + allocator constructor", &::test::tiny_test_case_9_section_4> {};
template<>
struct jh::test::tiny_test::session<"test module test_runtime_arr case 31">
    : jh::test::tiny_test::session_definition<
          "test module test_runtime_arr case 31", jh::test::tiny_test::test<"runtime_arr allocator-aware constructors from vector and iterators / iterator + allocator constructor">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_runtime_arr case 31"> registration_31{};
}
template<>
struct jh::test::tiny_test::test<"runtime_arr allocator-aware constructors from vector and iterators / iterator + allocator reset_all behavior">
    : jh::test::tiny_test::test_definition<"runtime_arr allocator-aware constructors from vector and iterators / iterator + allocator reset_all behavior", &::test::tiny_test_case_9_section_5> {};
template<>
struct jh::test::tiny_test::session<"test module test_runtime_arr case 32">
    : jh::test::tiny_test::session_definition<
          "test module test_runtime_arr case 32", jh::test::tiny_test::test<"runtime_arr allocator-aware constructors from vector and iterators / iterator + allocator reset_all behavior">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_runtime_arr case 32"> registration_32{};
}

