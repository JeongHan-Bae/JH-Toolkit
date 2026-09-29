#include "jh/flat_multimap"

struct unordered_key final {
    int value;
};

namespace {
    using invalid_map = jh::flat_multimap<unordered_key, int>;
    invalid_map map{};
}
