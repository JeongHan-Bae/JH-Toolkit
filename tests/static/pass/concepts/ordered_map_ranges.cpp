#include <ranges>

#include "jh/ordered_map"

static_assert(std::ranges::bidirectional_range<jh::ordered_set<int>>);
static_assert(std::ranges::bidirectional_range<jh::ordered_map<int, int>>);

