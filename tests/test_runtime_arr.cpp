#define CATCH_CONFIG_MAIN

#include <catch2/catch_all.hpp>
#include "jh/runtime_arr.h"
#include <tuple>
#include <memory>
#include <vector>
#include <random>
#include "jh/pod.h"

using namespace jh;

// concepts for static interface checks
template<typename T>
concept has_data = requires(T t) {
    { t.data() };
};

template<typename T>
concept bool_like = requires(T t) {
    { static_cast<bool>(t) } -> std::same_as<bool>;
};

template<typename T>
concept convertible_to_bool = std::is_convertible_v<T, bool>;


template<typename T>
concept has_address_of = requires(T t) {
    { &t[0] };
};

template<typename T>
using begin_t = decltype(std::declval<T>().begin());


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
    constexpr int N = 100'000;
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

    SECTION("roundtrip vector<bool> conversion") {
        std::vector ref = {true, false, true, true, false};
        runtime_arr tmp(std::move(ref));
        auto out = static_cast<std::vector<bool>>(std::move(tmp));
        REQUIRE(out == std::vector<bool>({true, false, true, true, false}));
    }
}

TEST_CASE("concept checks for runtime_arr<T> and runtime_arr<bool>", "[concepts]") {
    SECTION("data() presence") {
        STATIC_REQUIRE(has_data<runtime_arr<int>>);
        STATIC_REQUIRE(!has_data<runtime_arr<bool>>);
        STATIC_REQUIRE(has_data<runtime_arr<int, test_allocator<int>>>);
    }

    SECTION("bit_ref explicit conversion only") {
        STATIC_REQUIRE(bool_like<runtime_arr<bool>::bit_ref>);
        STATIC_REQUIRE(!convertible_to_bool<runtime_arr<bool>::bit_ref>);
    }

    SECTION("iterator type correctness") {
        STATIC_REQUIRE(std::is_same_v<begin_t<runtime_arr<int>>, int *>);
        STATIC_REQUIRE(std::is_same_v<begin_t<runtime_arr<int, test_allocator<int>>>, int *>);
        STATIC_REQUIRE(std::is_same_v<begin_t<runtime_arr<bool>>, runtime_arr<bool>::iterator>);
    }

    SECTION("range compliance") {
        STATIC_REQUIRE(std::ranges::range<runtime_arr<int>>);
        STATIC_REQUIRE(std::ranges::random_access_range<runtime_arr<int>>);
        STATIC_REQUIRE(!std::ranges::range<runtime_arr<bool>>);
        STATIC_REQUIRE(!std::ranges::random_access_range<runtime_arr<bool>>);
    }

    SECTION("address-of access") {
        STATIC_REQUIRE(has_address_of<runtime_arr<int>>);
    }
}

// NOTE: &N required for GCC visibility inside BENCHMARK lambda
TEST_CASE("Advanced Benchmark: runtime_arr vs std::vector<MyPod> (1024x)") {
    constexpr size_t N = 1024;
    std::vector<pod::pair<int, float>> inputs;
    std::vector<MyPod> input_vals;
    std::vector<int> int_vals;
    inputs.reserve(N);

    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> id_dist(0, 10000);
    std::uniform_real_distribution<float> score_dist(0.0f, 1000.0f);

    for (size_t i = 0; i < N; ++i) {
        inputs.emplace_back(id_dist(gen), score_dist(gen));
        input_vals.emplace_back(id_dist(gen), score_dist(gen));
        int_vals.emplace_back(id_dist(gen));
    }

    BENCHMARK_ADVANCED("std::vector<MyPod> by construction (1024x)")() {
            std::vector<MyPod> buffer(N);
            return [buffer = std::move(buffer), &inputs, &N]() mutable {
                buffer.clear();
                for (size_t i = 0; i < N; ++i) {
                    const auto &[id, score] = inputs[i]; // NOLINT
                    buffer[i] = MyPod{id, score};
                }
            };
        };

    BENCHMARK_ADVANCED("runtime_arr<MyPod> by construction (1024x)")() {
            runtime_arr<MyPod> buffer(N);
            return [buffer = std::move(buffer), &inputs, &N]() mutable {
                for (size_t i = 0; i < N; ++i) {
                    const auto &[id, score] = inputs[i]; //NOLINT
                    buffer.set(i, id, score);
                }
            };
        };

    BENCHMARK_ADVANCED("std::vector<MyPod> set by value (1024x)")() {
            std::vector<MyPod> buffer(N);
            return [buffer = std::move(buffer), &input_vals, &N]() mutable {
                buffer.clear();
                for (size_t i = 0; i < N; ++i) {
                    buffer[i] = input_vals[i];
                }
            };
        };

    BENCHMARK_ADVANCED("runtime_arr<MyPod> by value (1024x)")() {
            runtime_arr<MyPod> buffer(N);
            return [buffer = std::move(buffer), &input_vals, &N]() mutable {
                for (size_t i = 0; i < N; ++i) {
                    buffer.set(i, input_vals[i]);
                }
            };
        };

    BENCHMARK_ADVANCED("std::vector<MyPod> set int (1024x)")() {
            std::vector<int> buffer(N);
            return [buffer = std::move(buffer), &int_vals, &N]() mutable {
                buffer.clear();
                for (size_t i = 0; i < N; ++i) {
                    buffer[i] = int_vals[i];
                }
            };
        };

    BENCHMARK_ADVANCED("runtime_arr<MyPod> set int (1024x)")() {
            runtime_arr<int> buffer(N);
            return [buffer = std::move(buffer), &int_vals, &N]() mutable {
                for (size_t i = 0; i < N; ++i) {
                    buffer.set(i, int_vals[i]);
                }
            };
        };
}
