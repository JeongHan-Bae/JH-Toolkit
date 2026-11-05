#include "jh/conceptual/sequence.h"

struct NonMovableRange {
    int values[2]{1, 2};

    NonMovableRange() = default;
    NonMovableRange(const NonMovableRange&) = delete;
    NonMovableRange(NonMovableRange&&) = delete;

    const int* begin() const { return values; }
    const int* end() const { return values + 2; }
};

NonMovableRange make_range() { return NonMovableRange{}; }

auto invalid = jh::to_range(make_range());
