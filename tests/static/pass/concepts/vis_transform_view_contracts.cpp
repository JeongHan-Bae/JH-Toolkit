#include <ranges>
#include <span>
#include <utility>
#include <vector>

#include "jh/ranges/views/vis_transform.h"

using ObservedView = decltype(jh::ranges::views::vis_transform(
    std::declval<std::vector<int>&>(), [](int value) { return value * 2; }));
using ObservedSpan = decltype(jh::ranges::views::vis_transform(
    std::declval<std::span<int>>(), [](int value) { return value * 2; }));

static_assert(std::ranges::view<ObservedView>);
static_assert(!std::ranges::borrowed_range<ObservedView>);
static_assert(std::ranges::borrowed_range<std::span<int>>);
static_assert(!std::ranges::borrowed_range<ObservedSpan>);
