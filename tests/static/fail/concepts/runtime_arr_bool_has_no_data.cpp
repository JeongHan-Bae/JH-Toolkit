#include "jh/runtime_arr"

#include <utility>

namespace {
    using invalid_data_access = decltype(std::declval<jh::runtime_arr<bool> &>().data());
}
