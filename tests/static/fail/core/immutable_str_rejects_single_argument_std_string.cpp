#include "jh/immutable_str"

#include <string>

namespace {
    jh::immutable_str invalid_construction{std::string{"text"}};
}
