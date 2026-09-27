#define TINY_TEST_MAIN
#include "jh/test/tiny_test/tiny_test.hpp"

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

#include <cmath>
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
void tiny_test_case_3() {
    int x = 7;
    flatten_proxy p{ std::tuple{ std::ref(x), std::tuple{2, 3} } };

    auto [a, b, c] = p;

    jh::test::tiny_test::expect(static_cast<bool>((a.get() == 7)), "a.get() == 7");
    jh::test::tiny_test::expect(static_cast<bool>((b == 2)), "b == 2");
    jh::test::tiny_test::expect(static_cast<bool>((c == 3)), "c == 3");

    // implicit conversion to std::tuple
    std::tuple<int&, int, int> t = p;
    jh::test::tiny_test::expect(static_cast<bool>((std::get<0>(t) == 7)), "std::get<0>(t) == 7");

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
