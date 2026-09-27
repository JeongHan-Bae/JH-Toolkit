#include "jh/metax/flatten_proxy.h"

#include <functional>
#include <tuple>

int main() {
    int value = 0;
    // Proxy construction is valid; only retaining a wrapper in the materialized tuple is rejected.
    jh::meta::flatten_proxy proxy{std::tuple{std::tuple{std::ref(value)}}};
    std::tuple<std::reference_wrapper<int>> materialized = proxy;
    return std::get<0>(materialized).get();
}
