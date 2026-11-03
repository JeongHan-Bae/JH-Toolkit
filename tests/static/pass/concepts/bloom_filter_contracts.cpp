#include "jh/bloom_filter"
#include "jh/pod"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>

struct tuple_hash {
    constexpr auto operator()(const int &value) const {
        return std::tuple{value, value + 1};
    }
};

struct pod_hash {
    constexpr auto operator()(const int &value) const {
        return jh::pod::make_tuple(value + 2, value + 3);
    }
};

struct array_hash {
    constexpr auto operator()(const int &value) const {
        return std::array{value + 4, value + 5, value + 6};
    }
};

struct scalar_hash {
    constexpr int operator()(const int &) const { return 1; }
};

struct nonnumeric_hash {
    auto operator()(const int &) const { return std::tuple{std::string{"bad"}}; }
};

struct explicit_hash_value {
    explicit operator std::size_t() const { return 7; }
};

struct explicit_hash {
    auto operator()(const int &) const { return std::tuple{explicit_hash_value{}}; }
};

struct move_only_hash_value {
    move_only_hash_value() = default;
    move_only_hash_value(const move_only_hash_value &) = delete;
    move_only_hash_value(move_only_hash_value &&) = default;
    explicit operator std::size_t() const { return 7; }
};

struct move_only_hash {
    auto operator()(const int &) const { return std::tuple{move_only_hash_value{}}; }
};

template<std::size_t Modulo>
concept accepts_modulo = requires {
    typename jh::bloom_filter<int, Modulo, jh::typed::monostate, tuple_hash{}>;
};

template<auto Hasher>
concept accepts_hasher = requires {
    typename jh::bloom_filter<int, 65, jh::typed::monostate, Hasher>;
};

struct no_allocator {};

template<typename T>
struct missing_allocate {
    using value_type = T;
    missing_allocate() = default;
    template<typename U>
    missing_allocate(const missing_allocate<U> &) {}
    void deallocate(T *, std::size_t) noexcept {}
};

template<typename Allocator>
concept accepts_allocator = requires {
    typename jh::bloom_filter<int, 65, Allocator, tuple_hash{}>;
};

static_assert(accepts_modulo<1>);
static_assert(accepts_modulo<jh::pod::max_pod_array_bytes * 8>);
using maximum_filter = jh::bloom_filter<int, jh::pod::max_pod_array_bytes * 8,
        jh::typed::monostate, tuple_hash{}>;
static_assert(sizeof(maximum_filter) >= jh::pod::max_pod_array_bytes);
static_assert(!accepts_modulo<0>);
static_assert(!accepts_modulo<jh::pod::max_pod_array_bytes * 8 + 1>);
static_assert(accepts_hasher<tuple_hash{}>);
static_assert(accepts_hasher<pod_hash{}>);
static_assert(accepts_hasher<array_hash{}>);
static_assert(accepts_hasher<explicit_hash{}>);
static_assert(!accepts_hasher<move_only_hash{}>);
static_assert(!accepts_hasher<scalar_hash{}>);
static_assert(!accepts_hasher<nonnumeric_hash{}>);
static_assert(accepts_allocator<jh::typed::monostate>);
static_assert(accepts_allocator<std::allocator<int>>);
static_assert(!accepts_allocator<const jh::typed::monostate>);
static_assert(!accepts_allocator<no_allocator>);
static_assert(!accepts_allocator<missing_allocate<int>>);

template<typename T>
concept accepts_no_hasher = requires {
    typename jh::bloom_filter<T, 65>;
};
static_assert(!accepts_no_hasher<int>);

using inline_filter = jh::bloom_filter<int, 65, jh::typed::monostate,
        tuple_hash{}, pod_hash{}, array_hash{}>;
using allocated_filter = jh::bloom_filter<int, 65, std::allocator<int>, tuple_hash{}>;
using large_allocated_filter = jh::bloom_filter<int, 8388608,
        std::allocator<std::uint64_t>, tuple_hash{}>;
static_assert(inline_filter::bit_count == 65);
static_assert(inline_filter::word_count == 2);
static_assert(large_allocated_filter::word_count == 131072);
static_assert(sizeof(large_allocated_filter) < 1024);
static_assert(std::is_default_constructible_v<inline_filter>);
static_assert(std::is_default_constructible_v<allocated_filter>);
static_assert(!std::is_copy_constructible_v<inline_filter>);
static_assert(!std::is_move_constructible_v<inline_filter>);
static_assert(!std::is_copy_constructible_v<allocated_filter>);
static_assert(!std::is_move_constructible_v<allocated_filter>);

template<typename Allocator, std::size_t Modulo>
concept accepts_storage = requires {
    typename jh::bloom_filter<int, Modulo, Allocator, tuple_hash{}>;
};
static_assert(!accepts_storage<jh::typed::monostate, 131073>);
static_assert(accepts_storage<std::allocator<std::uint64_t>, 131073>);
static_assert(accepts_storage<std::allocator<std::uint64_t>, 8388608>);
