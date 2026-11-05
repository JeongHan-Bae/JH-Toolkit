#include <concepts>
#include <iterator>
#include <ranges>
#include <sstream>
#include <utility>
#include <vector>

#include "jh/ranges/views/zip.h"

#if !JH_HAS_STD_ZIP_VIEW
using ForwardZip = jh::ranges::zip_view<
    std::ranges::ref_view<std::vector<int>>,
    std::ranges::ref_view<std::vector<int>>>;

using InputZip = jh::ranges::zip_view<
    std::ranges::istream_view<int>,
    std::ranges::istream_view<int>>;

template<typename T>
concept ZipAdaptorAccepts = requires(T& source) {
    jh::ranges::views::zip(source, source);
};

static_assert(std::ranges::forward_range<ForwardZip>);
static_assert(std::ranges::forward_range<const ForwardZip>);
static_assert(std::ranges::borrowed_range<ForwardZip>);
static_assert(std::same_as<
    typename std::iterator_traits<std::ranges::iterator_t<ForwardZip>>::iterator_category,
    std::input_iterator_tag>);
static_assert(std::same_as<
    decltype(jh::ranges::zip_view{std::declval<std::vector<int>&>(),
                                  std::declval<std::vector<int>&>()}),
    ForwardZip>);
static_assert(std::ranges::input_range<InputZip>);
static_assert(!std::ranges::forward_range<InputZip>);
static_assert(!std::ranges::range<const InputZip>);
static_assert(ZipAdaptorAccepts<std::vector<int>>);
static_assert(!jh::concepts::sequence<std::ranges::istream_view<int>>);
static_assert(!ZipAdaptorAccepts<std::ranges::istream_view<int>>);
static_assert(!std::ranges::borrowed_range<decltype(
    jh::ranges::views::zip(std::declval<std::vector<int>>(),
                           std::declval<std::vector<int>>()))>);
static_assert(!std::ranges::borrowed_range<decltype(
    std::declval<std::vector<int>>() |
    jh::ranges::views::zip(std::declval<std::vector<int>&>()))>);

void zip_move_only_input_iterators() {
    std::istringstream left{"1 2"};
    std::istringstream right{"3 4"};
    InputZip zipped{std::ranges::istream_view<int>{left},
                    std::ranges::istream_view<int>{right}};
    for (auto [a, b] : zipped) {
        (void)a;
        (void)b;
    }
}
#endif
