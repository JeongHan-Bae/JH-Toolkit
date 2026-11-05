#include "jh/conceptual/sequence.h"

#include <initializer_list>

auto invalid = jh::to_range(std::initializer_list<int>{1, 2, 3});
