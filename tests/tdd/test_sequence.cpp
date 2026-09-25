#include <catch2/catch_all.hpp>

#include <algorithm>
#include <ranges>

#include "jh/conceptual/sequence.h"
#include "jh/pod"

TEST_CASE("Sequence to Range") {
    constexpr jh::pod::array<int, 3> vec = {1, 2, 3}; // sequence, not a range
    auto range_ = jh::to_range(vec);
    auto it = range_.begin();

    std::ranges::for_each(range_, [&](const int a) {
        REQUIRE(a == *it);
        ++it;
    });

}
