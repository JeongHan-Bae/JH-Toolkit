#include "jh/metax/flatten_proxy.h"
#include "jh/pod"

namespace {
    const auto proxy = jh::meta::flatten_proxy{jh::pod::make_tuple(1, 2)};
    [[maybe_unused]] const int &reference = proxy.get_ref<0>();
}
