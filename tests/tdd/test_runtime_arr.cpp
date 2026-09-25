#include <catch2/catch_all.hpp>

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

TEST_CASE("runtime_arr<int> full test", "[pod]") {
    constexpr int N = 32;
    runtime_arr<int> arr(N);

    SECTION("set and verify values") {
        for (int i = 0; i < N; ++i)
            arr[i] = i * i;

        for (int i = 0; i < N; ++i)
            REQUIRE(arr[i] == i * i);
    }

    SECTION("reset_all clears to zero") {
        arr.reset_all();
        for (int i = 0; i < N; ++i)
            REQUIRE(arr[i] == 0);
    }

    SECTION("move constructor") {
        for (int i = 0; i < N; ++i)
            arr[i] = i;

        runtime_arr<int> moved = std::move(arr);
        for (int i = 0; i < N; ++i)
            REQUIRE(moved[i] == i);
        REQUIRE(arr.data() == nullptr);
    }

    SECTION("conversion to vector") {
        for (int i = 0; i < N; ++i)
            arr[i] = N - i;

        std::vector<int> vec = static_cast<std::vector<int>>(std::move(arr));
        for (int i = 0; i < N; ++i)
            REQUIRE(vec[i] == N - i);
    }
}


JH_POD_STRUCT(MyPod,
              int id;
                      float score;
);

TEST_CASE("runtime_arr<MyPod> full test", "[pod][struct]") {
    constexpr int N = 64;
    runtime_arr<MyPod> arr(N);

    SECTION("initialize values") {
        for (int i = 0; i < N; ++i) {
            arr.set(i, i, static_cast<float>(i) * 0.5f);
        }

        for (int i = 0; i < N; ++i) {
            REQUIRE(arr[i].id == i);
            REQUIRE(arr[i].score == Catch::Approx(i * 0.5f));
        }
    }

    SECTION("reset_all to zero") {
        arr.reset_all();
        for (int i = 0; i < N; ++i) {
            REQUIRE(arr[i].id == 0);
            REQUIRE(arr[i].score == 0.0f);
        }
    }

    SECTION("move to vector<MyPod>") {
        for (int i = 0; i < N; ++i)
            arr.set(i, i, static_cast<float>(i) + 0.1f);

        std::vector<MyPod> vec = static_cast<std::vector<MyPod>>(std::move(arr));
        REQUIRE(vec.size() == N);
        REQUIRE(vec[5].id == 5);
        REQUIRE(vec[5].score == Catch::Approx(5.1f));
    }

    SECTION("move construction keeps values") {
        for (int i = 0; i < N; ++i)
            arr.set(i, 100 + i, 2.0f * static_cast<float>(i));

        auto moved = std::move(arr);
        for (int i = 0; i < N; ++i) {
            REQUIRE(moved[i].id == 100 + i);
            REQUIRE(moved[i].score == Catch::Approx(2.0f * i));
        }
    }
}


TEST_CASE("runtime_arr<int, test_allocator> behavior", "[pod][alloc]") {
    using T = runtime_arr<int, test_allocator<int> >;
    T arr(5, test_allocator<int>{});

    SECTION("set and get") {
        for (int i = 0; i < 5; ++i)
            arr.set(i, i + 100);
        REQUIRE(arr[2] == 102);
    }

    SECTION("reset_all and verify") {
        arr.reset_all();
        for (int i = 0; i < 5; ++i)
            REQUIRE(arr[i] == 0);
    }
}

#if 0 // Ten-million-element allocation is a load test, not a correctness-suite case.
TEST_CASE("runtime_arr<int, std::allocator> for very large allocation", "[pod][alloc]") {
    constexpr int N = 10'000'000;
    runtime_arr<int, std::allocator<int>> arr{N, std::allocator<int>{}};

    SECTION("set and get") {
        for (int i = 0; i < N; ++i)
            arr.set(i, i + 100);
        REQUIRE(arr[2] == 102);
    }

    SECTION("reset_all and verify") {
        arr.reset_all();
        for (int i = 0; i < N; ++i)
            REQUIRE(arr[i] == 0);
    }
}
#endif

TEST_CASE("runtime_arr<tuple> structured ops", "[non-pod]") {
    using tup = std::tuple<int, int>;

    runtime_arr<tup> arr(3);

    SECTION("set and access via get") {
        arr.set(0, 10, 20);
        arr.set(1, 30, 40);
        REQUIRE(std::get<0>(arr[1]) == 30);
        REQUIRE(std::get<1>(arr[1]) == 40);
    }

    SECTION("move to vector") {
        arr.set(0, 1, 2);
        arr.set(1, 3, 4);
        arr.set(2, 5, 6);
        std::vector<tup> vec = static_cast<std::vector<tup>>(std::move(arr));
        REQUIRE(vec[2] == std::make_tuple(5, 6));
    }
}

