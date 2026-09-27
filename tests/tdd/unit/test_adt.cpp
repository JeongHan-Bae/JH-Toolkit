#define TINY_TEST_MAIN
#include "jh/test/tiny_test/tiny_test.hpp"

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

#include <cmath>
#include <functional>
#include <memory>
#include <tuple>
#include <utility>
#include "jh/meta"

using namespace jh::meta;

namespace demo {

    struct proxy {
        int i;
        double d;
    };

    template<std::size_t I>
    decltype(auto) get(proxy& p) noexcept {
        if constexpr (I == 0) return (p.i);
        else return (p.d);
    }

    template<std::size_t I>
    decltype(auto) get(const proxy& p) noexcept {
        if constexpr (I == 0) return (p.i);
        else return (p.d);
    }

} // namespace demo

namespace std {

    template<>
    struct tuple_size<demo::proxy> : std::integral_constant<size_t, 2> {};

    template<size_t I>
    struct tuple_element<I, demo::proxy> {
        using type = std::conditional_t<I==0, int, double>;
    };

} // namespace std



namespace test {
void tiny_test_case_1() {
    demo::proxy p{10, 3.5};
    auto r = adl_apply([](auto &&a, auto &&b){
        return a + b;
    }, p);
    jh::test::tiny_test::expect(static_cast<bool>((std::abs(r - 13.5) < 1e-9)), "std::abs(r - 13.5) < 1e-9");

}
}
template<>
struct jh::test::tiny_test::test<"adl_apply expands user tuple-like">
    : jh::test::tiny_test::test_definition<"adl_apply expands user tuple-like", &::test::tiny_test_case_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_adt case 1">
    : jh::test::tiny_test::session_definition<
          "test module test_adt case 1", jh::test::tiny_test::test<"adl_apply expands user tuple-like">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_adt case 1"> registration_1{};
}



namespace test {
void tiny_test_case_2() {
    auto t = std::tuple{1, std::tuple{2, 3}};
    auto flat = tuple_materialize(t);

    jh::test::tiny_test::expect(static_cast<bool>((flat == std::tuple{1,2,3})), "flat == std::tuple{1,2,3}");

}
}
template<>
struct jh::test::tiny_test::test<"tuple_materialize flattens nested tuple">
    : jh::test::tiny_test::test_definition<"tuple_materialize flattens nested tuple", &::test::tiny_test_case_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_adt case 2">
    : jh::test::tiny_test::session_definition<
          "test module test_adt case 2", jh::test::tiny_test::test<"tuple_materialize flattens nested tuple">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_adt case 2"> registration_2{};
}



namespace test {
struct transfer_probe {
    static inline int copies = 0;
    static inline int moves = 0;

    int value;

    explicit transfer_probe(int input) : value(input) {}
    transfer_probe(const transfer_probe& other) : value(other.value) { ++copies; }
    transfer_probe(transfer_probe&& other) noexcept : value(other.value) { ++moves; }
};

void tiny_test_case_3() {
    int x = 7;
    flatten_proxy p{ std::tuple{ std::tie(x), std::tuple{2, 3} } };

    auto [a, b, c] = p;

    jh::test::tiny_test::expect(static_cast<bool>((a == 7)), "a == 7");
    jh::test::tiny_test::expect(static_cast<bool>((b == 2)), "b == 2");
    jh::test::tiny_test::expect(static_cast<bool>((c == 3)), "c == 3");

    std::tuple<int&, int, int> t = p;
    jh::test::tiny_test::expect(static_cast<bool>((std::get<0>(t) == 7)), "std::get<0>(t) == 7");
    std::get<0>(t) = 8;
    jh::test::tiny_test::expect(static_cast<bool>((x == 8)), "x == 8");

    std::tuple<const int&, int, int> const_refs = p;
    jh::test::tiny_test::expect(static_cast<bool>((std::get<0>(const_refs) == 8)), "const ref target");
    std::tuple<int, int, int> values = p;
    jh::test::tiny_test::expect(static_cast<bool>((values == std::tuple{8, 2, 3})), "value target");

    flatten_proxy wrapped_source{std::tuple{std::ref(x)}};
    std::tuple<int&> unwrapped_reference = wrapped_source;
    jh::test::tiny_test::expect(
        static_cast<bool>((std::get<0>(unwrapped_reference) == 8)),
        "source wrapper materializes as a reference"
    );

    transfer_probe wrapper_source_value{23};
    flatten_proxy wrapped_rvalue_proxy{std::tuple{std::ref(wrapper_source_value)}};
    transfer_probe::copies = 0;
    transfer_probe::moves = 0;
    std::tuple<transfer_probe> &&copied_from_wrapper = std::move(wrapped_rvalue_proxy);
    jh::test::tiny_test::expect(
        static_cast<bool>((transfer_probe::copies == 1 && transfer_probe::moves == 1)),
        "moving a proxy copies its referenced value, then moves the copy"
    );
    jh::test::tiny_test::expect(
        static_cast<bool>((
            wrapper_source_value.value == 23 &&
            std::get<0>(copied_from_wrapper).value == 23
        )),
        "wrapper source remains unchanged"
    );

    transfer_probe tied_value{31};
    flatten_proxy tied_rvalue_proxy{std::tie(tied_value)};
    transfer_probe::copies = 0;
    transfer_probe::moves = 0;
    std::tuple<transfer_probe> &&copied_then_moved = std::move(tied_rvalue_proxy);
    jh::test::tiny_test::expect(
        static_cast<bool>((transfer_probe::copies == 1 && transfer_probe::moves == 1)),
        "rvalue proxy copies a tied value, then moves the copy"
    );
    jh::test::tiny_test::expect(
        static_cast<bool>((tied_value.value == 31 && std::get<0>(copied_then_moved).value == 31)),
        "moving a tied proxy leaves its source unchanged"
    );

    std::tuple<transfer_probe> nested_value{transfer_probe{37}};
    transfer_probe mutable_value{41};
    const transfer_probe const_value{43};
    transfer_probe wrapped_value{47};
    auto reference_wrapper = std::ref(wrapped_value);
    flatten_proxy mixed_proxy{
        std::tie(nested_value, mutable_value, const_value, reference_wrapper)
    };

    std::tuple<transfer_probe&, transfer_probe&, const transfer_probe&, transfer_probe&> refs =
        std::move(mixed_proxy);
    jh::test::tiny_test::expect(
        static_cast<bool>((
            &std::get<0>(refs) == &std::get<0>(nested_value) &&
            &std::get<1>(refs) == &mutable_value &&
            &std::get<2>(refs) == &const_value &&
            &std::get<3>(refs) == &wrapped_value
        )),
        "rvalue tie preserves mixed reference sources"
    );

    transfer_probe::copies = 0;
    transfer_probe::moves = 0;
    std::tuple<transfer_probe, transfer_probe, transfer_probe, transfer_probe> mixed_copies =
        mixed_proxy;
    jh::test::tiny_test::expect(
        static_cast<bool>((transfer_probe::copies == 4 && transfer_probe::moves == 0)),
        "lvalue proxy copies mixed sources directly"
    );

    transfer_probe::copies = 0;
    transfer_probe::moves = 0;
    auto mixed_rvalue_proxy = std::move(mixed_proxy);
    std::tuple<transfer_probe, transfer_probe, transfer_probe, transfer_probe> mixed_rvalues =
        std::move(mixed_rvalue_proxy);
    jh::test::tiny_test::expect(
        static_cast<bool>((transfer_probe::copies == 4 && transfer_probe::moves == 4)),
        "rvalue proxy copies referenced sources and moves copies"
    );
    jh::test::tiny_test::expect(
        static_cast<bool>((
            std::get<0>(nested_value).value == 37 &&
            mutable_value.value == 41 &&
            const_value.value == 43 &&
            wrapped_value.value == 47 &&
            std::get<0>(mixed_copies).value == 37 &&
            std::get<0>(mixed_rvalues).value == 37
        )),
        "mixed sources remain unchanged after materialization"
    );

}
}
template<>
struct jh::test::tiny_test::test<"flatten_proxy behaves as a flattened tuple">
    : jh::test::tiny_test::test_definition<"flatten_proxy behaves as a flattened tuple", &::test::tiny_test_case_3> {};
