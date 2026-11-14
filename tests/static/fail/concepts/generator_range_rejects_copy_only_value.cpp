#include "jh/generator"

struct copy_only {
    copy_only(const copy_only&) = default;
    copy_only(copy_only&&) = delete;
};

using invalid = jh::async::generator_range<copy_only>;
