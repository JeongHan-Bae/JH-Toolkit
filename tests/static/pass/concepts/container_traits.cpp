#include <concepts>
#include <set>
#include <string>
#include <vector>

#include "jh/conceptual/closable_container.h"
#include "jh/conceptual/container_traits.h"

struct DeclaredOnly { using value_type = double; };
struct DeducedOnly { int* begin(); int* end(); };
struct CompatibleProxy {
    using value_type = bool;
    struct proxy { operator bool() const { return true; } };
    proxy* begin();
    proxy* end();
};
struct Conflict {
    using value_type = int;
    struct proxy { operator std::string() const { return {}; } };
    proxy* begin();
    proxy* end();
};
struct Voidish {};
class my_vector : public std::vector<char> {};

template<>
struct jh::container_deduction<my_vector> { using value_type = unsigned char; };

static_assert(std::same_as<jh::concepts::container_value_t<DeclaredOnly>, double>);
static_assert(std::same_as<jh::concepts::container_value_t<DeducedOnly>, int>);
static_assert(std::same_as<jh::concepts::container_value_t<CompatibleProxy>, bool>);
static_assert(std::same_as<jh::concepts::container_value_t<Conflict>, void>);
static_assert(std::same_as<jh::concepts::container_value_t<Voidish>, void>);
static_assert(std::same_as<jh::concepts::container_value_t<my_vector>, unsigned char>);

static_assert(jh::concepts::closable_container_for<std::vector<int>, std::vector<int>>);
static_assert(jh::concepts::closable_container_for<std::set<int>, std::vector<int>>);
static_assert(jh::concepts::closable_container_for<std::vector<double>, std::vector<int>>);
static_assert(!jh::concepts::closable_container_for<std::vector<std::string>, std::vector<int>>);

