#include <cstddef>
#include <string_view>
#include <utility>

#include "jh/pattern_matcher"

struct first_rule {
    static constexpr std::size_t rank = 10;
    std::pair<bool, std::size_t> match(std::string_view) const { return {true, 0}; }
};

struct second_rule {
    static constexpr std::size_t rank = 10;
    std::pair<bool, std::size_t> match(std::string_view) const { return {true, 0}; }
};

using invalid_matcher = jh::pattern_matcher<int, 512, first_rule{}, second_rule{}>;
static_assert(sizeof(invalid_matcher) > 0);
