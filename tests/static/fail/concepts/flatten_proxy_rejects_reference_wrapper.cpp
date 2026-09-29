#include "jh/metax/flatten_proxy.h"

#include <functional>
#include <tuple>

namespace {
    int value = 0;
    // Proxy construction is valid; only retaining a wrapper in the materialized tuple is rejected.
    [[maybe_unused]] jh::meta::flatten_proxy proxy{std::tuple{std::tuple{std::ref(value)}}};
    [[maybe_unused]] std::tuple<std::reference_wrapper<int>> materialized = proxy;
}
