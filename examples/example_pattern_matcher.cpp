/**
 * @file example_pattern_matcher.cpp
 * @brief Demonstrates immutable pattern matching and capture ownership.
 */

#include <jh/pattern_matcher>

#include <iostream>
#include <string>
#include <string_view>
#include <utility>

struct segment_rule {
    static constexpr std::size_t rank = 10;

    std::pair<bool, std::size_t> match(std::string_view input) const {
        const auto end = input.find('/');
        const auto length = end == std::string_view::npos ? input.size() : end;
        return {length > 0, length};
    }
};

int main() {
    using namespace jh::pattern;
    jh::pattern_matcher_builder<std::string, 512, segment_rule{}> builder;
    builder.insert({{{"/items/", rule_tag<segment_rule{}>{}},
                     {"", none{}}}, "item"});
    builder.insert({{{"/items", none{}}}, "index"});
    auto matcher = std::move(builder).build();

    std::string input = "/items/42";
    auto result = matcher.match(input);
    if (!result) return 1;

    std::string owned_capture(result.value().first[0]);
    std::cout << result.value().second << ' ' << owned_capture << '\n';
    return 0;
}
