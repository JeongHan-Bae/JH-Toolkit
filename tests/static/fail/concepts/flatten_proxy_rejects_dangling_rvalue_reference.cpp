#include "jh/metax/flatten_proxy.h"

#include <tuple>
#include <utility>

namespace {
    [[maybe_unused]] std::tuple<int &&> materialized =
        std::move(jh::meta::flatten_proxy{std::tuple{42}});
}
