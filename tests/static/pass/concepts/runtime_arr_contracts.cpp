#include <concepts>
#include <new>
#include <ranges>
#include <span>
#include <type_traits>
#include <utility>

#include "jh/runtime_arr"

template<class T>
concept has_mutable_data = requires(T& value) {
    { value.data() } -> std::same_as<typename T::value_type*>;
};

template<class T>
concept has_mutable_span = requires(T& value) {
    { value.as_span() } -> std::same_as<std::span<typename T::value_type>>;
};

template<class T>
concept has_const_span = requires(const T& value) {
    { value.as_span() } -> std::same_as<std::span<const typename T::value_type>>;
};

template<class T>
using begin_t = decltype(std::declval<T>().begin());

template<class T>
struct test_allocator {
    static T* allocate(std::size_t count)
    {
        return static_cast<T*>(::operator new[](count * sizeof(T)));
    }

    static void deallocate(T* pointer, std::size_t)
    {
        ::operator delete[](pointer);
    }
};

using jh::runtime_arr;
using jh::runtime_arr_helper::bool_flat_alloc;

static_assert(has_mutable_data<runtime_arr<int>>);
static_assert(has_mutable_span<runtime_arr<int>>);
static_assert(has_const_span<runtime_arr<int>>);
static_assert(!has_mutable_data<runtime_arr<bool>>);
static_assert(!has_mutable_span<runtime_arr<bool>>);
static_assert(!has_const_span<runtime_arr<bool>>);
static_assert(has_mutable_data<runtime_arr<bool, bool_flat_alloc>>);
static_assert(has_mutable_span<runtime_arr<bool, bool_flat_alloc>>);
static_assert(has_const_span<runtime_arr<bool, bool_flat_alloc>>);

static_assert(jh::concepts::indirectly_writable<runtime_arr<bool>::iterator, bool>);
static_assert(jh::concepts::output_iterator<runtime_arr<bool>::iterator, bool>);
static_assert(std::output_iterator<runtime_arr<bool>::iterator, bool>);
static_assert(std::ranges::range<runtime_arr<int>>);
static_assert(std::ranges::range<runtime_arr<bool, bool_flat_alloc>>);
static_assert(std::ranges::random_access_range<runtime_arr<int>>);
static_assert(std::ranges::random_access_range<runtime_arr<bool, bool_flat_alloc>>);
static_assert(!std::ranges::range<runtime_arr<bool>>);
static_assert(!std::ranges::random_access_range<runtime_arr<bool>>);

static_assert(std::same_as<begin_t<runtime_arr<int>>, int*>);
static_assert(std::same_as<begin_t<runtime_arr<int, test_allocator<int>>>, int*>);
static_assert(std::same_as<begin_t<runtime_arr<bool>>, runtime_arr<bool>::iterator>);
static_assert(has_mutable_data<runtime_arr<int, test_allocator<int>>>);
static_assert(requires(runtime_arr<int>& value) { &value[0]; });

using ReboundArray = runtime_arr<int, std::allocator<double>>;
static_assert(!std::same_as<ReboundArray::allocator_type, std::allocator<double>>);
static_assert(std::same_as<ReboundArray::allocator_type,
                           std::allocator_traits<std::allocator<double>>::rebind_alloc<int>>);
