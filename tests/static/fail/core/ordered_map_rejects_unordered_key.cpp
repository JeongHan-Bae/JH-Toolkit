#include "jh/ordered_map"

struct unordered_key final {
    int value;
};

namespace {
    using invalid_map = jh::ordered_map<unordered_key, int>;
    invalid_map map{};
}
