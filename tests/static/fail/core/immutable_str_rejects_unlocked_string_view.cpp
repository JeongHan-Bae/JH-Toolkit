#include "jh/immutable_str"

#include <string_view>

namespace {
    jh::immutable_str invalid_construction{std::string_view{"text"}};
}
