#include <catch2/catch_all.hpp>
#include "jh/meta"

using namespace jh::meta;

// Example wide check from your instruction
template<typename Inner, typename Variant>
struct some_check {
    using _unused [[maybe_unused]] = Variant;
    static constexpr bool value = std::is_default_constructible_v<Inner>;
};

// Narrow check: require trivial type
template<typename T>
struct is_trivial_check {
    static constexpr bool value = std::is_trivial_v<T>;
};

TEST_CASE("check_all - wide example from user") {
    using V = std::variant<int, double>;
    STATIC_REQUIRE(check_all<some_check, V>);
}

TEST_CASE("check_all - narrow success") {
    using V = std::variant<int, char>;
    STATIC_REQUIRE(check_all<is_trivial_check, V>);
}

TEST_CASE("check_all - narrow failure") {
    using V = std::variant<int, std::string>;
    STATIC_REQUIRE_FALSE(check_all<is_trivial_check, V>);
}

// transformations
template<typename T>
struct as_pointer {
    using type = std::unique_ptr<T>;
};

template<typename T>
struct as_shared {
    using type = std::shared_ptr<T>;
};

template<typename T>
struct as_weak {
    using type = std::weak_ptr<T>;
};

template<typename T>
struct as_int {
    using _unused [[maybe_unused]] = T;
    using type = int;
};

TEST_CASE("deduce_type_t basic correctness") {
    using V = std::variant<int, double>;
    STATIC_REQUIRE(std::is_same_v<deduce_type_t<0, V, as_pointer>, std::unique_ptr<int>>);
    STATIC_REQUIRE(std::is_same_v<deduce_type_t<1, V, as_pointer>, std::unique_ptr<double>>);
}

TEST_CASE("variant_transform_t pointer") {
    using V = std::variant<int, double>;
    using R = variant_transform_t<V, as_pointer>;
    STATIC_REQUIRE(std::is_same_v<
            R,
            std::variant<std::unique_ptr<int>, std::unique_ptr<double>>
    >);
}

TEST_CASE("variant_transform_t shared") {
    using V = std::variant<int, double>;
    using R = variant_transform_t<V, as_shared>;
    STATIC_REQUIRE(std::is_same_v<
            R,
            std::variant<std::shared_ptr<int>, std::shared_ptr<double>>
    >);
}

TEST_CASE("variant_transform_t weak") {
    using V = std::variant<int, double>;
    using R = variant_transform_t<V, as_weak>;
    STATIC_REQUIRE(std::is_same_v<
            R,
            std::variant<std::weak_ptr<int>, std::weak_ptr<double>>
    >);
}


TEST_CASE("variant_collapse_t collapse to single type") {
    using V = std::variant<int, double>;
    using R = variant_collapse_t<V, as_int>;
    STATIC_REQUIRE(std::is_same_v<R, int>);
}

TEST_CASE("variant_collapse_t fail -> void") {
    using V = std::variant<int, double>;
    using R = variant_collapse_t<V, as_pointer>;
    STATIC_REQUIRE(std::is_same_v<R, void>);
}

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
