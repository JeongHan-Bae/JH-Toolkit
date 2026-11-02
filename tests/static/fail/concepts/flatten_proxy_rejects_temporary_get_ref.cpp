#include "jh/metax/flatten_proxy.h"

#include <tuple>

namespace {
    [[maybe_unused]] int &reference =
        jh::meta::flatten_proxy{std::tuple{42}}.get_ref<0>();
}
