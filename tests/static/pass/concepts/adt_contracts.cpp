#include <concepts>
#include <memory>
#include <string>
#include <type_traits>
#include <variant>

#include "jh/meta"

template<class Inner, class Variant>
struct some_check {
    using _unused [[maybe_unused]] = Variant;
    static constexpr bool value = std::is_default_constructible_v<Inner>;
};

template<class T>
struct is_trivial_check {
    static constexpr bool value = std::is_trivial_v<T>;
};

using WideVariant = std::variant<int, double>;
static_assert(jh::meta::check_all<some_check, WideVariant>);
static_assert(jh::meta::check_all<is_trivial_check, std::variant<int, char>>);
static_assert(!jh::meta::check_all<is_trivial_check, std::variant<int, std::string>>);

template<class T>
struct as_pointer { using type = std::unique_ptr<T>; };
template<class T>
struct as_shared { using type = std::shared_ptr<T>; };
template<class T>
struct as_weak { using type = std::weak_ptr<T>; };
template<class T>
struct as_int {
    using _unused [[maybe_unused]] = T;
    using type = int;
};

static_assert(std::same_as<jh::meta::deduce_type_t<0, WideVariant, as_pointer>, std::unique_ptr<int>>);
static_assert(std::same_as<jh::meta::deduce_type_t<1, WideVariant, as_pointer>, std::unique_ptr<double>>);
static_assert(std::same_as<
    jh::meta::variant_transform_t<WideVariant, as_pointer>,
    std::variant<std::unique_ptr<int>, std::unique_ptr<double>>>);
static_assert(std::same_as<
    jh::meta::variant_transform_t<WideVariant, as_shared>,
    std::variant<std::shared_ptr<int>, std::shared_ptr<double>>>);
static_assert(std::same_as<
    jh::meta::variant_transform_t<WideVariant, as_weak>,
    std::variant<std::weak_ptr<int>, std::weak_ptr<double>>>);
static_assert(std::same_as<jh::meta::variant_collapse_t<WideVariant, as_int>, int>);
static_assert(std::same_as<jh::meta::variant_collapse_t<WideVariant, as_pointer>, void>);
