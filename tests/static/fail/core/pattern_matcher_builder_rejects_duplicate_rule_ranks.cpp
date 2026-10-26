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

jh::pattern_matcher_builder<int, 512, first_rule{}, second_rule{}> invalid_builder;