TEST_CASE("runtime_arr<T> as_span() and const variant", "[span]") {
    constexpr std::size_t N = 8;
    jh::runtime_arr<int> arr(N);

    for (std::size_t i = 0; i < N; ++i)
        arr[i] = static_cast<int>(i * 2);

    SECTION("as_span() non-const reflects underlying data") {
        auto s = arr.as_span();
        REQUIRE(s.size() == N);
        for (std::size_t i = 0; i < N; ++i)
            REQUIRE(s[i] == static_cast<int>(i * 2));

        s[3] = 999;
        REQUIRE(arr[3] == 999);
    }

    SECTION("as_span() const returns read-only view") {
        for (std::size_t i = 0; i < N; ++i)
            arr[i] = static_cast<int>(i * 2);

        const auto &cref = arr;
        auto s = cref.as_span();

        REQUIRE(s.size() == N);
        REQUIRE(std::is_same_v<decltype(s), std::span<const int>>);
        REQUIRE(s[3] == 6);
    }
}


TEST_CASE("runtime_arr<bool> full test", "[bool]") {
    constexpr std::size_t N = 128;
    runtime_arr<bool> bits(N);

    SECTION("set bits randomly and test") {
        std::mt19937 rng(123);  // NOLINT
        std::bernoulli_distribution dist(0.5);
        std::vector<bool> ref(N);

        for (std::size_t i = 0; i < N; ++i) {
            bool b = dist(rng);
            ref[i] = b;
            bits.set(i, b);
        }

        for (std::size_t i = 0; i < N; ++i) {
            REQUIRE(static_cast<bool>(bits[i]) == static_cast<bool>(ref[i]));
        }
    }

    SECTION("reset_all and verify zeroed") {
        bits.reset_all();
        for (std::size_t i = 0; i < N; ++i)
            REQUIRE(!bits[i]);
    }

    SECTION("unset and test specific bits") {
        bits.reset_all();
        bits.set(3);
        bits.set(7);
        REQUIRE(bits.test(3));
        REQUIRE(bits.test(7));

        bits.unset(3);
        REQUIRE_FALSE(bits.test(3));
        REQUIRE(bits.test(7));
    }

    SECTION("raw_data and raw_word_count structure") {
        bits.reset_all();
        constexpr std::size_t NWORDS = (N + 63) / 64;
        auto *raw = bits.raw_data();
        REQUIRE(raw != nullptr);
        REQUIRE(bits.raw_word_count() == NWORDS);

        bits.set(1);
        bits.set(65);

        REQUIRE((raw[0] & (1ULL << 1)) != 0);
        REQUIRE((raw[1] & (1ULL << 1)) != 0);
        REQUIRE(bits.test(1));
        REQUIRE(bits.test(65));
    }

}

TEST_CASE("runtime_arr initializer_list construction", "[initlist]") {
    using namespace jh;

    SECTION("int version") {
        runtime_arr<int> arr{1, 2, 3, 4, 5};
        REQUIRE(arr.size() == 5);
        for (int i = 0; i < 5; ++i)
            REQUIRE(arr[i] == i + 1);
    }

    SECTION("pmr<int> version") {
        std::pmr::monotonic_buffer_resource res;
        runtime_arr<int, std::pmr::polymorphic_allocator<int>> arr({10, 20, 30},
                                                                   std::pmr::polymorphic_allocator<int>(&res));
        REQUIRE(arr.size() == 3);
        REQUIRE(arr[0] == 10);
        REQUIRE(arr[1] == 20);
        REQUIRE(arr[2] == 30);
    }

    SECTION("bit-packed bool version") {
        runtime_arr<bool> bits{true, false, true, true, false};
        REQUIRE(bits.size() == 5);
        REQUIRE(bits.test(0));
        REQUIRE_FALSE(bits.test(1));
        REQUIRE(bits.test(2));
        REQUIRE(bits.test(3));
        REQUIRE_FALSE(bits.test(4));
    }

    SECTION("flat bool version (byte-based allocator)") {
        using jh::runtime_arr_helper::bool_flat_alloc;
        runtime_arr<bool, bool_flat_alloc> arr{{true, false, false, true},
                                               {}};
        REQUIRE(arr.size() == 4);
        REQUIRE(arr[0]);
        REQUIRE_FALSE(arr[1]);
        REQUIRE_FALSE(arr[2]);
        REQUIRE(arr[3]);
    }
}

