#include <catch2/catch_all.hpp>
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


TEST_CASE("adl_apply expands user tuple-like") {
    demo::proxy p{10, 3.5};
    auto r = adl_apply([](auto &&a, auto &&b){
        return a + b;
    }, p);
    REQUIRE(r == Catch::Approx(13.5));
}

TEST_CASE("tuple_materialize flattens nested tuple") {
    auto t = std::tuple{1, std::tuple{2, 3}};
    auto flat = tuple_materialize(t);

    STATIC_REQUIRE(std::is_same_v<decltype(flat), std::tuple<int,int,int>>);
    REQUIRE(flat == std::tuple{1,2,3});
}

TEST_CASE("flatten_proxy behaves as a flattened tuple") {
    int x = 7;
    flatten_proxy p{ std::tuple{ std::ref(x), std::tuple{2, 3} } };

    auto [a, b, c] = p;

    REQUIRE(a.get() == 7);
    REQUIRE(b == 2);
    REQUIRE(c == 3);

    // implicit conversion to std::tuple
    std::tuple<int&, int, int> t = p;
    REQUIRE(std::get<0>(t) == 7);
}
