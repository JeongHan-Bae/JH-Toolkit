#include "jh/runtime_arr"

#include <utility>

namespace {
    using invalid_copy = decltype(
        jh::runtime_arr<int>(std::declval<const jh::runtime_arr<int> &>())
    );
}