TEST_CASE("runtime_arr<int, std::allocator<double>> rebind behavior", "[alloc][rebind]") {
    using AllocD = std::allocator<double>;
    using Arr = runtime_arr<int, AllocD>;

    SECTION("basic construction and write/read") {
        Arr arr(8, AllocD{});   // AllocD::value_type = double, but T = int → MUST rebind

        for (int i = 0; i < 8; ++i)
            arr.set(i, i * 10);

        for (int i = 0; i < 8; ++i)
            REQUIRE(arr[i] == i * 10);
    }

    SECTION("reset_all works") {
        Arr arr(5, AllocD{});

        for (int i = 0; i < 5; ++i)
            arr.set(i, 123);

        arr.reset_all();

        for (int i = 0; i < 5; ++i)
            REQUIRE(arr[i] == 0);
    }

    SECTION("initializer_list construction works") {
        Arr arr({1, 2, 3, 4}, AllocD{});

        REQUIRE(arr.size() == 4);
        REQUIRE(arr[0] == 1);
        REQUIRE(arr[1] == 2);
        REQUIRE(arr[2] == 3);
        REQUIRE(arr[3] == 4);
    }

    SECTION("move construction keeps values") {
        Arr arr(4, AllocD{});
        arr.set(0, 10);
        arr.set(1, 20);
        arr.set(2, 30);
        arr.set(3, 40);

        Arr moved = std::move(arr);

        REQUIRE(moved[0] == 10);
        REQUIRE(moved[1] == 20);
        REQUIRE(moved[2] == 30);
        REQUIRE(moved[3] == 40);
        REQUIRE(arr.data() == nullptr);  // moved-from safety
    }

    SECTION("conversion to vector<int> still works") {
        Arr arr({5, 6, 7, 8}, AllocD{});

        std::vector<int> vec = static_cast<std::vector<int>>(std::move(arr));

        REQUIRE(vec.size() == 4);
        REQUIRE(vec[0] == 5);
        REQUIRE(vec[1] == 6);
        REQUIRE(vec[2] == 7);
        REQUIRE(vec[3] == 8);
    }
}

TEST_CASE("runtime_arr allocator-aware constructors from vector and iterators", "[alloc][vector][iterator]") {

    SECTION("vector with different allocator -> runtime_arr with custom allocator") {

        using VecAlloc = std::allocator<int>;
        using ArrAlloc = test_allocator<int>;

        std::vector<int, VecAlloc> vec{1,2,3,4,5};

        runtime_arr<int, ArrAlloc> arr(std::move(vec), ArrAlloc{});

        REQUIRE(arr.size() == 5);
        for (int i = 0; i < 5; ++i)
            REQUIRE(arr[i] == i + 1);
    }

    SECTION("vector with pmr allocator -> runtime_arr with std::allocator") {

        std::pmr::monotonic_buffer_resource res;

        std::vector<int, std::pmr::polymorphic_allocator<int>> vec(
                {10,20,30,40},
                std::pmr::polymorphic_allocator<int>(&res)
        );

        runtime_arr<int, std::allocator<int>> arr(std::move(vec), std::allocator<int>{});

        REQUIRE(arr.size() == 4);
        REQUIRE(arr[0] == 10);
        REQUIRE(arr[3] == 40);
    }

    SECTION("vector<T, Alloc> constructor using vec.get_allocator()") {

        std::pmr::monotonic_buffer_resource res;

        std::vector<int, std::pmr::polymorphic_allocator<int>> vec(
                {7,8,9},
                std::pmr::polymorphic_allocator<int>(&res)
        );

        runtime_arr<int, std::pmr::polymorphic_allocator<int>> arr(std::move(vec));

        REQUIRE(arr.size() == 3);
        REQUIRE(arr[0] == 7);
        REQUIRE(arr[2] == 9);
    }

    SECTION("iterator + allocator constructor") {

        std::vector<int> src{11,22,33,44};

        runtime_arr<int, test_allocator<int>> arr(
                src.begin(),
                src.end(),
                test_allocator<int>{}
        );

        REQUIRE(arr.size() == 4);
        REQUIRE(arr[0] == 11);
        REQUIRE(arr[3] == 44);
    }

    SECTION("iterator + allocator reset_all behavior") {

        std::vector<int> src{5,6,7};

        runtime_arr<int, test_allocator<int>> arr(
                src.begin(),
                src.end(),
                test_allocator<int>{}
        );

        arr.reset_all();

        for (int i : arr)
            REQUIRE(i == 0);
    }
}