template<>
struct jh::test::tiny_test::session<"test module test_adt case 3">
    : jh::test::tiny_test::session_definition<
          "test module test_adt case 3", jh::test::tiny_test::test<"flatten_proxy behaves as a flattened tuple">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_adt case 3"> registration_3{};
}



namespace test {
void tiny_test_case_4() {
    transfer_probe original{17};
    flatten_proxy lvalue_proxy{std::tie(original)};

    transfer_probe::copies = 0;
    transfer_probe::moves = 0;
    std::tuple<transfer_probe> copied = lvalue_proxy;
    jh::test::tiny_test::expect(
        static_cast<bool>((transfer_probe::copies == 1 && transfer_probe::moves == 0)),
        "lvalue conversion copies directly into target"
    );
    jh::test::tiny_test::expect(
        static_cast<bool>((std::get<0>(copied).value == 17)),
        "lvalue target value"
    );

    flatten_proxy rvalue_proxy{std::tuple{transfer_probe{29}}};
    transfer_probe::copies = 0;
    transfer_probe::moves = 0;
    std::tuple<transfer_probe> moved = std::move(rvalue_proxy);
    jh::test::tiny_test::expect(
        static_cast<bool>((transfer_probe::copies == 0 && transfer_probe::moves == 1)),
        "rvalue conversion moves directly into target"
    );
    jh::test::tiny_test::expect(
        static_cast<bool>((std::get<0>(moved).value == 29)),
        "rvalue target value"
    );

    flatten_proxy move_only_proxy{std::tuple{std::make_unique<int>(41)}};
    std::tuple<std::unique_ptr<int>> move_only_target = std::move(move_only_proxy);
    jh::test::tiny_test::expect(
        static_cast<bool>((*std::get<0>(move_only_target) == 41)),
        "move-only target"
    );

    std::tuple<transfer_probe> &&returned_tuple =
        flatten_proxy{std::tuple{transfer_probe{59}}};
    jh::test::tiny_test::expect(
        static_cast<bool>((std::get<0>(returned_tuple).value == 59)),
        "rvalue tuple result remains alive when bound to a reference"
    );
}
}
template<>
struct jh::test::tiny_test::test<"flatten_proxy forwards elements without an intermediate value tuple">
    : jh::test::tiny_test::test_definition<
          "flatten_proxy forwards elements without an intermediate value tuple",
          &::test::tiny_test_case_4
      > {};
template<>
struct jh::test::tiny_test::session<"test module test_adt case 4">
    : jh::test::tiny_test::session_definition<
          "test module test_adt case 4",
          jh::test::tiny_test::test<
              "flatten_proxy forwards elements without an intermediate value tuple"
          >
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_adt case 4"> registration_4{};
}
