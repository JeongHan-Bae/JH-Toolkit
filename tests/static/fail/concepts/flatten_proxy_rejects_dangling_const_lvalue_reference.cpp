#include "jh/metax/flatten_proxy.h"

#include <tuple>
#include <utility>

int main() {
    std::tuple<const int &> materialized =
        std::move(jh::meta::flatten_proxy{std::tuple{42}});
    return std::get<0>(materialized);
}
